#include <stdio.h>

/*
int isValidOperator(struct Token *foo) {
	if (foo->len == 1) {
		switch(foo->word[0]) {
			case '|': 
			case ';':
			case '(':
			case ')':
			case '<':
			case '>':
				return true;
			default: 
				return false;
		}
	}
	if (foo->len == 2) {
		if (foo->word[0] == '&' && foo->word[1] == '&')
			return true;
		if (foo->word[1] == '|' && foo->word[1] == '|')
			return true;
	}
	
	return false;
	
}

int isValidWord() {

}
*/



void tokenize(char *word) 
{
  int idx = 0;
  struct Token cur = malloc(sizeof(struct Token)); 
  for (; word[idx] != '\0'; idx++)
  {
   1 = 1;
  }

}

int main (int argc, char const *argv[])
{
    tokenize(argv[1]);
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
