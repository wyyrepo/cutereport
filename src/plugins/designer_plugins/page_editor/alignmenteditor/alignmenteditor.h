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
#ifndef ALIGNMENTEDITOR_H
#define ALIGNMENTEDITOR_H

#include <QToolBar>
#include <QPointer>
#include <QAction>


class AlignmentEditor : public QToolBar
{
    Q_OBJECT

public:
    explicit AlignmentEditor(QWidget *parent = 0);
    ~AlignmentEditor();

    void setObject(QObject * object);
    void setObjectList(const QObjectList &objects);
    void setAlignPropertyName(const QString & propertyName);
    void update();
    void clear();
    QString alignPropertyName();

    void setAlignment(int alignment);
    int alignment();

private slots:
    void alignmentToObjectProperty();
    void alignmentFromObjectProperty();
    void actionTriggered();

private:
    QAction * m_alignLeft;
    QAction * m_alignCenter;
    QAction * m_alignRight;
    QAction * m_alignJustify;
    QAction * m_alignTop;
    QAction * m_alignMiddle;
    QAction * m_alignBottom;
    QAction * m_wordWrap;

    QPointer<QObject> m_object;
    QObjectList m_objectList;
    QString m_alignPropertyName;
    bool m_doProcessControls;
};

#endif // ALIGNMENTEDITOR_H
