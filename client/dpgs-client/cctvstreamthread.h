#ifndef CCTVSTREAMTHREAD_H
#define CCTVSTREAMTHREAD_H

#include <QThread>
#include <QImage>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>

class CCTVStreamThread : public QThread
{
    Q_OBJECT

public:
    explicit CCTVStreamThread(bool &cctv2_mode_ref, QObject *parent = nullptr);
    ~CCTVStreamThread();

    bool init(const QString &uri, const QString &certPath);
    void stop_streaming();

signals:
    void frame_ready(const QImage &image);

protected:
    void run() override;

private:
    GstElement *pipeline;

    bool &cctv2_mode;

    bool setup_pipeline(const QString &uri);
    bool setup_tls(const QString &certPath);
    QImage convert_sample_to_image(GstSample *sample);
};

#endif
