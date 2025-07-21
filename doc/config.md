# 配置系统
通过配置文件，给日志系统配置输出格式、文件路径等
Config ---->  yaml文件解析

- 原则：约定大于配置

## 必要组件
- 安装boost库
```shell
sudo apt update
sudo apt install libboost-all-dev
```
- 安装yaml-ccp库
```shell
mkdir build
cd build
cmake ..
make install
```
- 常用的查询命令
```shell
find . *.h | xargs grep -inr load

# xargs：该命令可以把前面命令的返回值作为后面命令的参数
# find . *.h：可以递归查找当前目录下的.h文件
# grep：选线-inr，i表示忽略大小写，n表示打印查找到的行号，r表示递归查询(本例中可以不用)
```

- 模板

1. 例子
```c++
template<calss T, class FromStr, classToStr>
class ConfigVar{};

// 使用到了仿函数，转换的基本模板
template<class F, class T> 
class LexicalCast
{
public:
    T operator()(const F& v)
    {
        return boost::lexical_cast<T>(v);
    }
}；

// 转换的偏特化模板，vector的转化
template<class T>
class LexicalCast<std::string, std::vector<T>>
{
    T operator()(const std::string& v)
    {
        YAML::Node node = YAML::Load()
    }
}



// 容器偏特化，支持vector
```

2. 思路
- 最初思路：只能将基本类型的值进行字符串化，使用lexical_cast进行转换，有局限性，对于复杂类型无法适应
- 当前思路：为了支持复杂类型的转化，通过使用模板来实现。例如针对vector类型的转换，进行偏特化声明，这样后期在实例化一个```std::vector<int>```类型的对象时，就可以实例化偏特化的模板，而不会使用原本的基础模板。