#include "trainmodelwindow.h"
#include "ui_trainmodelwindow.h"

TrainModelWindow::TrainModelWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TrainModelWindow)
{
    ui->setupUi(this);
    setWindowTitle("Training Model");

    defaultDXFDirectory = "D:/Data/Images/";
    defualtModelsDirectory = "D:/Data/Models/";

    Disp = new QHalconWindow(this);
    Disp->setMinimumSize(50, 250);
    ui->verticalLayout->addWidget(Disp, 1);

    ui->tableWidget->setColumnCount(6);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setColumnWidth(0, 100);
    ui->tableWidget->setColumnWidth(1, 150);
    ui->tableWidget->setColumnWidth(2, 150);
    ui->tableWidget->setColumnWidth(3, 150);
    ui->tableWidget->setColumnWidth(4, 150);
    ui->tableWidget->setColumnWidth(5, 200);
    ui->tableWidget->setHorizontalHeaderLabels({"SpecID", "SpecValue (mm)", "Tol+ (mm)", "Tol- (mm)", "Meaurement Type", "Notes"});
    ui->tableWidget->setMinimumHeight(150);

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

    ui->tableWidget->setStyleSheet(tableStyleSheet);
    ui->tableWidget->setFont(QFont("Calibri", 12));


    QWidget* containerWidget = new QWidget(this);
    containerWidget->setLayout(ui->verticalLayout);

    QSplitter *splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Vertical);
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(ui->tableWidget);
    splitter->addWidget(containerWidget);

    ui->groupBox_3->setLayout(new QVBoxLayout);
    ui->groupBox_3->layout()->addWidget(splitter);

    // used to allow only numbers for the edit lines
    ui->profileLengthEdit->setValidator(new QIntValidator(0, 100000000, this));

    connect(Disp, &QHalconWindow::mouse_position, this, &TrainModelWindow::onImageClick);

}

TrainModelWindow::~TrainModelWindow()
{
    delete ui;
}

void TrainModelWindow::on_loadModelBtn_clicked()
{
    QString modelDir = QFileDialog::getExistingDirectory(this, "Model Data", "D:\\Data\\Images");

    // read the "json" and "dxf" files from model directory
    QString measJsonFile;
    QString dxfFileName;
    if (!modelDir.isEmpty())
    {
        QDir dir(modelDir);
        QFileInfoList fileNames = dir.entryInfoList(QStringList() << "*.dxf" << "*.json");

        for (auto &f : fileNames)
        {
            if (f.suffix() == "dxf")
            {
                dxfFileName = f.filePath();
            }

            if (f.suffix() == "json")
            {
                measJsonFile = f.filePath();
            }
        }
    }

    if (measJsonFile.isEmpty() || dxfFileName.isEmpty())
        return;

    ui->dxfPathEdit->setText(dxfFileName);
    ui->jsonPathEdit->setText(measJsonFile);

    halconCore->trainModel(dxfFileName, measJsonFile);

    ImageTemplate = halconCore->getImageTemplate();
    ModelContours = halconCore->getModelContour();
    MeaureContours = halconCore->getMeasureContours();
    measurementList = halconCore->getMeasListTrain();

    HTuple num;
    HalconCpp::CountObj(ModelContours, &num);
    numberMetrologyContours = num.I();

    //halconCore->scaleDown(0.1, ImageTemplate, ModelContours, MeaureContours);

    ImageTemplate.GetImageSize(&ImageWidth, &ImageHeight);
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

    Disp->GetHalconBuffer()->DispObj(ImageTemplate);
    Disp->GetHalconBuffer()->SetColor("red");
    Disp->GetHalconBuffer()->SetLineWidth(3);
    Disp->GetHalconBuffer()->DispObj(ModelContours);
    Disp->GetHalconBuffer()->SetLineWidth(1);
    Disp->GetHalconBuffer()->DispObj(MeaureContours);
    Disp->GetHalconBuffer()->FlushBuffer();

    //int contoursCount = ModelContours.CountObj();

    ui->tableWidget->setRowCount(measurementList.size());
    for (int i = 0; i < measurementList.size(); i++)
    {

        QTableWidgetItem *itemSpecId = new QTableWidgetItem(measurementList[i].specId);
        ui->tableWidget->setItem(i, 0, itemSpecId);

        QTableWidgetItem *itemSpecValue = new QTableWidgetItem(QString::number(measurementList[i].specValue));
        ui->tableWidget->setItem(i, 1, itemSpecValue);

        QTableWidgetItem *itemToleranceMax = new QTableWidgetItem(QString::number(measurementList[i].toleranceMax));
        ui->tableWidget->setItem(i, 2, itemToleranceMax);

        QTableWidgetItem *itemToleranceMin = new QTableWidgetItem(QString::number(measurementList[i].toleranceMin));
        ui->tableWidget->setItem(i, 3, itemToleranceMin);

        QTableWidgetItem *itemMeasType = new QTableWidgetItem(measurementList[i].measType);
        ui->tableWidget->setItem(i, 4, itemMeasType);

        QTableWidgetItem *itemNotes = new QTableWidgetItem(measurementList[i].notes);
        ui->tableWidget->setItem(i, 5, itemNotes);
    }
}

void TrainModelWindow::setHalconCore(QSharedPointer<HalconCore> newHalconCore)
{
    halconCore = newHalconCore;
}

void TrainModelWindow::savePLCRecipe(const QString &modelFullPathName)
{
    QJsonObject jsonObject;
    jsonObject["Part Number"] = ui->modelPartNoEdit->text();
    jsonObject["Profile Length"] = ui->profileLengthEdit->text().toInt();
    jsonObject["Top View"] = ui->topChkBox->isChecked();
    jsonObject["Left View"] = ui->leftChkBox->isChecked();
    jsonObject["Right View"] = ui->rightChkBox->isChecked();
    jsonObject["Bottom View"] = ui->bottomChkBox->isChecked();
    QJsonDocument jsonDoc(jsonObject);

    QString plcRecipeFileName = modelFullPathName + "PLC_Recipe.json";
    QFile jsonFile(plcRecipeFileName);
    if (!jsonFile.open(QIODevice::Text | QIODevice::WriteOnly))
    {
        qDebug() << "Failed to open json file";
        return;
    }

    jsonFile.write(jsonDoc.toJson());
    jsonFile.close();
}

void TrainModelWindow::on_saveModelBtn_clicked()
{
    if (ui->dxfPathEdit->text().isEmpty())
    {
        QMessageBox::critical(this, "DXF File", "Please open DXF file to create your model");
        return;
    }

    if (ui->modelPartNoEdit->text().isEmpty())
    {
        QMessageBox::critical(this, "Empty Part Number", "Please enter part number for your model");
        return;
    }

    QString view;
    QAbstractButton *checkedRadioView = ui->buttonGroup->checkedButton();
    if (checkedRadioView)
    {
        view = checkedRadioView->text();
    }

    QString modelDirectory = defualtModelsDirectory + ui->modelPartNoEdit->text();
    QDir modelDir(modelDirectory);
    if (!modelDir.exists())
    {
        modelDir.mkdir(".");
    }

    QString modelFullPathName = defualtModelsDirectory + ui->modelPartNoEdit->text() + "/" + ui->modelPartNoEdit->text()  + "_" + view + "_";
    halconCore->saveTrainModel(modelFullPathName);

    // save PLC Recipe
    QString plcRecipeFileName = defualtModelsDirectory + ui->modelPartNoEdit->text() + "/" + ui->modelPartNoEdit->text()  + "_";
    savePLCRecipe(plcRecipeFileName);

    QMessageBox::information(this, "Save Model", QString("Your model %1 is saved successfuly!").arg(ui->modelPartNoEdit->text()));
}

void TrainModelWindow::on_cancelBtn_clicked()
{
    close();
}


void TrainModelWindow::on_tableWidget_cellClicked(int row, int column)
{
    QString ID = ui->tableWidget->item(row, 0)->text();

    HalconCore::MeasurementSpec measurementSpec = measurementList[row];
    QVector<QVariant> ids = measurementSpec.halconID;
    int objectIndex = 0;
    int objectIndex1 = 0;
    int objectIndex2 = 0;
    if (ids.size() == 1)
    {
        objectIndex = ids[0].toInt();
    }
    else  // handle cases when [edge - to - object] like start_y, start_x
    {
        int numIds = ids.length();  // logically it is equals to 2 as between two objects
        if (numIds == 2)
        {
            QVariant id1 = ids[0];
            QVariant id2 = ids[1];

            if (id1.typeId() == QMetaType::QString)
            {
                QString id_name = id1.toString();
                int halconId = getHalconIdFromName(id_name);
                objectIndex1 = halconId;
                //qDebug() << "Halcon Id1: " << objectIndex1;
            }
            else if (id1.typeId() == QMetaType::Int)
            {
                objectIndex1 = id1.toInt();
                //qDebug() << "Halcon Id1: " << objectIndex1;
            }

            if (id2.typeId() == QMetaType::QString)
            {
                QString id_name = id2.toString();
                int halconId = getHalconIdFromName(id_name);
                objectIndex2 = halconId;
                //qDebug() << "Halcon Id2: " << halconId;
            }
            else if (id2.typeId() == QMetaType::Int)
            {
                objectIndex2 = id2.toInt();
                //qDebug() << "Halcon Id2: " << id2.toInt();
            }
        }
    }


    HalconCpp::HXLD SelectedContour;
    HalconCpp::HXLD DisplayedContours;
    if (ids.size() == 1)
    {
        HalconCpp::SelectObj(ModelContours, &SelectedContour, objectIndex + 1);
        DisplayedContours = SelectedContour;
    }
    else
    {
        HalconCpp::HXLD SelectedContour1, SelectedContour2;
        HalconCpp::SelectObj(ModelContours, &SelectedContour1, objectIndex1 + 1);
        HalconCpp::SelectObj(ModelContours, &SelectedContour2, objectIndex2 + 1);
        HalconCpp::ConcatObj(SelectedContour1, SelectedContour2, &DisplayedContours);

        HalconCpp::HRegion Region1, Region2, RegionUnion, Rectangle;
        HalconCpp::HTuple Row1, Column1, Row2, Column2;
        HalconCpp::GenRegionContourXld(SelectedContour1, &Region1, "filled");
        HalconCpp::GenRegionContourXld(SelectedContour2, &Region2, "filled");
        HalconCpp::Union2(Region1, Region2, &RegionUnion);
        HalconCpp::SmallestRectangle1(RegionUnion, &Row1, &Column1, &Row2, &Column2);
        HalconCpp::GenRectangle1(&Rectangle, Row1, Column1, Row2, Column2);
        HalconCpp::GenContourRegionXld(Rectangle, &SelectedContour, "border");
    }

    // center to display the text
    HalconCpp::HTuple Area, Row, Col, PointOrder;
    HalconCpp::AreaCenterXld(SelectedContour, &Area, &Row, &Col, &PointOrder);

    // the window-part to display
    HalconCpp::HTuple Row1, Col1, Row2, Col2;
    HalconCpp::SmallestRectangle1Xld(SelectedContour, &Row1, &Col1, &Row2, &Col2);

    // Display selected object
    // first we need to set the view to defined area in anyway
    Disp->GetHalconBuffer()->SetPart(Row1, Col1, Row2, Col2);
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

    Disp->GetHalconBuffer()->DispObj(ImageTemplate);
    Disp->GetHalconBuffer()->SetLineWidth(1);
    Disp->GetHalconBuffer()->SetColor("red");
    Disp->GetHalconBuffer()->DispObj(MeaureContours);
    Disp->GetHalconBuffer()->SetColor("blue");
    Disp->GetHalconBuffer()->SetLineWidth(3);
    Disp->GetHalconBuffer()->DispObj(DisplayedContours);
    Disp->GetHalconBuffer()->DispText(HTuple(ID.toStdString().c_str()), HTuple("image"), Row, Col, HTuple(), HTuple(), HTuple());
    Disp->GetHalconBuffer()->FlushBuffer();
}

void TrainModelWindow::onImageClick(QPointF pos, double lastRow1, double lastCol1, double lastRow2, double lastCol2)
{
    double scalex = (lastCol2 - lastCol1 + 1) / (double)(Disp->width());    //ImageWidth
    double scaley = (lastRow2 - lastRow1 + 1) / (double)(Disp->height());   //ImageHeight
    double row = lastRow1 + (pos.y() * scaley);
    double col = lastCol1 + (pos.x() * scalex);
    //qDebug() << "Image click: " << lastRow1 + (pos.y() * scaley) << ", " << lastCol1 + (pos.x() * scalex);

    HObject xld;
    HalconCpp::SelectXldPoint(ModelContours, &xld, HTuple(row), HTuple(col));
    if (xld.CountObj() == 1)
    {
        Disp->GetHalconBuffer()->DispObj(ImageTemplate);
        Disp->GetHalconBuffer()->SetColor("red");
        Disp->GetHalconBuffer()->SetLineWidth(3);
        Disp->GetHalconBuffer()->DispObj(ModelContours);
        Disp->GetHalconBuffer()->SetLineWidth(1);
        Disp->GetHalconBuffer()->DispObj(MeaureContours);
        Disp->GetHalconBuffer()->SetColor("blue");
        Disp->GetHalconBuffer()->SetLineWidth(5);
        Disp->GetHalconBuffer()->DispObj(xld);
        Disp->GetHalconBuffer()->FlushBuffer();

        HTuple SelectedArea, SelectedCenterRow, SelectedCenterCol, SelectedPointOrder;
        HalconCpp::AreaCenterXld(xld, &SelectedArea, &SelectedCenterRow, &SelectedCenterCol, &SelectedPointOrder);

        int countObjects = ModelContours.CountObj();
        std::vector<double> distances;
        for (int i = 0; i < countObjects; i++)
        {
            HXLD selectedXld;
            HalconCpp::SelectObj(ModelContours, &selectedXld, HTuple(i+1));
            HTuple Area, CenterRow, CenterCol, PointOrder;
            HalconCpp::AreaCenterXld(selectedXld, &Area, &CenterRow, &CenterCol, &PointOrder);
            HTuple distance;
            HalconCpp::DistancePp(SelectedCenterRow, SelectedCenterCol, CenterRow, CenterCol, &distance);
            distances.push_back(distance.D());
        }

        auto minDistance = std::min_element(distances.begin(), distances.end());
        int index = std::distance(distances.begin(), minDistance);
        if (index > -1)
        {
            ui->tableWidget->selectRow(index);
        }
    }

}

int TrainModelWindow::getHalconIdFromName(QString id_name)
{
    if (id_name == "start_x")
    {
        return (numberMetrologyContours - 4);
    }
    else if (id_name == "end_x")
    {
        return (numberMetrologyContours - 3);
    }
    else if (id_name == "start_y")
    {
        return (numberMetrologyContours - 2);
    }
    else if (id_name == "end_y")
    {
        return (numberMetrologyContours - 1);
    }
}


