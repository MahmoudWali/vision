#ifndef CAMERAPARAMETERS_H
#define CAMERAPARAMETERS_H

#include <QObject>

struct CameraParams {
    int id;
    int xSize;
    int ySize;
    QString deviceIP;
    bool extTrigger;
    int totalImageHeight;
    int timeout;
};


#endif // CAMERAPARAMETERS_H

