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
  watchdog('islandora_westvault', '!basename found.', array(!basename => $basename), WATCHDOG_NOTICE);
  if (!is_dir($final_path)) {
    mkdir($final_path);
    watchdog('islandora_westvault', 'Directory created', array(), WATCHDOG_NOTICE);
  }
  rename(realpath($bag_path), $final_path . $basename);
}

function islandora_westvault_owncloud_sync() {
  $owncoud_dir = variable_get('islandora_westvault_owncloud_path', '/tmp/owncloud/');
}
