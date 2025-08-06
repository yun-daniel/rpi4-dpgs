#include "cctvstreamthread.h"
#include "tls_helper.h"

#include <QDebug>
#include <QFile>
#include <QStandardPaths>

CCTVStreamThread::CCTVStreamThread(bool &cctv2_mode_ref, QObject *parent)
    : QThread(parent), pipeline(nullptr), cctv2_mode(cctv2_mode_ref)
{
    qDebug() << "[CCTV] 스레드 생성";
    gst_init(nullptr, nullptr);
}

CCTVStreamThread::~CCTVStreamThread()
{
    stop_streaming();
}

bool CCTVStreamThread::init(const QString &uri, const QString &certPath)
{
    if (!setup_pipeline(uri))
        return false;
    if (!setup_tls(certPath))
        return false;
    return true;
}

bool CCTVStreamThread::setup_pipeline(const QString &uri)
{
    QString desc = QString(
                       "rtspsrc name=src location=%1 tls-validation-flags=1 latency=30 ! "
                       "decodebin ! videoconvert ! video/x-raw,format=RGB,width=640,height=360 ! "
                       "appsink name=appsink").arg(uri);

    GError *err = nullptr;
    pipeline = gst_parse_launch(desc.toUtf8().constData(), &err);
    if (!pipeline)
    {
        qDebug() << "[GStreamer] 파이프라인 생성 실패";
        if (err) g_error_free(err);
        return false;
    }

    GstElement *appsink = gst_bin_get_by_name(GST_BIN(pipeline), "appsink");
    if (appsink)
    {
        g_object_set(appsink,
                     "emit-signals", FALSE,
                     "sync", FALSE,
                     "max-buffers", 1,
                     "drop", TRUE,
                     nullptr);
    }
    qDebug() << "[CCTV] 파이프라인 생성 완료";
    return true;
}

bool CCTVStreamThread::setup_tls(const QString &certPath)
{
    gst_element_set_state(pipeline, GST_STATE_READY);
    GstElement *src = gst_bin_get_by_name(GST_BIN(pipeline), "src");
    if (!src)
    {
        qDebug() << "[CCTV] src 요소 찾기 실패";
        return false;
    }

    GTlsDatabase *db = load_tls_database(certPath.toUtf8().constData());
    if (!db)
    {
        qDebug() << "[CCTV] TLS 데이터베이스 로드 실패";
        gst_object_unref(src);
        return false;
    }

    g_object_set(G_OBJECT(src), "tls-database", db, nullptr);
    g_object_unref(db);
    gst_object_unref(src);
    qDebug() << "[CCTV] TLS 설정 완료";
    return true;
}

void CCTVStreamThread::run()
{
    qDebug() << "[CCTV] 스레드 실행";
    if (!pipeline)
    {
        qDebug() << "[CCTV] 파이프라인 없음";
        return;
    }

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    GstElement *appsink = gst_bin_get_by_name(GST_BIN(pipeline), "appsink");
    if (!appsink)
    {
        qDebug() << ("[GStreamer] appsink 요소 찾을 수 없음");
        return;
    }

    while (!isInterruptionRequested())
    {
        if (cctv2_mode)
        {
            QThread::msleep(100);
            continue;
        }

        GstSample *sample = gst_app_sink_try_pull_sample(GST_APP_SINK(appsink), 100 * GST_MSECOND);
        if (!sample)
        {
            qDebug() << "[CCTV] 샘플 없음";
            continue;
        }

        QImage img = convert_sample_to_image(sample);
        gst_sample_unref(sample);

        if (img.isNull()) {
            qDebug() << "[CCTV] 변환된 이미지 NULL";
        } else {
            qDebug() << "[CCTV] 프레임 수신 성공";
            emit frame_ready(img);
        }
    }

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(appsink);
    gst_object_unref(pipeline);
    pipeline = nullptr;

    qDebug() << "[CCTV] 스레드 종료";
}

QImage CCTVStreamThread::convert_sample_to_image(GstSample *sample)
{
    GstBuffer *buffer = gst_sample_get_buffer(sample);
    GstCaps *caps = gst_sample_get_caps(sample);
    GstStructure *s = gst_caps_get_structure(caps, 0);
    const gchar *format = gst_structure_get_string(s, "format");
    gint width, height;
    gst_structure_get_int(s, "width", &width);
    gst_structure_get_int(s, "height", &height);

    GstMapInfo map;
    gst_buffer_map(buffer, &map, GST_MAP_READ);

    QImage img;
    if (QString(format) == "RGB")
    {
        img = QImage((uchar *)map.data, width, height, QImage::Format_RGB888).copy();
    }

    gst_buffer_unmap(buffer, &map);
    return img;
}

void CCTVStreamThread::stop_streaming()
{
    requestInterruption();
    wait();
}
