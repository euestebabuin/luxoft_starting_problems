

typedef struct sl_list_str{
	struct sl_list_str *next;
	int val;
	void (*callback_print)(int);
} sl_list;

int add_node(sl_list **L, int val);

int delete_node(sl_list **L, int val);

void print_list(sl_list *L);

int sort_list(sl_list *L);

int flush_list(sl_list **L);

void start_list_work();

void stop_list_work();
