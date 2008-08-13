<?php
// $Id:

/**
 * Menu callback: Library Actions
 * Theme Form to create a new action
 *
 * @see library_admin_action()
 * @see library_admin_action_validate()
 * @see library_admin_action_submit()
 */
function theme_library_admin_new_action($form) {
  $header = array(t('Name'), t('Item Status'), t('Edit'));
  foreach (library_actions() as $aid => $action) {
    switch ($action['status_change']) {
      case LIBRARY_ACTION_TYPE_UNAVAILABLE:
        $status_text = 'Unavailable';
        break;
      case LIBRARY_ACTION_TYPE_AVAILABLE:
        $status_text = 'Available';
        break;
      default:
        $status_text = 'No Change';
    }
    $rows[] = array($action['name'], $status_text, l(t('edit action'), 'admin/settings/library/actions/edit/'. $aid));
  }
  $rows[] = array(drupal_render($form['name']), drupal_render($form['status_change']), drupal_render($form['submit']));

  $output = drupal_render($form);
  $output .= theme('table', $header, $rows);

  return $output;
}

function theme_library_items($node) {
  $barcodes = (variable_get('library_item_barcodes', LIBRARY_NO_BARCODES) == LIBRARY_BARCODES);
  $display_status = (variable_get(library_status_display, 0) == 1);
  
  $items = $node->items;
  $header = array(t('Copy'));
  if ($barcodes) {
    $header[] = t('Barcode');
  }
  if ($display_status) {
    $header[] = t('Status');
  }
  $header[] = t('Notes');
  $header[] = t('Actions');
  $rows = array();
  
  foreach ($items as $key => $item) {
    $temp_array = array(($key + 1));
    if ($barcodes) {
      $temp_array[] = $item['barcode'];
    }
    if ($display_status) {
      $temp_array[] = library_get_status_text($item);
    }
    $temp_array[] = $item['notes'];
    $temp_array[] = implode(" | ", library_get_action_links($item)) . ' ';
    $rows[] = $temp_array;
  }
  return theme('table', $header, $rows);
}


/**
 * Theme the items section on library entries.
 *
 * @ingroup themeable
 */
function theme_library_items_field($form) {
  // Change the button title to reflect the behavior when using JavaScript.
  drupal_add_js('if (Drupal.jsEnabled) { $(document).ready(function() { $("#edit-library-more").val("'. t('Add an Item') .'"); }); }', 'inline');

  $barcodes = variable_get('library_item_barcodes', LIBRARY_NO_BARCODES) == LIBRARY_BARCODES;

  $rows = array();

  if ($barcodes) {
    $headers[] = t('Barcode');
  }
  else {
    $headers[] = t('Copy');
  }
  $headers[] = t('Reference Only');
  $headers[] = t('Notes');
  $headers[] = t('');
  $headers[] = t('Delete');

  foreach (element_children($form) as $key) {
    // No need to print the field title every time.
    unset($form[$key]['barcode']['#title'], $form[$key]['in_circulation']['#title'], $form[$key]['notes']['#title'], $form[$key]['delete']['#title']);

    // Build the table row.

    if ($barcodes) {
      $row['data'][] = array('data' => drupal_render($form[$key]['barcode']), 'class' => 'library-barcode');
    }
    else {
      $row['data'][] = array('data' => ($key + 1), 'class' => 'library-copy');
    }

    $row['data'][] = array('data' => drupal_render($form[$key]['in_circulation']), 'class' => 'library_circulation');
	$row['data'][] = array('data' => drupal_render($form[$key]['notes']), 'class' => 'library_notes');
    $row['data'][] = array('data' => drupal_render($form[$key]['id']), 'class' => 'library-id');

    if ($key > 0) {
      $row['data'][] = array('data' => drupal_render($form[$key]['delete']), 'class' => 'library-delete');
    }
    else {
      $row['data'][] = array();
    }

    // Add additional attributes to the row, such as a class for this row.
    if (isset($form[$key]['#attributes'])) {
      $row = array_merge($row, $form[$key]['#attributes']);
    }
    $rows[] = $row;
    $row = array();
  }

  $output = theme('table', $headers, $rows);
  $output .= drupal_render($form);
  return $output;
}