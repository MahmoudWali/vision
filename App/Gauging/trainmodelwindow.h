#ifndef TRAINMODELWINDOW_H
#define TRAINMODELWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QDialog>
#include <QMessageBox>
#include <QSplitter>
#include <QSharedPointer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include "HalconCpp.h"
#include "HDevEngineCpp.h"
#include "qhalconwindow.h"
#include "halconcore.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace Ui {
class TrainModelWindow;
}

using namespace HalconCpp;
using namespace HDevEngineCpp;

class TrainModelWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TrainModelWindow(QWidget *parent = nullptr);
    ~TrainModelWindow();


    void setHalconCore(QSharedPointer<HalconCore> newHalconCore);

    void savePLCRecipe(const QString &modelFullPathName);

private slots:
    void on_saveModelBtn_clicked();

    void on_cancelBtn_clicked();

    void on_tableWidget_cellClicked(int row, int column);

    void on_loadModelBtn_clicked();

public slots:
    void onImageClick(QPointF pos, double lastRow1, double lastCol1, double lastRow2, double lastCol2);

private:
    int getHalconIdFromName(QString id_name);

private:
    Ui::TrainModelWindow *ui;

    QSharedPointer<HalconCore> halconCore;
    QString defaultDXFDirectory;
    QString defualtModelsDirectory;

    QHalconWindow* Disp;

    HImage ImageTemplate;
    HXLD ModelContours;
    HXLD MeaureContours;
    Hlong ImageWidth, ImageHeight;
    int numberMetrologyContours;
    QVector<HalconCore::MeasurementSpec> measurementList;
};

#endif // TRAINMODELWINDOW_H
