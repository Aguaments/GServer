#pragma once

#include <memory>

#include "http11_parser.h"
#include "httpclient_parser.h"
#include "http.h"

namespace agent{
    namespace http{
        class HttpRequestParser{
        public:
            using ptr = std::shared_ptr<HttpRequestParser>;
            HttpRequestParser();

            int isFinished();
            int hasError();
            size_t execute(char* data, size_t len, size_t off);

            HttpRequest::ptr getData() const { return m_data;}
            void setError(int v) { m_error = v;}
            uint64_t getContentLength();
            const http_parser& getParser() const { return m_parser;}


            static uint64_t GetHttpRequestBufferSize();
            static uint64_t GetHttpRequestMaxBodySize();

        private:
            http_parser m_parser;
            HttpRequest::ptr m_data;
            int m_error;
        };

        class HttpResponseParser{
        public:
            using ptr = std::shared_ptr<HttpResponseParser>; 
            HttpResponseParser();

            int isFinished();
            int hasError();
            size_t execute(char* data, size_t len, bool chunck);

            HttpResponse::ptr getData() const { return m_data;}
            void setError(int v) { m_error = v;}
            uint64_t getContentLength();
            const httpclient_parser& getParser() const { return m_parser;}

            static uint64_t GetHttpResponseBufferSize();
            static uint64_t GetHttpResponseMaxBodySize();

        private:
            httpclient_parser m_parser;
            HttpResponse::ptr m_data;
            int m_error;
        };
    }
}