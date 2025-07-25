#pragma once

namespace agent{
    template<typename T, typename X=void, int N=0>
    class Singleton
    {
    public:
        static T* getInstance()
        {
            static T instance;
            return &instance;
        }
    private:
        Singleton(){}
    };

    template<typename T, typename X=void, int N=0>
    class SingletonPtr
    {
    public:
        static std::shared_ptr<T> getInstance()
        {
            static std::shared_ptr<T> instancePtr(new T);
            return instancePtr;
        }
    private:
        SingletonPtr(){}
    };
}