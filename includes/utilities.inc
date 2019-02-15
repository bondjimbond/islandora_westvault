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
