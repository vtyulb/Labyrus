#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QObject::connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    QObject::connect(ui->start, SIGNAL(clicked()), this, SLOT(start()));
    QObject::connect(ui->allowConnections, SIGNAL(toggled(bool)), ui->widget, SLOT(setVisible(bool)));
    QObject::connect(ui->actionAbout_Qt, SIGNAL(triggered()), this, SLOT(aboutQt()));
    QObject::connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));

    loadSettings();
}

MainWindow::~MainWindow() {
    server->terminate();
    saveSettings();
    delete ui;
}

void MainWindow::start() {
    QStringList attributes;
    attributes << "-n";
    attributes << ui->fieldSize->text();
    attributes << "--height" << QString::number(ui->heightOfField->value());
    attributes << "--latency" << ui->spinBox->text();
    if (ui->cheats->checkState() == Qt::Checked)
        attributes << "--cheats";

    if (ui->allowConnections->checkState() == Qt::Checked) {
        attributes << "-p";
        attributes << ui->numberPlayers->text();
        if (ui->strong->checkState() == Qt::Checked)
            attributes << "--strong";
    } else {
        attributes << "-p";
        attributes << "1";
        attributes << "--strong";
    }
    this->hide();
    server = new QProcess;
    client = new QProcess;
    QEventLoop *loop = new QEventLoop;
    QTimer *timeout = new QTimer;
    serverShutDown = new QTimer;
    serverShutDown->setInterval(1000);
    console = new Console;
    console->setMinimumSize(QSize(500, 300));
    console->setMaximumSize(QSize(500, 300));
    console->show();

    QObject::connect(timeout, SIGNAL(timeout()), loop, SLOT(quit()));
    timeout->setInterval(7000);
    timeout->start();


    QObject::connect(client, SIGNAL(finished(int)), server, SLOT(terminate()));
    QObject::connect(server, SIGNAL(readyReadStandardOutput()), loop, SLOT(quit()));
    QObject::connect(server, SIGNAL(finished(int)), loop, SLOT(quit()));
    QObject::connect(server, SIGNAL(finished(int)), console, SLOT(deleteLater()));
    QObject::connect(server, SIGNAL(readyReadStandardError()), this, SLOT(serverSaid()));
    QObject::connect(serverShutDown, SIGNAL(timeout()), this, SLOT(checkForShutDown()));
    server->start(prefix + "labyrus-server", attributes);
    loop->exec();

    server->setReadChannel(QProcess::StandardOutput);
    QString res = server->readLine();
    if (res != "map generated\n") {
        QMessageBox::about(this, QString("Die"), res);
        return;
    }

    attributes.clear();
    attributes << "-n" << ui->name->text();
    attributes << "-p" << "7777";
    attributes << "-i" << "127.0.0.1" ;
    attributes << "--start";
    client->start(prefix + "labyrus-client", attributes);
    serverShutDown->start();
}

void MainWindow::aboutQt() {
    QMessageBox::aboutQt(this);
}

void MainWindow::about() {
    QMessageBox::about(this, "About Labyrus", tr("Labyrus is crossplatform game written with opengl and QT Written by Vladislav Tyulbashev"));
}

void MainWindow::saveSettings() {
    QSettings s(settingsFile, QSettings::IniFormat);
    s.setValue("singlePlayerGeometry", QVariant(saveGeometry()));
    s.setValue("name", QVariant(ui->name->text()));
    s.setValue("size", QVariant(ui->fieldSize->value()));
    s.setValue("height", QVariant(ui->heightOfField->value()));
    s.setValue("cheats", QVariant(ui->cheats->checkState() == Qt::Checked));
    s.setValue("allowRemoteConnections", QVariant(ui->allowConnections->checkState() == Qt::Checked));
    s.setValue("strongNumberPlayers", QVariant(ui->strong->checkState() == Qt::Checked));
    s.setValue("numberPlayers", QVariant(ui->numberPlayers->value()));
    s.setValue("latency", QVariant(ui->spinBox->value()));
}

void MainWindow::loadSettings() {
    QSettings s(settingsFile, QSettings::IniFormat);
    restoreGeometry(s.value("singlePlayerGeometry").toByteArray());
    ui->name->setText(s.value("name", QVariant("vlad")).toString());
    ui->fieldSize->setValue(s.value("size", QVariant(10)).toInt());
    ui->heightOfField->setValue(s.value("height", QVariant(1)).toInt());
    ui->cheats->setChecked(s.value("cheats", QVariant(false)).toBool());
    ui->allowConnections->setChecked(s.value("allowRemoteConnections", QVariant(false)).toBool());
    ui->widget->setVisible(ui->allowConnections->checkState() == Qt::Checked);
    ui->strong->setChecked(s.value("strongNumberPlayers", QVariant(false)).toBool());
    ui->numberPlayers->setValue(s.value("numberPlayers", QVariant(1)).toInt());
    ui->spinBox->setValue(s.value("latency", QVariant(25)).toInt());
}

void MainWindow::serverSaid() {
    server->setReadChannel(QProcess::StandardError);
    while (server->canReadLine())
        console->addString(server->readLine());
}

void MainWindow::checkForShutDown() {
    if (!console->isVisible()) {
        qDebug() << "shutting down server";
        server->terminate();
        client->terminate();
        this->close();
    }
}
