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
    return drush_set_error('WESTVAULT_CREDENTIALS_NOT_SET', dt('Please set OwnCloud credentials in module configuration.'));
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
    return drush_set_error('DS_BASIC_PLUGIN_MISSING', dt('Operation aborted. Select the plugin "plugin_object_ds_basic" in your Islandora Bagit configuration.'));
  }
  if ($collection_setting !== 'no_children') {
    return drush_set_error('BAGIT_COLLECTION_CONFIG', dt('Operation aborted. In Islandora Bagit Configuration, select "Collecion object only" under Collection Batch Type.'));
  }
  if ($complex_cmodels['islandora:bookCModel'] == '0' || $complex_cmodels['islandora:islandora:newspaperIssueCModel'] == '0' || $complex_cmodels['islandora:compoundCModel'] != '0' || $complex_cmodels['islandora:newspaperCModel'] != '0') {
    return drush_set_error('BAGIT_COMPLEX_CONFIG', dt('Operation aborted. In Islandora Bagit Configuration, under Complex Objects select only Books and Newspaper Issues.'));
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
  // Use bag-it
  module_load_include('module', 'islandora_bagit');
  foreach ($filtered_list AS $object) {
    $object_get = islandora_object_load($object);
    // Bag-it does not work on collections.
      islandora_bagit_create_bag($object_get);
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
}
