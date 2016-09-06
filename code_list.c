#include "code_list.h"

#include <stdlib.h>
#include <stdio.h>

struct CodeHolder {
	char *code;
	struct CodeHolder *next;
	struct CodeHolder *last;
};

struct CodeList {
	struct CodeHolder *first;
	int length;
};

struct CodeList *createCodeList() {
	struct CodeList *list = malloc(sizeof(struct CodeList));
	list->first = NULL;
	list->length = 0;
	return list;
}

void destroyCodeList(struct CodeList *list) {
	struct CodeHolder *lastNode = list->first;
	
	while (lastNode) {
		struct CodeHolder *nextNode = lastNode;
		free(lastNode);
		lastNode = nextNode;
	}

	free(list);
}

struct CodeHolder *getNextItem(struct CodeHolder *node) {
	return node->next;
}

struct CodeHolder *getFirstItem(struct CodeList *list) {
	return list->first;
}

void addListItem(struct CodeList *list, char code[4]) {
	struct CodeHolder *oldFirst = list->first;
	struct CodeHolder *newFirst = malloc(sizeof(struct CodeHolder));
	if (oldFirst) {
		oldFirst->last = newFirst;
	}
	newFirst->code = code;
	newFirst->next = oldFirst;
	newFirst->last = NULL;
	list->first = newFirst;
	++list->length;
}

void removeListItem(struct CodeList *list, struct CodeHolder *node) {
	if (list->first == node) {
		list->first = node->next;
	}
	if (node->next != NULL) {
		node->next->last = node->last;
	}
	if (node->last != NULL) {
		node->last->next = node->next;
	}
	--list->length;
	free(node);
}

int getListLength(struct CodeList *list) {
	return list->length;
}

// we do this so that the client can get nodes, and use them for order 1 removal, but can still
//   extract the code from them without having access to the underlying structure
char *getCodeFromListItem(struct CodeHolder *node) {
	return node->code;
}




	

