<?php

/**
 * @file
 * This file holds supporting admin functions for Islandora Westvault module.
 */

/**
 * Defines the admin settings form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_westvault_admin(array $form, array &$form_state) {
  $form['islandora_westvault_lockss_endpoint'] = array(
    '#type' => 'textfield',
    '#title' => t('LOCKSS endpoint'),
    '#description' => t('URL to your LOCKSS API endpoint.'),
    '#default_value' => variable_get('islandora_westvault_lockss_endpoint', ''),
  );
  $form['islandora_westvault_pid_file_location'] = array(
    '#type' => 'textfield',
    '#title' => t('Temporary file location'),
    '#description' => t('Path to your temporary file where PIDs of objects to be preserved are stored. Include trailing slash.'),
    '#default_value' => variable_get('islandora_westvault_pid_file_location', '/tmp/'),
  );
  $form['islandora_westvault_credentials_user'] = array(
    '#type' => 'textfield',
    '#title' => t('WestVault user ID'),
    '#description' => t('User ID for WestVault API'),
    '#default_value' => variable_get('islandora_westvault_credentials_user', ''),
  );
  $form['islandora_westvault_credentials_password'] = array(
    '#type' => 'textfield',
    '#title' => t('WestVault password'),
    '#description' => t('Password for WestVault API'),
    '#default_value' => variable_get('islandora_westvault_credentials_password', ''),
  );
  return system_settings_form($form);
}

