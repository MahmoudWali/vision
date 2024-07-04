#include "cameraworkerthread.h"

CameraWorkerThread::CameraWorkerThread(QObject *parent)
    : QThread{parent}
{
    acquire_image = "acquire_image";
}

void CameraWorkerThread::run()
{
    try {
        //qDebug() << "Thread " << QThread::objectName() << " is started";

        //HalconCpp::SetFramegrabberParam(acqHandle, "grab_timeout", params.timeout);
        HalconCpp::GrabImageStart(acqHandle, -1);

        int CurrentImageSize = 0;
        HImage Images;
        HalconCpp::GenEmptyObj(&Images);

        while (CurrentImageSize < params.totalImageHeight)
        {
            HImage ImageTmp;
            HalconCpp::GrabImageAsync(&ImageTmp, acqHandle, -1);
            HalconCpp::ConcatObj(Images, ImageTmp, &Images);
            HTuple Width, SingleImageSize;
            HalconCpp::GetImageSize(ImageTmp, &Width, &SingleImageSize);
            HTuple ImageCount;
            HalconCpp::CountObj(Images, &ImageCount);
            CurrentImageSize = SingleImageSize.I() * ImageCount.I() ;

            if (m_stop)
            {
                break;
            }
        }

        HImage Image;
        HalconCpp::TileImages(Images, &Image, 1, "vertical");

        if (!m_stop)
        {
            emit imageReady(Image, scanView);
        }

        //qDebug() << "Thread " << QThread::objectName() << " is finished";
    }
    catch (HDevEngineException& hdev_exception)
    {
        qDebug() << hdev_exception.Message();
    }
}

bool CameraWorkerThread::stop() const
{
    return m_stop;
}

void CameraWorkerThread::setStop(bool newStop)
{
    m_stop = newStop;
}

int CameraWorkerThread::getScanView() const
{
    return scanView;
}

void CameraWorkerThread::setScanView(int newScanView)
{
    scanView = newScanView;
}

HTuple CameraWorkerThread::getAcqHandle() const
{
    return acqHandle;
}

void CameraWorkerThread::setAcqHandle(const HTuple &newAcqHandle)
{
    acqHandle = newAcqHandle;
}

void CameraWorkerThread::setParams(const CameraParams &newParams)
{
    params = newParams;
}
