#include "connectionswindow.h"
#include "ui_connectionswindow.h"

ConnectionsWindow::ConnectionsWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OPC)
{
    ui->setupUi(this);
    setWindowTitle("Connections Settings");

    connect(this, &ConnectionsWindow::cam1ConnectionChange, this, &ConnectionsWindow::onCam1ConnectionChange);
    connect(this, &ConnectionsWindow::cam2ConnectionChange, this, &ConnectionsWindow::onCam2ConnectionChange);
    connect(this, &ConnectionsWindow::opcConnectionChange, this, &ConnectionsWindow::onOpcConnectionChange);
    connect(this, &ConnectionsWindow::appConfigChange, this, &ConnectionsWindow::onAppConfigChange);

}

ConnectionsWindow::~ConnectionsWindow()
{
    delete ui;
}

void ConnectionsWindow::assignHalconCore(QSharedPointer<HalconCore> newHalconCore)
{
    halconCore = newHalconCore;
    connect(halconCore.data(), &HalconCore::camera1ConnectionStatus, [this](bool status) {
        ui->cam1ConnectionLbl->setText(status ? "<font color=blue> Connected </font>" : "<font color=red> Disconnected </font>");
    });

    connect(halconCore.data(), &HalconCore::camera2ConnectionStatus, [this](bool status) {
        ui->cam2ConnectionLbl->setText(status ? "<font color=blue> Connected </font>" : "<font color=red> Disconnected </font>");
    });
}

void ConnectionsWindow::assignOpcBackend(QSharedPointer<OpcBackend> newOpcBackend)
{
    opcBackend = newOpcBackend;
    connect(opcBackend.data(), &OpcBackend::connectedChanged, [this](bool status) {
        ui->opcConnectionLbl->setText(status ? "<font color=blue> Connected </font>" : "<font color=red> Disconnected </font>");
    });
}

void ConnectionsWindow::on_opcConnectBtn_clicked()
{
    opcBackend->connectToEndPoint(ui->opcIPEdit->text(), 0);
}

void ConnectionsWindow::on_opcDisconnectBtn_clicked()
{
    opcBackend->disconnectFromEndpoint();
}

void ConnectionsWindow::on_cam1ConnectBtn_clicked()
{
    halconCore->connectCamera1();
}

void ConnectionsWindow::on_cam1DisconnectBtn_clicked()
{
    halconCore->disConnectCamera1();
}

void ConnectionsWindow::on_cam2ConnectBtn_clicked()
{
    halconCore->connectCamera2();
}

void ConnectionsWindow::on_cam2DisconnectBtn_clicked()
{
    halconCore->disConnectCamera2();
}

void ConnectionsWindow::onCam1ConnectionChange()
{
    ui->cam1IPEdit->setText(m_cam1Connection.ip);
    ui->trigger1Chk->setChecked(m_cam1Connection.externalTrigger);
    ui->cam1XSizeEdit->setText(QString::number(m_cam1Connection.xsize));
    ui->cam1YSizeEdit->setText(QString::number(m_cam1Connection.ysize));
}

void ConnectionsWindow::onCam2ConnectionChange()
{
    ui->cam2IPEdit->setText(m_cam2Connection.ip);
    ui->trigger2Chk->setChecked(m_cam2Connection.externalTrigger);
    ui->cam2XSizeEdit->setText(QString::number(m_cam2Connection.xsize));
    ui->cam2YSizeEdit->setText(QString::number(m_cam2Connection.ysize));
}

void ConnectionsWindow::onOpcConnectionChange()
{
    ui->opcIPEdit->setText(m_opcConnection.ip);
}

void ConnectionsWindow::onAppConfigChange()
{
    ui->heartbeatIntervalEdit->setText(QString::number(m_appConfig.heartbeatInterval));
}

ConnectionConfig::ApplicationConfig ConnectionsWindow::appConfig() const
{
    return m_appConfig;
}

void ConnectionsWindow::setAppConfig(const ConnectionConfig::ApplicationConfig &newAppConfig)
{
    m_appConfig = newAppConfig;
    emit appConfigChange();
}

ConnectionConfig::OPCConnection ConnectionsWindow::opcConnection() const
{
    return m_opcConnection;
}

void ConnectionsWindow::setOpcConnection(const ConnectionConfig::OPCConnection &newOpcConnection)
{
    m_opcConnection = newOpcConnection;
    emit opcConnectionChange();
}

ConnectionConfig::CameraConnection ConnectionsWindow::cam2Connection() const
{
    return m_cam2Connection;
}

void ConnectionsWindow::setCam2Connection(const ConnectionConfig::CameraConnection &newCam2Connection)
{
    m_cam2Connection = newCam2Connection;
    emit cam2ConnectionChange();
}

ConnectionConfig::CameraConnection ConnectionsWindow::cam1Connection() const
{
    return m_cam1Connection;
}

void ConnectionsWindow::setCam1Connection(const ConnectionConfig::CameraConnection &newCam1Connection)
{
    m_cam1Connection = newCam1Connection;
    emit cam1ConnectionChange();
}

void ConnectionsWindow::on_closeBtn_clicked()
{
    close();
}

