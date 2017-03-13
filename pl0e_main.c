/*
 * pl0e_main.c
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "codegen.h"
#include "getSource.h"

static void printUsage(const char *argv0) {
    printf("Usage: %s [-h] <input_file>\n", argv0);
}

int main(int argc, char *argv[]) {
    FILE *fin = NULL;
    int opt;
    /* コマンドラインオプションを解釈 */
    while ((opt = getopt(argc, argv, "h")) != -1) {
        switch (opt) {
        case 'h': printUsage(argv[0]); return 0;      /* ヘルプ表示 */
        default:  printUsage(argv[0]); return 1;      /* 無効なオプション */
        }
    }
    if (optind >= argc) {
        fprintf(stderr, "%s: no input file\n", argv[0]);
        printUsage(argv[0]);
        return 1;
    }
    if ((fin = fopen(argv[optind], "r")) == NULL) { /* 命令語ファイルの読み込み */
        printf("can't open %s\n", argv[optind]);
        return 1;
    }
    if (readCode(fin))
        return 1;
    /* listCode(stdout, 1); */
    execute(); /* エラーがなければ実行 */
    fclose(fin);
    return 0;
}
