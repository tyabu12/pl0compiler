
   /**************getSource.c************/

#include <stdio.h>
#include <string.h>
#include "getSource.h"

#define MAXLINE 120		/*　１行の最大文字数　*/
#define MAXERROR 30		/*　これ以上のエラーがあったら終り　*/
#define MAXNUM  14		/*　定数の最大桁数　*/
#define TAB   5				/*　タブのスペース　*/
#define INSERT_C  "#0000FF"  /* 挿入文字の色 */
#define DELETE_C  "#FF0000"  /* 削除文字の色 */
#define TYPE_C  "#00FF00"  /* タイプエラー文字の色 */

static FILE *fpi;				/*　ソースファイル　*/
static FILE *fptex;			/*　LaTeX出力ファイル　*/
static char line[MAXLINE];	/*　１行分の入力バッファー　*/
static int lineIndex;			/*　次に読む文字の位置　*/
static char ch;				/*　最後に読んだ文字　*/

static Token cToken;			/*　最後に読んだトークン　*/
static KindT idKind;			/*　現トークン(Id)の種類　*/
static int spaces;			/*　そのトークンの前のスペースの個数　*/
static int CR;				/*　その前のCRの個数　*/
static int printed;			/*　トークンは印字済みか　*/

static int errorNo = 0;			/*　出力したエラーの数　*/
static char nextChar();		/*　次の文字を読む関数　*/
static int isKeySym(KeyId k);	/*　tは記号か？　*/
static int isKeyWd(KeyId k);		/*　tは予約語か？　*/
static void printSpaces();		/*　トークンの前のスペースの印字　*/
static void printcToken();		/*　トークンの印字　*/

struct keyWd {				/*　予約語や記号と名前(KeyId)　*/
	char *word;
	KeyId keyId;
};

static struct keyWd KeyWdT[] = {	/*　予約語や記号と名前(KeyId)の表　*/
	{"begin", Begin},
	{"end", End},
	{"if", If},
	{"then", Then},
	{"while", While},
	{"do", Do},
	{"return", Ret},
	{"function", Func},
	{"var", Var},
	{"const", Const},
	{"odd", Odd},
	{"write", Write},
	{"writeln",WriteLn},
	{"$dummy1",end_of_KeyWd},
							/*　記号と名前(KeyId)の表　*/
	{"+", Plus},
	{"-", Minus},
	{"*", Mult},
	{"/", Div},
	{"(", Lparen},
	{")", Rparen},
	{"=", Equal},
	{"<", Lss},
	{">", Gtr},
	{"<>", NotEq},
	{"<=", LssEq},
	{">=", GtrEq},
	{",", Comma},
	{".", Period},
	{";", Semicolon},
	{":=", Assign},
	{"$dummy2",end_of_KeySym}
};

int isKeyWd(KeyId k)			/*　キーkは予約語か？　*/
{
	return (k < end_of_KeyWd);
}

int isKeySym(KeyId k)		/*　キーkは記号か？　*/
{
	if (k < end_of_KeyWd)
		return 0;
	return (k < end_of_KeySym);
}

static KeyId charClassT[256];		/*　文字の種類を示す表にする　*/

static void initCharClassT()		/*　文字の種類を示す表を作る関数　*/
{
	int i;
	for (i=0; i<256; i++)
		charClassT[i] = others;
	for (i='0'; i<='9'; i++)
		charClassT[i] = digit;
	for (i='A'; i<='Z'; i++)
		charClassT[i] = letter;
	for (i='a'; i<='z'; i++)
		charClassT[i] = letter;
	charClassT['+'] = Plus; charClassT['-'] = Minus;
	charClassT['*'] = Mult; charClassT['/'] = Div;
	charClassT['('] = Lparen; charClassT[')'] = Rparen;
	charClassT['='] = Equal; charClassT['<'] = Lss;
	charClassT['>'] = Gtr; charClassT[','] = Comma;
	charClassT['.'] = Period; charClassT[';'] = Semicolon;
	charClassT[':'] = colon;
}

int openSource(char fileName[]) 		/*　ソースファイルのopen　*/
{
	char fileNameO[30];
	if ( (fpi = fopen(fileName,"r")) == NULL ) {
		printf("can't open %s\n", fileName);
		return 0;
	}
	strcpy(fileNameO, fileName);
	strcat(fileNameO,".html");  /* strcat(fileNameO,".tex"); */
	if ( (fptex = fopen(fileNameO,"w")) == NULL ) {	 /*　.html（または.tex）ファイルを作る　*/
		printf("can't open %s\n", fileNameO);
		return 0;
	} 
	return 1;
}

void closeSource()				 /*　ソースファイルと.html（または.tex）ファイルをclose　*/
{
	fclose(fpi);
	fclose(fptex);
}

void initSource()
{
	lineIndex = -1;				 /*　初期設定　*/
	ch = '\n';
	printed = 1;
	initCharClassT();
	fprintf(fptex,"<HTML>\n");   /*　htmlコマンド　*/
	fprintf(fptex,"<HEAD>\n<TITLE>compiled source program</TITLE>\n</HEAD>\n");
	fprintf(fptex,"<BODY>\n<PRE>\n");
/*  　LaTeXコマンド　
	fprintf(fptex,"\\documentstyle[12pt]{article}\n");   
	fprintf(fptex,"\\begin{document}\n");
	fprintf(fptex,"\\fboxsep=0pt\n");
	fprintf(fptex,"\\def\\insert#1{$\\fbox{#1}$}\n");
	fprintf(fptex,"\\def\\delete#1{$\\fboxrule=.5mm\\fbox{#1}$}\n");
	fprintf(fptex,"\\rm\n");
*/
}

void finalSource()
{
	if (cToken.kind==Period)
		printcToken();
	else
		errorInsert(Period);
	fprintf(fptex,"\n</PRE>\n</BODY>\n</HTML>\n");
	/* 	fprintf(fptex,"\n\\end{document}\n"); */
}
	
/*　通常のエラーメッセージの出力の仕方（参考まで）　*/
/*
void error(char *m)	
{
	if (lineIndex > 0)
		printf("%*s\n", lineIndex, "***^");
	else
		printf("^\n");
	printf("*** error *** %s\n", m);
	errorNo++;
	if (errorNo > MAXERROR){
		printf("too many errors\n");
		printf("abort compilation\n");	
		exit (1);
	}
}
*/

void errorNoCheck()			/*　エラーの個数のカウント、多すぎたら終わり　*/
{
	if (errorNo++ > MAXERROR){
		fprintf(fptex, "too many errors\n</PRE>\n</BODY>\n</HTML>\n");
		/* fprintf(fptex, "too many errors\n\\end{document}\n"); */
		printf("abort compilation\n");	
		exit (1);
	}
}

void errorType(char *m)		/*　型エラーを.html（または.tex）ファイルに出力　*/
{
	printSpaces();
	fprintf(fptex, "<FONT COLOR=%s>%s</FONT>", TYPE_C, m);
	/* fprintf(fptex, "\\(\\stackrel{\\mbox{\\scriptsize %s}}{\\mbox{", m); */
	printcToken();
	/* fprintf(fptex, "}}\\)"); */
	errorNoCheck();
}

void errorInsert(KeyId k)		/*　keyString(k)を.html（または.tex）ファイルに挿入　*/
{
	fprintf(fptex, "<FONT COLOR=%s><b>%s</b></FONT>", INSERT_C, KeyWdT[k].word);
	// 	if (k < end_of_KeyWd) 	/*　予約語　*/
	//		 fprintf(fptex, "\\ \\insert{{\\bf %s}}", KeyWdT[k].word); 
	//	else 					/*　演算子か区切り記号　*/
	//	fprintf(fptex, "\\ \\insert{$%s$}", KeyWdT[k].word); */
	errorNoCheck();
}

void errorMissingId()			/*　名前がないとのメッセージを.html（または.tex）ファイルに挿入　*/
{
	fprintf(fptex, "<FONT COLOR=%s>Id</FONT>", INSERT_C);
	/* fprintf(fptex, "\\insert{Id}"); */
	errorNoCheck();
}

void errorMissingOp()		/*　演算子がないとのメッセージを.html（または.tex）ファイルに挿入　*/
{
	fprintf(fptex, "<FONT COLOR=%s>@</FONT>", INSERT_C);
	/* fprintf(fptex, "\\insert{$\\otimes$}"); */
	errorNoCheck();
}

void errorDelete()			/*　今読んだトークンを読み捨てる　*/
{
	int i=(int)cToken.kind;
	printSpaces();
	printed = 1;
	if (i < end_of_KeyWd) 							/*　予約語　*/
		fprintf(fptex, "<FONT COLOR=%s><b>%s</b></FONT>", DELETE_C, KeyWdT[i].word);
		/* fprintf(fptex, "\\delete{{\\bf %s}}", KeyWdT[i].word); */
	else if (i < end_of_KeySym)					/*　演算子か区切り記号　*/
		fprintf(fptex, "<FONT COLOR=%s>%s</FONT>", DELETE_C, KeyWdT[i].word);
		/* fprintf(fptex, "\\delete{$%s$}", KeyWdT[i].word); */
	else if (i==(int)Id)								/*　Identfier　*/
		fprintf(fptex, "<FONT COLOR=%s>%s</FONT>", DELETE_C, cToken.u.id);
		/* fprintf(fptex, "\\delete{%s}", cToken.u.id); */
	else if (i==(int)Num)								/*　Num　*/
		fprintf(fptex, "<FONT COLOR=%s>%d</FONT>", DELETE_C, cToken.u.value);
		/* fprintf(fptex, "\\delete{%d}", cToken.u.value); */
}

void errorMessage(char *m)	/*　エラーメッセージを.html（または.tex）ファイルに出力　*/
{
	fprintf(fptex, "<FONT COLOR=%s>%s</FONT>", TYPE_C, m);
	/* fprintf(fptex, "$^{%s}$", m); */
	errorNoCheck();
}

void errorF(char *m)			/*　エラーメッセージを出力し、コンパイル終了　*/
{
	errorMessage(m);
	fprintf(fptex, "fatal errors\n</PRE>\n</BODY>\n</HTML>\n");
	/* fprintf(fptex, "fatal errors\n\\end{document}\n"); */
	if (errorNo)
		printf("total %d errors\n", errorNo);
	printf("abort compilation\n");	
	exit (1);
}

int errorN()				/*　エラーの個数を返す　*/
{
	return errorNo;
}

char nextChar()				/*　次の１文字を返す関数　*/
{
	char ch;
	if (lineIndex == -1){
		if (fgets(line, MAXLINE, fpi) != NULL){ 
/*			puts(line); */	/*　通常のエラーメッセージの出力の場合（参考まで）　*/
			lineIndex = 0;
		} else {
			errorF("end of file\n");      /* end of fileならコンパイル終了 */
		}
	}
	if ((ch = line[lineIndex++]) == '\n'){	 /*　chに次の１文字　*/
		lineIndex = -1;				/*　それが改行文字なら次の行の入力準備　*/
		return '\n';				/*　文字としては改行文字を返す　*/
	}
	return ch;
}

Token nextToken()			/*　次のトークンを読んで返す関数　*/
{
	int i = 0;
	int num;
	KeyId cc;
	Token temp;
	char ident[MAXNAME];
	printcToken(); 			/*　前のトークンを印字　*/
	spaces = 0; CR = 0;
	while (1){				/*　次のトークンまでの空白や改行をカウント　*/
		if (ch == ' ')
			spaces++;
		else if	(ch == '\t')
			spaces+=TAB;
		else if (ch == '\n'){
			spaces = 0;  CR++;
		}
		else break;
		ch = nextChar();
	}
	switch (cc = charClassT[ch]) {
	case letter: 				/* identifier */
		do {
			if (i < MAXNAME)
				ident[i] = ch;
			i++; ch = nextChar();
		} while (  charClassT[ch] == letter
				|| charClassT[ch] == digit );
		if (i >= MAXNAME){
			errorMessage("too long");
			i = MAXNAME - 1;
		}	
		ident[i] = '\0'; 
		for (i=0; i<end_of_KeyWd; i++)
			if (strcmp(ident, KeyWdT[i].word) == 0) {
				temp.kind = KeyWdT[i].keyId;  		/*　予約語の場合　*/
				cToken = temp; printed = 0;
				return temp;
			}
		temp.kind = Id;		/*　ユーザの宣言した名前の場合　*/
		strcpy(temp.u.id, ident);
		break;
	case digit: 					/* number */
		num = 0;
		do {
			num = 10*num+(ch-'0');
			i++; ch = nextChar();
		} while (charClassT[ch] == digit);
      		if (i>MAXNUM)
      			errorMessage("too large");
      		temp.kind = Num;
		temp.u.value = num;
		break;
	case colon:
		if ((ch = nextChar()) == '=') {
			ch = nextChar();
			temp.kind = Assign;		/*　":="　*/
			break;
		} else {
			temp.kind = nul;
			break;
		}
	case Lss:
		if ((ch = nextChar()) == '=') {
			ch = nextChar();
			temp.kind = LssEq;		/*　"<="　*/
			break;
		} else if (ch == '>') {
			ch = nextChar();
			temp.kind = NotEq;		/*　"<>"　*/
			break;
		} else {
			temp.kind = Lss;
			break;
		}
	case Gtr:
		if ((ch = nextChar()) == '=') {
			ch = nextChar();
			temp.kind = GtrEq;		/*　">="　*/
			break;
		} else {
			temp.kind = Gtr;
			break;
		}
	default:
		temp.kind = cc;
		ch = nextChar(); break;
	}
	cToken = temp; printed = 0;
	return temp;
}

Token checkGet(Token t, KeyId k)			/*　t.kind == k のチェック　*/
	/*　t.kind == k なら、次のトークンを読んで返す　*/
	/*　t.kind != k ならエラーメッセージを出し、t と k が共に記号、または予約語なら　*/
	/*　t を捨て、次のトークンを読んで返す（ t を k で置き換えたことになる）　*/
	/*　それ以外の場合、k を挿入したことにして、t を返す　*/
{
	if (t.kind==k)
			return nextToken();
	if ((isKeyWd(k) && isKeyWd(t.kind)) ||
		(isKeySym(k) && isKeySym(t.kind))){
			errorDelete();
			errorInsert(k);
			return nextToken();
	}
	errorInsert(k);
	return t;
}

static void printSpaces()			/*　空白や改行の印字　*/
{
	while (CR-- > 0)
		fprintf(fptex, "\n");
		/* fprintf(fptex, "\\ \\par\n"); */
	while (spaces-- > 0)
		fprintf(fptex, " ");
		/* fprintf(fptex, "\\ "); */
	CR = 0; spaces = 0;
}

void printcToken()				/*　現在のトークンの印字　*/
{
	int i=(int)cToken.kind;
	if (printed){
		printed = 0; return;
	}
	printed = 1;
	printSpaces();				/*　トークンの前の空白や改行印字　*/
	if (i < end_of_KeyWd) 						/*　予約語　*/
		fprintf(fptex, "<b>%s</b>", KeyWdT[i].word);
		/* fprintf(fptex, "{\\bf %s}", KeyWdT[i].word); */
	else if (i < end_of_KeySym)					/*　演算子か区切り記号　*/
		fprintf(fptex, "%s", KeyWdT[i].word);
		/* fprintf(fptex, "$%s$", KeyWdT[i].word); */
	else if (i==(int)Id){							/*　Identfier　*/
		switch (idKind) {
		case varId: 
			fprintf(fptex, "%s", cToken.u.id); return;
		case parId: 
			fprintf(fptex, "<i>%s</i>", cToken.u.id); return;
			/* fprintf(fptex, "{\\sl %s}", cToken.u.id); return; */
		case funcId: 
			fprintf(fptex, "<i>%s</i>", cToken.u.id); return;
			/* fprintf(fptex, "{\\it %s}", cToken.u.id); return; */
		case constId: 
			fprintf(fptex, "<tt>%s</tt>", cToken.u.id); return;
			/* fprintf(fptex, "{\\sf %s}", cToken.u.id); return; */
		}
	}else if (i==(int)Num)			/*　Num　*/
		fprintf(fptex, "%d", cToken.u.value);
}

void setIdKind (KindT k)			/*　現トークン(Id)の種類をセット　*/
{
	idKind = k;
}



