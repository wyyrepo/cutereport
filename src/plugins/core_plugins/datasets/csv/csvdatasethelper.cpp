/***************************************************************************
 *   This file is part of the CuteReport project                           *
 *   Copyright (C) 2012 by Mikhalov Alexander                              *
 *   alexander.mikhalov@gmail.com                                          *
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
 ****************************************************************************/
#include <QtGui>
#include "csvdatasethelper.h"
#include "csvdataset.h"
#include "ui_csvdatasethelper.h"
#include "reportcore.h"
#include "designercore.h"

CsvDatasetHelper::CsvDatasetHelper(CsvDataset * dataset, CuteDesigner::Core *designer) :
    DatasetHelperInterface(designer),
    m_ui(new Ui::CsvDatasetHelper),
    m_dataset(dataset)
{
    m_ui->setupUi(this);

    load();
    QString fileUrl = m_dataset->getFileName();
    if (!fileUrl.isEmpty())
        loadFile(fileUrl);
    connect (m_ui->cbKeepData, SIGNAL(toggled(bool)), this, SLOT(slotKeepDataInternalChanged(bool)));

    m_ui->cbMaxRecords->hide();
    m_ui->sbMaxRecords->hide();
}


CsvDatasetHelper::~CsvDatasetHelper()
{
    delete m_ui;
}


void CsvDatasetHelper::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


void CsvDatasetHelper::load()
{
    m_ui->leFileName->setText( m_dataset->getFileName());
    m_ui->leDelimeter->setText(m_dataset->getDelimeter());
    m_ui->leFileName->setText(m_dataset->getFileName());
    m_ui->cbFirstRowIsHeader->setCheckState(m_dataset->getFirstRowIsHeader() ? Qt::Checked : Qt::Unchecked);
    m_ui->cbFixProblems->setChecked(m_dataset->getFixFileIssues() ? Qt::Checked : Qt::Unchecked);
    m_ui->leSortFields->setText(m_dataset->getSortFields());
    m_ui->text->setPlainText(m_dataset->getTextData());
}


void CsvDatasetHelper::save()
{
    m_dataset->setFileName(m_ui->leFileName->text().trimmed());
    m_dataset->setDelimeter(m_ui->leDelimeter->text().trimmed());
    m_dataset->setFirstRowIsHeader((m_ui->cbFirstRowIsHeader->checkState() == Qt::Checked) ? true : false);
    m_dataset->setFixFileIssues((m_ui->cbFixProblems->checkState() == Qt::Checked) ? true : false );
    m_dataset->setSortFields(m_ui->leSortFields->text().trimmed());
    m_dataset->setTextData(m_dataset->getFileName().isEmpty() ? m_ui->text->toPlainText().trimmed() : QString());
}


void CsvDatasetHelper::on_bBrowse_clicked()
{
    QString fileURL = m_designer->loadObjectDialog(QStringList() << "csv" << "txt", "Load CSV file");
    loadFile(fileURL);
}


void CsvDatasetHelper::slotKeepDataInternalChanged(bool checked)
{
    m_ui->leFileName->setEnabled(!checked);
}


void CsvDatasetHelper::loadFile(const QString &fileURL)
{
    CuteReport::ReportInterface * report = static_cast<CuteReport::ReportInterface*> (m_dataset->parent());
    QByteArray fileData =  m_dataset->reportCore()->loadObject(fileURL, report);
    QTextStream stream(fileData);
    stream.setAutoDetectUnicode(true);
    QString fileContent = stream.readAll();

    if (!m_ui->cbKeepData->isChecked())
        m_ui->leFileName->setText(fileURL);
    m_ui->text->setPlainText(fileContent);
}

