struct CodeHolder;

struct CodeList;

struct CodeList *createCodeList();

void destroyCodeList(struct CodeList *list);

struct CodeHolder *getNextItem(struct CodeHolder *node);

struct CodeHolder *getFirstItem(struct CodeList *list);

void addListItem(struct CodeList *list, char code[4]);

void removeListItem(struct CodeList *list, struct CodeHolder *node);

int getListLength(struct CodeList *list);

char *getCodeFromListItem(struct CodeHolder *node);




	

