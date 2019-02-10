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
  $pid = $object->id;
  $form['islandora_westvault_preserve_object'] = array(
    '#type' => 'checkbox',
    '#title' => t('Preserve this object?'),
    '#default_value' => variable_get('islandora_westvault_preserve_object', 0),
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Configuration'),
  );

  return array($form);
}

function islandora_westvault_form_submit($form, &$form_state) {
  variable_set('islandora_westvault_preserve_object', $form_state['values']['islandora_westvault_preserve_object']);
  if ($form_state['values']['islandora_westvault_preserve_object'] === 1) {
    $object = islandora_object_load(arg(2));
    if (empty($object['PRESERVATION'])) {
      $ds = $object->constructDatastream('PRESERVATION', 'X');
      $ds->label = 'WestVault Preservation data';
      $ds->setContentFromString('<preservation></preservation>');
      $object->ingestDatastream($ds);
    }
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
