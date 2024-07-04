#include "exportcsv.h"

ExportCSV::ExportCSV(QObject *parent)
    : QObject{parent}
{
    m_outputDir = "";
    m_timeStamp = QDateTime::currentDateTime();
}

void ExportCSV::setOutputDirectory(const QString &dir)
{
    if (dir != m_outputDir)
    {
        m_outputDir = dir;
    }
}


void ExportCSV::exportMeasurementToCSV(const HalconCore::MeasSpecView &measSpecView)
{
    QString filePath = m_outputDir + "/" + measSpecView.partNumber + "_" + measSpecView.view + "_" + m_timeStamp.toString("yyyy_MM_dd_hh_mm_ss") + ".csv";

    QFile file(filePath);

    if (!file.open(QIODevice::Text | QIODevice::WriteOnly))
    {
        qDebug() << "Unable to open file" << filePath <<  "for writing";
        return;
    }

    QTextStream stream(&file);

    // write header data
    stream << "Part No.: " << "," << measSpecView.partNumber << "\n";
    stream << "Export Time: " << "," << m_timeStamp.toString("yyyy-MM-dd hh:mm:ss") <<  "\n";
    stream << "Side View: " << "," << measSpecView.view << "\n";
    stream << "Side Status: " << "," << measSpecView.inspectionStatus << "\n";
    stream << "\n";

    // write columns titles
    stream << "SpecId" << "," <<
        "Reference Value (mm)" << "," <<
        "Inspection Value (mm)" << "," <<
        "Difference (mm)" << "," <<
        "Tolerance Min. (mm)" << "," <<
        "Tolerance Max. (mm)" << "," <<
        "Status" << "\n";

    // write measurement content
    for (int i = 0; i < measSpecView.measList.size(); i++)
    {
        stream << measSpecView.measList[i].specId << "," <<
            measSpecView.measList[i].specValue << "," <<
            measSpecView.measList[i].measValue << "," <<
            measSpecView.measList[i].difference << "," <<
            measSpecView.measList[i].toleranceMin << "," <<
            measSpecView.measList[i].toleranceMax << "," <<
            measSpecView.measList[i].status << "," << "\n";
    }

    file.close();
}
