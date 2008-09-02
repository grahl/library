<?php
//$Id

/**
 * Implementation of hook_views_data()
 */
function library_views_data() {
  $data = array();
  
  // Basic table information.

  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['library']['table']['group']  = t('Library');

  // For other base tables, explain how we join
  $data['library']['table']['join'] = array(
    'node' => array(
      'left_field' => 'nid',
      'field' => 'nid',
     ),
  );

  // ----------------------------------------------------------------
  // Fields
  $data['library']['in_circulation'] = array(
    'title' => t('Reference Only'), // The item it appears as on the UI,
    'help' => t('Whether a library item is in circulation.'), // The help that appears on the UI,
    'filter' => array(
      'title' => t('Reference Only'),
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Reference Only'),
    ),
  );
  
  $data['library']['library_status'] = array(
    'title' => t('Item Status'), // The item it appears as on the UI,
    'help' => t('Whether a library item is available or unavailable. Note that this requires an additional query per row.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_library_status',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_library_status_operator',
      'label' => t('Item Status'),
    ),
  );
  
  $data['library']['quantity'] = array(
    'field' => array(
      'title' => t('Quantity'),
      'handler' => 'views_handler_field_library_quantity',
      'help' => t('Number of copies.'),
     ),
  );
  return $data;
}

/**
 * Display a node's status in the library as a whole
 */
class views_handler_field_library_status extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['nid'] = 'nid';
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function render($values) {
    $nid = $values->{$this->aliases['nid']};
    $node = node_load($nid);
    $item = library_get_status_item($node);
    if ($item) {
      return library_get_status_text($item);
    }
    return '';
  }
}

class views_handler_filter_library_status_operator extends views_handler_filter_boolean_operator {
  function construct() {
    parent::construct();
    $this->value_value = t('Is Available');
  }

  function query() {
    $this->ensure_my_table();
    $field = "$this->table_alias.$this->real_field";
    $field2 = "$this->table_alias.in_circulation";
    $node = $this->query->ensure_table('library');
    $this->query->add_where($this->options['group'], "$field " . (empty($this->value) ? '!=' : '=') . " ". LIBRARY_ITEM_AVAILABLE ." AND $field2 = ". LIBRARY_CIRCULATION);
  }
}

/**
 * Display the library quantity
 */
class views_handler_field_library_quantity extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['nid'] = 'nid';
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function render($values) {
    $nid = $values->{$this->aliases['nid']};
    $node = node_load($nid);
    if (isset($node->items) && !empty($node->items)) {
      return count($node->items);
    }
    return '';
  }
}