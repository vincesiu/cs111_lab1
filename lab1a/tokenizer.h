
typedef TRUE 1
typedef FALSE 0

struct Token 
{
  enum token_type
  {
    COMMAND,
    OPERATOR,
  };
  
  char *word;
  int len;
  int line_num;
  struct Token *next;
  token_type type;
}
