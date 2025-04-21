
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

#include "cseries.h"
#include "sl_list.h"

// ALAN Begin: added headers
#include <string.h>
// ALAN End

enum
{
	// ALAN Begin: fixed multi-character character constant warnings
//	_sl_list_signature= 'list',
//	_sl_list_element_signature= 'lsel'

	_sl_list_signature= 0x6c697374,
	_sl_list_element_signature= 0x6c73656c
	// ALAN End
};

struct sl_list_internal
{
	char name[MAX_SL_LIST_NAME_LENGTH+1];
	struct sl_list_element_internal *head_element;
	sl_list_comp_func comp_func;
#ifdef DEBUG
	long signature;
#endif
};

struct sl_list_element_internal
{
	void *data;
	void *key;

	struct sl_list_element_internal *next;
#ifdef DEBUG
	long signature;
#endif
};

#ifdef DEBUG
static void verify_list(struct sl_list_internal *i_list);
static void verify_list_element(struct sl_list_internal *i_list, struct sl_list_element_internal *i_element);
static void dump_list(struct sl_list_internal *i_list);
#else
#define verify_list(x)
#define verify_list_element(x, y)
#endif

struct sl_list *sl_list_new(
	char *name,
	sl_list_comp_func comp_func)
{
	struct sl_list_internal *i_list;

	assert(name);
	assert(comp_func);

	i_list= malloc(sizeof(struct sl_list_internal));
	if (i_list)
	{
		memset(i_list, 0, sizeof(struct sl_list_internal));
		strncpy(i_list->name, name, MAX_SL_LIST_NAME_LENGTH);
		i_list->name[MAX_SL_LIST_NAME_LENGTH]= '\0';
		i_list->comp_func= comp_func;
	#ifdef DEBUG
		i_list->signature= _sl_list_signature;
	#endif
		verify_list(i_list);
	}

	return (struct sl_list *)i_list;
}

void sl_list_dispose(
	struct sl_list *list)
{
	struct sl_list_internal *i_list= (struct sl_list_internal *)list;

	if (i_list) 
	{
		verify_list(i_list);
		free(i_list);
	}
}

struct sl_list_element *sl_list_new_element(
	struct sl_list *list,
	void *data, 
	void *key)
{
	struct sl_list_element_internal *i_element;

	i_element= malloc(sizeof(struct sl_list_element_internal));
	if (i_element)
	{
		memset(i_element, 0, sizeof(struct sl_list_element_internal));
		i_element->data= data;
		i_element->key= key;
	#ifdef DEBUG
		i_element->signature= _sl_list_element_signature;
	#endif
		verify_list_element((struct sl_list_internal *)list, i_element);
	}

	return (struct sl_list_element *)i_element;
}

void sl_list_dispose_element(
	struct sl_list *list,
	struct sl_list_element *element)
{
	verify_list_element((struct sl_list_internal *)list, (struct sl_list_element_internal *)element);

	free((struct sl_list_element_internal *)element);
}

struct sl_list_element *sl_list_get_head_element(
	struct sl_list *list)
{
	struct sl_list_internal *i_list= (struct sl_list_internal *)list;

	verify_list(i_list);

	return (struct sl_list_element *)i_list->head_element;
}

struct sl_list_element *sl_list_search_for_element(
	struct sl_list *list,
	void *key)
{
	struct sl_list_internal *i_list= (struct sl_list_internal *)list;
	struct sl_list_element_internal *i_element;

	verify_list(i_list);

	for (i_element= i_list->head_element; i_element; i_element= i_element->next)
	{
		verify_list_element(i_list, i_element);
		if (!i_list->comp_func(i_element->key, key)) break;
	}

	return (struct sl_list_element *)i_element;	
}

struct sl_list_element *sl_list_get_next_element(
	struct sl_list *list, 
	struct sl_list_element *element)
{
	struct sl_list_element *next;

	verify_list_element((struct sl_list_internal *)list, (struct sl_list_element_internal *)element);

	next= (struct sl_list_element *)((struct sl_list_element_internal *)(element))->next;

	return next;
}

void sl_list_insert_element(
	struct sl_list *list,
	struct sl_list_element *element)
{
	struct sl_list_internal *i_list= (struct sl_list_internal *)list;
	struct sl_list_element_internal *i_element;
	struct sl_list_element_internal *p_element= NULL;

	verify_list(i_list);
	verify_list_element(i_list, (struct sl_list_element_internal *)element);
	assert(((struct sl_list_element_internal *)(element))->next==NULL);

	for (i_element= i_list->head_element; i_element; i_element= i_element->next) 
	{
		verify_list_element(i_list, i_element);
		p_element= i_element;
	}

	if (p_element)
		p_element->next= (struct sl_list_element_internal *)element;
	else
		i_list->head_element= (struct sl_list_element_internal *)element;
}

void sl_list_remove_element(
	struct sl_list *list,
	struct sl_list_element *element)
{
	struct sl_list_internal *i_list= (struct sl_list_internal *)list;
	struct sl_list_element_internal *i_element;
	struct sl_list_element_internal *p_element= NULL;

	verify_list(i_list);
	verify_list_element(i_list, (struct sl_list_element_internal *)element);

	for (i_element= i_list->head_element; i_element; i_element= i_element->next)
	{
		verify_list_element(i_list, i_element);
		if (i_element==(struct sl_list_element_internal *)element) break;
		p_element= i_element;
	}

	if (p_element)
	{
		p_element->next= ((struct sl_list_element_internal *)(element))->next;
	}
	else
	{
		assert(i_list->head_element==(struct sl_list_element_internal *)element);
		i_list->head_element= ((struct sl_list_element_internal *)(element))->next;
	}
}

#ifdef DEBUG
static void verify_list(
	struct sl_list_internal *i_list)
{
	if (!i_list->name[0] || !i_list->comp_func || i_list->signature!=_sl_list_signature)
	{
		dump_list(i_list);
		halt();
	}
}

static void verify_list_element(
	struct sl_list_internal *i_list,
	struct sl_list_element_internal *i_element)
{
	if (!i_element->data || !i_element->key || i_element->signature!=_sl_list_element_signature || i_element==i_element->next)
	{
		dump_list(i_list);
		halt();
	}
}

static void dump_list(
	struct sl_list_internal *i_list)
{
	struct sl_list_element_internal *i_element;
	FILE *fp= fopen("list_dump", "a+");

	if (fp)
	{
		unsigned long n= 0;

		fprintf(fp, "<------------------------------------------------------------------>\n");
		fprintf(fp, "dump for list %s: head_element %p: comp_func %p: signature %s\n",
			i_list->name, i_list->head_element, i_list->comp_func, 
			(i_list->signature==_sl_list_signature)?"VALID":"INVALID");
		for (i_element= i_list->head_element; i_element; i_element= i_element->next)
		{
			fprintf(fp, "element %ul: data %p: key %p: signature %s\n",
				n++, i_element->data, i_element->key,
				(i_element->signature==_sl_list_element_signature)?"TRUE":"FALSE");

			if (i_element->signature!=_sl_list_element_signature ||i_element==i_element->next) break;
		}

		fclose(fp);
	}
}
#endif
