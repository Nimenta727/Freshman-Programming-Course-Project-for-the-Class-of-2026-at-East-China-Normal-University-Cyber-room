# 赛博密室 · 零点逃脱（cyber_room）

用 C 语言实现的密室小游戏：从文件读取关卡和操作脚本，玩家在地图上移动、调查线索取得开门密码、拾取钥匙、打开密码门并抵达出口；行动次数用完仍未逃脱则失败。密码被简化为"是否已通过 `inspect` 取得"的状态。

## 构建与运行

```bash
make clean
make
./cyber_room --level level.txt --script script.txt --output result.txt
```

- 三个文件路径均通过命令行参数传入，不写死在程序中。
- 除命令行参数外，程序不要求任何手动输入；过程提示输出到终端，不写入结果文件。

## 模块分工

| 文件 | 职责 |
|------|------|
| `main.c` | 程序入口：解析 `--level` / `--script` / `--output` 参数，串联各模块 |
| `level.h` / `level.c` | 关卡模块：读取并校验关卡文件（尺寸范围、TURNS 范围、地图行列数、合法字符、`@ C K D E` 各恰好一次） |
| `game.h` / `game.c` | 游戏模块：维护游戏状态，按脚本逐条执行 `move up/down/left/right`、`inspect`、`take`、`open`，判定行动是否成功、是否消耗次数，以及 ESCAPED / FAILED 的最终结果 |

## 文件格式

### 关卡文件 level.txt

```
SIZE 3 11
TURNS 20
MAP
###########
#@.C.K.D.E#
###########
ENDMAP
```

- `SIZE 行数 列数`：行数 3~20，列数 3~40，地图行列数必须与之一致。
- `TURNS n`：最大行动次数，1~100。
- 地图字符：`#` 墙、`.` 空地、`@` 起点、`C` 线索、`K` 钥匙、`D` 密码门、`E` 出口；后五者各恰好出现一次。

### 脚本文件 script.txt

每行一条命令，空行忽略，大小写严格匹配：

- `move up` / `move down` / `move left` / `move right`：向相应方向移动一格；撞到地图外或墙、或进入未打开的 `D` / `E` 时失败且不消耗次数，正常移动消耗 1 次。
- `inspect`：仅在位于 `C` 且尚未调查时成功，取得密码并消耗 1 次。
- `take`：仅在位于 `K` 且尚未拾取时成功，取得钥匙并消耗 1 次。
- `open`：仅在与 `D` 上下左右相邻、已取得密码和钥匙、门尚未打开时成功，消耗 1 次。
- 无法识别的命令不改变状态、不消耗次数（终端会提示）。

### 结果文件 result.txt

正常完成时恰好五行：

```
RESULT ESCAPED
ACTIONS 11
CLUE_FOUND 1
KEY_TAKEN 1
DOOR_OPENED 1
```

`RESULT` 只可能是 `ESCAPED` 或 `FAILED`；`ACTIONS` 为已消耗的行动次数；后三个字段为 0 或 1。

## 结束条件

每次消耗行动后检查：若刚进入出口则判定 `ESCAPED`（优先）；否则已用行动数达到 `TURNS` 判定 `FAILED`；脚本执行完仍未逃脱也判定 `FAILED`。得到最终结果后不再执行后续命令。
