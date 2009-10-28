<?php

// $Id$

/**
 * @file
 * CCK related hook calls
 * 
 * @author Tim Stackhouse
 */

// ===== HELPER FUNCTIONS===============================================================================================

function _ucpc_get_product($nid) {
  if ($nid == 'none') {
    $obj = (object) array('title' => 'None');
    return $obj;
  }
  else {
    return node_load($nid);
  }
}

// ===== END HELPER FUNCTIONS===========================================================================================

//===== THEME FUNCTIONS ================================================================================================

/**
 * Return CCK related theme hooks
 */
function _ucpc_cck_themes() {
  return array(
    'uc_product_content_radios' => array(
      'arguments' => array('element' => NULL),
      ),
    'uc_product_content_formatter_default' => array(
      'arguments' => array('element' => NULL),
    ),
  );
}

/**
 * Implementation of hook_field_formatter_info().
 */
function uc_product_content_field_formatter_info() {
  return array(
    'default' => array(
      'label' => t('Default'),
      'field types' => array('uc_product_content'),
      'multiple values' => CONTENT_HANDLE_CORE,
    ),
  );
}

/**
 * Theme function for 'default' product field formatter.
 */
function theme_uc_product_content_formatter_default($element) {
  $product = _ucpc_get_product($element['#item']['product']);
  
  return $product->title;
}

/**
 * Implementation of hook_widget_info().
 */
function uc_product_content_widget_info() {
  return array(
    'uc_product_content_radios' => array(
      'label' => t('Radio Buttons/Checkboxes'),
      'field types' => array('uc_product_content'),
      'multiple values' => CONTENT_HANDLE_CORE,
      'callbacks' => array(
        'default value' => CONTENT_CALLBACK_DEFAULT,
      ),
    ),
  );
}

/**
 * FAPI theme for an individual product elements.
 *
 * $element['#field_name'] contains the field name
 * $element['#delta]  is the position of this element in the group
 */
function theme_uc_product_content_radios($element) {
  return $element['#children'];
}

//===== END THEME FUNCTIONS ============================================================================================


//===== FIELD FUNCTIONS ================================================================================================

/**
 * Implementation of hook_field_info().
 */
function uc_product_content_field_info() {
  return array(
    'uc_product_content' => array(
      'label' => t('Product'),
      'description' => t('Create product references that can be purchased at node creation.'),
    ),
  );
}


/**
 * Implementation of hook_field_settings().
 */
function uc_product_content_field_settings($op, $field) {
  switch ($op) {
    case 'database columns':
      $columns = array(
        'product' => array('type' => 'text', 'not null' => FALSE),
      );
      return $columns;
    case 'save':
      return array('product');
  }
}


/**
 * Implementation of hook_field().
 */
function uc_product_content_field($op, &$node, $field, &$items, $teaser, $page) {
  //dsm(array('field_' . $op, $node));
  
  switch ($op) {
    case 'load':
      // FUTURE: possibly load the product into the node?
      
      break;
      
    case 'presave':  

      break;
    
    case 'update':
      // update if not new revision
    case 'insert':
      //dsm($node);
    
      // make sure we're not repeating adding to the cart if the order is complete for this node already
      if (! isset($node->order_id)) {
        // fetch the cart contents and the product choices for this field
        $cart = uc_cart_get_contents();
        $choices = $field['widget']['product_options'];

        foreach ($items as $element) {
          // check if there's one of the item in cart and remove items not chosen for this field, if present
          $in_cart = FALSE;
          $nid = $element['product'];
          foreach ($cart as $item) {
            if($item->nid == $nid && $item->data['product_content_source_nid'] == $node->nid) {
              // there's aready one of these in the cart!
              $in_cart = TRUE;
              
            }
            else if (in_array($item->nid, $choices) || $item->data['product_content_source_nid'] != $node->nid) {
              // we're looking at an item that's a choice, but not chosen, and is in the cart. remove it.
              uc_cart_remove_item($item->nid, $item->cart_id, $item->data);
            }
          }

          // make sure we don't add the non-existant 'none' option to the cart
          if($nid != 'none' && $nid != 0) {
            // add product to cart, if it's not already in there.
            if (! $in_cart) {
              uc_cart_add_item($nid, 1, array('product_content_source_nid' => $node->nid), NULL, FALSE);
            }
          }
          // flag node as having one or more products
          $node->has_product_content = TRUE;
        }
      }
      
      break;
    
    case 'sanitize':
      
      break;
  }
  
  //dsm(array('$op' => $op, '$node' => $node, '$field' => $field, '$items' => $items));
}

/**
 * Implementation of hook_content_is_empty().
 */
function uc_product_content_content_is_empty($item, $field) {
  if (empty($item['product']) && (string)$item['product'] !== '0') {
    return TRUE;
  }
  return FALSE;
}

/**
 * Implementation of hook_default_value()
 */
function uc_product_content_default_value(&$form, &$form_state, $field, $delta) {
  return '';
}

//===== END FIELD FUNCTIONS ============================================================================================


//===== WIDGET FUNCTIONS ===============================================================================================

/**
 * Implementation of FAPI hook_elements().
 *
 * Any FAPI callbacks needed for individual widgets can be declared here,
 * and the element will be passed to those callbacks for processing.
 *
 * Drupal will automatically theme the element using a theme with
 * the same name as the hook_elements key.
 *
 * Autocomplete_path is not used by text_widget but other widgets can use it
 * (see nodereference and userreference).
 */
function uc_product_content_elements() {
  return array(
    'uc_product_content_radios' => array(
      '#input' => TRUE,
      '#columns' => array('product'), '#delta' => 0,
      '#process' => array('uc_product_content_radios_process'),
    ),
  );
}

/**
 * Process an individual element.
 *
 * Build the form element. When creating a form using FAPI #process,
 * note that $element['#value'] is already set.
 *
 * The $fields array is in $form['#field_info'][$element['#field_name']].
 */
function uc_product_content_radios_process($element, $edit, $form_state, $form) {
  //dsm(array('$element' => $element, '$edit' => $edit, '$form_state' => $form_state, '$form' => $form));

  $field = $form['#field_info'][$element['#field_name']];
  $field_key = $element['#columns'][0];
  $delta = $element['#delta'];

  $options = array();
  if (isset($field['widget']['product_options'])) {
   // do some cleanup on the strange return values from the checkboxes
   $field['widget']['product_options'] = array_keys(array_filter($field['widget']['product_options']));

   if ( ($pos = array_search('none', $field['widget']['product_options'])) !== FALSE) {
     $options['none'] = t('None');
     unset($field['widget']['product_options'][$pos]);
   }
   $nids = implode(', ', $field['widget']['product_options']);
   $data = db_query("SELECT nid, title FROM {node} WHERE nid IN (%s) ORDER BY title ASC", $nids);
   while ($obj = db_fetch_object($data)) {
     $options[$obj->nid] = $obj->title;
   }
  }

  $element[$field_key] = array(
   '#type' => 'radios',
   '#options' => $options,
   '#default_value' => isset($element['#value'][$field_key]) ? $element['#value'][$field_key] : NULL,
   '#autocomplete_path' => $element['#autocomplete_path'],
   // The following values were set by the content module and need
   // to be passed down to the nested element.
   '#title' => $element['#title'],
   '#description' => $element['#description'],
   '#required' => $element['#required'],
   '#field_name' => $element['#field_name'],
   '#type_name' => $element['#type_name'],
   '#delta' => $element['#delta'],
   '#columns' => $element['#columns'],
  );

  // Used so that hook_field('validate') knows where to flag an error.
  $element['_error_element'] = array(
   '#type' => 'value',
   '#value' => implode('][', array_merge($element['#parents'], array($field_key))),
  );

  //dsm($element);

  return $element;
}

/**
 * Implementation of hook_widget_settings().
 */
function uc_product_content_widget_settings($op, $widget) {
  // do some cleanup on the strange return values form the checkboxes
  if(isset($widget['product_options'])) {
    $widget['product_options'] = array_keys(array_filter($widget['product_options']));
  }
  
  switch ($op) {
    case 'form':
      $products = array();
  
      $data = db_query("SELECT n.nid, n.title FROM {uc_products} AS p INNER JOIN {node} AS n ON p.nid = n.nid ORDER BY n.title ASC");
      while ($obj = db_fetch_object($data)) {
        $products[$obj->nid] = $obj->title;
      }
      $products['none'] = 'None';

      $form['product_options'] = array(
        '#type' => 'checkboxes',
        '#title' => t('Product Options'),
        '#default_value' => isset($widget['product_options']) ? $widget['product_options'] : array(),
        '#options' => $products,
        '#required' => TRUE,
        '#description' => t('The group of products that may be selected.'),
      );
      // $form['array_filter'] = array('#type' => 'hidden'); // Doesn't seem to work
      
      return $form;
      break;
      
    case 'validate':
      
      break;
      
    case 'save':
      return array('product_options');
  }
}

/**
 * Implementation of hook_widget().
 *
 * Attach a single form element to the form. It will be built out and
 * validated in the callback(s) listed in hook_elements. We build it
 * out in the callbacks rather than here in hook_widget so it can be
 * plugged into any module that can provide it with valid
 * $field information.
 *
 * Content module will set the weight, field name and delta values
 * for each form element. This is a change from earlier CCK versions
 * where the widget managed its own multiple values.
 *
 * If there are multiple values for this field, the content module will
 * call this function as many times as needed.
 *
 * @param $form
 *   the entire form array, $form['#node'] holds node information
 * @param $form_state
 *   the form_state, $form_state['values'][$field['field_name']]
 *   holds the field's form values.
 * @param $field
 *   the field array
 * @param $items
 *   array of default values for this field
 * @param $delta
 *   the order of this item in the array of subelements (0, 1, 2, etc)
 *
 * @return
 *   the form item for a single element for this field
 */
function uc_product_content_widget(&$form, &$form_state, $field, $items, $delta = 0) {  
  $element = array(
    '#type' => $field['widget']['type'],
    '#default_value' => isset($items[$delta]) ? $items[$delta] : '',
  );
  return $element;
}

//===== END WIDGET FUNCTIONS ===========================================================================================
