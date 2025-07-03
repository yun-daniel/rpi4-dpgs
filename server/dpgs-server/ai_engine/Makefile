CC=g++

CFLAGS = -O3 `pkg-config --cflags --libs opencv4`

all:
	g++ -O3 tb/main.cpp ai_engine.cpp -o tb/example `pkg-config --cflags --libs opencv4` -I.
