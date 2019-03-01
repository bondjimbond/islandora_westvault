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
        'newchildren' => t('New children of this collection'),
        'allchildren' => t('All children of this collection'),
        'deep_traversal' => t('All children of this collection and collections within this collection (existing and new).'),
      ),
    );

        module_load_include('module', 'islandora_basic_collection');
        $collection_children = islandora_basic_collection_get_member_objects($object);
//print_r($collection_children[1]);
foreach ($collection_children[1] AS $child) {
  print_r($child['object']['value']);
}
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
/*    if (empty($object['PRESERVATION'])) {
      $time = time();
      $ds = $object->constructDatastream('PRESERVATION', 'X');
      $ds->label = 'WestVault Preservation data';
      $ds->setContentFromString('<preservation>
  <datePreserved>' . date("Y-m-d H:i:s", $time) . '</datePreserved>
  <preservedTimestamp>' . $time . '</preservedTimestamp>
  </preservation>');
      $object->ingestDatastream($ds);
      drupal_set_message('Preservation data created'); */
    islandora_westvault_add_preservation_datastream($object);
      if (in_array('islandora:collectionCModel', $object->models)) {
      // Copy the PRESERVATION info to all the children.
        module_load_include('module', 'islandora_basic_collection');
        $collection_children = islandora_basic_collection_get_member_objects($object);
        foreach ($collection_children[1] AS $child) {
          // Apply the PRESERVATION datastream!
          islandora_westvault_add_preservation_datastream($child);
//        }
      }

    }
  }
  else {
    $object->purgeDatastream('PRESERVATION');
    drupal_set_message('Preservation data removed');
  }
}

function islandora_westvault_add_preservation_datastream($object) {
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
