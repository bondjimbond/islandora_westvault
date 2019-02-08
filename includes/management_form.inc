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
  print_r($pid);
  $form['islandora_westvault_preserve_object'] = array(
    '#type' => 'checkbox',
    '#title' => t('Preserve this object?'),
//    '#return_value' => 1,
    '#default_value' => variable_get('islandora_westvault_preserve_object', FALSE),
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Configuration'),
  );

  return array($form);
}

function islandora_westvault_form_submit($form, &$form_state) {
print_r($form_state['values']);
print_r('hoho');
  if ($form['islandora_westvault_preserve_object'] == TRUE) {
print_r("Preserve true");
  if (empty($object['PRESERVATION'])) {
    $ds = $object->constructDatastream('PRESERVATION', 'X');
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
