CXX = clang++ # your compiler goes here, clang++ in my case
CXXFLAGS =-Wall -pedantic # any flags you might want, these are basic ones
LDFLAGS =-L /usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs -lm# any libraries(*.so files) you want to link go here like: -l(libname)

INC =-I /usr/local/include/opencv4 # path to your header files

main: main.cpp
	$(CXX) -std=c++14 $(INC) main.cpp -o main $(CXXFLAGS) $(LDFLAGS) 