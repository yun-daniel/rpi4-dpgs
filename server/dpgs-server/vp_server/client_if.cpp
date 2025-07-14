#include "client_if.hpp"
#include "vp_engine.hpp"

GstAppSrc *global_appsrc = NULL;


void ClientIF::media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer user_data) {
    
    // media 객체로부터 내부 GStreamer 파이프라인을 가져옴
    GstElement *element = gst_rtsp_media_get_element(media);
    // mysrc라는 이름의 요소(appsrc)를 재귀적으로 탐색
    GstElement *appsrc = gst_bin_get_by_name_recurse_up(GST_BIN(element), "mysrc");

    if (appsrc) {
        // 찾은 appsrc를 global_appsrc에 저장 + do-timestamp 옵션으로 GStreamer가 각 버퍼에 자동으로 타임스탬프 부여
        global_appsrc = GST_APP_SRC(appsrc);
        g_object_set(G_OBJECT(appsrc), "do-timestamp", TRUE, NULL);

        GstCaps *caps = gst_caps_new_simple("video/x-raw",
            "format", G_TYPE_STRING, "BGR",
            "width", G_TYPE_INT, 640,
            "height", G_TYPE_INT, 360,
            "framerate", GST_TYPE_FRACTION, 30, 1, NULL);
        gst_app_src_set_caps(global_appsrc, caps);
        gst_caps_unref(caps);

        cout << "Connect Client Success" << endl;
    } else {
        cerr << "Connect Client Failed" << endl;
    }

    gst_object_unref(element);
}

void ClientIF::init_rtsp_server(const string& service_port, const string& path){
    GstRTSPServer *server = gst_rtsp_server_new();
    gst_rtsp_server_set_service(server, service_port.c_str());
    gst_rtsp_server_set_address(server, "0.0.0.0");

    GTlsCertificate *cert = g_tls_certificate_new_from_files("server.crt", "server.key", NULL);
    if (!cert) {
        g_printerr("No certificate and key!\n");
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

    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);
    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();

    gst_rtsp_media_factory_set_launch(factory,
        "( appsrc name=mysrc is-live=true format=3 do-timestamp=true "
        "! videoconvert ! video/x-raw,format=I420,width=640,height=360,framerate=30/1 "
        "! x264enc tune=zerolatency byte-stream=true bitrate=500 speed-preset=ultrafast key-int-max=60 "
        "! rtph264pay config-interval=1 pt=96 name=pay0 )");
    
    gst_rtsp_media_factory_set_shared(factory, TRUE);
    gst_rtsp_media_factory_set_enable_rtcp(factory, TRUE);
    g_signal_connect(factory, "media-configure", G_CALLBACK(media_configure), NULL);


    GstRTSPPermissions *p = gst_rtsp_permissions_new();
    gst_rtsp_permissions_add_role (p, "anonymous", GST_RTSP_PERM_MEDIA_FACTORY_ACCESS, G_TYPE_BOOLEAN, TRUE,
                                    GST_RTSP_PERM_MEDIA_FACTORY_CONSTRUCT, G_TYPE_BOOLEAN, TRUE, NULL);

    gst_rtsp_media_factory_set_permissions(factory, p);
    gst_rtsp_permissions_unref(p);
    
    //RTSP 서버에서 특정 RTSP 경로에 대해 스트림을 제공할 수 있도록 RTSP 미디어 팩토리 등록
    gst_rtsp_mount_points_add_factory(mounts, path.c_str(), factory);
    g_object_unref(mounts);
    //RTSP 서버를 GStreamer의 메인 루프에 등록하여 실제로 동작하게 만듦
    gst_rtsp_server_attach(server, NULL);

    // cout << "RTSP Running: rtsp://192.168.0.32:8555/stream" << endl;
}

void ClientIF::push_frame_to_rtsp(const Mat& frame){
    if(!global_appsrc) return;

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

    //if (ret != GST_FLOW_OK) {
        //cerr << "GStreamer push-buffer error." << endl;
    //}

}

Mat ClientIF::create_dummy_frame(int queue_index){
    int width = 640, height = 360;

    Mat dummy = Mat::zeros(height, width, CV_8UC3);

    time_t now = time(nullptr);
    tm* local_time = localtime(&now);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y.%m.%d %H:%M", local_time);

    string message = to_string(queue_index) + " OFFLINE - " + time_str;

    int fontFace = FONT_HERSHEY_SIMPLEX;
    double fontScale = 1.0;
    int thickness = 2;
    Scalar textColor(255,255,255);

    Size textSize = getTextSize(message, fontFace, fontScale, thickness, nullptr);
    Point textOrg((width - textSize.width) / 2, (height + textSize.height) / 2);

    putText(dummy, message, textOrg, fontFace, fontScale, textColor, thickness);

    //imshow("dummy", dummy);
    //waitKey(0);

    return dummy;
}

void ClientIF::recv_image(int queue_index){
    queue<Mat>* selected_queue = nullptr;
    mutex* selected_mutex = nullptr;
    condition_variable* selected_cv = nullptr;

    if(queue_index == 1){
        cout << "[ClientIF] selected queue_index = 1" << endl;
        selected_queue = &VPEngine::frame_queue_1;
        selected_mutex = &VPEngine::queue_mutex_1;
        selected_cv = &VPEngine::queue_cv_1;
    }
    else if (queue_index == 2) {
        cout << "[ClientIF] selected queue_index = 2" << endl;
        selected_queue = &VPEngine::frame_queue_2;
        selected_mutex = &VPEngine::queue_mutex_2;
        selected_cv = &VPEngine::queue_cv_2;
    } else {
        std::cerr << "[ClientIF] Invalid queue_index: " << queue_index << std::endl;
        return;
    }

    while(true){
        cout << "[ClientIF] pop queue[" << queue_index << "] " << endl;
        unique_lock<mutex> lock(*selected_mutex);
        selected_cv->wait(lock, [&](){ return !selected_queue->empty(); });
        
        Mat processed = selected_queue->front();
        // cout << "[DEBUG] Mat is empty? : " << processed.empty() << endl;
        selected_queue->pop();
        lock.unlock();
        
        auto now = chrono::steady_clock::now();
        cout << "[ClientIF] Frame consumed at " << chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count() << "ms\n";

        if(processed.empty()){
            continue;
        }
        else{
            push_frame_to_rtsp(processed);
        }
        
        //push_frame_to_rtsp(processed);
    }
}


void* ClientIF::run(void * args){
    cout << "[DEBUG] clientIF TEST" << endl;
    gst_init(nullptr, nullptr);
    init_rtsp_server("8555", "/stream");

    recv_image(*(int*)args);

    return nullptr;
}