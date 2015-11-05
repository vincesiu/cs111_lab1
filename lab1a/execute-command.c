// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"
#include "execute-command.h"

#include <error.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>


#define PIPE_WRITEEND 1
#define PIPE_READEND 0

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

int
command_status (command_t c)
{
  return c->status;
}

void 
simple_apply_io (command_t c)
{
  if (c->type != SIMPLE_COMMAND)
    error_parsing(c->lineno, "simple_apply_io was passed a non simple command");

  if (c->input != NULL)
  {
    if (c->r_input != -1)
      close(c->r_input);

    c->r_input = open(c->input, O_RDONLY);
  }

  if (c->output != NULL)
  {
    if (c->r_output != -1)
      close(c->r_output);
    
    c->r_output = open(c->output, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR );
  }
}

void
simple_execute (command_t c)
{
  if (c->type != SIMPLE_COMMAND)
    error_parsing(c->lineno, "simple_execute was passed a non simple command");

  pid_t pid;
  int status = 0;
  
  int in, out;

  if ((pid = fork()) < 0)
  {
    error_parsing(c->lineno, "forking failed when running simple command");
  }
  else if (pid == 0)
  {
    if (c->r_input != -1)
    {
      dup2(c->r_input, 0);
    }
    if (c->r_output != -1)
    {
      dup2(c->r_output, 1);
    }

    execvp(c->u.word[0], (c->u.word));
  }
  else 
  {
    waitpid(pid, &status, 0);

    c->status = status;

    if (c->r_input != -1)
      close(c->r_input);
    if (c->r_output != -1)
      close(c->r_output);
  }

}

void
subshell_propagate_helper(command_t c, command_t top)
{
  if (c->type == SIMPLE_COMMAND || c->type == SUBSHELL_COMMAND || c->type == PIPE_COMMAND)
  {
    if (c->r_input == -1)
      c->r_input = top->r_input;
    if (c->r_output == -1)
      c->r_output = top->r_output;
    if (c->input == NULL)
      c->input = top->input;
    if (c->output == NULL)
      c->output = top->output;
  }
  else 
  {
    subshell_propagate_helper(c->u.command[0], top);
    subshell_propagate_helper(c->u.command[1], top);
  }
}

void 
subshell_propagate_io (command_t c)
{
  command_t command_first = c->u.subshell_command;
  command_t command_last = c->u.subshell_command;

  while(command_first->type != SIMPLE_COMMAND && command_first->type != SUBSHELL_COMMAND) 
    command_first = command_first->u.command[0];
  while(command_last->type != SIMPLE_COMMAND && command_last->type != SUBSHELL_COMMAND)
    command_last = command_last->u.command[1];

  subshell_propagate_helper(c->u.subshell_command, c);
}


void
pipe_connect (command_t c)
{
  command_t command_send = c->u.command[0];
  command_t command_get = c->u.command[1];

  int fd[2];
  pipe(fd);

  while(command_send->type != SIMPLE_COMMAND && command_send->type != SUBSHELL_COMMAND)
    command_send = command_send->u.command[1];
  while(command_get->type != SIMPLE_COMMAND && command_get->type != SUBSHELL_COMMAND)
    command_get = command_get->u.command[0];

  if (pipe(fd) == -1)
    error_parsing(c->lineno, "Pipe allocation failed");

  if (command_get->r_input == -1)
   command_get->r_input = fd[PIPE_READEND]; 
  else
    close(fd[PIPE_READEND]);

  if (command_send->r_output == -1)
    command_send->r_output = fd[PIPE_WRITEEND];
  else
    close(fd[PIPE_WRITEEND]);

}




void
pipe_propagate_io (command_t c)
{
  command_t command_first = c->u.command[0];
  command_t command_last = c->u.command[1];

  while(command_first->type != SIMPLE_COMMAND && command_first->type != SUBSHELL_COMMAND)
    command_first = command_first->u.command[0];

  while(command_last->type != SIMPLE_COMMAND && command_last->type != SUBSHELL_COMMAND)
    command_last = command_last->u.command[1];

  if (command_first->r_input == -1)
    command_first->r_input = c->r_input;
  if (command_last->r_output == -1)
    command_last->r_output = c->r_output;

  if (command_first->input == NULL)
    command_first->input = c->input;
  if (command_last->output == NULL)
    command_last->output = c->output;
}


void
execute_command (command_t c, int time_travel)
{
  switch(c->type)
  {
    case(SIMPLE_COMMAND):
      simple_apply_io(c);
      simple_execute(c);
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
      //interesting example
      //(cat <file1 && tr a z && cat) < file2
      //(echo lmao > file1 | tr h z) < file2

      subshell_propagate_io(c);
      execute_command(c->u.subshell_command, time_travel);
      c->status = c->u.subshell_command->status;
      
      break;

    case (PIPE_COMMAND):
      //interesting example
      //(echo hello | tr h z) 
      //(echo hello | tr h z < file1)
      pipe_propagate_io(c);
      pipe_connect(c);
      execute_command(c->u.command[0], time_travel);
      execute_command(c->u.command[1], time_travel);

      break;

    default:
      error_parsing(c->lineno, "Invalid/new command_t type passed to execute function");
  }


}


