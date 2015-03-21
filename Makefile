CXX=g++-4.8
INCLUDE_PATHS=-I$(HOME)/Apps/boost_1_55_0/ 

all: KMB.cpp
	$(CXX) -std=c++11 $(INCLUDE_PATHS) KMB.cpp -o KMB

clean:
	rm -f KMB 
