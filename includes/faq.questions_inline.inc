<?php

/**
 * @file
 * FAQ page callbacks for the "questions inline" layouts.
 */

/**
 * Create the FAQ page if set to show the questions inline.
 *
 * @param array &$variables
 *   Array reference of arguments given to the theme() function.
 */
function template_preprocess_faq_questions_inline(&$variables) {
  $config = config('faq.settings');
  $data = $variables['data'];

  // Fetch configuration.
  $disable_node_links = $config->get('faq_disable_node_links');
  $teaser = $config->get('faq_use_teaser');
  $links = $config->get('faq_show_node_links');

  // Configure "back to top" link.
  $this_page = $_GET['q'];
  $back_to_top = faq_init_back_to_top($this_page);

  // Configure labels.
  $variables['question_label'] = '';
  $variables['answer_label'] = '';
  if ($config->get('faq_qa_mark')) {
    $variables['question_label'] = check_plain($config->get('faq_question_label'));
    $variables['answer_label'] = check_plain($config->get('faq_answer_label'));
  }

  $nodes = array();
  $count = 0;
  foreach ($data as $node) {
    $anchor = 'n' . $node->nid;
    faq_view_question($nodes[$count], $node, NULL, $anchor);
    faq_view_answer($nodes[$count], $node, $back_to_top, $teaser, $links);
    $count++;
  }
  $variables['use_teaser'] = $teaser;
  $variables['nodes'] = $nodes;
}

/**
 * Create categorized FAQ page if set to show/hide the questions inline.
 *
 * @param array &$variables
 *   Array reference of arguments given to the theme() function.
 */
function template_preprocess_faq_category_questions_inline(&$variables) {
  $config = config('faq.settings');
  $data = $variables['data'];
  $category_display = $variables['category_display'];
  $term = $variables['term'];
  $parent_term = $variables['parent_term'];
  $class = $variables['class'];

  // Fetch configuration.
  $teaser = $config->get('faq_use_teaser');
  $links = $config->get('faq_show_node_links');
  $disable_node_links = $config->get('faq_disable_node_links');
  $display_faq_count = $config->get('faq_count');
  $hide_child_terms = $config->get('faq_hide_child_terms');
  $show_term_page_children = $config->get('faq_show_term_page_children');
  $default_sorting = $config->get('faq_default_sorting');

  // Initialise some variables.
  $variables['use_teaser'] = $teaser;
  $default_weight = 0;
  if ($default_sorting != 'DESC') {
    $default_weight = 1000000;
  }
  $this_page = $_GET['q'];
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

  // Configure "back to top" link.
  $back_to_top = faq_init_back_to_top($this_page);

  // Configure labels.
  $variables['question_label'] = '';
  $variables['answer_label'] = '';
  if ($config->get('faq_qa_mark')) {
    $variables['question_label'] = check_plain($config->get('faq_question_label'));
    $variables['answer_label'] = check_plain($config->get('faq_answer_label'));
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
    $variables['subcat_body_list'] = faq_get_child_categories_faqs($term, 'faq_category_questions_inline', $default_weight, $default_sorting, $category_display, $variables['class'], $parent_term);
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
    $node_var = array();
    $anchor = 't' . $term->tid . 'n' . $node->nid;
    faq_view_question($node_var, $node, NULL, $anchor);
    faq_view_answer($node_var, $node, $back_to_top, $teaser, $links);
    $nodes[] = $node_var;
  }
  $variables['nodes'] = $nodes;
  $variables['question_count'] = $count;
}
