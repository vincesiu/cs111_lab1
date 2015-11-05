#pragma once

#include "command.h" 
#include "command-internals.h"
#include "parser.h"

void parallel_execute(command_stream_t head);
void parallel_apply_dependencies(command_stream_t head);
