#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    serial = new QSerialPort(this);

    // Mevcut portları listele
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->comboPort->addItem(info.portName());
    }

    // Sık kullanılan baud rate'leri ekle
    ui->comboBaud->addItem("9600");
    ui->comboBaud->addItem("115200");
    ui->comboBaud->setCurrentText("115200"); // STM32 için varsayılan
}

MainWindow::~MainWindow()
{
    if(serial->isOpen()) {
        serial->close();
    }
    delete ui;
}

void MainWindow::on_btnConnect_clicked()
{
    QString portName = ui->comboPort->currentText();
    QString baudRate = ui->comboBaud->currentText();

    if(portName.isEmpty()) {
        QMessageBox::warning(this, "Hata", "Bir port seçiniz!");
        return;
    }

    // Eğer port zaten açıksa önce kapat
    if(serial->isOpen()) {
        serial->close();
    }

    // Port ayarlarını yap
    serial->setPortName(portName);
    serial->setBaudRate(baudRate.toInt());
    serial->setDataBits(QSerialPort::Data8);      // 8 bit veri
    serial->setParity(QSerialPort::NoParity);      // Parity yok
    serial->setStopBits(QSerialPort::OneStop);     // 1 stop bit
    serial->setFlowControl(QSerialPort::NoFlowControl); // Flow control yok

    // Portu açmayı dene
    if(serial->open(QIODevice::ReadWrite)) {
        qDebug() << "Port başarıyla açıldı:" << portName;
        QMessageBox::information(this, "Bağlantı", "STM32 ile bağlantı kuruldu!");

        ui->btnConnect->setEnabled(false);
        ui->btnDisconnect->setEnabled(true);

        // STM32'den veri gelince readSerialData fonksiyonunu çağır
        connect(serial, &QSerialPort::readyRead, this, &MainWindow::readSerialData);

        // Hata sinyalini de dinle
        connect(serial, &QSerialPort::errorOccurred, this, &MainWindow::handleSerialError);
    } else {
        qDebug() << "Port açılamadı:" << serial->errorString();
        QMessageBox::critical(this, "Hata",
                              "Bağlantı kurulamadı!\n" + serial->errorString());
    }
}

void MainWindow::readSerialData()
{
    QByteArray data = serial->readAll();
    QString tempStr = QString::fromUtf8(data).trimmed();

    qDebug() << "Gelen veri:" << tempStr;

    // Eğer sayı formatındaysa LCD'ye yaz
    bool ok;
    float temperature = tempStr.toFloat(&ok);

    if(ok) {
        ui->lcdTemperature->display(temperature);
        qDebug() << "Sıcaklık:" << temperature;
    } else {
        qDebug() << "Geçersiz veri formatı:" << tempStr;
    }
}

void MainWindow::on_btnDisconnect_clicked()
{
    if(serial->isOpen()) {
        // Sinyalleri kes
        disconnect(serial, &QSerialPort::readyRead, this, &MainWindow::readSerialData);
        disconnect(serial, &QSerialPort::errorOccurred, this, &MainWindow::handleSerialError);

        serial->close();
        QMessageBox::information(this, "Bağlantı", "Bağlantı kesildi!");

        ui->btnConnect->setEnabled(true);
        ui->btnDisconnect->setEnabled(false);
    }
}



void MainWindow::handleSerialError(QSerialPort::SerialPortError error)
{
    if(error != QSerialPort::NoError && error != QSerialPort::TimeoutError) {
        qDebug() << "Seri port hatası:" << serial->errorString();
        QMessageBox::critical(this, "Seri Port Hatası", serial->errorString());

        if(serial->isOpen()) {
            serial->close();
        }

        ui->btnConnect->setEnabled(true);
        ui->btnDisconnect->setEnabled(false);
    }
}
