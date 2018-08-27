/** NOTE: this example will work only with Pro version of CuteReport */

#include <QApplication>

#include <CuteReport>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();

    CuteReport::ReportCore * reportCore = new CuteReport::ReportCore(0 ,0, false);
    CuteReport::ReportPreview * preview = new CuteReport::ReportPreview();

    preview->setReportCore(reportCore);
    QString reportUrl;
#ifdef Q_OS_WIN
    reportUrl = "file:/C/Program Files (x86)/CuteReport/development/examples/reports/Translation.qtrp";
#else
    reportUrl = "file:/usr/share/cutereport/examples/examples/reports/Translation.qtrp";
#endif
    CuteReport::ReportInterface * reportObject = reportCore->loadReport(reportUrl);
    if (!reportObject) {
        QMessageBox::critical(0, QObject::tr("CuteReport example"),
                              QObject::tr("Report file loading error"),
                              QMessageBox::Ok);
        exit(1);
    }
    QString currentLang = reportObject->variableValue("tr").toString();
    QStringList languagesAvailable = reportObject->customData("translator_languages").toStringList();
    QStringList langCodesAvailable = reportObject->customData("translator_codes").toStringList();

    /// NOTE: this operation will not have effect on non Professional CuteReport version
    reportObject->setVariableValue("tr", "uk");

    QString newLang = reportObject->variableValue("tr").toString();
    preview->connectReport(reportObject);
    preview->show();
    preview->run();

    a.exec();

    delete preview;
    delete reportCore;
}
