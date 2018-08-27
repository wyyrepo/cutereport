#include <QApplication>

#include <CuteReport>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    CuteReport::ReportCore * reportCore = new CuteReport::ReportCore(0 ,0, false);
    CuteReport::ReportPreview * preview = new CuteReport::ReportPreview();

    preview->setReportCore(reportCore);

    /// preparing template

    CuteReport::ReportInterface * reportObject = reportCore->createReport();
    CuteReport::DatasetInterface * data = reportCore->createDatasetObject("Model", reportObject);
    reportObject->addDataset(data);

    /// Extended Page is more functional but available only in Pro version
    /// So we are trying to create extended page first and then Standard one if Extended is not available

    CuteReport::PageInterface * page = reportCore->createPageObject("Extended::Page", reportObject);
    if (!page)
        page = reportCore->createPageObject("Standard::Page",reportObject);
    reportObject->addPage(page);

    CuteReport::BaseItemInterface * band = reportCore->createItemObject("Detail", reportObject, page);
    page->addItem(band);

    /// Extended Memo is more functional but available only in Pro version
    /// So we are trying to create extended page first and then Standard one if Extended is not available

    CuteReport::BaseItemInterface * memo = reportCore->createItemObject("Extended::Memo", reportObject, band);
    if (!memo)
        memo = reportCore->createItemObject("Standard::Memo", reportObject, band);
    page->addItem(memo);

    band->setProperty("dataset", data->objectName());
    data->setProperty("addressVariable","model1");
    memo->setProperty("text", "[data.\"1\"]");

    // preparing model

    QStringList list;
    list << "11111" << "2222" << "333" << "44" << "5";

    QStringListModel * model = new QStringListModel();
    model->setStringList(list);

    // sending model address to the report

    reportObject->setVariableValue("model1",quint64(model));

    preview->connectReport(reportObject);
    preview->show();
    preview->run();

    a.exec();

    delete preview;
    delete reportCore;
}
