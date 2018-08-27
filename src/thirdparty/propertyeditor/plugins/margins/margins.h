/***************************************************************************
 *   This file is part of the propertyEditor project                       *
 *   Copyright (C) 2015 by Alexander Mikhalov                              *
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

#ifndef MARGINS_H
#define MARGINS_H

#include <propertyinterface.h>
#include <QMargins>

using namespace PropertyEditor;

/**
    @author Mikhalov Alexander <alexander.mikhalov@gmail.com>
*/

class Margins : public PropertyEditor::PropertyInterface
{
    Q_OBJECT
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "eu.licentia.PropertyEditor.PropetyInterface/1.0")
#endif
    Q_INTERFACES(PropertyEditor::PropertyInterface)

    Q_PROPERTY(qreal left READ left WRITE setLeft USER true)
    Q_PROPERTY(qreal top READ top WRITE setTop USER true)
    Q_PROPERTY(qreal bottom READ bottom WRITE setBottom USER true)
    Q_PROPERTY(qreal right READ right WRITE setRight USER true)

    Q_PROPERTY(int _current_property READ _currentProperty WRITE _setCurrentProperty DESIGNABLE false)
    Q_PROPERTY(int _current_property_precision READ _current_property_precision DESIGNABLE false)

public:
    Margins(QObject* parent = 0, QObject* object = 0, int property = -1, const PropertyModel * propertyModel = 0);
    QWidget* createEditor(QWidget * parent, const QModelIndex & index);

    QVariant data(const QModelIndex & index);
    bool setData(QVariant data, const QModelIndex & index);
    void setEditorData(QWidget * /*editor*/, const QModelIndex & /*index*/){}

    bool canHandle(QObject * object, int property)const;

    qreal left();
    void setLeft(qreal val);

    qreal top();
    void setTop(qreal top);

    qreal right();
    void setRight(qreal val);

    qreal bottom();
    void setBottom(qreal bottom);

    Qt::ItemFlags flags(const QModelIndex &index);

    PropertyInterface* createInstance(QObject * object, int property, const PropertyModel * propertyModel) const;

    void _setCurrentProperty(int num) {m_currentProperty = num;}
    int _currentProperty() { return m_currentProperty;}
    int _current_property_precision();

private:
    int m_currentProperty;
};

Q_DECLARE_METATYPE(QMargins)

#endif
