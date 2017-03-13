%union{
  char *name;
  int val;
}

%{
#include <stdio.h>
#include "codegen.h"
#include "table.h"
#define FIRSTADDR 2

extern int warning(const char* s);

%}

%token		COLOEQ EQ NOTEQ LT GT LE GE
%token		CONST VAR FUNCTION BEGINN END IF THEN WHILE
%token		DO RETURN WRITE WRITELN ODD IDENT NUMBER

%token  <name> IDENT
%token  <val>  NUMBER

%%
program	:  			{ blockBegin(FIRSTADDR); }
		  block '.'
		;

block		: 			{ $<val>$ = genCodeV(jmp,0);}
                declList		{ backPatch($<val>1);
		    			 genCodeV(ict, frameL()); }
                statement		{ genCodeR(); blockEnd(); }
		;

declList	: /* empty */
		| declList decl
		;

decl		: constDecl
		| varDecl
		| funcDecl
		;

constDecl	: CONST numberList ';'
		;

numberList	: IDENT EQ NUMBER		{ enterTconst($1, $3); }
		| numberList COMMA IDENT EQ NUMBER
                  				{ enterTconst($3, $5); }
		;

varDecl	: VAR identList ';'
		;

identList	: IDENT 			{ enterTvar($1);}
		| identList COMMA IDENT	{ enterTvar($3); }
		;

optParList	: /* empty */
		| parList
		;
parList        : IDENT 			{ enterTpar($1);}
		| parList COMMA IDENT	{ enterTpar($3); }
		;

funcDecl	: FUNCTION IDENT   	{ enterTfunc($2, nextCode()); blockBegin(FIRSTADDR);}
                  '('  optParList ')' 	{ endpar();}
			block ';'
		;

statement	: /* empty */
		| IDENT COLOEQ expression
                    			{ genCodeT(sto, searchT($1, varId));}
		| BEGINN statement stateList END
		| IF condition THEN   { $<val>$ = genCodeV(jpc, 0); }
                   statement       { backPatch($<val>4); }
		| WHILE		{ $<val>$ = nextCode(); }
		   condition DO	{ $<val>$ = genCodeV(jpc, 0); }
                    statement	{ genCodeV(jmp, $<val>2);
		      			backPatch($<val>5); }
		| RETURN expression	{ genCodeR(); }
		| WRITE expression	{ genCodeO(wrt); }
		| WRITELN		{ genCodeO(wrl); }
		;

stateList	: /* empty */
		| stateList ';' statement
		;

condition	: ODD expression 			{ genCodeO(odd); }
		| expression EQ expression 		{ genCodeO(eq); }
		| expression NOTEQ expression 	{ genCodeO(neq); }
		| expression LT expression 		{ genCodeO(ls); }
		| expression GT expression 		{ genCodeO(gr); }
		| expression LE expression 		{ genCodeO(lseq); }
		| expression GE expression 		{ genCodeO(greq); }
		;

expression	: '-'  term 	{ genCodeO(neg);}
		   termList
		| term  termList
		;

termList	:  /* empty */
		| termList '+' term 	{ genCodeO(add); }
		| termList '-' term 	{ genCodeO(sub); }
		;

term		: factor factList
		;
factList	: /* empty */
		| factList '*' factor 	{ genCodeO(mul); }
		| factList '/' factor 	{ genCodeO(div); }
		;

factor		: IDENT	{ int j, k; j = searchT($1, varId); k = kindT(j);
		    			switch(k){
		     			case varId: case parId:
		      				genCodeT(lod, j); break;
		     			case constId:
		      				genCodeV(lit, val(j)); break;
		    			}
		  		}
		| NUMBER 	{ genCodeV(lit, $1);}

		| IDENT '(' expList ')'	{ genCodeT(cal, searchT($1, varId));}
		| '(' expression ')'
		;

expList	:
		| expression
		| expList ',' expression
		;

COMMA	: ','
		| 		{ warning("warning: missing comma\n");}
		;

%%

int warning(const char* s) {
  fprintf(stderr, "warn: %s\n", s);
  return 0;
}

void errorType(char* s)  { yyerror(s); }
void errorF(char* s)     { yyerror(s); }
