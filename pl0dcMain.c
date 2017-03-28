/*
 * pl0dcMain.c
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#ifdef YACC
    extern FILE *yyin;
    int yylex(void);
    void yyerror(const char* s);
#   include "y.tab.c"
#else
#   include "getSource.h"
#endif

int compile(FILE *fout);

#ifdef YACC
void yyerror(const char* s) {
  fprintf(stderr, "error: %s\n", s);
}
#endif

static void printUsage(const char *argv0) {
#ifdef YACC
    printf("Usage: %s [-h] [-o <output_file>] <input_file>\n", argv0);
#else
    printf("Usage: %s [-g] [-h] [-o <output_file>] <input_file>\n", argv0);
#endif
}

int main(int argc, char *argv[]) {
    char outFileName[FILENAME_MAX] = "a.out"; /* 出力プログラムファイル名 */
    FILE *fout = NULL;
    int opt;
    /* コマンドラインオプションを解釈 */
#ifdef YACC
    while ((opt = getopt(argc, argv, "o:h")) != -1) {
        switch (opt) {
        case 'o': strcpy(outFileName, optarg); break; /* 出力ファイル名取得 */
        case 'h': printUsage(argv[0]); return 0;      /* ヘルプ表示 */
        default:  printUsage(argv[0]); return 1;      /* 無効なオプション */
        }
    }
#else
    int outHtml = 0;
    while ((opt = getopt(argc, argv, "o:gh")) != -1) {
        switch (opt) {
        case 'o': strcpy(outFileName, optarg); break; /* 出力ファイル名取得 */
        case 'g': outHtml = 1; break;
        case 'h': printUsage(argv[0]); return 0;      /* ヘルプ表示 */
        default:  printUsage(argv[0]); return 1;      /* 無効なオプション */
        }
    }
#endif
    if (optind >= argc) {
        fprintf(stderr, "%s: no input file\n", argv[0]);
        printUsage(argv[0]);
        return 1;
    }
    if ((fout = fopen(outFileName, "w")) == NULL) { /* 出力ファイルを作る */
        printf("can't open %s\n", outFileName);
        return 1;
    }
#ifdef YACC
    if ((yyin = fopen(argv[optind], "r")) == NULL) {
        printf("can't open %s\n", argv[optind]);
        return 1;
    }
    yyparse();
    listCode(fout, 0);
    fclose(yyin);
#else
    if (!openSource(argv[optind], outHtml)) /* ソースプログラムファイルのopen */
        return 1;               /* openに失敗すれば終わり               */
    compile(fout);              /* コンパイル */
    closeSource();              /* ソースプログラムファイルのclose      */
#endif
    fclose(fout);
    return 0;
}
