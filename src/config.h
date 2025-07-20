#pragma once

#include <memory>
#include <sstream>
#include <map>
#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>
#include "log.h"

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

    protected:
        std::string m_name;
        std::string m_description;
    };

    // Config item
    template<typename T>
    class ConfigVar: public ConfigVarBase
    {
    public:
        using ptr = std::shared_ptr<ConfigVar>;
        
        ConfigVar(const std::string& name, const T& default_value, const std::string& description = "")
        :ConfigVarBase(name, description)
        ,m_val(default_value)
        {}

        std::string toString() override
        {
            try
            {
                return boost::lexical_cast<std::string>(m_val);
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
                m_val = boost::lexical_cast<T>(val);
                return true;
            }
            catch(std::exception& e)
            {
                AGENT_LOG_ERROR(AGENT_LOG_ROOT()) << "ConfigVar::fromString exception "
                << e.what() << " convert: string to " << typeid(m_val).name();
            }
            return false;
        }

        const T getValue() const {return m_val;}
        void setValue(const T& v) {m_val = v;}

    private:
        T m_val;
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
        static typename ConfigVar<T>::ptr Lookup(const std::string& s_name, const T& default_value, const std::string& description = "")
        {
            auto name = s_name;
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            auto tmp = Lookup<T>(name);
            if(tmp)
            { 
                AGENT_LOG_INFO(AGENT_LOG_ROOT()) << "Lookup name= " << "exist";
                return tmp;
            }
            if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos)
            {
                AGENT_LOG_ERROR(AGENT_LOG_ROOT()) << "Lookup name invalid " << name;
                throw std::invalid_argument(name);
            }
            typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
            m_data[name] = v;
            return v;
        }

        template<typename T>
        static typename ConfigVar<T>::ptr Lookup(const std::string& name)
        {
            auto it = m_data.find(name);
            if(it == m_data.end())
            {
                return nullptr;
            }
            return std::dynamic_pointer_cast<ConfigVar<T>>(it -> second);
        }

        
        static void LoadFromYaml(const YAML::Node& root);
        static ConfigVarBase::ptr LookupBase(const std::string& name);
        
    private:
        static ConfigVarMap m_data;
    };

}