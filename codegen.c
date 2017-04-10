/**
 * @file codegen.c
 */

#include <stdio.h>
#include <string.h>
#include "codegen.h"
#ifndef TBL
#   define TBL
#   include "table.h"
#endif
#include "getSource.h"

#define MAXCODE 200             /* 目的コードの最大長さ                 */
#define MAXMEM 2000             /* 実行時スタックの最大長さ             */
#define MAXREG 20               /* 演算レジスタスタックの最大長さ       */
#define MAXLEVEL 5              /* ブロックの最大深さ                   */

typedef struct inst {           /* 命令語の型                           */
    OpCode opCode;
    union {
        RelAddr addr;
        int value;
        Operator optr;
    } u;
} Inst;

static Inst code[MAXCODE];      /* 目的コードが入る                     */
static int cIndex = -1;         /* 最後に生成した命令語のインデックス   */
static void checkMax();         /* 目的コードのインデックスの増加とチェック */
static void printCode(FILE *fp, int i); /* 命令語の印字 */

int nextCode() {                /* 次の命令語のアドレスを返す           */
    return cIndex+1;
}

int genCodeV(OpCode op, int v) { /* 命令語の生成、アドレス部にv         */
    checkMax();
    code[cIndex].opCode = op;
    code[cIndex].u.value = v;
    return cIndex;
}

int genCodeT(OpCode op, int ti) { /* 命令語の生成、アドレスは名前表から */
    checkMax();
    code[cIndex].opCode = op;
    code[cIndex].u.addr = relAddr(ti);
    return cIndex;
}

int genCodeO(Operator p) {      /* 命令語の生成、アドレス部に演算命令    */
    checkMax();
    code[cIndex].opCode = opr;
    code[cIndex].u.optr = p;
    return cIndex;
}

int genCodeR() {                /* ret命令語の生成                      */
    if (code[cIndex].opCode == ret)
        return cIndex;          /* 直前がretなら生成せず                */
    checkMax();
    code[cIndex].opCode = ret;
    code[cIndex].u.addr.level = bLevel();
    code[cIndex].u.addr.addr = fPars(); /* パラメタ数（実行スタックの解放用） */
    return cIndex;
}

void checkMax() {               /* 目的コードのインデックスの増加とチェック */
    if (++cIndex < MAXCODE)
        return;
    errorF("too many code");
}

void backPatch(int i) {         /* 命令語のバックパッチ（次の番地を）   */
    code[i].u.value = cIndex+1;
}

void listCode(FILE *fp, int showLineNumber) { /* 命令語のリスティング */
    int i;
    if (fp == NULL) {
      fp = stdin;
      fprintf(stdout, "\ncode\n");
    }
    for (i = 0; i <= cIndex; i++) {
        if (showLineNumber) fprintf(fp, "%3d: ", i);
        printCode(fp, i);
    }
}

void printCode(FILE *fp, int i) { /* 命令語の印字 */
    int flag;
    switch (code[i].opCode) {
    case lit: fprintf(fp, "lit"); flag=1; break;
    case opr: fprintf(fp, "opr"); flag=3; break;
    case lod: fprintf(fp, "lod"); flag=2; break;
    case sto: fprintf(fp, "sto"); flag=2; break;
    case cal: fprintf(fp, "cal"); flag=2; break;
    case ret: fprintf(fp, "ret"); flag=2; break;
    case ict: fprintf(fp, "ict"); flag=1; break;
    case jmp: fprintf(fp, "jmp"); flag=1; break;
    case jpc: fprintf(fp, "jpc"); flag=1; break;
    }
    switch (flag) {
    case 1:
        fprintf(fp, ",%d\n", code[i].u.value);
        return;
    case 2:
        fprintf(fp, ",%d", code[i].u.addr.level);
        fprintf(fp, ",%d\n", code[i].u.addr.addr);
        return;
    case 3:
        switch (code[i].u.optr) {
        case neg: fprintf(fp, ",neg\n"); return;
        case add: fprintf(fp, ",add\n"); return;
        case sub: fprintf(fp, ",sub\n"); return;
        case mul: fprintf(fp, ",mul\n"); return;
        case div: fprintf(fp, ",div\n"); return;
        case odd: fprintf(fp, ",odd\n"); return;
        case eq: fprintf(fp, ",eq\n"); return;
        case ls: fprintf(fp, ",ls\n"); return;
        case gr: fprintf(fp, ",gr\n"); return;
        case neq: fprintf(fp, ",neq\n"); return;
        case lseq: fprintf(fp, ",lseq\n"); return;
        case greq: fprintf(fp, ",greq\n"); return;
        case rd: fprintf(fp, ",rd\n"); return;
        case wrt: fprintf(fp, ",wrt\n"); return;
        case wrl: fprintf(fp, ",wrl\n"); return;
        }
    }
}

/* 命令語ファイルの読み取り */
int readCode(FILE *fp) {
struct {
  const char *name;
  OpCode opCode;
  int flag;
} static const opTbl[] = {
  { "lit", lit, 1 },
  { "opr", opr, 3 },
  { "lod", lod, 2 },
  { "sto", sto, 2 },
  { "cal", cal, 2 },
  { "ret", ret, 2 },
  { "ict", ict, 1 },
  { "jmp", jmp, 1 },
  { "jpc", jpc, 1 },
  { NULL, } /* 番兵 */
};
struct {
  const char *name;
  Operator optr;
} static const optrTbl[] = {
  { "neg",  neg  },
  { "add",  add  },
  { "sub",  sub  },
  { "mul",  mul  },
  { "div",  div  },
  { "odd",  odd  },
  { "eq",   eq   },
  { "lseq", lseq },
  { "ls",   ls   },
  { "greq", greq },
  { "gr",   gr   },
  { "neq",  neq  },
  { "rd",   rd   },
  { "wrt",  wrt  },
  { "wrl",  wrl  },
  { NULL, } /* 番兵 */
};
    char str[256];
    char *p;
    int op, optr, i;
    for (i = 0; fgets(str, 256, fp) != NULL; i++) {
        p = str;
        /* 行番号のスキップ */
        while (*p != ':' && *p != '\0') p++;
        if (*p == ':') p++;
        while (*p == ' ' && *p != '\0') p++;
        if (*p == '\0') return 1;
        /* 命令語をテーブルから検索 */
        for (op = 0; opTbl[op].name != NULL; op++) {
            if (strncmp(opTbl[op].name, p, 3) == 0)
                break;
        }
        if (opTbl[op].name == NULL) {
            fprintf(stderr, "Unknown op: %s\n", str);
            return 1;
        }
        code[i].opCode = opTbl[op].opCode;
        /* 命令語の形式毎に処理 */
        switch (opTbl[op].flag) {
        case 1:
            sscanf(p+4, "%d", &code[i].u.value);
            break;
        case 2:
            sscanf(p+4, "%d,%d",
                &code[i].u.addr.level, &code[i].u.addr.addr);
            break;
        case 3:
            for (optr = 0; optrTbl[optr].name != NULL; optr++) {
                if (strncmp(optrTbl[optr].name, p+4,
                  strlen(optrTbl[optr].name)) == 0)
                    break;
            }
            if (optrTbl[optr].name == NULL) {
                fprintf(stderr, "Unknown operator: %s\n", str);
                return 1;
            }
            code[i].u.optr = optrTbl[optr].optr;
            break;
        }
    }
    cIndex = i;
    return 0;
}

void execute() {                /* 目的コード（命令語）の実行           */
    int stack[MAXMEM];          /* 実行時スタック                       */
    int display[MAXLEVEL];      /* 現在見える各ブロックの先頭番地のディスプレイ */
    int pc, top, lev, temp;
    Inst i;                     /* 実行する命令語                       */
    /* printf("start execution\n"); */
    top = 0;  pc = 0;           /* top:次にスタックに入れる場所、pc:命令語のカウンタ */
    stack[0] = 0;  stack[1] = 0; /* stack[top]はcalleeで壊すディスプレイの退避場所 */
                                /* stack[top+1]はcallerへの戻り番地     */
    display[0] = 0;             /* 主ブロックの先頭番地は 0             */
    do {
        i = code[pc++];         /* これから実行する命令語               */
        switch (i.opCode) {
        case lit: stack[top++] = i.u.value;
            break;
        case lod: stack[top++] = stack[display[i.u.addr.level] + i.u.addr.addr];
            break;
        case sto: stack[display[i.u.addr.level] + i.u.addr.addr] = stack[--top];
            break;
        case cal: lev = i.u.addr.level +1; /* i.u.addr.levelはcalleeの名前のレベル */
                                /* calleeのブロックのレベルlevはそれに＋１したもの */
            stack[top] = display[lev]; /* display[lev]の退避            */
            stack[top+1] = pc; display[lev] = top;
                                /* 現在のtopがcalleeのブロックの先頭番地 */
            pc = i.u.addr.addr;
            break;
        case ret: temp = stack[--top]; /* スタックのトップにあるものが返す値 */
            top = display[i.u.addr.level]; /* topを呼ばれたときの値に戻す */
            display[i.u.addr.level] = stack[top]; /* 壊したディスプレイの回復 */
            pc = stack[top+1];
            top -= i.u.addr.addr; /* 実引数の分だけトップを戻す         */
            stack[top++] = temp; /* 返す値をスタックのトップへ          */
            break;
        case ict: top += i.u.value;
            if (top >= MAXMEM-MAXREG)
                errorF("stack overflow");
            break;
        case jmp: pc = i.u.value; break;
        case jpc: if (stack[--top] == 0) pc = i.u.value;
            break;
        case opr:
            switch (i.u.optr) {
            case neg: stack[top-1] = -stack[top-1]; continue;
            case add: --top;  stack[top-1] += stack[top]; continue;
            case sub: --top; stack[top-1] -= stack[top]; continue;
            case mul: --top;  stack[top-1] *= stack[top];  continue;
            case div: --top;  stack[top-1] /= stack[top]; continue;
            case odd: stack[top-1] = stack[top-1] & 1; continue;
            case eq: --top;  stack[top-1] = (stack[top-1] == stack[top]); continue;
            case ls: --top;  stack[top-1] = (stack[top-1] < stack[top]); continue;
            case gr: --top;  stack[top-1] = (stack[top-1] > stack[top]); continue;
            case neq: --top;  stack[top-1] = (stack[top-1] != stack[top]); continue;
            case lseq: --top;  stack[top-1] = (stack[top-1] <= stack[top]); continue;
            case greq: --top;  stack[top-1] = (stack[top-1] >= stack[top]); continue;
            case rd: scanf("%d", &stack[top++]); continue;
            case wrt: printf("%d ", stack[--top]); continue;
            case wrl: printf("\n"); continue;
            }
        }
    } while (pc != 0);
}
