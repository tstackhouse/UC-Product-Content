<?php

// $Id$

/**
 * @file
 * Ubercart related hook calls
 */


// ===== HELPER FUNCTIONS===============================================================================================

// No helpers at this time.

// ===== END HELPER FUNCTIONS===========================================================================================

// ===== THEME FUNCTIONS================================================================================================

/**
 * Return Ubercart related theme hooks
 */
function _ucpc_ubercart_themes() {
  return array(
    'uc_product_content_show_order' => array(
      'arguments' => array('node' => NULL),
    ),
    'uc_product_content_checkout_pane_node_info' => array(
      'arguments' => array(),
    ),
    'uc_product_content_order_pane_node_info' => array(
      'arguments' => array(),
    ),
  );
}

/**
 * Theme a node to show the order information.
 */
function theme_uc_product_content_show_order($node) {
  //dsm($node);
  
  $ret = '<div class="field"><div class="field-label">Invoice:</div>';
  $ret .= '<div class="field-items">';
  $ret .= uc_order_load_invoice($node->order);
  $ret .= '</div></div>';
  
  return $ret;
}

/**
 * Display a pane in the checkout screen with node info
 */
function theme_uc_product_content_checkout_pane_node_info() {  
  $cart = uc_cart_get_contents();
  
  $source_nid = '';

  foreach ($cart as $item) {
    if (isset($item->data['product_content_source_nid'])) {
      $source_nid = $item->data['product_content_source_nid'];
    }
  }
  
  if ($source_nid != '') {
    return node_view(node_load($source_nid)) . '<p>Not right? ' . l(t('<< Make Changes'), 'node/' . $source_nid . '/edit') . '</p>';
  }
}

/**
 * Display a pane in the order information with node info
 */
function theme_uc_product_content_order_pane_node_info($node) {
  return l($node->title, 'node/' . $node->nid);
}

/**
 * Display a pane in the cart with node info
 */
/*function _ucpc_cart_pane_node_info($node) {
  return array(
    'id' => 'node_info',
    'title' => t('Node Details'),
    'enabled' => TRUE,
    'weight' => -10,
    'body' => node_view($node),
  );
}

// ===== END THEME FUNCTIONS============================================================================================

// ===== CART FUNCTIONS=================================================================================================

/**
 * 
 */
/*function _ucpc_cart_view_form_alter(&$form, &$form_state) {
  $items = uc_cart_get_contents();
  
  $source_nid = '';
  //$other_items = FALSE;
  //$i = 0;
  foreach ($items as $item) {
    if (isset($item->data['product_content_source_nid'])) {
      $source_nid = $item->data['product_content_source_nid'];
    
      // remove links to the products directly
      $form['items'][$i]['title']['#value'] = $item->title;
    
      // Rebuild the description from the title and options.
      $form['items'][$i]['desc']['#value'] = $form['items'][$i]['title']['#value'] . $form['items'][$i]['description']['#value'];
    
      // don't allow removal of this item in the cart, user must edit the node
      unset($form['items'][$i]['remove']['#type']);
      
      
      // don't allow quantity editing
      // make the quantity field a hidden element and display the value as plain text
      // can't just remove the field b/c cart expects it for checkout submit
      $form['items'][$i]['qty'][0]['#type'] = 'hidden';
      $form['items'][$i]['qty'][0]['#value'] = $form['items'][$i]['qty']['#default_value'];
      $form['items'][$i]['qty'][1]['#value'] = $form['items'][$i]['qty']['#default_value'];
      unset($form['items'][$i]['qty']['#type']);
              
      $i++;
    }
    else {
      $other_items = TRUE;
    }
  }
  
  if ($source_nid != '') {
    // we're dealing with at least one item governed by uc_product_content, send them to checkout!
    //drupal_goto('cart/checkout');
      
    // hijack the continue shopping link to go back and edit
    $form['continue_shopping']['#value'] = l('<< Make Changes', 'node/'. $source_nid .'/edit', array('query' => 'destination=cart'));
    
    // remove the update cart button (possibly move this to the theme layer/site specific tweaks module?)
    unset($form['update']);
  }
  
  // make some more changes to the form if there are no other items besides those governed by product_content
  if (! $other_items) {
    // get rid of the image and remove columns, but make the desc colum have the image column title first
    $form['items']['#columns']['desc']['cell'] = $form['items']['#columns']['image']['cell'];
    unset($form['items']['#columns']['remove']);
    unset($form['items']['#columns']['image']);
  }
}*/

/**
 * Implementation of hook_cart_pane()
 */
/*function uc_product_content_cart_pane($items) {
  $source_nid = '';

  foreach ($items as $item) {
    if (isset($item->data['product_content_source_nid'])) {
      $source_nid = $item->data['product_content_source_nid'];
    }
  }

  $node = node_load($source_nid);

  $panes[] = _ucpc_cart_pane_node_info($node);
  return $panes;
}

// ===== END CART FUNCTIONS=============================================================================================

// ===== ORDER FUNCTIONS================================================================================================

/**
 * Display order information on a node
 */
function _ucpc_show_order(&$node) {
  $node->order = uc_order_load($node->order_id);
  $node->content['order']['#value'] = theme('uc_product_content_show_order', $node);
  $node->content['order']['#weight'] = 100;
}

/**
 * Implementation of hook_order_pane()
 */
function uc_product_content_order_pane() {
  $panes[] = array(
    'id' => 'node_info',
    'callback' => 'uc_product_content_order_pane_node_info',
    'title' => t('Order Information'),
    'desc' => t('Details related to this order'),
    'class' => 'pos-left',
    'weight' => -10,
    'show' => array('view', 'edit', 'customer'),
  );
  return $panes;
}

function uc_product_content_order_pane_node_info($op, $a1) {
  switch ($op) {
    case 'customer':
      return theme('uc_product_content_order_pane_node_info', node_load($a1->nid));
  }
}

// ===== END ORDER FUNCTIONS============================================================================================

// ===== CHECKOUT FUNCTIONS=============================================================================================

/**
 * Implementation of hook_uc_checkout_complete()
 */
function uc_product_content_uc_checkout_complete($order, $account) {
  $source_nid = '';
  
  foreach ($order->products as $item) {
    if (isset($item->data['product_content_source_nid'])) {
      $source_nid = $item->data['product_content_source_nid'];
    }
  }
  
  // if we're dealing with product_checkout products, store an association of node to order
  // and lock the node from user editing, now that the order is complete
  if ($source_nid != '') {
    $node = node_load($source_nid);
    
    // make the magic happen
    // store the order number
    $node->order_id = $order->order_id;
    
    // store the originator uid
    $node->originator = $node->uid;
    
    // give the node to the superuser so that only admins can edit it
    // FUTURE: make a config option or make a module like node_access a depencency
    $node->uid = 1;
        
    node_save($node);
  }
}

/**
 * Implementation of hook_checkout_pane()
 */
function uc_product_content_checkout_pane() {
  $panes[] = array(
    'id' => 'node_info',
    'callback' => 'uc_product_content_checkout_pane_node_info',
    'title' => t('Order Information'),
    'desc' => t("Display information related to the order (the contents of the node)"),
    'weight' => -10,
    'process' => FALSE,
    'collapsible' => FALSE,
  );
  return $panes;
}

function uc_product_content_checkout_pane_node_info($op, $a1, $a2) {
  switch ($op) {
    case 'view':
      $contents['node_info'] = array(
        '#value' => theme('uc_product_content_checkout_pane_node_info'),
        '#weight' => variable_get('uc_pane_cart_field_cart_weight', 2),
      );
      return array('contents' => $contents, 'next-button' => FALSE);
  }
}

// ===== END CHECKOUT FUNCTIONS=========================================================================================


// END OF FILE uc_product_content.ubercart.inc