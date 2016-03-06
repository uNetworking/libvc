default:
	g++ -O2 -s -std=c++11 main.cpp -I . -L . -l:libvulkan.so.1 -o libvc_test
run:
	LD_LIBRARY_PATH=. ./libvc_test
clean:
	rm -f libvc_test
