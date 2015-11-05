#pragma once

#include "parser.h"

int command_status (command_t c);
void simple_apply_io (command_t c);
void simple_execute (command_t c, int time_travel);
void subshell_propagate_helper (command_t c, command_t top);
void subshell_propagate_io (command_t c);
void pipe_connect (command_t c);
void pipe_propagate_io (command_t c);
void execute_command (command_t c, int time_travel);
