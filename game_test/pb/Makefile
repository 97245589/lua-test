pb.so : c/*.c
	gcc -o $@ $^ -I../lua -O2 -fPIC -shared

.PHONY : clean
clean :
	rm *.so