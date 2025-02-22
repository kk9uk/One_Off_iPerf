.PHONY: clean

One_Off_iPerf: One_Off_iPerf.cpp
	g++ -Wconversion -Wall -Werror -Wextra -pedantic -O3 One_Off_iPerf.cpp -o One_Off_iPerf

clean:
	rm -rf One_Off_iPerf
