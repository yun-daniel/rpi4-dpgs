//loginworker.cpp
#include "loginworker.h"

LoginWorker::LoginWorker(const QString &id, const QString &pw)
    : m_id(id), m_pw(pw) {}

void LoginWorker::process()
{
    QByteArray idData = m_id.toUtf8();
    QByteArray pwData = m_pw.toUtf8();
    idData.resize(16, '\0');
    pwData.resize(16, '\0');

    QTcpSocket socket;
    qDebug() << "서버 연결 시도: 192.168.0.67:9999";
    socket.connectToHost("192.168.0.67", 9999);
    if (!socket.waitForConnected(3000)) {
        qDebug() << "연결 실패:" << socket.errorString();
        emit loginResult(false, "서버에 연결할 수 없습니다.");
        return;
    }

    socket.write(idData);
    socket.write(pwData);
    socket.flush();

    if (!socket.waitForReadyRead(2000)) {
        emit loginResult(false, "서버 응답이 없습니다.");
        return;
    }

    QByteArray response = socket.read(1);
    qDebug() << "응답값 (char):" << response[0];
    if (!response.isEmpty() && response[0] == '1')
        emit loginResult(true, "로그인 성공");
    else
        emit loginResult(false, "ID 또는 비밀번호가 틀렸습니다.");
}
