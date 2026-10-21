#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void game_init(Game *game, const Level *level)
{
    game->level = level;
    game->row = level->start_r;
    game->col = level->start_c;
    game->actions = 0;
    game->clue_found = 0;
    game->key_taken = 0;
    game->door_opened = 0;
    game->result = RESULT_NONE;
}

/* 每次消耗行动后检查结束条件：优先判断 ESCAPED，其次判断行动数是否达到上限。 */
static void check_after_action(Game *game, int entered_exit)
{
    if (entered_exit) {
        game->result = RESULT_ESCAPED;
    } else if (game->actions >= game->level->turns) {
        game->result = RESULT_FAILED;
    }
}

/*
 * 目标格是否可进入：
 *   地图外或 '#' 墙：不可进入；
 *   'D'（密码门）：门打开后才可进入；
 *   'E'（出口）  ：门打开后才可进入，进入即逃脱；
 *   其余（'.' 'C' 'K' 及已打开的 'D'）：可进入。
 */
static int target_cell(const Game *game, int r, int c, int *is_exit)
{
    const Level *lv = game->level;

    *is_exit = 0;
    if (r < 0 || r >= lv->rows || c < 0 || c >= lv->cols) {
        return '#'; /* 地图外按墙处理 */
    }
    return lv->grid[r][c];
}

static void do_move(Game *game, int dr, int dc)
{
    const Level *lv = game->level;
    int r = game->row + dr;
    int c = game->col + dc;
    int is_exit = 0;
    char cell;

    if (r < 0 || r >= lv->rows || c < 0 || c >= lv->cols) {
        printf("move：目标是地图外，行动失败，不消耗次数。\n");
        return;
    }
    cell = target_cell(game, r, c, &is_exit);
    (void)is_exit;
    if (cell == '#') {
        printf("move：目标是墙，行动失败，不消耗次数。\n");
        return;
    }
    if ((cell == 'D' || cell == 'E') && !game->door_opened) {
        printf("move：目标被未打开的密码门挡住，行动失败，不消耗次数。\n");
        return;
    }

    game->row = r;
    game->col = c;
    game->actions++;

    if (cell == 'E') {
        /* 已进入出口，立刻判定 ESCAPED（该次移动消耗行动数）。 */
        check_after_action(game, 1);
    } else {
        check_after_action(game, 0);
    }
}

static void do_inspect(Game *game)
{
    const Level *lv = game->level;

    if (game->row == lv->clue_r && game->col == lv->clue_c && !game->clue_found) {
        game->clue_found = 1;
        game->actions++;
        printf("inspect：调查线索成功，取得开门密码。\n");
        check_after_action(game, 0);
    } else {
        printf("inspect：此处没有可调查的线索（或已调查过），不消耗次数。\n");
    }
}

static void do_take(Game *game)
{
    const Level *lv = game->level;

    if (game->row == lv->key_r && game->col == lv->key_c && !game->key_taken) {
        game->key_taken = 1;
        game->actions++;
        printf("take：拾取钥匙成功。\n");
        check_after_action(game, 0);
    } else {
        printf("take：此处没有可拾取的钥匙（或已拾取过），不消耗次数。\n");
    }
}

/* 是否与 D 上下左右相邻。 */
static int adjacent_to_door(const Game *game)
{
    const Level *lv = game->level;
    int dr = game->row - lv->door_r;
    int dc = game->col - lv->door_c;

    return (dr == 0 && (dc == 1 || dc == -1)) || (dc == 0 && (dr == 1 || dr == -1));
}

static void do_open(Game *game)
{
    if (adjacent_to_door(game) && game->clue_found && game->key_taken &&
        !game->door_opened) {
        game->door_opened = 1;
        game->actions++;
        printf("open：密码门已打开。\n");
        check_after_action(game, 0);
    } else {
        printf("open：条件不满足（需相邻、已取得密码和钥匙、门未开），不消耗次数。\n");
    }
}

/* 去掉命令行首尾的空白字符与行尾换行符。 */
static char *trim(char *line)
{
    char *start = line;
    char *end;

    while (*start == ' ' || *start == '\t') {
        start++;
    }
    end = start + strlen(start);
    while (end > start &&
           (end[-1] == '\n' || end[-1] == '\r' || end[-1] == ' ' || end[-1] == '\t')) {
        *--end = '\0';
    }
    return start;
}

static void run_command(Game *game, const char *cmd)
{
    if (strcmp(cmd, "move up") == 0) {
        do_move(game, -1, 0);
    } else if (strcmp(cmd, "move down") == 0) {
        do_move(game, 1, 0);
    } else if (strcmp(cmd, "move left") == 0) {
        do_move(game, 0, -1);
    } else if (strcmp(cmd, "move right") == 0) {
        do_move(game, 0, 1);
    } else if (strcmp(cmd, "inspect") == 0) {
        do_inspect(game);
    } else if (strcmp(cmd, "take") == 0) {
        do_take(game);
    } else if (strcmp(cmd, "open") == 0) {
        do_open(game);
    } else {
        /* 不属于规定格式的命令：不改变状态、不消耗次数，仅提示。 */
        printf("提示：忽略无法识别的命令 \"%s\"。\n", cmd);
    }
}

void game_run_script(Game *game, const char *script_path)
{
    FILE *fp;
    char line[256];

    fp = fopen(script_path, "r");
    if (fp == NULL) {
        fprintf(stderr, "错误：无法打开脚本文件 %s。\n", script_path);
        game->result = RESULT_FAILED;
        return;
    }

    while (game->result == RESULT_NONE && fgets(line, sizeof(line), fp) != NULL) {
        char *cmd = trim(line);

        if (*cmd == '\0') {
            continue; /* 空行忽略 */
        }
        run_command(game, cmd);
    }
    fclose(fp);

    /* 脚本执行完仍未逃脱，判定 FAILED。 */
    if (game->result == RESULT_NONE) {
        game->result = RESULT_FAILED;
    }
}

int game_write_result(const Game *game, const char *output_path)
{
    FILE *fp = fopen(output_path, "w");

    if (fp == NULL) {
        fprintf(stderr, "错误：无法写入结果文件 %s。\n", output_path);
        return 0;
    }
    fprintf(fp, "RESULT %s\n", game->result == RESULT_ESCAPED ? "ESCAPED" : "FAILED");
    fprintf(fp, "ACTIONS %d\n", game->actions);
    fprintf(fp, "CLUE_FOUND %d\n", game->clue_found);
    fprintf(fp, "KEY_TAKEN %d\n", game->key_taken);
    fprintf(fp, "DOOR_OPENED %d\n", game->door_opened);
    fclose(fp);
    return 1;
}
