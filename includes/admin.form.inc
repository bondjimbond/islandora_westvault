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
  $form['islandora_westvault_owncloud_path'] = array(
    '#type' => 'textfield',
    '#title' => t('OwnCloud Local Path'),
    '#description' => t('Path to your OwnCloud sync directory. Include trailing slash.'),
    '#default_value' => variable_get('islandora_westvault_owncloud_path', '/tmp/owncloud/'),
  );
  $form['islandora_westvault_endpoint'] = array(
    '#type' => 'textfield',
    '#title' => t('OwnCloud WebDAV URL'),
    '#description' => t('URL to your OwnCloud WestVault directory.sync directory. No trailing slash.'),
    '#default_value' => variable_get('islandora_westvault_endpoint', 'https://deposit.coppul.ca/remote.php/webdav/lockss-preserved'),
  );
  $form['islandora_westvault_pid_file_location'] = array(
    '#type' => 'textfield',
    '#title' => t('Temporary file location'),
    '#description' => t('Path to your temporary file where PIDs of objects to be preserved are stored. Include trailing slash.'),
    '#default_value' => variable_get('islandora_westvault_pid_file_location', '/tmp/'),
  );
  $form['islandora_westvault_credentials_user'] = array(
    '#type' => 'textfield',
    '#title' => t('OwnCloud user ID'),
    '#description' => t('Your user ID for the OwnCloud account associated with your WestVault account.'),
    '#default_value' => variable_get('islandora_westvault_credentials_user', ''),
  );
  $passexist = variable_get('islandora_westvault_credentials_password', 0);
  if (isset($passexist)) {
    $password_message = "Password is saved.";
  }
  else {
    $password_message = "No password saved.";
  }
  $form['islandora_westvault_credentials_password'] = array(
    '#type' => 'password',
    '#title' => t('OwnCloud password'),
    '#description' => t('Your password for the OwnCloud account associated with your WestVault account. Leave blank if password is already saved.
      <b>Current status: ' . $password_message . '</b>'),
    '#required' => !isset($passexist),
  );
  $form = system_settings_form($form);
  array_unshift($form['#submit'], 'islandora_westvault_default_password');
  return $form;
}

function islandora_westvault_default_password($form, &$form_state) {
  if ($form_state['values']['islandora_westvault_credentials_password'] == '') {
    $form_state['values']['islandora_westvault_credentials_password'] = variable_get('islandora_westvault_credentials_password', '');
  }
  $form['buttons']['submit'] = array('#type' => 'submit', '#value' => t('Save configuration'));
  $form['#submit'] = array('islandora_westvault_credentials_password', 'islandora_westvault_admin_submit_form');

  return $form;
}

function islandora_westvault_admin_submit_form($form, &$form_state) {
    foreach ($form_state['values'] as $key => $value) {
    if (is_array($value) && isset($form_state['values']['array_filter'])) {
      $value = array_keys(array_filter($value));
    }
    variable_set($key, $value);
  }
  drupal_set_message(t('The settings have been updated!'));
}
