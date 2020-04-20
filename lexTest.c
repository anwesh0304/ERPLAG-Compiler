/*
Group #52 -
Anwesh Bhattacharya (2016B5A70590P)
Deepak Chahar (2017A7PS0147P)
Rohan Kela (2016B1A70822P)
Komal Vasudeva (2017A7PS0103P)
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lexerDef.h"
#include "lexer.h"


extern char* tokenIDToString (tokenID id);
extern int charBeforeAsterisk;
extern int lineBeforeAsterisk;
// Goes to driver
int printNextToken (twinBuffer* twinBuf)
{
	token* tk ;
	int ret = 1 ;
	tk = getNextToken (twinBuf) ;

	if (tk == NULL)
	{
		printf ("printNextToken() Error!\n") ;
		ret = 0 ;
	}
	else if (tk->id == TK_EOF)
	{	
		//printf ("Reached end of file at line number %d\n" , tk->lineNumber) ;
		//printf ("----------------------\n") ;
		ret = 0 ;
	}
	else if (tk->id == TK_LEXERROR)
		printf ("Error at line number %d : %s\n" , tk->lineNumber, tk->lexeme) ;
	else if (tk->id == TK_COMMENT)
		; 
		//printf ("Comment ending at line number %d\n" , tk->lineNumber) ;
	else
		printf ("%15d\t%15s\t%15s\n", tk->lineNumber, tk->lexeme, tokenIDToString(tk->id)) ;

	return ret ;
}

int printNextTokenCOMMENT (twinBuffer *twinBuf)
{
	token* tk ;
	int ret = 1 ;

	tk = getNextTokCOMMENT (twinBuf) ;

	if (tk->id == TK_EOF)
		ret = 0 ;
	else if (tk->id == TK_COMMENT) 
	{
		int i ;
		if (charBeforeAsterisk == '\n')
		{
			if (*twinBuf->forward != '\n')
				printf("\n") ;

			for (i = lineBeforeAsterisk+2 ; i < tk->lineNumber ; i++)
				printf("\n") ;
		}
		else
		{
			if (*twinBuf->forward != '\n')
				printf ("\n") ;
			
			for (i = lineBeforeAsterisk +1 ; i < tk->lineNumber ; i++)
				printf("\n") ;
		}
	}
	else
		printf ("%s", tk->lexeme) ;

	return ret ;
}

// int hola ()
// {
// 	//twinBuffer *twinBuf = lexer_init ("parseTest.txt") ;
	
// 	printf ("----------------------\n") ;

// 	while (printNextTokenCOMMENT(twinBuf))
// 		;
	

// 	printf ("\n------------------Done removeComments\n") ;
// 	return 0;
// }
