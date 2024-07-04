#ifndef OPCBACKEND_H
#define OPCBACKEND_H

#include <QObject>
#include <QtOpcUa>
#include <QProcess>

class OpcBackend : public QObject
{
    Q_OBJECT
public:

    enum class MachineState : quint32 {
        Idle,
        Scanning,
        Homing,
        Fault
    };

    explicit OpcBackend(QObject *parent = nullptr);
    ~OpcBackend();

    void connectToEndPoint(const QString &url, qint32 index);
    void startInspection();
    void stopInspection();
    void resetSystem();

    void sendPartNumber(const QString &partNumber);
    void sendLength(int imageHeight);
    void sendCurrentSideCombination(int side);
    void setTopActivation(bool enabled);
    void setLeftActivation(bool enabled);
    void setRightActivation(bool enabled);
    void setBottomActivation(bool enabled);
    void sendAppHeartBeat(bool status);
    void sendAppReady(bool status);
    void sendFaultCode(unsigned int code);
    void sendCam1State(unsigned int state);
    void sendCam2State(unsigned int state);
    void sendTopInspectionResult(bool status);
    void sendLeftInspectionResult(bool status);
    void sendRightInspectionResult(bool status);
    void sendBottomInspectionResult(bool status);
    void sendFinalResult(bool result);

    QString message() const;
    void setMessage(const QString &newMessage);

    bool connected() const;

signals:
    void connectedChanged(bool connected);
    void machineStateChanged(MachineState state);
    void switchSideScanSignal();
    void messageChanged();

public slots:
    void endpointsRequestFinished(QList<QOpcUaEndpointDescription> endpoints, QOpcUa::UaStatusCode statusCode, QUrl requestUrl);
    void clientStateChanged(QOpcUaClient::ClientState state);
    void disconnectFromEndpoint();
    void onMethodCallResult(QString methodNodeId, QVariant result, QOpcUa::UaStatusCode statusCode);
    void onMachineStateDataChanged(QOpcUa::NodeAttribute attr, QVariant value);
    void onSwitchSideScanChanged(QOpcUa::NodeAttribute attr, QVariant value);
    void onEnableMonitoringFinished(QOpcUa::NodeAttribute attr, QOpcUa::UaStatusCode status);
    void onLengthChanged(QOpcUa::NodeAttribute attr, QVariant value);
    void onPartNumberChanged(QOpcUa::NodeAttribute attr, QVariant value);
    void onCurrentSideCombinationChanged(QOpcUa::NodeAttribute attr, QVariant value);
    void onTopActivationoChanged(QOpcUa::NodeAttribute attr, QVariant value);
    void onLeftActivationoChanged(QOpcUa::NodeAttribute attr, QVariant value);
    void onRightActivationoChanged(QOpcUa::NodeAttribute attr, QVariant value);
    void onBottomActivationoChanged(QOpcUa::NodeAttribute attr, QVariant value);
    void onAppHeartBeatChanged(QOpcUa::NodeAttribute attr, QVariant value);
    void onAppReadyChanged(QOpcUa::NodeAttribute attr, QVariant value);
    void onFaultCodeChanged(QOpcUa::NodeAttribute attr, QVariant value);
    void onCam1StateChanged(QOpcUa::NodeAttribute attr, QVariant value);
    void onCam2StateChanged(QOpcUa::NodeAttribute attr, QVariant value);
    void onTopResultChanged(QOpcUa::NodeAttribute attr, QVariant value);
    void onLeftResultChanged(QOpcUa::NodeAttribute attr, QVariant value);
    void onRightResultChanged(QOpcUa::NodeAttribute attr, QVariant value);
    void onBottomResultChanged(QOpcUa::NodeAttribute attr, QVariant value);
    void onFinalResultChanged(QOpcUa::NodeAttribute attr, QVariant value);
    void recipetWritten(QOpcUa::NodeAttribute attr, QOpcUa::UaStatusCode status);

private:
    bool m_connected {false};
    QStringList m_backends;

    QScopedPointer<QOpcUaClient> m_client;
    bool m_successfullyCreated {false};

    QScopedPointer<QOpcUaNode> m_machineNode;
    QScopedPointer<QOpcUaNode> m_machineStateNode;
    QScopedPointer<QOpcUaNode> m_switchSideScanNode;
    QScopedPointer<QOpcUaNode> m_LengthNode;
    QScopedPointer<QOpcUaNode> m_currentSideCombinationNode;
    QScopedPointer<QOpcUaNode> m_topActivationNode;
    QScopedPointer<QOpcUaNode> m_leftActivationNode;
    QScopedPointer<QOpcUaNode> m_rightActivationNode;
    QScopedPointer<QOpcUaNode> m_bottomActivationNode;
    QScopedPointer<QOpcUaNode> m_finalResultNode;
    QScopedPointer<QOpcUaNode> m_partNumberNode;
    QScopedPointer<QOpcUaNode> m_appHeartBeatNode;
    QScopedPointer<QOpcUaNode> m_appReadyNode;
    QScopedPointer<QOpcUaNode> m_faultCodeNode;
    QScopedPointer<QOpcUaNode> m_cam1StateNode;
    QScopedPointer<QOpcUaNode> m_cam2StateNode;
    QScopedPointer<QOpcUaNode> m_topResultNode;
    QScopedPointer<QOpcUaNode> m_leftResultNode;
    QScopedPointer<QOpcUaNode> m_rightResultNode;
    QScopedPointer<QOpcUaNode> m_bottomResultNode;

    MachineState m_machineState;
    bool m_switchSideScan;
    QString m_message;
    int m_length;
    QString m_partNumber;
    unsigned int m_currentSideCombination;
    bool m_topActivation;
    bool m_leftActivation;
    bool m_rightActivation;
    bool m_bottomActivation;
    bool m_appHeartBeat;
    bool m_appReady;
    unsigned int m_faultCode;
    unsigned int m_cam1State;
    unsigned int m_cam2State;
    bool m_topResult;
    bool m_leftResult;
    bool m_rightResult;
    bool m_bottomResult;
    bool m_finalResult;
};

#endif // OPCBACKEND_H
