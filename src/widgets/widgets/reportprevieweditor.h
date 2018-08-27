#ifndef REPORTPREVIEWEDITOR_H
#define REPORTPREVIEWEDITOR_H

class QGraphicsScene;
class QGraphicsPolygonItem;
class QGraphicsSceneMouseEvent;
class QWidget;

namespace CuteReport
{

class RenderedItemInterface;

class ReportPreviewEditor
{
public:
    ReportPreviewEditor(QGraphicsScene * scene, QWidget * topWidget);
    ~ReportPreviewEditor();

    void sceneMouseMoveEvent ( QGraphicsSceneMouseEvent * mouseEvent );
    void mouseDoubleClickEvent( QGraphicsSceneMouseEvent * mouseEvent );

private:
    inline void deleteSelectionItem();

    QGraphicsScene * m_scene;
    QGraphicsPolygonItem * m_selectionItem;
    RenderedItemInterface * m_editedItem;
    QWidget * m_topWidget;
};

} //namespace

#endif // REPORTPREVIEWEDITOR_H
