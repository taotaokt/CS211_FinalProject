# Final Cache Predictor 代码说明

Group Number: 7

Group Members: Zhitong Guo, Catherine Liu

## 1. Final Predictor 的目标

Final Predictor 读取 `cache_input.txt` 中的历史地址，生成最多 6 个可能的后续地址，并写入 `predictions.txt`。它与 Initial Predictor 使用相同接口：

```bash
./Cache_predictor.out cache_input.txt predictions.txt
```

Initial 只根据最后两个地址预测一个结果。Final 同时分析重复序列、稳定 stride、历史地址转移和近期高频地址，因此能够提高 Accuracy 和 Coverage。

## 2. Candidate 结构体

```c
typedef struct
{
    unsigned long long address;
    int score;
} Candidate;
```

每个候选包含一个预测地址和一个置信度分数。多个算法可以提出候选；分数越高，输出优先级越高。

`add_candidate()` 负责加入候选。如果地址已经存在，就保留较高分数，因此输出中不会重复同一地址。

## 3. 读取和保存输入

程序使用 `malloc()` 建立初始容量为 1024 的动态数组。输入超过容量时，`realloc()` 将容量扩大一倍。

`fgets()` 每次读取一行，`strtoull()` 将十进制或十六进制文本转换成 `unsigned long long` 地址。程序结束前使用 `free()` 释放动态数组。

## 4. 重复序列检测

`detect_repeating_pattern()` 比较输入末尾的两段地址，周期长度从 1 到 16。

例如：

```text
10 20 30 10 20 30
```

最后两个长度为 3 的序列相同，因此程序将 `10、20、30` 加入高分候选。

## 5. 稳定 stride 检测

`detect_stable_stride()` 计算最近最多 8 个地址差值，并寻找出现次数最多的 stride。

例如：

```text
16 20 24 28
```

最常见差值是 4，因此程序预测：

```text
32 36 40 44
```

如果没有重复至少两次的 stride，程序使用最后一个 stride 产生一个较低分候选。

## 6. 历史地址转移检测

`detect_transitions()` 查找历史上当前最后地址出现的位置，并观察它后面出现过什么地址。

如果历史中多次出现：

```text
100 -> 200
```

而当前最后地址是 100，程序就把 200 加入候选。出现次数越多，候选分数越高。

## 7. 近期高频地址检测

`detect_recent_frequency()` 检查最近最多 64 个地址。出现至少两次的地址会成为候选，出现次数越多，分数越高。

这个策略利用 temporal locality：最近经常访问的地址可能很快再次出现。

## 8. 排序与输出

程序把最后两个历史地址作为低分 locality 候选，保证常见地址仍有机会被预测。

`qsort()` 使用 `compare_candidates()` 按以下顺序排列：

1. 分数较高的候选优先。
2. 分数相同，地址较小的候选优先。

程序最多输出前 6 个地址，每行一个，不输出解释文字。

## 9. 使用的主要 C 功能

- `struct`：将候选地址和分数组合在一起。
- `malloc()`：建立动态地址数组。
- `realloc()`：输入过多时扩大数组。
- `free()`：释放动态内存。
- `qsort()`：按置信度排列候选。
- 函数指针：把 `compare_candidates()` 传给 `qsort()`。

程序没有使用链表、哈希表、递归或多线程。

## 10. Eval 验证结果

Group 7 的 Eval `Group07_Eval_20260714-043828` 得到：

```text
Initial compiled successfully: PASS
Final compiled successfully: PASS
Improved metrics: 5 / 8
Base score: 30 / 30
Extra credit: +6 / +12
Displayed evaluation score: 36 / 30
```

Final 的 Accuracy 从 0.11% 提高到 2.22%，Coverage 从 0.10% 提高到 0.59%。Eval 分数用于开发参考，最终成绩仍以 Gradescope 和正式 Benchmark 完成情况为准。
