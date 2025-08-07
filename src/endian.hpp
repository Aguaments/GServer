#pragma once

#include <type_traits>
#include <byteswap.h>
#include <cstdint>

// 网络字节序转换

#define AGENT_LIMITE_ENDIAN 1
#define AGENT_BIG_ENDIAN 2

namespace agent{

    template<typename T>
    struct byteswapImpl;

    template<>
    struct byteswapImpl<uint16_t>{
        static uint16_t swap(uint16_t val){
            return __builtin_bswap16(val);
        }
    };

    template<>
    struct byteswapImpl<int16_t>{
        static int16_t swap(int16_t val){
            return static_cast<int16_t>(__builtin_bswap16(static_cast<uint16_t>(val)));
        }
    };

    template<>
    struct byteswapImpl<uint32_t>{
        static uint32_t swap(uint32_t val){
            return __builtin_bswap32(val);
        }
    };
    
    template<>
    struct byteswapImpl<int32_t>{
        static int32_t swap(int32_t val){
            return static_cast<int32_t>(__builtin_bswap32(static_cast<uint32_t>(val)));
        }
    };

    template<>
    struct byteswapImpl<uint64_t>{
        static uint64_t swap(uint64_t val){
            return __builtin_bswap64(val);
        }
    };

    template<>
    struct byteswapImpl<int64_t>{
        static int64_t swap(int64_t val){
            return static_cast<int64_t>(__builtin_bswap64(static_cast<uint64_t>(val)));
        }
    };

    template<typename T>
    T byteswap(T x) {
        static_assert(std::is_integral<T>::value, "Only integral types supported");
        return byteswapImpl<T>::swap(x);
    }

    // template<class T>
    // typename std::enable_if<sizeof(T) == 64, uint64_t>::type
    // byteswap(T value){
    //     return (T)bswap_64((uint64_t)value);
    // }

    // template<class T>
    // typename std::enable_if<sizeof(T) == 32, uint32_t>::type
    // byteswap(T value){
    //     return (T)bswap_32((uint32_t)value);
    // }

    // template<class T>
    // typename std::enable_if<sizeof(T) == 16, uint16_t>::type
    // byteswap(T value){
    //     return (T)bswap_16((uint16_t)value);
    // }

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