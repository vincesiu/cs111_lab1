#include "parallelizer.h"
#include "execute-command.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void parallel_execute(command_stream_t head)
{

  command_stream_t current = head;

  parallel_apply_dependencies(head);

  int num_running = 0;
  pid_t pid;
  int status = 0;
  int idx = 0;

  do 
  {
    num_running = 0;
    
    for ( current = head; current != NULL; current = current->next)
    {

      if (!current->flag_commanddone && current->dependency_num == 0)
      {
        if ((pid = fork()) < 0)
        {
          error_parsing(0, "forking failed when trying to parallelize");
        }
        else if (pid == 0)
        {
          execute_command(current->tree, 0);
          exit(current->tree->status);
        }
        else
        {
          current->flag_commandrunning = 1;
          current->pid = pid;
          num_running++;
        }
      }

    }

    for ( idx = num_running; idx > 0; idx-- )
    {
      pid = waitpid( -1, &status, 0);

      if (pid == -1)
      {
        error_parsing(current->tree->lineno, "waitpid returned unsuccessfully");
      }

      for ( current = head; current != NULL; current = current->next)
      {
        if (current->pid == pid)
        {
          current->flag_commandrunning = 0;
          current->flag_commanddone = 1;
          current->tree->status = status;

          break;
        }
      }


      
    }

  } while (num_running != 0);
  
  return;
}




void parallel_apply_dependencies(command_stream_t head)
{
  
  command_stream_t current = head;


}


