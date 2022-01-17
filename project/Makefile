VPATH = src: include

all: datasink

datasink: src/data_sink.cpp
	c++ -O3 -Wall --shared -std=c++17 -fPIC $(python3 -m pybind11 --includes) src/data_sink.cpp `python -m pybind11 --include` -o datasink.so
