#include <QApplication>

#include <CuteReport>
#include "helperobject.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    CuteReport::ReportCore * reportCore = new CuteReport::ReportCore(0 ,0, false);
    CuteReport::ReportPreview * preview = new CuteReport::ReportPreview();

    preview->setReportCore(reportCore);

    // preparing template

    CuteReport::ReportInterface * reportObject = reportCore->createReport();

    CuteReport::DatasetInterface * data = reportCore->createDatasetObject("Model");
    reportObject->addDataset(data);

    /// Extended Page is more functional but available only in Pro version
    /// So we are trying to create extended page first and then Standard one if Extended is not available

    CuteReport::PageInterface * page = reportCore->createPageObject("Extended::Page; Standard::Page");
    reportObject->addPage(page);

    CuteReport::BaseItemInterface * band = reportCore->createItemObject("Detail");
    page->addItem(band);

    CuteReport::BaseItemInterface * memo = reportCore->createItemObject("Extended::Memo, Standard::Memo");
    page->addItem(memo);
    memo->setParentItem(band);

    band->setProperty("dataset", data->objectName());
    data->setProperty("addressVariable","model1");

    /// get text from named and column number 1 (starting from 1)
    memo->setProperty("text", QString("[%1.\"1\"]").arg(data->objectName()));

    // preparing model

    QStringList list;
    list << "11111" << "2222" << "333" << "44" << "5";

    QStringListModel * model = new QStringListModel();
    model->setStringList(list);

    // sending model address to the report

    /// old style deprecated
    reportObject->setVariableValue("model1", quint64(model));

    /// new style. Does not work - regression in v 1.3.0
//    reportObject->setVariableObject("model1", model);

    HelperObject helper(memo);

    /// set random background
    QObject::connect(memo, SIGNAL(printBefore()), &helper, SLOT(slotMemoBeforePrint()));

    preview->connectReport(reportObject);
    preview->show();
    preview->run();

    a.exec();

    delete preview;
    delete reportCore;
}

