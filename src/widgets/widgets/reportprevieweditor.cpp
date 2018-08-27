#include "reportprevieweditor.h"
#include <renderediteminterface.h>

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>

namespace CuteReport
{

ReportPreviewEditor::ReportPreviewEditor(QGraphicsScene *scene, QWidget *topWidget)
    :m_scene(scene), m_selectionItem(0), m_editedItem(0), m_topWidget(topWidget)
{
}


ReportPreviewEditor::~ReportPreviewEditor()
{
    deleteSelectionItem();
}


void ReportPreviewEditor::sceneMouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (m_editedItem && m_editedItem->isEditMode())
        return;

    QList<QGraphicsItem *> items = m_scene->items(mouseEvent->scenePos(), Qt::ContainsItemBoundingRect, Qt::DescendingOrder);
//    qDebug() << "item count: " << items.count();
    foreach(QGraphicsItem * item, items) {
        if (item == m_selectionItem)
            continue;
        RenderedItemInterface * rendItem = dynamic_cast<RenderedItemInterface*>(item);

        if (rendItem && rendItem->hasEditMode()) {
            if (!m_selectionItem) {
                m_selectionItem = new QGraphicsPolygonItem();
                m_scene->addItem(m_selectionItem);
                m_selectionItem->setBrush(Qt::green);
                m_selectionItem->setOpacity(0.3);
                m_selectionItem->setZValue(10000);
            }
            // TODO: not fully implemented:  rendItem->editShape()
            QPolygonF selectedRect = rendItem->mapToScene(rendItem->boundingRect());
            m_selectionItem->setPos(rendItem->boundingRect().topLeft());
            m_selectionItem->setPolygon(selectedRect);
            return;
        }
    }

    deleteSelectionItem();
}


void ReportPreviewEditor::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (m_editedItem && m_editedItem->isEditMode())
        return;

    QList<QGraphicsItem *> items = m_scene->items(mouseEvent->scenePos(), Qt::ContainsItemBoundingRect, Qt::DescendingOrder);
    foreach(QGraphicsItem * item, items) {
        CuteReport::RenderedItemInterface * rendItem = qgraphicsitem_cast<CuteReport::RenderedItemInterface*>(item);
        if (rendItem) {
            bool hasEditMode = rendItem->setEditMode(m_topWidget);
            if (hasEditMode)
                m_editedItem = rendItem;
            rendItem->redraw(false);
            rendItem->update();
            break;
        }
    }
}


void ReportPreviewEditor::deleteSelectionItem()
{
    if (!m_selectionItem)
        return;
    m_scene->removeItem(m_selectionItem);
    delete m_selectionItem;
    m_selectionItem = 0;
}


} //namespace
