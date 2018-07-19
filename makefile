all:
	g++ -std=c++14 -o socketserver *.cpp  -lstdc++fs -lpthread `pkg-config opencv  --cflags --libs`  \
	~/lib/darknet/darknet.so -DOPENCV=1