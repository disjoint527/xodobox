#include <include/epoll.hpp>

struct TcpSocket : Epoll::Interface
{
    virtual void process(uint32_t events) override
    {
        auto data = buffer + length;
        auto size = sizeof(buffer) - length;

        if(auto new_size = recv(fd, data, size, 0); new_size > 0)
        {
            length = length + new_size;
        }
        else
        {
            length = 0;
            async_connect(....);
        }
    }
    void connect()
    {
    }

    static void async_connect(TcpSocket * pThis)
    {
        /*
        sockaddr_in addr = { AF_INET, htons(pThis->port), reinterpret_cast<const in_addr&>(pThis->ipaddr) };
        if( 0 == connect(pThis->fd, .... ))
        {
            std::cout << ipaddr << ':' << port << std::endl;
        }
        pthis->selector.add(pThis->fd, pThis);
        // http request : yunhq.sse.com.cn/.....
        */
    }
    auto send(const char * data, size_t size)
    {
        auto status = ::send(fd, data, size,  MSG_DONTWAIT | MSG_NOSIGNAL);
        if(status <= 0) { connect(); return false; }
        else return true;
    }

    TcpSocket(Epoll& selector, uint32_t _ipaddr, uint32_t _port) : selector_(selector)
    {
        fd = socket(AF_INET,SOCK_STREAM,0);
    }

    Epoll& selector_;

    int fd;
    uint32_t ipaddr;
    uint32_t port;
    char buffer[2*1024*1024];
    int  length = 0;


};

struct HttpSocket : TcpSocket
{
    virtual void process_http_data(const char* data, size_t size) = 0;

    auto GET(std::string hostname, std::string url)
    {
        std::string request = "GET " + url + " HTTP/1.1\r\n";
        request += "Host: " + hostname + "\r\n";
        request += "Connection: keep-alive\r\n" + 
                   "DNT: 1\r\n" +
                   "Upgrade-Insecure-Requests: 1\r\n" +
                   "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/96.0.4664.110 Safari/537.36\r\n" +
                   "Accept: text/html,application/xhtml+xml,application/xml\r\n" +
                   //"Accept-Encoding: gzip, deflate\r\n"
                   "Accept-Encoding: identity\r\n"
                   "Accept-Language: zh-CN,zh;q=0.9,zh-TW;q=0.8,en-US;q=0.7,en;q=0.6\r\n";
        TcpSocket::send(request.c_str(), request.size());
    }

    virtual void process(uint32_t events) override
    {
        TcpSocket::process(events);
        if(TcpSocket::length > 0)
        {
            auto payload = std::string(TcpSocket::buffer, TcpSocket::length);
            if(auto header_end = payload.find("\r\n\r\n"); header != std::string::npos)
            {
                auto header = std::string(TcpSocket::buffer, header_end + 4);
                if(auto length_start = header.find("Content-Length:"); length_start!= std::string::npos)
                {
                    if(auto length_end = header.find("\r\n", length_start); length_end != std::string::npos)
                    {
                        auto len = std::string(header.c_str() + length_start, length_end - length_start);
                        auto content_length = atoi(len.c_str());
                        if(TcpSocket::length >= (header.size() + content_length))
                        {
                            process_http_data(TcpSocket::buffer + header.size(), content_length);
                            auto discard_len = header.size() + content_length;
                            memmove(TcpSocket::buffer + 0, TcpSocket::buffer + discard_len, TcpSocket::length - discard_len);
                        }
                    }
                }
            }
        }
    }
};

struct Websocket : HttpSocket
{
    virtual void process(uint32_t events) override;
};


struct yunhq_socket : HttpSocket
{
    virtual void process_http_data(const char* data, size_t size) override
    {
        auto js=parse(std::string(data, size));
        std::vector<DataFile::Row> array;
        for(auto& x : js["list"])
        {
            auto timestamp = x[0].get<int>();
            auto last = x[1].get<float>();
            auto volume = x[2].get<double>();
            auto amount = x[3].get<double>()o;
            //array.push_back({ timestamp, last, volume, amount });
            array.emplace_back(timestamp, last, volume, amount);
        }
        store.add_row(array.data(), array.size());
    }
    auto parse(std::string content)
    {
        auto j3 = json::parse(content)
        return j3
    }
};

