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
#include "alignmenteditor.h"
//#include "ui_alignmenteditor.h"

#include <QMetaProperty>
#include <QDebug>

AlignmentEditor::AlignmentEditor(QWidget *parent) :
    QToolBar(parent),
//    ui(new Ui::AlignmentEditor)
  m_doProcessControls(true)
{
//    m_setupUi(this);

    m_alignLeft = new QAction(QIcon(":/images/format-justify-left.png"), "Align Left", this);
    m_alignLeft->setCheckable(true);
    addAction(m_alignLeft);

    m_alignCenter = new QAction(QIcon(":/images/format-justify-center.png"), "Align Center", this);
    m_alignCenter->setCheckable(true);
    addAction(m_alignCenter);

    m_alignRight = new QAction(QIcon(":/images/format-justify-right.png"), "Align Right", this);
    m_alignRight->setCheckable(true);
    addAction(m_alignRight);

    m_alignJustify = new QAction(QIcon(":/images/format-justify-fill.png"), "Align Justify", this);
    m_alignJustify->setCheckable(true);
    addAction(m_alignJustify);

    addSeparator();

    m_alignTop = new QAction(QIcon(":/images/format-justify-top.png"), "Align Top", this);
    m_alignTop->setCheckable(true);
    addAction(m_alignTop);

    m_alignMiddle = new QAction(QIcon(":/images/format-justify-middle.png"), "Align Middle", this);
    m_alignMiddle->setCheckable(true);
    addAction(m_alignMiddle);

    m_alignBottom = new QAction(QIcon(":/images/format-justify-bottom.png"), "Align Bottom", this);
    m_alignBottom->setCheckable(true);
    addAction(m_alignBottom);

    addSeparator();

    m_wordWrap = new QAction(QIcon(":/images/word-wrap.png"), "Word Wrap", this);
    m_wordWrap->setCheckable(true);
    addAction(m_wordWrap);

    connect(m_alignLeft, SIGNAL(triggered(bool)), this, SLOT(actionTriggered()));
    connect(m_alignRight, SIGNAL(triggered(bool)), this, SLOT(actionTriggered()));
    connect(m_alignCenter, SIGNAL(triggered(bool)), this, SLOT(actionTriggered()));
    connect(m_alignJustify, SIGNAL(triggered(bool)), this, SLOT(actionTriggered()));
    connect(m_alignTop, SIGNAL(triggered(bool)), this, SLOT(actionTriggered()));
    connect(m_alignBottom, SIGNAL(triggered(bool)), this, SLOT(actionTriggered()));
    connect(m_alignMiddle, SIGNAL(triggered(bool)), this, SLOT(actionTriggered()));
    connect(m_wordWrap, SIGNAL(triggered(bool)), this, SLOT(actionTriggered()));

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
}


AlignmentEditor::~AlignmentEditor()
{
}


void AlignmentEditor::setObject(QObject *object)
{
    if (m_object == object)
        return;

    if (m_object)
        disconnect(m_object, 0, this, 0);

    m_object = object;
    m_objectList.clear();
}


void AlignmentEditor::setObjectList(const QObjectList &objects)
{
    setObject(0);
    m_objectList = objects;
}


void AlignmentEditor::setAlignPropertyName(const QString &propertyName)
{
    m_alignPropertyName = propertyName;
}


void AlignmentEditor::update()
{
    if (m_object) {

        QVariant valueVar = m_object ? m_object->property(m_alignPropertyName.toLatin1()) : QVariant();
        int value = (valueVar.isValid() && valueVar.canConvert<int>()) ? valueVar.value<int>() : 0;
        setAlignment(value);

        int indexFont = m_object->metaObject()->indexOfProperty(m_alignPropertyName.toLatin1());
        if (m_object->metaObject()->property(indexFont).hasNotifySignal()) {
            QMetaMethod signal = m_object->metaObject()->property(indexFont).notifySignal();
            QMetaMethod slot = this->metaObject()->method(this->metaObject()->indexOfSlot("alignmentFromObjectProperty()"));
            connect(m_object, signal, this, slot, Qt::UniqueConnection);
        }

    }  else if (m_objectList.count()) {

        bool alignFound = false;
        int value = 0;

        foreach(QObject * object, m_objectList) {
            QVariant alignValue = object->property(m_alignPropertyName.toLatin1());
            if (alignValue.isValid() && alignValue.canConvert<int>()) {
                int a = alignValue.value<int>();
                if (!alignFound) {
                    value = a;
                    alignFound = true;
                } else {
                    if ((a & Qt::AlignLeft) != (value & Qt::AlignLeft))
                        value &= ~(Qt::AlignLeft);
                    if ((a & Qt::AlignRight) != (value & Qt::AlignRight))
                        value &= ~(Qt::AlignRight);
                    if ((a & Qt::AlignHCenter) != (value & Qt::AlignHCenter))
                        value &= ~(Qt::AlignHCenter);
                    if ((a & Qt::AlignJustify) != (value & Qt::AlignJustify))
                        value &= ~(Qt::AlignJustify);
                    if ((a & Qt::AlignTop) != (value & Qt::AlignTop))
                        value &= ~(Qt::AlignTop);
                    if ((a & Qt::AlignBottom) != (value & Qt::AlignBottom))
                        value &= ~(Qt::AlignBottom);
                    if ((a & Qt::AlignVCenter) != (value & Qt::AlignVCenter))
                        value &= ~(Qt::AlignVCenter);
                    if ((a & Qt::TextWordWrap) != (value & Qt::TextWordWrap))
                        value &= ~(Qt::TextWordWrap);
                }
            }
        }

        setAlignment(value);

    } else {
        clear();
    }
}


void AlignmentEditor::clear()
{
    if (m_object)
        disconnect(m_object, 0, this, 0);
    m_object = 0;
    m_objectList.clear();
    m_alignPropertyName = QString();
    setAlignment(0);
}


void AlignmentEditor::setAlignment(int alignment)
{
    m_doProcessControls = false;
    m_alignLeft->setChecked(alignment & Qt::AlignLeft);
    m_alignRight->setChecked(alignment & Qt::AlignRight);
    m_alignCenter->setChecked(alignment & Qt::AlignHCenter);
    m_alignJustify->setChecked(alignment & Qt::AlignJustify);
    m_alignTop->setChecked(alignment & Qt::AlignTop);
    m_alignBottom->setChecked(alignment & Qt::AlignBottom);
    m_alignMiddle->setChecked(alignment & Qt::AlignVCenter);
    m_wordWrap->setChecked(alignment & Qt::TextWordWrap);
    m_doProcessControls = true;
}


int AlignmentEditor::alignment()
{
//    QVariant valueVar = m_object ? m_object->property(m_alignPropertyName.toLatin1()) : QVariant();
//    int value = valueVar.value<int>();
    int value = 0;
    if (m_alignLeft->isChecked()) value |= Qt::AlignLeft; //else value &= (~Qt::AlignLeft);
    if (m_alignRight->isChecked()) value |= Qt::AlignRight; //else value &= (~Qt::AlignRight);
    if (m_alignCenter->isChecked()) value |= Qt::AlignHCenter; //else value &= (~Qt::AlignHCenter);
    if (m_alignJustify->isChecked()) value |= Qt::AlignJustify; //else value &= (~Qt::AlignJustify);
    if (m_alignTop->isChecked()) value |= Qt::AlignTop; //else value &= (~Qt::AlignTop);
    if (m_alignBottom->isChecked()) value |= Qt::AlignBottom; //else value &= (~Qt::AlignBottom);
    if (m_alignMiddle->isChecked()) value |= Qt::AlignVCenter; //else value &= (~Qt::AlignVCenter);
    if (m_wordWrap->isChecked()) value |= Qt::TextWordWrap; //else value &= (~Qt::TextWordWrap);

    return value;
}


void AlignmentEditor::alignmentToObjectProperty()
{
    if ((!m_object && m_objectList.isEmpty()) || !m_doProcessControls)
        return;

    int a = alignment();

    if (m_object)
        m_object->setProperty(m_alignPropertyName.toLatin1(), a);
    if (!m_objectList.isEmpty())
        foreach (QObject * object, m_objectList)
            object->setProperty(m_alignPropertyName.toLatin1(), a);
}


void AlignmentEditor::alignmentFromObjectProperty()
{
    if (!m_object)
        return;

    QVariant valueVar = m_object ? m_object->property(m_alignPropertyName.toLatin1()) : QVariant();
    int value = valueVar.value<int>();
    setAlignment(value);
}


void AlignmentEditor::actionTriggered()
{
    QAction * action = dynamic_cast<QAction*>(sender());
    if (!action)
        return;

    if (action == m_alignLeft || action == m_alignRight || action == m_alignCenter || action == m_alignJustify) {
        if (action != m_alignLeft) m_alignLeft->setChecked(false);
        if (action != m_alignRight) m_alignRight->setChecked(false);
        if (action != m_alignCenter) m_alignCenter->setChecked(false);
        if (action != m_alignJustify) m_alignJustify->setChecked(false);
        action->setChecked(true);
    }  else if (action == m_alignTop || action == m_alignBottom || action == m_alignMiddle) {
        if (action != m_alignTop) m_alignTop->setChecked(false);
        if (action != m_alignBottom) m_alignBottom->setChecked(false);
        if (action != m_alignMiddle) m_alignMiddle->setChecked(false);
        action->setChecked(true);
    } /*else if (button == m_wordWrap)
        button->setChecked(!button->isChecked());*/

    alignmentToObjectProperty();
}
