/** NOTE: this example will work only with Pro version of CuteReport */

#include <QApplication>

#include <CuteReport>
#include "painterdelegate.h"

void errorAndExit() {
    QMessageBox::critical(0, QObject::tr("CuteReport example"),
                          QObject::tr("This example is designed to user features of the Professional version of CuteReport.\n"),
                          QMessageBox::Ok);
    exit(1);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    CuteReport::ReportCore * reportCore = new CuteReport::ReportCore(0 ,0, false);
    CuteReport::ReportPreview * preview = new CuteReport::ReportPreview();

    preview->setReportCore(reportCore);
    preview->setFitMode(CuteReport::ReportPreview::FitWidth);

    /// preparing template

    CuteReport::ReportInterface * reportObject = reportCore->createReport();
    CuteReport::DatasetInterface * data = reportCore->createDatasetObject("Model", reportObject);
    reportObject->addDataset(data);

    CuteReport::PageInterface * page = reportCore->createPageObject("Extended::Page",reportObject);
    if (!page) errorAndExit();
    reportObject->addPage(page);

    CuteReport::BaseItemInterface * band = reportCore->createItemObject("Detail",reportObject, page);
    if (!band) errorAndExit();
    page->addItem(band);

    CuteReport::BaseItemInterface * memo = reportCore->createItemObject("Extended::Memo", reportObject, band);
    if (!memo) errorAndExit();
    page->addItem(memo);

    CuteReport::BaseItemInterface * canvas = reportCore->createItemObject("Extended::Canvas", reportObject, band);
    if (canvas) {
        page->addItem(canvas);
        canvas->setProperty("geometry", QRectF(40,5,20,20));
        canvas->setProperty("backgroundBrush", QBrush(QColor(Qt::yellow)));
        canvas->setProperty("painterVariable", "painter");
        qDebug() << canvas->property("painterVariable");
    } else
        errorAndExit();

    band->setProperty("dataset", data->objectName());
    data->setProperty("addressVariable","model1");
    memo->setProperty("text", "[data.\"1\"]");

    /// preparing model

    QStringList list;
    list << "11111" << "2222" << "333" << "44" << "5";

    QStringListModel * model = new QStringListModel();
    model->setStringList(list);

    PainterDelegate paintDelegate;

    reportObject->setVariableObject("painter", &paintDelegate);

    preview->connectReport(reportObject);
    preview->show();
    preview->run();

    a.exec();

    delete preview;
    delete reportCore;
}

