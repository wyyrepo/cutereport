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
 ****************************************************************************/
#ifndef BASEITEMINTERFACE_H
#define BASEITEMINTERFACE_H

#include <QObject>
#include <QtCore>
#include <QGraphicsWidget>
#include <QPen>
#include <QWidget>
#include <QPolygonF>
#include <QIcon>

#include "reportplugininterface.h"
#include "cutereport_types.h"
#include "cutereport_functions.h"
#include "cutereport_globals.h"

#define ITEMVIEWTYPE QGraphicsItem::UserType + 101

#define SAVEIFCHANGED(OLD, NEW) \
    if (d-> OLD == NEW) \
        return; \
    d-> OLD = NEW;

#define PROCESS_NEW_PROPERTY_VALUE(PropertyName, VALUE) \
    if (d-> PropertyName == VALUE) \
        return; \
    d-> PropertyName = VALUE; \
    if (d->state == CuteReport::NormalState) { \
        emit PropertyName##Changed ( d-> PropertyName ); \
        emit changed(); \
    }

#define PROCESS_NEW_PROPERTY_VALUE_WITH_GUI_UPDATE(PropertyName, VALUE) \
    if (d-> PropertyName == VALUE) \
        return; \
    d-> PropertyName = VALUE; \
    update_gui(); \
    if (d->state == CuteReport::NormalState) { \
        emit PropertyName##Changed ( d-> PropertyName ); \
        emit changed(); \
    }

#define PROPERTY_CHANGE_SIGNALS(PropertyName) \
    if (d->state == CuteReport::NormalState) { \
        emit PropertyName##Changed ( d-> PropertyName ); \
        emit changed(); \
    }

#define NOTPROPERTY_CHANGE_SIGNALS(SIGNALNAME, VARIABLE) \
    if (d->state == NormalState) { \
        emit SIGNALNAME ( VARIABLE ); \
        emit changed(); \
    }





class QScriptEngine;

namespace CuteReport
{

class DesignerItemInterface;
class PageInterface;
class BaseItemHelperInterface;
class BaseItemInterfacePrivate;
class RenderedItemInterfacePrivate;
class ItemInterfaceView;
class RendererInterface;
class RenderedItemInterface;
class RendererPublicInterface;
class ReportInterface;
class ItemExternalDataInterface;
class ScriptEngineInterface;


class CUTEREPORT_EXPORTS BaseItemInterface : public ReportPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(CuteReport::ReportPluginInterface)

    Q_FLAGS(Frames)
    Q_ENUMS(Frame ResizeFlags BorderType BaseItemFlag)

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QRectF geometry READ geometry WRITE setGeometry NOTIFY geometryChanged)
//    Q_PROPERTY(QRectF boundingRect READ boundingRect NOTIFY boundingRectChanged) //if rotated
    Q_PROPERTY(Frames frame READ frame WRITE setFrame NOTIFY frameChanged)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
    Q_PROPERTY(QPen borderPen READ borderPen WRITE setBorderPen NOTIFY borderPenChanged)
    Q_PROPERTY(QString borderType READ borderTypeStr WRITE setBorderTypeStr NOTIFY borderTypeChanged)
    Q_PROPERTY(QBrush backgroundBrush READ backgroundBrush WRITE setBackgroundBrush NOTIFY backgroundBrushChanged)
    Q_PROPERTY(int order READ order WRITE setOrder NOTIFY orderChanged)
    Q_PROPERTY(int baseItemFlags READ baseItemFlags WRITE setBaseItemFlags NOTIFY baseItemFlagsChanged DESIGNABLE false)

    Q_PROPERTY(BorderType borderType READ borderType WRITE setBorderType NOTIFY borderTypeChanged DESIGNABLE false STORED false)
    Q_PROPERTY(QStringList _borderType_variants READ _borderType_variants DESIGNABLE false STORED false)

    Q_PROPERTY(int _current_property READ _currentProperty WRITE _setCurrentProperty DESIGNABLE false STORED false)
    Q_PROPERTY(QString _current_property_description READ _current_property_description DESIGNABLE false STORED false)
    Q_PROPERTY(int _current_property_precision READ _current_property_precision DESIGNABLE false STORED false)
    Q_PROPERTY(QIcon _icon READ itemIcon DESIGNABLE false STORED false)

public:
    enum Frame {DrawLeft = CuteReport::DrawLeft, /**< Draw left frame*/
                DrawRight = CuteReport::DrawRight, /**< Draw right frame*/
                DrawTop = CuteReport::DrawTop, /**< Draw top frame*/
                DrawBottom = CuteReport::DrawBottom /**< Draw bottom frame*/
               };
    Q_DECLARE_FLAGS(Frames, Frame)

    enum ResizeFlags {Fixed = 0, /**< Item cant be resized*/
                      ResizeLeft = 1, /**< Item can be resized to left*/
                      ResizeRight = 2, /**< Item can be resized to right*/
                      ResizeTop = 4, /**< Item can be resized to top*/
                      ResizeBottom = 8, /**< Item can be resized to bottom*/
                      FixedPos = 16 /**< Item cant be moved*/
                     };

    enum BorderType { Middle = 0, Inner = 1, Outer = 2 };

    enum BaseItemFlag {
        BaseReportItem  = 0x0001,            /** item is integrated from base report and cannot be changed */
        LockedItem    = 0x0002,              /** item cannot be edited*/
        AutoRenamedFromBaseReport = 0x0004   /** item is renamed in range of inheritance */
//        CanExtendWidthOnRendering = 0x0004,
//        CanExtendHeightOnRendering = 0x0008,
    };

    explicit BaseItemInterface(QObject * parent);
    virtual ~BaseItemInterface();

    /** this method used for register metatypes, etc. It should be called only once */
    virtual bool moduleInit();

    /** every new created item should be inited using this method */
    virtual void init();

    virtual void check_gui();
    virtual void update_gui();
    virtual void init_gui() = 0;

    virtual BaseItemInterface * createInstance(QObject * parent) const = 0;
    virtual BaseItemInterface * clone(bool withChildren = true, bool init = true) const;
    virtual ItemExternalDataInterface * createDataObjectByTypeName(const QString & className) {Q_UNUSED(className); return 0;}

    virtual PageInterface * page() const;
    virtual void setPage(PageInterface * page);
    virtual BaseItemInterface * parentItem() const;
    virtual void setParentItem(BaseItemInterface *parentItem);
    CuteReport::ReportInterface * reportObject();
    virtual int childLevel();
    CuteReport::BandInterface * carrierBand();

    virtual BaseItemInterface * topmostChildAt(const QPointF & localPos);
    virtual QList<BaseItemInterface *> childrenAt(const QPointF & localPos);
    QList<BaseItemInterface *> allChildrenAt(const QPointF & localPos);

    virtual BaseItemHelperInterface * createHelper(CuteReport::DesignerItemInterface * designer) { Q_UNUSED(designer); return 0; }
    virtual QByteArray serialize();
    virtual void deserialize(QByteArray &data);

    virtual bool canContain(QObject * object);
    virtual ItemInterfaceView * view();
//    virtual void updateView();

    virtual void saveState();
    virtual void restoreState();

    /** method called when renderer starts its work, nothing rendered yet */
    virtual void renderInit(CuteReport::ScriptEngineInterface *scriptEngine, CuteReport::InitFlags flags) = 0;

    /** method called when renderer has completed its work, all is already rendered */
    virtual void renderReset();

    /** method is called to calculate rendered item data, replace internal item's data pointer with the
     * new rendered data and return rendered itemView if needed
     * return bool: true - item is printable; false - item is not printable
     */
    virtual bool renderBegin() = 0;

    /** render image to print */
    virtual RenderedItemInterface * renderView(quint32 id) = 0;

    /** method called after rendered item's geometry and position was managed and became final
     * Every item must implement this method, return back correct internal data pointer and send printAfter signal
     */
    virtual bool renderEnd();

    virtual void renderPassStart() {}
    virtual void renderPassEnd() {}
    virtual void renderCannotFitToPage() {}

    /** method called before items on the same level (have the same parent) will be rendered */
    virtual void renderSiblingsBefore(QList<BaseItemInterface*> siblings) {Q_UNUSED(siblings);}
    /** method called after items on the same level (have the same parent) have rendered */
    virtual void renderSiblingsAfter(QList<BaseItemInterface*> siblings) {Q_UNUSED(siblings);}

    /** item provides its own geometry logic so renderer will net change it */
    virtual bool selfRendering();

    /** item renders its children by itself. Renderer will not touch children */
    virtual bool childrenSelfRendering();

    virtual QIcon itemIcon() const = 0;
    virtual QString itemGroup() const = 0;
    virtual int itemSortOrderInGroup() const {return 0;}

    virtual Unit unit() const;
    virtual void setUnit(Unit unit);

    virtual QRectF geometry(Unit unit = UnitNotDefined) const;
    virtual void setGeometry(const QRectF & rect, Unit unit = UnitNotDefined);
    virtual qreal height(Unit unit = UnitNotDefined) const;
    virtual void setHeight(qreal height, Unit unit = UnitNotDefined);
    virtual qreal width(Unit unit = UnitNotDefined) const;
    virtual void setWidth(qreal width, Unit unit = UnitNotDefined);
    virtual QSizeF minSize(Unit unit = UnitNotDefined) const;
    virtual QRectF boundingRect(Unit unit = UnitNotDefined, bool withTransformation = true, bool withBorder = true) const;
    virtual QPolygonF polygon(Unit unit = UnitNotDefined) const;

    virtual qint8 frame() const;
    virtual void setFrame(Frames frame);
    virtual qreal opacity() const;
    virtual void setOpacity(qreal opacity);
    virtual qreal rotation() const;
    virtual void setRotation(qreal angle);

    virtual QPen borderPen() const;
    virtual void setBorderPen(const QPen & pen);
    virtual QBrush backgroundBrush() const;
    virtual void setBackgroundBrush(const QBrush & brush);

    virtual BorderType borderType() const;
    virtual void setBorderType(BaseItemInterface::BorderType borderType);
    virtual QString borderTypeStr() const;
    virtual void setBorderTypeStr(const QString & borderType);

    int resizeFlags() const;

    int baseItemFlags() const;
    void setBaseItemFlags(int flags, bool value = true);
//    void setBaseItemFlag(int flag, bool value = true);
    bool isBaseItemFlagSet(int flag);

    virtual bool enabled() const;
    virtual void setEnabled(bool b = true);

    virtual int order() const;
    virtual void setOrder(int order);

    virtual void setDpi(int dpi);
    virtual int dpi() const;

    virtual bool isSplitAllowed() const {return false;}

    virtual QList<CuteReport::InternalStringData> renderingStrings(){return QList<InternalStringData>();}

    // common painter
    static void paintBegin(QPainter * painter, const QStyleOptionGraphicsItem *option, BaseItemInterfacePrivate * data, const QRectF &boundingRect = QRectF(), RenderingType type = RenderingTemplate);
    static void paintEnd(QPainter * painter, const QStyleOptionGraphicsItem *option, BaseItemInterfacePrivate *data, const QRectF &boundingRect = QRectF(), RenderingType type = RenderingTemplate);
    static QRectF paintArea(const BaseItemInterfacePrivate * data, const QRectF &boundingRect = QRectF());
    static qreal borderWidth(const BaseItemInterfacePrivate * data, Unit unit = UnitNotDefined, qreal *insideWidth = 0, qreal *outsideWidth = 0);

    virtual QTransform transform() const;

    // values in local unit
    // NotDefined = default item meassure
    QRectF mapFromPage(const QRectF & rect, Unit inputUnit = UnitNotDefined, Unit outputUnit = UnitNotDefined) const;
    QPointF mapFromPage(const QPointF & point, Unit inputUnit = UnitNotDefined, Unit outputUnit = UnitNotDefined) const;
    QRectF mapToPage(const QRectF & rect, Unit inputUnit = UnitNotDefined, Unit outputUnit = UnitNotDefined) const;
    QPointF mapToPage(const QPointF & point, Unit inputUnit = UnitNotDefined, Unit outputUnit = UnitNotDefined) const;
    QRectF mapToParent(const QRectF & rect, Unit inputUnit = UnitNotDefined, Unit outputUnit = UnitNotDefined) const ;
    QPointF mapToParent(const QPointF & point, Unit inputUnit = UnitNotDefined, Unit outputUnit = UnitNotDefined) const;
    QRectF mapFromParent(const QRectF & rect, Unit inputUnit = UnitNotDefined, Unit outputUnit = UnitNotDefined) const ;
    QPointF mapFromParent(const QPointF & point, Unit inputUnit = UnitNotDefined, Unit outputUnit = UnitNotDefined) const;
    QRectF absoluteGeometry(Unit outputUnit = UnitNotDefined) const;
    void setAbsoluteGeometry(const QRectF & rect, Unit inputUnit = UnitNotDefined);
    QRectF absoluteBoundingRect(Unit unit = UnitNotDefined, bool withTransformation = true, bool withBorder = true) const;
    QPolygonF absolutePolygon(Unit unit = UnitNotDefined) const;

    static CuteReport::PageInterface * itemPage(CuteReport::BaseItemInterface * item);
    static QFont fontPrepared(const QFont & font, qint16 dpi);
    static QPen penPrepared(const QPen & pen, qint16 dpi); /// pen width is always in mm, so we convert it to required measure
    static QTransform transform(const BaseItemInterfacePrivate *d);
    static QPointF transformedPos(const BaseItemInterfacePrivate *d, const QRectF &rect);

    static BorderType borderTypeFromString(const QString & bType);
    static QString borderTypeToString(BorderType bType);

    virtual StdEditorPropertyList stdEditorList() const;

    QStringList _borderType_variants();

    virtual void _setCurrentProperty(int num) {m_currentProperty = num;}
    virtual int _currentProperty() { return m_currentProperty;}
    virtual QString _current_property_description() const;
    virtual int _current_property_precision() const;

    virtual TemplateItemDetailLevel templateDetailLevel();
    virtual void setTemplateDetailLevel(const TemplateItemDetailLevel &value);

    static TemplateItemDetailLevel defaultTemplateDetailLevel();
    static void setDefaultTemplateDetailLevel(const TemplateItemDetailLevel &value);

    void aboutToBeDeleted();

signals:
    void unitChanged(Unit);
    void geometryChanged(QRectF newGeometry);
    void opacityChanged(qreal newOpacity);
    void frameChanged(Frames);
    void rotationChanged(qreal angle);
    void borderPenChanged(QPen pen);
    void backgroundBrushChanged(QBrush brush);
    void dpiChanged(int dpi);
    void enabledChanged(bool);
    void orderChanged(int);
    void parentItemChanged(CuteReport::BaseItemInterface * parent);
    void renderingStringsChanged();
    void stdEditorListChanded(StdEditorPropertyList);
    void borderTypeChanged(BorderType);
    void borderTypeChanged(QString);
    void baseItemFlagsChanged(int);

    void boundingRectChanged();

    /** signal emited when renderInit() method is called */
    void printInit();

    /** signal emited when renderReset() method is called */
    void printReset();

    /** signal emited when render() method processing started
     * All access to the Item's methods affects original (template) item, but not generated one
    */
    void printCreateBefore();

    /** signal emited in render() method when item's private data replaced by itemView's private data
     * All access to the Item's methods affects generated copy  of item that will be printer.
     * No data of item processed yet
    */
    void printDataBefore();

    /** signal emited in render() method when item's private data replaced by itemView's private data
     * All access to the Item's methods affects generated copy  of item that will be printer.
     * All data is already processed and item is totally ready to print
     */
    void printBefore();

    /** signal emited when itemView already generated and all preprocessing is done
     * This signal must be inited after all geometry arrangement is done, so only rendering module can init it from outside item
    */
    void printAfter();
    
public slots:
//    void updateViewIfExists();

protected:
    BaseItemInterface(BaseItemInterfacePrivate *dd, QObject * parent);
    virtual BaseItemInterface * itemClone() const = 0;
    void pushContext(BaseItemInterfacePrivate * r);
    void popContext();

    virtual void childAdded(BaseItemInterface * item) { Q_UNUSED(item); }
    virtual void childRemoved(BaseItemInterface * item) { Q_UNUSED(item); }

    void setResizeFlags(int resizeFlags);
    static const QRectF adjustRect(QRectF & rect, const QPen & pen);
    virtual void adjustOrder();
    virtual void updateMeassure();

protected:
    void renderInitBase(CuteReport::ScriptEngineInterface *scriptEngine, CuteReport::InitFlags flags);
    void rendererBeginBaseStart(BaseItemInterfacePrivate *p);
    void rendererBeginBaseEnd();
    void renderResetBase();

    BaseItemInterfacePrivate * d_ptr;
    BaseItemInterfacePrivate * orig_ptr;
    QList <BaseItemInterfacePrivate*> dataStack;
    ItemInterfaceView * m_gui;
    PageInterface * m_page;
    BaseItemInterface * m_parentItem;
//    bool m_inited;
    int m_currentProperty;
    TemplateItemDetailLevel m_templateDetailLevel;

    static TemplateItemDetailLevel g_templateDetailLevel;

private:
    void initMe();
    Q_DECLARE_PRIVATE(BaseItemInterface)

    friend class ReportCore;
    friend class BaseRenderedItemInterface;
    friend class ItemInterfaceView;
};


class CUTEREPORT_EXPORTS BaseItemHelperInterface: public QWidget
{
public:
    virtual void sync(){}
    virtual bool screenBack(bool accept = true){Q_UNUSED(accept); return false;} //only one screen by default;
};


Q_DECLARE_OPERATORS_FOR_FLAGS(BaseItemInterface::Frames)
//Q_DECLARE_OPERATORS_FOR_FLAGS(BaseItemInterface::BaseItemFlags)
} // namespace

Q_DECLARE_INTERFACE(CuteReport::BaseItemInterface, "CuteReport.BaseItemInterface/1.0")
Q_DECLARE_METATYPE(CuteReport::BaseItemInterface*)
Q_DECLARE_METATYPE(CuteReport::BaseItemInterface::Frame)
Q_DECLARE_METATYPE(CuteReport::BaseItemInterface::ResizeFlags)
Q_DECLARE_METATYPE(CuteReport::BaseItemInterface::BorderType)
Q_DECLARE_METATYPE(CuteReport::BaseItemInterface::BaseItemFlag)

#endif // BASEITEMINTERFACE_H
