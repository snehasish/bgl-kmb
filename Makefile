CXX=g++-4.8
BOOST_INSTALL_ROOT=$(HOME)/Apps/boost_1_55_0/
INCLUDE_PATHS=-I$(BOOST_INSTALL_ROOT)
LIB_PATHS=-Wl,-rpath=$(BOOST_INSTALL_ROOT)/lib -L$(BOOST_INSTALL_ROOT)/lib

all: KMB.cpp 
	$(CXX) -O3 -std=c++11 $(INCLUDE_PATHS) $(LIB_PATHS) KMB.cpp -o KMB -lboost_graph

debug: KMB.cpp 
	$(CXX) -DDEBUG -ggdb3 -std=c++11 $(INCLUDE_PATHS) $(LIB_PATHS) KMB.cpp -o KMB-dbg -lboost_graph

test: debug
	./KMB-dbg 0.1 steiner.dot debug.dot

clean:
	@rm -f KMB KMB-dbg debug.dot 

.PHONY: clean test
