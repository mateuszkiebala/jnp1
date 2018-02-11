all: main
main: very_long_int main.cc
	c++ -std=c++11 -g very_long_int main.cc -o main -Wall -Werror -O2

very_long_int: very_long_int.cc very_long_int.h
	c++ -g -c -std=c++11 very_long_int.cc -o very_long_int -Wall -Werror -O2
