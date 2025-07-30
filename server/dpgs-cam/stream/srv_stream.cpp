#include "srv_stream.h"

#define CERT_FILE_PATH  "config/server.crt"
#define KEY_FILE_PATH   "config/server.key"


// ===== Static =====
GstAppSrc* global_appsrc = NULL;

void SrvStream::media_configure(GstRTSPMediaFactory* factory, GstRTSPMedia* media, gpointer user_data) {

    GstElement* element = gst_rtsp_media_get_element(media);
    GstElement* appsrc = gst_bin_get_by_name_recurse_up(GST_BIN(element), "mysrc");

    if (appsrc) {
        global_appsrc = GST_APP_SRC(appsrc);
        g_object_set(G_OBJECT(appsrc), "do-timestamp", TRUE, NULL);

        GstCaps *caps = gst_caps_new_simple("video/x-raw",
            "format", G_TYPE_STRING, "BGR",
            "width", G_TYPE_INT, 640,
            "height", G_TYPE_INT, 360,
            "framerate", GST_TYPE_FRACTION, 30, 1, NULL);
        gst_app_src_set_caps(global_appsrc, caps);
        gst_caps_unref(caps);

        std::cout << "Connect Client Success\n";
    } else {
        std::cerr << "Connect Client Failed\n";
    }

    gst_object_unref(element);
}
// =====================


SrvStream::SrvStream(StrFrameBuffer& _fb)
    : fb(_fb) {
}

SrvStream::~SrvStream() {
}


bool SrvStream::initialize() {
    std::cout << "[SRV_STREAM] Start to initialize...\n";

    const std::string service_port = "8555";
    const std::string path = "/stream";

    gst_init(nullptr, nullptr);

    GstRTSPServer *server = gst_rtsp_server_new();
    gst_rtsp_server_set_service(server, service_port.c_str());
    gst_rtsp_server_set_address(server, "0.0.0.0");

/*
    GTlsCertificate *cert = g_tls_certificate_new_from_files(CERT_FILE_PATH, KEY_FILE_PATH, NULL);
    if (!cert) {
        std::cerr << "No certificate and key!\n";
        exit (1);
    }

    GstRTSPAuth *auth = gst_rtsp_auth_new();
    gst_rtsp_server_set_auth(server, auth);
    gst_rtsp_auth_set_tls_certificate(auth, cert);

    GstRTSPToken *token = gst_rtsp_token_new(GST_RTSP_TOKEN_MEDIA_FACTORY_ROLE, G_TYPE_STRING, "anonymous", NULL);
    gst_rtsp_auth_set_default_token(auth, token);
    gst_rtsp_token_unref(token);
    g_object_unref(cert);
    g_object_unref(auth);
*/

    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);
    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();

    gst_rtsp_media_factory_set_launch(factory,
        "( appsrc name=mysrc is-live=true format=3 do-timestamp=true "
        "! videoconvert ! video/x-raw,format=I420,width=640,height=360,framerate=30/1 "
        "! x264enc tune=zerolatency byte-stream=true bitrate=500 speed-preset=ultrafast key-int-max=60 "
        "! rtph264pay config-interval=1 pt=96 name=pay0 )");
    
//    gst_rtsp_media_factory_set_shared(factory, TRUE);
//    gst_rtsp_media_factory_set_enable_rtcp(factory, TRUE);
    g_signal_connect(factory, "media-configure", G_CALLBACK(media_configure), NULL);

/*
    GstRTSPPermissions *p = gst_rtsp_permissions_new();
    gst_rtsp_permissions_add_role (p, "anonymous", GST_RTSP_PERM_MEDIA_FACTORY_ACCESS, G_TYPE_BOOLEAN, TRUE,
                                    GST_RTSP_PERM_MEDIA_FACTORY_CONSTRUCT, G_TYPE_BOOLEAN, TRUE, NULL);

    gst_rtsp_media_factory_set_permissions(factory, p);
    gst_rtsp_permissions_unref(p);
*/  
  
    //RTSP 서버에서 특정 RTSP 경로에 대해 스트림을 제공할 수 있도록 RTSP 미디어 팩토리 등록
    gst_rtsp_mount_points_add_factory(mounts, path.c_str(), factory);
    g_object_unref(mounts);
    //RTSP 서버를 GStreamer의 메인 루프에 등록하여 실제로 동작하게 만듦
    gst_rtsp_server_attach(server, NULL);

//    std::thread([]{GMainLoop* loop = g_main_loop_new(NULL,FALSE);
//                g_main_loop_run(loop);
//                g_main_loop_unref(loop);}).detach();



    std::cout << "[SRV_STREAM] Success: Server Streaming Module initialized\n";
    return true;
}


void SrvStream::push_to_server(const cv::Mat& frame) {
    if (!global_appsrc) {
        std::cout << "[SRV_STREAM] No global appsrc\n";
        return;
    }

    GstBuffer *buffer;
    GstFlowReturn ret;
    int size = frame.total() * frame.elemSize();
    buffer = gst_buffer_new_allocate(nullptr, size, nullptr);
    
    //버퍼를 MAP하여 포인터로 접근 가능하게 만듦
    GstMapInfo map;
    gst_buffer_map(buffer, &map, GST_MAP_WRITE);
    memcpy(map.data, frame.data, size);
    gst_buffer_unmap(buffer, &map);

    //GStreamer 파이프라인에 현재 버퍼 삽입(RTSP 송출 시작), 결과는 ret으로 전달
    g_signal_emit_by_name(global_appsrc, "push-buffer", buffer, &ret);
    gst_buffer_unref(buffer);
}


void SrvStream::run() {
    std::cout << "[SRV_STREAM] Run Server Streaming Module\n";

    cv::Mat frame;

    is_running = true;
    while (is_running) {

        frame = fb.pop();
        if (frame.empty()) {
            continue;
        }
        cv::Mat frame_copy = frame.clone();

        // [Debug Session]
        cv::imshow("[SRV_STREAM] frame", frame_copy);
        cv::waitKey(1);

//        push_to_server(frame_copy);

    }

}


void SrvStream::stop() {
    std::cout << "[SRV_STREAM] Server Streaming Module Terminating...\n";

    is_running = false;

    std::cout << "[SRV_STREAM] Server Streaming Module Terminated\n";
}


void SrvStream::clear() {
    std::cout << "[SRV_STREAM] clear: Cleanning...\n";


    std::cout << "[SRV_STREAM] clear: Cleanning Success\n";
}
