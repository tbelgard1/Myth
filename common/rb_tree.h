#ifndef __RB_TREE_H
#define __RB_TREE_H

#define	MAX_TREE_NAME_LENGTH		64

struct element
{
	long				color;
	struct element *		parent_element;
	struct element *		left_element;
	struct element *		right_element;
	void *				data;
	void *				key;
};

struct rb_tree
{
	char 				name[MAX_TREE_NAME_LENGTH];
	struct element *		root_element;
	int				(*comp_func)(void *, void *);
};

struct element * search_for_element(
	struct rb_tree * tree, 
	void * key);

struct element * find_previous_element(
	struct rb_tree * tree, 
	struct element * starting_element);

struct element * find_next_element(
	struct rb_tree * tree, 
	struct element * starting_element);

struct element * find_minimum_element(
	struct rb_tree * tree, 
	struct element * starting_element);

struct element * find_maximum_element(
	struct rb_tree * tree, 
	struct element * starting_element);

void insert_element(
	struct rb_tree * tree, 
	struct element * tree_element);

void remove_element(
	struct rb_tree * tree, 
	struct element * tree_element);

#endif
