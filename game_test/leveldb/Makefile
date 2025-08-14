lleveldb.so : cc/*.cc
	g++ -o $@ $^ -I./cc -I./lua -O2 -fPIC -std=c++17 -shared -lleveldb

.PHONY : clean
clean:
	rm *.so