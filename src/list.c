#include "cnake.h"

t_list	*list_new(void *content)
{
	t_list	*new = malloc(sizeof(t_list));
	if (!new) {
		trace_log(INFO, "Malloc Error");
		return (NULL);
	}
	new->content = content;
	new->next = NULL;
	return (new);
}

void	list_add_front(t_list **list, t_list *new_element)
{
	if (*list)
		new_element->next = *list;
	*list = new_element;
}

t_list	*list_last(t_list *list)
{
	t_list	*curr = list;
	for (; curr->next; curr = curr->next)
		;
	return (curr);
}

void	list_add_back(t_list **list, t_list *new_element)
{
	if (*list) {
		t_list	*last = list_last(*list);
		last->next = new_element;
	} else *list = new_element;
}

void	list_free(t_list *list)
{
	t_list *tmp;
	while (list) {
		tmp = list;
		list = list->next;
		free(tmp);
	}
}

void	list_del_front(t_list **list)
{
	t_list	*tmp = *list;
	*list = (*list)->next;
	free(tmp);
}

size_t	list_size(t_list *list)
{
	size_t	size = 0;
	while (list) {
		++size;
		list = list->next;
	}
	return (size);
}

void	list_print(t_list *list)
{
	for (; list; list = list->next)
		printf("%d\n", *(int *)list->content);
}
