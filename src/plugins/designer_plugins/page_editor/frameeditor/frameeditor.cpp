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

#include "frameeditor.h"
#include "cutereport_types.h"

#include <QMetaProperty>
#include <QPen>

FrameEditor::FrameEditor(QWidget *parent) :
    QToolBar(parent)
{
    m_frameLeft = new QAction(QIcon(":/images/frame_left.png"), "Left Frame", this);
    m_frameLeft->setCheckable(true);
    addAction(m_frameLeft);

    m_frameRight = new QAction(QIcon(":/images/frame_right.png"), "Right Frame", this);
    m_frameRight->setCheckable(true);
    addAction(m_frameRight);

    m_frameTop = new QAction(QIcon(":/images/frame_top.png"), "Top Frame", this);
    m_frameTop->setCheckable(true);
    addAction(m_frameTop);

    m_frameBottom = new QAction(QIcon(":/images/frame_bottom.png"), "Bottom Frame", this);
    m_frameBottom->setCheckable(true);
    addAction(m_frameBottom);

    m_frameAll = new QAction(QIcon(":/images/frame_all.png"), "Full Frame", this);
    addAction(m_frameAll);

    m_frameNone = new QAction(QIcon(":/images/frame_none.png"), "No Frame", this);
    addAction(m_frameNone);

    m_penWidth = new QDoubleSpinBox(this);
    m_penWidth->setDecimals(2);
    m_penWidth->setMaximum(10);
    m_penWidth->setSingleStep(0.1);
    m_penWidth->setToolTip("Frame width in mm");
//    m_penWidth->setSuffix(" mm");
    addWidget(m_penWidth);

    connect(m_frameLeft, SIGNAL(toggled(bool)), this, SLOT(actionToggled()));
    connect(m_frameRight, SIGNAL(toggled(bool)), this, SLOT(actionToggled()));
    connect(m_frameTop, SIGNAL(toggled(bool)), this, SLOT(actionToggled()));
    connect(m_frameBottom, SIGNAL(toggled(bool)), this, SLOT(actionToggled()));
    connect(m_frameAll, SIGNAL(triggered(bool)), this, SLOT(actionToggled()));
    connect(m_frameNone, SIGNAL(triggered(bool)), this, SLOT(actionToggled()));

    connect(m_penWidth, SIGNAL(valueChanged(double)), this, SLOT(penWidthChanged()));

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
}


FrameEditor::~FrameEditor()
{
}


void FrameEditor::setObject(QObject *object)
{
    if (m_object == object)
        return;

    if (m_object)
        disconnect(m_object, 0, this, 0);

    m_object = object;
    m_objectList.clear();
}


void FrameEditor::setObjectList(const QObjectList &objects)
{
    setObject(0);
    m_objectList = objects;
}


void FrameEditor::setFramePropertyName(const QString &propertyName)
{
    m_framePropertyName = propertyName;
}


void FrameEditor::setPenPropertyName(const QString &propertyName)
{
    m_penPropertyName = propertyName;
}


void FrameEditor::update()
{

    if (m_object) {
        QVariant frameValueVar = m_object ? m_object->property(m_framePropertyName.toLatin1()) : QVariant();
        int valueFrame = (frameValueVar.isValid() && frameValueVar.canConvert<int>()) ? frameValueVar.value<int>() : 0;
        setFrame(valueFrame);

        int indexFrame = m_object->metaObject()->indexOfProperty(m_framePropertyName.toLatin1());
        if (m_object->metaObject()->property(indexFrame).hasNotifySignal()) {
            QMetaMethod signal = m_object->metaObject()->property(indexFrame).notifySignal();
            QMetaMethod slot = this->metaObject()->method(this->metaObject()->indexOfSlot("frameFromObjectProperty()"));
            connect(m_object, signal, this, slot, Qt::UniqueConnection);
        }

        QVariant penValueVar = m_object ? m_object->property(m_penPropertyName.toLatin1()) : QVariant();
        QPen valuePen = (penValueVar.isValid() && penValueVar.canConvert<QPen>()) ? penValueVar.value<QPen>() : QPen();
        setPen(valuePen, m_object);

        int indexPen = m_object->metaObject()->indexOfProperty(m_penPropertyName.toLatin1());
        if (m_object->metaObject()->property(indexPen).hasNotifySignal()) {
            QMetaMethod signal = m_object->metaObject()->property(indexPen).notifySignal();
            QMetaMethod slot = this->metaObject()->method(this->metaObject()->indexOfSlot("penFromObjectProperty()"));
            connect(m_object, signal, this, slot, Qt::UniqueConnection);
        }
    } else if (m_objectList.count()) {

        bool frameFound = false;
        int value = 0;

        foreach(QObject * object, m_objectList) {
            QVariant frameValue = object->property(m_framePropertyName.toLatin1());
            if (frameValue.isValid() && frameValue.canConvert<int>()) {
                int f = frameValue.value<int>();
                if (!frameFound) {
                    value = f;
                    frameFound = true;
                } else {
                    if ((f & CuteReport::DrawLeft) != (value & CuteReport::DrawLeft))
                        value &= ~(CuteReport::DrawLeft);
                    if ((f & CuteReport::DrawRight) != (value & CuteReport::DrawRight))
                        value &= ~(CuteReport::DrawRight);
                    if ((f & CuteReport::DrawTop) != (value & CuteReport::DrawTop))
                        value &= ~(CuteReport::DrawTop);
                    if ((f & CuteReport::DrawBottom) != (value & CuteReport::DrawBottom))
                        value &= ~(CuteReport::DrawBottom);
                }
            }
        }

        setFrame(value);

        for (int i = m_objectList.size()-1; i >=0; --i) {
            QObject * object = m_objectList.at(i);
            QVariant penValueVar = object->property(m_penPropertyName.toLatin1());
            QPen valuePen = (penValueVar.isValid() && penValueVar.canConvert<QPen>()) ? penValueVar.value<QPen>() : QPen();
            setPen(valuePen, object);
        }

    } else {
        clear();
    }
}


void FrameEditor::clear()
{
    if (m_object)
        disconnect(m_object, 0, this, 0);
    m_object = 0;
    m_objectList.clear();
    m_framePropertyName = QString();
    setFrame(0);
}


void FrameEditor::setFrame(int frame)
{
    m_doProcessControls = false;
    m_frameLeft->setChecked(frame & CuteReport::DrawLeft);
    m_frameRight->setChecked(frame & CuteReport::DrawRight);
    m_frameTop->setChecked(frame & CuteReport::DrawTop);
    m_frameBottom->setChecked(frame & CuteReport::DrawBottom);
    m_doProcessControls = true;
}


int FrameEditor::frame()
{
//    QVariant valueVar = m_object ? m_object->property(m_framePropertyName.toLatin1()) : QVariant();
//    int value = valueVar.value<int>();
    int value = 0;
    if (m_frameLeft->isChecked()) value |= CuteReport::DrawLeft;// else value &= (~CuteReport::DrawLeft);
    if (m_frameRight->isChecked()) value |= CuteReport::DrawRight;// else value &= (~CuteReport::DrawRight);
    if (m_frameTop->isChecked()) value |= CuteReport::DrawTop;// else value &= (~CuteReport::DrawTop);
    if (m_frameBottom->isChecked()) value |= CuteReport::DrawBottom;// else value &= (~CuteReport::DrawBottom);

    return value;
}


void FrameEditor::setPen(const QPen & pen, QObject *object)
{
    m_origPen.insert(object, pen);
    m_penWidth->setValue(pen.widthF());
//    bool equal = true;
//    QHashIterator<QObject *, QPen> i(m_origPen);
//    while (i.hasNext()) {
//        i.next();
//        if (i.value().widthF() != pen.widthF()) {
//            equal = false;
//            break;
//        }
//    }
//    m_penWidth->setValue(equal ? pen.widthF() : 0);
}


QPen FrameEditor::pen(QObject * object)
{
    QPen origPen = m_origPen.value(object);
    origPen.setWidthF(m_penWidth->value());
    return origPen;
}



void FrameEditor::frameToObjectProperty()
{
    if ((!m_object && m_objectList.isEmpty()) || !m_doProcessControls)
        return;

    int f = frame();

    if (m_object)
        m_object->setProperty(m_framePropertyName.toLatin1(), f);
    if (!m_objectList.isEmpty())
        foreach (QObject * object, m_objectList)
            object->setProperty(m_framePropertyName.toLatin1(), f);
}


void FrameEditor::frameFromObjectProperty()
{
    if (!m_object)
        return;

    QVariant valueVar = m_object ? m_object->property(m_framePropertyName.toLatin1()) : QVariant();
    int value = valueVar.value<int>();
    setFrame(value);
}


void FrameEditor::penToObjectProperty()
{
    if ((!m_object && m_objectList.isEmpty()) || !m_doProcessControls)
        return;

    if (m_object) {
        QPen p = pen(m_object);
        m_object->setProperty(m_penPropertyName.toLatin1(), p);
    }
    if (!m_objectList.isEmpty())
        foreach (QObject * object, m_objectList) {
            QPen p = pen(object);
            object->setProperty(m_penPropertyName.toLatin1(), p);
        }
}


void FrameEditor::penFromObjectProperty()
{
    if (!m_object)
        return;

    QVariant valueVar = m_object ? m_object->property(m_penPropertyName.toLatin1()) : QVariant();
    QPen value = valueVar.value<QPen>();
    setPen(value, m_object);
}


void FrameEditor::actionToggled()
{
    QAction * action = dynamic_cast<QAction*>(sender());
    if (!action)
        return;

    if (action == m_frameAll || action == m_frameNone) {
        m_frameLeft->setChecked(action == m_frameAll);
        m_frameRight->setChecked(action == m_frameAll);
        m_frameTop->setChecked(action == m_frameAll);
        m_frameBottom->setChecked(action == m_frameAll);
    }

    frameToObjectProperty();
}


void FrameEditor::penWidthChanged()
{
    penToObjectProperty();
}
