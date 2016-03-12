default:
	g++ -O2 -s -std=c++11 case1_vulkan.cpp -I ../include -L ../lib -l:libvulkan.so.1 -o case1_vulkan
run:
	LD_LIBRARY_PATH=../lib ./case1_vulkan
clean:
	rm -f case1_vulkan
