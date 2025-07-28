# 协程

1.ucontext_t

2.制定宏，用于调试

## 设计
1. 协程的封装
协程模型：主协程可以开启多个子协程，非对称
```
Thread -> main_coroutine <------------> sub_coroutine : 主协程可以开启多个子协程，非对称
                ^
                |
                |
                v
          sub_coroutine
```
2. 协程调度scheduler
多个线程消费一个协程池
scheduler 1 -- N Thread 1 -- N Coroutine
- 线程池 : 包含协程队列（可以是function<void()>， 也可以是coroutine）
- 协程调度器：将协程指定到响应的线程上执行

## 基础

### backtrace函数
- 返回当前函数的调用栈，需要传入一个buffer（void*）和一个调用栈栈帧的最大条目数
- 原型
```c++
#include <execinfo.h>

int backtrace(void* buffer, int size); // 将当前函数调用栈内容放到buffer中

char** backtrace_symbols(void* const *buffer, int size); // 将buffer中的内容解析成符号，返回一个malloced的内存地址，需要手动释放
```