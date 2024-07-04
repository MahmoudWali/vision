#ifndef CONNECTIONSWINDOW_H
#define CONNECTIONSWINDOW_H

#include <QDialog>
#include <QSharedPointer>
#include "opcbackend.h"
#include "halconcore.h"
#include "connectionconfig.h"

namespace Ui {
class OPC;
}

class ConnectionsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionsWindow(QWidget *parent = nullptr);
    ~ConnectionsWindow();

    void assignOpcBackend(QSharedPointer<OpcBackend> newOpcBackend);
    void assignHalconCore(QSharedPointer<HalconCore> newHalconCore);

    ConnectionConfig::CameraConnection cam1Connection() const;
    void setCam1Connection(const ConnectionConfig::CameraConnection &newCam1Connection);

    ConnectionConfig::CameraConnection cam2Connection() const;
    void setCam2Connection(const ConnectionConfig::CameraConnection &newCam2Connection);

    ConnectionConfig::OPCConnection opcConnection() const;
    void setOpcConnection(const ConnectionConfig::OPCConnection &newOpcConnection);

    ConnectionConfig::ApplicationConfig appConfig() const;
    void setAppConfig(const ConnectionConfig::ApplicationConfig &newAppConfig);

signals:
    void cam1ConnectionChange();
    void cam2ConnectionChange();
    void opcConnectionChange();
    void appConfigChange();

private slots:

    void on_opcConnectBtn_clicked();
    void on_opcDisconnectBtn_clicked();

    void on_cam1ConnectBtn_clicked();
    void on_cam1DisconnectBtn_clicked();

    void on_cam2ConnectBtn_clicked();
    void on_cam2DisconnectBtn_clicked();

    void onCam1ConnectionChange();
    void onCam2ConnectionChange();
    void onOpcConnectionChange();
    void onAppConfigChange();


    void on_closeBtn_clicked();

private:
    Ui::OPC *ui;

    QSharedPointer<OpcBackend> opcBackend;
    QSharedPointer<HalconCore> halconCore;

    ConnectionConfig::CameraConnection m_cam1Connection;
    ConnectionConfig::CameraConnection m_cam2Connection;
    ConnectionConfig::OPCConnection m_opcConnection;
    ConnectionConfig::ApplicationConfig m_appConfig;

};

#endif // CONNECTIONSWINDOW_H
