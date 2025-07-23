#include <iostream>
#include <tuple>
#include <map>
#include <ctime>
#include <functional>
#include <string.h>

#include <stdarg.h>

#include "log.h"
#include "config.h"

// #define PARSE_TEST
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

    const LogLevel FromStringToLogLevel(const std::string& str)
    {
        #define XX(level, v) \
            if(str == #v) \
            {\
                return LogLevel::level;\
            }
        XX(DEBUG, debug);
        XX(INFO, info);
        XX(WARN, warn);
        XX(ERROR, error);
        XX(FATAL, fatal);

        XX(DEBUG, DEBUG);
        XX(INFO, INFO);
        XX(WARN, WARN);
        XX(ERROR, ERROR);
        XX(FATAL, FATAL);
        return LogLevel::UNKONWN;
        #undef XX
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
            os << event -> getLogger() -> getName();
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
                #ifdef PARSE_TEST
                std::cout << nstr << " | " << "null" << " | " << 2 << std::endl;
                #endif
                nstr.clear();
            }

            int m = i + 1;

            std::string symbol;
            std::string fmt;

            int fmt_status = 0; // 进入{}的状态位，目的是为了解析出{}内的格式化串，0表示未进入，1表示进入
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
                    #ifdef PARSE_TEST
                    std::cout << symbol << " | " << fmt << " | " << 1 << std::endl;
                    #endif
                }
                else
                {
                    vec.push_back(std::make_tuple(symbol, "", 0));
                    #ifdef PARSE_TEST
                    std::cout << symbol << " | " << fmt << " | " << 0 << std::endl;
                    #endif
                }
            }
            i = m;
        }
    
        // 将formatter中解析出来的类型绑定对应的解析类，通过解析类创建解析对象，进而实现格式化
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
                    m_error = true; // 出现formatter解析错误
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

    std::string SoutLogAppender::toYamlString()
    {
        YAML::Node node;
        node["type"] = "SoutLogAppender";
        node["level"] = LogLevelToString(m_level);
        //std::cout << "============ SoutLogAppender log level ===================="<< LogLevelToString(m_level)<< std::endl;
        if(m_formatter)
        {
            node["formatter"] = m_formatter -> getPattern();
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    /**************************************************************/
    /* Class: FileLogAppender */
    /**************************************************************/
    FileLogAppender::FileLogAppender(const std::string filename)
    :m_filename(filename)
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

    std::string FileLogAppender::toYamlString()
    {
        YAML::Node node;
        node["type"] = "FileLogAppender";
        node["file"] = m_filename;
        node["level"] = LogLevelToString(m_level);
        if(m_formatter)
        {
            node["formatter"] = m_formatter -> getPattern();
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    /**************************************************************/
    /* Class: Logger */
    /**************************************************************/
    Logger::Logger(const std::string& name)
    :m_name(name)
    ,m_level(LogLevel::DEBUG)
    {
        m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%C%T%[%p]%T(%f:%l)%T%m%n"));

        // if(name == "root")
        // {
        //     m_appenders.push_back(SoutLogAppender::ptr(new SoutLogAppender));
        // }
    }

    void Logger::addAppender(LogAppender::ptr appender)
    {
        if(!appender -> getFormatter())
        {
            appender -> setFormatter(m_formatter);
        }
        m_appenders.push_back(appender);
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

    void Logger::clearAppenders()
    {
        m_appenders.clear();
    }

    void Logger::setFormatter(LogFormatter::ptr val)
    {
        m_formatter = val;
    }

    void Logger::setFormatter(const std::string& val)
    {
        LogFormatter::ptr new_val(new LogFormatter(val));
        if(new_val -> isError()) return;
        m_formatter = new_val;
    }

    // 通用日志打印方法
    void Logger::log(LogLevel ll, LogEvent::ptr event)
    {
        if(ll >= m_level)
        {
            auto self = shared_from_this();
            if(!m_appenders.empty())
            {
                for(auto it : m_appenders)
                {
                    it -> log(self, ll, event);
                }
            }
            else if(m_root) // root logger自身没有这个内容，不会给自身成员变量赋值为自身
            {
                m_root -> log(ll, event);
            }
            
        }
    }

    // 按级别打印日志
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

    std::string Logger::toYamlString()
    {
        YAML::Node node;
        node["name"] = m_name;
        node["level"] = LogLevelToString(m_level);
        //std::cout << "============ logger log level ===================="<< LogLevelToString(m_level)<< std::endl;
        if(m_formatter)
        {
            node["formatter"] = m_formatter -> getPattern();
        }

        for(auto& i: m_appenders)
        {
            node["appenders"].push_back(YAML::Load(i -> toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    /**************************************************************/
    /* Class: LoggerManager */
    /**************************************************************/
    LoggerManager::LoggerManager() // 声明为了Logger的友元类了，所以可以访问私有成员对象
    {
        m_root.reset(new Logger());
        m_root -> addAppender(LogAppender::ptr(new SoutLogAppender()));
        m_root -> addAppender(LogAppender::ptr(new FileLogAppender("../log/log.txt")));

        m_logger_map[m_root -> m_name] = m_root;
        init();
    }

    Logger::ptr LoggerManager::getLogger(const std::string& name)
    {
        auto it = m_logger_map.find(name);
        if(it != m_logger_map.end())
        {
            return it -> second;
        }
        Logger::ptr logger(new Logger(name));
        logger -> m_root = m_root;
        m_logger_map[name] = logger;
        return logger;
    }

    std::string LoggerManager::toYamlString() const
    {
        YAML::Node node;
        for(auto& i: m_logger_map)
        {
            node.push_back(YAML::Load(i.second -> toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    /****************************************************************************
    * @struct LogAppenderDefine 
    * @brief 
    * @author Guanyue Gao
    * @since 2025-07-23 17:43:51
    ****************************************************************************/
    struct LogAppenderDefine
    {
        int type = 0; // 1. File 2. Sout
        LogLevel level = LogLevel::UNKONWN;
        std::string formatter;
        std::string file;

        bool operator==(const LogAppenderDefine& oth) const
        {
            return type == oth.type
                && level == oth.level
                && formatter == oth.formatter
                && file == oth.file;
        }
    };

    struct LogDefine
    {
        std::string name;
        LogLevel level = LogLevel::UNKONWN;
        std::string formatter;

        std::vector<LogAppenderDefine> appenders;

        bool operator==(const LogDefine& oth) const
        {
            return name == oth.name
                && level == oth.level
                && formatter == oth.formatter
                && appenders == oth.appenders;
        }

        bool operator<(const LogDefine& oth) const
        {
            return name < oth.name;
        }
    };

    // std::string <-----> set<LogDefine>
    template<>
    class LexicalCast<std::string, std::set<LogDefine>>
    {
    public:
        std::set<LogDefine> operator()(const std::string& v)
        {
            YAML::Node node = YAML::Load(v);
            std::set<LogDefine> vec;
            std::stringstream ss;
            for(size_t i = 0; i < node.size(); ++i)
            {
                const auto& n = node[i];
                if(!n["name"].IsDefined())
                {
                    std::cout << "log config error: name is null, " << n << std::endl;
                    continue;
                }

                LogDefine ld;
                ld.name = n["name"].as<std::string>();
                ld.level = FromStringToLogLevel(n["level"].IsDefined()? n["level"].as<std::string>(): "");
                if(n["formatter"].IsDefined())
                {
                    ld.formatter = n["formatter"].as<std::string>();
                }

                if(n["appenders"].IsDefined())
                {
                    for(size_t x = 0; x < n["appenders"].size(); ++x)
                    {
                        auto a = n["appenders"][x];
                        if(!a["type"].IsDefined())
                        {
                            std::cout << "log config error: appender type is null, " << a << std::endl;
                            continue;
                        }
                        std::string type = a["type"].as<std::string>();
                        LogAppenderDefine lad;
                        if(type == "FileLogAppender")
                        {
                            lad.type = 1;
                            if(!a["file"].IsDefined())
                            {
                                std::cout << "Log config error: fileappender is invalid, " << a << std::endl;
                                continue;
                            }
                            lad.file = a["file"].as<std::string>();
                            if(a["formatter"].IsDefined())
                            {
                                lad.formatter = a["formatter"].as<std::string>();
                            }
                        }
                        else if(type == "SoutLogAppender")
                        {
                            lad.type = 2;
                        }
                        else
                        {
                            std::cout << "Log config error: appender type is invalid, " << a << std::endl;
                            continue;
                        }
                        lad.level = FromStringToLogLevel(a["level"].IsDefined()? a["level"].as<std::string>(): "");
                        ld.appenders.push_back(lad);
                    }
                }
                vec.insert(ld);
            }
            return vec;
        }
    };

    template<>
    class LexicalCast<std::set<LogDefine>, std::string>
    {
    public:
        std::string operator()(const std::set<LogDefine>& v)
        {
            YAML::Node node; // 明确node是一个序列节点， 直接YAML::Node node;也可以，是隐式的
            for(auto& i: v)
            {
                YAML::Node n;
                n["name"] = i.name;
                n["level"] = LogLevelToString(i.level);
                if(!i.formatter.empty())
                {
                    n["formatter"] = i.formatter;
                }
                for(auto& a: i.appenders)
                {
                    YAML::Node na;
                    if(a.type == 1)
                    {
                        na["type"] = "FileLogAppender";
                        na["file"] = a.file;
                    }
                    else if(a.type == 2)
                    {
                        na["type"] = "SoutLogAppender";
                    }
                    na["level"] = LogLevelToString(a.level);

                    if(!a.formatter.empty())
                    {
                        na["formatter"] = a.formatter;
                    }
                    n["appenders"].push_back(na);
                }
                node.push_back(n);
            }
            std::stringstream ss;
            ss << node; 
            return ss.str();
        }
    };

    agent::ConfigVar<std::set<LogDefine>>::ptr g_log_defines = agent::Config::Lookup("logs", std::set<LogDefine>(), "log config");


    struct LogIniter
    {
        LogIniter()
        {
            g_log_defines -> addListener(0xF2E231, [](const std::set<LogDefine>& old_value,
                const std::set<LogDefine>& new_value){
                AGENT_LOG_INFO(AGENT_LOG_ROOT()) << "on_logger_conf_changed";
                for(auto& i: new_value)
                {
                    auto it = old_value.find(i);
                    Logger::ptr logger;
                    if(it == old_value.end())
                    {
                        // 新增logger逻辑
                        logger = AGENT_LOG_BY_NAME(i.name);
                    }
                    else
                    {
                        if(!(i == *it))
                        {
                            // 修改logger逻辑
                            logger = AGENT_LOG_BY_NAME(i.name);
                        }
                    }
                    logger -> setLevel(i.level);
                    if(!i.formatter.empty())
                    {
                        logger -> setFormatter(i.formatter);
                    }
                    logger -> clearAppenders();
                    for(auto& a : i.appenders)
                    {
                        LogAppender::ptr ap;
                        if(a.type == 1)
                        {
                            ap.reset(new FileLogAppender(a.file));
                        }
                        else if(a.type == 2)
                        {
                            ap.reset(new SoutLogAppender);
                        }
                        ap -> setLevel(a.level);
                        logger -> addAppender(ap);
                    }
                }
                
                // 删除
                for(auto & i: old_value)
                {
                    auto it = new_value.find(i);
                    if(it == new_value.end())
                    {
                        // 删除logger
                        auto logger = AGENT_LOG_BY_NAME(i.name);
                        logger -> setLevel((LogLevel)100);
                        logger -> clearAppenders();
                    }
                }
            });
        }
    };

    static LogIniter __log_init;

    void LoggerManager::init()
    {

    }
}