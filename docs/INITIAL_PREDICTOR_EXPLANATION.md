# Initial Cache Predictor 代码说明

Group Number: 7

Group Members: Zhitong Guo, Catherine Liu

## 1. 这个程序做什么

模拟器会用下面的形式运行程序：

```bash
./Cache_predictor_initial.out cache_input.txt predictions.txt
```

程序从 `argv[1]` 读取地址，并向 `argv[2]` 写入预测。每个有效输入地址对应一行输出，所以输入和输出的地址数量相同。

Initial Predictor 使用简单的 stride（相邻地址差值）预测：

```text
输入地址：16, 20, 24
输出预测：20, 24, 28
```

第一行还没有历史 stride，因此默认向前预测一个 4-byte cache block。之后每一行都使用当前地址和前一个地址之间的 stride。

## 2. 读取和保存输入

程序使用 `fgets()` 逐行读取文本，再用 `strtoull()` 解析十进制或带 `0x` 前缀的十六进制地址。无法解析的行会被跳过。

地址保存在动态数组中：

- `malloc()` 建立初始容量为 1024 的数组。
- 数组满时，`realloc()` 将容量扩大一倍。
- 所有处理结束后，`free()` 释放数组。

如果输入文件没有有效地址，程序返回失败状态，不生成预测。

## 3. 逐行计算预测

对第一个地址，程序默认向前移动一个 cache block：

```c
prediction = (long)addresses[0] + BLOCK_SIZE;
```

对后续每个地址 `i`，先计算相邻 stride，再从当前地址向前预测：

```c
stride = (long)addresses[i] - (long)addresses[i - 1];
prediction = (long)addresses[i] + stride;
```

例如输入 `100, 104, 108` 时，三行输出依次是 `104, 108, 112`。

## 4. 控制地址范围

默认最大地址为 4096，程序把每个预测限制在 `0` 到 `4095`：

```c
prediction = prediction % MAX_ADDRESS;
if (prediction < 0) {
    prediction = prediction + MAX_ADDRESS;
}
```

例如预测结果 4098 会转换为地址 2，负数预测也会回绕到合法范围。

## 5. 输出格式

输出文件使用 `"w"` 模式创建或覆盖。每个有效输入地址只产生一个十进制预测地址：

```c
fprintf(output_file, "%ld\n", prediction);
```

输出不包含 `HIT`、`MISS` 或解释文字。

## 6. 编译和手动测试

在 `pa3` 目录运行：

```bash
make initial
```

Makefile 使用严格警告设置编译：

```bash
gcc -Wall -Werror -O2 Cache_predictor_initial.c -o Cache_predictor_initial.out
```

例如输入：

```text
0x10
0x14
0x18
0x1c
```

预期输出为：

```text
20
24
28
32
```

## 7. 算法限制

Initial Predictor 只使用相邻两个地址的差值，不能识别复杂循环、多个交替规律或历史转移。这是有意保留的简单基线；Final Predictor 会综合多种历史模式选择更高置信度的结果。
