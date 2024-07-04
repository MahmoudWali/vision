#ifndef CONNECTIONCONFIG_H
#define CONNECTIONCONFIG_H

#include <QObject>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDebug>

class ConnectionConfig : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionConfig(QObject *parent = nullptr);

    QString getConfigFileName() const;
    void setConfigFileName(const QString &newConfigFileName);

    bool readConfigFile();

    struct CameraConnection {
        int cameraId;
        QString ip;
        bool externalTrigger;
        int xsize;
        int ysize;
        int timeout;
    };

    struct OPCConnection {
        QString ip;
    };


    struct ApplicationConfig {
        int heartbeatInterval {1000};
    };

    // cam1
    CameraConnection cam1Connection() const;
    void setCam1Connection(const CameraConnection &newCam1Connection);

    // cam2
    CameraConnection cam2Connection() const;
    void setCam2Connection(const CameraConnection &newCam2Connection);

    // opc
    OPCConnection opcConnection() const;
    void setOpcConnection(const OPCConnection &newOpcConnection);

    // application configuration
    ApplicationConfig appConfig() const;
    void setAppConfig(const ApplicationConfig &newAppConfig);

private:
    QString m_configFileName;

    CameraConnection m_cam1Connection;
    CameraConnection m_cam2Connection;
    OPCConnection m_opcConnection;
    ApplicationConfig m_appConfig;

};

#endif // CONNECTIONCONFIG_H
