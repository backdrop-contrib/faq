<?php

/**
 * @file
 * FAQ page callbacks for the "new page" layouts.
 */

/**
 * Create FAQ page if set to show the answer in a new page.
 *
 * @param array &$variables
 *   Array reference of arguments given to the theme() function.
 */
function template_preprocess_faq_new_page(&$variables) {
  $items = array();
  $data = $variables['data'];

  foreach ($data as $node) {
    $items[] = l($node->title, "node/$node->nid");
  }

  $list_style = config_get('faq.settings', 'faq_question_listing');
  $variables['list_style'] = $list_style;
  $variables['list_items'] = $items;

  $variables['list'] = theme('item_list',
    array(
    'items' => $items,
    'title' => NULL,
    'type' => $list_style,
    'attributes' => array("class" => "faq-question-listing"),
  )
  );
}

/**
 * Create categorized FAQ page if set to show answer in a new page.
 *
 * @param array &$variables
 *   Array reference of arguments given to the theme() function.
 */
function template_preprocess_faq_category_new_page(&$variables) {
  $config = config('faq.settings');
  $data = $variables['data'];
  $category_display = $variables['category_display'];
  $term = $variables['term'];
  $parent_term = $variables['parent_term'];
  $class = $variables['class'];
  $this_page = $_GET['q'];

  // Fetch configuration.
  $display_faq_count = $config->get('faq_count');
  $hide_child_terms = $config->get('faq_hide_child_terms');
  $show_term_page_children = $config->get('faq_show_term_page_children');

  // Initialise some variables.
  $get_child_terms = 0;
  // Check if we're on a faq page.
  if (arg(0) == _faq_path()) {
    // Check if we're on a categorized faq page.
    if (is_numeric(arg(1))) {
      $get_child_terms = arg(1);
    }
  }
  // Force some settings in case we're processing a special faq question list
  // created by a custom call to faq_page().
  elseif (!empty($parent_term)) {
    $get_child_terms = $parent_term->tid;
    $show_term_page_children = TRUE;
  }

  $default_sorting = $config->get('faq_default_sorting');
  $default_weight = 0;
  if ($default_sorting != 'DESC') {
    $default_weight = 1000000;
  }

  // Get number of questions, and account for hidden sub-categories.
  $count = 0;
  if ($display_faq_count && $hide_child_terms) {
    $count = faq_taxonomy_term_count_nodes($term->tid);
  }
  $variables['display_faq_count'] = $display_faq_count;

  // Get taxonomy image.
  $variables['term_image'] = '';
  if (module_exists('taxonomy_image') && function_exists('taxonomy_image_display')) {
    $variables['term_image'] = taxonomy_image_display($term->tid, array('class' => 'faq-tax-image'));
  }

  // Configure header.
  $variables['category_depth'] = $term->depth;
  if ($category_display == 'hide_qa') {
    $variables['header_title'] = l(faq_tt("taxonomy:term:$term->tid:name", $term->name), "faq-page/$term->tid");
  }
  else {
    $variables['header_title'] = check_plain(faq_tt("taxonomy:term:$term->tid:name", $term->name));
  }

  // Configure category description.
  $variables['description'] = check_markup(faq_tt("taxonomy:term:$term->tid:description", $term->description));

  // Get list of sub-categories if necessary.
  $child_categories = array();
  if (($show_term_page_children || $hide_child_terms) && $category_display == 'new_page') {
    $child_categories = faq_view_child_category_headers($term);
  }
  $variables['subcat_list'] = $child_categories;
  $variables['subcat_list_style'] = $config->get('faq_category_listing');

  // Configure class (faq-qa or faq-qa-hide).
  if ($get_child_terms == $term->tid) {
    $variables['container_class'] = 'faq-qa';
  }
  else {
    $variables['container_class'] = $class;
  }

  // Configure sub-category bodies (theme recursively).
  $variables['subcat_body_list'] = array();
  if (($get_child_terms && $category_display == 'categories_inline') || ((($show_term_page_children && $this_page != _faq_path()) || $hide_child_terms) && $category_display == 'hide_qa')) {
    $variables['subcat_body_list'] = faq_get_child_categories_faqs($term, 'faq_category_new_page', $default_weight, $default_sorting, $category_display, $variables['class'], $parent_term);
  }

  if (!count($data)) {
    $variables['question_count'] = $count;
    $variables['nodes'] = array();
    return;
  }

  $nodes = array();
  foreach ($data as $node) {
    if (!$hide_child_terms) {
      $count++;
    }
    $nodes[] = l($node->title, "node/$node->nid");
  }
  $variables['question_list'] = $nodes;
  $variables['question_list_style'] = $config->get('faq_question_listing');
  $variables['question_count'] = $count;
}
