
#ifndef GAMEPADMONITOR_H
#define GAMEPADMONITOR_H

#include <QtCore/QObject>
#include <QtCore/QTimer>

#include <QTcpSocket>
#include <QDataStream>

#define JS_KIRI		0
#define JS_KANAN	1
#define MAJU 		0
#define MUNDUR 		1
#define GESER_KIRI 	2
#define GESER_KANAN 	3
#define PUTAR_KIRI	4
#define PUTAR_KANAN	5
#define HEADER 		"OMNI"

QT_BEGIN_NAMESPACE
class QGamepad;
QT_END_NAMESPACE

class GamepadMonitor : public QObject
{
    Q_OBJECT
public:
    explicit GamepadMonitor(QObject *parent = 0);
    ~GamepadMonitor();

private:
    QGamepad *m_gamepad;
    QTcpSocket *tcp = nullptr;
    double rx,ry,lx,ly;
    bool readyToSend;
    qint64 bytesToSend;
signals:
    void finished();
public slots:
    void finish();
    void getRX(double val);
    void getRY(double val);
    void getLX(double val);
    void getLY(double val);
    void btnPressed(bool status);
    void prepareCmd(int m);
    void sendGreetings();
    void sendCmd(QString cmd);
    void connected();
    void bytesWritten(qint64 bytes);
    void displayError(QAbstractSocket::SocketError socketError);
};

#endif // GAMEPADMONITOR_H
