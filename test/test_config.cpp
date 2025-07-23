#include "log.h"
#include "config.h"
#include <iostream>
#include <yaml-cpp/yaml.h>

#if 0
agent::ConfigVar<int>::ptr g_int_value_config = agent::Config::Lookup("system.port", (int)8080, "System port"); 
agent::ConfigVar<float>::ptr g_int_value_x_config = agent::Config::Lookup("system.port", (float)8080, "System port"); 


agent::ConfigVar<float>::ptr g_float_value_config = agent::Config::Lookup("system.value", (float)10.2f, "System value");
agent::ConfigVar<std::vector<int>>::ptr g_int_vector_value_config = agent::Config::Lookup("system.vec", std::vector<int>{1,2}, "System vector");
agent::ConfigVar<std::list<int>>::ptr g_int_list_value_config = agent::Config::Lookup("system.list", std::list<int>{1,2}, "System list");
agent::ConfigVar<std::set<int>>::ptr g_int_set_value_config = agent::Config::Lookup("system.set", std::set<int>{1,2}, "System set");
agent::ConfigVar<std::unordered_set<int>>::ptr g_int_unordered_set_value_config = agent::Config::Lookup("system.unordered_set", std::unordered_set<int>{1,2}, "System unordered_set");
agent::ConfigVar<std::map<std::string, int>>::ptr g_int_map_value_config = agent::Config::Lookup("system.map", std::map<std::string, int>{{"k", 2}}, "System map");
agent::ConfigVar<std::unordered_map<std::string, int>>::ptr g_int_unordered_map_value_config = agent::Config::Lookup("system.unordered_map", std::unordered_map<std::string, int>{{"k", 2}}, "System unordered_map");


void test_yaml()
{
    try{
        YAML::Node  root = YAML::LoadFile("../config/log.yml");
        std::cout << root.Type() << std::endl;
        AGENT_LOG_INFO(AGENT_LOG_ROOT()) << root;
    }
    catch(std::exception& e)
    {
        std::cout << e.what() <<std::endl;
        AGENT_LOG_INFO(AGENT_LOG_ROOT()) << e.what();
    }

}

void print_yaml(const YAML::Node& node, int level)
{
    if(node.IsScalar())
    {
        AGENT_LOG_INFO(AGENT_LOG_ROOT()) << std::string(level + 4, ' ')
            << node.Scalar() << " _ " << node.Type() << " _ " << level;
    }
    else if(node.IsMap())
    {
        for(auto it = node.begin(); it != node.end(); ++it)
        {
            AGENT_LOG_INFO(AGENT_LOG_ROOT()) << std::string(level + 4, ' ')
                << it -> first << " _ " << it -> second.Type() << " _ " << level;
            print_yaml(it -> second, level + 1);
        }
    }
    else if(node.IsSequence())
    {
        for(size_t i = 0; i < node.size(); ++i)
        {
            AGENT_LOG_INFO(AGENT_LOG_ROOT()) << std::string(level + 4, ' ')
                << i << " _ " << node[i].Type() << " _ " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

void test_config()
{
    AGENT_LOG_INFO(AGENT_LOG_ROOT()) << "before: " << g_int_value_config -> getValue();
    AGENT_LOG_INFO(AGENT_LOG_ROOT()) << "before: " << g_float_value_config -> getValue();
    #define XX(target, name, prefix)\
    {\
        auto vec =  target -> getValue();\
        for(auto& i : vec){\
            AGENT_LOG_INFO(AGENT_LOG_ROOT()) << #prefix " " #name " : " << i;\
        }\
        AGENT_LOG_INFO(AGENT_LOG_ROOT()) << #prefix " " #name " yaml: " << target -> toString();\
    }
    #define XX_M(target, name, prefix)\
    {\
        auto vec =  target -> getValue();\
        for(auto& i : vec){\
            AGENT_LOG_INFO(AGENT_LOG_ROOT()) << #prefix " " #name " : {" << i.first << ": " << i.second << "}";\
        }\
        AGENT_LOG_INFO(AGENT_LOG_ROOT()) << #prefix " " #name " yaml: " << target -> toString();\
    }

    XX(g_int_vector_value_config, int_vec, before);
    XX(g_int_list_value_config, int_list, before);
    XX(g_int_set_value_config, int_set, before);
    XX(g_int_unordered_set_value_config, int_unordered_set, before);
    XX_M(g_int_map_value_config, int_map, before);
    XX_M(g_int_unordered_map_value_config, int_unordered_map, before);

    YAML::Node root = YAML::LoadFile("../config/log.yml");
    agent::Config::LoadFromYaml(root);

    AGENT_LOG_INFO(AGENT_LOG_ROOT()) << "after: " << g_int_value_config -> getValue();
    AGENT_LOG_INFO(AGENT_LOG_ROOT()) << "after: " << g_float_value_config -> getValue();
    
    XX(g_int_vector_value_config, int_vec, after);
    XX(g_int_list_value_config, int_list, after);
    XX(g_int_set_value_config, int_set, after);
    XX(g_int_unordered_set_value_config, int_unordered_set, after);
    XX_M(g_int_map_value_config, int_map, after);
    XX_M(g_int_unordered_map_value_config, int_unordered_map, after);
}
#endif

class Person
{
public:
    Person(){};
    std::string m_name;
    int m_age = 0;
    bool m_sex = 0;

    std::string toString() const
    {
        std::stringstream ss;
        ss << "[Person name = " << m_name
        << " age = " << m_age
        << " sex = " << m_sex
        << "]";
        return ss.str();
    }

    bool operator==(const Person& oth) const
    {
        return m_name == oth.m_name && m_age == oth.m_age && m_sex == oth.m_sex;
    }
};

namespace agent{
    // std::string <----> Person
    template<>
    class LexicalCast<std::string, Person>
    {
    public:
        Person operator()(const std::string& v)
        {
            YAML::Node node = YAML::Load(v);
            Person p;
            p.m_name = node["name"].as<std::string>();
            p.m_age = node["age"].as<int>();
            p.m_sex = node["sex"].as<bool>();
            return p;
        }
    };

    template<>
    class LexicalCast<Person, std::string>
    {
    public:
        std::string operator()(const Person& v)
        {
            YAML::Node node; // 明确node是一个序列节点， 直接YAML::Node node;也可以，是隐式的
            node["name"] = v.m_name;
            node["age"] = v.m_age;
            node["sex"] = v.m_sex;
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };
}

agent::ConfigVar<Person>::ptr g_person = agent::Config::Lookup("class.person", Person(), "system person");
agent::ConfigVar<std::map<std::string, Person>>::ptr g_map_person = agent::Config::Lookup("class.map_person", std::map<std::string, Person>(), "system map person");

void test_class()
{
    g_person -> addListener(10, [](const Person& old_value, const Person& new_value){
        AGENT_LOG_INFO(AGENT_LOG_ROOT()) << "old_value=" << old_value.toString() << " new_value=" << new_value.toString();
    });
    AGENT_LOG_INFO(AGENT_LOG_ROOT()) << "before: " << g_person -> getValue().toString() << " - " << g_person -> toString();

    #define XX_PM(g_var, prefix)\
    {\
        auto m = g_var -> getValue();\
        for(auto& i : m)\
        {\
            AGENT_LOG_INFO(AGENT_LOG_ROOT()) << prefix << ": " << i.first << " - " << i.second.toString();\
        }\
    }
    
    XX_PM(g_map_person, "class.map before");

    
    
    YAML::Node root = YAML::LoadFile("../config/log.yml");
    // print_yaml(root, 0);
    agent::Config::LoadFromYaml(root);
    AGENT_LOG_INFO(AGENT_LOG_ROOT()) << "after: " << g_person -> getValue().toString() << " - " << g_person -> toString();
    XX_PM(g_map_person, "class.map after");
}

void test_log()
{
    static agent::Logger::ptr system_logger = AGENT_LOG_BY_NAME("root");
    AGENT_LOG_INFO(system_logger) << "hello system" << std::endl;
    std::cout << agent::LoggerMgr::getInstance() -> toYamlString() << std::endl;
    YAML::Node root = YAML::LoadFile("../config/log.yml");
    agent::Config::LoadFromYaml(root);
    std::cout << "==========================" << std::endl;
    std::cout << agent::LoggerMgr::getInstance() -> toYamlString() << std::endl;
    AGENT_LOG_INFO(system_logger) << "hello system" << std::endl;
}

int main()
{
    // test_config();
    // test_class();
    test_log();
    return 0;
}