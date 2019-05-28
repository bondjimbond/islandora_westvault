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

  // For Collection objects, select method for preserving children.
  if (in_array('islandora:collectionCModel', $object->models)) {
    $ds = $object["PRESERVATION"];
    $options_default = 'allchildren';
    if (!empty($ds)) {
      $xml = simplexml_load_string($ds->content);
      $options_default = $xml->preservationMethod;
    }  
    // Is the include line below actually needed?
    module_load_include('inc', 'islandora', 'includes/utilities');
    $form['islandora_westvault_collection'] = array(
      '#type' => 'select',
      '#title' => t('Apply preservation policy to:'),
      '#default_value' => $options_default,
      '#options' => array(
        'newchildren' => t('New children of this collection (excludes current children)'),
        'allchildren' => t('All children of this collection (existing and new)'),
        'currentchildren' => t('Current children only - new objects will not be preserved'),
      ),
      '#description' => t('How to handle children of this collection. This setting excludes child collections.
      Child collections must be configured individually.'),
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
      // If this is a collection, proceed with changes to child objects.
      if ($form_state['values']['islandora_westvault_collection'] == 'allchildren' || 
      $form_state['values']['islandora_westvault_collection'] == 'currentchildren') {
        // Create preservation datastreams on all applicable child objects.
        module_load_include('module', 'islandora_basic_collection');
        $collection_children = islandora_basic_collection_get_member_objects($object);
        foreach ($collection_children[1] AS $child) {
          // Apply the PRESERVATION datastream!
          $child_object = islandora_object_load($child['object']['value']);
          if (!in_array('islandora:collectionCModel', $child_object->models)) {
            islandora_westvault_add_preservation_datastream($child_object);
          }
        }
      }
    }
    elseif (in_array('islandora:newspaperCModel', $object->models)) {
      // Add Preservation tag to issues
      module_load_include('inc', 'islandora_newspaper', 'includes/utilities');
      $issues = islandora_newspaper_get_issues($object);
      foreach ($issues AS $issue) {
        $issue_object = islandora_object_load($issue['pid']);
        islandora_westvault_add_preservation_datastream($issue_object);
      }
    }
    // Deal with Compound objects too
    elseif (in_array('islandora:compoundCModel', $object->models)) {
      module_load_include('module', 'islandora_compound_object', 'islandora_compound_object');
      $compound_parts = islandora_compound_object_get_parts($object->id);
      foreach ($compound_parts AS $part) {
        $part_object = islandora_object_load($part);
        islandora_westvault_add_preservation_datastream($part_object);
      }
    }
  }
  else {
    $object->purgeDatastream('PRESERVATION');
    drupal_set_message(t('Preservation data removed'));
  }
}

function islandora_westvault_add_preservation_datastream($object, &$form_state = NULL) {
  if (!empty($object['PRESERVATION'])) {
  // Check if it's a preserved Collection. Then check if the preservation method has changed.
  // If it has, purge and recreate the PRESERVATION datastream.
    if (in_array('islandora:collectionCModel', $object->models)) {
      $ds = $object["PRESERVATION"];
      $xml = simplexml_load_string($ds->content);
      $original_method = $xml->preservationMethod;
      $selected_method = $form_state['values']['islandora_westvault_collection'];
      if ($selected_method != $original_method) {
        $object->purgeDatastream('PRESERVATION');
      }
    }
  }
  if (empty($object['PRESERVATION'])) {
      $time = time();
      $ds = $object->constructDatastream('PRESERVATION', 'X');
      $ds->label = 'WestVault Preservation data';
      $content_string = '<preservation>
  <datePreserved>' . date("Y-m-d H:i:s", $time) . '</datePreserved>
  <preservedTimestamp>' . $time . '</preservedTimestamp>';
      if (in_array('islandora:collectionCModel', $object->models)) {
        $preservation_method = $form_state['values']['islandora_westvault_collection'];
        $content_string = $content_string . '<preservationMethod>' . $preservation_method . '</preservationMethod>';
      }
      $content_string = $content_string . '</preservation>';

      $ds->setContentFromString($content_string);
      $object->ingestDatastream($ds);
      drupal_set_message(t('Preservation data created'));
  }
}
