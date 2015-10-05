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
};

typedef struct Subtoken
{
  enum subtoken_type type;
  char *word;
  int length;
  struct Subtoken *next;
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
} token;

int isOperatorChar(char c);
int isCommandChar(char c);


subtoken *subtoken_init();
subtoken *subtoken_addNew(subtoken *cur_subtoken);
subtoken *subtoken_nullTokenHandle(char c);
void subtoken_debug(subtoken *head);


/*
struct Token *token_init();
void token_setType(enum token_type type);
void token_addChar(char c);
*/
