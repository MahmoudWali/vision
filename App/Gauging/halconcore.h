#ifndef HALCONCORE_H
#define HALCONCORE_H

#ifdef _WIN32
#define NOMINMAX
#endif // _WIN32

#include <QObject>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QMap>
#include "HalconCpp.h"
#include "HDevEngineCpp.h"
#include "spdlog/spdlog.h"
#include "connectionconfig.h"
#include "cameraworkerthread.h"
#include "CameraParameters.h"

using namespace HalconCpp;
using namespace HDevEngineCpp;

class HalconCore : public QObject
{
    Q_OBJECT
public:
    explicit HalconCore(QObject *parent = nullptr);

    struct MeasurementSpec {
        QString specId;
        double specValue;
        double toleranceMin;
        double toleranceMax;
        QString measType;
        QVector<QVariant> halconID;
        QString notes;
        double measValue;
        QString status;
        double difference;
    };

    struct MeasSpecView {
        QVector<HalconCore::MeasurementSpec> measList;
        HTuple metrologyHandle;
        HTuple modelId;
        HTuple measSpec;
        QString inspectionStatus;
        bool result {false};
        QString partNumber;
        QString view;

    public:
        bool getResult() const;
    };

    void loadImageHDevEngine();

    void openFrameGrabber(const CameraParams& params);
    void connectCamera1();
    void connectCamera2();
    void disConnectCamera1();
    void disConnectCamera2();
    void acquireCamera1(int scanView);
    void acquireCamera2(int scanView);
    void stopCamera1();
    void stopCamera2();
    void closeFrameGrabber(int cameraId);
    void closeAllFrameGrabber();
    void closeAllFrameGrabberByHalcon();
    void createMetrologyFromDxf(QString dxfFileName);
    void generateCameraCalibrationParams();
    void generateMeaurementPlaneParams();
    void saveTrainModel(QString modelFullPathName);
    void loadModel(const QString &view);
    void createAlignmentModel();
    void trainModel(const QString &dxfFileName, const QString &measJsonFile);
    void parseMeasurementSpec(const QString &jsonFilePath);
    void performMeasurement(QString view);

    void scaleDown(double scale, HImage &image, HXLD &modelContour, HXLD &measContour);

    QVector<HalconCore::MeasurementSpec> extractMeasurementFromDict(const HTuple &measurementSpecDict);

    HImage getImage1() const;
    HImage getImage2() const;
    HImage getImage3() const;
    HImage getImage4() const;

    void setCameraConnection(ConnectionConfig::CameraConnection cameraConnection);
    void setCameraAcquringParams(int cameraId, int totalImgeHeight);

    HObject getImageTemplate() const;

    HObject getModelContour() const;

    HObject getMeasureContours() const;

    QMap<QString, HalconCore::MeasSpecView> getMeasSpecViews() const;

    QVector<HalconCore::MeasurementSpec> getMeasListTrain() const;

    void setModelName(const QString &newModelName);
    QString getModelName() const;

    QString modelDirectory() const;
    void setModelDirectory(const QString &newModelDirectory);

    bool getCamera1ConnectionStatus() const;

    bool getCamera2ConnectionStatus() const;

    bool topLoadModelStatus() const;

    bool leftLoadModelStatus() const;

    bool rightLoadModelStatus() const;

    bool bottomLoadModelStatus() const;

signals:
    void processingReady(int sideScan);

    void camera1ConnectionStatus(bool status);
    void camera2ConnectionStatus(bool status);

    void camera1AcquiringStatus(bool status, int scanView);
    void camera2AcquiringStatus(bool status, int scanView);


public slots:
    void handleGrabbedImage1(const HObject &image, int scanView);
    void handleGrabbedImage2(const HObject &image, int scanView);
    void onThreadFinished();

private:
    HDevEngine hDevEngine;
    HDevProgram hDevProgram;
    HDevProgramCall hDevProgramCall;

    HImage Image1;
    HImage Image2;
    HImage Image3;
    HImage Image4;

    std::string dxf2imageProc;
    std::string close_keyence_LJXA;
    std::string open_keyence_LJXA;
    std::string create_metrology_from_dxf;
    std::string gen_cam_par_area_scan_division;
    std::string create_alignment_model;
    std::string parse_measurement_spec;
    std::string perform_measurement;

    // Camera framegrabber handles
    HTuple acqHandle1;
    HTuple acqHandle2;
    bool m_camera1ConnectionStatus {false};
    bool m_camera2ConnectionStatus {false};

    CameraParams m_camera1Params;
    CameraParams m_camera2Params;

    // metrology model
    HTuple measurementPlaneParams;
    HTuple cameraCalibrationParams;
    HTuple metrologyHandle;
    HObject modelContour;
    HObject measureContours;
    HObject imageTemplate;
    HTuple modelId;
    HTuple measurementSpecDict;

    bool m_topLoadModelStatus {false};
    bool m_leftLoadModelStatus {false};
    bool m_rightLoadModelStatus {false};
    bool m_bottomLoadModelStatus {false};

    CameraWorkerThread* camWorkerThread1;
    CameraWorkerThread* camWorkerThread2;

    // training list of measurements from json file
    QVector<HalconCore::MeasurementSpec> measListTrain;

    QMap<QString, HalconCore::MeasSpecView> measSpecViews;
    QString m_modelName;
    QString m_modelDirectory;

    bool topResult { false };
    bool leftResult { false };
    bool rightResult { false };
    bool bottomResult { false };

};

#endif // HALCONCORE_H
