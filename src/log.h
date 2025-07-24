#pragma once
#include <string>
#include <memory>
#include <list>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include <yaml-cpp/yaml.h>

#include <stdint.h>

#include "utils.h"
#include "singleton.h"



// 普通的字符串输出
#define AGENT_LOG_LEVEL(logger, level)\
    if(logger -> getLevel() <= level)\
        agent::LogEventWrapper(agent::LogEvent::ptr(new agent::LogEvent(logger, level, __FILE__, __LINE__, 0, agent::Utils::getThreadId(),\
        agent::Utils::getCoroutineId(), time(0)))).getSS()

#define AGENT_LOG_DEBUG(logger) AGENT_LOG_LEVEL(logger, agent::LogLevel::DEBUG)
#define AGENT_LOG_INFO(logger) AGENT_LOG_LEVEL(logger, agent::LogLevel::INFO)
#define AGENT_LOG_WARN(logger) AGENT_LOG_LEVEL(logger, agent::LogLevel::WARN)
#define AGENT_LOG_ERROR(logger) AGENT_LOG_LEVEL(logger, agent::LogLevel::ERROR)
#define AGENT_LOG_FATAL(logger) AGENT_LOG_LEVEL(logger, agent::LogLevel::FATAL)


// 支持格式化输出模式
#define AGENT_LOG_FMT_LEVEL(logger, level, fmt, ...)\
    if(logger -> getLevel() <= level)\
        agent::LogEventWrapper(agent::LogEvent::ptr(new agent::LogEvent(logger, level, __FILE__, __LINE__, 0, agent::Utils::getThreadId(),\
        agent::Utils::getCoroutineId(), time(0)))).getEvent() -> format(fmt, __VA_ARGS__)

#define AGENT_LOG_FMT_DEBUG(logger, fmt, ...) AGENT_LOG_FMT_LEVEL(logger, agent::LogLevel::DEBUG, fmt,  __VA_ARGS__)
#define AGENT_LOG_FMT_INFO(logger, fmt, ...) AGENT_LOG_FMT_LEVEL(logger, agent::LogLevel::INFO, fmt,  __VA_ARGS__)
#define AGENT_LOG_FMT_WARN(logger, fmt, ...) AGENT_LOG_FMT_LEVEL(logger, agent::LogLevel::WARN, fmt,  __VA_ARGS__)
#define AGENT_LOG_FMT_ERROR(logger, fmt, ...) AGENT_LOG_FMT_LEVEL(logger, agent::LogLevel::ERROR, fmt,  __VA_ARGS__)
#define AGENT_LOG_FMT_FATAL(logger, fmt, ...) AGENT_LOG_FMT_LEVEL(logger, agent::LogLevel::FATAL, fmt,  __VA_ARGS__)

#define AGENT_LOG_ROOT() agent::LoggerMgr::getInstance() -> getRoot()
#define AGENT_LOG_BY_NAME(name) agent::LoggerMgr::getInstance() -> getLogger(name)


namespace agent{

    class Logger;
    class LoggerManager;

    enum class LogLevel{
        UNKONWN = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3, 
        ERROR = 4,
        FATAL = 5
    };

    const char* LogLevelToString(const LogLevel& ll) ;
    const LogLevel FromStringToLogLevel(const std::string& str);

    /*******************************************************************************
      * Class name    : [LogLevel]
      * Description   : [日志事件的详情]
      * Date          : 2025/07/09 14:36:34
      * Author        : [Guanyue Gao]
     *******************************************************************************/
    class LogEvent{
    public:
        using ptr = std::shared_ptr<LogEvent>;
        LogEvent(std::shared_ptr<Logger> logger, LogLevel level
            ,const char* file, int32_t line, uint32_t elapse
            ,uint32_t thread_id, uint32_t coroutine_id, uint64_t time
            );

        std::shared_ptr<Logger> getLogger() const {return m_logger;}
        const char* getFile() const {return m_filename;}
        int32_t getline() const {return m_line;}
        uint32_t getThreadId() const {return m_threadId;}
        uint32_t getCoroutineId() const {return m_coroutineId;}
        uint64_t getTime() const {return m_time;}
        uint32_t getElapse() const {return m_elapse;}
        std::string getContent() const {return m_ss.str();}
        LogLevel getLevel() const {return m_level;}

        std::stringstream& getSS() {return m_ss;}

        void format(const char* fmt, ...);
        void format(const char* fmt, va_list al);

        
    private:
        std::shared_ptr<Logger> m_logger;               // 日志器 
        const char* m_filename = nullptr;   // 发生错误的文件
        LogLevel m_level;                   // 日志级别
        int32_t m_line = 0;                 // 出错的行号
        uint32_t m_threadId = 0;            // 线程ID
        uint32_t m_coroutineId = 0;         // 协程ID
        uint64_t m_time = 0;                // 时间戳
        uint32_t m_elapse = 0;              // 程序启动到当前消耗的毫秒数
        std::stringstream m_ss;             // 内容 
    };

    /*******************************************************************************
      * Class name    : [LogEventWrapper]
      * Description   : LogEvent的包装器
      * Date          : 2025/07/13 14:18:03
      * Author        : [Guanyue Gao]
     *******************************************************************************/
    class LogEventWrapper
    {
    public:
        LogEventWrapper(LogEvent::ptr event);
        ~LogEventWrapper();
        std::stringstream& getSS();

        LogEvent::ptr getEvent(){return m_event;}
    private:
        LogEvent::ptr m_event;
    };

    /*******************************************************************************
      * Class name    : [LogFormatter]
      * Description   : 将LogEvent处理为指定格式的数据
      * Date          : 2025/07/09 14:49:33
      * Author        : [Guanyue Gao]
     *******************************************************************************/
    class LogFormatter{
    public:
        using ptr =  std::shared_ptr<LogFormatter>;

        LogFormatter(const std::string& pattern);
        std::string format(std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event);
    
    public:
        class FormatItem{
        public:
            using ptr = std::shared_ptr<FormatItem>;
            virtual ~FormatItem(){};
            virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel ll, LogEvent::ptr event) = 0;
        };
        void init();
        
        bool isError() const {return m_error;} 

        const std::string getPattern() const {return m_pattern;}
        
    private:
        std::string m_pattern;
        std::vector<FormatItem::ptr> m_items;
        bool m_error = false;
    };

    
    /*******************************************************************************
      * Class name    : [LogAppender]
      * Description   : 将日志输出到指定位置的操作器
      * Date          : 2025/07/09 16:24:53
      * Author        : [Guanyue Gao]
     *******************************************************************************/
    class LogAppender{
        friend class Logger;
    public:
        using ptr = std::shared_ptr<LogAppender>;
        virtual ~LogAppender(){};

        virtual void log(std::shared_ptr<Logger> logger, LogLevel ll, LogEvent::ptr event) = 0;

        LogFormatter::ptr getFormatter() const {return m_formatter;};
        void setFormatter(LogFormatter::ptr formatter);
        const LogLevel getLevel() const {return m_level;}
        void setLevel(LogLevel ll) {m_level = ll;}

        virtual std::string toYamlString() = 0;

    protected:
        LogLevel m_level = LogLevel::DEBUG;
        bool m_hasFormatter = false; // 判断当前appender是否有formatter
        LogFormatter::ptr m_formatter;
    };

    // 定制化appender
    // 1. 定位到标准输出的appender
    class SoutLogAppender: public LogAppender{
    public: 
        typedef std::shared_ptr<SoutLogAppender> ptr;
        virtual void log(std::shared_ptr<Logger> Logger, LogLevel ll, LogEvent::ptr event) override;
        virtual std::string toYamlString() override; 
    };


    // 2. 定位到文件的appender
    class FileLogAppender: public LogAppender{
    public:
        using ptr =  std::shared_ptr<FileLogAppender>;
        FileLogAppender(const std::string filename);

        virtual void log(std::shared_ptr<Logger> logger, LogLevel ll, LogEvent::ptr event) override;

        bool reopen(); // 重新开启文件流，判断是否成功
        virtual std::string toYamlString() override;   // 转为string
    
    private:
        std::string m_filename;
        std::ofstream m_filestream;
    };


    /*******************************************************************************
      * Class name    : [Logger]
      * Description   : 日志器，用于执行日志打印的外壳
      * Date          : 2025/07/09 16:25:50
      * Author        : [Your Name Here]
     *******************************************************************************/
    class Logger: public std::enable_shared_from_this<Logger>{
        friend class LoggerManager;
    public:
        using ptr =  std::shared_ptr<Logger>;
        Logger(const std::string& name = "root");

        void info(LogEvent::ptr event);
        void debug(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);      
        
        void addAppender(LogAppender::ptr appender);
        void delAppender(LogAppender::ptr appender);
        void clearAppenders();

        LogLevel getLevel() const {return m_level;};
        void setLevel(const LogLevel level){m_level = level;};
        LogFormatter::ptr getFormatter(){return m_formatter;};
        void setFormatter(LogFormatter::ptr val);
        void setFormatter(const std::string& val);
        const std::string& getName() const {return m_name;}

        void log(LogLevel ll, LogEvent::ptr event); // basic log info 

        std::string toYamlString();

    private:
        std::string m_name;
        LogLevel m_level;
        std::list<LogAppender::ptr> m_appenders;    // appender list
        LogFormatter::ptr m_formatter;              // default formater
        Logger::ptr m_root;
    };

    /*******************************************************************************
      * Class name    : [LoggerManager]
      * Description   : 
      * Date          : 2025/07/14 10:27:58
      * Author        : [Guanyue Gao]
     *******************************************************************************/
    class LoggerManager
    {
    public:
        LoggerManager();
        Logger::ptr getLogger(const std::string& name);

        void init();
        Logger::ptr getRoot() const {return m_root;}

        std::string toYamlString() const;
    private:
        std::unordered_map<std::string, Logger::ptr> m_logger_map;
        Logger::ptr m_root;
    };

    using LoggerMgr = agent::Singleton<LoggerManager>;
}