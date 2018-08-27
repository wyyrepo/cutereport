/***************************************************************************
 *   This file is part of the eXaro project                                *
 *   Copyright (C) 2008 by BogDan Vatra                                    *
 *   bog_dan_ro@yahoo.com                                                  *
 **                   GNU General Public License Usage                    **
 *   Copyright (C) 2016-2017 by Mikhalov Alexander                              *
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
 *   it under the terms of the GNU Lesser General Public License as        *.
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library.                                      *.
 *   If not, see <http://www.gnu.org/licenses/>.                           *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ****************************************************************************/

#ifndef OBJECTINSPECTOR_H
#define OBJECTINSPECTOR_H

#include <QTreeView>
#include "widgets_thirdparty_export.h"

class ObjectModel;

class WIDGET_THIRDPARTY_EXPORTS ObjectInspector : public QTreeView
{
    Q_OBJECT
public:
    explicit ObjectInspector(QWidget *parent = 0);
	~ObjectInspector();
    QObjectList selectedObjects();

signals:
    void objectChanged( QObject * object);
    void selectionChanged();
    void deletePressed();

public slots:
	void setRootObject( QObject * rootObject);
	void selectObject( QObject * object, QItemSelectionModel::SelectionFlag selFlag = QItemSelectionModel::ClearAndSelect);

private slots:
    void slotObjectChanged(const QModelIndex & index, const QModelIndex & );
    void slotSelectionChanged(QItemSelection, QItemSelection);

private:
	bool selectObject(QObject * object, QModelIndex index, QItemSelectionModel::SelectionFlag selFlag = QItemSelectionModel::ClearAndSelect);
    virtual void keyPressEvent ( QKeyEvent * event );

private:
	ObjectModel * m_objectModel;
	QItemSelectionModel *m_selectionModel;
};

#endif // OBJECTINSPECTOR_H
