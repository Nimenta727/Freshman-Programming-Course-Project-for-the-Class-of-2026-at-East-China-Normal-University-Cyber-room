#include "level.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* 去掉行尾的换行符与回车符，返回处理后的行。 */
static char *strip_eol(char *line)
{
    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[--len] = '\0';
    }
    return line;
}

/*
 * 解析一行中的整数：要求整行恰好是 "NAME n1 n2"（SIZE）或 "NAME n"（TURNS）。
 * 返回 1 表示解析成功，0 表示格式非法。
 */
static int parse_size_line(const char *line, int *rows, int *cols)
{
    char name[32];
    char extra[8];
    int r, c;

    if (sscanf(line, " %31s", name) != 1 || strcmp(name, "SIZE") != 0) {
        return 0;
    }
    extra[0] = '\0';
    if (sscanf(line, " %*s %d %d %7s", &r, &c, extra) != 2 || extra[0] != '\0') {
        return 0;
    }
    if (r < MIN_ROWS || r > MAX_ROWS || c < MIN_COLS || c > MAX_COLS) {
        fprintf(stderr, "错误：SIZE 超出范围（行数 %d~%d，列数 %d~%d）。\n",
                MIN_ROWS, MAX_ROWS, MIN_COLS, MAX_COLS);
        return 0;
    }
    *rows = r;
    *cols = c;
    return 1;
}

static int parse_turns_line(const char *line, int *turns)
{
    char name[32];
    char extra[8];
    int t;

    if (sscanf(line, " %31s", name) != 1 || strcmp(name, "TURNS") != 0) {
        return 0;
    }
    extra[0] = '\0';
    if (sscanf(line, " %*s %d %7s", &t, extra) != 1 || extra[0] != '\0') {
        return 0;
    }
    if (t < MIN_TURNS || t > MAX_TURNS) {
        fprintf(stderr, "错误：TURNS 超出范围（%d~%d）。\n", MIN_TURNS, MAX_TURNS);
        return 0;
    }
    *turns = t;
    return 1;
}

/* 记录特殊格子的位置，并检查 '@' 'C' 'K' 'D' 'E' 各恰好出现一次。 */
static int track_cell(Level *level, int counts[5], char ch, int r, int c)
{
    int idx;

    switch (ch) {
    case '@': idx = 0; level->start_r = r; level->start_c = c; break;
    case 'C': idx = 1; level->clue_r = r;  level->clue_c = c;  break;
    case 'K': idx = 2; level->key_r = r;   level->key_c = c;   break;
    case 'D': idx = 3; level->door_r = r;  level->door_c = c;  break;
    case 'E': idx = 4; level->exit_r = r;  level->exit_c = c;  break;
    default:  return 1; /* '#' 或 '.'，无需记录 */
    }
    counts[idx]++;
    return 1;
}

int level_load(const char *path, Level *level)
{
    FILE *fp;
    char line[1024];
    int counts[5] = {0, 0, 0, 0, 0};
    int rows = 0, cols = 0, turns = 0;
    int r, c;

    memset(level, 0, sizeof(*level));

    fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "错误：无法打开关卡文件 %s。\n", path);
        return 0;
    }

    /* 第一行：SIZE 行数 列数 */
    if (fgets(line, sizeof(line), fp) == NULL ||
        !parse_size_line(strip_eol(line), &rows, &cols)) {
        fprintf(stderr, "错误：关卡文件缺少合法的 SIZE 行。\n");
        fclose(fp);
        return 0;
    }

    /* 第二行：TURNS 最大行动次数 */
    if (fgets(line, sizeof(line), fp) == NULL ||
        !parse_turns_line(strip_eol(line), &turns)) {
        fprintf(stderr, "错误：关卡文件缺少合法的 TURNS 行。\n");
        fclose(fp);
        return 0;
    }

    /* 第三行：MAP */
    if (fgets(line, sizeof(line), fp) == NULL ||
        strcmp(strip_eol(line), "MAP") != 0) {
        fprintf(stderr, "错误：关卡文件缺少 MAP 行。\n");
        fclose(fp);
        return 0;
    }

    /* 读取地图 */
    level->grid = malloc(sizeof(char *) * (size_t)rows);
    if (level->grid == NULL) {
        fprintf(stderr, "错误：内存不足。\n");
        fclose(fp);
        return 0;
    }
    for (r = 0; r < rows; r++) {
        char *row;

        if (fgets(line, sizeof(line), fp) == NULL) {
            fprintf(stderr, "错误：地图行数不足，应为 %d 行。\n", rows);
            fclose(fp);
            level->rows = r; /* 只释放已分配的部分 */
            level_free(level);
            return 0;
        }
        row = strip_eol(line);
        if ((int)strlen(row) != cols) {
            fprintf(stderr, "错误：地图第 %d 行列数不符，应为 %d 列。\n", r + 1, cols);
            fclose(fp);
            level->rows = r;
            level_free(level);
            return 0;
        }
        level->grid[r] = malloc(sizeof(char) * (size_t)(cols + 1));
        if (level->grid[r] == NULL) {
            fprintf(stderr, "错误：内存不足。\n");
            fclose(fp);
            level->rows = r;
            level_free(level);
            return 0;
        }
        for (c = 0; c < cols; c++) {
            char ch = row[c];

            if (ch != '#' && ch != '.' && ch != '@' && ch != 'C' &&
                ch != 'K' && ch != 'D' && ch != 'E') {
                fprintf(stderr, "错误：地图第 %d 行第 %d 列出现非法字符 '%c'。\n",
                        r + 1, c + 1, ch);
                fclose(fp);
                level->rows = r + 1;
                level_free(level);
                return 0;
            }
            level->grid[r][c] = ch;
            track_cell(level, counts, ch, r, c);
        }
        level->grid[r][cols] = '\0';
    }

    /* ENDMAP 行 */
    if (fgets(line, sizeof(line), fp) == NULL ||
        strcmp(strip_eol(line), "ENDMAP") != 0) {
        fprintf(stderr, "错误：地图后缺少 ENDMAP 行。\n");
        fclose(fp);
        level->rows = rows;
        level_free(level);
        return 0;
    }
    fclose(fp);

    /* 检查 '@' 'C' 'K' 'D' 'E' 各恰好出现一次 */
    {
        const char *names[5] = {"@", "C", "K", "D", "E"};

        for (c = 0; c < 5; c++) {
            if (counts[c] != 1) {
                fprintf(stderr, "错误：地图中 %s 应恰好出现 1 次，实际出现 %d 次。\n",
                        names[c], counts[c]);
                level->rows = rows;
                level_free(level);
                return 0;
            }
        }
    }

    level->rows = rows;
    level->cols = cols;
    level->turns = turns;
    return 1;
}

void level_free(Level *level)
{
    int r;

    if (level->grid != NULL) {
        for (r = 0; r < level->rows; r++) {
            free(level->grid[r]);
        }
        free(level->grid);
        level->grid = NULL;
    }
}
