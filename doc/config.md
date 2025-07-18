# 配置系统
通过配置文件，给日志系统配置输出格式、文件路径等
Config ---->  yaml文件解析

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