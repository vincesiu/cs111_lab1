#include "parallelizer.h"
#include "execute-command.h"
#include <stdlib.h>
#include <unistd.h>


void parallel_execute(command_stream_t head)
{

  command_stream_t current = head;

  parallel_apply_dependencies(head);

  int num_running = 0;
  pid_t pid;

  do 
  {
    if (current->dependency_num == 0)
    {
      if ((pid = fork()) < 0)
      {
        error_parsing(0, "forking failed when trying to parallelize");
      }
      else if (pid == 0)
      {
        execute_command(current->tree, 0);
        exit(EXIT_SUCCESS);
      }
      else
      {
        current->flag_commandrunning = 1;
        current->pid = pid;
        num_running++;
      }
    }
    current = current->next;
  } while (num_running != 0);
  
  return;
}




void parallel_apply_dependencies(command_stream_t head)
{
  
  command_stream_t current = head;


}


