/*
 * getSource.h
 */

#include <stdio.h>
#ifndef TBL
#   define TBL
#   include "table.h"
#endif

#define MAXNAME 31              /* 名前の最大長さ                       */

typedef enum keys {             /* キーや文字の種類（名前）             */
    Begin,                      /* 予約語の名前                         */
    End,
    If,
    Then,
    While,
    Do,
    Ret,
    Func,
    Var,
    Const,
    Odd,
    Write,
    WriteLn,
    end_of_KeyWd,               /* 予約語の名前はここまで               */
    Plus,                       /* 演算子と区切り記号の名前             */
    Minus,
    Mult,
    Div,
    Lparen,
    Rparen,
    Equal,
    Lss,
    Gtr,
    NotEq,
    LssEq,
    GtrEq,
    Comma,
    Period,
    Semicolon,
    Assign,
    end_of_KeySym,              /* 演算子と区切り記号の名前はここまで   */
    Id,                         /* トークンの種類                       */
    Num,
    nul,
    end_of_Token,
    letter,                     /* 上記以外の文字の種類                 */
    digit,
    colon,
    others
} KeyId;

typedef struct token {          /* トークンの型                         */
    KeyId kind;                 /* トークンの種類かキーの名前           */
    union {
        char id[MAXNAME];       /* Identfierの時、その名前              */
        int value;              /* Numの時、その値                      */
    } u;
} Token;

Token nextToken();              /* 次のトークンを読んで返す             */
Token checkGet(Token t, KeyId k); /* t.kind==kのチェック                */
                                /* t.kind==kなら、次のトークンを読んで返す */
                                /* t.kind!=kならエラーメッセージを出し、 */
                                /* tとkが共に記号、または予約語なら     */
                                /* tを捨て、次のトークンを読んで返す    */
                                /* （tをkで置き換えたことになる）       */
                                /* それ以外の場合、kを挿入したことにして、tを返す */

int openSource(char fileName[], int outHtml); /* ソースファイルのopen */
void closeSource();             /* ソースファイルのclose                */
void initSource();              /* テーブルの初期設定、texファイルの初期設定 */
void finalSource();             /* ソースの最後のチェック、texファイルの最終設定 */
void errorType(char *m);        /* 型エラーを.texファイルに出力         */
void errorInsert(KeyId k);      /* keyString(k)を.texファイルに挿入     */
void errorMissingId();          /* 名前がないとのメッセージを.texファイルに挿入 */
void errorMissingOp();          /* 演算子がないとのメッセージを.texファイルに挿入 */
void errorDelete();             /* 今読んだトークンを読み捨て（.texファイルに出力）*/
void errorMessage(char *m);     /* エラーメッセージを.texファイルに出力 */
void errorF(char *m);           /* エラーメッセージを出力し、コンパイル終了 */
int errorN();                   /* エラーの個数を返す                   */

void setIdKind(KindT k);        /* 現トークン(Id)の種類をセット（.texファイル出力のため）*/
