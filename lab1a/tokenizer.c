#include <stdio.h>

isOperatorCha
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
    while(word[idx] != '\0') {
        printf("lawl\n");			
        idx++;
    }
}

int main (int argc, char const *argv[])
{
    tokenize(argv[1]);
    return 0;
}
