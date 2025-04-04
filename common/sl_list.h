typedef int (*sl_list_comp_func)(void *, void *);

enum
{
	MAX_SL_LIST_NAME_LENGTH= 31
};

struct sl_list_element
{
	void *data;
	void *key;
};

struct sl_list *sl_list_new(char *name, sl_list_comp_func comp_func);
void sl_list_dispose(struct sl_list *list);

struct sl_list_element *sl_list_new_element(struct sl_list *list, void *data, void *key);
void sl_list_dispose_element(struct sl_list *list, struct sl_list_element *element);

struct sl_list_element *sl_list_get_head_element(struct sl_list *list);
struct sl_list_element *sl_list_search_for_element(struct sl_list *list, void *key);
struct sl_list_element *sl_list_get_next_element(struct sl_list *list, struct sl_list_element *element);

void sl_list_insert_element(struct sl_list *list, struct sl_list_element *element);
void sl_list_remove_element(struct sl_list *list, struct sl_list_element *element);
