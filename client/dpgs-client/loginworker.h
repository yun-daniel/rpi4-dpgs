//loginworker.h
#ifndef LOGINWORKER_H
#define LOGINWORKER_H

#include <QObject>
#include <QTcpSocket>

class LoginWorker : public QObject
{
    Q_OBJECT

public:
    LoginWorker(const QString &id, const QString &pw);

public slots:
    void process();

signals:
    void loginResult(bool success, const QString &message);

private:
    QString m_id;
    QString m_pw;
};

#endif // LOGINWORKER_H
