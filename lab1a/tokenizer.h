#ifndef TOKENIZER_H
#define TOKENIZER_H


#define true 1
#define false 0

enum subtoken_type {
  S_NULLTOKEN,
  S_NEWLINE,
  S_SEQUENCE,
  S_AND,
  S_OR,
  S_PIPE,
  S_INPUT,
  S_OUTPUT,
  S_SUBSHELLLEFT,
  S_SUBSHELLRIGHT,
  S_INCOMPLETEOR,
  S_INCOMPLETEAND,
  S_COMMAND,
  S_COMMENT,
};

typedef struct Subtoken
{
  enum subtoken_type type;
  char *word;
  int length;
  struct Subtoken *next;
  int line_num;
} subtoken;





enum token_type {
  STARTNEWCOMMAND,
  AND,
  SEQUENCE,
  OR,
  PIPE,
  SIMPLE,
  INPUT,
  OUTPUT,
  SUBSHELLLEFT,
  SUBSHELLRIGHT,
};

typedef struct Token
{
  enum token_type type;
  char *word;
  int length;
  struct Token *next;
  int line_num;
} token;

int isOperatorChar(char c);
int isCommandChar(char c);



subtoken *subtokenize(const char *word); 
subtoken *subtoken_init();
void subtoken_addNew(subtoken **cur_subtoken);
void subtoken_debug(subtoken *head);
void subtoken_destructor(subtoken *head);

token *tokenize(subtoken *subtoken_head);
token *token_init(subtoken *input);
void *token_debug(token *head);
void *token_destructor(token *head);


#endif //TOKENIZER_H
