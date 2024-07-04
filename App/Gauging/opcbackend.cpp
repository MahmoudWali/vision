#include "opcbackend.h"

OpcBackend::OpcBackend(QObject *parent)
    : QObject{parent}
    , m_connected(false)
    , m_successfullyCreated(false)
    , m_switchSideScan(false)
    , m_message("Ready to connect")
    , m_length (0)
    , m_partNumber("")
    , m_currentSideCombination(0)
    , m_topActivation(false)
    , m_leftActivation(false)
    , m_rightActivation(false)
    , m_bottomActivation(false)
    , m_appHeartBeat(false)
    , m_appReady(false)
    , m_faultCode(0)
    , m_cam1State (false)
    , m_cam2State(false)
    , m_finalResult (0)
{
    QOpcUaProvider provider;
    m_backends = provider.availableBackends();
}

OpcBackend::~OpcBackend()
{
    if (m_client && m_client->state() == QOpcUaClient::ClientState::Connected)
    {
        m_client->disconnectFromEndpoint();
    }
}

void OpcBackend::connectToEndPoint(const QString &url, qint32 index)
{
    if (m_connected)
        return;

    if (index < 0 || index >= m_backends.size())
        return;

    if (!m_client || (m_client && m_client->backend() != m_backends.at(index)))
    {
        QOpcUaProvider provider;
        m_client.reset(provider.createClient(m_backends.at(index)));
        if (m_client)
        {
            QObject::connect(m_client.data(), &QOpcUaClient::endpointsRequestFinished, this, &OpcBackend::endpointsRequestFinished);
            QObject::connect(m_client.data(), &QOpcUaClient::stateChanged, this, &OpcBackend::clientStateChanged);
        }
    }

    // in case client failed to assign backend to it
    if (!m_client)
    {
        qWarning() << "Couldn't create client";
        m_successfullyCreated = false;
        return;
    }

    m_successfullyCreated = true;
    m_client->requestEndpoints(url);
}

void OpcBackend::startInspection()
{
    m_machineNode->callMethod("ns=2;s=Machine.Start");
}

void OpcBackend::stopInspection()
{
    m_machineNode->callMethod("ns=2;s=Machine.Stop");
}

void OpcBackend::resetSystem()
{
    m_machineNode->callMethod("ns=2;s=Machine.Reset");
}

void OpcBackend::sendPartNumber(const QString &partNumber)
{
    if (m_partNumberNode)
    {
        m_partNumberNode->writeAttribute(QOpcUa::NodeAttribute::Value, partNumber);
    }
}

void OpcBackend::sendLength(int length)
{
    if (m_LengthNode)
    {
        m_LengthNode->writeAttribute(QOpcUa::NodeAttribute::Value, length);
    }
}

void OpcBackend::sendCurrentSideCombination(int side)
{
    if (m_currentSideCombinationNode)
    {
        m_currentSideCombinationNode->writeAttribute(QOpcUa::NodeAttribute::Value, side);
    }
}

void OpcBackend::setTopActivation(bool enabled)
{
    if (m_topActivationNode)
    {
        m_topActivationNode->writeAttribute(QOpcUa::NodeAttribute::Value, enabled);
    }
}

void OpcBackend::setLeftActivation(bool enabled)
{
    if (m_leftActivationNode)
    {
        m_leftActivationNode->writeAttribute(QOpcUa::NodeAttribute::Value, enabled);
    }
}

void OpcBackend::setRightActivation(bool enabled)
{
    if (m_rightActivationNode)
    {
        m_rightActivationNode->writeAttribute(QOpcUa::NodeAttribute::Value, enabled);
    }
}

void OpcBackend::setBottomActivation(bool enabled)
{
    if (m_bottomActivationNode)
    {
        m_bottomActivationNode->writeAttribute(QOpcUa::NodeAttribute::Value, enabled);
    }
}

void OpcBackend::sendFinalResult(bool result)
{
    if (m_finalResultNode)
    {
        m_finalResultNode->writeAttribute(QOpcUa::NodeAttribute::Value, result);
    }
}

void OpcBackend::sendAppHeartBeat(bool status)
{
    if (m_appHeartBeatNode)
    {
        m_appHeartBeatNode->writeAttribute(QOpcUa::NodeAttribute::Value, status);
    }
}

void OpcBackend::sendAppReady(bool status)
{
    if (m_appReadyNode)
    {
        m_appReadyNode->writeAttribute(QOpcUa::NodeAttribute::Value, status);
    }
}

void OpcBackend::sendFaultCode(unsigned int code)
{
    if (m_faultCodeNode)
    {
        m_faultCodeNode->writeAttribute(QOpcUa::NodeAttribute::Value, code);
    }
}

void OpcBackend::sendCam1State(unsigned int state)
{
    if (m_cam1StateNode)
    {
        m_cam1StateNode->writeAttribute(QOpcUa::NodeAttribute::Value, state);
    }
}

void OpcBackend::sendCam2State(unsigned int state)
{
    if (m_cam2StateNode)
    {
        m_cam2StateNode->writeAttribute(QOpcUa::NodeAttribute::Value, state);
    }
}

void OpcBackend::sendTopInspectionResult(bool status)
{
    if (m_topResultNode)
    {
        m_topResultNode->writeAttribute(QOpcUa::NodeAttribute::Value, status);
    }
}

void OpcBackend::sendLeftInspectionResult(bool status)
{
    if (m_leftResultNode)
    {
        m_leftResultNode->writeAttribute(QOpcUa::NodeAttribute::Value, status);
    }
}

void OpcBackend::sendRightInspectionResult(bool status)
{
    if (m_rightResultNode)
    {
        m_rightResultNode->writeAttribute(QOpcUa::NodeAttribute::Value, status);
    }
}

void OpcBackend::sendBottomInspectionResult(bool status)
{
    if (m_bottomResultNode)
    {
        m_bottomResultNode->writeAttribute(QOpcUa::NodeAttribute::Value, status);
    }
}

void OpcBackend::endpointsRequestFinished(QList<QOpcUaEndpointDescription> endpoints, QOpcUa::UaStatusCode statusCode, QUrl requestUrl)
{
    if (endpoints.isEmpty())
    {
        qWarning() << "The server did not return any endpoints";
        clientStateChanged(QOpcUaClient::ClientState::Disconnected);
        return;
    }

    m_client->connectToEndpoint(endpoints.at(0));
}

void OpcBackend::clientStateChanged(QOpcUaClient::ClientState state)
{
    m_connected = (state == QOpcUaClient::ClientState::Connected);
    emit connectedChanged(m_connected);

    if (state == QOpcUaClient::ClientState::Connected)
    {
        // add nodes at serverside to communicate with
        m_machineNode.reset(m_client->node("ns=2;s=Machine"));
        m_machineStateNode.reset(m_client->node("ns=2;s=Machine.State"));
        m_switchSideScanNode.reset(m_client->node("ns=2;s=Machine.SwitchSideScan"));
        m_LengthNode.reset(m_client->node("ns=2;s=Machine.Length"));
        m_partNumberNode.reset(m_client->node("ns=2;s=Machine.PartNumber"));
        m_currentSideCombinationNode.reset(m_client->node("ns=2;s=Machine.CurrentSideCombination"));
        m_topActivationNode.reset(m_client->node("ns=2;s=Machine.TopActivation"));
        m_leftActivationNode.reset(m_client->node("ns=2;s=Machine.LeftActivation"));
        m_rightActivationNode.reset(m_client->node("ns=2;s=Machine.RightActivation"));
        m_bottomActivationNode.reset(m_client->node("ns=2;s=Machine.BottomActivation"));
        m_appHeartBeatNode.reset(m_client->node("ns=2;s=Machine.AppHeartBeat"));
        m_appReadyNode.reset(m_client->node("ns=2;s=Machine.AppReady"));
        m_faultCodeNode.reset(m_client->node("ns=2;s=Machine.FaultCode"));
        m_cam1StateNode.reset(m_client->node("ns=2;s=Machine.Cam1Status"));
        m_cam2StateNode.reset(m_client->node("ns=2;s=Machine.Cam2Status"));
        m_finalResultNode.reset(m_client->node("ns=2;s=Machine.FinalResult"));
        m_topResultNode.reset(m_client->node("ns=2;s=Machine.TopResult"));
        m_leftResultNode.reset(m_client->node("ns=2;s=Machine.LeftResult"));
        m_rightResultNode.reset(m_client->node("ns=2;s=Machine.RightResult"));
        m_bottomResultNode.reset(m_client->node("ns=2;s=Machine.BottomResult"));

        // signal-slots connections to response to communication handlers
        // start/stop methods
        connect (m_machineNode.data(), &QOpcUaNode::methodCallFinished, this, &OpcBackend::onMethodCallResult);
        // machine status
        connect (m_machineStateNode.data(), &QOpcUaNode::dataChangeOccurred, this, &OpcBackend::onMachineStateDataChanged);
        connect (m_switchSideScanNode.data(), &QOpcUaNode::dataChangeOccurred, this, &OpcBackend::onSwitchSideScanChanged);

        // send plc recipe
        connect (m_LengthNode.data(), &QOpcUaNode::dataChangeOccurred, this, &OpcBackend::onLengthChanged);
        connect (m_partNumberNode.data(), &QOpcUaNode::dataChangeOccurred, this, &OpcBackend::onPartNumberChanged);
        connect (m_currentSideCombinationNode.data(), &QOpcUaNode::dataChangeOccurred, this, &OpcBackend::onCurrentSideCombinationChanged);
        connect (m_topActivationNode.data(), &QOpcUaNode::dataChangeOccurred, this, &OpcBackend::onTopActivationoChanged);
        connect (m_leftActivationNode.data(), &QOpcUaNode::dataChangeOccurred, this, &OpcBackend::onLeftActivationoChanged);
        connect (m_rightActivationNode.data(), &QOpcUaNode::dataChangeOccurred, this, &OpcBackend::onRightActivationoChanged);
        connect (m_bottomActivationNode.data(), &QOpcUaNode::dataChangeOccurred, this, &OpcBackend::onBottomActivationoChanged);
        connect (m_appHeartBeatNode.data(), &QOpcUaNode::dataChangeOccurred, this, &OpcBackend::onAppHeartBeatChanged);
        connect (m_appReadyNode.data(), &QOpcUaNode::dataChangeOccurred, this, &OpcBackend::onAppReadyChanged);
        connect (m_faultCodeNode.data(), &QOpcUaNode::dataChangeOccurred, this, &OpcBackend::onFaultCodeChanged);
        connect (m_cam1StateNode.data(), &QOpcUaNode::dataChangeOccurred, this, &OpcBackend::onCam1StateChanged);
        connect (m_cam2StateNode.data(), &QOpcUaNode::dataChangeOccurred, this, &OpcBackend::onCam2StateChanged);
        connect (m_topResultNode.data(), &QOpcUaNode::dataChangeOccurred, this, &OpcBackend::onTopResultChanged);
        connect (m_leftResultNode.data(), &QOpcUaNode::dataChangeOccurred, this, &OpcBackend::onLeftResultChanged);
        connect (m_rightResultNode.data(), &QOpcUaNode::dataChangeOccurred, this, &OpcBackend::onRightResultChanged);
        connect (m_bottomResultNode.data(), &QOpcUaNode::dataChangeOccurred, this, &OpcBackend::onBottomResultChanged);
        connect (m_finalResultNode.data(), &QOpcUaNode::dataChangeOccurred, this, &OpcBackend::onFinalResultChanged);


        // Add handles check after writing the values to PLC
        QObject::connect(m_LengthNode.data(), &QOpcUaNode::attributeWritten, this, &OpcBackend::recipetWritten);
        QObject::connect(m_partNumberNode.data(), &QOpcUaNode::attributeWritten, this, &OpcBackend::recipetWritten);
        QObject::connect(m_currentSideCombinationNode.data(), &QOpcUaNode::attributeWritten, this, &OpcBackend::recipetWritten);
        QObject::connect(m_topActivationNode.data(), &QOpcUaNode::attributeWritten, this, &OpcBackend::recipetWritten);
        QObject::connect(m_leftActivationNode.data(), &QOpcUaNode::attributeWritten, this, &OpcBackend::recipetWritten);
        QObject::connect(m_rightActivationNode.data(), &QOpcUaNode::attributeWritten, this, &OpcBackend::recipetWritten);
        QObject::connect(m_bottomActivationNode.data(), &QOpcUaNode::attributeWritten, this, &OpcBackend::recipetWritten);
        QObject::connect(m_appHeartBeatNode.data(), &QOpcUaNode::attributeWritten, this, &OpcBackend::recipetWritten);
        QObject::connect(m_appReadyNode.data(), &QOpcUaNode::attributeWritten, this, &OpcBackend::recipetWritten);
        QObject::connect(m_faultCodeNode.data(), &QOpcUaNode::attributeWritten, this, &OpcBackend::recipetWritten);
        QObject::connect(m_cam1StateNode.data(), &QOpcUaNode::attributeWritten, this, &OpcBackend::recipetWritten);
        QObject::connect(m_cam2StateNode.data(), &QOpcUaNode::attributeWritten, this, &OpcBackend::recipetWritten);
        QObject::connect(m_topResultNode.data(), &QOpcUaNode::attributeWritten, this, &OpcBackend::recipetWritten);
        QObject::connect(m_leftResultNode.data(), &QOpcUaNode::attributeWritten, this, &OpcBackend::recipetWritten);
        QObject::connect(m_rightResultNode.data(), &QOpcUaNode::attributeWritten, this, &OpcBackend::recipetWritten);
        QObject::connect(m_bottomResultNode.data(), &QOpcUaNode::attributeWritten, this, &OpcBackend::recipetWritten);
        QObject::connect(m_finalResultNode.data(), &QOpcUaNode::attributeWritten, this, &OpcBackend::recipetWritten);


        // activate monitoring the nodes values at 100 ms
        m_machineStateNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
        m_switchSideScanNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
        m_LengthNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
        m_partNumberNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
        m_currentSideCombinationNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
        m_topActivationNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
        m_leftActivationNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
        m_rightActivationNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
        m_bottomActivationNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
        m_appHeartBeatNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
        m_appReadyNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
        m_faultCodeNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
        m_cam1StateNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
        m_cam2StateNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
        m_topResultNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
        m_leftResultNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
        m_rightResultNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
        m_bottomResultNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
        m_finalResultNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));

        connect (m_machineStateNode.data(), &QOpcUaNode::enableMonitoringFinished, this, &OpcBackend::onEnableMonitoringFinished);
        connect (m_switchSideScanNode.data(), &QOpcUaNode::enableMonitoringFinished, this, &OpcBackend::onEnableMonitoringFinished);
        connect (m_LengthNode.data(), &QOpcUaNode::enableMonitoringFinished, this, &OpcBackend::onEnableMonitoringFinished);
        connect (m_partNumberNode.data(), &QOpcUaNode::enableMonitoringFinished, this, &OpcBackend::onEnableMonitoringFinished);
        connect (m_currentSideCombinationNode.data(), &QOpcUaNode::enableMonitoringFinished, this, &OpcBackend::onEnableMonitoringFinished);
        connect (m_topActivationNode.data(), &QOpcUaNode::enableMonitoringFinished, this, &OpcBackend::onEnableMonitoringFinished);
        connect (m_leftActivationNode.data(), &QOpcUaNode::enableMonitoringFinished, this, &OpcBackend::onEnableMonitoringFinished);
        connect (m_rightActivationNode.data(), &QOpcUaNode::enableMonitoringFinished, this, &OpcBackend::onEnableMonitoringFinished);
        connect (m_bottomActivationNode.data(), &QOpcUaNode::enableMonitoringFinished, this, &OpcBackend::onEnableMonitoringFinished);
        connect (m_appHeartBeatNode.data(), &QOpcUaNode::enableMonitoringFinished, this, &OpcBackend::onEnableMonitoringFinished);
        connect (m_appReadyNode.data(), &QOpcUaNode::enableMonitoringFinished, this, &OpcBackend::onEnableMonitoringFinished);
        connect (m_faultCodeNode.data(), &QOpcUaNode::enableMonitoringFinished, this, &OpcBackend::onEnableMonitoringFinished);
        connect (m_cam1StateNode.data(), &QOpcUaNode::enableMonitoringFinished, this, &OpcBackend::onEnableMonitoringFinished);
        connect (m_cam2StateNode.data(), &QOpcUaNode::enableMonitoringFinished, this, &OpcBackend::onEnableMonitoringFinished);
        connect (m_topResultNode.data(), &QOpcUaNode::enableMonitoringFinished, this, &OpcBackend::onEnableMonitoringFinished);
        connect (m_leftResultNode.data(), &QOpcUaNode::enableMonitoringFinished, this, &OpcBackend::onEnableMonitoringFinished);
        connect (m_rightResultNode.data(), &QOpcUaNode::enableMonitoringFinished, this, &OpcBackend::onEnableMonitoringFinished);
        connect (m_bottomResultNode.data(), &QOpcUaNode::enableMonitoringFinished, this, &OpcBackend::onEnableMonitoringFinished);
        connect (m_finalResultNode.data(), &QOpcUaNode::enableMonitoringFinished, this, &OpcBackend::onEnableMonitoringFinished);
    }

}

void OpcBackend::disconnectFromEndpoint()
{
    if (m_connected)
    {
        m_client->disconnectFromEndpoint();
    }
}


void OpcBackend::onMethodCallResult(QString methodNodeId, QVariant result, QOpcUa::UaStatusCode statusCode)
{
    Q_UNUSED(result);

    if (methodNodeId == "ns=2;s=Machine.Start")
    {
        if (statusCode == QOpcUa::UaStatusCode::Good)
        {
            setMessage("Scan successfully started");
        }
        else
        {
            setMessage("Unable to start scan");
        }
    }
    else if (methodNodeId == "ns=2;Machine.Stop")
    {
        if (statusCode == QOpcUa::UaStatusCode::Good)
        {
            setMessage("Scan successfully stopped");
        }
        else
        {
            setMessage("Unable to stop scan");
        }
    }
    else if (methodNodeId == "ns=2;Machine.Reset")
    {
        if (statusCode == QOpcUa::UaStatusCode::Good)
        {
            setMessage("Reset scanner position");
        }
        else
        {
            setMessage("Unable to rest the scanner position");
        }
    }
}

void OpcBackend::onMachineStateDataChanged(QOpcUa::NodeAttribute attr, QVariant value)
{
    Q_UNUSED(attr);

    MachineState state = static_cast<MachineState>(value.toUInt());
    if (state != m_machineState)
    {
        m_machineState = state;
        emit machineStateChanged(m_machineState);
    }
}

void OpcBackend::onSwitchSideScanChanged(QOpcUa::NodeAttribute attr, QVariant value)
{
    Q_UNUSED(attr);

    bool switchSide = value.toBool();
    if (switchSide != m_switchSideScan)
    {
        m_switchSideScan = switchSide;
        emit switchSideScanSignal();
    }
}

void OpcBackend::onEnableMonitoringFinished(QOpcUa::NodeAttribute attr, QOpcUa::UaStatusCode status)
{
    Q_UNUSED(attr);

    if (!sender())
        return;

    if (status == QOpcUa::UaStatusCode::Good)
    {
        qDebug() << "Monitoring successfully enabled for " << qobject_cast<QOpcUaNode*>(sender())->nodeId();
        setMessage("Monitoring successfully enabled");
    }
    else
    {
        qDebug() << "Failed to enable monitoring for " << qobject_cast<QOpcUaNode*>(sender())->nodeId() << " : " << status;
        setMessage("Failed to enable monitoring");
    }
}

void OpcBackend::onLengthChanged(QOpcUa::NodeAttribute attr, QVariant value)
{
    Q_UNUSED(attr);
    m_length = value.toInt();
}

void OpcBackend::onFinalResultChanged(QOpcUa::NodeAttribute attr, QVariant value)
{
    Q_UNUSED(attr);
    m_finalResult = value.toInt();
}

void OpcBackend::onPartNumberChanged(QOpcUa::NodeAttribute attr, QVariant value)
{
    Q_UNUSED(attr);
    m_partNumber = value.toString();
}

void OpcBackend::onCurrentSideCombinationChanged(QOpcUa::NodeAttribute attr, QVariant value)
{
    Q_UNUSED(attr);
    m_currentSideCombination = value.toUInt();
}

void OpcBackend::onTopActivationoChanged(QOpcUa::NodeAttribute attr, QVariant value)
{
    Q_UNUSED(attr);
    m_topActivation = value.toBool();
}

void OpcBackend::onLeftActivationoChanged(QOpcUa::NodeAttribute attr, QVariant value)
{
    Q_UNUSED(attr);
    m_leftActivation = value.toBool();
}

void OpcBackend::onRightActivationoChanged(QOpcUa::NodeAttribute attr, QVariant value)
{
    Q_UNUSED(attr);
    m_rightActivation = value.toBool();
}

void OpcBackend::onBottomActivationoChanged(QOpcUa::NodeAttribute attr, QVariant value)
{
    Q_UNUSED(attr);
    m_bottomActivation = value.toBool();
}

void OpcBackend::onAppHeartBeatChanged(QOpcUa::NodeAttribute attr, QVariant value)
{
    Q_UNUSED(attr);
    m_appHeartBeat = value.toBool();
}

void OpcBackend::onAppReadyChanged(QOpcUa::NodeAttribute attr, QVariant value)
{
    Q_UNUSED(attr);
    m_appReady = value.toBool();
}

void OpcBackend::onFaultCodeChanged(QOpcUa::NodeAttribute attr, QVariant value)
{
    Q_UNUSED(attr);
    m_faultCode = value.toUInt();
}

void OpcBackend::onCam1StateChanged(QOpcUa::NodeAttribute attr, QVariant value)
{
    Q_UNUSED(attr);
    m_cam1State = value.toUInt();
}

void OpcBackend::onCam2StateChanged(QOpcUa::NodeAttribute attr, QVariant value)
{
    Q_UNUSED(attr);
    m_cam2State = value.toUInt();
}

void OpcBackend::onTopResultChanged(QOpcUa::NodeAttribute attr, QVariant value)
{
    Q_UNUSED(attr);
    m_topResult = value.toBool();
}

void OpcBackend::onLeftResultChanged(QOpcUa::NodeAttribute attr, QVariant value)
{
    Q_UNUSED(attr);
    m_leftResult = value.toBool();
}

void OpcBackend::onRightResultChanged(QOpcUa::NodeAttribute attr, QVariant value)
{
    Q_UNUSED(attr);
    m_rightResult = value.toBool();
}

void OpcBackend::onBottomResultChanged(QOpcUa::NodeAttribute attr, QVariant value)
{
    Q_UNUSED(attr);
    m_bottomResult = value.toBool();
}

void OpcBackend::recipetWritten(QOpcUa::NodeAttribute attr, QOpcUa::UaStatusCode status)
{
    // set message for successful written value
}

bool OpcBackend::connected() const
{
    return m_connected;
}

QString OpcBackend::message() const
{
    return m_message;
}

void OpcBackend::setMessage(const QString &newMessage)
{
    if (m_message == newMessage)
        return;
    m_message = newMessage;
    emit messageChanged();
}
