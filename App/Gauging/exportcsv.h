#ifndef EXPORTCSV_H
#define EXPORTCSV_H

#include <QObject>
#include <QTextStream>
#include <QFile>
#include <QDateTime>
#include <QDebug>
#include "halconcore.h"

class ExportCSV : public QObject
{
    Q_OBJECT
public:
    explicit ExportCSV(QObject *parent = nullptr);

    void setOutputDirectory(const QString &dir);
    void exportMeasurementToCSV(const HalconCore::MeasSpecView &measSpecView);
private:
    QString m_outputDir;
    QDateTime m_timeStamp;
};

#endif // EXPORTCSV_H
