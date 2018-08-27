#include "optionsdialogdesigner.h"
#include "ui_optionsdialogdesigner.h"
#include "designercore.h"
#include "reportinterface.h"
#include "baseiteminterface.h"
#include "reportcore.h"

OptionsDialogDesigner::OptionsDialogDesigner(CuteDesigner::Core *core) :
    OptionsDialogPageInterface(core),
    ui(new Ui::OptionsDialogDesigner),
    m_core(core)
{
    ui->setupUi(this);

    ui->cbItemsDetailLevel->addItem("Lowest", (int)CuteReport::TemplateDetailLevel_Lowest);
    ui->cbItemsDetailLevel->addItem("Name only", (int)CuteReport::TemplateDetailLevel_NameOnly);
    ui->cbItemsDetailLevel->addItem("Medium", (int)CuteReport::TemplateDetailLevel_Medium);
    ui->cbItemsDetailLevel->addItem("Full Context", (int)CuteReport::TemplateDetailLevel_Full);
}


OptionsDialogDesigner::~OptionsDialogDesigner()
{
    delete ui;
}


void OptionsDialogDesigner::activate()
{
    int loadMethod = m_core->settings()->value("CuteReport/ReportLoadingMethod", 1).toInt();
    switch (loadMethod) {
        case 0: ui->rbLoadingStorage->setChecked(true); break;
        case 1: ui->rbLoadingOS->setChecked(true); break;
        case 2: ui->rbLoadingQt->setChecked(true); break;
    }

    int dLevel = m_core->settings()->value("CuteReport_Designer/ItemDetailLevel", (int)CuteReport::TemplateDetailLevel_Full).toInt();
    ui->cbItemsDetailLevel->setCurrentIndex( ui->cbItemsDetailLevel->findData(dLevel) );
}


void OptionsDialogDesigner::deactivate()
{
    int loadMethod = 0;
    if (ui->rbLoadingOS->isChecked()) loadMethod = 1;
    if (ui->rbLoadingQt->isChecked()) loadMethod = 2;
    m_core->settings()->setValue("CuteReport/ReportLoadingMethod", loadMethod);

    int dOldLevel = m_core->settings()->value("CuteReport_Designer/ItemDetailLevel", (int)CuteReport::TemplateDetailLevel_Full).toInt();
    int dLevel = ui->cbItemsDetailLevel->itemData(ui->cbItemsDetailLevel->currentIndex()).toInt();
    m_core->settings()->setValue("CuteReport_Designer/ItemDetailLevel", dLevel);

    /// update detail level for all items if necessary
    if (dOldLevel != dLevel) {
        QList<CuteReport::ReportInterface *> reports = m_core-> reportCore()->reports();
        foreach (CuteReport::ReportInterface * report, reports) {
            foreach (CuteReport::BaseItemInterface * item, report->items()) {
                item->setTemplateDetailLevel(CuteReport::TemplateItemDetailLevel(dLevel));
            }
        }
    }
}


QListWidgetItem *OptionsDialogDesigner::createButton(QListWidget *listWidget)
{
    QListWidgetItem *configButton = new QListWidgetItem(listWidget);
    configButton->setIcon(QIcon(":/images/options_report.png"));
    configButton->setText(tr("Designer"));
    configButton->setTextAlignment(Qt::AlignHCenter);
    configButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    return configButton;
}
