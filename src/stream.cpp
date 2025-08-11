#include "stream.h"

namespace agent{
    int Stream::readFixSize(void* buffer, size_t length){
        size_t offset = 0;
        size_t rest_bytes = length;
        while(rest_bytes > 0){
            size_t len = read((char*)buffer + offset, rest_bytes);
            if(len <= 0){
                return len;
            }
            offset += len;
            rest_bytes -= len;
        }
        return length;
    }

    int Stream::readFixSize(ByteArray::ptr ba, size_t length){
        size_t rest_bytes = length;
        while(rest_bytes > 0){
            size_t len = read(ba, rest_bytes);
            if(len <= 0){
                return len;
            }
            rest_bytes -= len;
        }
        return length;
    }

    int Stream::writeFixSize(const void* buffer, size_t length){
        size_t offset = 0;
        size_t rest_bytes = length;
        while(rest_bytes > 0){
            size_t len = write((char*)buffer + offset, rest_bytes);
            if(len <= 0){
                return len;
            }
            offset += len;
            rest_bytes -= len;
        }
        return length;
    }

    int Stream::writeFixsize(ByteArray::ptr ba, size_t length){
        size_t rest_bytes = length;
        while(rest_bytes > 0){
            size_t len = write(ba, rest_bytes);
            if(len <= 0){
                return len;
            }
            rest_bytes -= len;
        }
        return length;
    }
}