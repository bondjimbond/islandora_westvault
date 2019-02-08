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
function islandora_westvault_form($form, &$form_state, $object) {
  $get_default_value = function($name, $default) use(&$form_state) {
    return isset($form_state['values'][$name]) ? $form_state['values'][$name] : variable_get($name, $default);
  };
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
