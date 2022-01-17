#pragma
#include <sys/timerfd.h>
#include <include/epoll.hpp>

struct Timer : Epoll::Interface
{
    virtual void process(uint32_t events) override
    {
        if((events & EPOLLIN))
        {
            uint64_t value; read(fd, &value, 8);
            std::cout << count++ << std::endl;
        }
    }

    Timer(Epoll & selector)
    {
        fd = timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK);
        selector.add(fd, this);
    }

    static constexpr int64_t nanos_per_sec = 1000 * 1000 * 1000;
    void set_timer(int64_t interval, int64_t initial = nanos_per_sec)
    {
        struct itimerspec newValue { { int(interval / nanos_per_sec), int(interval % nanos_per_sec)},
                                     { int(initial  / nanos_per_sec), int(initial  % nanos_per_sec)}
                                   };
        timerfd_settime(fd,0,&newValue,nullptr);
    }

    int fd = 0;
    int count = 0; 
};

