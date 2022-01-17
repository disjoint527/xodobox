#include <memory>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <ctime>
#include <stdio.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <include/epoll.hpp>
#include <include/timerfd.hpp>
#include <include/sockets.hpp>
#include <thread>
namespace py = pybind11; 

struct DataFile
{
    struct Row
    {
        char code[8];
        uint32_t time;
        float last;
        double volume;
        double amount;
    };

    uint32_t num_rows = 0;
    std::array<Row, 32 * 1024 * 1024> book;

    /// Row book[32 * 1024 * 1024]
    /// book.data() == &book[0]
    /// book.size() == 32 * 1024 * 1024;

    void add_row(const Row* array, size_t count)
    {
        for(size_t i = 0; i < count; ++i)
        {
            auto& dst = book[num_rows];
            auto& src = array[i];
            dst = src;
            num_rows++;
        }
    }
};

struct DataFileHolder
{
    DataFileHolder(std::string date)
    {
        static constexpr const char * root_path = "/home/xodobox/data/logs/";
        std::string filename = std::string(root_path) + date;
        std::cout << filename << std::endl;

        if(fd_ = open(filename.c_str(), O_CREAT|O_RDWR|O_NOATIME|O_NONBLOCK, S_IRUSR|S_IWUSR ); fd_ >= 0)
        {
            constexpr size_t size = sizeof(DataFile);
            ftruncate(fd_, size);
            if(auto data = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_, 0); data != MAP_FAILED )
            {
                data_ = (DataFile*)data;
            }
            else
            {
                close(fd_);
                fd_ = -1;
            }
        }
        std::cout << "fd=" << fd_ << ", data=" << data_ << std::endl;
    }
    int fd_ = -1;
    DataFile * data_ = nullptr;
};

PYBIND11_MODULE(datasink, m) {

    PYBIND11_NUMPY_DTYPE(DataFile::Row, code, time, last, volume, amount);

    py::class_<Epoll>(m,"Epoll")
        .def(py::init<>())
        .def("wait", &Epoll::wait)
        ;
    py::class_<Timer>(m,"Timer")
        .def(py::init<Epoll&>())
        .def("set", &Timer::set_timer)
        ;
    py::class_<TcpSocket>(m,"TcpSocket")
        .def(py::init<Epoll&, char*, uint32_t>())
        .def("async_connect", &TcpSocket::async_connect)
        ;
    py::class_<DataFileHolder>(m, "DataStore")
        .def(py::init<std::string>())
        .def_property_readonly("book", [](DataFileHolder const& o)
                                       {
                                            return py::array_t<DataFile::Row>
                                            { 
                                                {o.data_->num_rows},
                                                {sizeof(DataFile::Row)},
                                                o.data_->book.data(),
                                                pybind11::str{}
                                            };
                                       })
        .def("add_row", [](DataFileHolder& o, py::array_t<DataFile::Row> array)
                        {
                            o.data_ -> add_row(array.data(), array.size());
                        })
        ;
}

