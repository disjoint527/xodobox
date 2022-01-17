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
            async_connect(this);
            process(events);
        }
    }
//    void connect()
//    {
//    }
// 123.125.108.87
    static void async_connect(TcpSocket * pThis)
    {
        sockaddr_in addr = { AF_INET, htons(pThis->port), {pThis->ipaddr} };
        if( 0 == connect(pThis->fd, (sockaddr*)&addr, sizeof(addr)))
        {	
            std::cout << pThis->ipaddr << ':' << pThis->port << std::endl;
        }
        else
        {
        	std::cout << strerror(errno) << std::endl;
        }
        pThis->selector.add(pThis->fd, pThis);
        
        // http request : yunhq.sse.com.cn/.....
    }
    
    auto send(const char * data, size_t size)
    {
        auto status = ::send(fd, data, size,  MSG_DONTWAIT | MSG_NOSIGNAL);
        if(status <= 0) { async_connect(tmp); return false; }
        else return true;
    }

    TcpSocket(Epoll& selector_, char* _ipaddr, uint32_t _port):
    selector(selector_), ipaddr(inet_addr(_ipaddr)), port(_port)
    {
        fd = socket(AF_INET,SOCK_STREAM,0);
        length = 0;
    }
    Epoll& selector;

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
        request += "Connection: keep-alive\r\n";
        request += "DNT: 1\r\n";
        request += "Upgrade-Insecure-Requests: 1\r\n";
        request += "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:78.0) Gecko/20100101 Firefox/78.0\r\n";
        request += "Accept: text/html,application/xhtml+xml,application/xml\r\n";
                   //"Accept-Encoding: gzip, deflate\r\n"
        request += "Accept-Encoding: identity\r\n";
        request += "Accept-Language: en-US,en;q=0.5\r\n";
        TcpSocket::send(request.c_str(), request.size());
    }
    
//Accept	text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
//Accept-Encoding	gzip, deflate
//Accept-Language	en-US,en;q=0.5
//Connection	keep-alive
//Host	yunhq.sse.com.cn:32041
//Upgrade-Insecure-Requests	1
//User-Agent	Mozilla/5.0 (X11; Linux x86_64; rv:78.0) Gecko/20100101 Firefox/78.0
    

    virtual void process(uint32_t events) override
    {
        TcpSocket::process(events);
        while(TcpSocket::length > 0)
        {
            auto payload = std::string(TcpSocket::buffer, TcpSocket::length);
            if(auto header_end = payload.find("\r\n\r\n"); header_end != std::string::npos) 
            {// header finished
                auto header = std::string(TcpSocket::buffer, header_end + 4); // specify header
                string con_len = "Content-Length: "
                if(auto length_start = header.find(con_len); length_start!= std::string::npos) 
                {// find length of content
                	length_start += con_len.size();
                    if(auto length_end = header.find("\r\n", length_start); length_end != std::string::npos)
                    {// determine the length
                        auto len = std::string(header.c_str() + length_start, length_end - length_start);
                        auto content_length = atoi(len.c_str());
                        if(TcpSocket::length >= (header.size() + content_length))
                        {
                            process_http_data(TcpSocket::buffer + header.size(), content_length);
                            auto discard_len = header.size() + content_length;
                            memmove(TcpSocket::buffer + 0, TcpSocket::buffer + discard_len, TcpSocket::length - discard_len);
                            TcpSocket::length -= discard_len;
                        }
                        else break;
                    }
                    else break;
                }
                else break;
            }
            else break;
        }
    }
};

/*struct Websocket : HttpSocket
{
    virtual void process(uint32_t events) override;
};*/


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
            auto amount = x[3].get<double>();
            //array.push_back({ timestamp, last, volume, amount });
            array.emplace_back({timestamp, last, volume, amount});
        }
        store.add_row(array.data(), array.size());
    }
    auto parse(std::string content)
    {
        auto j3 = json::parse(content)
        return j3
    }
};



























