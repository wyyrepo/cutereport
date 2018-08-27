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

#ifndef GRADIENT_H
#define GRADIENT_H

#include <propertyinterface.h>
#include <QGradient>

using namespace PropertyEditor;


//struct GradientData
//{
//    explicit GradientData() : type(QGradient::NoGradient){}
//    explicit GradientData(QGradient::Type _type) : type(_type){}

//    QGradient::Type type;
//    QList<QGradientStop> stops;
//};


class Gradient : public PropertyEditor::PropertyInterface
{
    Q_OBJECT
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "eu.licentia.PropertyEditor.PropetyInterface/1.0")
#endif
    Q_INTERFACES(PropertyEditor::PropertyInterface)

public:
    Gradient(QObject* parent = 0, QObject* object = 0, int property = -1, const PropertyModel * propertyModel = 0);
    QWidget* createEditor(QWidget * parent, const QModelIndex & index);

    QVariant data(const QModelIndex & index);
    bool setData(QVariant data, const QModelIndex & index);
    void setEditorData(QWidget * editor, const QModelIndex & index);

    bool canHandle(QObject * object, int property)const;

    PropertyInterface* createInstance(QObject * object, int property, const PropertyModel * propertyModel) const;

public slots:
    void setValue(const QGradient &g);
};

Q_DECLARE_METATYPE(QGradient)

#endif
