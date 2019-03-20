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


// Special treatment for Collection objects
  if (in_array('islandora:collectionCModel', $object->models)) {
    module_load_include('inc', 'islandora', 'includes/utilities');
    $form['islandora_westvault_collection'] = array(
      '#type' => 'select',
      '#title' => t('Apply preservation policy to:'),
      '#options' => array(
        'newchildren' => t('New children of this collection (excludes current children)'),
        'allchildren' => t('All children of this collection (existing and new)'),
        'currentchildren' => t('Current children only - new objects will not be preserved'),
      ),
    );
    module_load_include('module', 'islandora_basic_collection');
    $collection_children = islandora_basic_collection_get_member_objects($object);
  }
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Configuration'),
  );

  return array($form);
}

function islandora_westvault_form_submit($form, &$form_state) {
  $object = islandora_object_load(arg(2));
  if ($form_state['values']['islandora_westvault_preserve_object'] === 1) {
    islandora_westvault_add_preservation_datastream($object, $form_state);
    if (in_array('islandora:collectionCModel', $object->models)) {
      // Copy the PRESERVATION info to all the children.
      if ($form_state['values']['islandora_westvault_collection'] == 'allchildren' || 
      $form_state['values']['islandora_westvault_collection'] == 'currentchildren') {
        module_load_include('module', 'islandora_basic_collection');
        $collection_children = islandora_basic_collection_get_member_objects($object);
        foreach ($collection_children[1] AS $child) {
          // Apply the PRESERVATION datastream!
          $child_object = islandora_object_load($child['object']['value']);
          islandora_westvault_add_preservation_datastream($child_object);
        }
      }
    }
  }
  else {
    $object->purgeDatastream('PRESERVATION');
    drupal_set_message('Preservation data removed');
  }
}

function islandora_westvault_add_preservation_datastream($object, &$form_state = NULL) {
  if (empty($object['PRESERVATION'])) {
      $time = time();
      $ds = $object->constructDatastream('PRESERVATION', 'X');
      $ds->label = 'WestVault Preservation data';
      $content_string = '<preservation>
  <datePreserved>' . date("Y-m-d H:i:s", $time) . '</datePreserved>
  <preservedTimestamp>' . $time . '</preservedTimestamp>';
      if (in_array('islandora:collectionCModel', $object->models)) {
        $preservation_method = $form_state['values']['islandora_westvault_collection'];
dd($preservation_method);
        if ($preservation_method == 'allchildren' || $preservation_method == 'newchildren') {
          $content_string = $content_string . '<preserveNewChildren>TRUE</preserveNewChildren>';
        }
      }
      $content_string = $content_string . '</preservation>';

      $ds->setContentFromString($content_string);
      $object->ingestDatastream($ds);
      drupal_set_message('Preservation data created');
  }
}
