<?php
/**
 * @file
 * Drush integration for Islandora Westvault
 * 
 * This script should be run as a Linux cron job with the drush --user
 * parameter indicating an Islandora user with sufficient privileges to
 * access all objects in your Fedora repository and the --root parameter
 * indicating the path to your Drupal installation root.
 *
 * A typical Linux crontab entry (in this case, to run once per day at 
 * 10 PM) for this drush script is ('\' at end of first line is to break
 * command to meet Drupal's 80-character line limitation):
 *
 * 0 22 * * * /usr/bin/drush --root=/var/www --user=fedoraAdmin \
 *   run-islandora-westvault-sync
 */

function islandora_westvault_drush_help($command) {
  switch ($command) {
    case 'drush:run-islandora-westvault-sync':
      return dt('Sync the local Bag directory with the OwnCloud preservation folder.' . 
      'Bags should be left in the directory after syncing. OwnCloud will delete the bags' . 
      'on the next sync after they have been pushed to WestVault.');
    case 'drush:run-islandora-westvault-bagit':
      return dt('Create Bags for all objects that have been marked for preservation.' .
      'Once Bags are generated, they are moved to the configured Owncloud directory.' .
      'PRESERVATION datastreams are updated so that they are not re-bagged in future.');
    }
}

/**
 * Implements hook_drush_command().
 */
function islandora_westvault_drush_command() {
  $items = array();
  $items['run-islandora-westvault-sync'] = array(
    'description' => dt('Sync the local Bag directory with the OwnCloud preservation folder.'),
    'examples' => array(
      'Standard example' => 'drush -u 1 run-islandora-westvault-sync',
      'Alias example' => 'drush -u 1 westvault-sync',
    ),
    'aliases' => array('westvault-sync'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );
  $items['run-islandora-westvault-bagit'] = array(
    'description' => dt('Create Bags for all objects marked for preservation.'),
    'examples' => array(
      'Standard example' => 'drush -u 1 run-islandora-westvault-bagit',
      'Alias example' => 'drush -u 1 westvault-bagit',
    ),
    'aliases' => array('westvault-bagit'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );
  return $items;
}

/**
 * Validation function for drush run-islandora-westvault-sync.
 */
function drush_islandora_westvault_run_islandora_westvault_sync_validate() {
  $user = variable_get('islandora_westvault_credentials_user', '');
  $password = variable_get('islandora_westvault_credentials_password', '');
  if (!isset($user) || !isset($password)) {
    $error_message = t('Please set OwnCloud credentials in module configuration.');
    watchdog("islandora_westvault", '%error_message', array('%error_message' => $error_message), WATCHDOG_ERROR);
    return drush_set_error('WESTVAULT_CREDENTIALS_NOT_SET', $error_message);
  }
}

function drush_islandora_westvault_run_islandora_westvault_sync() {
  $owncloud_dir = variable_get('islandora_westvault_owncloud_path', '/tmp/owncloud/');
  $user = variable_get('islandora_westvault_credentials_user', '');
  $password = variable_get('islandora_westvault_credentials_password', '');
  $endpoint = variable_get('islandora_westvault_endpoint', 'https://deposit.coppul.ca/remote.php/webdav/lockss-preserved');
  $command = 'owncloudcmd -u ' . $user . ' -p ' . $password . ' ' . $owncloud_dir . ' ' . $endpoint;
  exec($command);
}

function drush_islandora_westvault_run_islandora_westvault_bagit() {
  // Goal: Detect objects with the preservation flag and preserve them. Skip objects already preserved.
  module_load_include('module', 'islandora_bagit');
  $plugins = variable_get('islandora_bagit_object_plugins', array(''));
  $collection_setting = variable_get('islandora_bagit_multiple_bag_type', 'object');
  $complex_cmodels = variable_get('islandora_bagit_complex_cmodels', array(''));
  if ($plugins['plugin_object_ds_basic'] == '0') {
    $error_message = t('Operation aborted. Select the plugin "plugin_object_ds_basic" in your Islandora Bagit configuration.');
    watchdog("islandora_westvault", '%error_message', array('%error_message' => $error_message), WATCHDOG_ERROR);
    return drush_set_error('DS_BASIC_PLUGIN_MISSING', $error_message);
  }
  if ($collection_setting !== 'no_children') {
    $error_message = t('Operation aborted. In Islandora Bagit Configuration, select "Collecion object only" under Collection Batch Type.');
    watchdog("islandora_westvault", '%error_message', array('%error_message' => $error_message), WATCHDOG_ERROR);
    return drush_set_error('BAGIT_COLLECTION_CONFIG', $error_message);
  }
  if ($complex_cmodels['islandora:bookCModel'] == '0' || $complex_cmodels['islandora:islandora:newspaperIssueCModel'] == '0' || $complex_cmodels['islandora:compoundCModel'] != '0' || $complex_cmodels['islandora:newspaperCModel'] != '0') {
    $error_message = t('Operation aborted. In Islandora Bagit Configuration, under Complex Objects select only Books and Newspaper Issues.');
    watchdog("islandora_westvault", '%error_message', array('%error_message' => $error_message), WATCHDOG_ERROR);
    return drush_set_error('BAGIT_COMPLEX_CONFIG', $error_message);
  }
  module_load_include('inc', 'islandora_westvault', 'includes/utilities');
  $timestamp = time();
  // Prepare list of PIDs with PRESERVATION datastream
  $object_list = islandora_westvault_get_preservation_targets();  
  // Eliminate already-preserved objects from list
  $filtered_list = array();
  foreach ($object_list AS $object) {
    $object_test = islandora_object_load($object);
    // Read the XML from the existing datastream, and look for an "already preserved" tag.
    $ds = $object_test["PRESERVATION"];
    $xml = simplexml_load_string($ds->content);
    if (!isset($xml->confirmPreserved)) {    
      $filtered_list[] = $object;
    }
  }
  global $base_url;
  $sitename = parse_url($base_url);

  // Pass object list to Bag-It
  module_load_include('module', 'islandora_bagit');
  $bagit_default_dir = variable_get('file_public_path', conf_path() . '/files');
  $bagit_directory = variable_get('islandora_westvault_owncloud_path', '/tmp/owncloud/');
  $bag_filename_prefix = variable_get('islandora_bagit_bag_name', 'Bag-');

  foreach ($filtered_list AS $object) {
    $object_get = islandora_object_load($object);
    // Bag-it does not work on collections.
    islandora_bagit_create_bag($object_get);

    // Now validate bag before updating PRESERVATION

    $is_valid = islandora_westvault_validate_bag($object, $bagit_directory, $bag_filename_prefix);

    if ($is_valid == TRUE) {
      // Update the PRESERVATION datastream to timestamp successful preservation
      $ds = $object_get["PRESERVATION"];
      $ds_content = $ds->content;
      $dom = new DOMDocument();
      $dom->preserveWhiteSpace = FALSE;
      $dom->formatOutput = TRUE;
      $dom->loadXML($ds_content);
      $fragment = "<confirmPreserved>" . $timestamp . "</confirmPreserved>";
      $frag = $dom->createDocumentFragment();
      $frag->appendXML($fragment);
      $dom->documentElement->appendChild($frag);
      $ds_content_updated = $dom->saveXML($dom->documentElement);
      $ds->setContentFromString($ds_content_updated);
    }
  // If the bag is not valid, delete it so it doesn't get preserved, and log it.
    else {
      $filename_pid = str_replace(array(':', '-'), '_', $object);
      $bag_file_name = $bag_filename_prefix . $filename_pid . ".tgz";
      $bag_path = $bagit_directory . $bag_file_name;
      unlink($bag_path);

      $error_message = dt("Bag !bag was missing files and will be removed. Check directory permissions and try again.",
        array('!bag' => $bag_path));
      watchdog("islandora_westvault", '%error_message', array('%error_message' => $error_message), WATCHDOG_ERROR);
    }
  }
}

function islandora_westvault_validate_bag($pid, $bagit_directory, $bag_filename_prefix) {
  $filename_pid = str_replace(array(':', '-'), '_', $pid);

  $bag_file_name = $bag_filename_prefix . $filename_pid . ".tgz";
  $bag_path = $bagit_directory . $bag_file_name;
  $bagit_library_dir = variable_get('islandora_bagit_library_dir', 'BagItPHP');
  if ($bagit_library_path = libraries_get_path($bagit_library_dir)) {
    require_once $bagit_library_path . '/lib/bagit.php';
  }
  $bag = new BagIt($bag_path);
  $bag_files = array();
  if ($bag->isValid()) {
    drush_log(dt("Bag at !path validates.", array('!path' => $bag_path), 'info'));
    foreach ($bag->getBagContents() as $filename) {
      $bag_files[] = $filename;
    }
  // Get the object's Solr document and use the fedora_datastreams_ms field to compare.
    $datastreams = islandora_westvault_get_datastreams($pid);

    $bag_contents = array();

    foreach ($bag_files AS $bag_file) {
      $trimmed_filename = explode("/", $bag_file);
      $bag_contents[] = explode(".", end($trimmed_filename))[0];
    }

  // Remove the AUDIT datastream before comparing lists.
    $audit_key = array_search("AUDIT", $datastreams);
    unset($datastreams[$audit_key]);

  // Prevents an incorrect 'false' result in the array_search below.
    $bag_contents['zero'] = $bag_contents[0];
    unset($bag_contents[0]);

  // Compare $datastreams to $bag_contents.
    foreach ($datastreams AS $datastream) {
      $datastream_locator = array_search($datastream, $bag_contents, TRUE);

      if ($datastream_locator == FALSE) {
        drush_log(dt("Bag at !path missing the !datastream datastream.", array('!path' => $bag_path, '!datastream' => $datastream), 'warning'));
        $is_valid = FALSE;
      }
    }
    if (!isset($is_valid)) {
      $is_valid = TRUE;
    }
  }
  else {
    drush_log(dt("Bag at !path does not validate.", array('!path' => $bag_path), 'warning'));
    $is_valid = FALSE;
  }

  return ($is_valid);
}

function islandora_westvault_get_datastreams($pid) {
  $qp = new IslandoraSolrQueryProcessor();
  $qp->buildQuery(format_string('@field:"@pid"', array(
    '@field' => 'PID',
    '@pid' => "{$pid}")
  ));
  $query_field = 'fedora_datastreams_ms';
  $qp->solrParams['fl'] = implode(', ', array(
    'PID',
    $query_field,
  ));
  $qp->solrStart = 0;
  $qp->solrLimit = 100000;
  $qp->executeQuery(FALSE);
  if ($qp->islandoraSolrResult['response']['numFound'] > 0) {
    if (array_key_exists($query_field, $qp->islandoraSolrResult['response']['objects']['0']['solr_doc'])) {
      $datastreams_array = ($qp->islandoraSolrResult['response']['objects']['0']['solr_doc'][$query_field]);
    }
  }
  return ($datastreams_array);
}
