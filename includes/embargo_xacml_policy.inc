<?php
/**
 * Created by PhpStorm.
 * User: rwaltz
 * Date: 4/5/17
 * Time: 10:52 AM
 */

function islandora_scholar_embargo_xacml_policy_create($pid, $dsids)
{
    $object = islandora_object_load($pid);
    $users = islandora_scholar_embargo_users_to_notify($object);
    $rule_users = array();
    foreach ($users as $user) {
        $rule_users[] = $user->name;
    }
    $roles = variable_get('islandora_scholar_embargo_whitelisted_roles', array());

    $policy_dom = islandora_scholar_embargo_xacml_policy_read($pid);
    $xpath = new DOMXPath($policy_dom);
    $result = $xpath->query("//Rule[@RuleId='deny-embargoed-ds']");
    if ($result->length > 0) {
        islandora_scholar_embargo_xacml_policy_delete();

    }

    $xacml_xsl = XacmlCreateXslt::getInstance();
    $xsl_proc = $xacml_xsl->getXsltProcessor();
    $xsl_proc->setParameter('', 'users', implode(',', $rule_users));

    if (isset($dsids) && (count($dsids) > 0)) {
        $xsl_proc->setParameter('', 'dsids', implode(',', $dsids));
    } else {
        $xsl_proc->setParameter('', 'dsids', '');
    }

    $xacml_string = $xsl_proc->transformToXml($policy_dom);

    if (!$xacml_string) {
        throw new Exception("unable to transform XACML");
    }

    if (isset($object['POLICY']->content)) {
        $modifyDatastreamArgs = array('dsString' => $xacml_string);
        # update
        $object->repository->api->m->modifyDatastream($pid, 'POLICY', $modifyDatastreamArgs);
    } else {
        # create
        $createDatastreamArgs = array('dsVersionable' => true, 'dsState' => 'A', 'dsMIME' => 'text/xml', 'dsLabel' => 'POLICY Datastream', 'dsChecksumType' => 'SHA-256');
        $object->repository->api->m->addDatastream($pid, 'POLICY', 'string', $xacml_string, $createDatastreamArgs);
    }
}

function islandora_scholar_embargo_xacml_policy_read($pid)
{
    $xacml_xml = "";
    $object = islandora_object_load($pid);
    if (isset($object['POLICY'])) {
        $xacml_xml = $object['POLICY']->content;
    } else {
        # when to load from the filesystem
        $module_path = drupal_get_path('module', 'islandora_scholar_embargo');
        $filepath = "$module_path/xml/islandora_empty_policy.xml";
        if (file_exists($filepath)) {
            $handle = fopen($filepath, "r");
            $xacml_xml = fread($handle, filesize($filepath));
            fclose($handle);
        } else {
            throw new Exception($filepath . " does not exist");
        }
    }
    $xacml_dom = islandora_scholar_embargo_dom_document_load($xacml_xml);
    return $xacml_dom;
}

function islandora_scholar_embargo_xacml_policy_update($pid, $dsids)
{
    islandora_scholar_embargo_xacml_policy_create($pid, $dsids);
}

function islandora_scholar_embargo_xacml_policy_delete($pid)
{
    $object = islandora_object_load($pid);
    $xacml_dom = islandora_scholar_embargo_xacml_policy_read($pid);
    $xacml_xsl = XacmlDeleteXslt::getInstance();
    $xsl_proc = $xacml_xsl->getXsltProcessor();

    $xacml_string = $xsl_proc->transformToXml($xacml_dom);
    $modifyDatastreamArgs = array('dsString' => $xacml_string);
    $object->repository->api->m->modifyDatastream($pid, 'POLICY', $modifyDatastreamArgs);
}

function islandora_scholar_embargo_dom_document_load($xacml_string)
{

    // Do some funky workaround stuff in order to get loadXML to throw an
    // exception instead of outputting a warning then failing in an ugly way
    // later on.
    $xacml_dom = new DomDocument();
    $xacml_dom->preserveWhiteSpace = FALSE;

    // Throw exception if DomDocument gave us a Parse error.
    if ($xacml_dom->loadXML($xacml_string) == FALSE) {
        throw new XacmlException('Error Parsing XML.');
    }

    // Do basic sanity check that root element is <Policy>.
    if ($xacml_dom->documentElement->tagName != 'Policy') {
        throw new XacmlException('Root tag is not Policy.');
    }

    return $xacml_dom;
}

class XacmlDeleteXslt extends XacmlXslt
{
    private static $xsltFilename = "xacml_delete_embargo_rule.xsl";

    public static function getInstance()
    {
        if (!isset($singletonXacmlXslt)) {
            $singletonXacmlXslt = new XacmlDeleteXslt();
        }
        return $singletonXacmlXslt;
    }

    protected function getXsltFilename()
    {
        //'static' will be resolved to the child class at run-time, so there will be a separate static variable for each child class
        return self::$xsltFilename;
    }
}

class XacmlCreateXslt extends XacmlXslt
{
    private static $xsltFilename = "xacml_create_embargo_rule.xsl";

    public static function getInstance()
    {
        if (!isset($singletonXacmlXslt)) {
            $singletonXacmlXslt = new XacmlCreateXslt();
        }
        return $singletonXacmlXslt;
    }

    protected function getXsltFilename()
    {
        //'static' will be resolved to the child class at run-time, so there will be a separate static variable for each child class
        return self::$xsltFilename;
    }
}

abstract class XacmlXslt
{
    private static $singletonXacmlXslt;
    private $xsltProcessor;
    private static $xsltFilename = NULL;

    protected function __construct()
    {

    }

    abstract static public function getInstance();

    abstract protected function getXsltFilename();

    public function getXsltProcessor()
    {
        $xacml_xsl = "";
        if (!isset($xsltProcessor)) {
            $stylesheetFilename = $this->getXsltFilename();
            $xsltProcessor = new XSLTProcessor();
            $module_path = drupal_get_path('module', 'islandora_scholar_embargo');
            $filepath = "$module_path/xml/$stylesheetFilename";
            if (file_exists($filepath)) {
                $handle = fopen($filepath, "r");
                $xacml_xsl = fread($handle, filesize($filepath));
                fclose($handle);
            } else {
                throw new Exception($filepath . " does not exist");
            }
            $xslt_dom = new DomDocument();
            $xslt_dom->preserveWhiteSpace = FALSE;

            // Throw exception if DomDocument gave us a Parse error.
            if ($xslt_dom->loadXML($xacml_xsl) == FALSE) {
                throw new XacmlException('Error Parsing XML.');
            }
            if (!$xsltProcessor->importStylesheet($xslt_dom)) {
                throw new Exception($filepath . " does not contain valid xsl");
            }
        }
        return $xsltProcessor;
    }

}