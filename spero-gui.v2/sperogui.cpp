#include "sperogui.h"
#include "ui_sperogui.h"
#include <qdebug.h>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDateTime>
#include <QLoggingCategory>
#include <math.h>
#include <QHostAddress>
#include <QNetworkDatagram>

speroGUI::speroGUI(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::speroGUI),
    pubConnection(false),
    subConnection(false)
{
    ui->setupUi(this);
    // Beberapa elemen harus diset readonly
    ui->cbCharging->setAttribute(Qt::WA_TransparentForMouseEvents); ui->cbCharging->setFocusPolicy(Qt::NoFocus);
    ui->tablet->setAttribute(Qt::WA_TransparentForMouseEvents); ui->tablet->setFocusPolicy(Qt::NoFocus);
    ui->cbConnected->setAttribute(Qt::WA_TransparentForMouseEvents); ui->cbConnected->setFocusPolicy(Qt::NoFocus);
    ui->cbDisinfectant->setAttribute(Qt::WA_TransparentForMouseEvents); ui->cbDisinfectant->setFocusPolicy(Qt::NoFocus);
    ui->cbJoystick->setAttribute(Qt::WA_TransparentForMouseEvents); ui->cbJoystick->setFocusPolicy(Qt::NoFocus);
    ui->cbCharging->setAttribute(Qt::WA_TransparentForMouseEvents); ui->cbCharging->setFocusPolicy(Qt::NoFocus);
    ui->cbLocked->setAttribute(Qt::WA_TransparentForMouseEvents); ui->cbLocked->setFocusPolicy(Qt::NoFocus);
    ui->cbLampu->setAttribute(Qt::WA_TransparentForMouseEvents); ui->cbLampu->setFocusPolicy(Qt::NoFocus);
    ui->s1->setReadOnly(true); ui->s2->setReadOnly(true); ui->s3->setReadOnly(true); ui->s4->setReadOnly(true);
    ui->s5->setReadOnly(true); ui->s6->setReadOnly(true); ui->s7->setReadOnly(true); ui->s8->setReadOnly(true);

    // matikan dulu yang bagianH How-to nya
    ui->tab2->hide();

    // TODO: omnicam dan frontcam belum terintegrasi, maka GUI-nya kita kecilkan saja
    // ukuran idealnya: 1180 x 840
    // kalau kontrolnya saja: 470 x 840
    this->setGeometry(0,0,470,840);

    /* Pertama, coba buka file konfigurasi. Jika file ditemukan, baca dari file tersebut. Jika tidak, bikin file baru.*/
    getConfig();

    // persiapkan soket udp
    qDebug() << "[MAIN] Coba bind UDP ";
    udpSock = new QUdpSocket(this);
    if(!udpSock->bind(QHostAddress::Any, TABLET_UDP_PORT)){
        QMessageBox::critical(this, "ERROR", "Tidak bisa binding port UDP!");
    }
    connect(udpSock, &QUdpSocket::readyRead, this, &speroGUI::readPendingUDP);
    // dan timer yang terkait:
    m_TabletInfo = new QTimer(this);
    m_TabletInfo->setInterval(TABLET_BAT_PERIOD);
    connect(m_TabletInfo, &QTimer::timeout, this, &speroGUI::askTabletBattery);
    m_TabletInfo->start();

    // berikut berhubungan dengan mqtt
    m_publisher = new QMqttClient(this);
    qDebug() << QString("Will use server at %1:1883 for publishing").arg(robotIP);
    m_publisher->setHostname(robotIP);
    m_publisher->setPort(1883);
    connect(m_publisher, &QMqttClient::stateChanged, this, &speroGUI::updateLogStateChange);
    connect(m_publisher, &QMqttClient::disconnected, this, &speroGUI::brokerDisconnected);
    connect(m_publisher, &QMqttClient::connected, this, &speroGUI::pubConnected);
    connect(m_publisher, &QMqttClient::disconnected, this, &speroGUI::pubDisconnected);
    m_subscriber = new QMqttClient(this);
    // TODO: Test pakai iot.petra.ac.id
    /*
    QString testServer = QString("203.189.123.207");    //pinjam iot.petra.ac.id
    qDebug() << QString("Will use server at %1:1883 for subscription").arg(testServer);
    m_subscriber->setHostname(testServer);
    */

    qDebug() << QString("Will use server at %1:1883 for subscription").arg(robotIP);
    m_subscriber->setHostname(robotIP);

    m_subscriber->setPort(1883);
    connect(m_subscriber, &QMqttClient::stateChanged, this, &speroGUI::updateLogStateChange);
    connect(m_subscriber, &QMqttClient::disconnected, this, &speroGUI::brokerDisconnected);
    connect(m_subscriber, &QMqttClient::connected, this, &speroGUI::subConnected);
    connect(m_subscriber, &QMqttClient::disconnected, this, &speroGUI::subDisconnected);
    connect(m_subscriber, &QMqttClient::messageReceived, this, &speroGUI::newMsg);
    qDebug() << QString("Will now connect to the server...");
    m_publisher->connectToHost();
    m_subscriber->connectToHost();

    // lalu setup pinger: m_ping akan periodik mengirim sinyal ke robot
    m_ping = new QTimer(this);
    m_ping->setInterval(ROBOT_PING_INTERVAL_MS);    //mungkin cukup tiap 1000ms kirim ping ke robot
    connect(m_ping, SIGNAL(timeout()), this, SLOT(sendPing()));
    m_ping->start();
    // m_timeout memberi waktu sejak ping reply diterima dan digunakan untuk menentukan apakah robot terkoneksi atau tidak
    m_timeout = new QTimer(this);
    m_timeout->setInterval(ROBOT_REPLAY_TIMEOUT_MS);
    connect(m_timeout, SIGNAL(timeout()), this, SLOT(pingTimeout()));

    // berikut berhubungan dengan joystick
    QLoggingCategory::setFilterRules(QStringLiteral("qt.gamepad.debug=true"));

    auto gamepads = QGamepadManager::instance()->connectedGamepads();
    if (gamepads.isEmpty()) {
        qDebug() << "Did not find any connected gamepads";
        b_gamepad = false;
    } else {
        m_gamepad = new QGamepad(*gamepads.begin(), this);
        connect(m_gamepad, SIGNAL(buttonStartChanged(bool)), this, SLOT(btnStartPressed(bool)));
        connect(m_gamepad, SIGNAL(axisLeftXChanged(double)), this, SLOT(getLX(double)));    //strive
        connect(m_gamepad, SIGNAL(axisLeftYChanged(double)), this, SLOT(getLY(double)));    //fwd-back
        connect(m_gamepad, SIGNAL(axisRightXChanged(double)), this, SLOT(getRX(double)));   //rotate
        connect(m_gamepad, SIGNAL(axisRightYChanged(double)), this, SLOT(getRY(double)));   //nothing
        connect(m_gamepad, SIGNAL(buttonSelectChanged(bool)), this, SLOT(btnSelectPressed(bool)));
        connect(m_gamepad, SIGNAL(buttonGuideChanged(bool)), this, SLOT(btnGuidePressed()));
        b_gamepad = true;
    }



    // Konfigurasi SIGNAL-SLOT GUI umum:
    connect(ui->pbStop, SIGNAL(clicked()), this, SLOT(pbStopClicked()));
    connect(ui->hSlider, SIGNAL(valueChanged(int)), this, SLOT(hSliderChanged(int)));
    connect(ui->vSlider, SIGNAL(valueChanged(int)), this, SLOT(vSliderChanged(int)));
    connect(ui->dial, SIGNAL(valueChanged(int)), this, SLOT(dialChanged(int)));
    connect(ui->pbSimpan, SIGNAL(clicked()), this, SLOT(pbSimpanClicked()));
    connect(ui->pbHalt, SIGNAL(clicked()), this, SLOT(pbHaltClicked()));
    connect(ui->pbChat, SIGNAL(clicked()), this, SLOT(pbChatClicked()));

    connect(ui->pbLepas, &QPushButton::clicked, this, &speroGUI::pbLepasClicked);
    // asumsikan robot tidak terkunci saat program ini pertama dijalankan
    ui->pbLepas->setEnabled(false);

    connect(ui->pbLampu, SIGNAL(clicked()),this, SLOT(pbLampuClicked()));
    connect(ui->servo, SIGNAL(valueChanged(int)), this, SLOT(servoChanged(int)));

    connect(ui->tray1, &QCheckBox::stateChanged, this, &speroGUI::tray1Clicked);
    connect(ui->tray2, &QCheckBox::stateChanged, this, &speroGUI::tray2Clicked);
    connect(ui->tray3, &QCheckBox::stateChanged, this, &speroGUI::tray3Clicked);
    connect(ui->tray4, &QCheckBox::stateChanged, this, &speroGUI::tray4Clicked);
    connect(ui->cbCam, &QCheckBox::stateChanged, this, &speroGUI::camClicked);
    connect(ui->cbReleaseLock, &QCheckBox::stateChanged, this, &speroGUI::cbReleaseLockChanged);

    // siapkan dulu Dialog-nya, jaga-jaga kalau battery-nya perlu dicharge...
    b_batWarning = false;
    batWarning = new BatteryWarning(this);
    batWarning->hide();
    connect(ui->pbTest1, &QPushButton::clicked, this, &speroGUI::pbTest1Clicked);
    connect(ui->pbTest2, &QPushButton::clicked, this, &speroGUI::pbTest2Clicked);
}

void speroGUI::cbReleaseLockChanged(int state)
{
    QString cmd;
    if(state != Qt::Unchecked)
        cmd = QString("%1,%2").arg(RELEASE_CMD).arg("1");
    else
        cmd = QString("%1,%2").arg(TRAY_HEADER).arg("0");
    sendMQTT(cmd);
}

speroGUI::~speroGUI()
{
    if(b_gamepad) delete m_gamepad;
    delete m_publisher;
    delete m_subscriber;
    delete m_ping;
    delete batWarning;
    delete udpSock;
    delete ui;
}

void speroGUI::pbChatClicked()
{
    udpSock->writeDatagram(QByteArray("jitsi"), QHostAddress(tabletIP), TABLET_UDP_PORT);
    qDebug() << QString("Kirim udp 'bat' ke-%1 port-%2").arg(tabletIP).arg(TABLET_UDP_PORT) << "\n";
}

void speroGUI::askTabletBattery()
{
    udpSock->writeDatagram(QByteArray("bat"), QHostAddress(tabletIP), TABLET_UDP_PORT);
    qDebug() << QString("Kirim udp 'bat' ke-%1 port-%2").arg(tabletIP).arg(TABLET_UDP_PORT) << "\n";
}

void speroGUI::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Up){
        ui->vSlider->setValue(ui->vSlider->value()+1);
    } else if(event->key() == Qt::Key_Down) {
        ui->vSlider->setValue(ui->vSlider->value()-1);
    } else if(event->key() == Qt::Key_Right) {
        ui->dial->setValue(ui->dial->value()+1);
    } else if(event->key() == Qt::Key_Left) {
        ui->dial->setValue(ui->dial->value()-1);
    }
}

void speroGUI::readPendingUDP()
{
    //baca baterry pake qtimer
    //qDebug() << "Terima paket UDP...";
    while (udpSock->hasPendingDatagrams()) {
            QNetworkDatagram datagram = udpSock->receiveDatagram();
            //jika dikirim beneran dari tablet, baca datanya
            //qDebug() << QString("Dapat dari %1 = %2").arg(datagram.senderAddress().toString()).arg(datagram.data());
            //qDebug() << QString("Data paket dari %1").arg(datagram.senderAddress().toString());
            //qDebug() << QString("Datanya = %1").arg(datagram.data().toInt());
            if(datagram.senderAddress().toString().contains(tabletIP)){
                //qDebug() << "Memang dikirim dari Tablet";
                int batLevel = QString(datagram.data()).toInt();
                ui->tabletBatLevel->setText(QString("%1%").arg(batLevel));
                //qDebug() << QString("Isinya: %1").arg(batLevel);
                // lalu broadcast pakai MQTT
                QString cmd = QString("%1,%2").arg(TABLET_HEADER).arg(batLevel);
                sendMQTT(cmd);
            }
        }
}

void speroGUI::showBatWarning()
{
    b_batWarning = false;
    batWarning->setModal(true);
    batWarning->show();
}

void speroGUI::pubConnected()
{
    pubConnection = true;
}

void speroGUI::pubDisconnected()
{
    pubConnection = false;
}

void speroGUI::subDisconnected()
{
    subConnection = false;
    //ui->cbConnected->setChecked(false);
}

void speroGUI::subConnected()
{
    subConnection = true;
    // untuk kondisi terkoneksi mula-mula, bisa kita aktifkan centang berikut
    ui->cbConnected->setChecked(true);  // tapi dia bisa berubah jika tidak ada reply dari robot dlm waktu yang ditentukan
    auto subscription = m_subscriber->subscribe(QString(TOPIC_STATUS));
    if (!subscription) {
            QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not subscribe. Is there a valid connection?"));
            return;
    }
}

/* Kirim sinyal secara periodik ke robot */
void speroGUI::sendPing()
{
    //qDebug() << "[INFO] Kirim ping cmd...";
    QString cmd = QString(PING_CMD);
    sendMQTT(cmd);
    // lalu tunggu sampai batas waktu yang ditentukan, robot harus reply
    if(!m_timeout->isActive()) {
        //qDebug() << "[INFO] Aktifkan m_timeout";
        m_timeout->start();
    }
}

void speroGUI::pingTimeout()
{
    // jike m_timeout sampai expired, berarti robot tidak me-reply
    //qDebug() << "[WARN] Timeout! Robot tidak me-reply!";
    ui->cbConnected->setChecked(false);
}

void speroGUI::newMsg(const QByteArray &message, const QMqttTopicName &topic)
{
    if(topic.name().contains(TOPIC_STATUS)){
        //qDebug() << "[INFO] Menerima pesan MQTT dari robot" << "\n";
        QString msg = QString(message);
        QStringList lst = msg.split(",");
        // jika jumlah hasil splitting sama dengan 9 (misal: SENSOR,1,2,3,4,5,6,7,8) maka valid
        if(lst.length()==9 && lst[0].contains(SENSOR_HEADER)){
            ui->s1->setText(lst[1]);
            ui->s2->setText(lst[2]);
            ui->s3->setText(lst[3]);
            ui->s4->setText(lst[4]);
            ui->s5->setText(lst[5]);
            ui->s6->setText(lst[6]);
            ui->s7->setText(lst[7]);
            ui->s8->setText(lst[8]);
        } else if(lst.length()==2 && lst[0].contains(COLLISION_HEADER)){
            if(lst[1].contains("0")){
                ui->cbLocked->setChecked(false);
                ui->pbLepas->setEnabled(false);
            } else if(lst[1].contains("1")){
                ui->cbLocked->setChecked(true);
                ui->pbLepas->setEnabled(true);
            }
        } else if(lst.length()==2 && lst[0].contains(PING_HEADER)){
            if(lst[1].contains(PING_REPLY)){
                //qDebug() << "[INFO] Pong dari robot..";
                ui->cbConnected->setChecked(true);
                this->m_timeout->start();   // jika robot me-reply, reset watchdog-nya
            }
        } else if(lst.length()==3 && lst[0].contains(BATTERY_HEADER)){
            double lvl = lst[1].toDouble();
            int level = (int)lvl; // masih mentah integer

            /* y = mx + b
             * 100 = m.ACCU_MAX + b
             * 0   = m.ACCU_MIN + b --> 100 = m(ACCU_MAX-ACCU_MIN)
             */
            /*
            double m = 100/(ACCU_MAX - ACCU_MIN);
            double b = 100 - m*ACCU_MAX;
            double pct = m*level + b;
            int lvl = (int)round(pct);
            lvl = lvl > 100 ? 100:lvl;
            lvl = lvl < 0 ? 0:lvl;
            qDebug() << QString("bat val = %1, pct=%2").arg(level).arg(pct);
            */
            ui->batLevel->setText(QString("%1%2").arg(level).arg("%"));
            if(level<=ACCU_THRESHOLD && !b_batWarning){
                showBatWarning();
            }
        }
    }
}

void speroGUI::pbHaltClicked()
{
    int ret = QMessageBox::warning(this, "PERINGATAN", "Anda yakin akan menghentikan operasional robot SPERO?",
                         QMessageBox::Yes | QMessageBox::Cancel);
    if(ret==QMessageBox::Yes){
        qDebug () << "Ada request untuk shutdown SPERO";
        QString cmd = QString(STOP_CMD);
        sendMQTT(cmd);
        // TODO: then wait until SPERO is completely shutted down (via ping?)
        this->close();
    }
}

void speroGUI::updateLogStateChange()
{
    const QString content1 = QDateTime::currentDateTime().toString()
                    + QLatin1String(": State Change")
                    + QString::number(m_publisher->state())
                    + QLatin1Char('\n');
    const QString content2 = QDateTime::currentDateTime().toString()
                    + QLatin1String(": State Change")
                    + QString::number(m_subscriber->state())
                    + QLatin1Char('\n');
    qDebug() << (content1);
    qDebug() << (content2);
    ui->console->appendPlainText(content1);
    ui->console->appendPlainText(content2);
}

void speroGUI::brokerDisconnected()
{
    QString msg = QString("[INFO] Terputus dari server...") + QLatin1Char('\n');;
    qDebug() << msg;
    ui->console->appendPlainText(msg);
}


void speroGUI::pbSimpanClicked()
{
    //qDebug() << "pbSimpan ditekan";
    robotIP = ui->robotIP->text();
    omnicamIP = ui->omnicamIP->text();
    frontcamIP = ui->frontcamIP->text();
    tabletIP = ui->tabletIP->text();
    if(saveConfig()==0){
        QMessageBox::information(this, "Informasi", "Penyimpanan konfigurasi berhasil dilakukan");
        QMessageBox::information(this, "Informasi", "Program akan ditutup supaya konfigurasi baru bisa dipakai!");
        this->close();
    } else {
        QMessageBox::critical(this, "Error", "Tidak berhasil menyimpan konfigurasi yang baru");
    }
}

int speroGUI::saveConfig()
{
    QFile file(CONFIG_FILE_NAME);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QString msg = QString("[ERROR] Tidak bisa menulis file konfigurasi");
        ui->console->insertPlainText(msg);
        qDebug() << msg;
        return -1;
    } else {
        QTextStream out(&file);
        out << QString("%1=").arg(PARAM_NAME_ROBOT_IP) << robotIP << "\n";
        out << QString("%1=").arg(PARAM_NAME_OMNICAM_IP) << omnicamIP << "\n";
        out << QString("%1=").arg(PARAM_NAME_FRONTCAM_IP) << frontcamIP << "\n";
        out << QString("%1=").arg(PARAM_NAME_TABLET_IP) << tabletIP << "\n";
        file.close();
        //lalu tampilkan di GUI-nya
        ui->robotIP->setText(robotIP);
        ui->omnicamIP->setText(omnicamIP);
        ui->frontcamIP->setText(frontcamIP);
        ui->tabletIP->setText(tabletIP);
        return 0;
    }
}

void speroGUI::getConfig()
{
    QFile file(CONFIG_FILE_NAME);
       if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
       {
           qDebug() << "[INFO] Mencoba membuat file konfigurasi baru";
           robotIP = DEFAULT_ROBOT_IP;
           omnicamIP = DEFAULT_OMNICAM_IP;
           frontcamIP = DEFAULT_FRONTCAM_IP;
           tabletIP = DEFAULT_TABLET_IP;
           saveConfig();
       } else {
           qDebug() << "[INFO] Membaca file konfigurasi";
           QTextStream in(&file);
           while (!in.atEnd()) {
               QString line = in.readLine();
               qDebug() << line;
               QStringList list = line.split(QLatin1Char('='));
               if(list[0].contains(PARAM_NAME_ROBOT_IP)){
                   ui->robotIP->setText(list[1]);
                   robotIP = list[1];
               } else if(list[0].contains(PARAM_NAME_OMNICAM_IP)) {
                   ui->omnicamIP->setText(list[1]);
                   omnicamIP = list[1];
               } else if(list[0].contains(PARAM_NAME_FRONTCAM_IP)) {
                   ui->frontcamIP->setText(list[1]);
                   frontcamIP = list[1];
               } else if(list[0].contains(PARAM_NAME_TABLET_IP)) {
                   ui->tabletIP->setText(list[1]);
                   tabletIP = list[1];
               }
            }
           file.close();
       }

}


/*-------------------------- Common GUI ------------------------------*/

void speroGUI::servoChanged(int newVal)
{
    QString cmd = QString("%1,%2").arg(SERVO_HEADER).arg(newVal);
    sendMQTT(cmd);
}

void speroGUI::pbLampuClicked()
{
    QString cmd;
    if(ui->pbLampu->isChecked()){
        ui->cbLampu->setChecked(true);
        cmd = QString("%1,%2").arg(LAMP_HEADER).arg(1);
    }
    else {
        ui->cbLampu->setChecked(false);
        cmd = QString("%1,%2").arg(LAMP_HEADER).arg(0);
    }
    sendMQTT(cmd);
}

void speroGUI::pbLepasClicked()
{
    // kirim perintah "RELEASE" jika robot sempat di lock karena mau nabrak
    int ret = QMessageBox::warning(this, "PERINGATAN", "Anda yakin akan melepas LOCK dari robot SPERO?",
                         QMessageBox::Yes | QMessageBox::Cancel);
    if(ret==QMessageBox::Yes){
        qDebug () << "Ada request untuk relase SPERO";
        QString cmd = QString(RELEASE_CMD);
        sendMQTT(cmd);
    }
}

void speroGUI::tray1Clicked(int state)
{
    QString cmd;
    if(state != Qt::Unchecked)
        cmd = QString("%1,%2,%3").arg(TRAY_HEADER).arg("1").arg("1");
    else
        cmd = QString("%1,%2,%3").arg(TRAY_HEADER).arg("1").arg("0");
    sendMQTT(cmd);
}

void speroGUI::tray2Clicked(int state)
{
    QString cmd;
    if(state != Qt::Unchecked)
        cmd = QString("%1,%2,%3").arg(TRAY_HEADER).arg("2").arg("1");
    else
        cmd = QString("%1,%2,%3").arg(TRAY_HEADER).arg("2").arg("0");
    sendMQTT(cmd);
}

void speroGUI::tray3Clicked(int state)
{
    QString cmd;
    if(state != Qt::Unchecked)
        cmd = QString("%1,%2,%3").arg(TRAY_HEADER).arg("3").arg("1");
    else
        cmd = QString("%1,%2,%3").arg(TRAY_HEADER).arg("3").arg("0");
    sendMQTT(cmd);
}

void speroGUI::tray4Clicked(int state)
{
    QString cmd;
    if(state != Qt::Unchecked)
        cmd = QString("%1,%2,%3").arg(TRAY_HEADER).arg("4").arg("1");
    else
        cmd = QString("%1,%2,%3").arg(TRAY_HEADER).arg("4").arg("0");
    sendMQTT(cmd);
}

void speroGUI::camClicked(int state)
{
    QString cmd;
    if(state != Qt::Unchecked)
        cmd = QString("%1,%2").arg(CAM_HEADER).arg("1");
    else
        cmd = QString("%1,%2").arg(CAM_HEADER).arg("0");
    sendMQTT(cmd);
}

void speroGUI::pbStopClicked()
{
    qDebug() << "Tombol Stop ditekan!";
    ui->hSlider->setValue(0);
    ui->vSlider->setValue(0);
    ui->dial->setValue(0);
}

void speroGUI::hSliderChanged(int newVal)
{
    qDebug() << QString("nilai hSlider = %1").arg(newVal);
    double x = (double)newVal / 100.0;
    lx = x;
    prepareCmd(JS_KIRI);
}

void speroGUI::vSliderChanged(int newVal)
{
    qDebug() << QString("nilai vSlider = %1").arg(newVal);
    double y = (double)newVal / 100.0;
    ly = y;
    prepareCmd(JS_KIRI);
}

void speroGUI::dialChanged(int newVal)
{
    qDebug() << QString("nilai dial = %1").arg(newVal);
    double r = (double)newVal / 100.0;
    rx = r;
    prepareCmd(JS_KANAN);
}

/*------------------- Joystick ------------------*/
void speroGUI::getRX(double val)
{
    qDebug() << QString("RX val = %1").arg(val);
    rx = val;
    prepareCmd(JS_KANAN);
}

void speroGUI::getRY(double val)
{
    ry = val;   // tidak perlu prepareCmd, karena joystick kanan hanya mendeteksi AXIS-X
}
void speroGUI::getLX(double val)
{
    qDebug() << QString("LX val = %1").arg(val);
    lx = val;
    prepareCmd(JS_KIRI);
}
void speroGUI::getLY(double val)
{
    qDebug() << QString("LY val = %1").arg(val);
    ly = val;
    prepareCmd(JS_KIRI);
}
void speroGUI::btnStartPressed(bool status)
{
    // tombol start kita gunakan untuk test base
    if(status){
        int ret = QMessageBox::question(this, "WARNING", "Akan dilakukan Test BASE. Pastikan di sekitar area sudah clear. Lanjut?",
                                        QMessageBox::Yes | QMessageBox::Cancel);
        if(ret == QMessageBox::Yes){
            QString cmd = QString(TEST_BASE);
            sendMQTT(cmd);
        }
    } else {
    }
}
void speroGUI::btnSelectPressed(bool status)
{
    // tombol select kita gunakan untuk test roda
    if(status){
        int ret = QMessageBox::question(this, "WARNING", "Akan dilakukan Test Roda. Pastikan robot dalam posisi floating. Lanjut?",
                                        QMessageBox::Yes | QMessageBox::Cancel);
        if(ret == QMessageBox::Yes){
            QString cmd = QString(TEST_RODA_CMD);
            sendMQTT(cmd);
        }
    }
}
void speroGUI::btnGuidePressed(bool status)
{
    qDebug() << "Guide button is pressed!";
    if(status){
        QString cmd = QString(BRAKE_CMD);
        sendMQTT(cmd);
    }
}

void speroGUI::prepareCmd(int m)
{
    /* stick KIRI untuk: majur, mundur, geser kanan, geser kiri
     * stick KANAN hanya untuk putar
     * *********************************************************/
    QString cmd;
    if(m==JS_KIRI){                     // joystick kiri dioperasikan
            if(fabs(ly)>fabs(lx)) {     // gerakan maju mundur
                    if(ly<0) {          // maka berarti maju
                        cmd = QString("%1,%2,%3")
                                .arg(HEADER).arg(MAJU).arg(fabs(ly));
                    } else {            // maka berarti mundur
                        cmd = QString("%1,%2,%3")
                                .arg(HEADER).arg(MUNDUR).arg(ly);
                    }
            } else {                    // gerakan geser kanan/kiri
                    if(lx<0) {          // geser kiri
                            cmd = QString("%1,%2,%3")
                                    .arg(HEADER).arg(GESER_KIRI).arg(fabs(lx));
                    } else {            // geser kanan
                            cmd = QString("%1,%2,%3")
                                    .arg(HEADER).arg(GESER_KANAN).arg(lx);
                    }
            }
    } else {                            // joystick kanan dioperaasikan
            if(rx<0){                   // putar kiri
                    cmd = QString("%1,%2,%3")
                            .arg(HEADER).arg(PUTAR_KIRI).arg(fabs(rx));
            } else {
                    cmd = QString("%1,%2,%3")
                            .arg(HEADER).arg(PUTAR_KANAN).arg(rx);
            }
    }
    sendMQTT(cmd);
}

void speroGUI::sendMQTT(QString cmd)
{
    // modifikasi dari on_buttonPublish_clicked()
    if (m_publisher->publish(QString(TOPIC_JS), cmd.toUtf8()) == -1)
        qDebug() << QString("[INFO] Error, could not send the MQTT message!");
}

void speroGUI::sendGreetings()
{
    qDebug() << QString("[INFO] Kirim greetings ke server...");
    sendMQTT(QString("Halo controller..."));
}

/*------------------------ Tombol-tombol Tester ----------------------------*/

void speroGUI::pbTest1Clicked()
{
    batWarning->tunjukkan();
}

void speroGUI::pbTest2Clicked()
{
    QString cmd = QString(RESET_SENSOR);
    sendMQTT(cmd);
    QMessageBox::information(this, "Information", "Perintah reset sensor sudah dikirim!");
}

void speroGUI::pbTest3Clicked()
{

}

void speroGUI::pbTest4Clicked()
{

}
