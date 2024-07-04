#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QDialog>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QSplitter>
#include <QLabel>
#include <QSplitter>
#include <QMap>
#include <QTableWidget>
#include <QSharedPointer>
#include <QTimer>
#include "trainmodelwindow.h"
#include "connectionswindow.h"
#include "HalconCpp.h"
#include "HDevEngineCpp.h"
#include "qhalconwindow.h"
#include "connectionconfig.h"
#include "halconcore.h"
#include "exportcsv.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using namespace HalconCpp;
using namespace HDevEngineCpp;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    enum FAULT_CODES {
        CAMAERS_CONNECTED = 0,
        CAM1_NOT_CONNECTED,
        CAM2_NOT_CONNECTED,
        CAM1_CAM2_NOT_CONNECTED,
    };

    void LoadConnectionConfig();
    bool LoadCameraParams();
    void connectCameras();
    void extracted();
    void acquireCameraImage();
    void applyInspection(int sideView);

    void tableFillReference(QVector<HalconCore::MeasurementSpec> measurementList, QTableWidget* tableWidget);
    void tableFillInspection(QVector<HalconCore::MeasurementSpec> measurementList, QTableWidget* tableWidget);

    void initializeSystem();
    void loadPLCRecipe(const QString &modelDir,
                       QString modelName,
                       unsigned int &length,
                       QString &partNumber,
                       bool &topActive,
                       bool &leftActive,
                       bool &rightActive,
                       bool &bottomActive);
    void updateFaultCode();

    void setResultLabel(bool sideActivation, QLabel *sideViewLabel, QLabel *resultLabel);

    void resetViewersAndResultLabels();


    void startSystem();

    void SwitchScan_from_TopLeft_to_RightBottom();

    void SwitchScan_from_RightBottom_to_TopLeft();

    void DisplayHalconImage(QHalconWindow* Disp, const HImage &Image, Hlong ImageWidth, Hlong ImageHeight);

private slots:

    void onProcessing(int sideScan);

    void on_trainModelBtn_clicked();

    void on_loadModelBtn_clicked();

    void on_opcBtn_clicked();

    void on_startBtn_clicked();

    void on_stopBtn_clicked();

    void onAnyStartStopClick();

    void onSystemInitailize(bool status);

    void onOpcConnectionChange(bool status);
    void onMachineStateChange(OpcBackend::MachineState state);
    void onSwitchSideSignal();
    void onCam1ConnectionChange(bool status);
    void onCam2ConnectionChange(bool status);
    void onCam1AcquiringChange(bool status, int scanView);
    void onCam2AcquiringChange(bool status, int scanView);
    void on_resetBtn_clicked();
    void on_reportBtn_clicked();
    void on_topViewActiveChk_toggled(bool checked);
    void on_leftViewActiveChk_toggled(bool checked);
    void on_rightViewActiveChk_toggled(bool checked);
    void on_bottomViewActiveChk_toggled(bool checked);
    void onHeartBeat();
private:
    Ui::MainWindow *ui;
    QHalconWindow* Disp1;
    QHalconWindow* Disp2;
    QHalconWindow* Disp3;
    QHalconWindow* Disp4;
    HImage Image1;
    HImage Image2;
    HImage Image3;
    HImage Image4;
    QSharedPointer<HalconCore> halconCore;

    ConnectionConfig::CameraConnection m_cam1Connection;
    ConnectionConfig::CameraConnection m_cam2Connection;
    ConnectionConfig::OPCConnection m_opcConnection;
    ConnectionConfig::ApplicationConfig m_appConfig;

    bool m_loadModelStatus{false};
    bool m_systemStatus {false};
    unsigned int m_faultCode;

    bool scanTopDone {false};
    bool scanLeftDone {false};
    bool scanRightDone {false};
    bool scanBottomDone {false};

    bool scanBothTopLeft { true };
    bool scanBothBottomRight { false };

    QString defualtModelsDirectory;

    std::shared_ptr<spdlog::logger> file_logger;

    QSharedPointer<OpcBackend> opcBackend;
    ConnectionsWindow *connectionsWindow;

    unsigned int m_length;
    QString m_partNumber;
    bool m_topActive;
    bool m_leftActive;
    bool m_rightActive;
    bool m_bottomActive;

    int numActivatedScans {0};
    int numInspections {0};

    QTimer *heartBeatTimer;

protected:
    void closeEvent(QCloseEvent *event);
};
#endif // MAINWINDOW_H
