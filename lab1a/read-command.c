// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "parser.h"
#include "tokenizer.h"

#include <error.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */

  subtoken *head_subtoken = subtokenize(get_next_byte, get_next_byte_argument);
  token *head_token = tokenize(head_subtoken);
  
  //subtoken_debug(head_subtoken);
  //token_debug(head_token);

  
  command_stream_t streamtest = parse_tokens(head_token);
  command_stream_t printer = streamtest;

  /*
  while( printer != NULL)
  {
    print_command(printer->tree);
    printer = printer->next;
  }

  */
  return streamtest;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */

  while (s != NULL && s->flag_used) 
  {
    s = s->next;
  }

  if (s != NULL)
  {
    s->flag_used = true;
    return s->tree;
  }
  else
  {
    return NULL;
  }
}
