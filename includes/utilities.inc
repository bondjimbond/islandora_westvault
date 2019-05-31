<?php
/**
 * @file
 * Connects Connects with Bag-It to create bags and send them to WestVault.
 */
function islandora_westvault_get_preservation_targets() {
  // Looking for the PRESERVATION datastream
  $query = "fedora_datastreams_ms:PRESERVATION";
  $qp = new islandoraSolrQueryProcessor();
  $qp->buildQuery($query);
  $qp->solrParams['fl'] = "PID";
  // Check islandora_compound_object settings and changes query filters to include compound children if necessary.
  if (variable_get('islandora_compound_object_hide_child_objects_solr', TRUE)) {
    $fq = variable_get('islandora_compound_object_solr_fq', '-RELS_EXT_isConstituentOf_uri_mt:[* TO *]');
    if (!empty($fq)) {
    // delete islandora_compound_object_solr_fq from the list of filters
      $filters = $qp->solrParams['fq'];
      if (($key = array_search($fq, $filters)) !== false) {
        unset($filters[$key]);
        $qp->solrParams['fq'] = $filters;
      }
    }
  }
  $qp->solrLimit = 1000000;
  $qp->executeQuery(FALSE);
  try {
    $results = $qp->islandoraSolrResult['response']['objects'];
  }
  catch (Exception $e) {
    watchdog_exception('Islandora Westvault', $e, 'Got an exception searching for PRESERVATION datastreams.', array(), WATCHDOG_ERROR);
    $results = array();
  }
  $objects = array();
  if ($results) {
    foreach ($results AS $result) {
      $objects[] = $result['PID'];
    }
  }  
  return $objects;
}

/**
 * Implements hook_islandora_bagit_post_create().
 *
 * @param string $pid
 *   The PID of the Islandora object that the Bag was just created for.
 *
 * @param string $bag_path
 *  The path to the Bag, relative to the Drupal installation directory.
 */
function islandora_westvault_islandora_bagit_post_create($pid, $bag_path) {
  watchdog('islandora_westvault', 'Post-create initiated', array(), WATCHDOG_NOTICE);
  $final_path = variable_get('islandora_westvault_owncloud_path', '/tmp/owncloud/');
  $basename = basename($bag_path);
  if (!is_dir($final_path)) {
    mkdir($final_path);
    watchdog('islandora_westvault', 'Directory created', array(), WATCHDOG_NOTICE);
  }
  rename(realpath($bag_path), $final_path . $basename);
}
