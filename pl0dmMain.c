/**
 * @file pl0dmMain.c
 */

#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include "codegen.h"
#include "getSource.h"

static void printUsage(const char *argv0) {
    printf("Usage: %s [-hs] <input_file>\n", argv0);
}

static Mem memory;

int main(int argc, char *argv[]) {
    FILE *fin = NULL;
    int opt, quit;
    int stepRun = 0; /* ステップ実行フラグ */
    /* コマンドラインオプションを解釈 */
    while ((opt = getopt(argc, argv, "hs")) != -1) {
        switch (opt) {
        case 'h': printUsage(argv[0]); return 0;      /* ヘルプ表示 */
        case 's': stepRun = 1; break;                 /* ステップ実行する */
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
    /* エラーがなければ実行 */
    if (!stepRun) { /* まとめて実行 */
      execute();
    } else { /* ステップ実行 */
      initMemory(&memory);
      quit = 0;
      while (!quit) {
        printMemory(&memory);
        if (memory.exitCode)
          printf("b: step backward, q: quit\n");
        else if (!memory.stepCount)
          printf("f: step forward, q: quit\n");
        else
          printf("f: step forward, b: step backward, q: quit\n");
        switch (getchar()) {
        case 'f': stepForward(&memory); break;
        case 'b': stepBackward(&memory); break;
        case 'q': quit = 1; break;
        default: break;
        }
      }
    }
    fclose(fin);
    return 0;
}
