#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "typeChecker.h"
#include "codegen.h"

extern baseST *realBase ;
extern int x ;

int moduleGeneration (astNode *node, int localBase, int rspDepth, moduleST *lst, varST *vst, FILE *fp)
{
	if (node == NULL)
		return 0 ;

	switch (node->id)
	{
		int start_label, end_label ;
		int reserveLabel[10] ;
		varST *searchedVar ;

		case statements :
			moduleGeneration (node->child, localBase, rspDepth, node->localST, vst, fp);		// Access local scope and move below
			break ;

		case statement :
			rspDepth = moduleGeneration(node->child, localBase, rspDepth, lst, vst, fp);
			if (node->next == NULL)
			{
				if (lst->tableType == SWITCH_ST)
					break ;

				if (lst->parent == realBase)
				{
					fprintf (fp, "\n\t\tMOV RSP, RBP\n") ;
					fprintf (fp, "\t\tPOP RBP\n") ;
					fprintf (fp, "\t\tret\n") ;
				}
				else if (rspDepth - localBase > 0)
					fprintf (fp, "\t\tADD RSP, %d\t\t\t\t\t\t\t\t; Restoring RSP to previous scope\n", (rspDepth-localBase)) ;
			}
			else
				moduleGeneration(node->next, localBase, rspDepth, lst, vst, fp);

			break ;

		case TK_DECLARE :
			;
			int switchPass = 0 ;
			if (lst->tableType != SWITCH_ST)
				; // Subtract RSP, and allocate memory if dynamic
			else if (node->parent != node)
				switchPass = 1 ; // Subtract RSP only, and do not allocate memory for dynamic
			else
			{
				switchPass = 2 ; // Do not subtract RSP, allocate memory if dynamic AND restore parent
				node->parent = node->next->parent ;
			}

			int endOffset, declCount = 1 ;
			astNode *idListHead = node->next->child ;
			astNode *dtNode = node->next->next ;

			while (idListHead->next != NULL)
			{
				declCount++ ;
				idListHead = idListHead->next ;
			}

			searchedVar = searchVar (realBase, lst, idListHead->tok->lexeme) ;
			endOffset = searchedVar->offset ;
			if (endOffset > 0 && (switchPass == 0 || switchPass == 1))
			{	
				fprintf (fp, "\t\tSUB RSP, %d\t\t\t\t\t\t\t\t; Updating RSP\n", (endOffset - rspDepth)) ;
				rspDepth = endOffset ;
			}

			if (dtNode->dtTag == ARRAY && !isVarStaticArr(searchedVar) && (switchPass == 0 || switchPass == 2))
				dynamicDeclareGeneration (lst, searchedVar, declCount, fp) ;

			
			break ;											

		case TK_ASSIGNOP :
			if (node->child->next == NULL)
				printf ("Unary\n") ;
			moduleGeneration(node->child->next, localBase, rspDepth,  lst, vst, fp);
			moduleGeneration(node->child, localBase, rspDepth, lst, vst, fp);
			fprintf (fp, "\n") ;
			break ;

		case TK_PLUS :
			moduleGeneration(node->child, localBase, rspDepth, lst, vst, fp);
			moduleGeneration(node->child->next, localBase, rspDepth,  lst, vst, fp);
			fprintf (fp, "\t\tPOP AX\n");
			fprintf (fp, "\t\tPOP BX\n");
			fprintf (fp, "\t\tADD AX,BX\n");
			fprintf (fp, "\t\tPUSH AX\n");

			break ;

		case TK_MINUS :
			moduleGeneration(node->child->next, localBase, rspDepth, lst, vst, fp);
			moduleGeneration(node->child, localBase, rspDepth, lst, vst, fp);
			fprintf (fp, "\t\tPOP AX\n");
			fprintf (fp, "\t\tPOP BX\n");
			fprintf (fp, "\t\tSUB AX,BX\n");
			fprintf (fp, "\t\tPUSH AX\n");

			break ;

		case TK_MUL :
			moduleGeneration(node->child->next, localBase, rspDepth, lst, vst, fp);
			moduleGeneration(node->child, localBase, rspDepth, lst, vst, fp);
			fprintf (fp, "\t\tPOP AX\n");
			fprintf (fp, "\t\tPOP BX\n");
			fprintf (fp, "\t\tIMUL BX\n");
			fprintf (fp, "\t\tPUSH AX\n");

			break ;

		case TK_DIV :
			moduleGeneration(node->child, localBase, rspDepth, lst, vst, fp);
			moduleGeneration(node->child->next, localBase, rspDepth, lst, vst, fp);
			fprintf (fp, "\t\tPOP BX\n");
			fprintf (fp, "\t\tPOP AX\n\n");

			fprintf (fp, "\t\tIDIV BX\n");
			fprintf (fp, "\t\tPUSH AX\n");
			break ;

		case TK_RNUM :
			fprintf (fp, "NO RNUM ALLOWED!\n") ;
			exit (0) ;
			break ;

		case TK_NUM : 
			if (node->prev == NULL)
			{
				fprintf (fp, "\t\tMOV AX, %s\n",node->tok->lexeme);
				fprintf (fp, "\t\tPUSH AX\n");	
			}
			else
			{
				fprintf (fp, "\t\tMOV BX, %s\n",node->tok->lexeme);
				fprintf (fp, "\t\tPUSH BX\n");	
			}
			break ;

		case TK_TRUE :
			if (node->prev == NULL)
			{
				fprintf (fp, "\t\tMOV AX, 1\n");
				fprintf (fp, "\t\tPUSH AX\n");	
			}
			else
			{
				fprintf (fp, "\t\tMOV BX, 1\n");
				fprintf (fp, "\t\tPUSH BX\n");	
			}
			break ;

		case TK_FALSE :
			if (node->prev == NULL)
			{
				fprintf (fp, "\t\tMOV AX, 0\n");
				fprintf (fp, "\t\tPUSH AX\n");	
			}
			else
			{
				fprintf (fp, "\t\tMOV BX, 0\n");
				fprintf (fp, "\t\tPUSH BX\n");	
			}
			break ;


		case TK_ID :
			IDGeneration (node, lst, fp) ;
			break ;
		
		case TK_FOR :
			node=node->next;

			fprintf (fp, "\t\tMOV CX,%s\n", node->next->tok->lexeme);
			fprintf (fp, "\t\tMOV [RBP - %d], CX\t\t\t\t\t\t\t\t; for loop lower lim\n" , searchVar(realBase, lst, node->tok->lexeme)->offset);

			fprintf (fp, "\n\t.forStart:\n");
			fprintf (fp, "\t\tMOV AX, %s\n", node->next->next->tok->lexeme) ;
			fprintf (fp, "\t\tCMP CX,AX\n");
			fprintf (fp, "\t\tJG .forEnd\n\n");

			moduleGeneration(node->next->next->next, rspDepth, rspDepth, lst, vst, fp);		// Statements

			fprintf (fp, "\n\t\tMOV CX, [RBP - %d]\t\t\t\t\t\t\t\t; Ending increment\n", searchVar(realBase, lst, node->tok->lexeme)->offset);
			fprintf (fp, "\t\tINC CX\n");
			fprintf (fp, "\t\tMOV [RBP - %d],CX\n", searchVar(realBase, lst, node->tok->lexeme)->offset);
			fprintf (fp, "\t\tJMP .forStart\n");
			fprintf (fp, "\n\t.forEnd:\n");
			break ;

		
		case TK_WHILE :
			node=node->next;
			start_label = get_label();
			end_label =  get_label();
			fprintf (fp, "\n\t.whileStart:\n");

			moduleGeneration(node, localBase, rspDepth, lst, vst, fp);	// expression

			fprintf (fp, "\t\tPOP AX\n");
			fprintf (fp, "\t\tCMP AX, 1\n");
			fprintf (fp, "\t\tJNE .whileEnd\n");

			moduleGeneration(node->next, rspDepth, rspDepth, lst, vst, fp);		// statements

			fprintf (fp, "\t\tJMP .whileStart\n");
			fprintf (fp, "\n\t.whileEnd:\n");

			break ;

		case TK_LT : case TK_GT : case TK_LE : case TK_GE : case TK_NE : case TK_EQ :
			moduleGeneration(node->child, localBase, rspDepth, lst, vst, fp);
			moduleGeneration(node->child->next, localBase, rspDepth, lst, vst, fp);

			fprintf (fp, "\t\tPOP BX\n");
			fprintf (fp, "\t\tPOP AX\n");
			fprintf (fp, "\t\tCMP AX,BX\n");

			switch (node->id)
			{
				case TK_LT :
					fprintf (fp, "\t\tSETL AL\n");
					break ;
				case TK_GT :
					fprintf (fp, "\t\tSETG AL\n");
					break ;
				case TK_LE :
					fprintf (fp, "\t\tSETLE AL\n");
					break ;
				case TK_GE :
					fprintf (fp, "\t\tSETGE AL\n");
					break ;
				case TK_NE :
					fprintf (fp, "\t\tSETNE AL\n");
					break ;
				case TK_EQ :
					fprintf (fp, "\t\tSETE AL\n");
					break ;
			}
			fprintf (fp, "\t\tMOVSX AX, AL\n") ;
			fprintf (fp, "\t\tPUSH AX\n") ;

			break ;

		case TK_AND : case TK_OR :
			moduleGeneration(node->child, localBase, rspDepth, lst, vst, fp);
			moduleGeneration(node->child->next, localBase, rspDepth, lst, vst, fp);

			fprintf (fp, "\t\tPOP BX\n");
			fprintf (fp, "\t\tPOP AX\n"); 

			fprintf (fp, "\t\tCMP AX, 1\n");
			if (node->id == TK_AND)
				fprintf (fp, "\t\tAND AX, BX\n");
			else
				fprintf (fp, "\t\tOR AX, BX\n");

			fprintf (fp, "\t\tPUSH AX\n") ;

			break ;

		case TK_PRINT :
			node = node->next ;
			printGeneration (node, lst, fp) ;
			break ;

		case TK_GET_VALUE :
			searchedVar = searchVar (realBase, lst, node->next->tok->lexeme) ;
			getValueGeneration (lst, searchedVar, rspDepth, fp) ;
			break ;

		case TK_SWITCH :
			;
			int i, caseCount, savedRspDepth, def_label ;
			int *caseLabels ;
			astNode *statementsNode = node->next->next->next->next ;

			savedRspDepth = rspDepth ;

			rspDepth = switchDeclareVars (node->next->next->next->next, vst, rspDepth, fp) ;
			caseCount = getCaseCount (node->next->next->next->next) ;
			caseLabels = (int *) malloc (sizeof(int) * caseCount) ;
			def_label = switchCaseLabels (node , lst, caseCount, caseLabels, fp) ;

			end_label = get_label () ;
			i = 0 ;
			while (statementsNode != NULL)
			{
				fprintf (fp, "\nLABEL%d:\n", caseLabels[i]) ;
				moduleGeneration (statementsNode, savedRspDepth, rspDepth, lst, vst, fp) ;
				fprintf (fp ,"\n\t\tJMP LABEL%d\n", end_label) ;

				i++ ;
				if (statementsNode->next != NULL)
				{
					if (statementsNode->next->id == TK_DEFAULT)
					{
						statementsNode = statementsNode->next->next ;
						fprintf (fp, "\nLABEL%d:\n", def_label) ;
						moduleGeneration (statementsNode, savedRspDepth, rspDepth, lst, vst, fp) ;

						break ;
					}
					statementsNode = statementsNode->next->next->next ;
				}
				else
					statementsNode = NULL ;
			}


			fprintf (fp ,"\nLABEL%d:\n", end_label) ;
			fprintf (fp, "\n\t\tADD RSP, %d\n", rspDepth - savedRspDepth) ;
			rspDepth = savedRspDepth ;

			break ;
	}

	return rspDepth ;
}

void codeGeneration(astNode *node, FILE* fp)
{
	if (node == NULL)
		return;

	switch (node->id)
	{
		case program :
			fprintf (fp, "\nmain:\n") ;
			fprintf (fp, "\t\tPUSH RBP\n") ;
			fprintf (fp, "\t\tMOV RBP, RSP\n\n") ;
			codeGeneration (node->child->next, fp) ;		// <otherModules>
			break ;

		case otherModules :
			codeGeneration (node->child, fp) ;					// Do module definitions
			codeGeneration (node->next, fp) ;					// <driverModule> or NULL
			break ;

		case driverModule :
			moduleGeneration(node->child, 0, 0, NULL, NULL, fp); 				// <statements>
			codeGeneration(node->next, fp); 				// Move to the next child of program
			break ;
	}
}

int main(int argc, char *argv[])
{
	if (argc != 2 && argc != 3)
	{
		printf ("\t\tIncorrect number of arguments\n") ;
		exit (0) ;
	}

	treeNode *root = parseTree (argv[1]) ;

	if (root->syntax_error)
	{
		printf ("The source file contains lexical/syntactical errors. Please correct them\n") ;
		exit (0) ;
	}

	astNode *astRoot ;
	astRoot = applyASTRule (root) ;
	realBase = fillSymbolTable(astRoot,0);

	if (argc == 3)
	{
		if (strcmp(argv[2], "-printAST") == 0)
			inorderAST (astRoot, 0) ;
		else
		{
			printf ("Incorrect command!\n") ;
			exit (0) ;
		}
	}

	if (!realBase->semanticError)
	{
		FILE *fp = fopen ("code.asm", "w") ;

		preamble (fp) ;
		codeGeneration (astRoot, fp) ;
		printCommentLineNASM (fp) ;
		postamble (fp) ;
	}
	else
		printf ("Semantic errors found. Please check the above messages\n") ;
	return 0;
}