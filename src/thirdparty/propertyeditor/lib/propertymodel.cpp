/***************************************************************************
 *   This file is part of the propertyEditor project                       *
 *   Copyright (C) 2008 by BogDan Vatra                                    *
 *   bog_dan_ro@yahoo.com                                                  *
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

#include <QMetaProperty>
#include <QMetaEnum>
#include <QSize>
#include <QFontMetrics>
#include <QtCore>

#include "propertymodel.h"
#include "propertyinterface.h"

//workaround for Qt4 that doesn't send notification for objectName changes
#if QT_VERSION <= 0x050000
#define FIRST_PROP 1
#else
#define FIRST_PROP 0
#endif

namespace PropertyEditor
{

PropertyModel::PropertyModel(const QList<PropertyInterface *> *plugins, QObject* parent):
    QAbstractItemModel(parent),
    m_object(0),
    m_plugins(plugins),
    m_readOnly(false)
{
    m_sizeHint = QFontMetrics(QFont()).lineSpacing()+6;
    refreshProperties();
}

PropertyModel::~PropertyModel()
{
    clearCreatedProperties();
}

void PropertyModel::clearCreatedProperties()
{
    foreach (QPointer<PropertyInterface> prop, m_createdProperties)
        delete prop;
    m_createdProperties.clear();
}

void PropertyModel::refreshProperties()
{
    beginResetModel();

    qDeleteAll(m_properties);
    m_properties.clear();
    clearCreatedProperties();

    if (!m_object || !m_plugins)
        return;

    QStringList hiddenList = propertiesHidden();
//    if (!hiddenList.isEmpty()) {
//        qDebug() << hiddenList;
//    }

    for (int i = FIRST_PROP;i < m_object->metaObject()->propertyCount();i++) {
        QMetaProperty prop = m_object->metaObject()->property(i);
//        qDebug() << "prop: " << m_object->metaObject()->property(i).name();
        if (prop.isDesignable() && prop.isValid() && prop.isWritable() && prop.isReadable() && !hiddenList.contains( prop.name() ) )
            foreach(PropertyInterface* plugin, (*m_plugins)) {
                if (plugin->canHandle(m_object, i)) {
                    PropertyInterface* pi = plugin->createInstance(m_object, i, this);
                    connect(pi, SIGNAL(propertyChanged(QObject*,QString,QVariant,QVariant)), QObject::parent(), SIGNAL(propertyChanged(QObject*,QString,QVariant,QVariant)));
                    m_createdProperties.push_back(pi);
                    m_properties.push_back(pi);
                    break;
                }
            }
    }

    endResetModel();
}


void PropertyModel::setReadOnly(bool b)
{
    m_readOnly = b;
}

PropertyInterface* PropertyModel::createPropery(QObject* object, int property) const
{
    PropertyInterface* pi = 0;
    QMetaProperty prop = object->metaObject()->property(property);
    if (prop.isValid() && prop.isWritable() && prop.isReadable()) {
        foreach(PropertyInterface* plugin, (*m_plugins)) {
            if (plugin->canHandle(object, property)) {
                pi = plugin->createInstance(object, property, this);
                break;
            }
        }
    }
    return pi;
}


PropertyInterface* PropertyModel::property(int number) const
{
    if (number >= m_properties.size())
        return 0;
    return m_properties[number];
}


void PropertyModel::setObject(QObject * object)
{
    beginResetModel();

    m_object = object;
    refreshProperties();

    endResetModel();
}

void PropertyModel::resetModel()
{
    beginResetModel();
    endResetModel();
}

QModelIndex PropertyModel::index(int row, int column, const QModelIndex & parent) const
{
    if (parent.isValid()) {
        if (row >= reinterpret_cast<PropertyInterface*>(parent.internalPointer())->rowCount(parent))
            return QModelIndex();
        reinterpret_cast<PropertyInterface*>(parent.internalPointer())->property(row)->setParentIndex(parent);
        QModelIndex nIndex = createIndex(row, column, reinterpret_cast<PropertyInterface*>(parent.internalPointer())->property(row));
        reinterpret_cast<PropertyInterface*>(parent.internalPointer())->property(row)->setIndex(nIndex);
        return nIndex;
    }
    if (row >= m_properties.size())
        return QModelIndex();
    if (0==column)
        m_properties[row]->setParentIndex(parent);
    QModelIndex nIndex = createIndex(row, column, m_properties[row]);
    m_properties[row]->setIndex(nIndex);
    return nIndex;
}


int PropertyModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return reinterpret_cast<PropertyInterface*>(parent.internalPointer())->rowCount(parent);
    return m_properties.size();
}

int PropertyModel::columnCount(const QModelIndex & parent) const
{
    if (parent.isValid())
        return reinterpret_cast<PropertyInterface*>(parent.internalPointer())->columnCount(parent);
    return 2;
}

void PropertyModel::setSizeHint(int s)
{
    m_sizeHint = s;
}

void PropertyModel::emitDataChangedSignal(const QModelIndex &index1, const QModelIndex &index2) const
{
    emit(const_cast<PropertyModel*>(this)->dataChanged(index1, index2));
}

QVariant PropertyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    switch (role) {
        case Qt::DisplayRole: return reinterpret_cast<PropertyInterface*>(index.internalPointer())->data(index);
        case Qt::SizeHintRole: {
                QFont font = data(index, Qt::FontRole).value<QFont>();
                QFontMetrics fm(font);
                int width = fm.width(reinterpret_cast<PropertyInterface*>(index.internalPointer())->data(index).toString());
                return QSize(width + 10,m_sizeHint);
            }
        case Qt::ToolTipRole:
            if (index.column() == 0)
                return reinterpret_cast<PropertyInterface*>(index.internalPointer())->data(index);
    }

    return QVariant();
}

bool PropertyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (Qt::EditRole != role)
        return false;
    bool ret = false;
    if (index.isValid() && Qt::DisplayRole == role)
        ret = reinterpret_cast<PropertyInterface*>(index.internalPointer())->setData(value, index);
    if (ret)
        emit dataChanged(index, index);
    return ret;
}

QModelIndex PropertyModel::parent(const QModelIndex & index) const
{
    if (index.isValid())
        return reinterpret_cast<PropertyInterface*>(index.internalPointer())->parentIndex(index);
    return QModelIndex();
}

Qt::ItemFlags PropertyModel::flags(const QModelIndex &index) const
{
  if (index.isValid()) {
      Qt::ItemFlags f = reinterpret_cast<PropertyInterface*>(index.internalPointer())->flags(index);
      if (m_readOnly)
          f &= ~Qt::ItemIsEditable;
      return f;
  }
   return Qt::NoItemFlags;
}

QVariant PropertyModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        switch (section)
        {
        case 0:
            return tr("Property");

        case 1:
            return tr("Value");

        default:
            return QVariant();
        }

    return QVariant();
}


QStringList PropertyModel::propertiesHidden()
{
    return m_object->property(QString("_properties_hidden").toLatin1()).toStringList();
}


}
