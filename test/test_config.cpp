#include "log.h"
#include "config.h"
#include <iostream>
#include <yaml-cpp/yaml.h>

agent::ConfigVar<int>::ptr g_int_value_config = agent::Config::Lookup("System.port", (int)8080, "System port"); 
agent::ConfigVar<float>::ptr g_float_value_config = agent::Config::Lookup("System.value", (float)10.2f, "System value");


void test_yaml()
{
    try{
        YAML::Node  root = YAML::LoadFile("./config/log.yml");
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

    YAML::Node root = YAML::LoadFile("./config/log.yml");
    agent::Config::LoadFromYaml(root);

    AGENT_LOG_INFO(AGENT_LOG_ROOT()) << "after: " << g_int_value_config -> getValue();
    AGENT_LOG_INFO(AGENT_LOG_ROOT()) << "after: " << g_float_value_config -> getValue();
}


int main()
{
    test_config();
    return 0;
}