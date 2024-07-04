#ifndef CAMERAWORKERTHREAD_H
#define CAMERAWORKERTHREAD_H

#include <QThread>
#include <QObject>
#include <QDebug>
#include "HalconCpp.h"
#include "HDevEngineCpp.h"
#include "CameraParameters.h"
#include "spdlog/spdlog.h"

using namespace HalconCpp;
using namespace HDevEngineCpp;

class CameraWorkerThread : public QThread
{
    Q_OBJECT
public:
    explicit CameraWorkerThread(QObject *parent = nullptr);

    void setParams(const CameraParams &newParams);

    HTuple getAcqHandle() const;
    void setAcqHandle(const HTuple &newAcqHandle);

    int getScanView() const;
    void setScanView(int newScanView);

    bool stop() const;
    void setStop(bool newStop);

signals:
    void imageReady(const HObject &image, int scanView);

    // QThread interface
protected:
    void run();

private:
    HTuple acqHandle;
    std::string acquire_image;
    CameraParams params;
    int scanView;

    bool m_stop { false };
};

#endif // CAMERAWORKERTHREAD_H
