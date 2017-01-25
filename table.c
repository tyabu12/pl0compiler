
   /*********table.c**********/
   
#ifndef TBL
#define TBL
#include "table.h"
#endif
#include "getSource.h"

#define MAXTABLE 100		/*　名前表の最大長さ　*/
#define MAXNAME  31		/*　名前の最大長さ　*/
#define MAXLEVEL 5		/*　ブロックの最大深さ　*/

typedef struct tableE {		/*　名前表のエントリーの型　*/
	KindT kind;			/*　名前の種類　*/
	char name[MAXNAME];	/*　名前のつづり　*/
	union {
		int value;			/*　定数の場合：値　*/
		struct {
			RelAddr raddr;	/*　関数の場合：先頭アドレス　*/
			int pars;		/*　関数の場合：パラメタ数　*/
		}f;
		RelAddr raddr;		/*　変数、パラメタの場合：アドレス　*/
	}u;
}TabelE;

static TabelE nameTable[MAXTABLE];		/*　名前表　*/
static int tIndex = 0;			/*　名前表のインデックス　*/
static int level = -1;			/*　現在のブロックレベル　*/
static int index[MAXLEVEL];   	/*　index[i]にはブロックレベルiの最後のインデックス　*/
static int addr[MAXLEVEL];    	/*　addr[i]にはブロックレベルiの最後の変数の番地　*/
static int localAddr;			/*　現在のブロックの最後の変数の番地　*/
static int tfIndex;

static char* kindName(KindT k)		/*　名前の種類の出力用関数　*/
{
	switch (k){
	case varId: return "var";
	case parId: return "par";
	case funcId: return "func";
	case constId: return "const";
	}
}

void blockBegin(int firstAddr)	/*　ブロックの始まり(最初の変数の番地)で呼ばれる　*/
{
	if (level == -1){			/*　主ブロックの時、初期設定　*/
		localAddr = firstAddr;
		tIndex = 0;
		level++;
		return;
	}
	if (level == MAXLEVEL-1)
		errorF("too many nested blocks");
	index[level] = tIndex;		/*　今までのブロックの情報を格納　*/
	addr[level] = localAddr;
	localAddr = firstAddr;		/*　新しいブロックの最初の変数の番地　*/
	level++;				/*　新しいブロックのレベル　*/
	return;
}

void blockEnd()				/*　ブロックの終りで呼ばれる　*/
{
	level--;
	tIndex = index[level];		/*　一つ外側のブロックの情報を回復　*/
	localAddr = addr[level];
}

int bLevel()				/*　現ブロックのレベルを返す　*/
{
	return level;
}

int fPars()					/*　現ブロックの関数のパラメタ数を返す　*/
{
	return nameTable[index[level-1]].u.f.pars;
}

void enterT(char *id)			/*　名前表に名前を登録　*/
{
	if (tIndex++ < MAXTABLE){
		strcpy(nameTable[tIndex].name, id);
	} else 
		errorF("too many names");
}
		
int enterTfunc(char *id, int v)		/*　名前表に関数名と先頭番地を登録　*/
{
	enterT(id);
	nameTable[tIndex].kind = funcId;
	nameTable[tIndex].u.f.raddr.level = level;
	nameTable[tIndex].u.f.raddr.addr = v;  		 /*　関数の先頭番地　*/
	nameTable[tIndex].u.f.pars = 0;  			 /*　パラメタ数の初期値　*/
	tfIndex = tIndex;
	return tIndex;
}

int enterTpar(char *id)				/*　名前表にパラメタ名を登録　*/
{
	enterT(id);
	nameTable[tIndex].kind = parId;
	nameTable[tIndex].u.raddr.level = level;
	nameTable[tfIndex].u.f.pars++;  		 /*　関数のパラメタ数のカウント　*/
	return tIndex;
}

int enterTvar(char *id)			/*　名前表に変数名を登録　*/
{
	enterT(id);
	nameTable[tIndex].kind = varId;
	nameTable[tIndex].u.raddr.level = level;
	nameTable[tIndex].u.raddr.addr = localAddr++;
	return tIndex;
}

int enterTconst(char *id, int v)		/*　名前表に定数名とその値を登録　*/
{
	enterT(id);
	nameTable[tIndex].kind = constId;
	nameTable[tIndex].u.value = v;
	return tIndex;
}

void endpar()					/*　パラメタ宣言部の最後で呼ばれる　*/
{
	int i;
	int pars = nameTable[tfIndex].u.f.pars;
	if (pars == 0)  return;
	for (i=1; i<=pars; i++)			/*　各パラメタの番地を求める　*/
		 nameTable[tfIndex+i].u.raddr.addr = i-1-pars;
}

void changeV(int ti, int newVal)		/*　名前表[ti]の値（関数の先頭番地）の変更　*/
{
	nameTable[ti].u.f.raddr.addr = newVal;
}

int searchT(char *id, KindT k)		/*　名前idの名前表の位置を返す　*/
							/*　未宣言の時エラーとする　*/
{
	int i;
	i = tIndex;
	strcpy(nameTable[0].name, id);			/*　番兵をたてる　*/
	while( strcmp(id, nameTable[i].name) )
		i--;
	if ( i )							/*　名前があった　*/
		return i;
	else {							/*　名前がなかった　*/
		errorType("undef");
		if (k==varId) return enterTvar(id);	/*　変数の時は仮登録　*/
		return 0;
	}
}

KindT kindT(int i)				/*　名前表[i]の種類を返す　*/
{
	return nameTable[i].kind;
}

RelAddr relAddr(int ti)				/*　名前表[ti]のアドレスを返す　*/
{
	return nameTable[ti].u.raddr;
}

int val(int ti)					/*　名前表[ti]のvalueを返す　*/
{
	return nameTable[ti].u.value;
}

int pars(int ti)				/*　名前表[ti]の関数のパラメタ数を返す　*/
{
	return nameTable[ti].u.f.pars;
}

int frameL()				/*　そのブロックで実行時に必要とするメモリー容量　*/
{
	return localAddr;
}

