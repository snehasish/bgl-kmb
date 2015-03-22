CXX=g++-4.8
INCLUDE_PATHS=-I$(HOME)/Apps/boost_1_55_0/ 
LIB_PATHS=-Wl,-rpath=$(HOME)/Apps/boost_1_55_0/lib -L$(HOME)/Apps/boost_1_55_0/lib

all: KMB.cpp 
	$(CXX) -std=c++11 $(INCLUDE_PATHS) $(LIB_PATHS) KMB.cpp -o KMB -lboost_graph

test:
	./KMB 0.2 test.dot out.dot

clean:
	rm -f KMB out.dot 

.PHONY: clean test
