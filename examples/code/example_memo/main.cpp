#include <QApplication>
#include <QMessageBox>

#include <CuteReport>

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

    // preparing template

    CuteReport::ReportInterface * reportObject = reportCore->createReport();
    CuteReport::PageInterface * page = reportCore->createPageObject("Extended::Page",reportObject);
    if (!page)
        errorAndExit();
    reportObject->addPage(page);

    CuteReport::BaseItemInterface * band = reportCore->createItemObject("Page Header",reportObject, page);
    page->addItem(band);

    CuteReport::BaseItemInterface * memo = reportCore->createItemObject("Extended::Memo", reportObject, band);
    if (!memo)
        errorAndExit();
    page->addItem(memo);

    // manage template objects
    band->setProperty("backgroundBrush", QBrush(Qt::darkBlue));

    memo->setProperty("backgroundBrush", QBrush(Qt::white));
    memo->setProperty("geometry", QRectF(5,5,50,10));
    memo->setProperty("opacity", 0.4);

    //

    preview->connectReport(reportObject);
    preview->show();
    preview->run();

    a.exec();

    delete preview;
    delete reportCore;
}
