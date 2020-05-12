output : erplag.o codegen.o ast.o typeChecker.o lexer.o parser.o symbolTable.o error.o lexer.h typeChecker.h lexerDef.h parser.h parserDef.h symbolTable.h codegen.h error.h
	gcc -w -o .compiler erplag.o codegen.o ast.o lexer.o parser.o symbolTable.o typeChecker.o error.o
	
allExec : erplag.o codegen.o astDriver.o ast.o typeChecker.o lexer.o parser.o symbolTableDriver.o error.o lexer.h typeChecker.h lexerDef.h symbolTable.o parser.h parserDef.h symbolTable.h codegen.h error.h
	gcc -w -o ast astDriver.o ast.o lexer.o parser.o error.o
	gcc -w -o ST symbolTableDriver.o ast.o lexer.o parser.o symbolTable.o typeChecker.o error.o
	gcc -w -o .compiler erplag.o codegen.o ast.o lexer.o parser.o symbolTable.o typeChecker.o error.o

erplag.o : erplag.c
	gcc -w -c erplag.c

error.o : error.c
	gcc -w -c error.c

codegen.o : codegen.c
	gcc -w -c codegen.c

astDriver.o : astDriver.c
	gcc -w -c astDriver.c

ast.o : ast.c
	gcc -w -c ast.c

typeChecker.o : typeChecker.c
	gcc -w -c typeChecker.c

parser.o : parser.c
	./parser.sh

lexer.o : lexer.c
	gcc -w -c lexer.c

symbolTableDriver.o : symbolTableDriver.c
	gcc -w -c symbolTableDriver.c

asm :
	make -f make.asm FILE=$(FILE)

erp :
	make
	./compiler $(FILE).erp $(FILE).asm

erpAsm :
	make erp FILE=$(FILE)
	make asm FILE=$(FILE)

erpExec :
	make erpAsm FILE=$(FILE)
	./$(FILE)

clean :
	rm -f *.o
	rm -f ST
	rm -f ast

exeClean :
	rm -f .compiler
	rm -f ST
	rm -f ast

objClean :
	rm -f *.o
