#pragma once

#include <memory>
#include <sstream>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <functional>

#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>

#include "log.h"
#include "thread.h"

typedef agent::RWMutex ConfigMutexType; 

namespace agent{
    class ConfigVarBase
    {
    public:
        using ptr = std::shared_ptr<ConfigVarBase>;
        
        ConfigVarBase(const std::string& name, const std::string& description = "")
        :m_name(name)
        ,m_description(description)
        {
            std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
        }
        virtual ~ConfigVarBase(){}

        const std::string& getName() const {return m_name;}
        const std::string& getDescription() const {return m_description;}

        virtual std::string toString() = 0;
        virtual bool fromString(const std::string& val) = 0; // 解析
        virtual std::string getTypeName() const = 0;

    protected:
        std::string m_name;
        std::string m_description;
    };

    /****************************************************************************
    * @class LexicalCast
    * @brief 复杂类型转换基本模板
    * @author Guanyue Gao
    * @since 2025-07-21 18:03:47
    ****************************************************************************/
    template<class F,class T>
    class LexicalCast
    {
    public:
        T operator()(const F& v)
        {
            return boost::lexical_cast<T>(v);
        }
    };

    // std::string <-----> std::vector
    template<class T>
    class LexicalCast<std::string, std::vector<T>>
    {
    public:
        std::vector<T> operator()(const std::string& v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::vector<T> vec;
            std::stringstream ss;
            for(size_t i = 0; i < node.size(); ++i)
            {
                ss.str("");
                ss << node[i];
                vec.push_back(LexicalCast<std::string ,T>()(ss.str()));
            }
            return vec;
        }
    };

    template<class T>
    class LexicalCast<std::vector<T>, std::string>
    {
    public:
        std::string operator()(const std::vector<T>& v)
        {
            YAML::Node node(YAML::NodeType::Sequence); // 明确node是一个序列节点， 直接YAML::Node node;也可以，是隐式的
            for(auto& i : v)
            {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            std::cout << ss.str() << std::endl;
            return ss.str();
        }
    };

    // std::string <----> std::list
    template<class T>
    class LexicalCast<std::string, std::list<T>>
    {
    public:
        std::list<T> operator()(const std::string& v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::list<T> vec;
            std::stringstream ss;
            for(size_t i = 0; i < node.size(); ++i)
            {
                ss.str("");
                ss << node[i];
                vec.push_back(LexicalCast<std::string ,T>()(ss.str()));
            }
            return vec;
        }
    };

    template<class T>
    class LexicalCast<std::list<T>, std::string>
    {
    public:
        std::string operator()(const std::list<T>& v)
        {
            YAML::Node node(YAML::NodeType::Null); // 明确node是一个序列节点， 直接YAML::Node node;也可以，是隐式的
            for(auto& i : v)
            {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // std::string <----> std::set
    template<class T>
    class LexicalCast<std::string, std::set<T>>
    {
    public:
        std::set<T> operator()(const std::string& v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::set<T> vec;
            std::stringstream ss;
            for(size_t i = 0; i < node.size(); ++i)
            {
                ss.str("");
                ss << node[i];
                vec.insert(LexicalCast<std::string ,T>()(ss.str()));
            }
            return vec;
        }
    };

    template<class T>
    class LexicalCast<std::set<T>, std::string>
    {
    public:
        std::string operator()(const std::set<T>& v)
        {
            YAML::Node node(YAML::NodeType::Null); // 明确node是一个序列节点， 直接YAML::Node node;也可以，是隐式的
            for(auto& i : v)
            {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // std::string <----> std::unordered_set
    template<class T>
    class LexicalCast<std::string, std::unordered_set<T>>
    {
    public:
        std::unordered_set<T> operator()(const std::string& v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::unordered_set<T> vec;
            std::stringstream ss;
            for(size_t i = 0; i < node.size(); ++i)
            {
                ss.str("");
                ss << node[i];
                vec.insert(LexicalCast<std::string ,T>()(ss.str()));
            }
            return vec;
        }
    };

    template<class T>
    class LexicalCast<std::unordered_set<T>, std::string>
    {
    public:
        std::string operator()(const std::unordered_set<T>& v)
        {
            YAML::Node node(YAML::NodeType::Null); // 明确node是一个序列节点， 直接YAML::Node node;也可以，是隐式的
            for(auto& i : v)
            {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // std::string <----> std::map
    template<class T>
    class LexicalCast<std::string, std::map<std::string, T>>
    {
    public:
        std::map<std::string, T> operator()(const std::string& v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::map<std::string, T> vec;
            std::stringstream ss;
            for(auto it = node.begin(); it != node.end(); ++ it)
            {
                ss.str("");
                ss << it -> second;
                vec.insert(std::make_pair(it -> first.Scalar(), LexicalCast<std::string ,T>()(ss.str())));
            }
            return vec;
        }
    };

    template<class T>
    class LexicalCast<std::map<std::string, T>, std::string>
    {
    public:
        std::string operator()(const std::map<std::string, T>& v)
        {
            YAML::Node node(YAML::NodeType::Map); //直接YAML::Node node;也可以，是隐式的
            for(auto& i : v)
            {
                node[i.first] = YAML::Load(LexicalCast<T,std::string>()(i.second));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // std::string <----> std::unordered_map
    template<class T>
    class LexicalCast<std::string, std::unordered_map<std::string, T>>
    {
    public:
        std::unordered_map<std::string, T> operator()(const std::string& v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::unordered_map<std::string, T> vec;
            std::stringstream ss;
            for(auto it = node.begin(); it != node.end(); ++ it)
            {
                ss.str("");
                ss << it -> second;
                vec.insert(std::make_pair(it -> first.Scalar(), LexicalCast<std::string ,T>()(ss.str())));
            }
            return vec;
        }
    };

    template<class T>
    class LexicalCast<std::unordered_map<std::string, T>, std::string>
    {
    public:
        std::string operator()(const std::unordered_map<std::string, T>& v)
        {
            YAML::Node node(YAML::NodeType::Map); //直接YAML::Node node;也可以，是隐式的
            for(auto& i : v)
            {
                node[i.first] = YAML::Load(LexicalCast<T,std::string>()(i.second));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    /****************************************************************************
    * @class 
    * @brief 配置条目类的定义
    * @author Guanyue Gao
    * @since 2025-07-21 18:02:48
    ****************************************************************************/
    template<typename T, class FromStr = LexicalCast<std::string, T>, class ToStr = LexicalCast<T, std::string>>
    class ConfigVar: public ConfigVarBase
    {
    public:
        using ptr = std::shared_ptr<ConfigVar>;
        using on_change_cb = std::function<void(const T& old_value, const T& new_value)>;
        
        ConfigVar(const std::string& name, const T& default_value, const std::string& description = "")
        :ConfigVarBase(name, description)
        ,m_val(default_value)
        {}

        std::string toString() override
        {
            try
            {
                ConfigMutexType::ReadLock lock(m_mutex);
                // return boost::lexical_cast<std::string>(m_val);
                return ToStr()(m_val);
            }
            catch(std::exception& e)
            {
                AGENT_LOG_ERROR(AGENT_LOG_ROOT()) << "ConfigVar::toString exception "
                << e.what() << " convert: " << typeid(m_val).name() << " to string";
            }
            return "";
        }
    
        bool fromString(const std::string& val) override
        {
            try
            {
                ConfigMutexType::WriteLock lock(m_mutex);
                // m_val = boost::lexical_cast<T>(val);
                // return true;
                setValue(FromStr()(val));
                return true;
            }
            catch(std::exception& e)
            {
                AGENT_LOG_ERROR(AGENT_LOG_ROOT()) << "ConfigVar::fromString exception "
                << e.what() << " convert: string to " << typeid(m_val).name();
            }
            return false;
        }

        const T getValue() const // 锁在函数内部是会变化的因此，需要将m_mutex设置为mutable
        {
            ConfigMutexType::ReadLock lock(m_mutex);
            return m_val;
        }
        void setValue(const T& v) 
        {
            {
                ConfigMutexType::ReadLock lock(m_mutex);
                if(v == m_val)
                {
                    return;
                }
                
                for(auto& i : m_cbs) // 触发变更的回调函数
                {
                    i.second(m_val, v);
                }
            }   
            ConfigMutexType::WriteLock lock(m_mutex);
            m_val = v;
        }
        std::string getTypeName() const override {return typeid(T).name();}

        uint64_t addListener(on_change_cb cb)
        {
            static uint64_t s_fun_id = 0;
            ConfigMutexType::WriteLock lock(m_mutex);
            ++s_fun_id;
            m_cbs[s_fun_id] = cb;
            return s_fun_id;
        }

        void delListerner(uint64_t key)
        {
            ConfigMutexType::WriteLock lock(m_mutex);
            m_cbs.erase(key);
        }

        on_change_cb getListener(uint64_t key)
        {
            ConfigMutexType::ReadLock lock(m_mutex);
            auto it = m_cbs.find(key);
            return it == m_cbs.end()? nullptr: it -> second;
        }

        void clearListener()
        {
            m_cbs.clear();
        }

    private:
        T m_val;
        // 变更回调函数数组，uint64_t key，可以使用hash值
        std::map<u_int64_t, on_change_cb> m_cbs;

        mutable ConfigMutexType m_mutex;
    };

    /****************************************************************************
    * @class 
    * @brief 配置器部分
    * @author Guanyue Gao
    * @since 2025-07-20 23:56:18
    ****************************************************************************/
    class Config
    {
    public:
        using ConfigVarMap = std::map<std::string, ConfigVarBase::ptr>;
        /****************************************************************************
        * @name 
        * @brief Lookup函数主要用于查询配置项，如果存在则返回，不存在则创建
        *****************************************************************************/
        template<typename T>
        static typename ConfigVar<T>::ptr Lookup(const std::string& name, const T& default_value, const std::string& description = "")
        {
            ConfigMutexType::WriteLock lock(GetMutex());
            auto it = GetDatas().find(name);
            if(it != GetDatas().end())
            {
                auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it -> second);
                if(tmp)
                {
                    AGENT_LOG_INFO(AGENT_LOG_ROOT()) << "Lookup name = " << name << " exists.";
                    return tmp;
                }
                else
                {
                    AGENT_LOG_ERROR(AGENT_LOG_ROOT()) << "Lookup name = " << name << " exists but type is incorrect ("
                    << typeid(T).name() << "). real_type = " << it -> second -> getTypeName()
                    << " " << it -> second -> toString();
                    return nullptr;
                }
            }
            if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos)
            {
                AGENT_LOG_ERROR(AGENT_LOG_ROOT()) << "Lookup name invalid " << name;
                throw std::invalid_argument(name);
            }
            typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
            GetDatas()[name] = v;
            return v;
        }

        template<typename T>
        static typename ConfigVar<T>::ptr Lookup(const std::string& name)
        {
            ConfigMutexType::ReadLock lock(GetMutex());
            auto it = GetDatas().find(name);
            if(it == GetDatas().end())
            {
                return nullptr;
            }
            return std::dynamic_pointer_cast<ConfigVar<T>>(it -> second);
        }

        
        static void LoadFromYaml(const YAML::Node& root);
        static ConfigVarBase::ptr LookupBase(const std::string& name);

        static void Visit(std::function<void(ConfigVarBase::ptr&)> cb);
        
    private:
        // static ConfigVarMap m_data;
        static ConfigVarMap& GetDatas()
        {
            static ConfigVarMap s_datas;
            return s_datas;
        }

        static ConfigMutexType& GetMutex()
        {
            static ConfigMutexType s_mutex;
            return s_mutex;
        }
    };
}