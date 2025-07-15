# Makefile for rpi4-dpgs


# Compiler
CROSS_COMPILE_PATH=/usr/aarch64-linux-gnu
CROSS_COMPILE=aarch64-linux-gnu-
#CXX		= $(CROSS_COMPILE)g++
CXX			= g++


# Directories
BUILD_DIR	= build
INCLUDE_DIR	= include
CONFIG_DIR	= config
TB_DIR		= tb

CORE_DIR		= core
AI_ENGINE_DIR	= ai_engine
VP_ENGINE_DIR	= vp_engine
MAP_MGR_DIR		= map_mgr
DEVICE_MGR_DIR	= device_mgr


# Output file
TARGET		= dpgs-server


# Compile options
#CXXFLAGS	= -Wall -Werror -O3 -I. -I$(INCLUDE_DIR) `pkg-config --cflags --libs opencv4`
CXXFLAGS	= -O3 -I. -I$(INCLUDE_DIR) `pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0 gstreamer-rtsp-server-1.0 gio-2.0 opencv4`
#CXXFLAGS	= -O3 -I. -I$(INCLUDE_DIR) `pkg-config --cflags --libs opencv4`

# Linker options
LDFLAGS		= -lpthread

BUILD_TYPE	?= dev
ifeq ($(BUILD_TYPE), release)
	CXX			= $(CROSS_COMPILE)g++
	CXXFLAGS	+= -I$(CROSS_COMPILE_PATH)/include
	LDFLAGS		+= -L$(CROSS_COMPILE_PATH)/lib
endif


# Source files
CORE_SRCS		= \
					$(CORE_DIR)/main.cpp \
					$(CORE_DIR)/dpgs_server.cpp \
					$(CORE_DIR)/core_proc_supv.cpp \
					$(CORE_DIR)/core_thr_supv.cpp \
					$(CORE_DIR)/frame_buffer.cpp \
					$(CORE_DIR)/frame_buffer_str.cpp
AI_ENGINE_SRCS	= \
					$(AI_ENGINE_DIR)/ai_engine.cpp \
					$(AI_ENGINE_DIR)/detector.cpp \
					$(AI_ENGINE_DIR)/classifier.cpp
VP_ENGINE_SRCS	= \
					$(VP_ENGINE_DIR)/vp_engine.cpp \
					$(VP_ENGINE_DIR)/cam_streaming_client.cpp
#					$(VP_ENGINE_DIR)/vp_engine_dummy.cpp
MAP_MGR_SRCS	= \
					$(MAP_MGR_DIR)/map_manager.cpp
DEVICE_MGR_SRCS	= \
					$(DEVICE_MGR_DIR)/device_manager.cpp \
					$(DEVICE_MGR_DIR)/remote_led_dp.cpp

SOURCES			= $(CORE_SRCS) $(AI_ENGINE_SRCS) $(VP_ENGINE_SRCS) $(MAP_MGR_SRCS) $(DEVICE_MGR_SRCS)


MAP_GEN_SRC		= $(TB_DIR)/map_gen.cpp



# targets
all: $(BUILD_DIR)/$(TARGET)

release:
	$(MAKE) BUILD_TYPE=release all

mapgen: $(BUILD_DIR)/map_gen


# Rules
$(BUILD_DIR)/$(TARGET): $(SOURCES)
	make clean
	mkdir -p $(dir $@)
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDFLAGS)

$(BUILD_DIR)/map_gen: $(MAP_GEN_SRC)
	mkdir -p $(dir $@)
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDFLAGS)


clean:
	rm -rf $(BUILD_DIR)


.PHONY: all clean release mapgen

#CFLAGS = -I /usr/aarch64-linux-gnu/include -I ./include -D_REENTERANT -c
#LDFLAGS = -L /usr/aarch64-linux-gnu/lib -lpthread
#
#CORE_DIR := ./core
#OBJS := dpgs-server.o core_thread_manager.o client_thread_manager.o core_thread_routine1.o 
#
#dpgs-server: $(OBJS)
#	$(CC) -o $@ $^ $(LDFLAGS)
#
#dpgs-server.o: main.cpp
#	$(CC) $(CFLAGS) $< -o $@
#
## ------------ CORE Compile ------------
#core_thread_manager.o: $(CORE_DIR)/core_thread_manager.cpp
#	$(CC) $(CFLAGS) $< -o $@
#
#client_thread_manager.o: $(CORE_DIR)/client_thread_manager.cpp
#	$(CC) $(CFLAGS) $< -o $@
#
#core_thread_routine1.o : $(CORE_DIR)/core_thread_routine1.cpp
#	$(CC) $(CFLAGS) $< -o $@
#
## ------------ Memory Check ------------- 
#valgrind: dpgs-server
#	valgrind --leak-check=full --leak-resolution=high --show-reachable=yes ./$<
#
#clean: 
#	rm -f dpgs-server *.o
#
