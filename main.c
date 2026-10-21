/*
 * 赛博密室 · 零点逃脱
 *
 * 从文件读取关卡和操作脚本，按脚本逐条执行移动、调查、拾取、开门，
 * 在行动次数限制内判定逃脱或失败，并按固定格式写出结果文件。
 *
 * 用法：
 *   ./cyber_room --level level.txt --script script.txt --output result.txt
 */

#include <stdio.h>
#include <string.h>

#include "level.h"
#include "game.h"

static void print_usage(const char *prog)
{
    fprintf(stderr,
            "用法：%s --level 关卡文件 --script 脚本文件 --output 结果文件\n",
            prog);
}

int main(int argc, char *argv[])
{
    const char *level_path = NULL;
    const char *script_path = NULL;
    const char *output_path = NULL;
    int i;

    /* 解析命令行参数（除这些参数外不要求任何手动输入）。 */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--level") == 0 && i + 1 < argc) {
            level_path = argv[++i];
        } else if (strcmp(argv[i], "--script") == 0 && i + 1 < argc) {
            script_path = argv[++i];
        } else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            output_path = argv[++i];
        } else {
            fprintf(stderr, "错误：无法识别的参数 \"%s\"。\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    if (level_path == NULL || script_path == NULL || output_path == NULL) {
        print_usage(argv[0]);
        return 1;
    }

    {
        Level level;
        Game game;
        int ok;

        if (!level_load(level_path, &level)) {
            /*
             * 关卡文件非法：终端提示原因，结果文件仍按固定格式
             * 写出 FAILED，便于调用方统一处理。
             */
            Game dummy;

            game_init(&dummy, &(Level){0});
            dummy.result = RESULT_FAILED;
            game_write_result(&dummy, output_path);
            return 1;
        }

        game_init(&game, &level);
        game_run_script(&game, script_path);
        ok = game_write_result(&game, output_path);
        level_free(&level);
        return ok ? 0 : 1;
    }
}
