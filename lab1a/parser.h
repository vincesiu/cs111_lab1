#pragma once

typedef struct cmd_node {
	command_t c;
	struct cmd_node * next;
	struct cmd_node * prev;
} node;

typedef struct cmd_stack {
	node * root;
	node * top;
	int empty;
} stack;

stack* stack_init();
void stack_push(stack* s, command_t cmd);
command_t stack_top(stack* s);
void delete_nodes(node* n);
void stack_pop(stack* s);
void stack_delete(stack* s);

// Should move to read-command
struct command_stream
{
	command_t tree;
	struct command_stream * next;
	struct command_stream * last;
};

void push_command_stream(command_stream_t cs, command_t cmd);

command_t construct_command(enum command_type t);
void pop_one_operator(stack* cstack, stack* opstack);
int precedence_cmp(enum command_type a, enum token_type b);

command_stream_t parse_tokens(token* T);
