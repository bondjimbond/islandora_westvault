<?php
/**
 * @file
 * Connects Connects with Bag-It to create bags and send them to WestVault.
 */

function islandora_westvault_get_preservation_targets() {
  module_load_include('inc', 'islandora', 'includes/utilities');

  // Build the Solr query
  $query = 'fedora_datastreams_ms:PRESERVATION';
  $objects = array();
  $query_processor = new IslandoraSolrQueryProcessor();
  $query_processor->solrQuery = $query;
  $fields_to_return = 'PID,fgs_label_s,fedora_datastreams_ms,fedora_datastream_version_PRESERVATION_CREATED_ms,fedora_datastream_latest_PRESERVATION_CREATED_ms';

  $query_processor->solrParams['fl'] = $fields_to_return;
  $query_processor->solrLimit = '1000000';

  $filters = array();


  if (count($filters)) {
    $query_processor->solrParams['fq'] = implode(' OR ', $filters);
  }

  $query_processor->executeQuery(FALSE);

      $objects[] = array(
        'pid' => $object['solr_doc']['PID'],
        'label' => $object['solr_doc']['fgs_label_s'],
      );


  return $objects;
}
