#include "parallelizer.h"
#include "execute-command.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int stringcmp( char *one, char *two );

void parallel_execute(command_stream_t head)
{

  command_stream_t current = head;

  parallel_apply_dependencies(head);

  dependency_list *cur_dependency;
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
      //pid = waitpid( -1, &status, 0);

      if ( (pid = waitpid( -1, &status, 0)) == -1)
        error_parsing(current->tree->lineno, "waitpid returned unsuccessfully");

      for ( current = head; current != NULL; current = current->next)
      {
        if (current->pid == pid)
        {
          current->flag_commandrunning = 0;
          current->flag_commanddone = 1;
          current->tree->status = status;

          for (cur_dependency = current->dependency_list; cur_dependency != NULL; cur_dependency = cur_dependency->next) 
          {
            cur_dependency->cur_stream->dependency_num--;
          }
          break;
        }
      }


      
    }

  } while (num_running != 0);
  
  return;
}




void parallel_apply_dependencies(command_stream_t head)
{
  command_stream_t forward_dep = head;
  command_stream_t current_dep = NULL;
  wordlist *cur_word = NULL;
  wordlist *dep_word = NULL;

  dependency_list *current_list = NULL;
  for ( ; forward_dep != NULL; forward_dep = forward_dep->next )
  {
    for ( current_dep = head; current_dep != forward_dep; current_dep = current_dep->next ) 
    {
      for ( cur_word = forward_dep->read_list; cur_word != NULL; cur_word = cur_word->next )
      {
        for (dep_word = current_dep->write_list; dep_word != NULL; dep_word = dep_word->next )
        {
          if (stringcmp(dep_word->word, cur_word->word))
          {
            forward_dep->dependency_num++;
            current_list = current_dep->dependency_list; 
            if (current_list != NULL)
            {
              for ( current_list = current_dep->dependency_list; current_list->next != NULL; current_list = current_list->next );
            }
            current_list->next = malloc(sizeof(dependency_list));
            current_list->next->cur_stream = forward_dep; 
          }
        }
      }


      for ( cur_word = forward_dep->write_list; cur_word != NULL; cur_word = cur_word->next )
      {
        for (dep_word = current_dep->write_list; dep_word != NULL; dep_word = dep_word->next )
        {
          if (stringcmp(dep_word->word, cur_word->word))
          {
            forward_dep->dependency_num++;
            current_list = current_dep->dependency_list; 
            if (current_list != NULL)
            {
              for ( current_list = current_dep->dependency_list; current_list->next != NULL; current_list = current_list->next );
            }
            current_list->next = malloc(sizeof(dependency_list));
            current_list->next->cur_stream = forward_dep; 
          }
        }
        for (dep_word = current_dep->read_list; dep_word != NULL; dep_word = dep_word->next )
        {
          if (stringcmp(dep_word->word, cur_word->word))
          {
            forward_dep->dependency_num++;
            current_list = current_dep->dependency_list; 
            if (current_list != NULL)
            {
              for ( current_list = current_dep->dependency_list; current_list->next != NULL; current_list = current_list->next );
            }
            current_list->next = malloc(sizeof(dependency_list));
            current_list->next->cur_stream = forward_dep; 
          }
        }
      }


    }

  }

}


int stringcmp( char *one, char *two )
{
  while (*one != 0 && *two != 0)
  {
    if (one++ != two++)
    {
      break;
    }
  }
  
  if (*one != 0 || *two != 0)
    return 0;
  else 
    return 1;
}
