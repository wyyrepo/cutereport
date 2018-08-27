/***************************************************************************
 *   This file is part of the propertyEditor project                       *
 *   Copyright (C) 2016-2017 by Alexander Mikhalov                              *
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
 ****************************************************************************/

#include <QtCore>
#include <QMetaProperty>
#include <QMetaEnum>

#include "changegradient.h"
#include "gradient.h"

Gradient::Gradient(QObject* parent, QObject* object, int property, const PropertyModel* propertyModel): PropertyInterface(parent, object, property, propertyModel)
{
    //qRegisterMetaType<GradientData>();
}


QWidget* Gradient::createEditor(QWidget * parent, const QModelIndex & index)
{
	Q_UNUSED(index);
    ChangeGradient *cf = new ChangeGradient(parent);
    connect(cf, SIGNAL(gradientChanged(const QGradient&)), this, SLOT(setValue(const QGradient&)));
	return cf;
}


void Gradient::setValue(const QGradient &g)
{
    PropertyInterface::setValue(QVariant::fromValue(g));
}


QVariant Gradient::data(const QModelIndex & index)
{
	if (!index.isValid() || !object() || -1 == objectProperty())
		return QVariant();
	switch (index.column())
	{
		case 0:
			return object()->metaObject()->property(objectProperty()).name();
		case 1:
			return value();
	}
	return QVariant();
}


bool Gradient::setData(QVariant data, const QModelIndex & index)
{
	Q_UNUSED(index);
	return PropertyInterface::setValue(data);
}


void Gradient::setEditorData(QWidget * editor, const QModelIndex & /*index*/)
{
	QVariant f;
    dynamic_cast<ChangeGradient *>(editor)->setGradient(value().value<QGradient>());
}


bool Gradient::canHandle(QObject * object, int property)const
{
	if (object->metaObject()->property(property).isEnumType() || object->metaObject()->property(property).isFlagType())
		return false;

    QVariant value = object->property(object->metaObject()->property(property).name());
    return value.canConvert<QGradient>();
}


PropertyInterface* Gradient::createInstance(QObject * object, int property, const PropertyModel * propertyModel) const
{
    return new Gradient(parent(), object, property, propertyModel);
}


#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(GradientProperty, Gradient)
#endif
