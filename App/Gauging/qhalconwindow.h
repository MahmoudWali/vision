// HALCON/Qt pattern matching and measure example
//
// (c) 2004-2023 MVTec Software GmbH

// QHalconWindow.h : Class used for opening HALCON windows in Qt
//
#ifndef QHALCONWINDOW_H
#define QHALCONWINDOW_H

#include <QPainter>
#include <QScopedPointer>
#include <QWidget>


#include "HalconCpp.h"

using namespace HalconCpp;

class QHalconWindow : public QWidget
{
    Q_OBJECT

public:
    QHalconWindow(QWidget* parent = 0, long Width = 0, long Height = 0);

    HalconCpp::HWindow* GetHalconBuffer(void)
    {
        return halconBuffer.data();
    }

    void enableShowFullImageSize();
    void disableShowFullImageSize();
    void setImageSize(int w, int h);

protected:
    void resizeEvent(QResizeEvent*);
    void paintEvent(QPaintEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);

signals:
    void mouse_position(QPointF pos, double lastRow1, double lastCol1, double lastRow2, double lastCol2);

private:
    void GetPartFloat(double* row1, double* col1, double* row2, double* col2);
    void SetPartFloat(double row1, double col1, double row2, double col2);
    QScopedPointer<HalconCpp::HWindow> halconBuffer;
    QPointF lastMousePos;
    double lastRow1, lastCol1, lastRow2, lastCol2;

    bool displayFullImageSize {false};
    int ImageWidth {100};
    int ImageHeight {100};
};

#endif // !QHALCONWINDOW_H
