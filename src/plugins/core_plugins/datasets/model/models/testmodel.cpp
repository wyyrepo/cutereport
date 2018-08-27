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
#include "testmodel.h"

#include <QtXml>

TestModel::TestModel(QObject *parent) :
    QStandardItemModel(parent)
{
}

void TestModel::load(const QByteArray data)
{
    if (data.isEmpty()) {
        return;
    }

    clear();

    QByteArray d(QByteArray::fromBase64(data));
    QDomDocument domDoc;

    QString error;
    int errLine = -1;

    if(domDoc.setContent(d, &error, &errLine)) {
        QDomElement domElement= domDoc.documentElement();
        traverseNode(domElement);
    } else {
//        qDebug() << "Error while QDomDocument::setContent :" << error << "line: " << errLine;
    }
}

QByteArray TestModel::save()
{
    QDomDocument doc;
    QDomProcessingInstruction pi = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf8\"");
    doc.appendChild(pi);

    QDomElement modelEl = doc.createElement("model");
    doc.appendChild(modelEl);

    QDomElement  cellsEl = doc.createElement("cells");
    cellsEl.setAttribute("rows", rowCount());
    cellsEl.setAttribute("cols", columnCount());
    modelEl.appendChild(cellsEl);

    QDomElement titlesEl = doc.createElement("titles");
    for (int i = 0; i < columnCount(); i++) {
        QDomElement col = doc.createElement(QString("col%1").arg(i));
        QDomText text = doc.createTextNode(headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());
        col.appendChild(text);
        titlesEl.appendChild(col);
    }
    modelEl.appendChild(titlesEl);

    for (int row = 0; row < rowCount(); row++) {
        QDomElement itemRow = doc.createElement(QString("row%1").arg(row));

        for (int col = 0; col < columnCount(); col++ ) {

            //TODO save images
            if (index(row, col).data().toString().simplified().isEmpty()) {
                continue;
            }

            QDomElement itemCol = doc.createElement(QString("col%1").arg(col));
            itemCol.setAttribute("type", QVariant::typeToName(columnType(col)));

            QDomText text = doc.createTextNode(index(row, col).data().toByteArray());
            itemCol.appendChild(text);
            itemRow.appendChild(itemCol);
        }

        if (itemRow.childNodes().count() > 0)
            cellsEl.appendChild(itemRow);
    }

//    QFile file("c:/test.xml");
//    if(file.open(QIODevice::WriteOnly)) {
//        QTextStream(&file) << doc.toString();
//        file.close();
//    }

    return doc.toByteArray().toBase64();
}

void TestModel::traverseNode(const QDomNode &node)
{
    QDomElement cells = node.toElement().firstChildElement("cells");
    int rows = cells.attribute("rows").toInt();
    int cols = cells.attribute("cols").toInt();

    insertRows(0, rows);
    insertColumns(0, cols);

    if (rows == 0 || cols == 0) {
        return;
    }

    QDomNode rowNode = cells.firstChild();
    while (!rowNode.isNull()) {
        int row = rowNode.nodeName().remove(0, 3).toInt();

         QDomNode colNode = rowNode.firstChild();
         while (!colNode.isNull()) {
            int col = colNode.nodeName().remove(0, 3).toInt();

            QDomElement colEl = colNode.toElement();
            setColumnType(col, QVariant::nameToType(colEl.attribute("type").toLatin1()));
            setItem(row, col, new QStandardItem(colEl.text()));

            colNode = colNode.nextSibling();
         }

        rowNode = rowNode.nextSibling();
    }

    // Fill empty indexes
    for (int row = 0; row < rowCount(); row++) {

        for (int col = 0; col < columnCount(); col++) {

            if(!item(row, col)) {
                setItem(row, col, new QStandardItem(QString()));
            }
        }
    }

    QDomElement titles = node.toElement().firstChildElement("titles");
    QDomNode colNode = titles.firstChild();
    while (!colNode.isNull()) {
        int col = colNode.nodeName().remove(0, 3).toInt();
        setHeaderData(col, Qt::Horizontal, colNode.toElement().text());

        colNode = colNode.nextSibling();
    }

}

void TestModel::setColumnType(const int col, const QVariant::Type type)
{
    m_typeByColumn.insert(col, type);
}

QVariant::Type TestModel::columnType(int col)
{
    return m_typeByColumn.value(col, QVariant::String);
}

bool TestModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (QStandardItemModel::setData(index, value, role)) {
        emit changed();
        return true;
    }

    return false;
}

bool TestModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    if (QStandardItemModel::removeColumns(column, count, parent)) {
        emit changed();
        return true;
    }

    return false;
}

bool TestModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (QStandardItemModel::removeRows(row, count, parent)) {
        emit changed();
        return true;
    }

    return false;
}

bool TestModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    if (QStandardItemModel::insertColumns(column, count, parent)) {
        emit changed();
        return true;
    }

    return false;
}

bool TestModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (QStandardItemModel::insertRows(row, count, parent)) {
        emit changed();
        return true;
    }

    return false;
}

QVariant TestModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return m_titleByColumn.value(section, QString("field%1").arg(section + 1));
    }

    return QStandardItemModel::headerData(section, orientation, role);
}

bool TestModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        m_titleByColumn.insert(section, value.toString());
        changed();
    }

    return true;
}


