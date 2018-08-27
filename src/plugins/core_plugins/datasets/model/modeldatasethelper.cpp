/***************************************************************************
 *   This file is part of the CuteReport project                           *
 *   Copyright (C) 2014 by Ivan Volkov                                     *
 *   wulff007@gmail.com                                                    *
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
 **                  GNU Lesser General Public License                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library.                                      *
 *   If not, see <http://www.gnu.org/licenses/>.                           *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ***************************************************************************/
#include "modeldatasethelper.h"
#include "ui_modeldatasethelper.h"
#include "modeldataset.h"
#include "models/columntypemodel.h"
#include "models/testmodel.h"

#include <QStringListModel>
#include <QShortcut>
#include <QInputDialog>
#include <QMenu>

ModelDatasetHelper::ModelDatasetHelper(ModelDataset *dataset, CuteDesigner::Core *designer) :
    CuteReport::DatasetHelperInterface(designer),
    ui(new Ui::ModelDatasetHelper),
    m_dataset(dataset),
    m_activatedSection(-1)
{
    ui->setupUi(this);

    m_typeModel = new ColumnTypeModel(this);
    ui->cmbCoulmnType->setModel(m_typeModel);
    ui->cmbCoulmnType->setModelColumn(ColumnTypeModel::NameCol);

    ui->labType->setVisible(false);
    ui->cmbCoulmnType->setVisible(false);

    ui->tableView->setModel(m_dataset->testModel());
    ui->tableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

    m_acRename = new QAction(tr("Rename column"), this);
    connect(m_acRename, SIGNAL(triggered()), SLOT(slotRenameActivatedColumn()));

    load();

    connect(ui->cmbCoulmnType, SIGNAL(currentIndexChanged(int)), SLOT(onCmbTypeChanged()));
    connect(ui->btnAddRow, SIGNAL(clicked()), SLOT(addRow()));
    connect(ui->btnDeleteRow, SIGNAL(clicked()), SLOT(removeRow()));
    connect(ui->btnAddColumn, SIGNAL(clicked()), SLOT(addColumn()));
    connect(ui->btnDeleteColumn, SIGNAL(clicked()), SLOT(removeColumn()));
    connect(ui->tableView->horizontalHeader(), SIGNAL(sectionDoubleClicked(int)), SLOT(slotRenameColumn(int)));
    connect(m_dataset->testModel(), SIGNAL(changed()), m_dataset, SIGNAL(changed()));
    connect(ui->edModelName, SIGNAL(textChanged(QString)), m_dataset, SIGNAL(changed()));
    connect(ui->tableView->horizontalHeader(), SIGNAL(customContextMenuRequested(QPoint)), SLOT(showContextHorizontalHeaderMenu(QPoint)));

    QShortcut *scRemove = new QShortcut(this);
    scRemove->setKey(QKeySequence::Delete);
    connect(scRemove, SIGNAL(activated()), SLOT(scRemove()));
}

ModelDatasetHelper::~ModelDatasetHelper()
{
    delete ui;
}

void ModelDatasetHelper::save()
{
    m_dataset->setAddressVariable(ui->edModelName->text());
    m_dataset->setTestModelData(m_dataset->testModel()->save());
}

void ModelDatasetHelper::load()
{
    ui->edModelName->setText(m_dataset->addressVariable());
    m_dataset->testModel()->load(m_dataset->testModelData());
}

void ModelDatasetHelper::addRow()
{
    saveCurrentEditedCell();
    int row = currentIndex().isValid()
            ? currentIndex().row() + 1
            : m_dataset->testModel()->rowCount();

    m_dataset->testModel()->insertRow(row);
}

void ModelDatasetHelper::removeRow()
{
    if (!currentIndex().isValid()) {
        return;
    }

    m_dataset->testModel()->removeRow(currentIndex().row());
}

void ModelDatasetHelper::addColumn()
{
    saveCurrentEditedCell();
    int col = currentIndex().isValid()
            ? currentIndex().column() + 1
            : m_dataset->testModel()->columnCount();

    m_dataset->testModel()->insertColumn(col);
}

void ModelDatasetHelper::removeColumn()
{
    if (!currentIndex().isValid()) {
        return;
    }

    m_dataset->testModel()->removeColumn(currentIndex().column());
}

void ModelDatasetHelper::onCmbTypeChanged()
{
}

void ModelDatasetHelper::scRemove()
{
    QModelIndexList cols = ui->tableView->horizontalHeader()->selectionModel()->selectedColumns();
    if (cols.count() > 0) {
        QListIterator<QModelIndex> iter(cols);
        while (iter.hasNext()) {
            QModelIndex col = iter.next();
            ui->tableView->model()->removeColumn(col.column());
        }
    }

    QModelIndexList rows = ui->tableView->verticalHeader()->selectionModel()->selectedRows();
    if (rows.count() > 0) {
        QListIterator<QModelIndex> iter(rows);
        while (iter.hasNext()) {
            QModelIndex row = iter.next();
            ui->tableView->model()->removeRow(row.row());
        }
    }

    QModelIndexList idxs = ui->tableView->selectionModel()->selectedIndexes();
    if (idxs.count() > 0) {
        QListIterator<QModelIndex> iter(idxs);
        while (iter.hasNext()) {
            QModelIndex index = iter.next();
            ui->tableView->model()->setData(index, QString());
        }
    }
}

void ModelDatasetHelper::slotRenameColumn(int column)
{
    QInputDialog dlg;
    dlg.setWindowTitle(tr("Rename column"));
    dlg.setInputMode(QInputDialog::TextInput);
    dlg.setTextValue(ui->tableView->model()->headerData(column, Qt::Horizontal, Qt::DisplayRole).toString());

    if (dlg.exec() == QDialog::Accepted) {
        model()->setHeaderData(column, Qt::Horizontal, dlg.textValue(), Qt::DisplayRole);
    }
}

void ModelDatasetHelper::slotRenameActivatedColumn()
{
    slotRenameColumn(m_activatedSection);
}

void ModelDatasetHelper::showContextHorizontalHeaderMenu(QPoint p)
{
    m_activatedSection = ui->tableView->horizontalHeader()->logicalIndexAt(p);

    QMenu menu;
    menu.addAction(m_acRename);

    menu.exec(QCursor::pos());
}

QVariant::Type ModelDatasetHelper::currentColumnType() const
{
    return m_typeModel->type(ui->cmbCoulmnType->currentIndex());
}

void ModelDatasetHelper::setCurrentColumnType(QVariant::Type type)
{
    for (int i = 0; i < m_typeModel->rowCount(); i++) {
        if (m_typeModel->type(i) == type) {
            ui->cmbCoulmnType->setCurrentIndex(i);
            break;
        }
    }
}

QModelIndex ModelDatasetHelper::currentIndex() const
{
    return ui->tableView->currentIndex();
}

TestModel *ModelDatasetHelper::model()
{
    return m_dataset->testModel();
}

void ModelDatasetHelper::saveCurrentEditedCell()
{
    ui->tableView->closePersistentEditor(ui->tableView->currentIndex());
}
