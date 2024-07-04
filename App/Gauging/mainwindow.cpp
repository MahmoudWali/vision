#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_faultCode(0)
    ,m_length(1000)
    ,m_partNumber("")
    ,m_topActive(false)
    ,m_leftActive(false)
    ,m_rightActive(false)
    ,m_bottomActive(false)
{
    ui->setupUi(this);

    setWindowTitle("Gauging | Industry Tech Solutions");

    file_logger = spdlog::basic_logger_mt("file_logger", "logfile.txt");
    spdlog::set_level(spdlog::level::debug);

    file_logger->info("System initialization...");

    defualtModelsDirectory = "D:/Data/Models/";

    ui->tabWidget->setStyleSheet("QTabBar::tab {"
                                 "background: #D9D9D9;"
                                 "border: 2px solid #F0F0F0;"
                                 "border-radius: 8px;"
                                 "padding: 8px;"
                                 "font-family: Calibri;"
                                 "font-size: 16px;"
                                 "}"
                                 "QTabBar::tab:selected {"
                                 "background: wheat;"
                                 "font-family: Calibri;"
                                 "font-weight: bold;"
                                 "font-size: 16px;"
                                 "}"
                                 "QTabBar::tab:hover {"
                                 "background: wheat;"
                                 "font-family: Calibri;"
                                 "font-size: 16px;"
                                 "}");

    QString tableStyleSheet =
        "QHeaderView::section {"
        "background-color: antiquewhite;"
        "color: black;"
        "font-family: Calibri;"
        "font-weight: bold;"
        "font-size: 16px;"
        "border: 1px solid floralwhite;"
        "padding: 4px;"
        "}"
        "QTableWidget::item {"
        "font-size: 14px;"
        "font-family: Calibri;"
        "}" ;

    ui->tableTop->setStyleSheet(tableStyleSheet);
    ui->tableLeft->setStyleSheet(tableStyleSheet);
    ui->tableRight->setStyleSheet(tableStyleSheet);
    ui->tableBottom->setStyleSheet(tableStyleSheet);

    ui->tableTop->setFont(QFont("Calibri", 13));
    ui->tableLeft->setFont(QFont("Calibri", 13));
    ui->tableRight->setFont(QFont("Calibri", 13));
    ui->tableBottom->setFont(QFont("Calibri", 13));


    halconCore = QSharedPointer<HalconCore>(new HalconCore());
    connect(halconCore.data(), &HalconCore::camera1ConnectionStatus, this, &MainWindow::onCam1ConnectionChange);
    connect(halconCore.data(), &HalconCore::camera2ConnectionStatus, this, &MainWindow::onCam2ConnectionChange);
    connect(halconCore.data(), &HalconCore::camera1AcquiringStatus, this, &MainWindow::onCam1AcquiringChange);
    connect(halconCore.data(), &HalconCore::camera2AcquiringStatus, this, &MainWindow::onCam2AcquiringChange);
    connect(halconCore.data(), &HalconCore::camera1ConnectionStatus, this, &MainWindow::onSystemInitailize);
    connect(halconCore.data(), &HalconCore::camera2ConnectionStatus, this, &MainWindow::onSystemInitailize);
    connect(halconCore.data(), &HalconCore::processingReady, this, &MainWindow::onProcessing);

    Disp1 = new QHalconWindow(this);
    Disp1->setMinimumSize(50, 50);

    Disp2 = new QHalconWindow(this);
    Disp2->setMinimumSize(50, 50);

    Disp3 = new QHalconWindow(this);
    Disp3->setMinimumSize(50, 50);

    Disp4 = new QHalconWindow(this);
    Disp4->setMinimumSize(50, 50);

    // One layout for HALCON widget and hint label
    QLabel *topViewLabel = new QLabel(this);
    QLabel *bottomViewLabel = new QLabel(this);
    QLabel *leftViewLabel = new QLabel(this);
    QLabel *rightViewLabel = new QLabel(this);

    topViewLabel->setText("Top View");
    topViewLabel->setFixedHeight(15);
    topViewLabel->setAlignment(Qt::AlignCenter);
    QFont labelFont = topViewLabel->font();
    labelFont.setBold(false);
    topViewLabel->setFont(labelFont);


    bottomViewLabel->setText("Bottom View");
    bottomViewLabel->setFixedHeight(15);
    bottomViewLabel->setAlignment(Qt::AlignCenter);
    bottomViewLabel->setFont(labelFont);

    leftViewLabel->setText("Left View");
    leftViewLabel->setFixedHeight(15);
    leftViewLabel->setAlignment(Qt::AlignCenter);
    leftViewLabel->setFont(labelFont);

    rightViewLabel->setText("Right View");
    rightViewLabel->setFixedHeight(15);
    rightViewLabel->setAlignment(Qt::AlignCenter);
    rightViewLabel->setFont(labelFont);

    ui->verticalLayout->addWidget(topViewLabel, 1);
    ui->verticalLayout->addWidget(Disp1, 1);
    ui->verticalLayout->addWidget(leftViewLabel, 1);
    ui->verticalLayout->addWidget(Disp2, 1);
    ui->verticalLayout->addWidget(rightViewLabel, 1);
    ui->verticalLayout->addWidget(Disp3, 1);
    ui->verticalLayout->addWidget(bottomViewLabel, 1);
    ui->verticalLayout->addWidget(Disp4, 1);
    ui->verticalLayout->addSpacing(8);

    ui->tableTop->setColumnCount(9);
    ui->tableTop->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableTop->setHorizontalHeaderLabels({"SpecID", "RefValue (mm)", "InspValue (mm)", "Diff (mm)", "Tol+ (mm)", "Tol- (mm)", "Meaurement Type", "Notes", "Status"});
    ui->tableTop->setColumnWidth(0, 100);
    ui->tableTop->setColumnWidth(1, 150);
    ui->tableTop->setColumnWidth(2, 150);
    ui->tableTop->setColumnWidth(3, 150);
    ui->tableTop->setColumnWidth(4, 150);
    ui->tableTop->setColumnWidth(5, 150);
    ui->tableTop->setColumnWidth(6, 150);
    ui->tableTop->setColumnWidth(7, 200);
    ui->tableTop->setColumnWidth(8, 150);


    ui->tableBottom->setColumnCount(9);
    ui->tableBottom->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableBottom->setHorizontalHeaderLabels({"SpecID", "RefValue (mm)", "InspValue (mm)", "Diff (mm)", "Tol+ (mm)", "Tol- (mm)", "Meaurement Type", "Notes", "Status"});
    ui->tableBottom->setColumnWidth(0, 100);
    ui->tableBottom->setColumnWidth(1, 150);
    ui->tableBottom->setColumnWidth(2, 150);
    ui->tableBottom->setColumnWidth(3, 150);
    ui->tableBottom->setColumnWidth(4, 150);
    ui->tableBottom->setColumnWidth(5, 150);
    ui->tableBottom->setColumnWidth(6, 150);
    ui->tableBottom->setColumnWidth(7, 200);
    ui->tableBottom->setColumnWidth(8, 150);


    ui->tableLeft->setColumnCount(9);
    ui->tableLeft->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableLeft->setHorizontalHeaderLabels({"SpecID", "RefValue (mm)", "InspValue (mm)", "Diff (mm)", "Tol+ (mm)", "Tol- (mm)", "Meaurement Type", "Notes", "Status"});
    ui->tableLeft->setColumnWidth(0, 100);
    ui->tableLeft->setColumnWidth(1, 150);
    ui->tableLeft->setColumnWidth(2, 150);
    ui->tableLeft->setColumnWidth(3, 150);
    ui->tableLeft->setColumnWidth(4, 150);
    ui->tableLeft->setColumnWidth(5, 150);
    ui->tableLeft->setColumnWidth(6, 150);
    ui->tableLeft->setColumnWidth(7, 200);
    ui->tableLeft->setColumnWidth(8, 150);


    ui->tableRight->setColumnCount(9);
    ui->tableRight->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableRight->setHorizontalHeaderLabels({"SpecID", "RefValue (mm)", "InspValue (mm)", "Diff (mm)", "Tol+ (mm)", "Tol- (mm)", "Meaurement Type", "Notes", "Status"});
    ui->tableRight->setColumnWidth(0, 100);
    ui->tableRight->setColumnWidth(1, 150);
    ui->tableRight->setColumnWidth(2, 150);
    ui->tableRight->setColumnWidth(3, 150);
    ui->tableRight->setColumnWidth(4, 150);
    ui->tableRight->setColumnWidth(5, 150);
    ui->tableRight->setColumnWidth(6, 150);
    ui->tableRight->setColumnWidth(7, 200);
    ui->tableRight->setColumnWidth(8, 150);

    // the tab widget that contains the table top, left, right, bottom
    ui->tabWidget->setMinimumHeight(100);

    // add splitter between table widget and views to allow sizing
    QWidget *containerWidget = new QWidget();
    containerWidget->setLayout(ui->verticalLayout);
    //
    QSplitter *splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Vertical);
    splitter->setChildrenCollapsible(false);
    // add the tab widget first, then 4 viewers between splitter
    splitter->addWidget(ui->tabWidget);
    splitter->addWidget(containerWidget);
    ui->viewerGroupBox->setLayout(new QVBoxLayout);
    ui->viewerGroupBox->layout()->addWidget(splitter);
    //

    // OPC settings
    opcBackend = QSharedPointer<OpcBackend>(new OpcBackend());
    connect(opcBackend.data(), &OpcBackend::connectedChanged, this, &MainWindow::onOpcConnectionChange);
    connect(opcBackend.data(), &OpcBackend::connectedChanged, this, &MainWindow::onSystemInitailize);
    connect(opcBackend.data(), &OpcBackend::machineStateChanged, this, &MainWindow::onMachineStateChange);
    connect(opcBackend.data(), &OpcBackend::switchSideScanSignal, this, &MainWindow::onSwitchSideSignal);

    // connections layer settings
    connectionsWindow = new ConnectionsWindow(this);
    connectionsWindow->assignHalconCore(halconCore);
    connectionsWindow->assignOpcBackend(opcBackend);

    connect(ui->startBtn, &QPushButton::clicked, this, &MainWindow::onAnyStartStopClick);
    connect(ui->stopBtn, &QPushButton::clicked, this, &MainWindow::onAnyStartStopClick);

    heartBeatTimer = new QTimer(this);
    connect (heartBeatTimer, &QTimer::timeout, this, &MainWindow::onHeartBeat);

    LoadConnectionConfig();
    initializeSystem();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::LoadConnectionConfig()
{
    ConnectionConfig config;
    config.setConfigFileName("connection_config.json");
    bool readIsOK = config.readConfigFile();

    if (readIsOK)
    {
        m_cam1Connection = config.cam1Connection();
        m_cam2Connection = config.cam2Connection();
        m_opcConnection = config.opcConnection();
        m_appConfig = config.appConfig();

        connectionsWindow->setCam1Connection(m_cam1Connection);
        connectionsWindow->setCam2Connection(m_cam2Connection);
        connectionsWindow->setOpcConnection(m_opcConnection);
        connectionsWindow->setAppConfig(m_appConfig);
    }
}

void MainWindow::connectCameras()
{
    if (!halconCore->getCamera1ConnectionStatus())
    {
        halconCore->setCameraConnection(m_cam1Connection);
        halconCore->connectCamera1();
    }

    if (!halconCore->getCamera2ConnectionStatus())
    {
        halconCore->setCameraConnection(m_cam2Connection);
        halconCore->connectCamera2();
    }
}

void MainWindow::SwitchScan_from_TopLeft_to_RightBottom()
{
    if (scanTopDone && scanLeftDone)
    {
        if (m_rightActive || m_bottomActive)
        {
            scanBothTopLeft = false;
            scanBothBottomRight = true;
        }
        else
        {
            scanBothTopLeft = true;
            scanBothBottomRight = false;
        }

        scanTopDone = false;
        scanLeftDone = false;

        ui->startBtn->setEnabled(true);
        ui->stopBtn->setEnabled(false);
    }
}

void MainWindow::SwitchScan_from_RightBottom_to_TopLeft()
{
    if (scanRightDone && scanBottomDone)
    {
        if (m_topActive || m_leftActive)
        {
            scanBothBottomRight = false;
            scanBothTopLeft = true;
        }
        else
        {
            scanBothBottomRight = true;
            scanBothTopLeft = false;
        }

        scanRightDone = false;
        scanBottomDone = false;

        ui->startBtn->setEnabled(true);
        ui->stopBtn->setEnabled(false);
    }
}

void MainWindow::acquireCameraImage()
{
    if (halconCore->getCamera1ConnectionStatus() && halconCore->getCamera2ConnectionStatus())
    {
        // collect how many scans is required to product the final result
        numActivatedScans = 0;
        if (m_topActive)
        {
            numActivatedScans++;
        }
        if (m_leftActive)
        {
            numActivatedScans++;
        }
        if (m_rightActive)
        {
            numActivatedScans++;
        }
        if (m_bottomActive)
        {
            numActivatedScans++;
        }
        //=============================//

        // hanlding top-left
        if (scanBothTopLeft)
        {
            // reset viewers and labels
            if (numInspections == 0)
            {
                resetViewersAndResultLabels();
            }

            // top
            if (m_topActive)   // to lock scanning while camera didn't finish first scan
            {
                halconCore->acquireCamera1(0);  // top
            }
            else
            {
                scanTopDone = true;
            }

            // left
            if (m_leftActive)
            {
                halconCore->acquireCamera2(1);  // left
            }
            else
            {
                scanLeftDone = true;
            }

            // switch the scan from top-left to right-bottom
            SwitchScan_from_TopLeft_to_RightBottom();
        }

        if (scanBothBottomRight)
        {
            // reset viewers and labels
            if (numInspections == 0)
            {
                resetViewersAndResultLabels();
            }

            // right
            if (m_rightActive)
            {
                halconCore->acquireCamera2(2);  // right
            }
            else
            {
                scanRightDone = true;
            }

            // bottom
            if (m_bottomActive)
            {
                halconCore->acquireCamera1(3);  // bottom
            }
            else
            {
                scanBottomDone = true;
            }

            // switch the scan right-bottom to top-left
            SwitchScan_from_RightBottom_to_TopLeft();
        }
    }
}

void MainWindow::applyInspection(int sideView)
{
    // get all measurement specification for all views
    if (sideView == 0 && m_topActive)
    {
        halconCore->performMeasurement("Top");
        QMap<QString, HalconCore::MeasSpecView> measSpecViews = halconCore->getMeasSpecViews();
        QVector<HalconCore::MeasurementSpec> measurementList = measSpecViews["Top"].measList;
        tableFillInspection(measurementList, ui->tableTop);
        QString topStatus = (measSpecViews["Top"].inspectionStatus == "Passed") ? "<font color=blue> Passed </font>" : "<font color=red> Failed </font>";
        ui->topResultLbl->setText(topStatus);
        opcBackend->sendTopInspectionResult(topStatus == "Passed" ? true : false);
    }

    if (sideView == 1 && m_leftActive)
    {
        halconCore->performMeasurement("Left");
        QMap<QString, HalconCore::MeasSpecView> measSpecViews = halconCore->getMeasSpecViews();
        QVector<HalconCore::MeasurementSpec> measurementList = measSpecViews["Left"].measList;
        measurementList = measSpecViews["Left"].measList;
        tableFillInspection(measurementList, ui->tableLeft);
        QString leftStatus = (measSpecViews["Left"].inspectionStatus == "Passed") ? "<font color=blue> Passed </font>" : "<font color=red> Failed </font>";
        ui->leftResultLbl->setText(leftStatus);
        opcBackend->sendLeftInspectionResult(leftStatus == "Passed" ? true : false);
    }

    if (sideView == 2 && m_rightActive)
    {
        halconCore->performMeasurement("Right");
        QMap<QString, HalconCore::MeasSpecView> measSpecViews = halconCore->getMeasSpecViews();
        QVector<HalconCore::MeasurementSpec> measurementList = measSpecViews["Right"].measList;
        measurementList = measSpecViews["Right"].measList;
        tableFillInspection(measurementList, ui->tableRight);
        QString rightStatus = (measSpecViews["Right"].inspectionStatus == "Passed") ? "<font color=blue> Passed </font>" : "<font color=red> Failed </font>";
        ui->rightResultLbl->setText(rightStatus);
        opcBackend->sendRightInspectionResult(rightStatus == "Passed" ? true : false);
    }

    if (sideView == 3 && m_bottomActive)
    {
        halconCore->performMeasurement("Bottom");
        QMap<QString, HalconCore::MeasSpecView> measSpecViews = halconCore->getMeasSpecViews();
        QVector<HalconCore::MeasurementSpec> measurementList = measSpecViews["Bottom"].measList;
        measurementList = measSpecViews["Bottom"].measList;
        tableFillInspection(measurementList, ui->tableBottom);
        QString bottomStatus = (measSpecViews["Bottom"].inspectionStatus == "Passed") ? "<font color=blue> Passed </font>" : "<font color=red> Failed </font>";
        ui->bottomResultLbl->setText(bottomStatus);
        opcBackend->sendBottomInspectionResult(bottomStatus == "Passed" ? true : false);
    }
}

void MainWindow::DisplayHalconImage(QHalconWindow* Disp, const HImage &Image, Hlong ImageWidth, Hlong ImageHeight)
{
    Disp->enableShowFullImageSize();
    Disp->setImageSize(ImageWidth, ImageHeight);

    // first we need to set the view to defined area in anyway
    Disp->GetHalconBuffer()->SetPart(0, 0, ImageHeight - 1, ImageWidth - 1);

    // new code...
    // then adjust the scaling
    Hlong row1, col1, row2, col2;
    Disp->GetHalconBuffer()->GetPart(&row1, &col1, &row2, &col2);
    double h = abs(double(row2 - row1));
    double w = abs(double(col2 - col1));
    double center_row = (row2 + row1) / 2.0;
    double center_col = (col2 + col1) / 2.0;

    double scale = double(Disp->height()) / double(Disp->width());

    if (w != 0) {          // check divid on Zero
        double ratio = h / w;

        double zoomRate;
        if (ratio < scale) {
            zoomRate = ratio / scale;
            h = h / zoomRate;
        }
        else {
            zoomRate = scale / ratio;
            w = w / zoomRate;
        }

        row1 = center_row - (h / 2.0);
        col1 = center_col - (w / 2.0);
        row2 = center_row + (h / 2.0);
        col2 = center_col + (w / 2.0);

        Disp->GetHalconBuffer()->SetPart(row1, col1, row2, col2);
    }
    // new code...

    Disp->GetHalconBuffer()->ClearWindow();
    Disp->GetHalconBuffer()->DispObj(Image);
    Disp->GetHalconBuffer()->FlushBuffer();
}

void MainWindow::onProcessing(int scanSide)
{
    qDebug() << "On Processing MainWindow slot is called to display images, side view: " << scanSide;

    if (scanSide == 0)
    {
        scanTopDone = true;

        Image1 = halconCore->getImage1();
        Hlong ImageWidth, ImageHeight;
        Image1.GetImageSize(&ImageWidth, &ImageHeight);
        DisplayHalconImage(Disp1, Image1, ImageWidth, ImageHeight);

        applyInspection(0);
        numInspections++;
    }

    if (scanSide == 1)
    {
        scanLeftDone = true;

        Image2 = halconCore->getImage2();
        Hlong ImageWidth, ImageHeight;
        Image2.GetImageSize(&ImageWidth, &ImageHeight);
        DisplayHalconImage(Disp2, Image2, ImageWidth, ImageHeight);

        applyInspection(1);
        numInspections++;
    }

    if (scanSide == 2)
    {
        scanRightDone = true;

        Image3 = halconCore->getImage3();
        Hlong ImageWidth, ImageHeight;
        Image3.GetImageSize(&ImageWidth, &ImageHeight);
        DisplayHalconImage(Disp3, Image3, ImageWidth, ImageHeight);

        applyInspection(2);
        numInspections++;
    }

    if (scanSide == 3)
    {
        scanBottomDone = true;

        Image4 = halconCore->getImage4();
        Hlong ImageWidth, ImageHeight;
        Image4.GetImageSize(&ImageWidth, &ImageHeight);
        DisplayHalconImage(Disp4, Image4, ImageWidth, ImageHeight);

        applyInspection(3);
        numInspections++;
    }

    // switch the scan from top-left to right-bottom
    SwitchScan_from_TopLeft_to_RightBottom();

    // switch the scan from right-bottom to top-left
    SwitchScan_from_RightBottom_to_TopLeft();

    // give the final result
    if (numInspections == numActivatedScans)  // if scanning top-left is true (that means) new scan is requrested and result of previous can be given
    {
        numInspections = 0;

        QMap<QString, HalconCore::MeasSpecView> measSpecViews = halconCore->getMeasSpecViews();

        bool topResult = false;
        topResult = m_topActive? measSpecViews["Top"].getResult() : true;

        bool leftResult = false;
        if (m_leftActive)
            leftResult = m_leftActive? measSpecViews["Left"].getResult() : true;

        bool rightResult = false;
        rightResult = m_rightActive ? measSpecViews["Right"].getResult() : true;

        bool bottomResult = false;
        bottomResult = m_bottomActive ? measSpecViews["Bottom"].getResult() : true;

        QString finalStatus;
        if (topResult && leftResult && rightResult && bottomResult)
        {
            finalStatus = "<font color=blue> Passed </font>";
            opcBackend->sendFinalResult(true);
        }
        else
        {
            finalStatus = "<font color=red> Failed </font>";
            opcBackend->sendFinalResult(false);
        }

        ui->finalResultLbl->setText(finalStatus);
    }

    ui->reportBtn->setEnabled(true);
}

void MainWindow::on_trainModelBtn_clicked()
{
    TrainModelWindow *trainModelWindow = new TrainModelWindow(this);
    //trainModelWindow->setWindowFlags(trainModelWindow->windowFlags() & ~Qt::WindowMinimizeButtonHint);
    //trainModelWindow->setFixedSize(width(), height());
    trainModelWindow->setHalconCore(halconCore);
    trainModelWindow->showMaximized();
}

void MainWindow::tableFillReference(QVector<HalconCore::MeasurementSpec> measurementList, QTableWidget* tableWidget)
{
    tableWidget->clearContents();
    tableWidget->setRowCount(measurementList.size());
    for (int i = 0; i < measurementList.size(); i++)
    {
        QTableWidgetItem *itemSpecId = new QTableWidgetItem(measurementList[i].specId);
        tableWidget->setItem(i, 0, itemSpecId);

        QTableWidgetItem *itemSpecValue = new QTableWidgetItem(QString::number(measurementList[i].specValue));
        tableWidget->setItem(i, 1, itemSpecValue);

        QTableWidgetItem *itemToleranceMax = new QTableWidgetItem(QString::number(measurementList[i].toleranceMax));
        tableWidget->setItem(i, 4, itemToleranceMax);

        QTableWidgetItem *itemToleranceMin = new QTableWidgetItem(QString::number(measurementList[i].toleranceMin));
        tableWidget->setItem(i, 5, itemToleranceMin);

        QTableWidgetItem *itemMeasType = new QTableWidgetItem(measurementList[i].measType);
        tableWidget->setItem(i, 6, itemMeasType);

        QTableWidgetItem *itemNotes = new QTableWidgetItem(measurementList[i].notes);
        tableWidget->setItem(i, 7, itemNotes);
    }
}

void MainWindow::tableFillInspection(QVector<HalconCore::MeasurementSpec> measurementList, QTableWidget *tableWidget)
{
    tableWidget->setRowCount(measurementList.size());
    for (int i = 0; i < measurementList.size(); i++)
    {

        QTableWidgetItem *itemMeasValue = new QTableWidgetItem(QString::number(measurementList[i].measValue));
        tableWidget->setItem(i, 2, itemMeasValue);

        // calculate diff
        QTableWidgetItem *itemDiff = new QTableWidgetItem(QString::number(measurementList[i].difference));
        tableWidget->setItem(i, 3, itemDiff);

        QTableWidgetItem *itemStatus = new QTableWidgetItem(measurementList[i].status);
        tableWidget->setItem(i, 8, itemStatus);
    }
}

void MainWindow::initializeSystem()
{
    ui->systemConnectionLbl->setText("Initializing");

    // connect to OPC Server
    opcBackend->connectToEndPoint(m_opcConnection.ip, 0);
    // start to send heart beat from app to PLC
    heartBeatTimer->setInterval(m_appConfig.heartbeatInterval);
    heartBeatTimer->start();

    // connect camera1 and camera2
    connectCameras();
}

void MainWindow::loadPLCRecipe(const QString &modelDir,
                               QString modelName,
                               unsigned int &length,
                               QString &partNumber,
                               bool &topActive,
                               bool &leftActive,
                               bool &rightActive,
                               bool &bottomActive)
{
    QString plcRecipeFileName = modelDir + "/" + modelName + "_PLC_Recipe.json";
    qDebug() << plcRecipeFileName;

    QFile jsonFile(plcRecipeFileName);

    if (!jsonFile.open(QIODevice::Text | QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open file for reading";
        return;
    }

    QJsonParseError error;
    QByteArray data = jsonFile.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError)
    {
        qDebug() << "Error parsing json file";
        return;
    }

    if (!jsonDoc.isObject())
    {
        qDebug() << "json file is not object";
        return;
    }

    QJsonObject jsonObject = jsonDoc.object();
    partNumber = jsonObject.value("Part Number").toString();
    length = jsonObject.value("Profile Length").toInt();
    topActive = jsonObject.value("Top View").toBool();
    leftActive = jsonObject.value("Left View").toBool();
    rightActive = jsonObject.value("Right View").toBool();
    bottomActive = jsonObject.value("Bottom View").toBool();
}

void MainWindow::updateFaultCode()
{
    m_faultCode = FAULT_CODES::CAMAERS_CONNECTED;

    if (!halconCore->getCamera1ConnectionStatus())
    {
        m_faultCode = FAULT_CODES::CAM1_NOT_CONNECTED;
    }

    if (!halconCore->getCamera2ConnectionStatus())
    {
        m_faultCode = FAULT_CODES::CAM2_NOT_CONNECTED;
    }

    if (!halconCore->getCamera1ConnectionStatus() && !halconCore->getCamera2ConnectionStatus())
    {
        m_faultCode = FAULT_CODES::CAM1_CAM2_NOT_CONNECTED;
    }
}

void MainWindow::on_loadModelBtn_clicked()
{
    QString modelDir = QFileDialog::getExistingDirectory(this, "Load Model", defualtModelsDirectory);
    QFileInfo fileInfo(modelDir);
    QString modelName = fileInfo.fileName();   // extract model name from directory
    ui->partNoLbl->setText(modelName);

    loadPLCRecipe(modelDir, modelName, m_length, m_partNumber, m_topActive, m_leftActive, m_rightActive, m_bottomActive);
    ui->profileLengthEdit->setText(QString::number(m_length));
    ui->partNumberEdit->setText(m_partNumber);
    ui->topViewActiveChk->setChecked(m_topActive);
    ui->leftViewActiveChk->setChecked(m_leftActive);
    ui->rightViewActiveChk->setChecked(m_rightActive);
    ui->bottomViewActiveChk->setChecked(m_bottomActive);

    opcBackend->sendPartNumber(modelName);
    opcBackend->sendLength(m_length);
    opcBackend->setTopActivation(ui->topViewActiveChk->isChecked());
    opcBackend->setLeftActivation(ui->leftViewActiveChk->isChecked());
    opcBackend->setRightActivation(ui->rightViewActiveChk->isChecked());
    opcBackend->setBottomActivation(ui->bottomViewActiveChk->isChecked());

    halconCore->setModelDirectory(modelDir);
    halconCore->setModelName(modelName);
    halconCore->setCameraAcquringParams(1, m_length);
    halconCore->setCameraAcquringParams(2, m_length);

    // load model data for each side
    halconCore->loadModel("Top");
    halconCore->loadModel("Left");
    halconCore->loadModel("Right");
    halconCore->loadModel("Bottom");

    // fill tables with loaded data
    QMap<QString, HalconCore::MeasSpecView> measSpecViews = halconCore->getMeasSpecViews();
    if (halconCore->topLoadModelStatus())
    {
        // fill top table
        QVector<HalconCore::MeasurementSpec> measurementList = measSpecViews["Top"].measList;
        tableFillReference(measurementList, ui->tableTop);
        ui->topViewActiveChk->setEnabled(true);
    }
    else
    {
        m_topActive = false;
    }

    if (halconCore->leftLoadModelStatus())
    {
        // fill left table
        QVector<HalconCore::MeasurementSpec> measurementList = measSpecViews["Left"].measList;
        tableFillReference(measurementList, ui->tableLeft);
        ui->leftViewActiveChk->setEnabled(true);
    }
    else
    {
        m_leftActive = false;
    }

    if (halconCore->rightLoadModelStatus())
    {
        // fill right table
        QVector<HalconCore::MeasurementSpec> measurementList = measSpecViews["Right"].measList;
        tableFillReference(measurementList, ui->tableRight);
        ui->rightViewActiveChk->setEnabled(true);
    }
    else
    {
        m_rightActive = false;
    }

    if (halconCore->bottomLoadModelStatus())
    {
        // fill bottom table
        QVector<HalconCore::MeasurementSpec> measurementList = measSpecViews["Bottom"].measList;
        tableFillReference(measurementList, ui->tableBottom);
        ui->bottomViewActiveChk->setEnabled(true);
    }
    else
    {
        m_bottomActive = false;
    }

    // decide which view is loaded or bypassed (no inspection is needed)
    if ((halconCore->topLoadModelStatus() || !m_topActive) ||
        (halconCore->leftLoadModelStatus() || !m_leftActive) ||
        (halconCore->rightLoadModelStatus() || !m_rightActive) ||
        (halconCore->bottomLoadModelStatus() || !m_bottomActive))
    {
        m_loadModelStatus = true;
        ui->startBtn->setEnabled(m_systemStatus);
    }
    else
    {
        m_loadModelStatus = false;
        ui->startBtn->setEnabled(false);
    }

    resetViewersAndResultLabels();

    qDebug() << "Number of Activated Scans: " << numActivatedScans;
}

void MainWindow::on_opcBtn_clicked()
{
    connectionsWindow->show();
}


void MainWindow::on_startBtn_clicked()
{
    opcBackend->startInspection();

    startSystem();
}

void MainWindow::startSystem()
{
    ui->startBtn->setEnabled(false);
    ui->stopBtn->setEnabled(true);

    acquireCameraImage();
}

void MainWindow::on_stopBtn_clicked()
{
    opcBackend->stopInspection();

    halconCore->stopCamera1();
    halconCore->stopCamera2();

    if (scanBothTopLeft)
    {
        // reset scanned sides
        scanTopDone = false;
        scanLeftDone = false;
    }

    if (scanBothBottomRight)
    {
        // reset scanned sides
        scanRightDone = false;
        scanBottomDone = false;
    }

    //numInspections = 0;

    // reset scan switching
    //scanBothTopLeft = true;
    //scanBothBottomRight = false;
}

void MainWindow::onAnyStartStopClick()
{
    // enable/disable start and stop buttons
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (btn->text() == "Start")
    {
        ui->startBtn->setEnabled(false);
        ui->stopBtn->setEnabled(true);
    }
    else if (btn->text() == "Stop")
    {
        ui->startBtn->setEnabled(m_systemStatus);  // enable but when system is ready and already model is loaded
        ui->stopBtn->setEnabled(false);
    }
}

void MainWindow::onOpcConnectionChange(bool status)
{
    QString plcConnectionText = status ? "<font color=blue> Connected </font>" : "<font color=red> Disconnected </font>";
    ui->plcConnectionLbl->setText(plcConnectionText);
}

void MainWindow::onMachineStateChange(OpcBackend::MachineState state)
{
    if (state == OpcBackend::MachineState::Scanning)
    {
        ui->machineStateLbl->setText("<font color=blue> Scanning </font>");
    }
    else if (state == OpcBackend::MachineState::Homing)
    {
        ui->machineStateLbl->setText("<font color=yellow> Homing </font>");
    }
    else if (state == OpcBackend::MachineState::Idle)
    {
        ui->machineStateLbl->setText("<font color=green> Idle </font>");
    }
    else if (state == OpcBackend::MachineState::Fault)
    {
        ui->machineStateLbl->setText("<font color=red> Fault </font>");
    }
}

void MainWindow::onSwitchSideSignal()
{
    startSystem();
}

void MainWindow::onCam1ConnectionChange(bool status)
{
    QString cam1ConnStatusText = status ? "<font color=blue> Connected </font>" : "<font color=red> Disconnected </font>";
    ui->camera1ConnectionLbl->setText(cam1ConnStatusText);
}

void MainWindow::onCam2ConnectionChange(bool status)
{
    QString cam2ConnStatusText = status ? "<font color=blue> Connected </font>" : "<font color=red> Disconnected </font>";
    ui->camera2ConnectionLbl->setText(cam2ConnStatusText);
}

void MainWindow::onCam1AcquiringChange(bool status, int scanView)
{
    opcBackend->sendCam1State(status ? 1 : 0);

    QString camStatusText = status ? "<font color=blue> Acquiring </font>" : "<font color=green> Idle </font>";
    ui->camera1StateLbl->setText(camStatusText);

    QString styleSheet = status ? QString("background-color: #ff8c00;") : QString("background-color: rgba(255, 255, 255, 0);");
    if (scanView == 0)
    {
        ui->topViewFrame->setStyleSheet(styleSheet);
    }
    else if (scanView == 1)
    {
        ui->leftViewFrame->setStyleSheet(styleSheet);
    }
    else if (scanView == 2)
    {
        ui->rightViewFrame->setStyleSheet(styleSheet);
    }
    else if (scanView == 3)
    {
        ui->bottomViewFrame->setStyleSheet(styleSheet);
    }
}

void MainWindow::onCam2AcquiringChange(bool status, int scanView)
{
    opcBackend->sendCam2State(status ? 1 : 0);

    QString camStatusText = status ? "<font color=blue> Acquiring </font>" : "<font color=green> Idle </font>";
    ui->camera2StateLbl->setText(camStatusText);

    QString styleSheet = status ? QString("background-color: #ff8c00;") : QString("background-color: rgba(255, 255, 255, 0);");
    if (scanView == 0)
    {
        ui->topViewFrame->setStyleSheet(styleSheet);
    }
    else if (scanView == 1)
    {
        ui->leftViewFrame->setStyleSheet(styleSheet);
    }
    else if (scanView == 2)
    {
        ui->rightViewFrame->setStyleSheet(styleSheet);
    }
    else if (scanView == 3)
    {
        ui->bottomViewFrame->setStyleSheet(styleSheet);
    }
}

void MainWindow::onSystemInitailize(bool status)
{
    m_systemStatus = (opcBackend->connected() &&
                      halconCore->getCamera1ConnectionStatus() &&
                      halconCore->getCamera2ConnectionStatus()) ? true : false;
    QString systemStatusText = m_systemStatus ? "<font color=blue> Ready </font>" : "<font color=red> Fault </font>";
    ui->systemConnectionLbl->setText(systemStatusText);

    opcBackend->sendAppReady(m_systemStatus);
    updateFaultCode();
    opcBackend->sendFaultCode(m_faultCode);

    if (!m_systemStatus)   // if system is not ready (regardless of model is loaded or not)
    {
        ui->startBtn->setEnabled(false);
    }
    else
    {
        if (m_loadModelStatus)  // if model is already loaded fine
        {
            ui->startBtn->setEnabled(true);
        }
        else   // if no model is loaded yet
        {
            ui->startBtn->setEnabled(false);
        }
    }
}

void MainWindow::on_resetBtn_clicked()
{
    opcBackend->resetSystem();

    // reset 4 viewers and result labels
    resetViewersAndResultLabels();
    numInspections = 0;

    // reset scan switching
    scanBothTopLeft = true;
    scanBothBottomRight = false;

    // reset scanned sides
    scanTopDone = false;
    scanLeftDone = false;
    scanRightDone = false;
    scanBottomDone = false;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    opcBackend->disconnectFromEndpoint();
    halconCore->closeAllFrameGrabberByHalcon();
}


void MainWindow::on_reportBtn_clicked()
{

    QString directory = QFileDialog::getExistingDirectory(
        this,
        "Select Directory to save exported CSV files",
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!directory.isEmpty())
    {
        ExportCSV csv;
        csv.setOutputDirectory(directory);

        QMap<QString, HalconCore::MeasSpecView> measSpecViews = halconCore->getMeasSpecViews();
        csv.exportMeasurementToCSV(measSpecViews["Top"]);
        csv.exportMeasurementToCSV(measSpecViews["Left"]);
        csv.exportMeasurementToCSV(measSpecViews["Right"]);
        csv.exportMeasurementToCSV(measSpecViews["Bottom"]);
    }
}

void MainWindow::setResultLabel(bool sideActivation, QLabel *sideViewLabel, QLabel *resultLabel)
{
    // label header
    sideViewLabel->setEnabled(sideActivation);
    // result label
    QString resultLabelText = sideActivation ? "<font color=blue> -- </font>" : "<font color=gray> ByPass </font>";
    resultLabel->setText(resultLabelText);
}

void MainWindow::resetViewersAndResultLabels()
{
    setResultLabel(m_topActive, ui->topViewLbl, ui->topResultLbl);
    setResultLabel(m_leftActive, ui->leftViewLbl, ui->leftResultLbl);
    setResultLabel(m_rightActive, ui->rightViewLbl, ui->rightResultLbl);
    setResultLabel(m_bottomActive, ui->bottomViewLbl, ui->bottomResultLbl);
    ui->finalResultLbl->setText("<font color = blue> -- </font>");

    QString styleSheet = QString("background-color: rgba(255, 255, 255, 0);");
    ui->topViewFrame->setStyleSheet(styleSheet);
    ui->leftViewFrame->setStyleSheet(styleSheet);
    ui->rightViewFrame->setStyleSheet(styleSheet);
    ui->bottomViewFrame->setStyleSheet(styleSheet);

    // reset viewers
    Disp1->GetHalconBuffer()->ClearWindow();
    Disp1->GetHalconBuffer()->FlushBuffer();

    Disp2->GetHalconBuffer()->ClearWindow();
    Disp2->GetHalconBuffer()->FlushBuffer();

    Disp3->GetHalconBuffer()->ClearWindow();
    Disp3->GetHalconBuffer()->FlushBuffer();

    Disp4->GetHalconBuffer()->ClearWindow();
    Disp4->GetHalconBuffer()->FlushBuffer();
}

void MainWindow::on_topViewActiveChk_toggled(bool checked)
{
    m_topActive = checked;
    opcBackend->setTopActivation(m_topActive);

    // reset scan switching
    scanBothTopLeft = true;
    scanBothBottomRight = false;

    setResultLabel(m_topActive, ui->topViewLbl, ui->topResultLbl);
    if (!m_topActive && !m_leftActive && !m_rightActive && !m_bottomActive)
    {
        ui->startBtn->setEnabled(false);
    }
    else
    {
        ui->startBtn->setEnabled(m_systemStatus);
    }
}

void MainWindow::on_leftViewActiveChk_toggled(bool checked)
{
    m_leftActive = checked;
    opcBackend->setLeftActivation(m_leftActive);

    // reset scan switching
    scanBothTopLeft = true;
    scanBothBottomRight = false;

    setResultLabel(m_leftActive, ui->leftViewLbl, ui->leftResultLbl);
    if (!m_topActive && !m_leftActive && !m_rightActive && !m_bottomActive)
    {
        ui->startBtn->setEnabled(false);
    }
    else
    {
        ui->startBtn->setEnabled(m_systemStatus);
    }

}

void MainWindow::on_rightViewActiveChk_toggled(bool checked)
{
    m_rightActive = checked;
    opcBackend->setRightActivation(m_rightActive);

    // reset scan switching
    scanBothTopLeft = true;
    scanBothBottomRight = false;

    setResultLabel(m_rightActive, ui->rightViewLbl, ui->rightResultLbl);
    if (!m_topActive && !m_leftActive && !m_rightActive && !m_bottomActive)
    {
        ui->startBtn->setEnabled(false);
    }
    else
    {
        ui->startBtn->setEnabled(m_systemStatus);
    }
}

void MainWindow::on_bottomViewActiveChk_toggled(bool checked)
{
    m_bottomActive = checked;
    opcBackend->setBottomActivation(m_bottomActive);

    // reset scan switching
    scanBothTopLeft = true;
    scanBothBottomRight = false;

    setResultLabel(m_bottomActive, ui->bottomViewLbl, ui->bottomResultLbl);
    if (!m_topActive && !m_leftActive && !m_rightActive && !m_bottomActive)
    {
        ui->startBtn->setEnabled(false);
    }
    else
    {
        ui->startBtn->setEnabled(m_systemStatus);
    }
}

void MainWindow::onHeartBeat()
{
    if (opcBackend->connected())
    {
        opcBackend->sendAppHeartBeat(true);
    }
    qDebug() << "App heart beat is sent";
}
