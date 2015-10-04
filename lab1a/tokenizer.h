
typedef TRUE 1
typedef FALSE 0

struct Token 
{
  char *word;
  enum token_type {COMMAND, OPERATOR, OPERATORINCOMPLETE} type;
  int len;
  int line_num;
  struct Token *next;
}

int isOperatorChar(char c);
int isCommandChar(char c);

