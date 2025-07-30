# Makefile for rpi4-dpgs/cam


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
STREAM_DIR		= stream
AI_ENGINE_DIR	= ai_engine
VP_ENGINE_DIR	= vp_engine
MAP_MGR_DIR		= map_mgr
DEVICE_MGR_DIR	= device_mgr
CLIENT_MGR_DIR	= client_mgr


# Output file
TARGET		= dpgs-cam


# Compile options
#CXXFLAGS	= -O3 -I. -I$(INCLUDE_DIR) `pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0 gstreamer-rtsp-server-1.0 gio-2.0 opencv4`
CXXFLAGS	= -I. -I$(INCLUDE_DIR) `pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0 gstreamer-rtsp-server-1.0 gio-2.0 opencv4`

# Linker options
LDFLAGS		= -lpthread -lssl -lcrypto

BUILD_TYPE	?= dev
ifeq ($(BUILD_TYPE), release)
	CXX			= $(CROSS_COMPILE)g++
	CXXFLAGS	+= -I$(CROSS_COMPILE_PATH)/include
	LDFLAGS		+= -L$(CROSS_COMPILE_PATH)/lib
endif


# Source files
CORE_SRCS		= \
					$(CORE_DIR)/main.cpp \
					$(CORE_DIR)/dpgs_cam.cpp \
					$(CORE_DIR)/srv_net.cpp \
					$(CORE_DIR)/cam_thr_supv.cpp
STREAM_SRCS		= \
					$(STREAM_DIR)/cam_stream.cpp \
					$(STREAM_DIR)/str_frame_buffer.cpp \
					$(STREAM_DIR)/srv_stream.cpp
#AI_ENGINE_SRCS	= \
#					$(AI_ENGINE_DIR)/ai_engine.cpp \
#					$(AI_ENGINE_DIR)/detector.cpp \
#					$(AI_ENGINE_DIR)/classifier.cpp
VP_ENGINE_SRCS	= \
					$(VP_ENGINE_DIR)/vp_engine.cpp 
#					$(VP_ENGINE_DIR)/cam_streaming_client.cpp
#MAP_MGR_SRCS	= \
#					$(MAP_MGR_DIR)/map_manager.cpp
#DEVICE_MGR_SRCS	= \
#					$(DEVICE_MGR_DIR)/device_manager.cpp \
#					$(DEVICE_MGR_DIR)/remote_led_dp.cpp
#CLIENT_MGR_SRCS	= \
#					$(CLIENT_MGR_DIR)/client_manager.cpp \
#					$(CLIENT_MGR_DIR)/connection_manager.cpp \
#					$(CLIENT_MGR_DIR)/map_monitor.cpp \
#					$(CLIENT_MGR_DIR)/client_info.cpp \
#					$(CLIENT_MGR_DIR)/srv_sock_utils.cpp \
#					$(CLIENT_MGR_DIR)/streaming_module.cpp

SOURCES			= $(CORE_SRCS) $(STREAM_SRCS) $(AI_ENGINE_SRCS) $(VP_ENGINE_SRCS) $(MAP_MGR_SRCS) $(DEVICE_MGR_SRCS) $(CLIENT_MGR_SRCS)
OBJS			:= $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SOURCES))


MAP_GEN_SRC		= $(TB_DIR)/map_gen.cpp



# targets
all:
	+$(MAKE) $(BUILD_DIR)/$(TARGET)

release:
	$(MAKE) BUILD_TYPE=release all

mapgen: $(BUILD_DIR)/map_gen


# Rules
$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDFLAGS)

$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(BUILD_DIR)/map_gen: $(MAP_GEN_SRC)
	mkdir -p $(dir $@)
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDFLAGS)


clean:
	rm -rf $(BUILD_DIR)


.PHONY: all clean release mapgen
