#include "emptydialog.h"
#include "ui_emptydialog.h"
#include "baseiteminterface.h"

#include <QAbstractButton>
#include <QKeyEvent>

EmptyDialog::EmptyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EmptyDialog),
    m_helperWidget(false),
    m_widget(0)
{
    ui->setupUi(this);
    foreach (QAbstractButton * b, ui->buttonBox->buttons())
        b->setFocusPolicy(Qt::NoFocus);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(slotAccept()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(slotReject()));
}


EmptyDialog::~EmptyDialog()
{
    delete ui;
}


void EmptyDialog::setHelperWidget(QWidget *widget)
{
    delete m_widget;
    m_widget = widget;
    ui->mainLayout->addWidget(widget);
    m_helperWidget = true;
}


void EmptyDialog::slotAccept()
{
    if (!m_helperWidget)
        accept();

    CuteReport::BaseItemHelperInterface * helper = dynamic_cast<CuteReport::BaseItemHelperInterface *>(m_widget);
    if (!helper || !helper->screenBack(true))
        accept();
}


void EmptyDialog::slotReject()
{
    if (!m_helperWidget)
        reject();
    CuteReport::BaseItemHelperInterface * helper = dynamic_cast<CuteReport::BaseItemHelperInterface *>(m_widget);
    if (!helper || !helper->screenBack(false))
        reject();
}


void EmptyDialog::keyPressEvent(QKeyEvent * event)
{
    if  (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (event->modifiers() == Qt::ControlModifier)
            accept();
        return;
    }

    QDialog::keyPressEvent(event);
}

