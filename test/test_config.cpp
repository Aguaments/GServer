#include "log.h"
#include "config.h"
#include <iostream>
#include <yaml-cpp/yaml.h>

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

agent::ConfigVar<int>::ptr g_int_value_config = agent::Config::Lookup("System.port", (int)8080, "System port"); 
int main()
{
    auto root = YAML::LoadFile("./config/log.yml");
    print_yaml(root, 0);
    return 0;
}