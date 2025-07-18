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