#define true 1
#define false 0

enum token_type 
  {
    COMMAND, 
    OPERATOR, 
    OPERATORINCOMPLETE, 
    UNLOADED,
  }; 

struct Token 
{
  char *word;
  int len;
  int line_num;
  struct Token *next;
  enum token_type type;
}

int isOperatorChar(char c);
int isCommandChar(char c);
struct Token *token_init();
void token_setType(enum token_type type);
void token_addChar(char c);
