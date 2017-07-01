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

int main(int argc, char *argv[]) {
    FILE *fin = NULL;
    int opt, quit, update;
    int stepRun = 0; /* ステップ実行フラグ */
    int secretMode = 0, secretValue; /* シークレットモード用 */
    /* コマンドラインオプションを解釈 */
    while ((opt = getopt(argc, argv, "hsz")) != -1) {
        switch (opt) {
        case 'h': printUsage(argv[0]); return 0;      /* ヘルプ表示 */
        case 's': stepRun = 1; break;                 /* ステップ実行する */
        case 'z': secretMode = 1; break;              /* シークレットモード有効化 */
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
    if (readCode(fin)) return 1;
    if (secretMode) scanf("%d", &secretValue); /* シークレットモード用の値の読み込み */
    /* エラーがなければ実行 */
    if (!stepRun) { /* まとめて実行 */
        execute(secretMode ? &secretValue : NULL);
    } else { /* ステップ実行 */
        static Mem memory;
        initMemory(&memory, (secretMode ? &secretValue : NULL));
        quit = 0; /* 終了するか */
        update = 1; /* 更新があったか */
        while (!quit) {
          if (update) { /* 更新があった場合のみ表示 */
            if (memory.top > memory.topMax) memory.topMax = memory.top;
            update = 0;
            printMemory(&memory);
            if (memory.exitCode) { /* 終了状態 */
              /*printf("q: quit\n");*/
              quit = 1; continue;
            } else
              printf("[ret] (step), c (cont.), b[0-%d] (set bp), q (quit)\n", getCLength()-1);
          }
          switch (getchar()) {
          case 'b': 
            scanf("%d ", &memory.breakPoint);
            update = 1; break;
          case 'q': quit = 1; break; /* 終了 */
          case 'c':
            while (1) {
              stepForward(&memory); 
              if (memory.top > memory.topMax) memory.topMax = memory.top;
              printMemory(&memory);
              if (memory.exitCode) { /* 終了状態 */
                quit = 1; break;
              } else if (memory.stepCount == memory.breakPoint-1)
                break;
            }
            break;
          /*case 's':*/
          default:
            stepForward(&memory); update = 1; break;  /* 1 step 前進 */
          }
        }
        printf("[finished]\n");
    }
    fclose(fin);
    return 0;
}
