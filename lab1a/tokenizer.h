
enum token_type
{
  COMMAND,
  OPERATOR,
};


struct Token 
{
  char *word;
  int len;
  int line_num;
  struct Token *next;
  token_type type;
}
