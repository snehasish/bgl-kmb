CXX=g++-4.8
INCLUDE_PATHS=-I$(HOME)/Apps/boost_1_55_0/ 
LIB_PATHS=-Wl,-rpath=$(HOME)/Apps/boost_1_55_0/lib -L$(HOME)/Apps/boost_1_55_0/lib

all: KMB.cpp 
	$(CXX) -O3 -std=c++11 $(INCLUDE_PATHS) $(LIB_PATHS) KMB.cpp -o KMB -lboost_graph

debug: KMB.cpp 
	$(CXX) -DDEBUG -ggdb3 -std=c++11 $(INCLUDE_PATHS) $(LIB_PATHS) KMB.cpp -o KMB-dbg -lboost_graph

test: debug
	./KMB-dbg 0.1 steiner.dot debug.dot

clean:
	@rm -f KMB KMB-dbg debug.dot 

.PHONY: clean test
