# Initial Cache Predictor 代码说明

## 1. 这个程序做什么

模拟器会用下面的形式运行程序：

```bash
./Cache_predictor_initial.out cache_input.txt predictions.txt
```

因此，程序从 `argv[1]` 指定的文件读取历史地址，并把预测结果写入 `argv[2]` 指定的文件。

这个 Initial Predictor 使用最简单的 stride（地址差值）预测：

```text
历史地址：16, 20
最近差值：20 - 16 = 4
预测地址：20 + 4 = 24
```

它只输出一个预测地址。这个版本主要作为稳定、容易理解的初始基线，之后再用 `Cache_predictor.c` 实现更好的 Final Predictor。

## 2. 使用的 C 语言内容

代码只使用了课程初期常见的 C 语言功能：

- `#include <stdio.h>`：文件输入输出。
- `#define`：定义默认最大地址和 block size。
- `long`：保存地址和地址差值。
- `if`：检查错误并选择预测方式。
- `while`：逐个读取地址。
- `fopen`、`fscanf`、`fprintf`、`fclose`：读写文件。

代码没有使用：

- 动态内存分配
- 指针运算
- 结构体
- 链表、哈希表或树
- 递归
- 多线程
- 外部第三方库

## 3. 命令行参数检查

```c
if (argc != 3)
```

`argc` 是命令行项目的数量。正确运行时共有三项：

1. 程序名称
2. 输入文件名称
3. 输出文件名称

如果数量不正确，程序打印用法并返回 `1`，表示运行失败。

## 4. 读取输入文件

```c
input_file = fopen(argv[1], "r");
```

`"r"` 表示只读。如果文件打不开，`fopen` 返回 `NULL`，程序立即显示错误并退出。

读取循环是：

```c
while (fscanf(input_file, "%li", &current_address) == 1)
```

`%li` 可以读取普通十进制地址，也可以读取以 `0x` 开头的十六进制地址。

程序不需要保存所有历史地址。每读取一个新地址，只更新：

- `previous_address`：倒数第二个地址
- `last_address`：最后一个地址
- `address_count`：一共读到了多少地址

这让程序保持简单，也避免使用数组或动态内存。

## 5. 计算预测地址

当文件中至少有两个地址时：

```c
stride = last_address - previous_address;
prediction = last_address + stride;
```

例如：

```text
previous_address = 100
last_address = 104
stride = 4
prediction = 108
```

如果输入只有一个地址，程序无法计算 stride，因此按照默认 cache block size 向前预测 4：

```c
prediction = last_address + BLOCK_SIZE;
```

## 6. 控制地址范围

默认最大地址是 4096，因此有效范围按照 `0` 到 `4095` 处理：

```c
prediction = prediction % MAX_ADDRESS;
if (prediction < 0) {
    prediction = prediction + MAX_ADDRESS;
}
```

例如预测结果为 4098 时，会转换成地址 2。负数预测也会转换回合法范围。

## 7. 写入输出文件

```c
output_file = fopen(argv[2], "w");
fprintf(output_file, "%ld\n", prediction);
```

`"w"` 会创建或覆盖输出文件。输出文件只有一个十进制地址和一个换行，不包含 `HIT`、`MISS` 或解释文字，符合模拟器要求。

## 8. 编译和手动测试

在 `pa3` 目录运行：

```bash
make initial
```

Makefile 会使用：

```bash
gcc -Wall -Werror -O2 Cache_predictor_initial.c -o Cache_predictor_initial.out
```

建立一个测试文件 `cache_input.txt`：

```text
0x10
0x14
0x18
0x1c
```

运行：

```bash
./Cache_predictor_initial.out cache_input.txt predictions.txt
cat predictions.txt
```

最后两个地址是 `0x18`（24）和 `0x1c`（28），stride 是 4，所以预期输出是：

```text
32
```

## 9. 这个初始算法的限制

程序只观察最后两个地址。如果最后一次地址变化是偶然的，预测就可能错误；它也无法识别复杂循环和多个交替规律。这是有意保留的限制，因为 Initial Predictor 应当作为容易解释的基线，后续 Final Predictor 才负责改善准确率、覆盖率和其他指标。
