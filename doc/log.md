# 日志

## 目标
建立日志器，定制化输出日志内容。

## 日志格式
仿照log4j日志格式，例如如下pattern
```xml
%d{yyyy-MM-dd HH:mm:ss} %-5p %c{1}:%L - %m%n
```
- 解析（目前只支持对日期的的不同格式解析）：
    - %d{yyyy-MM-dd HH:mm:ss}
    - %-5p
    - %c{1}
    - %L
    - %m
    - %n
## 解析思路
- 不包含格式的解析类型  %xxx          ----------type 0
- 包含格式的解析类型    %xxx{xxx}     ----------type 1
- 普通的自定义格式串    other symbol  ----------type 2

## 设计思路
logger ---->  appender.1 ----> Formatter
              appender.2 ----> Formatter
              appender.3 ----> Formatter

Logger ----> LogEventWrapper（控制LogEvent的生命周期，在析构中做日志打印操作）----> LogEvent 

# 日志开发中编程注意事项
## ofstream文件流
1. 如果其成为成员变量，不需要在类的析构函数中显式调用close方法，ofstream的析构函数会自动调用
2. 常见的文件打开模式（非ofstream独有）：
    - std::ios::in（文件不存在不会创建文件）
    - std::ios::out（文件不存在会创建文件）
    - std::ios::app（文件不存在会创建文件）
3. 文件的创建方式
```c++
// 创建并打开文件
ofstream ofs("filename", std::ios::out | std::ios::app); // 默认std::ios::out模式

// 需要使用open显示调用并打开文件
ofstream ofs；
ofs.open("filename", mode);
```
## ifstream
## fstream
```c++
fstream file("filename", std::ios::in | std::ios::out | std::ios::app);
```