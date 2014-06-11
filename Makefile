CXX=g++
CXXFLAGS=-Wall -pedantic -g -ggdb -std=c++11


all: cli srv-seq srv-select srv-thr

cli: cli.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

srv-seq: srv-seq.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

srv-select: srv-select.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

srv-thr: srv-thr.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< -lpthread

clean:
	rm -f *~ *.bak core srv-seq srv-thr srv-select cli
