#include "cseries.h"
#include "rb_tree.h"

 /*
  * The metaserver code changes that fall outside the original Bungie.net metaserver code 
  * license were written and are copyright 2002, 2003 of the following individuals:
  *
  * Copyright (c) 2002, 2003 Alan Wagner
  * Copyright (c) 2002 Vishvananda Ishaya
  * Copyright (c) 2003 Bill Keirstead
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted provided that the following conditions
  * are met:
  *
  * 1. Redistributions of source code must retain the above copyright
  *    notice, this list of conditions and the following disclaimer.
  *
  * 2. Redistributions in binary form must reproduce the above copyright
  *    notice, this list of conditions and the following disclaimer in the
  *    documentation and/or other materials provided with the distribution.
  *
  * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
  * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
  * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  * SUCH DAMAGE.
  *
  */

// ALAN Begin: added headers
#include <string.h>
// ALAN End

enum
{
	_black,
	_red
};

static void set_tree_root_element(
	struct rb_tree * tree,
	struct element * tree_element);

static void rb_tree_insert(
	struct rb_tree * tree,
	struct element * tree_element);

static void rb_remove_element(
	struct rb_tree * tree,
	struct element * tree_element,
	struct element * nil_element);

static void rotate_left(
	struct rb_tree * tree,
	struct element * tree_element);

static void rotate_right(
	struct rb_tree * tree,
	struct element * tree_element);

#ifdef DEBUG
static void verify_rb_tree(
	struct rb_tree * tree);

static void verify_tree_element(
	struct rb_tree * tree, 
	struct element * tree_element);

static void dump_tree(
	struct rb_tree * tree);
#else
#define	verify_rb_tree(x)
#define	verify_tree_element(x, y)
#endif

struct element * search_for_element(
	struct rb_tree * tree, 
	void * key)
{
	struct element * tree_element;

	assert(key);

	verify_rb_tree(tree);

	tree_element = tree->root_element;
	while (tree_element != NULL)
	{
		verify_tree_element(tree, tree_element);
		if (!tree->comp_func(key, tree_element->key))
		{
			break;
		}

		if (tree->comp_func(key, tree_element->key) < 0)
		{
			tree_element = tree_element->left_element;
		}
		else
		{
			tree_element = tree_element->right_element;
		}
	}

	return tree_element;
}

struct element * find_minimum_element(
	struct rb_tree * tree, 
	struct element * starting_element)
{
	struct element * tree_element;

	verify_rb_tree(tree);
	assert(starting_element);

	tree_element = starting_element;
	while (tree_element->left_element != NULL)
	{
		verify_tree_element(tree, tree_element);
		tree_element = tree_element->left_element;
	}

	return tree_element;
}

struct element * find_maximum_element(
	struct rb_tree * tree, 
	struct element * starting_element)
{
	struct element * tree_element;

	verify_rb_tree(tree);
	assert(starting_element);

	tree_element = tree->root_element;
	while (tree_element->right_element != NULL)
	{
		verify_tree_element(tree, tree_element);
		tree_element = tree_element->right_element;
	}

	return tree_element;
}

struct element * find_previous_element(
	struct rb_tree * tree, 
	struct element * starting_element)
{
	struct element * tree_element;
	struct element * parent_tree_element;

	verify_rb_tree(tree);
	assert(starting_element);

	tree_element = starting_element;
	verify_tree_element(tree, tree_element);
	if (tree_element->left_element != NULL)
	{
		tree_element = find_maximum_element(tree, tree_element->left_element);
	}
	else
	{
		parent_tree_element = tree_element->parent_element;
		while ((parent_tree_element) && (tree_element == parent_tree_element->left_element))
		{
			verify_tree_element(tree, parent_tree_element);
			tree_element = parent_tree_element;
			parent_tree_element = tree_element->parent_element;
		}

		tree_element = parent_tree_element;
		verify_tree_element(tree, tree_element);
	}

	return tree_element;
}

struct element * find_next_element(
	struct rb_tree * tree, 
	struct element * starting_element)
{
	struct element * tree_element;
	struct element * parent_tree_element;

	assert(tree);
	assert(starting_element);

	tree_element = starting_element;
	verify_tree_element(tree, tree_element);
	if (tree_element->right_element != NULL)
	{
		tree_element = find_minimum_element(tree, tree_element->right_element);
	}
	else
	{
		parent_tree_element = tree_element->parent_element;
		while ((parent_tree_element) && (tree_element == parent_tree_element->right_element))
		{
			verify_tree_element(tree, parent_tree_element);
			tree_element = parent_tree_element;
			parent_tree_element = tree_element->parent_element;
		}

		tree_element = parent_tree_element;
		verify_tree_element(tree, tree_element);
	}

	return tree_element;
}

void insert_element(
	struct rb_tree * tree, 
	struct element * tree_element)
{
	struct element * insertion_point;
	struct element * parent_element;

	verify_rb_tree(tree);

	assert(!tree_element->right_element);
	assert(!tree_element->left_element);
	assert(!tree_element->parent_element);

	parent_element = NULL;
	insertion_point = tree->root_element;
	while (insertion_point != NULL)
	{
		parent_element = insertion_point;
		if (tree->comp_func(tree_element->key, insertion_point->key) < 0)
			insertion_point = insertion_point->left_element;
		else
			insertion_point = insertion_point->right_element;
	}

	tree_element->parent_element = parent_element;
	if (!parent_element)
	{
		set_tree_root_element(tree, tree_element);
	}
	else if (tree->comp_func(tree_element->key, parent_element->key) < 0)
	{
		verify_tree_element(tree, parent_element);
		parent_element->left_element = tree_element;
		if (tree_element->parent_element != tree->root_element)
		{
			rb_tree_insert(tree, tree_element);
		}
		else
		{
			tree_element->color = _red;
		}
	}
	else
	{
		verify_tree_element(tree, tree_element);
		parent_element->right_element = tree_element;
		if (tree_element->parent_element != tree->root_element)
		{
			rb_tree_insert(tree, tree_element);
		}
		else
		{
			tree_element->color = _red;
		}
	}

	return;
}

static struct element temp_element;
void remove_element(
	struct rb_tree * tree, 
	struct element * tree_element)
{
	struct element * splice_out_element;
	struct element * splice_in_element;

	verify_rb_tree(tree);
	verify_tree_element(tree, tree_element);

	if (!(tree_element->left_element) || !(tree_element->right_element))
	{
		splice_out_element = tree_element;
	}
	else
	{
		splice_out_element = find_next_element(tree, tree_element);
		verify_tree_element(tree, splice_out_element);
	}

	if (splice_out_element->left_element)
	{
		splice_in_element = splice_out_element->left_element;
	}
	else
	{
		splice_in_element = splice_out_element->right_element;
	}

	if (splice_in_element)
	{
		verify_tree_element(tree, splice_in_element);
		splice_in_element->parent_element = splice_out_element->parent_element;
	}
	else
	{
		memset(&temp_element, 0, sizeof(temp_element));
		splice_in_element = &temp_element;
		splice_in_element->parent_element = splice_out_element->parent_element;
	}

	if (!splice_out_element->parent_element)
	{
		set_tree_root_element(tree, splice_in_element);
	}
	else
	{
		if (splice_out_element == splice_out_element->parent_element->left_element)
		{
			splice_out_element->parent_element->left_element = splice_in_element;
		}
		else
		{
			splice_out_element->parent_element->right_element = splice_in_element;
		}
	}

	if (splice_out_element != tree_element)
	{
		if (tree_element->left_element != splice_out_element)
		{
			splice_out_element->left_element = tree_element->left_element;
		}
		if (tree_element->right_element != splice_out_element)
		{
			splice_out_element->right_element = tree_element->right_element;
		}
		if (splice_out_element->left_element)
		{
			splice_out_element->left_element->parent_element = splice_out_element;
		}
		if (splice_out_element->right_element)
		{
			splice_out_element->right_element->parent_element = splice_out_element;
		}

		splice_out_element->parent_element = tree_element->parent_element;

		if (!splice_out_element->parent_element)
		{
			set_tree_root_element(tree, splice_out_element);
		}
		else
		{
			if (tree_element->parent_element->left_element == tree_element)
			{
				splice_out_element->parent_element->left_element = splice_out_element;
			}
			else
			{
				splice_out_element->parent_element->right_element = splice_out_element;
			}
		}
	}

	if (splice_out_element->color == _black)
	{
		verify_tree_element(tree, splice_in_element);
		rb_remove_element(tree, splice_in_element, &temp_element);
	}
	else if (splice_in_element == &temp_element)
	{
		if (splice_in_element == splice_in_element->parent_element->left_element)
		{
			splice_in_element->parent_element->left_element = NULL;
		}
		else
		{
			splice_in_element->parent_element->right_element = NULL;
		}
		splice_in_element->parent_element = NULL;
	}

	return;
}

static void set_tree_root_element(
	struct rb_tree * tree,
	struct element * tree_element)
{
	tree->root_element = tree_element;
	tree_element->color = _black;
}

static void rb_tree_insert(
	struct rb_tree * tree,
	struct element * tree_element)
{
	struct element * element_pointer;
	struct element * uncle_pointer;

	assert(tree);
	assert(tree_element);

	tree_element->color = _red;
	
	element_pointer = tree_element;
	while ((element_pointer != tree->root_element) &&
		element_pointer->color == _red)
	{
		if (!element_pointer->parent_element->parent_element)
		{
			break;
		}

		if (element_pointer->parent_element == 
			element_pointer->parent_element->parent_element->left_element)
		{
			uncle_pointer = element_pointer->parent_element->parent_element->right_element;
			if (uncle_pointer && uncle_pointer->color == _red)
			{
				element_pointer->parent_element->color = _black;
				uncle_pointer->color = _black;
				element_pointer->parent_element->parent_element->color = _red;
				element_pointer = element_pointer->parent_element->parent_element;
			}
			else 
			{
				if (element_pointer == element_pointer->parent_element->right_element)
				{
					element_pointer = element_pointer->parent_element;
					rotate_left(tree, element_pointer);
				}

				element_pointer->parent_element->color = _black;
				element_pointer->parent_element->parent_element->color = _red;
				rotate_right(tree, element_pointer->parent_element->parent_element);
				break;
			}
		}
		else
		{
			uncle_pointer = element_pointer->parent_element->parent_element->left_element;
			if (uncle_pointer && uncle_pointer->color == _red)
			{
				element_pointer->parent_element->color = _black;
				uncle_pointer->color = _black;
				element_pointer->parent_element->parent_element->color = _red;
				element_pointer = element_pointer->parent_element->parent_element;
			}
			else 
			{
				if (element_pointer == element_pointer->parent_element->left_element)
				{
					element_pointer = element_pointer->parent_element;
					rotate_right(tree, element_pointer);
				}

				element_pointer->parent_element->color = _black;
				element_pointer->parent_element->parent_element->color = _red;
				rotate_left(tree, element_pointer->parent_element->parent_element);
				break;
			}
		}
		tree->root_element->color = _black;
	}

	tree->root_element->color = _black;
}

static void rb_remove_element(
	struct rb_tree * tree,
	struct element * tree_element,
	struct element * nil_element)
{
	struct element * element_pointer;
	struct element * fix_up_element;
	struct element * parent_element;

	assert(tree);
	assert(tree_element);

	element_pointer = tree_element;
	while ((element_pointer != tree->root_element) &&
		(element_pointer->color == _black))
	{
		parent_element = element_pointer->parent_element;
		if (parent_element->left_element == element_pointer)
		{
			fix_up_element = parent_element->right_element;

			if (element_pointer == nil_element)
			{
				element_pointer->parent_element->left_element = NULL;
				element_pointer->parent_element = NULL;
			}

			if (!fix_up_element)
			{
				break;
			}

			if (fix_up_element->color == _red)
			{
				fix_up_element->color = _black;
				parent_element->color = _red;
				rotate_left(tree, parent_element);
				fix_up_element = parent_element->right_element;
			}

			if ((!fix_up_element->left_element || (fix_up_element->left_element->color == _black)) && 
				(!fix_up_element->right_element || (fix_up_element->right_element->color == _black)))
			{
				fix_up_element->color = _red;
				element_pointer = parent_element;
			}
			else 
			{
				if (!fix_up_element->right_element || (fix_up_element->right_element->color == _black))
				{
					if (fix_up_element->left_element)
					{
						fix_up_element->left_element->color = _black;
					}
					fix_up_element->color = _red;
					rotate_right(tree, fix_up_element);
					assert(element_pointer != nil_element);
					fix_up_element = element_pointer->right_element;
				}

				fix_up_element->color = parent_element->color;
				parent_element->color = _black;
				if (fix_up_element->right_element)
				{
					fix_up_element->right_element->color = _black;
				}
				rotate_left(tree, parent_element);
				element_pointer = tree->root_element;
			}
		}
		else
		{
			fix_up_element = parent_element->left_element;

			if (element_pointer == nil_element)
			{
				element_pointer->parent_element->right_element = NULL;
				element_pointer->parent_element = NULL;
			}

			if (!fix_up_element)
			{
				break;
			}

			if (fix_up_element && fix_up_element->color == _red)
			{
				fix_up_element->color = _black;
				parent_element->color = _red;
				rotate_right(tree, parent_element);
				fix_up_element = parent_element->left_element;
			}

			if (((!fix_up_element->right_element) || (fix_up_element->right_element->color == _black)) && 
				((!fix_up_element->left_element) || (fix_up_element->left_element->color == _black)))
			{
				fix_up_element->color = _red;
				element_pointer = parent_element;
			}
			else 
			{
				if (!fix_up_element->left_element || fix_up_element->left_element->color == _black)
				{
					if (fix_up_element->right_element)
					{
						fix_up_element->right_element->color = _black;
					}
					fix_up_element->color = _red;
					rotate_left(tree, fix_up_element);
					fix_up_element = element_pointer->left_element;
				}

				fix_up_element->color = parent_element->color;
				parent_element->color = _black;
				if (fix_up_element->left_element)
				{
					fix_up_element->left_element->color = _black;
				}
				rotate_right(tree, parent_element);
				element_pointer = tree->root_element;
			}
		}
	}

	element_pointer->color = _black;
}

static void rotate_left(
	struct rb_tree * tree,
	struct element * tree_element)
{
	struct element * rotate_element;

	assert(tree);
	assert(tree_element);

	rotate_element = tree_element->right_element;

	tree_element->right_element = rotate_element->left_element;
	if (rotate_element->left_element)
	{
		rotate_element->left_element->parent_element = tree_element;
	}

	rotate_element->parent_element = tree_element->parent_element;
	if (!tree_element->parent_element)
	{
		set_tree_root_element(tree, rotate_element);
	}
	else if (tree_element == tree_element->parent_element->left_element)
	{
		tree_element->parent_element->left_element = rotate_element;
	}
	else
	{
		tree_element->parent_element->right_element = rotate_element;
	}

	rotate_element->left_element = tree_element;
	tree_element->parent_element = rotate_element;
}

static void rotate_right(
	struct rb_tree * tree,
	struct element * tree_element)
{
	struct element * rotate_element;

	assert(tree);
	assert(tree_element);
	assert(tree_element->right_element != &temp_element);
	assert(tree_element->left_element != &temp_element);
	assert(tree_element->parent_element != &temp_element);

	rotate_element = tree_element->left_element;

	tree_element->left_element = rotate_element->right_element;
	if (rotate_element->right_element)
	{
		rotate_element->right_element->parent_element = tree_element;
	}

	rotate_element->parent_element = tree_element->parent_element;
	if (!tree_element->parent_element)
	{
		set_tree_root_element(tree, rotate_element);
	}
	else if (tree_element == tree_element->parent_element->left_element)
	{
		tree_element->parent_element->left_element = rotate_element;
	}
	else
	{
		tree_element->parent_element->right_element = rotate_element;
	}

	rotate_element->right_element = tree_element;
	tree_element->parent_element = rotate_element;
}

#ifdef DEBUG
static void verify_rb_tree(
	struct rb_tree * tree)
{
	if (!tree->name || !tree->name[0] || !tree->comp_func) 
	{
		dump_tree(tree);
		halt();
	}
}

static void verify_tree_element(
	struct rb_tree * tree,
	struct element * tree_element)
{
	if ((!tree_element->parent_element && tree->root_element != tree_element) ||
		!tree_element->data || !tree_element->key)
	{
		dump_tree(tree);
		halt();
	}
}

static void dump_tree(
	struct rb_tree * tree)
{
	FILE * fp = fopen("rb_tree_dump", "a+");

	if (fp)
	{
		struct element * tree_element;
		unsigned long n= 0;

		for (tree_element= tree->root_element; tree_element; tree_element= find_next_element(tree, tree_element))
		{
			fprintf(fp, "tree element %ul: parent_element %p: left_element %p: right_element %p: data %p: key %p\n",
				tree_element->parent_element, tree_element->left_element, tree_element->right_element, tree_element->data,
				tree_element->key);
		}
		fclose(fp);
	}
}

#endif
