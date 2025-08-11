#pragma once

#include <memory>

#include "bytearray.h"

namespace agent{
    class Stream{
    public:
        typedef std::shared_ptr<Stream> ptr;

        virtual ~Stream(){}


        virtual int read(void* buffer, size_t length) = 0;
        virtual int read(ByteArray::ptr ba, size_t length) = 0;
        virtual int readFixSize(void* buffer, size_t lenght);
        virtual int readFixSize(ByteArray::ptr ba, size_t length);

        virtual int write(const void* buffer, size_t length) = 0;
        virtual int write(ByteArray::ptr ba, size_t length) = 0;
        virtual int writeFixSize(const void* buffer, size_t length);
        virtual int writeFixsize(ByteArray::ptr ba, size_t length);

        virtual void close() = 0;
    };
}