#include <stdio.h>
#include <stdlib.h>

#include "tokenizer.h"

#define INPUTSTRING ": : : \n\n a b<c > d \n\n a&&b||\nc &&\nd | e && f|\n\ng < h"

int main (int argc, char const *argv[])
{
  char *input = INPUTSTRING;

  subtoken *head_subtoken = subtokenize(input);
  subtoken_debug(head_subtoken);

  token *head_token = tokenize(head_subtoken);
  token_debug(head_token);

  subtoken_destructor(head_subtoken);
  token_destructor(head_token);
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

subtoken *subtokenize(const char *word) 
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
      else if (cur_char == '#')
      {
        cur_subtoken->type = S_COMMENT;
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
        cur_subtoken->type = S_AND;
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

    else if (cur_subtoken->type == S_COMMENT)
    {
      if (cur_char == '\n')
      {
        subtoken_addNew(&cur_subtoken); 
      }
      idx++;
    } 


  }



  if (cur_subtoken->type == S_INCOMPLETEOR || cur_subtoken->type == S_INCOMPLETEAND) 
    abort();

  return head;

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

void subtoken_destructor(subtoken *head)
{
  subtoken *prev = NULL;

  while(head != NULL)
  {
    prev = head;
    if (head->word != NULL)
      free(head->word);
    head = head->next;
    free(prev);
  }
}


token *tokenize(subtoken *subtoken_head)
{
  subtoken *cur_subtoken = subtoken_head;
  token *cur_token = NULL;
  token *prev_token = NULL;
  token *token_head = NULL;

  int count_newline = 0;

  while (cur_subtoken != NULL)
  {
    if (cur_subtoken->type == S_NULLTOKEN || cur_subtoken->type == S_INCOMPLETEOR || cur_subtoken->type == S_INCOMPLETEAND)
    {
      printf("Error, invalid token in subtoken stream when tokenizing");
      abort();
    }
    else if (cur_subtoken->type == S_NEWLINE)
    {
      if (cur_token != NULL)
      {
        count_newline++;
      }
      cur_subtoken = cur_subtoken->next;
    }
    else
    {
      if (count_newline > 0)
      {
        if (prev_token->type == INPUT || prev_token->type == OUTPUT)
        {
          printf("Error, a newline followed a < or >");
          abort();
        }
        if (cur_subtoken->type != S_SUBSHELLLEFT && cur_subtoken->type != S_SUBSHELLRIGHT && cur_subtoken->type != S_COMMAND)
        {
          printf("Error, newline may not precede this token: %i", cur_subtoken->type);
          abort();
        }

        if (prev_token->type == SIMPLE)
        {
          if (count_newline == 1)
          {
            cur_token->next = token_init(cur_subtoken);
            cur_token = cur_token->next;
            cur_token->type = SEQUENCE;
          }
          else if (count_newline > 1)
          {
            cur_token->next = token_init(cur_subtoken);
            cur_token = cur_token->next;
            cur_token->type = STARTNEWCOMMAND;
          }
        }
        count_newline = 0;
      }
      if (cur_token == NULL)
      {
        cur_token = token_init(cur_subtoken);
        token_head = cur_token;
      }
      else
      {
        cur_token->next = token_init(cur_subtoken);
        cur_token = cur_token->next;
      }

      switch(cur_subtoken->type)
      {
        case S_COMMAND:
          cur_token->type = SIMPLE;
          break;
        case S_SEQUENCE:
          cur_token->type = SEQUENCE;
          break;
        case S_AND:
          cur_token->type = AND;
          break;
        case S_OR:
          cur_token->type = OR;
          break;
        case S_PIPE:
          cur_token->type = PIPE;
          break;
        case S_INPUT:
          cur_token->type = INPUT;
          break;
        case S_OUTPUT:
          cur_token->type = OUTPUT;
          break;
        case S_SUBSHELLLEFT:
          cur_token->type = SUBSHELLLEFT;
          break;
        case S_SUBSHELLRIGHT:
          cur_token->type = SUBSHELLRIGHT;
          break;
        default:
          printf("What happened....");
          abort();
      }
      prev_token = cur_token;
      cur_subtoken = cur_subtoken->next;
    }
  }
  return token_head;
}

token *token_init(subtoken *input)
{
  token *foo = malloc(sizeof(token));

  foo->length = input->length;
  foo->word = input->word;
  foo->next = NULL;

  return foo;
}

void *token_debug(token *head)
{
  printf("DEBUGGING THE TOKEN STREAM:\n");
  while (head != NULL)
  {
    switch(head->type)
    {
      case STARTNEWCOMMAND:
        printf("STARTNEWCOMMAND\n");
        break;
      case AND:
        printf("AND\n");
        break;
      case SEQUENCE:
        printf("SEQUENCE\n");
        break;
      case OR:
        printf("OR\n");
        break;
      case PIPE:
        printf("PIPE\n");
        break;
      case INPUT:
        printf("INPUT\n");
        break;
      case OUTPUT:
        printf("OUTPUT\n");
        break;
      case SUBSHELLLEFT:
        printf("SUBSHELLLEFT\n");
        break;
      case SUBSHELLRIGHT:
        printf("SUBSHELLRIGHT\n");
        break;
      case SIMPLE:
        printf("SIMPLE\n");
        break;
      default:
        printf("type code: %i\n", head->type);
        printf("Error, invalid token in final stream\n");
        abort();
    }
    head = head->next;
  }
}


void *token_destructor(token *head)
{
  token *prev = NULL;

  while(head != NULL)
  {
    prev = head;
//  if (head->word != NULL)
//    free(head->word);
    head = head->next;
    free(prev);
  }
}
