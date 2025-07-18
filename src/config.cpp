#include "config.h"

namespace agent{
    Config::ConfigVarMap Config::m_data;

    static void ListAllMember(const std::string& prefix, 
        const YAML::Node& node, 
        std::list<std::pair<std::string, const YAML::Node>>& output)
    {
        
    }

    void Config::LoadFromYaml(const YAML::Node& root)
    {
        std::list<std::pair<std::string, const YAML::Node>> all_nodes;
        ListAllMember("", root, all_nodes);
    }
}