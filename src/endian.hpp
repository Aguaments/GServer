#pragma once

#include <type_traits>
#include <byteswap.h>
#include <stdint.h>

// 网络字节序转换

#define AGENT_LIMITE_ENDIAN 1
#define AGENT_BIG_ENDIAN 2

namespace agent{
    template<class T>
    typename std::enable_if<std::is_same<T, uint64_t>::value, T>::type
    byteswap(T value){
        return (T)bswap_64((uint64_t)value);
    }

    template<class T>
    typename std::enable_if<std::is_same<T, uint32_t>::value, T>::type
    byteswap(T value){
        return (T)bswap_32((uint64_t)value);
    }

    template<class T>
    typename std::enable_if<std::is_same<T, uint16_t>::value, T>::type
    byteswap(T value){
        return (T)bswap_16((uint64_t)value);
    }

    #if BYTE_ORDER == BIG_ENDIAN
    #define AGENT_BYTE_ORDER AGENT_BIG_ENDIAN
    #else
    #define AGENT_BYTE_ORDER AGENT_LIMITE_ENDIAN
    #endif
    
    #if AGENT_BYTE_ORDER == AGENT_BIG_ENDIAN
    template<class T>
    T byteswapOnLittleEndian(T t){
        return t;
    }

    template<class T>
    T byteswapOnBigEndian(T t){
        return byteswap(t);
    }

    #else
    template<class T>
    T byteswapOnLittleEndian(T t){
        return byteswap(t);
    }

    template<class T>
    T byteswapOnBigEndian(T t){
        return t;
    }
    #endif
}