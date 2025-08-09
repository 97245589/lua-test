lrank.so : cc/*.cc
	g++ -o $@ $^ -Icc -I../../tools/lua -O2 -fPIC -std=c++17 -shared

.PHONY : clean
clean:
	rm *.so