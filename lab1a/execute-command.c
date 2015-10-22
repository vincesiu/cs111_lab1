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
    else if (c->pipe_redirection[0] == 1 || c->pipe_redirection[0] == 3)
    {
      dup2(c->pipe_redirection[1], 0);
    }

    if (c->output != NULL)
    {
      out = open(c->output, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR );
      dup2(out, 1);
      close(out);
    }
    else if (c->pipe_redirection[0] >= 2)
    {
      dup2(c->pipe_redirection[2], 1);
    }

    if (c->pipe_redirection[0] == 1)
      close(c->pipe_redirection[1]);
    if (c->pipe_redirection[0] == 2)
      close(c->pipe_redirection[2]);
    
    execvp(c->u.word[0], (c->u.word));
  }
  else 
  {
    waitpid(pid, &status, 0);
    c->status = status;
    if (c->pipe_redirection[0] >= 2)
    {
      close(c->pipe_redirection[2]);
    }
    if (c->pipe_redirection[0] == 1 || c->pipe_redirection[0] == 3)
    {
      close(c->pipe_redirection[1]);
    }
  }
    
}


void
subshell_propagate_helper(command_t c, char *input, char *output)
{
  if (c->type == SIMPLE_COMMAND || c->type == SUBSHELL_COMMAND || c->type == PIPE_COMMAND)
  {
    if (c->input == NULL)
      c->input = input;
    if (c->output == NULL)
      c->output = output;
  }
  else 
  {
    subshell_propagate_helper(c->u.command[0], input, output);
    subshell_propagate_helper(c->u.command[1], input, output);
  }
}




void 
subshell_propagate_io (command_t c, char *input, char *output, int *pipe_redirection)
{
    command_t command_first = c;
    command_t command_last = c;

    while(1)
    {
      if (command_first->type == SIMPLE_COMMAND || command_first->type == SUBSHELL_COMMAND)
      {
        break;
      }
      else
      {
        command_first = command_first->u.command[0];
      }
    }

    while(1)
    {
      if (command_last->type == SIMPLE_COMMAND || command_last->type == SUBSHELL_COMMAND)
      {
        break;
      }
      else
      {
        command_last = command_last->u.command[1];
      }
    }


    if (pipe_redirection[0] == 1 || pipe_redirection[0] == 3)
    {
      command_first->pipe_redirection[1] = pipe_redirection[1];
      command_first->pipe_redirection[0] += 1;
    }

    if (pipe_redirection[0] == 2 || pipe_redirection[0] == 3)
    {
      command_last->pipe_redirection[2] = pipe_redirection[2];
      command_last->pipe_redirection[0] += 2;
    }

    subshell_propagate_helper(c, input, output);


}


void
pipe_connect (command_t c)
{
  command_t command_send = c->u.command[0];
  command_t command_get = c->u.command[1];

//  int *fd = malloc(sizeof(int) * 2);
  int fd[2];
  pipe(fd);


  while(1)
  {
    if (command_send->type == SIMPLE_COMMAND || command_send->type == SUBSHELL_COMMAND)
    {
      break;
    }
    else
    {
      command_send = command_send->u.command[1];
    }
  }

  while(1)
  {
    if (command_get->type == SIMPLE_COMMAND || command_get->type == SUBSHELL_COMMAND)
    {
      break;
    }
    else
    {
      command_get = command_get->u.command[0];
    }
  }

  /*
      if (command_send->output == NULL && command_get->input == NULL)
      {
        command_send->pipe_redirection[0] += 2;
        command_send->pipe_redirection[2] = fd[1];
      }

      if (command_get->input == NULL && command_send->output == NULL)
      {
        command_get->pipe_redirection[0] += 1;
        command_get->pipe_redirection[1] = fd[0];
      }
      */
        command_send->pipe_redirection[0] += 2;
        command_send->pipe_redirection[2] = fd[1];
        command_get->pipe_redirection[0] += 1;
        command_get->pipe_redirection[1] = fd[0];
}




void
pipe_propagate_io (command_t c, char *input, char *output)
{
  command_t command_first = c->u.command[0];
  command_t command_last = c->u.command[1];

  while(1)
  {
    if (command_first->type == SIMPLE_COMMAND || command_first->type == SUBSHELL_COMMAND)
    {
      if (command_first->input == NULL)
        command_first->input = input;
      break;
    }
    else
    {
      command_first = command_first->u.command[0];
    }
  }

  while(1)
  {
    if (command_last->type == SIMPLE_COMMAND || command_last->type == SUBSHELL_COMMAND)
    {
      if (command_last->output == NULL)
        command_last->output = output;
      break;
    }
    else
    {
      command_last = command_last->u.command[1];
    }
  }

}


void
execute_command (command_t c, int time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */


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
      //interesting example
      //(cat <file1 && tr a z && cat) < file2
      //(echo lmao > file1 | tr h z) < file2

      subshell_propagate_io(c->u.subshell_command, c->input, c->output, c->pipe_redirection);
      execute_command(c->u.subshell_command, time_travel);
      c->status = c->u.subshell_command->status;
      
      break;

    case (PIPE_COMMAND):
      //interesting example
      //(echo hello | tr h z) 
      //(echo hello | tr h z < file1)
      pipe_propagate_io(c, c->input, c->output);
      pipe_connect(c);
      execute_command(c->u.command[0], time_travel);
      execute_command(c->u.command[1], time_travel);

      break;

    default:
      printf("error: invalid command passed u broke our code");
  }

	time_travel;

  //run_simple_command(c);

  // error (1, 0, "command execution not yet implemented");
}


