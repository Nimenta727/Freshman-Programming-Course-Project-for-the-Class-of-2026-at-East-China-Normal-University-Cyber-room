#ifndef LEVEL_H
#define LEVEL_H

/*
 * 关卡模块：负责读取并校验关卡文件（level.txt）。
 *
 * 关卡文件格式（纯文本，固定格式）：
 *   SIZE 行数 列数        （行数 3~20，列数 3~40）
 *   TURNS 最大行动次数   （1~100）
 *   MAP
 *   地图第1行
 *   ...
 *   地图第N行
 *   ENDMAP
 *
 * 地图字符：'#' 墙，'.' 空地，'@' 玩家起点，'C' 线索，
 *           'K' 钥匙，'D' 密码门，'E' 出口。
 * '@'、'C'、'K'、'D'、'E' 各恰好出现一次。
 */

#define MIN_ROWS 3
#define MAX_ROWS 20
#define MIN_COLS 3
#define MAX_COLS 40
#define MIN_TURNS 1
#define MAX_TURNS 100

typedef struct {
    int rows;      /* 地图行数 */
    int cols;      /* 地图列数 */
    int turns;     /* 最大行动次数 */
    char **grid;   /* 地图内容，grid[r][c]，坐标从左上角 (0,0) 开始 */
    int start_r;   /* 玩家起点 @ 的行 */
    int start_c;   /* 玩家起点 @ 的列 */
    int clue_r;    /* 线索 C 的行 */
    int clue_c;    /* 线索 C 的列 */
    int key_r;     /* 钥匙 K 的行 */
    int key_c;     /* 钥匙 K 的列 */
    int door_r;    /* 密码门 D 的行 */
    int door_c;    /* 密码门 D 的列 */
    int exit_r;    /* 出口 E 的行 */
    int exit_c;    /* 出口 E 的列 */
} Level;

/*
 * 从文件中读取并校验关卡。
 * 成功返回 1，并通过 level 返回关卡数据（调用方负责 level_free）；
 * 失败返回 0，并向 stderr 输出错误原因。
 */
int level_load(const char *path, Level *level);

/* 释放关卡占用的内存。 */
void level_free(Level *level);

#endif /* LEVEL_H */
