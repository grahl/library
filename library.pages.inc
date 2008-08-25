<?php
// $Id:

function library_transaction_form(&$form_state, $item = NULL) {
  $aid = arg(3);
  $patron_id = arg(4);
  $form = array();
  global $user;

  if ($aid) {
    $action = library_get_action($aid);
  }
  if ($patron_id) {
    $patron = node_load($patron_id);
  }
  $action_list = array();
  $aids = array();
  if ($item) { 
    if ($item->in_circulation == LIBRARY_REFERENCE_ONLY) {
      $action_array = variable_get('library_links_display_reference', array());
    }
    else if ($item->library_status == LIBRARY_ITEM_AVAILABLE) {
      $action_array = variable_get('library_links_display_available', array());
    }
    else {
      $action_array = variable_get('library_links_display_unavailable', array());
    }
    foreach ($action_array as $aid) {
      $aids[] = $aid;
    }
  }
  foreach (library_actions($aids, $user) as $key => $value) {
    $action_list[$key] = $value['name'];
  }
  if (user_access('administer transactions')) {
    $form['action_aid'] = array(
      '#type' => 'select',
      '#title' => t('Action'),
      '#default_value' => ($action->aid ? $action->aid : ''),
      '#options' => $action_list,
      '#required' => TRUE
    );
  }
  else {
    $form['action_aid'] = array(
      '#type' => 'select',
      '#title' => t('Action'),
      '#default_value' => ($action->aid ? $action->aid : ''),
      '#options' => array($action->aid => $action->name),
      '#required' => TRUE,
      '#disabled' => TRUE,
    );
  }
  $form['item_id'] = library_autocomplete_input($item);
  if (user_access('view patron content')) {
    $form['patron_id'] = array(
      '#type' => 'textfield',
      '#title' => t('Patron'),
      '#default_value' => ($patron ? $patron->title .' [nid:'. $patron->nid .']' : ''),
      '#autocomplete_path' => 'patrons/autocomplete',
      '#required' => TRUE
    );
  }
  elseif ($patron) {
    $form['patron_id'] = array(
      '#type' => 'textfield',
      '#title' => t('Patron'),
      '#value' => ($patron ? $patron->title .' [nid:'. $patron->nid .']' : ''),
      '#disabled' => TRUE,
    );
  }
  else {
    $user_patron = patron_load_by_uid($user->uid);
    if (is_object($user_patron)) {
      $form['patron_id'] = array(
        '#type' => 'textfield',
        '#title' => t('Patron'),
        '#value' => ($user_patron ? $user_patron->title .' [nid:'. $user_patron->nid .']' : ''),
        '#disabled' => TRUE,
      );	
    }
    else {
      form_set_error('', t('You do not have sufficient permission to perform a library transaction.'));
    }
  }
  $form['notes'] = array(
    '#type' => 'textarea',
    '#title' => t('Message'),
    '#required' => FALSE,
    '#maxlength' => 250, 
    '#default_value' => '',
    '#description' => t('If you are reserving an item, make sure to include the date and time you would like it to be ready.'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['#validate'][] = 'library_transaction_form_validate';
  $form['#submit'][] = 'library_transaction_form_submit';
  
  return $form;
}

/**
 * Handles transaction form submissions.
 *
 * @see library_transaction_form()
 */
function library_transaction_form_validate($form, &$form_state) {
  $barcodes = variable_get('library_item_barcodes', LIBRARY_NO_BARCODES) == LIBRARY_BARCODES;
  if ($form_state['values']['op'] == t('Save')) {
    if ($form_state['values']['item_id']) {
      if ($barcodes) {
        preg_match('/^(?:\s*|(.*))?\[\s*title\s*:\s*(.+)\s*\]\s*?\[\s*id\s*:\s*(\d+)\s*\]$/', $form_state['values']['item_id'], $matches);
      }
      else {
        preg_match('/^(?:\s*|(.*))?\[\s*id\s*:\s*(\d+)\s*\]$/', $form_state['values']['item_id'], $matches);
      }
      if (!empty($matches)) {
        if ($barcodes) {
          list($temp, $barcode, $title, $id) = $matches;
        }
        else {
          list($temp, $title, $id) = $matches;
        }
        $item = library_load($id);
        if (isset($item) && !empty($title) && (strcasecmp(rtrim(check_plain($title)), rtrim(check_plain($item->title))) != 0)) {
          form_set_error('item_id', t('Library Item : Title mismatch. Please check your selection.'));
        }
        elseif (!empty($item) && !empty($title) && !empty($id)) {
          $form_state['values']['item_id'] = $item->id;
          $form_state['values']['item_name'] = $item->title;
          $form_state['values']['nid'] = $item->nid;
          $form_state['values']['barcode'] = $item->barcode;
          $form_state['values']['node_type'] = $item->type;
          if ($form_state['values']['action_aid']) {
            $action = library_get_action($form_state['values']['action_aid']);
            if (!empty($action)) {
              if ($item->in_circulation == LIBRARY_REFERENCE_ONLY && $action->status_change <> LIBRARY_ACTION_NO_CHANGE) {
                form_set_error('action_aid', t('This item is not in circulation.'));
              } 
              elseif ($item->library_status == LIBRARY_ITEM_AVAILABLE && $action->status_change == LIBRARY_ITEM_TYPE_AVAILABLE) {
                form_set_error('action_aid', t('This item is already available.'));
              }
              elseif ($item->library_status == LIBRARY_ITEM_UNAVAILABLE &&  $action->status_change == LIBRARY_ITEM_TYPE_UNAVAILABLE) {
                form_set_error('action_aid', t('This item is not currently available.'));
              }
              $form_state['values']['action_name'] = $action->name;
              $form_state['values']['status_change'] = $action->status_change;
            }
          }
        }
        else {
          form_set_error('item_id', t('You must specify a valid library item.'));
        }
      }
      else {
        form_set_error('item_id', t('You must specify a valid library item.'));
      }
    }
    else {
      form_set_error('item_id', t('You must specify a library item.'));
    }
    if ($form_state['values']['patron_id']) {
      preg_match('/^(?:\s*|(.*) )?\[\s*nid\s*:\s*(\d+)\s*\]$/', $form_state['values']['patron_id'], $matches);
      if (!empty($matches)) {
        // explicit nid
        list($temp, $title, $nid) = $matches;
        if (!empty($title) && ($n = node_load($nid))) {
          if ($title != $n->title) {
            form_set_error('patron_id', t('Library Patron : Name mismatch. Please check your selection.'));
          }
          $form_state['values']['patron_email'] = $n->email;
          $form_state['values']['patron_uid'] = $n->patron_uid;
          $form_state['values']['patron_id'] = $nid;
          $form_state['values']['patron_name'] = $title;
          $form_state['values']['patron_name_last'] = $n->name_last;
          $form_state['values']['patron_name_first'] = $n->name_first;
        }
      }
    }
    else {
      form_set_error('patron_id', t('You must specify a library patron.'));
    }
  }
}


/**
 * Handles transaction form submissions.
 *
 * @see library_transaction_form()
 */
function library_transaction_form_submit($form, &$form_state) {
  if ($form_state['values']['op'] == t('Save')) {
    $now = time();
    $duedate = NULL;
    $clean = library_clean_action_name($form_state['values']['action_name']);
    if ($form_state['values']['status_change'] == LIBRARY_ACTION_TYPE_UNAVAILABLE) {
      $type = $form_state['values']['node_type'];
      $period = variable_get('library_period_for_'. $type .'_'. $clean, 0);
      if ($period > 0) {
        $duedate = library_get_due_date($now, $clean, $type);
      }
      $new_status = LIBRARY_ITEM_UNAVAILABLE;
    }
    elseif ($form_state['values']['status_change'] == LIBRARY_ACTION_TYPE_AVAILABLE) {
      $new_status = LIBRARY_ITEM_AVAILABLE;
    }
    db_query("INSERT {library_transactions} (nid, item_id, patron_id, action_aid, duedate, notes, created) VALUES (%d, %d, %d, %d, %d, '%s', %d)", $form_state['values']['nid'], $form_state['values']['item_id'], $form_state['values']['patron_id'], $form_state['values']['action_aid'], $duedate, check_plain($form_state['values']['notes']), $now);
    if ($form_state['values']['status_change'] > LIBRARY_ACTION_NO_CHANGE) {
      db_query("UPDATE {library} set library_status = %d WHERE id = %d", $new_status, $form_state['values']['item_id']);
    }

    drupal_set_message(t($form_state['values']['patron_name_first'] .' '. $form_state['values']['patron_name_last'] .' has performed a '. $form_state['values']['action_name'] .' on '. $form_state['values']['item_name']));
  }
  $transaction_id = db_last_insert_id('library_transactions', 'tid');
  $context = array();
  $object = (object) $context;
  $context['item'] = array(
    'id' => $form_state['values']['item_id'], 'nid' => $form_state['values']['nid'], 'title' => $form_state['values']['item_name'], 'barcode' => $form_state['values']['barcode'], 'node_type' => $form_state['values']['node_type']
  );
  $context['transaction'] = array('id' => $transaction_id, 'action_name' => $form_state['values']['action_name'], 'aid' => $form_state['values']['action_aid'], 'notes' => check_plain($form_state['values']['notes']), 'duedate' => $duedate, 'created' => $now);
  $context['patron'] = array('nid' => $form_state['values']['patron_id'], 'name' => $form_state['values']['patron_name_first'] .' '. $form_state['values']['patron_name_last'], 'email' => $form_state['values']['patron_email'], 'uid' => $form_state['values']['patron_uid']);
  $actions = _library_get_hook_aids('library', 'after_'. $clean);
  if (!empty($actions)) {
    actions_do($actions, $object, $context);
  }
  $form_state['redirect'] = 'node/'. $form_state['values']['nid'];
  return;
}

function library_transaction_view($tid) {
  $transaction = library_get_transaction_by_tid($tid);
  if (isset($transaction)) {
    $list[] = t("Date: @action_date", array('@action_date' => format_date($transaction->created, 'custom', 'M j, Y')));
    $list[] = t("Item: ". l($transaction->item_name, 'node/'. $transaction->nid));
    $list[] = t("Action: @action", array('@action' => $transaction->action_name));
    $list[] = t("Patron: ". l($transaction->name_first .' '. $transaction->name_last, 'node/'. $transaction->patron_id));
	
    if (!empty($transaction->duedate)) {
      $list[] = t('Due Date: @due_date', array('@due_date' => format_date($transaction->duedate, 'custom', 'M j, Y')));
    }
    if (!empty($transaction->notes)) {
      $list[] = $transaction->notes;
    }
    return theme('item_list', $list);
  }
  else {
    drupal_set_message(t('No transaction found.'), array());
  }
}

/**
 * Menu callback; show the transaction history of a single node.
 */
function library_history($node) { 
  $duedate_enabled = library_duedates_enabled($node->type);  
  global $user;
  $user_has_access = user_access('administer transactions') && patron_access('view', $node, $user);
  if ($node->type == 'patron') {
    $is_patron = TRUE;
  }
  else {
    $is_patron = FALSE;
  }
  if ($is_patron && !$user_has_access) {
    drupal_set_message(t('You do not have permission to view patron information.'));
    $form_state['redirect'] = 'library-items';
  }
  if (library_item_in_library($node) || $is_patron) {
    $header = array(t('Date'), t('Transaction'), t('Notes'));
    if ($duedate_enabled) {
      $header[] = t('Due Date');
    }
    if ($user_has_access) {
      $header[] = t('Actions');
    }
    $rows = array();

    $transactions = library_get_transactions_by_node($node);

    if (!empty($transactions[0])) {
      foreach ($transactions as $value) {
        $last_item = '';
        foreach ($value as $transaction) {
          $detail_link = '';
          $action_link = '';
          $cur_item = $transaction->item_id;
          $created = format_date($transaction->created, 'small');
          if ($duedate_enabled && $cur_item <> $last_item && !empty($transaction->duedate)) {
            $duedate = format_date($transaction->duedate, 'small');
          }
          elseif ($duedate_enabled) {
            $duedate = '';
          }
          if ($user_has_access) {
            $detail_link = l('View Details', 'library-items/transaction/view/'. $transaction->tid);

            if ($cur_item <> $last_item) {
              $item = array('id' => $transaction->item_id, 'library_status' => $transaction->library_status, 'last_patron_id' => $transaction->patron_id, 'in_circulation' => $transaction->in_circulation);
              $links = library_get_action_links($item);
              if (!empty($links)) {
                $action_link = ' | '. implode(" | ", $links);
              }
            }
          }
		  
          $temp_array = array($created, $transaction->action_name, $transaction->notes);
          if (isset($duedate)) { 
            $temp_array[] = $duedate; 
          }
          if (!empty($detail_link) || !empty($action_link)) {
            $temp_array[] = $detail_link . $action_link;
          }
          $rows[] = $temp_array;
          $last_item = $cur_item;
        }
      }
      return theme('table', $header, $rows);
    }
    else {
      if ($is_patron) {
        return "<p>This patron has not performed any actions.</p>";
      }
      else {
        return "<p>No actions have been performed on this item.</p>";
      }
    }
  }
  else {
    return "<p>This item type is not part of the library.</p>";
  }
}

function library_display_items() {
  $var = library_get_table_header();
  $header = $var['header'];

  $nodes = library_get_items_group_by_node();
  $rows = array();
  foreach ($nodes as $node) {
    $node_rows = library_get_table_row($node, $var);
    foreach ($node_rows as $row) {
      $rows[] = $row;
    }
  }
  $output = theme('table', $header, $rows, array('class' => 'library-list'));
  $output .= theme('pager', NULL, LIBRARY_RESULTS_PER_PAGE);
  return $output;
}

function library_overdue_items() {
  if (library_duedates_enabled()) {
    $records = library_get_overdue_items();
    if (empty($records)) {
      return "<p>No overdue items at this time.</p>";
    }
    else {
      $header = array(t('Title'), t('Patron'), t('Due Date'), t('Actions'));
      $rows = array();
      foreach ($records as $patron_id => $record) {
        $patron_name = $record['patron']['patron_name'];
        $patron_email = $record['patron']['patron_email'];
        foreach ($record['items'] as $id => $values) {
          $item = array('library_status' => LIBRARY_ITEM_UNAVAILABLE, 'id' => $id, 'last_patron_id' => $patron_id, 'in_circulation' => $values['in_circulation']);
          $links = implode(" | ", library_get_action_links($item));

          $rows[] = array($values['item_name'], $patron_name, format_date($values['due_date'], 'short'), l('Item Details', 'node/'. $values['nid']) .' | '. $links);
          $item = array();
        }
      }
      $output = theme('table', $header, $rows);
      $output .= theme('pager', NULL, LIBRARY_RESULTS_PER_PAGE, 0);
      return $output;
    }
  }
  else {
    return "<p>Library due dates are not enabled.</p>";
  }
}
