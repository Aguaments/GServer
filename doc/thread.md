# 线程库

## 目标
封装Thread 、 Mutex

## 基础

### pthread
1. pthread_create函数
- 原型：
```c++
#include <pthread.h>
int pthread_create(
    pthread_t *thread,
    const pthread_attr_T * attr,
    void* (*start_routine)(void*),
    void *arg
)

// thread：输出，线程ID
// attr：输入，线程属性（栈大小、是否为可分离线程，通常传递null，表示默认属性）
// start_routine：输入，线程执行函数
// arg：输入， 传递给函数的参数

// 返回值：
//      - 0：创建成功
//      - 非0：创建失败
```
2. pthread_join函数
- 原型
```c++
int pthread_join(
    pthread_t thread,
    void** retval
)

// thread：输入，要等待的线程的id
// retval：输出，线程函数的返回值，不啊关心可以传递nullptr
```
3. pthread_detach函数
- 主动分离线程，子线程不再依赖主线程
- 原型
```c++
int pthread_detach(
    pthread_t thread
)

// thread：输入，要分离的线程的ID
```

4. pthread_exit函数
- 原型
```c++
void pthread_exit(void* retval);

// retval：输出，线程返回值，主动让当前执行函数的线程退出
```

### semaphore
1. sem_init函数
- 原型
```c++
int sem_init(
    sem_t * sem,
    int pshared,
    unsigned int value
)

// sem：指向sem_t的指针
// pshared：是否在进程之间共享，0表示共享
// value：信号量的初始值，一般为资源的数量
```