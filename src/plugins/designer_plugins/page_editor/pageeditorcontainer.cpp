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
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ***************************************************************************/
#include "pageeditorcontainer.h"
#include "ui_pageeditorcontainer.h"
#include "pageinterface.h"
#include "reportcore.h"
#include "pageeditor.h"


PageEditorContainer::PageEditorContainer(PageEditor * pageEditor, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageEditorContainer),
    m_pageEditor(pageEditor)
{
    ui->setupUi(this);
    ui->pageTabs->hide();
}


PageEditorContainer::~PageEditorContainer()
{
    delete ui;
}


void PageEditorContainer::init()
{
    connect(ui->deletePageButton, SIGNAL(clicked()), this, SLOT(slotDeleteClicked()));
    connect(ui->addPageButton, SIGNAL(clicked()), this, SLOT(slotCreateClicked()));
    connect(ui->clonePageButton, SIGNAL(clicked()), this, SLOT(slotCloneClicked()));
    connect(ui->pageMoveFront, SIGNAL(clicked()), m_pageEditor, SLOT(slotPageMoveFront()));
    connect(ui->pageMoveBack, SIGNAL(clicked()), m_pageEditor, SLOT(slotPageMoveBack()));
    connect(ui->pageTabs, SIGNAL(CurrentChanged(int)), this, SLOT(slotCurrentTabChanged(int)));
    connect(ui->pageTabs, SIGNAL(tabDoubleClicked(int)), this, SLOT(slotTabDoubleClicked(int)));

    ui->tools->setImagesPath(m_pageEditor->core()->reportCore()->imagesPath());
}


void PageEditorContainer::saveSettings()
{
    m_pageEditor->core()->setSettingValue("CuteReport_PageEditor/splitterState", ui->splitter->saveState());
    m_pageEditor->core()->setSettingValue("CuteReport_PageEditor/splitter2State", ui->splitter2->saveState());
    m_pageEditor->core()->setSettingValue("CuteReport_PageEditor/tabMode", ui->pageTabs->mode());
}


void PageEditorContainer::updateButtonsStatus()
{
    ui->deletePageButton->setEnabled(ui->pageTabs->tabsCount());
    ui->clonePageButton->setEnabled(ui->pageTabs->tabsCount());
    ui->pageMoveFront->setEnabled(ui->pageTabs->tabsCount() > 1);
    ui->pageMoveBack->setEnabled(ui->pageTabs->tabsCount() > 1);
    ui->actions->setEnabled(ui->pageTabs->tabsCount());
    ui->tools->setEnabled(ui->pageTabs->tabsCount());
}


FontEditor *PageEditorContainer::fontEditor()
{
    return ui->fontEditor;
}


AlignmentEditor *PageEditorContainer::alignmentEditor()
{
    return ui->alignmentEditor;
}


FrameEditor *PageEditorContainer::frameEditor()
{
    return ui->frameEditor;
}


void PageEditorContainer::reloadSettings()
{
    QVariant value;
    if ((value =  m_pageEditor->core()->getSettingValue("CuteReport_PageEditor/splitterState")).isNull())
        ui->splitter->setSizes( QList<int>() << width()*0.8 << width()*0.2 );
    else
        ui->splitter->restoreState(value.toByteArray());

    if ((value =  m_pageEditor->core()->getSettingValue("CuteReport_PageEditor/splitter2State")).isNull())
        ui->splitter2->setSizes( QList<int>() << width()*0.4 << width()*0.6 );
    else
        ui->splitter2->restoreState(value.toByteArray());

    FancyTabWidget::Mode default_mode = FancyTabWidget::Mode_LargeSidebar;
    ui->pageTabs->SetMode(FancyTabWidget::Mode(m_pageEditor->core()->getSettingValue("CuteReport_PageEditor/tabMode", default_mode).toInt()));
}


void PageEditorContainer::addPropertyEditor(QWidget * widget)
{
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->PropertyEditorLayout->addWidget(widget);
}


void PageEditorContainer::addObjectInspector(QWidget * widget)
{
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->ObjectInspectorLayout->addWidget(widget);
}


void PageEditorContainer::addTab(QWidget * widget, QIcon icon, const QString &name)
{
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->pageTabs->AddTab(widget, icon, name);
    ui->pageTabs->updateState();
    updateButtonsStatus();
}


void PageEditorContainer::removeTab(const QString & name)
{
    ui->pageTabs->deleteTab(name);
    ui->pageTabs->updateState();
    updateButtonsStatus();
}


void PageEditorContainer::removeAllTabs()
{
    ui->pageTabs->deleteAllTabs();
    ui->pageTabs->updateState();
    updateButtonsStatus();
}


void  PageEditorContainer::setCurrentTab(const QString &pageName)
{
    ui->pageTabs->blockSignals(true);
    for (int i=0; i<ui->pageTabs->tabsCount(); ++i)
        if (ui->pageTabs->tabText(i) == pageName) {
            ui->pageTabs->SetCurrentIndex(i);
            break;
        }
    ui->pageTabs->blockSignals(false);
}


void PageEditorContainer::setNewPageName(const QString &pageName, const QString &newName)
{
    for (int i=0; i<ui->pageTabs->tabsCount(); ++i)
        if (ui->pageTabs->tabText(i) == pageName) {
            ui->pageTabs->changeText(newName, i );
            break;
        }
}


void PageEditorContainer::addPagePlugins(QList<CuteReport::ReportPluginInterface*> pages)
{
    if (pages.count() > 1) {
        delete ui->addPageButton->menu();
        Menu * menu = new Menu( this );
        foreach(CuteReport::ReportPluginInterface * plugin, pages) {
            CuteReport::PageInterface * page = static_cast<CuteReport::PageInterface *>(plugin);
            QString actionName = QString("%1 (%2)").arg(page->moduleShortName(), page->suitName());
            QAction * newItem = new QAction(page->icon(), actionName, this );
            newItem->setData(page->moduleFullName());
            menu->addAction(newItem);
        }
        ui->addPageButton->setMenu(menu);
    } else if (pages.count() == 1){
        m_pageeModuleName = pages.at(0)->moduleFullName();
    }
}


void PageEditorContainer::slotDeleteClicked()
{
    if (!ui->pageTabs->tabsCount())
        return;

    emit requestForDeletePage(ui->pageTabs->current_text());
}


void PageEditorContainer::slotCreateClicked()
{
    emit requestForCreatePage(m_pageeModuleName);
}


void PageEditorContainer::slotCloneClicked()
{
    if (!ui->pageTabs->tabsCount())
        return;

    emit requestForClonePage(ui->pageTabs->current_text());
}


void PageEditorContainer::slotCurrentTabChanged(int)
{
    emit currentTabChanged(ui->pageTabs->current_text());
}


void PageEditorContainer::slotTabDoubleClicked(int index)
{
    emit requestForRenamePage(ui->pageTabs->tabText(index));
}


//void PageEditorContainer::slotNewPageActions(QList<CuteReport::PageAction*> actions)
//{
//    ui->pageActionsToolBar->clear();
//    ui->menuPage->clear();
//    ui->menuPage->addActions(defaultPageActions);
//    foreach (CuteReport::PageAction * pageAction, actions) {
//        ui->pageActionsToolBar->addAction(pageAction->action);
//        ui->menuPage->addAction(pageAction->action);
//    }
//}


void PageEditorContainer::addItem(const QIcon &icon, const QString &name, const QString &suiteName, const QString &group)
{
    ui->tools->addItem(icon, name, suiteName, group);
}


void PageEditorContainer::setPageActions(QList<CuteReport::PageAction*> actions)
{
    QList<QAction*> actionList = ui->actions->actions();
    foreach (QAction * action, actionList)
        ui->actions->removeAction(action);


    foreach (CuteReport::PageAction * pageAction, actions) {
        ui->actions->addAction(pageAction->action);
    }
}
