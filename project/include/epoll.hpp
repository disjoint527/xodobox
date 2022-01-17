#pragma once
#include <sys/types.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>

struct Epoll
{
    static constexpr int max_events = 128;
    struct Interface
    {
        virtual void process(uint32_t events) = 0;
    };

    void wait(int timeout=-1)
    {
        std::array<epoll_event, max_events> events;
        if(int count = epoll_wait(fd, events.data(), events.size(), timeout); count > 0)
        {
            for(int i = 0; i < count; ++i)
            {
                auto& entry = events[i];
                auto ptr = (Interface*)entry.data.ptr;
                ptr->process(entry.events);
            }
        }
        else
        {
        	std::cout << "wait" << std::endl;
        }
    }
    int add( int _fd, Interface * inf )
    {
        epoll_event e = { EPOLLIN | EPOLLRDHUP, inf };
        return epoll_ctl( fd, EPOLL_CTL_ADD, _fd, &e );
    }

    Epoll() : fd(epoll_create1(0)) { }
    const int fd;
};
