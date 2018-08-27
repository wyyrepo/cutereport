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

#include <QtCore>
#include <QMetaProperty>
#include <QMetaEnum>

#include "margins.h"

Margins::Margins(QObject* parent, QObject* object, int property, const PropertyModel* propertyModel)
    : PropertyInterface(parent, object, property, propertyModel),
      m_currentProperty(-1)
{
	refreshPropertis(propertyModel);
}


QWidget* Margins::createEditor(QWidget * parent, const QModelIndex & index)
{
	Q_UNUSED(index);
	Q_UNUSED(parent);
	return 0;
}

QVariant Margins::data(const QModelIndex & index)
{
	switch (index.column())
	{
		case 0:
			return object()->metaObject()->property(objectProperty()).name();
        case 1: {
            int precision = propertyPrecision();
            QMargins margins = value().value<QMargins>();
            return QString("[L:%1 T:%2 R:%3 B:%4]").arg(margins.left(), 0, 'f', precision).arg(margins.top(), 0, 'f', precision)
                .arg(margins.right(), 0, 'f', precision).arg(margins.bottom(), 0, 'f', precision);
            }
	}
	return QVariant();
}

bool Margins::setData(QVariant data, const QModelIndex & index)
{
	Q_UNUSED(data);
	Q_UNUSED(index);
	return false;
}

bool Margins::canHandle(QObject * object, int property) const
{
	if (object->metaObject()->property(property).isEnumType() || object->metaObject()->property(property).isFlagType())
		return false;

    QString typeName = object->metaObject()->property(property).typeName();
    return (typeName == "QMargins");
//    QString name = object->metaObject()->property(property).name();
//    QVariant value = object->property(object->metaObject()->property(property).name());
//    qDebug() << value.typeName();
//    if (value.canConvert<QMargins>())
//        return true;
//    else
//        return false;

//    return false;

//    if ()
//	switch (object->property(object->metaObject()->property(property).name()).type())
//	{
//        case QVariant::Margins:
//			return true;
//		default:
//			return false;
//	}
//	return false;
}


qreal Margins::left()
{
    QMargins margins = value().value<QMargins>();
    return margins.left();
}


void Margins::setLeft(qreal val)
{
    QMargins s = value().value<QMargins>();
    s.setLeft(val);
    setValue(QVariant::fromValue(s));
}

qreal Margins::top()
{
    QMargins margins = value().value<QMargins>();
    return margins.top();
}


void Margins::setTop(qreal val)
{
    QMargins s = value().value<QMargins>();
    s.setTop(val);
    setValue(QVariant::fromValue(s));
}


qreal Margins::right()
{
    QMargins margins = value().value<QMargins>();
    return margins.right();
}


void Margins::setRight(qreal val)
{
    QMargins s = value().value<QMargins>();
    s.setRight(val);
    setValue(QVariant::fromValue(s));
}


qreal Margins::bottom()
{
    QMargins margins = value().value<QMargins>();
    return margins.bottom();
}


void Margins::setBottom(qreal val)
{
    QMargins s = value().value<QMargins>();
    s.setBottom(val);
    setValue(QVariant::fromValue(s));
}


Qt::ItemFlags Margins::flags(const QModelIndex &/*index*/)
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

PropertyInterface* Margins::createInstance(QObject * object, int property, const PropertyModel * propertyModel) const
{
    return new Margins(parent(), object, property, propertyModel);
}


int Margins::_current_property_precision()
{
    object()->setProperty("_current_property", objectProperty());
    return object()->property("_current_property_precision").toInt();
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(MarginsProperty, Margins)
#endif
