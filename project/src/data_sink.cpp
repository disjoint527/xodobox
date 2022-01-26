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
//#include <include/sockets.hpp>
//#include <include/timerfd.hpp>
#include <include/yhq_source.hpp>
// #include <include/Data_File.hpp>
#include <thread>
namespace py = pybind11; 

PYBIND11_MODULE(datasink, m) {

    PYBIND11_NUMPY_DTYPE(DataFiles::Row, code, time, last, volume, amount);

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
                                            return py::array_t<DataFiles::Row>
                                            { 
                                                {o.data_->num_rows},
                                                {sizeof(DataFiles::Row)},
                                                o.data_->book.data(),
                                                pybind11::str{}
                                            };
                                       })
        .def("add_row", [](DataFileHolder& o, py::array_t<DataFiles::Row> array)
                        {
                            o.data_ -> add_row(array.data(), array.size());
                        })
        ;
}

