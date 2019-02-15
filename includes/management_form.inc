<?php
/**
 * @file
 * Management forms for objects and collections.
 */

/**
 * Object management form. 
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 *
 * @return array
 *   Returns the form.
 */
function islandora_westvault_form($form, &$form_state, AbstractObject $object) {

// Just testing the utilities
module_load_include('inc', 'islandora_westvault', 'includes/utilities');
    $object_list = islandora_westvault_get_preservation_targets();
print_r($object_list);

  $pid = $object->id;
  if ($object['PRESERVATION']) {
    $checkbox_default = 1;
  }
  else {
    $checkbox_default = 0;
  }
  $form['islandora_westvault_preserve_object'] = array(
    '#type' => 'checkbox',
    '#title' => t('Preserve this object?'),
    '#default_value' => $checkbox_default,
    '#description' => 'Checking this box creates a PRESERVATION datastream, which will flag this object for preservation on the next cron run.
      Un-checking this box will delete the PRESERVATION datastream, which will not delete the preserved objects, but stop the preservation of updates to the object.',
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Configuration'),
  );

  return array($form);
}

function islandora_westvault_form_submit($form, &$form_state) {
  $object = islandora_object_load(arg(2));
  if ($form_state['values']['islandora_westvault_preserve_object'] === 1) {
    if (empty($object['PRESERVATION'])) {
      $time = time();
      $ds = $object->constructDatastream('PRESERVATION', 'X');
      $ds->label = 'WestVault Preservation data';
      $ds->setContentFromString('<preservation>
  <datePreserved>' . date("Y-m-d H:i:s", $time) . '</datePreserved>
  <preservedTimestamp>' . $time . '</preservedTimestamp>
  </preservation>');
      $object->ingestDatastream($ds);
      drupal_set_message('Preservation data created');
    }
  }
  else {
    $object->purgeDatastream('PRESERVATION');
    drupal_set_message('Preservation data removed');
  }
}

function islandora_westvault_collection_manage_form(array $form, array &$form_state) {
  print_r('Hi hi');
  $form['islandora_westvault_preserve_object'] = array(
    '#type' => 'checkbox',
    '#title' => t('Preserve this object?'),
//    '#return_value' => 1,
    '#default_value' => variable_get('islandora_westvault_preserve_object', FALSE),
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Configuration'),
    '#weight' => 0,
  );
  return array($form);
}
