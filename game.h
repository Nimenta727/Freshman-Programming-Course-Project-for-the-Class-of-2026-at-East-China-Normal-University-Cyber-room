#ifndef GAME_H
#define GAME_H

#include "level.h"

/*
 * 游戏模块：负责按照脚本逐条执行命令，维护游戏状态，
 * 并判定 ESCAPED / FAILED。
 */

typedef enum {
    RESULT_NONE = 0,   /* 尚无最终结果 */
    RESULT_ESCAPED,    /* 成功逃脱 */
    RESULT_FAILED      /* 行动次数用完或脚本执行完仍未逃脱 */
} GameResult;

typedef struct {
    const Level *level;  /* 关卡（不接管其内存） */
    int row, col;        /* 玩家当前位置 */
    int actions;         /* 已消耗的行动次数 */
    int clue_found;      /* 是否已通过 inspect 取得开门密码 */
    int key_taken;       /* 是否已取得钥匙 */
    int door_opened;     /* 密码门是否已打开 */
    GameResult result;   /* 最终结果 */
} Game;

/* 初始化游戏状态：玩家位于 @，已用行动数为 0，密码/钥匙/门均为初始状态。 */
void game_init(Game *game, const Level *level);

/*
 * 从脚本文件逐条读取并执行命令，直到出现最终结果或脚本结束。
 * 空行与无法识别的命令不改变游戏状态、不消耗次数（会向终端提示）。
 */
void game_run_script(Game *game, const char *script_path);

/* 按固定格式把结果写入输出文件。返回 1 表示写入成功，0 表示失败。 */
int game_write_result(const Game *game, const char *output_path);

#endif /* GAME_H */
