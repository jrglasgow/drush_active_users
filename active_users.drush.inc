<?php

/**
 * @file
 *
 * Adds a drush command to see what users are currently active on the site.
 */


 /**
 * Implements hook_drush_command().
 */
function active_users_drush_command() {
  $items = array();

  $items['active-users'] = array(
    'description'   =>  'List active users on the site.',
    'arguments'     => array(
      'timeout in minutes'  => 'Timout for the last time the user loaded a page, default is 15 minutes',
    ),
    'aliases'       => array('users'),
    'bootstrap'     => DRUSH_BOOTSTRAP_DRUPAL_FULL, // Full bootstrap into Drupal,
  );

  return $items;
}

/**
 * Fetch and display a list of users active in the system.
 */
function drush_active_users($timeout = 15) {
  // Count users active within the defined period.
  $interval = REQUEST_TIME - ($timeout * 60);

  // Perform database queries to gather online user lists. We use s.timestamp
  // rather than u.access because it is much faster.
  $authenticated_count = db_query("SELECT COUNT(DISTINCT s.uid) FROM {sessions} s WHERE s.timestamp >= :timestamp AND s.uid > 0", array(':timestamp' => $interval))->fetchField();

  drush_print(format_plural($authenticated_count, 'There is currently 1 user online.', 'There are currently @count users online.'));

  // Display a list of currently online users.
  $max_users = variable_get('user_block_max_list_count', 10);
  if ($authenticated_count && $max_users) {
    $items = db_query_range('SELECT u.uid, u.name, MAX(s.timestamp) AS max_timestamp FROM {users} u INNER JOIN {sessions} s ON u.uid = s.uid WHERE s.timestamp >= :interval AND s.uid > 0 GROUP BY u.uid, u.name ORDER BY max_timestamp DESC', 0, $max_users, array(':interval' => $interval))->fetchAll();

    foreach($items AS $key => $item) {
      $items[$key]->last_access = time_ago($item->max_timestamp);
      unset($items[$key]->max_timestamp);
    }
    array_unshift($items, array(dt('User ID'), dt('User Name'), dt('Last Access')));
    drush_print();
    drush_print_table($items, TRUE);
  }
}

function time_ago($timestamp) {
  $seconds = REQUEST_TIME - $timestamp;
  $time = '';
  $minutes = 0;
  if ($seconds >= 60 ) {
    $minutes = intval($seconds / 60);
    $seconds = $seconds - ($minutes * 60);
  }

  return dt('!min minutes, !sec seconds', array('!min' => $minutes, '!sec' => $seconds));
}
