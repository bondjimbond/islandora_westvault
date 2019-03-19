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
