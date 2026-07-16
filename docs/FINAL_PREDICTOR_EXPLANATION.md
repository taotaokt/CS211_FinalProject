# Final Cache Predictor 代码说明

Group Number: 7

Group Members: Zhitong Guo, Catherine Liu

## 1. Final Predictor 的目标

Final Predictor 使用与 Initial Predictor 相同的命令行接口：

```bash
./Cache_predictor.out cache_input.txt predictions.txt
```

程序为每个有效输入地址生成一条预测，因此输入和输出的行数相同。对输入位置 `i`，程序使用该位置之前的历史地址建立候选、排序，并只输出得分最高的地址。第一行没有历史数据时，默认预测首地址加 4。

Final Predictor 综合重复序列、稳定 stride、历史地址转移和近期高频地址；Initial Predictor 则只使用相邻地址的 stride。

## 2. Candidate 结构体

```c
typedef struct
{
    unsigned long long address;
    int score;
} Candidate;
```

每个候选包含地址和置信度分数。`add_candidate()` 会合并重复地址并保留较高分数，候选数组最多保存 256 项。

## 3. 读取和保存输入

程序使用 `malloc()` 建立初始容量为 1024 的动态数组。输入超过容量时，`realloc()` 将容量扩大一倍。

`fgets()` 每次读取一行，`strtoull()` 将十进制或十六进制文本转换为 `unsigned long long` 地址。无法解析的行会被跳过，程序结束前使用 `free()` 释放动态数组。

## 4. 逐个位置建立候选

主循环为每个有效输入位置重新清空候选数组。位置 `i` 大于 0 时，四种检测算法只分析 `addresses[0]` 到 `addresses[i - 1]`，避免使用当前或未来地址作预测。

程序还会加入最近一个和最近两个历史地址作为低置信度 locality 候选，确保候选列表不会为空。

## 5. 重复序列检测

`detect_repeating_pattern()` 比较历史末尾的两段地址，周期长度从 1 到 16。

例如历史为：

```text
10 20 30 10 20 30
```

最后两个长度为 3 的序列相同，程序会把 `10、20、30` 加入高分候选，并优先采用最短的重复周期。

## 6. 稳定 stride 检测

`detect_stable_stride()` 检查最近最多 8 个地址差值，并寻找出现次数最多的 stride。

例如历史为：

```text
16 20 24 28
```

最常见差值是 4，因此程序建立 `32、36、40、44` 四个候选。最接近的预测分数最高。如果没有至少重复两次的稳定 stride，程序会以较低分数使用最近一次 stride。

## 7. 历史地址转移检测

`detect_transitions()` 查找历史上当前末尾地址出现的位置，并统计它后面曾出现的地址。

如果历史中多次出现 `100 -> 200`，而当前末尾地址是 100，程序会把 200 加入候选；同一转移出现越频繁，分数越高。

## 8. 近期高频地址检测

`detect_recent_frequency()` 检查最近最多 64 个历史地址。出现至少两次的地址会成为候选，出现次数越多，分数越高。这个策略利用 temporal locality：近期频繁访问的地址可能很快再次出现。

## 9. 排序与输出

`qsort()` 使用 `compare_candidates()` 按以下顺序排列候选：

1. 分数较高的候选优先。
2. 分数相同，地址较小的候选优先。

每个输入位置只写入排序后的第一个候选：

```c
fprintf(output, "%llu\n", candidates[0].address);
```

输出每行只有一个十进制地址，不包含解释文字。

## 10. 使用的主要 C 功能

- `struct`：将候选地址和分数组合在一起。
- `malloc()`、`realloc()` 和 `free()`：管理动态地址数组。
- `fgets()` 和 `strtoull()`：解析十进制或十六进制输入。
- `qsort()`：按置信度排列候选。
- 函数指针：把 `compare_candidates()` 传给 `qsort()`。

程序没有使用链表、哈希表、递归、多线程或第三方库。

## 11. 新版验证结果

仓库更新时使用 `gcc -Wall -Werror -O2` 成功编译两个预测器。对 Benchmark Suite 生成的 1024 行 `cache_input.txt`，Final 和 Initial 均生成 1024 行预测，满足逐输入地址输出一条预测的接口要求。

压缩包附带的四次 Benchmark 摘要显示 Final Predictor 的 Prediction Accuracy 约为 13.07%–13.67%，Coverage 为 100.00%，Overall Score 约为 19.29–19.46 / 100。正式成绩仍以课程平台的最终运行结果为准。
