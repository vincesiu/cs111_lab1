#include <stdio.h>
#include <stdlib.h>

#include "command-internals.h"
#include "command.h"
#include "tokenizer.h"
#include "parser.h"

#define INPUTSTRING "(a)(b) c"
#define DEBUG_TOKENIZER false 

#if DEBUG_TOKENIZER

int main (int argc, char const *argv[])
{
  char *input = INPUTSTRING;

  subtoken *head_subtoken = subtokenize(input);
  subtoken_debug(head_subtoken);

  token *head_token = tokenize(head_subtoken);
  token_debug(head_token);


  command_stream_t streamtest = parse_tokens(head_token);
  command_stream_t printer = streamtest;

  while (printer != NULL)
  {
    print_command(printer->tree);
    printer = printer->next;
  }

  subtoken_destructor(head_subtoken);
  token_destructor(head_token);
  return 0;
}

#endif

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
  if ('0' <= c && c <= '9')
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



/*
 * Given a getc function and a stream will return the head of a stream
 * of subtokens, which should be passed to the tokenize function
 */
subtoken *subtokenize(int (*get_next_byte) (void *), void *get_next_byte_argument)
{
  subtoken *head = subtoken_init();
  subtoken *cur_subtoken = head; 

  int idx = 0;
  char cur_char;
  int flag_readChar = false;
  int line_num = 0;

  cur_char = get_next_byte(get_next_byte_argument);

  while (cur_char != EOF)
  {
    if (cur_subtoken->type == S_NULLTOKEN)
    {
      cur_subtoken->line_num = line_num;
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
        line_num++;
      }
      else if (cur_char == '#')
      {
        cur_subtoken->type = S_COMMENT;
      }
      else if (cur_char != ' ' && cur_char != '\t')
      {
        printf("cur_char : %d or %c\n", cur_char, cur_char);
        error_parsing(line_num, "Invalid character in input");
      }
      flag_readChar = true;
    }





    else if (cur_subtoken->type == S_INCOMPLETEOR)
    {
      if (cur_char == '|')
      {
        cur_subtoken->type = S_OR;
        flag_readChar = true;
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
        flag_readChar = true;
        subtoken_addNew(&cur_subtoken);
      }
      else
      {
        error_parsing(cur_subtoken->line_num, "Invalid character & in input");
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
        flag_readChar = true;
      }
    }

    else if (cur_subtoken->type == S_COMMENT)
    {
      if (cur_char == '\n')
      {
        subtoken_addNew(&cur_subtoken); 
      }
      flag_readChar = true;
    } 

    if (flag_readChar)
    {
      cur_char = get_next_byte(get_next_byte_argument);
      flag_readChar = false;
    }


  }



  /*
   * Thinking about including a general error check:
   * if last token is operator, then error, which might have some exceptions
   * like sequence, which is the only one I can think of off the top of my
   * head
   */
  if (cur_subtoken->type == S_INCOMPLETEOR) 
  {
    error_parsing(cur_subtoken->line_num, "Last token is an operator");
  }
  if (cur_subtoken->type == S_INCOMPLETEAND)
  {
    error_parsing(cur_subtoken->line_num, "Invalid character & in input");
  }

  

  return head;
}

/*
 * Initializes a blank template subtoken.
 */
subtoken *subtoken_init()
{
  subtoken *foo = malloc(sizeof(subtoken));

  foo->length = 0;
  foo->word = NULL;
  foo->next = NULL;
  foo->type = S_NULLTOKEN;
  foo->line_num = 0;

  return foo;
}

/*
 * Prints out a list based on the subtokens
 */
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
      case S_SEQUENCE:
        printf("SEQUENCE\n");
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

/*
 * Pretty much condenses two lines into one. 
 * tbh, this isn't really necessary, it just makes the code more concise
 */
void subtoken_addNew(subtoken **head)
{
  (*head)->next = subtoken_init(); 
  *head = (*head)->next; 
}

/*
 * Destructor yey
 */
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

/*
 * Takes in a linked list of subtokens as generated by the subtoken function,
 * and generates a linked list of tokens which will then be passed to the
 * parser function
 */
token *tokenize(subtoken *subtoken_head)
{
  subtoken *cur_subtoken = subtoken_head;
  token *cur_token = NULL;
  token *prev_token = NULL;
  token *token_head = NULL;

  int count_newline = 0;

  while (cur_subtoken != NULL)
  {
    if (cur_subtoken->type == S_NULLTOKEN && cur_subtoken->next == NULL)
    {
      break;
    }
    if (cur_subtoken->type == S_NULLTOKEN || cur_subtoken->type == S_INCOMPLETEOR || cur_subtoken->type == S_INCOMPLETEAND)
    {
      error_parsing(cur_subtoken->line_num, "Invalid token in subtoken stream when tokenizing -- this is most likely an edge case that we missed");
    }
    else if (cur_subtoken->type == S_NEWLINE)
    {
      if (cur_token != NULL)
      {
        count_newline++;
      }
      cur_subtoken = cur_subtoken->next;
    }
    else if (cur_subtoken->type == S_COMMENT)
    {
      cur_subtoken = cur_subtoken->next;
    }
    else
    {
      if (count_newline > 0)
      {
        if (prev_token->type == INPUT || prev_token->type == OUTPUT)
        {
          // TODO: replace this error handler 
          // error_parsing(cur_subtoken->line_num, "Newlines may not come after < or >");
        }
        if (cur_subtoken->type != S_SUBSHELLLEFT && cur_subtoken->type != S_SUBSHELLRIGHT && cur_subtoken->type != S_COMMAND)
        {
          error_parsing(cur_subtoken->line_num, "Newlines may only precede subshell parentheses or a simple command");
        }

        if (prev_token->type == SIMPLE || prev_token->type == SUBSHELLRIGHT)
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
          /*
          //hackerz fix
          if (cur_subtoken->next->type != S_COMMAND)
          {
            error_parsing(cur_subtoken->line_num, "Simple commands must follow an input or output");
          }
          cur_token->word = cur_subtoken->next->word;
          cur_token->length = cur_subtoken->next->length;
          //cur_subtoken = cur_subtoken->next;
          */
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
          error_parsing(cur_subtoken->line_num, "Subtoken processing into token error, most likey an edge case that I missed");
      }
      
      prev_token = cur_token;
      cur_subtoken = cur_subtoken->next;
    }
  }


  cur_token = token_head;
  while (cur_token != NULL)
  {
    if (cur_token->type == INPUT || cur_token->type == OUTPUT)
    {
          //hackerz fix
          if (cur_token->next == NULL || cur_token->next->type != SIMPLE)
          {
            error_parsing(cur_subtoken->line_num, "Simple commands must follow an input or output");
          }
          cur_token->word = cur_token->next->word;
          cur_token->length = cur_token->next->length;
          cur_token->next = cur_token->next->next;
    }
    cur_token = cur_token->next;
  }
  return token_head;
}

/*
 * Takes a subtoken as input and creates a token which copies the internals
 * of the subtoken with the exception of the type
 */
token *token_init(subtoken *input)
{
  token *foo = malloc(sizeof(token));

  foo->length = input->length;
  foo->word = input->word;
  foo->next = NULL;
  foo->line_num = input->line_num;

  return foo;
}

/* 
 * Outputs a list to standard output of the types of tokens in the
 * linked list of tokens
 */
void token_debug(token *head)
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

/* 
 * Destructor for the linked list of tokens. This one is weird because it uses
 * the pointers for *word that subtoken uses, and since subtoken has already
 * freed its memory, then this should be irrelevant.
 */
void token_destructor(token *head)
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
