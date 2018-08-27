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

#include <QtCore>
#include <QMetaProperty>
#include <QMetaEnum>

#include "cr_margins_prop.h"
#include "cutereport_types.h"

CR_MarginsProp::CR_MarginsProp(QObject* parent, QObject* object, int property, const PropertyModel * propertyModel): PropertyInterface(parent, object, property, propertyModel)
{
    refreshPropertis(propertyModel);
}


QWidget* CR_MarginsProp::createEditor(QWidget * parent, const QModelIndex & index)
{
    Q_UNUSED(index);
    Q_UNUSED(parent);
    return 0;
}

QVariant CR_MarginsProp::data(const QModelIndex & index)
{
    switch (index.column())
    {
        case 0:
            return object()->metaObject()->property(objectProperty()).name();
        case 1:
            int precision = propertyPrecision();
            CuteReport::Margins margins = value().value<CuteReport::Margins>();
            return QString("[L:%1 T:%2 R:%3 B:%4]").arg(margins.left(), 0, 'f', precision).arg(margins.top(), 0, 'f', precision)
                    .arg(margins.right(), 0, 'f', precision).arg(margins.bottom(), 0, 'f', precision);
    }
    return QVariant();
}

bool CR_MarginsProp::setData(QVariant data, const QModelIndex & index)
{
    Q_UNUSED(index);
    Q_UNUSED(data);
    //return PropertyInterface::setValue(data);
    return false;
}


bool CR_MarginsProp::canHandle(QObject * object, int property) const
{
    if (object->metaObject()->property(property).isEnumType() || object->metaObject()->property(property).isFlagType())
        return false;
    return object->property(object->metaObject()->property(property).name()).canConvert<CuteReport::Margins>();
}


qreal CR_MarginsProp::left()
{
    return value().value<CuteReport::Margins>().left();
}


qreal CR_MarginsProp::top()
{
    return value().value<CuteReport::Margins>().top();
}


qreal CR_MarginsProp::right()
{
    return value().value<CuteReport::Margins>().right();
}


qreal CR_MarginsProp::bottom()
{
    return value().value<CuteReport::Margins>().bottom();
}


void CR_MarginsProp::setLeft(qreal left)
{
    CuteReport::Margins m = value().value<CuteReport::Margins>();
    m.setLeft(left);
    QVariant v;
    v.setValue(m);
    setValue(v);
}


void CR_MarginsProp::setTop(qreal top)
{
    CuteReport::Margins m = value().value<CuteReport::Margins>();
    m.setTop(top);
    QVariant v;
    v.setValue(m);
    setValue(v);
}


void CR_MarginsProp::setRight(qreal right)
{
    CuteReport::Margins m = value().value<CuteReport::Margins>();
    m.setRight(right);
    QVariant v;
    v.setValue(m);
    setValue(v);
}


void CR_MarginsProp::setBottom(qreal bottom)
{
    CuteReport::Margins m = value().value<CuteReport::Margins>();
    m.setBottom(bottom);
    QVariant v;
    v.setValue(m);
    setValue(v);
}


Qt::ItemFlags CR_MarginsProp::flags(const QModelIndex &/*index*/)
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}


PropertyInterface* CR_MarginsProp::createInstance(QObject * object, int property, const PropertyModel * propertyModel) const
{
    return new CR_MarginsProp(parent(), object, property, propertyModel);
}


#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(CRMarginsProperty, CR_MarginsProp)
#endif
