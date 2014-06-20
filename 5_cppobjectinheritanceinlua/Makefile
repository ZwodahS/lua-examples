WARNING= -Wextra -Wno-switch -Wno-sign-compare -Wno-missing-braces -Wno-unused-parameter
CXX=clang++ -std=c++11 $(WARNING) 


run : main.o
	$(CXX) main.o -o run -llua -ldl  

main.o : main.cpp
	$(CXX) -c main.cpp -o main.o

clean :
	rm main.o
	rm run
