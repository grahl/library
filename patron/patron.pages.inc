<?php
// $Id$

/**
 * @file
 * Functions for generating page displays related to the patron module
 */


function patron_list_patrons() {
  $patrons = patron_get_patrons('all');
  if ($patrons) {
    $header = array(t('Name'), t('Status'));
    $header[] = t('Actions');
    $rows = array();
    foreach ($patrons as $patron) {
      if ($patron->disabled == PATRON_DISABLED) {
        $patron->disabled = 'Disabled';
      }
      else {
        $patron->disabled = 'Active';
      }
      $rows[] = array($patron->name_last .', '. $patron->name_first, $patron->disabled, l('View', 'node/'. $patron->nid) .' | '. patron_email_link($patron));
    }
    return theme('table', $header, $rows); 
  }
  else {
    return "<p>No patrons found.</p>";
  }
}

function patron_email_form(&$form_state) {
  $form = array();

  $patron_id = arg(2);
  if ($patron_id) {
    $patron = patron_get_patrons($patron_id);
    $to = $patron[0]->email;
    $name = $patron[0]->name_first .' '. $patron[0]->name_last;
    if ($patron) {
      $form['to'] = array(
        '#type' => 'value',
        '#value' => ($to ? $to : ''),
        '#required' => FALSE,
      );
      $form['patron_name'] = array(
        '#type' => 'item',
        '#title' => t('To'),
        '#value' => $name,
        '#required' => FALSE,
      );
      $form['patron'] = array(
        '#type' => 'value',
        '#value' => $name,
        '#required' => FALSE,
      );
    }
  }

  $form['subject'] = array(
    '#type' => 'textfield',
    '#title' => t('Subject'),
    '#maxlength' => 255,
    '#required' => TRUE
  );
  $form['message'] = array(
    '#type' => 'textarea',
    '#title' => t('Message'),
    '#required' => TRUE,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Send E-mail'),
  );
  
  return $form;
}

function patron_email_form_validate($form, &$form_state) {
  if ($form_state['values']['to'] && !valid_email_address($form_state['values']['to'])) {
    form_set_error('to', t('Invalid patron email.'));
  }
}

function patron_email_form_submit($form, &$form_state) {
  $values = $form_state['values']; 
  if ($values['to']) {
    $to = $values['to'];
    $recipients = $values['patron'];
  }
  else { 
    $patrons = patron_get_patrons();

    $to_array = array();
    $recipient_array = array();
    foreach ($patrons as $patron) {
      $to_array[] = $patron->email;
      $recipient_array[] = $patron->name_first .' '. $patron->name_last;
    }
    $to = implode(',', $to_array);
    $recipients = implode(', ', $recipient_array);
  }
  if (drupal_mail('patron', 'email_form', $to, language_default(), $values)) {
    watchdog('library', 'Sent email to %recipient', array('%recipient' => $recipients));
    drupal_set_message(t('Your message has been sent to '. $recipients));
  }
  else {
    watchdog('error', 'Unable to send email to %recipient', array('%recipient' => $recipients));
  }
  return;
}