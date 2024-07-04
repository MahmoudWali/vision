#include "connectionconfig.h"

ConnectionConfig::ConnectionConfig(QObject *parent)
    : QObject{parent}
{}

QString ConnectionConfig::getConfigFileName() const
{
    return m_configFileName;
}

void ConnectionConfig::setConfigFileName(const QString &newConfigFileName)
{
    m_configFileName = newConfigFileName;
}

bool ConnectionConfig::readConfigFile()
{
    // check if json file name is not empty
    if (m_configFileName.isEmpty())
        return false;

    // open json file
    QFile jsonFile(m_configFileName);
    if (!jsonFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open file for reading.";
        return false;
    }

    // read json file to Byte array
    QByteArray data = jsonFile.readAll();
    jsonFile.close();

    // parse json file as json document with error checking
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError)
    {
        qDebug() << "Error parsing JSON: " << error.errorString();
        return false;
    }


    // check if json file contains only objects
    if (!jsonDoc.isObject())
    {
        qDebug() << "Json file doesn't contain objects";
        return false;
    }

    // Access the 3 objects "Camera1", "Camera2", "OPC"
    QJsonObject rootObject = jsonDoc.object();
    QJsonObject camera1Object = rootObject.value("Camera1").toObject();
    QJsonObject camera2Object = rootObject.value("Camera2").toObject();
    QJsonObject opcObject = rootObject.value("OPC").toObject();
    QJsonObject application = rootObject.value("Application").toObject();

    // Access properties

    //cam1
    m_cam1Connection.cameraId = camera1Object.value("ID").toInt();
    m_cam1Connection.ip = camera1Object.value("IP").toString();
    m_cam1Connection.externalTrigger = camera1Object.value("ExternalTrig").toBool();
    m_cam1Connection.xsize = camera1Object.value("Xsize").toInt();
    m_cam1Connection.ysize = camera1Object.value("Ysize").toInt();
    m_cam1Connection.timeout = camera1Object.value("TimeOut").toInt();

    // cam2
    m_cam2Connection.cameraId = camera2Object.value("ID").toInt();
    m_cam2Connection.ip = camera2Object.value("IP").toString();
    m_cam2Connection.externalTrigger = camera2Object.value("ExternalTrig").toBool();
    m_cam2Connection.xsize = camera2Object.value("Xsize").toInt();
    m_cam2Connection.ysize = camera2Object.value("Ysize").toInt();
    m_cam2Connection.timeout = camera2Object.value("TimeOut").toInt();

    // opc
    m_opcConnection.ip = opcObject.value("IP").toString();

    // application
    m_appConfig.heartbeatInterval = application.value("HeartBeat").toInt();

    return true;
}

ConnectionConfig::CameraConnection ConnectionConfig::cam1Connection() const
{
    return m_cam1Connection;
}

void ConnectionConfig::setCam1Connection(const CameraConnection &newCam1Connection)
{
    m_cam1Connection = newCam1Connection;
}

ConnectionConfig::CameraConnection ConnectionConfig::cam2Connection() const
{
    return m_cam2Connection;
}

void ConnectionConfig::setCam2Connection(const CameraConnection &newCam2Connection)
{
    m_cam2Connection = newCam2Connection;
}

ConnectionConfig::OPCConnection ConnectionConfig::opcConnection() const
{
    return m_opcConnection;
}

void ConnectionConfig::setOpcConnection(const OPCConnection &newOpcConnection)
{
    m_opcConnection = newOpcConnection;
}

ConnectionConfig::ApplicationConfig ConnectionConfig::appConfig() const
{
    return m_appConfig;
}

void ConnectionConfig::setAppConfig(const ApplicationConfig &newAppConfig)
{
    m_appConfig = newAppConfig;
}
