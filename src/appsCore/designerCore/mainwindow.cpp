/***************************************************************************
 *   This file is part of the CuteReport project                           *
 *   Copyright (C) 2012-2017 by Alexander Mikhalov                         *
 *   alexander.mikhalov@gmail.com                                          *
 *                                                                         *
 **                   GNU General Public License Usage                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ***************************************************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "stylehelper.h"
#include "designercore.h"
#include "pageinterface.h"
#include "rendererinterface.h"
#include "aboutdialog.h"
#include "optionsdialog.h"
#include "messagelist.h"
#include "reportcore.h"
#include "log.h"
#include "functions_gui.h"

#include <QtGui>
#include <QDesktopWidget>

MainWindow::MainWindow(CuteDesigner::Core *core) :
    QMainWindow(),
    ui(new Ui::MainWindow),
    m_core(core)
{
    ui->setupUi(this);
    //    defaultPageActions = ui->menuPage->actions();

    //    foreach (QAction * action, defaultPageActions)
    //        action->setParent(this);

    ui->actionExport->setEnabled(m_core->reportCore()->modules(CuteReport::ExportModule).size());

    // Initialise the global search widget
    //StyleHelper::setBaseColor(palette().color(QPalette::Highlight).darker());

    //ui->tabs->SetBackgroundPixmap(QPixmap(":/images/sidebar_background.png"));

    connect(ui->actionRender, SIGNAL(triggered()), core, SLOT(render()), Qt::QueuedConnection);
    connect(ui->actionRenderStop, SIGNAL(triggered()), core, SLOT(stopRenderer()), Qt::QueuedConnection);
    connect(ui->tabs, SIGNAL(CurrentChanged(int)), this, SLOT(slotTabsSwitched(int)));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(slotAbout()));
    connect(ui->actionOptions, SIGNAL(triggered()), this, SLOT(slotOptions()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SIGNAL(closeRequest()));

    connect(m_core, SIGNAL(newReport_after(CuteReport::ReportInterface*)), this, SLOT(slotNewReport_after(CuteReport::ReportInterface*)), Qt::QueuedConnection);
    connect(m_core, SIGNAL(newPage_after(CuteReport::PageInterface*)), this, SLOT(slotNewPage_after(CuteReport::PageInterface*)), Qt::QueuedConnection);
    connect(m_core, SIGNAL(loadReport_after(CuteReport::ReportInterface*)), this, SLOT(slotLoadReport_after(CuteReport::ReportInterface*)), Qt::QueuedConnection);
    //    connect(m_core, SIGNAL(newPageActions(QList<CuteReport::PageAction*>)), this, SLOT(slotNewPageActions(QList<CuteReport::PageAction*>)), Qt::QueuedConnection);
    connect(CuteReport::Log::instance(), SIGNAL(logMessage(CuteReport::LogLevel,QString,QString,QString)), ui->message, SLOT(pushMessage(CuteReport::LogLevel,QString,QString,QString)));
    connect(m_core->reportCore(), SIGNAL(rendererStarted(CuteReport::ReportInterface*)), ui->message, SLOT(clear()));
    //    reloadSettings();

    ui->geometryLabel->hide();
    ui->coordinateLabel->hide();

    prepareStdActions();

    m_updateTabTimer.setInterval(10);
    m_updateTabTimer.setSingleShot(true);
    connect(&m_updateTabTimer, SIGNAL(timeout()), this, SLOT(slotUpdateTabs()));
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::closeEvent( QCloseEvent* event)
{
    emit closeRequest();
    event->ignore();
}


void MainWindow::reloadSettings()
{
    CuteReport::widgetGeometryFromString(this, m_core->getSettingValue("CuteReport_Designer/geometry").toString());
    this->setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            this->size(),
            qApp->desktop()->availableGeometry()
        )
    );
    FancyTabWidget::Mode default_mode = FancyTabWidget::Mode_LargeSidebar;
    ui->tabs->SetMode(FancyTabWidget::Mode(m_core->getSettingValue("CuteReport_Designer/tabMode", default_mode).toInt()));
    ui->tabs->SetCurrentIndex(0);
}


void MainWindow::saveSettings()
{
    m_core->setSettingValue("CuteReport_Designer/geometry", CuteReport::widgetGeometryToString(this));
    m_core->setSettingValue("CuteReport_Designer/tabMode", ui->tabs->mode());
}


void MainWindow::addTab(QWidget * widget, QIcon icon, QString name)
{
    ui->tabs->AddTab(widget, icon, name);
    //    ui->tabs->updateState();
    m_updateTabTimer.start();
}


void MainWindow::addTabSpacer()
{
    ui->tabs->AddSpacer(40, QSizePolicy::Expanding);
}


void MainWindow::slotLoadReport_after(CuteReport::ReportInterface *report)
{
    Q_UNUSED(report)
}


void MainWindow::slotNewReport_after(CuteReport::ReportInterface * report)
{
    Q_UNUSED(report)
    setWindowTitle(tr("CuteReport v%1 (unsaved report)").arg( REPORT_VERSION ));
}


void MainWindow::slotNewPage_after(CuteReport::PageInterface*)
{
    setWindowTitle(tr("CuteReport v%1 (unsaved report)").arg( REPORT_VERSION ));
}


void MainWindow::slotTabsSwitched(int index)
{
    m_core->moduleSwitched(index);
}


void MainWindow::slotAbout()
{
    AboutDialog dialog;
    dialog.exec();
}


void MainWindow::slotOptions()
{
    OptionsDialog * dialog = new OptionsDialog(m_core);
    dialog->exec();
    delete dialog;
    emit optionsDialogClosed();
}


void MainWindow::slotUpdateTabs()
{
    ui->tabs->updateState();
}


void MainWindow::slotStdActionTriggered()
{
    QAction * action = dynamic_cast<QAction*>(sender());
    CuteDesigner::Core::StdAction actionType = m_actionsMap.value(action, CuteDesigner::Core::StdAction(0));
    emit stdActionTriggered(actionType, action);
}


void MainWindow::switchToTab(int index)
{
    ui->tabs->SetCurrentIndex(index);
}


void MainWindow::addToStatusBar(QLabel *label)
{
    ui->statusBar->layout()->addWidget(label);
    label->setFrameShape(QFrame::Panel);
    label->setFrameShadow(QFrame::Sunken);
    label->setLineWidth(1);
}

void MainWindow::showMetric(CuteReport::MetricType type, const QVariant & value)
{
    Q_UNUSED(value);
    QLabel * label = 0;
    QString msg;
    switch (type) {
        //    case MessageGeneral: label = ui->messageLabel; break;
        case CuteReport::MetricGeometry: label = ui->geometryLabel; break;
        case CuteReport::MetricPosition: label = ui->coordinateLabel; break;
        default: break;
    }

    //    switch (level){
    //    case MessageUndefined:
    //    case MessageNormal: msg = message; break;
    //    case MessageWarning: QString("<font color='#FFA858'>%1</font>").arg(message); break;
    //    case MessageError: QString("<font color='red'>%1</font>").arg(message); break;
    //    }

    if (label)
        label->setText(msg);
}


void MainWindow::setStdActions(CuteDesigner::Core::StdActions actions)
{
    QHash<QAction*,CuteDesigner::Core::StdAction>::iterator i;
    for (i = m_actionsMap.begin(); i != m_actionsMap.end(); ++i)
        i.key()->setEnabled(actions.testFlag(i.value()));
}


void MainWindow::prepareStdActions()
{
    m_actionsMap.insert(ui->actionUndo, CuteDesigner::Core::ActionUndo);
    m_actionsMap.insert(ui->actionRedo, CuteDesigner::Core::ActionRedo);
    m_actionsMap.insert(ui->actionCut, CuteDesigner::Core::ActionCut);
    m_actionsMap.insert(ui->actionCopy, CuteDesigner::Core::ActionCopy);
    m_actionsMap.insert(ui->actionPaste, CuteDesigner::Core::ActionPaste);
    m_actionsMap.insert(ui->actionDelete, CuteDesigner::Core::ActionDelete);
    connect(ui->actionUndo, SIGNAL(triggered()), this, SLOT(slotStdActionTriggered()));
    connect(ui->actionRedo, SIGNAL(triggered()), this, SLOT(slotStdActionTriggered()));
    connect(ui->actionCut, SIGNAL(triggered()), this, SLOT(slotStdActionTriggered()));
    connect(ui->actionCopy, SIGNAL(triggered()), this, SLOT(slotStdActionTriggered()));
    connect(ui->actionPaste, SIGNAL(triggered()), this, SLOT(slotStdActionTriggered()));
    connect(ui->actionDelete, SIGNAL(triggered()), this, SLOT(slotStdActionTriggered()));
}
