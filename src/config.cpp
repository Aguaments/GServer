#include "config.h"

namespace agent{
    Config::ConfigVarMap Config::m_data;

    ConfigVarBase::ptr Config::LookupBase(const std::string& name)
    {
        auto it = m_data.find(name);
        return it == m_data.end() ? nullptr: it -> second; 
    }

    static void ListAllMember(const std::string& prefix, 
        const YAML::Node& node, 
        std::list<std::pair<std::string, const YAML::Node>>& output)
    {
        // find_first_not_of函数：找到prefix中第一个不在字符集中的字符的位置（不在字符集中的字符在原字符串中的位置），如果都在字符集中则返回std::string::npos
        // 对于空字符串""也直接返回npos
        if(prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._123456789") != std::string::npos)
        {
            AGENT_LOG_ERROR(AGENT_LOG_ROOT()) << "Config invalid name: " << prefix << " : " << node;
            return;
        }
        output.push_back(std::make_pair(prefix, node));
        if(node.IsMap())
        {
            for(auto it = node.begin(); it != node.end(); ++ it)
            {
                ListAllMember(prefix.empty() ? it -> first.Scalar() 
                    : prefix + "." + it -> first.Scalar(), it -> second, output);
            }
        }
        
    }

    /*
        description：从解析好的yaml节点中提取数据
    */
    void Config::LoadFromYaml(const YAML::Node& root)
    {
        std::list<std::pair<std::string, const YAML::Node>> all_nodes;
        ListAllMember("", root, all_nodes);

        for(auto& i : all_nodes)
        {
            std::string key = i.first; 
            if(key.empty())
            {
                continue;
            }

            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            ConfigVarBase::ptr var = LookupBase(key);

            if(var)
            {
                if(i.second.IsScalar())
                {
                    var -> fromString(i.second.Scalar());
                }
                else
                {
                    std::stringstream ss;
                    ss << i.second;
                    var -> fromString(ss.str());
                }
            }
        }
    }
}