#ifndef __DPGS_CONFIG_H__
#define __DPGS_CONFIG_H__

// === [DPGS Server] ===
// FB_SHM_NAME
//   FrameBuffer Shared Memory Name
#define FB_SHM_NAME     "DPGS_FB"
// MM_SHM_NAME
//   MapManager Shared Memory Name
#define MM_SHM_NAME     "DPGS_MM"
// MM_MAP_FILE_PATH
//   MapManager Map File Path
//#define MM_MAP_FILE_PATH    "config/map.json"
#define MM_MAP_FILE_PATH    "config/test_map.json"
// =====================


// === [Camera Source] ===
// TEST_CAM_SRC (only for test)
//   0 : use Camera RTSP
//   1 : use TEST_VIDEO
#define TEST_CAM_SRC    1
#define TEST_VIDEO      "tb/test_a.mp4"
// =======================


// === [AI Engine] ===
// ENABLE_AI_ENGINE
//   0 : disable AI Engine
//   1 : enable AI Engine
#define ENABLE_AI_ENGINE    1
// ===================


// === [VP Engine] ===
// ENABLE_DIST_CORRECTION
//  0 : disable distortion correction
//  1 : enable distortion correction
#define ENABLE_DIST_CORRECTION  0
// ===================


// === [Client Manager] ===
// ENABLE_CLIENT_CONNECTION
//   0 : disable client connection
//   1 : enable client connection
#define ENABLE_CLIENT_CONNECTION    0
// ========================


// === [Device Manager] ===
// ENABLE_REMOTE_LED_DP
//   0 : disable Remoted LED Display
//   1 : enable remoted LED Display
#define ENABLE_REMOTED_LED_DP   0
// ========================



#endif // __DPGS_CONFIG_H__
