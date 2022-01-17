
struct YHQInstance
{
    struct RequestTimer : Timer
    {
        YHQInstance& host_;
        virtual void on_timer()
        virtual void process(uint32_t events) override
        {
            Timer::process(events);
            for(auto& socket : host_.sockets_)
            {
                socket->GET("yunhq.sse.com.cn:32041", "/v1/sh1/list/exchange/equity?select=date,time,code,last,volume,amount");
            }
        }
    };
    std::vector<yunhq_socket*> sockets_;
    RequestTimer timer_;

    YHQInstance(Epoll& selector, std::vector<uint32_t> address)
    {
        for(auto& addr : address)
        {
            sockets.push_back( new yunhq_socket(addr) );
        }
        timer_.set_timer(500 * 1000 * 1000, 0);
    }
};

