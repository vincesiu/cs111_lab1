#include "parallelizer.h"
#include "execute-command.h"

#define NULL 0

void parallel_execute(command_stream_t head)
{

  command_stream_t current = head;

  parallel_apply_dependencies(head);

  while (head != NULL)
  {
    execute_command(head->tree, 0);
    head = head->next;
  }
  
  return;
}




void parallel_apply_dependencies(command_stream_t head)
{
  
  command_stream_t current = head;


}


