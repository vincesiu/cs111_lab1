#include "parallelizer.h"

void parallel_execute(command_stream_t head)
{
  if (head->flag_used)
    head->flag_used + 1;

  return;
}
