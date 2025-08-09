lworld.so : cc/*.cc
	g++ -o $@ $^ -O2 -fPIC -std=c++17 -shared -I../lua -I./cc

.PHONY : clean
clean:
	rm *.so