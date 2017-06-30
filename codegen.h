/*
 * codegen.h
 */

#define MAXCODE 200             /* 目的コードの最大長さ                 */
#define MAXMEM 2000             /* 実行時スタックの最大長さ             */
#define MAXREG 20               /* 演算レジスタスタックの最大長さ       */
#define MAXLEVEL 5              /* ブロックの最大深さ                   */
#define MAXSCREEN 512           /* print用バッファサイズ */

typedef enum codes {            /* 命令語のコード                       */
    lit, opr, lod, sto, cal, ret, ict, jmp, jpc
} OpCode;

typedef enum ops {              /* 演算命令のコード                     */
    neg, add, sub, mul, div, odd, eq, ls, gr,
    neq, lseq, greq, rd, wrt, wrl
} Operator;

typedef struct {
  int stack[MAXMEM];          /* 実行時スタック */
  int display[MAXLEVEL];      /* 現在見える各ブロックの先頭番地のディスプレイ */
  int pc;                     /* 命令語のカウンタ */
  int top;                    /* 次にスタックに入れる場所 */
  int exitCode;               /* 終了状態かどうか */
  int stepCount;              /* 現在のステップ数 */
  char screen[MAXSCREEN];     /* print用バッファ */
  int secretMode;             /* シークレットモード用 */
  int secretValue;            /* シークレットモード用 */
} Mem;

int genCodeV(OpCode op, int v); /* 命令語の生成、アドレス部にv          */
int genCodeT(OpCode op, int ti); /* 命令語の生成、アドレスは名前表から  */
int genCodeO(Operator p);       /* 命令語の生成、アドレス部に演算命令   */
int genCodeR();                 /* ret命令語の生成                      */
void backPatch(int i);          /* 命令語のバックパッチ（次の番地を）   */

int nextCode();                 /* 次の命令語のアドレスを返す           */
void listCode(FILE *fp, int showLineNumber); /* 目的コード（命令語）のリスティング */
int readCode(FILE *fp);         /* 目的コード（命令語）の読み込み */

void initMemory(Mem *m, const int *secretValue);
void printMemory(const Mem *m);
void stepForward(Mem *m);
void stepBackward(Mem *m);

void execute(const int *secretValue);  /* 目的コード（命令語）の実行           */
