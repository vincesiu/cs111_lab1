// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>


/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

int
command_status (command_t c)
{
  return c->status;
}

void
run_simple_command (command_t c)
{
  if (c->type != SIMPLE_COMMAND)
    printf("lol what is dis u gave me a nonsimple command");

  pid_t pid;  
  int status = 0;

  int in, out;

  if ((pid = fork()) < 0) 
  {
    printf("error forking");
  }
  else if (pid == 0) 
  {
    if (c->input != NULL)
    {
      in = open(c->input, O_RDONLY);
      dup2(in, 0);
      close(in);
    }

    if (c->output != NULL)
    {
      out = open(c->output, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR );
      dup2(out, 1);
      close(out);
    }
    
    execvp(c->u.word[0], (c->u.word));
  }
  else 
    waitpid(pid, &status, 0);

  c->status = status;
}


void
execute_command (command_t c, int time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */

  command_t subshell_first = NULL;
  command_t subshell_last = NULL;

  switch(c->type)
  {
    case(SIMPLE_COMMAND):
      run_simple_command(c);
      break;

    case (SEQUENCE_COMMAND):
      execute_command(c->u.command[0], time_travel);
      execute_command(c->u.command[1], time_travel);
      c->status = c->u.command[1]->status;
      break;

    case (AND_COMMAND):
      execute_command(c->u.command[0], time_travel);
      if (c->u.command[0]->status == 0)
      {
        execute_command(c->u.command[1], time_travel);
        c->status = c->u.command[1]->status;
      }
      else
      {
        c->status = c->u.command[0]->status;
      }
      break;
      
    case (OR_COMMAND):
      execute_command(c->u.command[0], time_travel);
      if (c->u.command[0]->status != 0)
      {
        execute_command(c->u.command[1], time_travel);
        c->status = c->u.command[1]->status;
      }
      else
      {
        c->status = c->u.command[0]->status;
      }
      break;

    case (SUBSHELL_COMMAND):
      if (c->input != NULL)
      {
        subshell_first = c->u.subshell_command;

        while(subshell_first->type != SIMPLE_COMMAND)
        {
          if (subshell_first->type == SUBSHELL_COMMAND)
            subshell_first = subshell_first->u.subshell_command;
          else
            subshell_first = subshell_first->u.command[0];
        }
        subshell_first->input = c->input;
      }

      if (c->output != NULL)
      {
        subshell_last = c->u.subshell_command;

        while(subshell_last->type != SIMPLE_COMMAND)
        {
          if (subshell_last->type == SUBSHELL_COMMAND)
            subshell_last = subshell_last->u.subshell_command;
          else
            subshell_last = subshell_last->u.command[1];
        }
        subshell_last->output = c->output;
      }

      execute_command(c->u.subshell_command, time_travel);
      c->status = c->u.subshell_command->status;
      
      break;

    case (PIPE_COMMAND):
      printf("poop");
      break;

    default:
      printf("error: invalid command passed u broke our code");
  }

	time_travel;

  //run_simple_command(c);

  // error (1, 0, "command execution not yet implemented");
}


