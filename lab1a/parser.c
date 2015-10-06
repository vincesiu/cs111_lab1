#include <stdio.h>
#include <stdlib.h>

#include "command-internals.h"
#include "command.h"
#include "tokenizer.h"
#include "parser.h"

stack* stack_init()
{
	stack* temp = malloc(sizeof(stack));
	temp->root = malloc(sizeof(node));
	temp->root->next = NULL;
	temp->root->prev = NULL;
	temp->empty = 1;
	temp->top = NULL;

	return temp;
}

void stack_push(stack* s, command_t cmd)
{
	if (s->empty)
	{
		s->top = s->root;
		s->empty = 0;
	}
	else
	{
		s->top->next = malloc(sizeof(node));
		s->top->next->next = NULL;
		s->top->next->prev = s->top;
		s->top = s->top->next;
	}

	s->top->c = cmd;
}

command_t stack_top(stack* s)
{
	if (s->empty)
		return NULL;
	else
		return s->top->c;
}

void delete_nodes(node* n)
{
	if (n->next != NULL)
		delete_nodes(n->next);
	node* nprev = n->prev;
	if (nprev != NULL)
		nprev->next = NULL;
	free(n);
	return;
}

void stack_pop(stack* s)
{
	if (s->empty)
		return;
	else
	{
		s->top = s->top->prev;
		if (s->top == NULL)
			s->empty = 1;
		else
		{
			delete_nodes(s->top->next);
			s->top->next = NULL;
		}
	}
}

void stack_delete(stack* s)
{
	delete_nodes(s->root);
	free(s);
}



void push_command_stream(command_stream_t cs, command_t cmd)
{
	if (cs->last == NULL)
	{
		cs->tree = cmd;
		cs->last = cs;
		return;
	}

	cs->last->next = malloc(sizeof(struct command_stream));
	cs->last = cs->last->next;
	cs->last->tree = cmd;
	cs->last->next = NULL;
	cs->last->last = NULL;
}

/* TODO:
- Switch all malloc, realloc to the correct "safe" implementation

*/

// Command constructor -- TODO error checking
command_t construct_command(enum command_type t)
{
	command_t newcmd = malloc(sizeof(struct command));
	newcmd->status = -1;
	newcmd->input = NULL;
	newcmd->output = NULL;
	newcmd->type = t;

	return newcmd;
}

// Pop one operator and construct appropriate command
void pop_one_operator(stack* cstack, stack* opstack)
{
	command_t top_op = stack_top(opstack);

	stack_pop(opstack);
	command_t one = stack_top(cstack);
	stack_pop(cstack);
	command_t two = stack_top(cstack);
	stack_pop(cstack);

	top_op->u.command[0] = one;
	top_op->u.command[1] = two;

	stack_push(cstack, top_op);

	return;
}

// true if a >= b in terms of precedence
int precedence_cmp(command_t a_cmd, enum token_type b)
{
	if (a_cmd == NULL)
		return false;

	enum command_type a = a_cmd->type;

	if (a == SUBSHELL_COMMAND)
		return false;

	if (a == PIPE_COMMAND)
		return true;
	else if (a == AND_COMMAND || a == OR_COMMAND)
	{
		if (b == PIPE)
			return false;
		else
			return true;
	}
	else
	{
		if (b == SEQUENCE)
			return true;
		else
			return false;
	}
}

void error_parsing(int lineno, char* msg)
{
	fprintf(stderr, "Line %d: %s\n", lineno, msg);
	exit(1);
}

command_stream_t parse_tokens(token* T)
{
	command_stream_t stream = malloc(sizeof(struct command_stream));
	stream->next = NULL;
	stream->last = NULL;

	stack* command_stack = stack_init();
	stack* op_stack = stack_init();

	// simple command helpers
	char ** curword = NULL;
	int curwordlen = 0;
	int simple_started = 0;
	int subshell_started = 0;

	command_t current = NULL;

	// malloc(sizeof(struct command));

	for (; T != NULL; T = T->next)
	{
		// SIMPLE, INPUT, OUTPUT token handler
		if (T->type == SIMPLE)
		{
			if (simple_started)
			{
				curwordlen++;

				// TODO: unsafe realloc
				curword = realloc(curword, sizeof(char*) * (curwordlen + 1));
				curword[curwordlen - 1] = T->word;
				curword[curwordlen] = NULL;
			}
			else
			{
				current = construct_command(SIMPLE_COMMAND);
				simple_started = 1;
				curwordlen = 1;
				curword = malloc(sizeof(char*) * 2);
				curword[0] = T->word;
				curword[1] = NULL;
			}
		}
		else if (T->type == INPUT)
		{
			if (!simple_started)
				error_parsing(T->line_num, "semantic error - input attempted without simple command\n");
			current->input = T->word;
		}
		else if (T->type == OUTPUT)
		{
			if (!simple_started)
				error_parsing(T->line_num, "semantic error - output attempted without simple command\n");
			current->output = T->word;
		}

		// All other command handling in this else
		else
		{
			if (simple_started)
			{
				current->u.word = curword;
				curword = NULL;
				curwordlen = 0;
				stack_push(command_stack, current);
			}

			if (T->type == STARTNEWCOMMAND)
			{
				if (subshell_started > 0)
					error_parsing(T->line_num, "semantic error - reached end of command with open subshell");

				while (op_stack->empty == 0)
					pop_one_operator(command_stack, op_stack);

				push_command_stream(stream, stack_top(command_stack));
				stack_pop(command_stack);
			}
			else if (T->type == SUBSHELLLEFT)
			{
				current = construct_command(SUBSHELL_COMMAND);
				stack_push(op_stack, current);
				subshell_started++;
			}
			else if (T->type == SUBSHELLRIGHT)
			{
				if (subshell_started == 0)
					error_parsing(T->line_num, "semantic error - invalid subshell closure attempted");

				command_t top_op = stack_top(op_stack);
				while (top_op->type != SUBSHELL_COMMAND)
				{
					pop_one_operator(command_stack, op_stack);
					top_op = stack_top(op_stack);
				}
				stack_pop(op_stack);
				command_t top_cmd = stack_top(command_stack);
				stack_pop(command_stack);
				top_op->u.subshell_command = top_cmd;
				stack_push(command_stack, top_op);
				subshell_started--;
			}

			// Standard operator handling
			//if (T->type == PIPE || T->type == AND || T->type == OR || T->type == SEQUENCE)
			else
			{
				if (!simple_started)
					error_parsing(T->line_num, "semantic error - operator attempted without simple command\n");

				if (op_stack->empty == 0)
				{
					command_t top_op = stack_top(op_stack);

					// precedence_cmp is true if top_op has greater or equal precedence to T->type
					while (precedence_cmp(top_op, T->type))
					{
						pop_one_operator(command_stack, op_stack);
						top_op = stack_top(op_stack);
					}
				}

				switch (T->type)
				{
				case PIPE: current = construct_command(PIPE_COMMAND); break;
				case AND: current = construct_command(AND_COMMAND); break;
				case OR: current = construct_command(OR_COMMAND); break;
				default: current = construct_command(SEQUENCE_COMMAND); break;
				}
				stack_push(op_stack, current);
			}

			simple_started = 0;
		}
	}

	if (simple_started)
	{
		current->u.word = curword;
		curword = NULL;
		curwordlen = 0;
		simple_started = 0;
		stack_push(command_stack, current);
	}

	if (subshell_started > 0)
		error_parsing(T->line_num, "semantic error - reached end of command with open subshell");

	while (op_stack->empty == 0)
		pop_one_operator(command_stack, op_stack);
	push_command_stream(stream, stack_top(command_stack));
	stack_pop(command_stack);

	// Free stack memory
	stack_delete(op_stack);
	stack_delete(command_stack);

	return stream;
}
