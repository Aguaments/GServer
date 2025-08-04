# 定时器
程序需要在某些时间周期执行任务，需要定时器管理

## 设计
### 在定时器中进行一些功能操作
1. 添加任务
2. 取消任务
3. 删除任务
4. 定时器触发距离现在的时间差
5. 返回当前触发的定时器

### 类设计

TimerManager -----------> Timer 
|                          |-------> cancel
|----> addTimer            |-------> refresh
|----> addConditionTimer   |-------> reset
|----> getNextTimer
|----> listExpiredCb // 返回失效或者已经到达时间的timer
|----> hasTimer
|----> detectClockRoller // 服务器向前推迟一段时间的检查方法
|----> onTimerInsertAtFront = 0 // 调用了一个tickle，唤醒epoll_wait的等待状态（iomanager继承timer，并实现该函数）

# 基础知识
1. vector容器insert函数的迭代器版本
```c++
insert(pos, begin, end) // pos表示要插入的位置，begin是迭代器的开始，end是迭代器的终止
```