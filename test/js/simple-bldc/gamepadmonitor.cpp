/* CATATAN:
 * 1. Untuk menghentikan server, kirim sinyal STOP dengan menekan tombol Start
 * 2. Joystick ditekan ke atas nilainya negatif
 * */

#include "gamepadmonitor.h"
#include <QtGamepad/QGamepad>

#include <QDebug>
#include <QLoggingCategory>
#include <math.h>
#include <QCoreApplication>

GamepadMonitor::GamepadMonitor(QObject *parent)
    : QObject(parent)
    , m_gamepad(0)
    , readyToSend(true)
{
    // inisialisasi tcp/ip
    tcp = new QTcpSocket(this);
    connect(tcp, SIGNAL(connected()), this, SLOT(connected()));
    connect(tcp, SIGNAL(disconnected()), this, SLOT(finish()));
    connect(tcp, &QAbstractSocket::disconnected, tcp, &QObject::deleteLater);
    connect(tcp, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));
    connect(tcp, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &GamepadMonitor::displayError);
    tcp->connectToHost("127.0.0.1", 9999);

    auto gamepads = QGamepadManager::instance()->connectedGamepads();
    if (gamepads.isEmpty()) {
        qDebug() << "Did not find any connected gamepads";
        return;
    }

    m_gamepad = new QGamepad(*gamepads.begin(), this);
    connect(m_gamepad, SIGNAL(buttonStartChanged(bool)), this, SLOT(btnPressed(bool)));
    connect(m_gamepad, SIGNAL(axisLeftXChanged(double)), this, SLOT(getLX(double)));	//strive
    connect(m_gamepad, SIGNAL(axisLeftYChanged(double)), this, SLOT(getLY(double)));	//fwd-back
    connect(m_gamepad, SIGNAL(axisRightXChanged(double)), this, SLOT(getRX(double)));	//rotate
    connect(m_gamepad, SIGNAL(axisRightYChanged(double)), this, SLOT(getRY(double)));	//nothing
}

void GamepadMonitor::displayError(QAbstractSocket::SocketError socketError)
{
    //qDebug() << "[ERROR] Ada yang salah dengan koneksinya";
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
	qDebug() << "The host was not found. Please check the host name and port settings.";
        break;
    case QAbstractSocket::ConnectionRefusedError:
	qDebug() << "The connection was refused by the peer. Make sure the server is running, and check that the host name and port settings are correct.";
        break;
    default:
	qDebug() << QString("The following error occurred: %1.").arg(tcp->errorString());
    }
    emit finished();
}

void GamepadMonitor::finish()
{
    emit finished();
    // QCoreApplication::quit();
}

GamepadMonitor::~GamepadMonitor()
{
    delete m_gamepad;
    //delete tcp; --> tidak perlu, karena sudah deleteLater, kalo pakai malah Segmentation Fault!
}

void GamepadMonitor::getRX(double val)
{
    //qDebug() << QString("val = %1").arg(val);
    rx = val;
    prepareCmd(JS_KANAN);
}

void GamepadMonitor::getRY(double val)
{
    //qDebug() << QString("val = %1").arg(val);
    ry = val;
    //prepareCmd(JS_KANAN);
}

void GamepadMonitor::getLX(double val)
{
    //qDebug() << QString("val = %1").arg(val);
    lx = val;
    prepareCmd(JS_KIRI);
}

void GamepadMonitor::getLY(double val)
{
    //qDebug() << QString("val = %1").arg(val);
    ly = val;
    prepareCmd(JS_KIRI);
}

void GamepadMonitor::btnPressed(bool status)
{
    if(status){
        QString cmd = QString("STOP");
        sendCmd(cmd);
    } else {
    }
}

void GamepadMonitor::prepareCmd(int m)
{
    /* stick KIRI untuk: majur, mundur, geser kanan, geser kiri
     * stick KANAN hanya untuk putar
     * *********************************************************/
    QString cmd;
    if(m==JS_KIRI){			// joystick kiri dioperasikan
	    if(fabs(ly)>fabs(lx)) { 	// gerakan maju mundur
		    if(ly<0) {		// maka berarti maju
			cmd = QString("%1,%2,%3")
				.arg(HEADER).arg(MAJU).arg(fabs(ly));
		    } else {		// maka berarti mundur
			cmd = QString("%1,%2,%3")
				.arg(HEADER).arg(MUNDUR).arg(ly);
		    }
	    } else { 			// gerakan geser kanan/kiri
		    if(lx<0) {		// geser kiri
			    cmd = QString("%1,%2,%3")
				    .arg(HEADER).arg(GESER_KIRI).arg(fabs(lx));
		    } else {		// geser kanan
			    cmd = QString("%1,%2,%3")
				    .arg(HEADER).arg(GESER_KANAN).arg(lx);
		    }
	    }
    } else {				// joystick kanan dioperaasikan
	    if(rx<0){			// putar kiri
		    cmd = QString("%1,%2,%3")
			    .arg(HEADER).arg(PUTAR_KIRI).arg(fabs(rx));
	    } else {
		    cmd = QString("%1,%2,%3")
			    .arg(HEADER).arg(PUTAR_KANAN).arg(rx);
	    }
    }
    sendCmd(cmd);
}

void GamepadMonitor::sendCmd(QString cmd)
{
    qDebug() << "kirim CMD = " << cmd;
    while(!readyToSend){
	    qApp->processEvents();
    }
    //return;
    // yang berikut ini, somehow, malah ada karaketer aneh 
    // yang terkirim, sehingga perintah STOP bahkan tidak bisa
    // dideteksi...
    /*
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);
    out << cmd;
    tcp->write(block);
    */
    //tcp->write(cmd.append('\n').toLatin1());
    tcp->write(cmd.toLatin1());
    bytesToSend = cmd.length();
}

void GamepadMonitor::sendGreetings()
{
    qDebug() << QString("[INFO] Kirim greetings ke server...");
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);
    out << QString("Hello server...");
    tcp->write(block);
}

void GamepadMonitor::connected()
{
    qDebug() << QString("[INFO] Connected to the server!");
    sendGreetings();
}

/************************************************************
 * Section di bawah ini buat debugging aja
 * *********************************************************/
void GamepadMonitor::bytesWritten(qint64 bytes)
{
	//qDebug() << QString("Mengirim sebanyak %1 bytes").arg(bytes);
	if(bytes==bytesToSend) readyToSend = true;
}
