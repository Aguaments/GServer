#include "log.h"
#include <iostream>
#include <tuple>
#include <map>
#include <ctime>
#include <functional>
#include <string.h>

#include <stdarg.h>

namespace agent{
    const char* LogLevelToString(const LogLevel& ll)
    {
        switch(ll)
        {
            #define XX(level) \
                case LogLevel::level: \
                    return #level; \
                    break;
            XX(DEBUG)
            XX(INFO)
            XX(WARN)
            XX(ERROR)
            XX(FATAL)
            #undef XX

            default:
                return "UNKNOW";
        }
        return "UNKONW";
    }

    /**************************************************************/
    /* Class: LogEvent */
    /**************************************************************/
    LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel level
            ,const char* file, int32_t line, uint32_t elapse
            ,uint32_t thread_id, uint32_t coroutine_id, uint64_t time
            )
            :m_logger(logger)
            ,m_filename(file)
            ,m_level(level)
            ,m_line(line)
            ,m_threadId(thread_id)
            ,m_coroutineId(coroutine_id)
            ,m_time(time)
            ,m_elapse(elapse)
            {}

    void LogEvent::format(const char* fmt, ...)
    {
        va_list vl;
        va_start(vl, fmt);
        format(fmt, vl);
        va_end(vl);
    }

    void LogEvent::format(const char* fmt, va_list vl)
    {
        char* buf = nullptr;
        int len = vasprintf(&buf, fmt, vl);
        if(len != -1)
        {
            m_ss << std::string(buf, len);
            free(buf);
        }
    }

    /**************************************************************/
    /* Class: LogEventWrapper */
    /**************************************************************/

    LogEventWrapper::LogEventWrapper(LogEvent::ptr event)
    :m_event(event)
    {}

    LogEventWrapper::~LogEventWrapper()
    {
        m_event -> getLogger() -> log(m_event -> getLevel(), m_event);
    }
        
    std::stringstream& LogEventWrapper::getSS()
    {
        return m_event -> getSS();
    }

    /**************************************************************/
    /* Class: Formatter */
    /**************************************************************/
    LogFormatter::LogFormatter(const std::string& pattern)
    :m_pattern(pattern)
    {
        init();
    }

    std::string LogFormatter::format(Logger::ptr logger, LogLevel ll, LogEvent::ptr event)
    {
        std::stringstream ss;
        for(auto& i: m_items)
        {
            i -> format(ss, logger, ll, event);
        }
        return ss.str();
    }  
    
    // ======================================================
    // 对每个类型的item单独进行format
    // ======================================================
    class MessageFormatItem: public LogFormatter::FormatItem{
    public:
        MessageFormatItem(const std::string& str = ""){}
        void format(std::ostream& os, Logger::ptr logger, LogLevel ll, LogEvent::ptr event) override
        {
            os << event -> getContent();
        }
    };

    class LevelFormatItem: public LogFormatter::FormatItem{
    public:
        LevelFormatItem(const std::string& str = ""){}
        void format(std::ostream& os, Logger::ptr logger, LogLevel ll, LogEvent::ptr event) override
        {
            os << LogLevelToString(ll);
        }
    };

    class DateTimeFormatItem: public LogFormatter::FormatItem{
    public:
        DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
        :m_format(format)
        {
            if(m_format.empty())
            {
                m_format = "%Y/%m/%d %H:%M:%S";
            }
        }
        void format(std::ostream& os, Logger::ptr logger, LogLevel ll, LogEvent::ptr event) override
        {
            struct tm tm;
            time_t time = event -> getTime();
            localtime_r(&time, &tm);
            char buf[64];
            strftime(buf, sizeof(buf), m_format.c_str(), &tm);
            
            os << buf;
        }
        // 解析时间基本格式
        void parse();
    private:
        std::string m_format;
    };

    class ElapseFormatItem: public LogFormatter::FormatItem{
    public:
        ElapseFormatItem(const std::string& str = ""){}
        void format(std::ostream& os, Logger::ptr logger, LogLevel ll, LogEvent::ptr event) override
        {
            os << event -> getElapse();
        }
    };

    class LoggerNameFormatItem: public LogFormatter::FormatItem{
    public:
        LoggerNameFormatItem(const std::string& str = ""){}
        void format(std::ostream& os, Logger::ptr logger, LogLevel ll, LogEvent::ptr event) override
        {
            os << logger -> getName();
        }
    };

    class ThreadIdFormatItem: public LogFormatter::FormatItem{
    public:
        ThreadIdFormatItem(const std::string& str = ""){}
        void format(std::ostream& os, Logger::ptr logger, LogLevel ll, LogEvent::ptr event) override
        {
            os << event -> getThreadId();
        }
    };

    class CoroutineIdFormatItem: public LogFormatter::FormatItem{
    public:
        CoroutineIdFormatItem(const std::string& str = ""){}
        void format(std::ostream& os, Logger::ptr logger, LogLevel ll, LogEvent::ptr event) override
        {
            os << event -> getCoroutineId();
        }
    };

    class LineFormatItem: public LogFormatter::FormatItem{
    public:
        LineFormatItem(const std::string& str = ""){}
        void format(std::ostream& os, Logger::ptr logger, LogLevel ll, LogEvent::ptr event) override
        {
            os << event -> getline();
        }
    };

    class NewLineFormatItem: public LogFormatter::FormatItem{
    public:
        NewLineFormatItem(const std::string& str = ""){}
        void format(std::ostream& os, Logger::ptr logger, LogLevel ll, LogEvent::ptr event) override
        {
            os << std::endl;
        }
    };

    class FilenameFormatItem: public LogFormatter::FormatItem{
    public:
        FilenameFormatItem(const std::string& str = ""){}
        void format(std::ostream& os, Logger::ptr logger, LogLevel ll, LogEvent::ptr event) override
        {
            os << event -> getFile();
        }
    };

    class StringFormatItem: public LogFormatter::FormatItem{
    public:
        StringFormatItem(const std::string& str)
        :m_string(str)
        {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel ll, LogEvent::ptr event) override
        {
            os << m_string;
        }
    private:
        std::string m_string;
    };

    class TabFormatItem: public LogFormatter::FormatItem{
    public:
        TabFormatItem(const std::string& str)
        {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel ll, LogEvent::ptr event) override
        {
            os << '\t';
        }
    };


    // 日志格式解析，仿照log4j
    /*
    
    */
    void LogFormatter::init() {
        /*
            1. 解析出来的内容要具备如下几种类型
                - %xxx          ----------type 0
                - %xxx{xxx}     ----------type 1
                - other symbol  ----------type 2
            2. 解析出来的内容要放到一个vector中
            3. 解析的Item要保存在一个tuple中, <symbol, format, type>
        */

        // Symbol, format, type
        // 
        if(m_pattern.empty())
        {
            // exception类做处理
            std::cout << "<Pattern error>" << std::endl;
            return ;
        }

        std::vector<std::tuple<std::string, std::string, int>> vec;

        int n = m_pattern.size();
        std::string nstr;

        for(size_t i = 0; i < m_pattern.size();) {
            if(m_pattern[i] != '%') {
                nstr.append(1, m_pattern[i++]);
                continue;
            }

            if((i + 1) < m_pattern.size()) {  // 单独处理百分号
                if(m_pattern[i + 1] == '%') {
                    nstr.append(1, '%');
                    i++;
                    continue;
                }
            }

            if(!nstr.empty())
            {
                vec.push_back(std::make_tuple(nstr, "", 2));
                std::cout << nstr << " | " << "null" << " | " << 2 << std::endl;
                nstr.clear();
            }

            int m = i + 1;

            std::string symbol;
            std::string fmt;

            int fmt_status = 0;
            int fmt_begin = 0;

            while(m <= n){
                if((m_pattern[m] != '{' && m_pattern[m] != '}' && !isalnum(m_pattern[m]))
                    && !fmt_status)
                {
                    symbol = m_pattern.substr(i + 1, m - i - 1);
                    break;
                }
                if(m_pattern[m] == '{' && fmt_status == 0)
                {
                    symbol = m_pattern.substr(i + 1, m - i - 1);
                    fmt_begin = m + 1;
                    fmt_status = 1;
                }
                if(m_pattern[m] == '}' && fmt_status == 1)
                {
                    fmt = m_pattern.substr(fmt_begin, m - fmt_begin);
                    fmt_status = 0;
                    fmt_begin = 0;
                    m ++;
                    break;
                }
                m ++;
            }
            if(!symbol.empty())
            {
                if(!fmt.empty())
                {
                    vec.push_back(std::make_tuple(symbol, fmt, 1));
                    std::cout << symbol << " | " << fmt << " | " << 1 << std::endl;
                }
                else
                {
                    vec.push_back(std::make_tuple(symbol, "", 0));
                    std::cout << symbol << " | " << fmt << " | " << 0 << std::endl;
                }
            }
            i = m;
        }
    
        static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)>> format_items = {
            #define XX(symbol, C)\
                {#symbol, [](const std::string& fmt){return FormatItem::ptr(new C(fmt));}}
                
                XX(m, MessageFormatItem),
                XX(p, LevelFormatItem),
                XX(r, ElapseFormatItem),
                XX(c, LoggerNameFormatItem),
                XX(t, ThreadIdFormatItem),
                XX(C, CoroutineIdFormatItem),
                XX(d, DateTimeFormatItem),
                XX(f, FilenameFormatItem),
                XX(l, LineFormatItem),
                XX(n, NewLineFormatItem),
                XX(T, TabFormatItem)
            #undef XX 
        };

        for(auto& i : vec)
        {
            if(std::get<2>(i) == 2)
            {
                m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
            }
            else
            {
                auto it = format_items.find(std::get<0>(i));
                if(it == format_items.end())
                {
                    m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                }
                else
                {
                    m_items.push_back(it -> second(std::get<1>(i))); // 根据不同的类型拿到对应的formatter，再把参数扔到里面去
                }
            }
        }
    }
        
    /**************************************************************/
    /* Class: SoutLogAppender */
    /**************************************************************/
    void SoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel ll, LogEvent::ptr event)
    {
        if(ll >= m_level)
        {
            std::cout << m_formatter -> format(logger, ll, event);
        }
    }

    /**************************************************************/
    /* Class: FileLogAppender */
    /**************************************************************/
    FileLogAppender::FileLogAppender(const std::string filename)
    :m_filename(filename)
    ,m_filestream(m_filename, std::ios::app)
    {}

    void FileLogAppender::log(Logger::ptr logger, LogLevel ll, LogEvent::ptr event)
    {
        if(!reopen())
        {
            return ;
        }
        if(ll >= m_level)
        {
            m_filestream << m_formatter -> format(logger, ll, event);
        }
    }

    bool FileLogAppender::reopen()
    {
        if(m_filestream)
        {
            m_filestream.close();
        }   
        m_filestream.open(m_filename, std::ios::out | std::ios::app);
        return !!m_filestream;
    }

    /**************************************************************/
    /* Class: Logger */
    /**************************************************************/
    Logger::Logger(const std::string& name)
    :m_name(name)
    ,m_level(LogLevel::DEBUG)
    {
        m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%C%T%[%p]%T(%f:%l)%T%m%n"));
    }

    void Logger::addAppender(LogAppender::ptr appender)
    {
        if(!appender -> getFormatter())
        {
            appender -> setFormatter(m_formatter);
        }
        m_appenders.emplace_back(appender);
    }

    void Logger::delAppender(LogAppender::ptr appender){
        for(auto it = m_appenders.begin(); it != m_appenders.end(); ++it)
        {
            if(*it == appender)
            {
                m_appenders.erase(it);
                break;
            }
        }
    }

    void Logger::log(LogLevel ll, LogEvent::ptr event)
    {
        if(ll >= m_level)
        {
            auto self = shared_from_this();
            for(auto it : m_appenders)
            {
                it -> log(self, ll, event);
            }
        }
    }

    void Logger::info(LogEvent::ptr event){
        log(LogLevel::INFO, event);
    }
    void Logger::debug(LogEvent::ptr event){
        log(LogLevel::DEBUG, event);
    }
    void Logger::warn(LogEvent::ptr event){
        log(LogLevel::WARN, event);
    }
    void Logger::error(LogEvent::ptr event){
        log(LogLevel::ERROR, event);
    }
    void Logger::fatal(LogEvent::ptr event){
        log(LogLevel::FATAL, event);
    }    

    /**************************************************************/
    /* Class: LoggerManager */
    /**************************************************************/
    LoggerManager::LoggerManager()
    {
        m_root.reset(new Logger());
        m_root -> addAppender(LogAppender::ptr(new SoutLogAppender()));
        m_root -> addAppender(LogAppender::ptr(new FileLogAppender("./log.txt")));
    }

    Logger::ptr LoggerManager::getLogger(const std::string& name)
    {
        auto it = m_logger_map.find(name);
        return it == m_logger_map.end()? m_root : it -> second;
    }
}