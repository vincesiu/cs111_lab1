#include <stdio.h>
#include <stdlib.h>

#include "tokenizer.h"

void tokenize(const char *word) 
{
  subtoken *head = subtoken_init();
  subtoken *cur_subtoken = head; 

  int idx = 0;
  char cur_char;

  while (word[idx] != '\0')
  {
    cur_char = word[idx];

    if (cur_subtoken->type == S_NULLTOKEN)
    {
      if (isOperatorChar(cur_char))
      {
        switch(cur_char)
        {
          case '(':
            cur_subtoken->type = S_SUBSHELLLEFT;
            break;
          case ')':
            cur_subtoken->type = S_SUBSHELLRIGHT;
            break;
          case ';':
            cur_subtoken->type = S_SEQUENCE;
            break;
          case '>':
            cur_subtoken->type = S_OUTPUT;
            break;
          case '<':
            cur_subtoken->type = S_INPUT;
            break;
          case '|':
            cur_subtoken->type = S_INCOMPLETEOR;
            break;
          case '&':
            cur_subtoken->type = S_INCOMPLETEAND;
            break;
        }

        if (cur_char != '|' && cur_char != '&')
        {
          subtoken_addNew(&cur_subtoken);
        }
      }
      else if (isCommandChar(cur_char))
      {
        cur_subtoken->type = S_COMMAND;
        cur_subtoken->length = 1;
        cur_subtoken->word = malloc(sizeof(char) * 2);
        *cur_subtoken->word = cur_char;
      }
      else if (cur_char == '\n')
      {
        cur_subtoken->type = S_NEWLINE;
        subtoken_addNew(&cur_subtoken);
      }
      else if (cur_char != ' ' && cur_char != '\t')
      {
        printf("error: invalid character\n");
        abort();
      }

      idx++;
    }





    else if (cur_subtoken->type == S_INCOMPLETEOR)
    {
      if (cur_char == '|')
      {
        cur_subtoken->type = S_OR;
        idx++;
      }
      else
      {
        cur_subtoken->type = S_PIPE; 
      }
      subtoken_addNew(&cur_subtoken);
    }




    else if (cur_subtoken->type == S_INCOMPLETEAND)
    {
      if (cur_char == '&')
      {
        cur_subtoken->type == S_AND;
        idx++;
        subtoken_addNew(&cur_subtoken);
      }
      else
      {
        printf("error: invalid character");
        abort();
      }
    }




    else if (cur_subtoken->type == S_COMMAND)
    {
      if (!isCommandChar(cur_char))
      {
        subtoken_addNew(&cur_subtoken); 
      }
      else
      {
        cur_subtoken->word[cur_subtoken->length] = cur_char;
        cur_subtoken->length++;
        cur_subtoken->word = realloc(cur_subtoken->word, sizeof(char) * (cur_subtoken->length + 1));
        idx++;
      }
    }




  }


  
  if (cur_subtoken->type == S_INCOMPLETEOR || cur_subtoken->type == S_INCOMPLETEAND) 
    abort();

  subtoken_debug(head);

}

int main (int argc, char const *argv[])
{
  char *input = "echo -v < foo > bar";
  tokenize(input);
  return 0;
}


int isOperatorChar(char c)
{
  switch(c) 
  {
    case '|': 
    case ';':
    case '(':
    case ')':
    case '<':
    case '>':
    case '&':
      return true;
    default:
      return false;
  }
}

int isCommandChar(char c)
{
  if ('a' <= c && c <= 'z')
    return true;
  if ('A' <= c && c <= 'Z')
    return true;
  switch(c)
  {
    case '!': 
    case '%':
    case '+':
    case ',':
    case '-':
    case '.':
    case '/':
    case ':':
    case '@':
    case '^':
    case '_':
      return true;
    default:
      return false;
  }
}

subtoken *subtoken_init()
{
  subtoken *foo = malloc(sizeof(subtoken));

  foo->length = 0;
  foo->word = NULL;
  foo->next = NULL;
  foo->type = S_NULLTOKEN;

  return foo;
}


void subtoken_debug(subtoken *head)
{
  printf("DEBUGGING THE SUBTOKEN STREAM:\n");
  while (head != NULL)
  {
    switch(head->type)
    {
      case S_NULLTOKEN:
        printf("NULLTOKEN\n");
        break;
      case S_NEWLINE:
        printf("NEWLINE\n");
        break;
      case S_AND:
        printf("AND\n");
        break;
      case S_OR:
        printf("OR\n");
        break;
      case S_PIPE:
        printf("PIPE\n");
        break;
      case S_INPUT:
        printf("INPUT\n");
        break;
      case S_OUTPUT:
        printf("OUTPUT\n");
        break;
      case S_SUBSHELLLEFT:
        printf("SUBSHELLLEFT\n");
        break;
      case S_SUBSHELLRIGHT:
        printf("SUBSHELLRIGHT\n");
        break;
      case S_INCOMPLETEOR:
        printf("INCOMPLETEOR\n");
        break;
      case S_INCOMPLETEAND:
        printf("INCOMPLETEAND\n");
        break;
      case S_COMMAND:
        printf("COMMAND\n");
        break;
      default:
        printf("type code: %i\n", head->type);
        printf("Error, invalid token in final stream\n");
        abort();
    }
    head = head->next;
  }
}

void subtoken_addNew(subtoken **head)
{
  (*head)->next = subtoken_init(); 
  *head = (*head)->next; 
}
