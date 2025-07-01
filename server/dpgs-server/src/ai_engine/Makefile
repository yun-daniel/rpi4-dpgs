CC=g++

CFLAGS = -O3 `pkg-config --cflags --libs opencv4`

all:
	g++ -O3 tb/main.cpp ai_engine.cpp detector.cpp classifier.cpp map_manager.cpp -o tb/example `pkg-config --cflags --libs opencv4` -I.

mapgen:
	g++ -O3 map_gen.cpp -o tb/map_gen `pkg-config --cflags --libs opencv4` -I.
