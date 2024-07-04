#include "halconcore.h"

HalconCore::HalconCore(QObject *parent)
    : QObject{parent}
{

    QString projectDir = QString(DIRECTORY_PATH);
    QDir dir(projectDir);
    dir.cdUp();
    dir.cdUp();
    QString proceduresPath = dir.path() + "/Halcon/HDevProcedures";
    hDevEngine.SetProcedurePath(proceduresPath.toStdString().c_str());

    open_keyence_LJXA = "open_keyence_LJXA";
    close_keyence_LJXA = "close_keyence_LJXA";
    dxf2imageProc = "dxf2image";
    create_metrology_from_dxf = "create_metrology_from_dxf";
    gen_cam_par_area_scan_division = "gen_cam_par_area_scan_division";
    create_alignment_model = "create_alignment_model";
    parse_measurement_spec = "parse_measurement_spec";
    perform_measurement = "perform_measurement";

    camWorkerThread1 = new CameraWorkerThread(this);
    camWorkerThread1->setObjectName("Cam1 Thread");
    QObject::connect(camWorkerThread1, &CameraWorkerThread::imageReady, this, &HalconCore::handleGrabbedImage1);
    QObject::connect(camWorkerThread1, &CameraWorkerThread::finished, this, &HalconCore::onThreadFinished);

    camWorkerThread2 = new CameraWorkerThread(this);
    camWorkerThread2->setObjectName("Cam2 Thread");
    QObject::connect(camWorkerThread2, &CameraWorkerThread::imageReady, this, &HalconCore::handleGrabbedImage2);
    QObject::connect(camWorkerThread2, &CameraWorkerThread::finished, this, &HalconCore::onThreadFinished);
}

void HalconCore::loadImageHDevEngine()
{
    Image1 = hDevProgramCall.GetIconicVarObject("ImageScaled");
    Image2 = hDevProgramCall.GetIconicVarObject("ImageScaled");
    Image3 = hDevProgramCall.GetIconicVarObject("ImageScaled");
    Image4 = hDevProgramCall.GetIconicVarObject("ImageScaled");
}

void HalconCore::openFrameGrabber(const CameraParams& params)
{
    try {
        HDevProcedure hDevProcedure(open_keyence_LJXA.c_str());
        HDevProcedureCall hDevProcedureCall(hDevProcedure);
        hDevProcedureCall.SetInputCtrlParamTuple("ImageWidth", params.xSize);
        hDevProcedureCall.SetInputCtrlParamTuple("ImageHeight", params.ySize);
        hDevProcedureCall.SetInputCtrlParamTuple("Device", params.deviceIP.toStdString().c_str());

        hDevProcedureCall.Execute();
        if (params.id == 1)
        {
            acqHandle1 = hDevProcedureCall.GetOutputCtrlParamTuple("AcqHandle");
            m_camera1ConnectionStatus = true;
            emit camera1ConnectionStatus(m_camera1ConnectionStatus);
        }
        else if (params.id == 2)
        {
            acqHandle2 = hDevProcedureCall.GetOutputCtrlParamTuple("AcqHandle");
            m_camera2ConnectionStatus = true;
            emit camera2ConnectionStatus(m_camera2ConnectionStatus);
        }
    }
    catch (HDevEngineException& hdev_exception)
    {
        if (params.id == 1)
        {
            m_camera1ConnectionStatus = false;
            emit camera1ConnectionStatus(m_camera1ConnectionStatus);
        }
        else if (params.id == 2)
        {
            m_camera2ConnectionStatus = false;
            emit camera2ConnectionStatus(m_camera2ConnectionStatus);
        }

        qDebug() << hdev_exception.Message();
    }
}

void HalconCore::connectCamera1()
{
    if (!m_camera1ConnectionStatus)
    {
        openFrameGrabber(m_camera1Params);
    }
}

void HalconCore::connectCamera2()
{
    if (!m_camera2ConnectionStatus)
    {
        openFrameGrabber(m_camera2Params);
    }
}

void HalconCore::disConnectCamera1()
{
    if (m_camera1ConnectionStatus)
    {
        closeFrameGrabber(1);
    }
}

void HalconCore::disConnectCamera2()
{
    if (m_camera2ConnectionStatus)
    {
        closeFrameGrabber(2);
    }
}

void HalconCore::acquireCamera1(int scanView)
{
    camWorkerThread1->setStop(false);
    camWorkerThread1->setParams(m_camera1Params);
    camWorkerThread1->setScanView(scanView);
    camWorkerThread1->setAcqHandle(acqHandle1);
    camWorkerThread1->start();
    emit camera1AcquiringStatus(true, scanView);  // used for frontend Device Status Display
}

void HalconCore::acquireCamera2(int scanView)
{
    camWorkerThread2->setStop(false);
    camWorkerThread2->setParams(m_camera2Params);
    camWorkerThread2->setScanView(scanView);
    camWorkerThread2->setAcqHandle(acqHandle2);
    camWorkerThread2->start();
    emit camera2AcquiringStatus(true, scanView);  // used for frontend Device Status Display
}

void HalconCore::stopCamera1()
{
    camWorkerThread1->setStop(true);
    emit camera1AcquiringStatus(false, 0);  // used for frontend Device Status Display
    emit camera1AcquiringStatus(false, 1);  // used for frontend Device Status Display
}

void HalconCore::stopCamera2()
{
    camWorkerThread2->setStop(true);
    emit camera2AcquiringStatus(false, 2);  // used for frontend Device Status Display
    emit camera2AcquiringStatus(false, 3);  // used for frontend Device Status Display
}

void HalconCore::closeFrameGrabber(int cameraId)
{
    try {
        HDevProcedure hDevProcedure(close_keyence_LJXA.c_str());
        HDevProcedureCall hDevProcedureCall(hDevProcedure);
        if (cameraId == 1)
        {
            hDevProcedureCall.SetInputCtrlParamTuple("AcqHandle", acqHandle1);
            m_camera1ConnectionStatus = false;
            emit camera1ConnectionStatus(m_camera1ConnectionStatus);
        }
        else if (cameraId == 2)
        {
            hDevProcedureCall.SetInputCtrlParamTuple("AcqHandle", acqHandle2);
            m_camera2ConnectionStatus = false;
            emit camera2ConnectionStatus(m_camera2ConnectionStatus);
        }

        hDevProcedureCall.Execute();
    }
    catch (HDevEngineException& hdev_exception)
    {
        qDebug() << hdev_exception.Message();
    }
}

void HalconCore::closeAllFrameGrabber()
{
    try {
        HTuple acqHandles;
        acqHandles.Append(acqHandle1).Append(acqHandle2);
        HDevProcedure hDevProcedure(close_keyence_LJXA.c_str());
        HDevProcedureCall hDevProcedureCall(hDevProcedure);
        hDevProcedureCall.SetInputCtrlParamTuple("AcqHandle", acqHandles);
        hDevProcedureCall.Execute();

        m_camera1ConnectionStatus = false;
        emit camera1ConnectionStatus(m_camera1ConnectionStatus);

        m_camera2ConnectionStatus = false;
        emit camera2ConnectionStatus(m_camera2ConnectionStatus);
    }
    catch (HDevEngineException& hdev_exception)
    {
        qDebug() << hdev_exception.Message();
    }
}

void HalconCore::closeAllFrameGrabberByHalcon()
{
    HalconCpp::CloseAllFramegrabbers();
}

void HalconCore::createMetrologyFromDxf(QString dxfFileName)
{
    try {

        generateCameraCalibrationParams();
        generateMeaurementPlaneParams();

        HTuple scale {1000};

        HDevProcedure hDevProcedure(create_metrology_from_dxf.c_str());
        HDevProcedureCall hDevProcedureCall(hDevProcedure);
        hDevProcedureCall.SetInputCtrlParamTuple("DxfPath", dxfFileName.toStdString().c_str());
        hDevProcedureCall.SetInputCtrlParamTuple("CameraParam", cameraCalibrationParams);
        hDevProcedureCall.SetInputCtrlParamTuple("MeasurementPlane", measurementPlaneParams);
        hDevProcedureCall.SetInputCtrlParamTuple("Scale", scale);
        hDevProcedureCall.Execute();
        metrologyHandle = hDevProcedureCall.GetOutputCtrlParamTuple("MetrologyHandle");
        modelContour = hDevProcedureCall.GetOutputIconicParamObject("ModelContour");
        measureContours = hDevProcedureCall.GetOutputIconicParamObject("MeasureContour");
        imageTemplate = hDevProcedureCall.GetOutputIconicParamObject("ImageTemplate");

        qDebug() << "createMetrologyFromDxf is done...";
    }
    catch (HDevEngineException &hdev_exception)
    {
        qDebug() << hdev_exception.Message();
    }
}

void HalconCore::generateCameraCalibrationParams()
{
    try {
        //0.0128649, -661.434, 5.30004e-006, 5.3e-006, 620.043, 497.402
        HTuple Focus(0.0128649);
        HTuple Kappa(0.0128649);
        HTuple Sx(0.0128649);
        HTuple Sy(0.0128649);
        HTuple Cx(0.0128649);
        HTuple Cy(0.0128649);
        HTuple ImageWidth(7995);
        HTuple ImageHeight(3200);

        HDevProcedure hDevProcedure(gen_cam_par_area_scan_division.c_str());
        HDevProcedureCall hDevProcedureCall(hDevProcedure);
        hDevProcedureCall.SetInputCtrlParamTuple("Focus", Focus);
        hDevProcedureCall.SetInputCtrlParamTuple("Kappa", Kappa);
        hDevProcedureCall.SetInputCtrlParamTuple("Sx", Sx);
        hDevProcedureCall.SetInputCtrlParamTuple("Sy", Sy);
        hDevProcedureCall.SetInputCtrlParamTuple("Cx", Cx);
        hDevProcedureCall.SetInputCtrlParamTuple("Cy", Cy);
        hDevProcedureCall.SetInputCtrlParamTuple("ImageWidth", 7995);
        hDevProcedureCall.SetInputCtrlParamTuple("ImageHeight", 3200);
        hDevProcedureCall.Execute();
        cameraCalibrationParams = hDevProcedureCall.GetOutputCtrlParamTuple("CameraParam");
    }
    catch (HDevEngineException &hdev_exception)
    {
        qDebug() << hdev_exception.Message();
    }

}

void HalconCore::generateMeaurementPlaneParams()
{
    // MeasurementPlane := [0.00940956, -0.00481017, 0.29128, 0.478648, 359.65, 0.785, 0]
    measurementPlaneParams.Append(0.00940956).Append(-0.00481017).Append(0.29128).Append(0.478648).Append(359.65).Append(0.785).Append(0);
}

void HalconCore::saveTrainModel(QString modelFullPathName)
{

    QString metrologyModelName = modelFullPathName  + "metrology.mtr";
    HalconCpp::WriteMetrologyModel(metrologyHandle, metrologyModelName.toStdString().c_str());

    QString shapeModelName = modelFullPathName  + "shapemodel.shm";
    HalconCpp::WriteShapeModel(modelId, shapeModelName.toStdString().c_str());

    QString refMeasValueName = modelFullPathName + "refMeasDict";
    HalconCpp::WriteDict(measurementSpecDict, refMeasValueName.toStdString().c_str(), HTuple(), HTuple());
}

void HalconCore::loadModel(const QString &view)
{    
    if (m_modelDirectory.isEmpty() || m_modelName.isEmpty())
    {
        return;
    }

    HalconCore::MeasSpecView measSpecView;

    QString model_view = m_modelDirectory + "/" + m_modelName + "_" + view;

    // load meterlogy model
    bool loadMetrolgyStatus = false;
    QString metrologyModelName = model_view + "_metrology.mtr";
    if (QFile::exists(metrologyModelName))
    {
        HTuple metrologyModelName_T(metrologyModelName.toStdString().c_str());
        HalconCpp::ReadMetrologyModel(metrologyModelName_T, &metrologyHandle);
        measSpecView.metrologyHandle = metrologyHandle;
        loadMetrolgyStatus = true;
    }

    // load shape model
    bool loadShapeModelStatus = false;
    QString shapeModelName = model_view + "_shapemodel.shm";
    if (QFile::exists(shapeModelName))
    {
        HTuple shapeModelName_T(shapeModelName.toStdString().c_str());
        HalconCpp::ReadShapeModel(shapeModelName_T, &modelId);
        measSpecView.modelId = modelId;
        loadShapeModelStatus = true;
    }

    // load reference values vector
    bool loadRefMeasStatus = false;
    QString refMeasValueName = model_view + "_refMeasDict.hdict";
    if (QFile::exists(refMeasValueName))
    {
        HTuple refMeasValueName_T(refMeasValueName.toStdString().c_str());
        HalconCpp::ReadDict(refMeasValueName_T, HTuple(), HTuple(), &measurementSpecDict);
        measSpecView.measList = extractMeasurementFromDict(measurementSpecDict);
        measSpecView.measSpec = measurementSpecDict;
        loadRefMeasStatus = true;
    }

    measSpecView.partNumber = m_modelName;
    measSpecView.view = view;

    // add new key by "view" to map
    measSpecViews[view] = measSpecView;

    if (loadMetrolgyStatus && loadShapeModelStatus && loadRefMeasStatus)
    {
        if (view == "Top")
        {
            m_topLoadModelStatus = true;
        }
        else if (view == "Left")
        {
            m_leftLoadModelStatus = true;
        }
        else if (view == "Right")
        {
            m_rightLoadModelStatus = true;
        }
        else if (view == "Bottom")
        {
            m_bottomLoadModelStatus = true;
        }
    }
}

void HalconCore::createAlignmentModel()
{
    try {
        qDebug() << "createAlignmentModel is started...";

        HTuple ROI;
        ROI.Append(-657.795).Append(-1015.81).Append(5098.8).Append(13289.3);

        HDevProcedure proc(create_alignment_model.c_str());
        HDevProcedureCall procCall(proc);
        procCall.SetInputIconicParamObject("ImageTemplateMetrology", imageTemplate);
        procCall.SetInputCtrlParamTuple("RectAlignModel", ROI);
        procCall.SetInputCtrlParamTuple("MetrologyModel", metrologyHandle);
        procCall.Execute();
        modelId = procCall.GetOutputCtrlParamTuple("ModelID");

        qDebug() << "createAlignmentModel is Done...";

    }
    catch (HDevEngineException &hdev_exception)
    {
        qDebug() << hdev_exception.Message();
    }
}

void HalconCore::trainModel(const QString &dxfFileName, const QString &measJsonFile)
{
    parseMeasurementSpec(measJsonFile);
    createMetrologyFromDxf(dxfFileName);
    createAlignmentModel();
}

QVector<HalconCore::MeasurementSpec> HalconCore::extractMeasurementFromDict(const HTuple &measurementSpecDict)
{
    HTuple keys;
    HalconCpp::GetDictParam(measurementSpecDict, "keys", HTuple(), &keys);

    QVector<HalconCore::MeasurementSpec> measList;
    for (int i = 0; i < keys.Length(); i++)
    {
        HTuple specId, spectValue, toleranceMin, toleranceMax, measType, halconId, notes;
        HTuple measDict;
        HalconCpp::GetDictTuple(measurementSpecDict, keys[i], &measDict);

        HalconCpp::GetDictTuple(measDict, "SpecID", &specId);
        HalconCpp::GetDictTuple(measDict, "SpecValue", &spectValue);
        HalconCpp::GetDictTuple(measDict, "Tol+", &toleranceMax);
        HalconCpp::GetDictTuple(measDict, "Tol-", &toleranceMin);
        HalconCpp::GetDictTuple(measDict, "MeasType", &measType);
        HalconCpp::GetDictTuple(measDict, "HalconID", &halconId);
        HalconCpp::GetDictTuple(measDict, "Notes", &notes);

        HalconCore::MeasurementSpec measSpec;
        measSpec.specId = specId.S();
        measSpec.specValue = spectValue.D();
        measSpec.toleranceMax = toleranceMax.D();
        measSpec.toleranceMin = toleranceMin.D();
        measSpec.measType = measType.S();
        for (int j = 0; j < halconId.Length(); j++)
        {
            if (halconId[j].Type() == STRING_PAR)
            {
                QString halconIdStr = halconId[j].S().Text();
                measSpec.halconID.push_back(QVariant(halconIdStr));
            }
            else if (halconId[j].Type() == INT_PAR)
            {
                int halconIdInt = halconId[j].I();
                measSpec.halconID.push_back(QVariant(halconIdInt));
            }
        }
        measSpec.notes = notes.S();

        measList.push_back(measSpec);
    }

    return measList;
}

void HalconCore::parseMeasurementSpec(const QString &jsonFilePath)
{
    try {
        HDevProcedure proc(parse_measurement_spec.c_str());
        HDevProcedureCall procCall(proc);
        procCall.SetInputCtrlParamTuple("SpecificationFile", jsonFilePath.toStdString().c_str());
        procCall.Execute();
        measurementSpecDict = procCall.GetOutputCtrlParamTuple("MeasurementSpec");

        HTuple keys;
        HalconCpp::GetDictParam(measurementSpecDict, "keys", HTuple(), &keys);

        measListTrain.clear();
        for (int i = 0; i < keys.Length(); i++)
        {
            HTuple specId, spectValue, toleranceMin, toleranceMax, measType, halconId, notes;
            HTuple measDict;
            HalconCpp::GetDictTuple(measurementSpecDict, keys[i], &measDict);

            HalconCpp::GetDictTuple(measDict, "SpecID", &specId);
            HalconCpp::GetDictTuple(measDict, "SpecValue", &spectValue);
            HalconCpp::GetDictTuple(measDict, "Tol+", &toleranceMax);
            HalconCpp::GetDictTuple(measDict, "Tol-", &toleranceMin);
            HalconCpp::GetDictTuple(measDict, "MeasType", &measType);
            HalconCpp::GetDictTuple(measDict, "HalconID", &halconId);
            HalconCpp::GetDictTuple(measDict, "Notes", &notes);

            HalconCore::MeasurementSpec measSpec;
            measSpec.specId = specId.S();
            measSpec.specValue = spectValue.D();
            measSpec.toleranceMax = toleranceMax.D();
            measSpec.toleranceMin = toleranceMin.D();
            measSpec.measType = measType.S();
            for (int j = 0; j < halconId.Length(); j++)
            {
                if (halconId[j].Type() == STRING_PAR)
                {
                    QString halconIdStr = halconId[j].S().Text();
                    measSpec.halconID.push_back(QVariant(halconIdStr));
                }
                else if (halconId[j].Type() == INT_PAR)
                {
                    int halconIdInt = halconId[j].I();
                    measSpec.halconID.push_back(QVariant(halconIdInt));
                }
            }
            measSpec.notes = notes.S();

            measListTrain.push_back(measSpec);
        }
    }
    catch (HDevEngineException &hdev_exception)
    {
        qDebug() << hdev_exception.Message();
    }
}

void HalconCore::performMeasurement(QString view)
{
    try {
        // prepare input based on view
        HTuple rectAlignModel;
        rectAlignModel.Append(-657.795).Append(-1015.81).Append(5098.8).Append(13289.3);
        HTuple partNumber(m_modelName.toStdString().c_str());
        HTuple viewSide(view.toStdString().c_str());
        HImage image;
        HTuple metrologyId, shapeModelId, measSpecDict;
        if (view == "Top")
        {
            metrologyId = measSpecViews["Top"].metrologyHandle;
            shapeModelId = measSpecViews["Top"].modelId;
            measSpecDict = measSpecViews["Top"].measSpec;
            image = Image1;
        }
        else if (view == "Left")
        {
            metrologyId = measSpecViews["Left"].metrologyHandle;
            shapeModelId = measSpecViews["Left"].modelId;
            measSpecDict = measSpecViews["Left"].measSpec;
            image = Image2;
        }
        else if (view == "Right")
        {
            metrologyId = measSpecViews["Right"].metrologyHandle;
            shapeModelId = measSpecViews["Right"].modelId;
            measSpecDict = measSpecViews["Right"].measSpec;
            image = Image3;
        }
        else if (view == "Bottom")
        {
            metrologyId = measSpecViews["Bottom"].metrologyHandle;
            shapeModelId = measSpecViews["Bottom"].modelId;
            measSpecDict = measSpecViews["Bottom"].measSpec;
            image = Image4;
        }

        // just for test:
        //image.ReadImage("D:/Data/Images/0400-0160-top.png");

        // call Hdev proc
        HDevProcedure proc(perform_measurement.c_str());
        HDevProcedureCall procCall(proc);
        procCall.SetInputIconicParamObject("Image", image);
        procCall.SetInputCtrlParamTuple("MetrologyModel", metrologyId);
        procCall.SetInputCtrlParamTuple("AlignmentModel", shapeModelId);
        procCall.SetInputCtrlParamTuple("RectAlignModel", rectAlignModel);
        procCall.SetInputCtrlParamTuple("MeasurementSpec", measSpecDict);
        procCall.SetInputCtrlParamTuple("PartNumber", partNumber);
        procCall.SetInputCtrlParamTuple("View", viewSide);
        procCall.Execute();

        HTuple keys;
        HalconCpp::GetDictParam(measSpecDict, "keys", HTuple(), &keys);

        for (int i = 0; i < keys.Length(); i++)
        {
            HTuple measDict;
            HalconCpp::GetDictTuple(measSpecDict, keys[i], &measDict);

            HTuple measValue, status;
            HalconCpp::GetDictTuple(measDict, "MeasValue", &measValue);
            HalconCpp::GetDictTuple(measDict, "Status", &status);

            if (view == "Top")
            {
                measSpecViews["Top"].measList[i].measValue = 100; //measValue.D();
                measSpecViews["Top"].measList[i].difference = abs(measSpecViews["Top"].measList[i].measValue - measSpecViews["Top"].measList[i].specValue);
                measSpecViews["Top"].measList[i].status = "OK"; /*QString::fromStdString(status.S().Text())*/;
            }
            else if (view == "Left")
            {
                measSpecViews["Left"].measList[i].measValue = 100; //measValue.D();
                measSpecViews["Left"].measList[i].difference = abs(measSpecViews["Left"].measList[i].measValue - measSpecViews["Left"].measList[i].specValue);
                measSpecViews["Left"].measList[i].status = "NOK"; /*QString::fromStdString(status.S().Text())*/;
            }
            else if (view == "Right")
            {
                measSpecViews["Right"].measList[i].measValue = 100; //measValue.D();
                measSpecViews["Right"].measList[i].difference = abs(measSpecViews["Right"].measList[i].measValue - measSpecViews["Right"].measList[i].specValue);
                measSpecViews["Right"].measList[i].status = QString::fromStdString(status.S().Text());
            }
            else if (view == "Bottom")
            {
                measSpecViews["Bottom"].measList[i].measValue = 100; //measValue.D();
                measSpecViews["Bottom"].measList[i].difference = abs(measSpecViews["Bottom"].measList[i].measValue - measSpecViews["Bottom"].measList[i].specValue);
                measSpecViews["Bottom"].measList[i].status = QString::fromStdString(status.S().Text());
            }
        }

        // final status for each view
        // top
        for (auto &measSpec : measSpecViews["Top"].measList)
        {
            if (measSpec.status == "NOK")
            {
                measSpecViews["Top"].inspectionStatus = "Failed";
                measSpecViews["Top"].result = false;
                break;
            }
            else if (measSpec.status == "OK")
            {
                measSpecViews["Top"].inspectionStatus = "Passed";
                measSpecViews["Top"].result = true;
            }
            else
            {
                measSpecViews["Top"].inspectionStatus = "N/D";
                measSpecViews["Top"].result = false;
            }
        }

        // left
        measSpecViews["Left"].inspectionStatus = "Passed";
        for (auto &measSpec : measSpecViews["Left"].measList)
        {
            if (measSpec.status == "NOK")
            {
                measSpecViews["Left"].inspectionStatus = "Failed";
                measSpecViews["Left"].result = false;
                break;
            }
            else if (measSpec.status == "OK")
            {
                measSpecViews["Left"].inspectionStatus = "Passed";
                measSpecViews["Left"].result = true;
            }
            else
            {
                measSpecViews["Left"].inspectionStatus = "N/D";
                measSpecViews["Left"].result = false;
            }
        }

        // right
        measSpecViews["Right"].inspectionStatus = "Passed";
        for (auto &measSpec : measSpecViews["Right"].measList)
        {
            if (measSpec.status == "NOK")
            {
                measSpecViews["Right"].inspectionStatus = "Failed";
                measSpecViews["Right"].result = false;
                break;
            }
            else if (measSpec.status == "OK")
            {
                measSpecViews["Right"].inspectionStatus = "Passed";
                measSpecViews["Right"].result = true;
            }
            else
            {
                measSpecViews["Right"].inspectionStatus = "N/D";
                measSpecViews["Right"].result = false;
            }
        }

        // bottom
        measSpecViews["Bottom"].inspectionStatus = "Passed";
        for (auto &measSpec : measSpecViews["Bottom"].measList)
        {
            if (measSpec.status == "NOK")
            {
                measSpecViews["Bottom"].inspectionStatus = "Failed";
                measSpecViews["Bottom"].result = false;
                break;
            }
            else if (measSpec.status == "OK")
            {
                measSpecViews["Bottom"].inspectionStatus = "Passed";
                measSpecViews["Bottom"].result = true;
            }
            else
            {
                measSpecViews["Bottom"].inspectionStatus = "N/D";
                measSpecViews["Bottom"].result = false;
            }
        }
    }
    catch (HDevEngineException &hdev_exception)
    {
        qDebug() << hdev_exception.Message();
    }
}

void HalconCore::scaleDown(double scale, HImage &image, HXLD &modelContour, HXLD &measContour)
{
    HTuple width, height;
    HTuple HomMat2DIdentity;
    HTuple HomMat2DScale;
    HTuple HomMat2DTranslate;
    HRegion Domain;

    HalconCpp::GetImageSize(image, &width, &height);
    HalconCpp::GetDomain(image, &Domain);
    HalconCpp::HomMat2dIdentity(&HomMat2DIdentity);
    HalconCpp::HomMat2dScale(HomMat2DIdentity, scale, scale, height/2.0, width/2.0, &HomMat2DScale);
    HalconCpp::HomMat2dTranslate(HomMat2DScale, -((height/2.0) - ((height * scale) / 2.0)) , -((width/2.0) - ((width * scale) / 2.0)), &HomMat2DTranslate);
    HalconCpp::AffineTransImage(image, &image, HomMat2DTranslate, "constant", "false");
    HalconCpp::AffineTransContourXld(modelContour, &modelContour, HomMat2DTranslate);
    HalconCpp::AffineTransContourXld(measContour, &measContour, HomMat2DTranslate);
    HalconCpp::AffineTransRegion(Domain, &Domain, HomMat2DTranslate, "nearest_neighbor");
    HalconCpp::ReduceDomain(image, Domain, &image);
    HalconCpp::CropDomain(image, &image);
}

HImage HalconCore::getImage1() const
{
    return Image1;
}

HImage HalconCore::getImage2() const
{
    return Image2;
}

HImage HalconCore::getImage3() const
{
    return Image3;
}

HImage HalconCore::getImage4() const
{
    return Image4;
}

void HalconCore::setCameraConnection(ConnectionConfig::CameraConnection cameraConnection)
{
    if (cameraConnection.cameraId == 1)
    {
        m_camera1Params.id = cameraConnection.cameraId;
        m_camera1Params.deviceIP = cameraConnection.ip;
        m_camera1Params.extTrigger = cameraConnection.externalTrigger;
        m_camera1Params.xSize = cameraConnection.xsize;
        m_camera1Params.ySize = cameraConnection.ysize;
        m_camera1Params.timeout = cameraConnection.timeout;
    }
    else if (cameraConnection.cameraId == 2)
    {
        m_camera2Params.id = cameraConnection.cameraId;
        m_camera2Params.deviceIP = cameraConnection.ip;
        m_camera2Params.extTrigger = cameraConnection.externalTrigger;
        m_camera2Params.xSize = cameraConnection.xsize;
        m_camera2Params.ySize = cameraConnection.ysize;
        m_camera2Params.timeout = cameraConnection.timeout;
    }
}

void HalconCore::setCameraAcquringParams(int cameraId, int totalImgeHeight)
{
    if (cameraId == 1)
    {
        m_camera1Params.totalImageHeight = totalImgeHeight;
    }
    else if (cameraId == 2)
    {
        m_camera2Params.totalImageHeight = totalImgeHeight;
    }
}

void HalconCore::handleGrabbedImage1(const HObject &image, int scanView)
{
    qDebug() << "Slot Cam1 is triggered";

    emit camera1AcquiringStatus(false, scanView);  // used for frontend Device Status Display

    if (scanView == 0)
    {
        Image1 = image;
    }
    else if (scanView == 3)
    {
        Image4 = image;
    }

    emit processingReady(scanView);
}

void HalconCore::handleGrabbedImage2(const HObject &image, int scanView)
{
    qDebug() << "Slot Cam2 is triggered";

    emit camera2AcquiringStatus(false, scanView);    // used for frontend Device Status Display

    if (scanView == 1)
    {
        Image2 = image;
    }
    else if (scanView == 2)
    {
        Image3 = image;
    }

    emit processingReady(scanView);
}

void HalconCore::onThreadFinished()
{
    //
}

void HalconCore::setModelName(const QString &newModelName)
{
    m_modelName = newModelName;
}

QString HalconCore::modelDirectory() const
{
    return m_modelDirectory;
}

void HalconCore::setModelDirectory(const QString &newModelDirectory)
{
    m_modelDirectory = newModelDirectory;
}

bool HalconCore::bottomLoadModelStatus() const
{
    return m_bottomLoadModelStatus;
}

bool HalconCore::rightLoadModelStatus() const
{
    return m_rightLoadModelStatus;
}

bool HalconCore::leftLoadModelStatus() const
{
    return m_leftLoadModelStatus;
}

bool HalconCore::topLoadModelStatus() const
{
    return m_topLoadModelStatus;
}

bool HalconCore::getCamera1ConnectionStatus() const
{
    return m_camera1ConnectionStatus;
}

bool HalconCore::getCamera2ConnectionStatus() const
{
    return m_camera2ConnectionStatus;
}

QString HalconCore::getModelName() const
{
    return m_modelName;
}

QVector<HalconCore::MeasurementSpec> HalconCore::getMeasListTrain() const
{
    return measListTrain;
}

QMap<QString, HalconCore::MeasSpecView> HalconCore::getMeasSpecViews() const
{
    return measSpecViews;
}


HObject HalconCore::getMeasureContours() const
{
    return measureContours;
}

HObject HalconCore::getModelContour() const
{
    return modelContour;
}

HObject HalconCore::getImageTemplate() const
{
    return imageTemplate;
}

bool HalconCore::MeasSpecView::getResult() const
{
    return result;
}
