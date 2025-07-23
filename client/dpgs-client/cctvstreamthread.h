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
    void stopStreaming();

signals:
    void frameReady(const QImage &image);

protected:
    void run() override;

private:
    GstElement *pipeline;

    bool &cctv2_mode;

    bool setupPipeline(const QString &uri);
    bool setupTLS(const QString &certPath);
    QImage convertSampleToImage(GstSample *sample);
};

#endif
