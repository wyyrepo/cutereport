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
#include "imageitem.h"
#include "imageitem_p.h"
#include "item_common/simpleitemview.h"
#include "item_common/simplerendereditem.h"
#include "pageinterface.h"
#include "rendererpublicinterface.h"
#include "datasetinterface.h"
#include "imageitemscript.h"
#include "reportcore.h"
#include "scriptengineinterface.h"

#define MODULENAME "ImageItem"

using namespace CuteReport;

inline void initMyResource() { Q_INIT_RESOURCE(imageitem); }

ImageItem::ImageItem(QObject * parent) :
    ItemInterface(new ImageItemPrivate, parent),
    m_renderer(0)
{
    Q_D(ImageItem);
    setResizeFlags( ResizeLeft | ResizeRight | ResizeTop | ResizeBottom);
    d->geometry = QRectF(0,0,30,30);
}


ImageItem::ImageItem(ImageItemPrivate *dd, QObject * parent)
    :ItemInterface(dd, parent),
      m_renderer(0)
{
    //    emptyBrush = QBrush(QImage(":/empty.png"));
}


ImageItem::~ImageItem()
{
}


bool ImageItem::moduleInit()
{
    initMyResource();
    return true;
}


void ImageItem::init()
{
    BaseItemInterface::init();
    tryToLoadDynamicImage();
}


CuteReport::BaseItemInterface * ImageItem::itemClone() const
{
    Q_D(const ImageItem);
    return new ImageItem(new ImageItemPrivate(*d), parent());
}


QByteArray ImageItem::serialize()
{
    Q_D(ImageItem);
    QByteArray ba;
    QDataStream s( &ba, QIODevice::ReadWrite );
    s << *d;
    return ba;
}


void ImageItem::deserialize(QByteArray &data)
{
    Q_D(ImageItem);
    QDataStream s( &data, QIODevice::ReadWrite );
    s >> *d;
}


QDataStream &operator<<(QDataStream &s, const ImageItemPrivate &p) {
    s << static_cast<const ItemInterfacePrivate&>(p);
    s << p.image << p.source  << (qint8)p.scaleType << (qint8)p.sourceType << p.keepAspectRatio << p.center;
    return s;
}


QDataStream &operator>>(QDataStream &s, ImageItemPrivate &p) {
    qint8 scaleType, sourceType;
    s >> static_cast<ItemInterfacePrivate&>(p);
    s >> p.image; s >> p.source; s >> scaleType; s >> sourceType; s >> p.keepAspectRatio; s >> p.center;

    p.scaleType = (ImageItem::ScaleType)scaleType;
    p.sourceType = (ImageItem::SourceType)sourceType;

    return s;
}


BaseItemInterface *ImageItem::createInstance(QObject * parent) const
{
    return new ImageItem(parent);
}


void ImageItem::init_gui()
{
    if (m_gui)
        return;
    m_gui = new ImageItemView(this);
}


//void ImageItem::update_gui()
//{

//}


bool ImageItem::canContain(QObject * object)
{
    return (dynamic_cast<CuteReport::BaseItemInterface*>(object));
}


QIcon ImageItem::itemIcon() const
{
    return QIcon(":/image.png");
}


QString ImageItem::moduleShortName() const
{
    return tr("Image");
}


QString ImageItem::itemGroup() const
{
    return tr("Shapes");
}


ImageItem::SourceType ImageItem::sourceType() const
{
    Q_D(const ImageItem);
    return d->sourceType;
}




void ImageItem::setSourceType(ImageItem::SourceType sourceType)
{
    Q_D(ImageItem);
    if (sourceType == d->sourceType)
        return;
    d->sourceType = sourceType;
    emit sourceTypeChanged(d->sourceType);
    emit sourceTypeChanged(sourceTypeToString(d->sourceType));
    emit changed();
}


QString ImageItem::sourceTypeStr() const
{
    Q_D(const ImageItem);
    return sourceTypeToString(d->sourceType);
}


void ImageItem::setSourceTypeStr(const QString & sourceType)
{
    bool ok = true;
    ImageItem::SourceType value = sourceTypeFromString(sourceType, &ok);
    if (ok)
        setSourceType(value);
}


QImage ImageItem::image() const
{
    Q_D(const ImageItem);
    return d->image;
}


void ImageItem::setImage(const QImage & image)
{
    Q_D(ImageItem);
    SAVEIFCHANGED(image, image)
    update_gui();
    emit imageChanged(d->image);

    if (d->state != NotInitedState)
        setSourceType(Static);
}


QString ImageItem::source() const
{
    Q_D(const ImageItem);
    return d->source;
}


void ImageItem::setSource(const QString & source)
{
    Q_D(ImageItem);
    SAVEIFCHANGED(source, source)
    if (d->state != NotInitedState) {
        tryToLoadDynamicImage();
        update_gui();
    }

    emit sourceChanged(d->source);
    emit renderingStringsChanged();
    emit changed();

}


ImageItem::ScaleType ImageItem::scaleType() const
{
    Q_D(const ImageItem);
    return d->scaleType;
}


void ImageItem::setScaleType(ImageItem::ScaleType scaleType)
{
    Q_D(ImageItem);
    if (d->scaleType == scaleType)
        return;
    d->scaleType = scaleType;
    update_gui();
    emit scaleTypeChanged(d->scaleType);
    emit scaleTypeChanged(scaleTypeToString(d->scaleType));
    emit changed();
}


QString ImageItem::scaleTypeStr() const
{
    Q_D(const ImageItem);
    return scaleTypeToString(d->scaleType);
}


void ImageItem::setScaleTypeStr(const QString &scaleType)
{
    bool ok = false;
    ImageItem::ScaleType value = scaleTypeFromString(scaleType, &ok);
    if (ok)
        setScaleType(value);
}


bool ImageItem::keepAspectRatio() const
{
    Q_D(const ImageItem);
    return d->keepAspectRatio;
}


void ImageItem::setKeepAspectRatio(bool keepAspectRatio)
{
    Q_D(ImageItem);
    if (d->keepAspectRatio == keepAspectRatio)
        return;
    d->keepAspectRatio = keepAspectRatio;
    update_gui();
    emit keepAspectRatioChanged(d->keepAspectRatio);
    emit changed();
}


bool ImageItem::center() const
{
    Q_D(const ImageItem);
    return d->center;
}


void ImageItem::setCenter(bool center)
{
    Q_D(ImageItem);
    if (center == d->center)
        return;

    d->center = center;
    update_gui();
    emit centerChanged(d->center);
    emit changed();
}


void ImageItem::renderInit(ScriptEngineInterface *scriptEngine, InitFlags flags)
{
    Q_D(ImageItem);
    renderInitBase(scriptEngine, flags);
    m_renderer = scriptEngine->rendererItemInterface();

    if (flags.testFlag(CuteReport::InitScript))
        registerImageItemScriptClass(scriptEngine);
    if (flags.testFlag(CuteReport::InitData))
        m_renderer->registerStrings( QList<InternalStringData>() << InternalStringData(0, d->source, false, StringHasExpression), this);
}


void ImageItem::renderReset()
{
    m_renderer = 0;
    renderResetBase();
}


bool ImageItem::renderBegin()
{
    rendererBeginBaseStart(new ImageItemPrivate(*(reinterpret_cast<ImageItemPrivate*>(d_ptr))));
    rendererBeginBaseEnd();
    Q_D(ImageItem);
    bool needProcessing = d->enabled;
    if (!needProcessing)
        renderEnd();

    return needProcessing;
}


RenderedItemInterface *ImageItem::renderView(quint32 id)
{
    Q_D(ImageItem);
    RenderedImageItem * view = new RenderedImageItem(this, new ImageItemPrivate(*d), m_renderer);
    return view;
}


void ImageItem::paint(QPainter * painter, const QStyleOptionGraphicsItem *option, BaseItemInterfacePrivate * data, const QRectF &boundingRect, RenderingType type )
{
    Q_UNUSED(option)

    ItemInterface::paintBegin(painter, option, data, boundingRect, type);

    const ImageItemPrivate * const d = static_cast<const ImageItemPrivate*>(data);


//    QRect rect = boundingRect.toRect();
    QRectF rect = paintArea(data, boundingRect);

    if (d->image.isNull()) {
        if (type == RenderingTemplate) {
            static QBrush emptyBrush(QImage(":/empty.png"));
            painter->setBrush(emptyBrush);
            qreal opacity = painter->opacity();
            painter->setOpacity(0.3);
            painter->drawRect(rect);
            painter->setOpacity(opacity);
        } else
            painter->drawText(rect, Qt::AlignCenter, "No Image");
    } else {
        qreal sw = 0;
        qreal sh = 0;

        QImage img(0, 0/*,QImage::Format_ARGB32_Premultiplied*/);

        QPointF point = rect.topLeft();

        // scale type is deprecated
//        switch (d->scaleType) {
//            case ScaleImage:
//                img = d->image.scaled(rect.width(), rect.height(), d->keepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
//                break;

//            case ScaleFrame:
//                img = d->image;
//                sw = img.width() - rect.width();
//                sh = img.height() - rect.height();
//                break;
//            default:
//                break;
//        }

        img = d->image.scaled(rect.width(), rect.height(), d->keepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        if (d->center) {
            int cx = 0;
            int cy = 0;
            int cw = img.width();
            int ch = img.height();

            point.setX(point.x() + (rect.width() - cw) /2);
            point.setY(point.y() + (rect.height() - ch) /2);

            if (point.x() < 0) {
                cx = abs(point.x());
                cw -= 2*cx;
                point.setX(0);
            }

            if (point.y() < 0) {
                cy = abs(point.y());
                ch -= 2*cy;
                point.setY(0);
            }

            img = img.copy(cx, cy, cw, ch);
        }

        painter->drawImage(point, img);

        if (sw || sh) {
            //        setWidth( boundingRect.width() + sw);
            //        setHeight( boundingRect.height() + sh);
        }
    }


    ItemInterface::paintEnd(painter, option, data, boundingRect, type);
}


QList<InternalStringData> ImageItem::renderingStrings()
{
    Q_D(ImageItem);
    return QList<InternalStringData>() << InternalStringData(0, d->source, false, StringHasExpression);
}


QString ImageItem::sourceTypeToString(ImageItem::SourceType value)
{
    int idx = -1;
    for (int i = 0; i < ImageItem_SourceType_num; ++i) {
        if (qtscript_ImageItem_SourceType_values[i] == value) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        CuteReport::ReportCore::log(LogError, MODULENAME, "sourceTypeToString", QString("Value \\'%1\\' is not within range").arg((int)value));
        return QString();
    }

    return QString(qtscript_ImageItem_SourceType_keys[idx]);
}


ImageItem::SourceType ImageItem::sourceTypeFromString(const QString &value, bool *ok)
{
    QString strIn = value.toLower();
    int idx = -1;
    if (ok) {*ok = true;}
    for (int i = 0; i < ImageItem_SourceType_num; ++i) {
        QString str(qtscript_ImageItem_SourceType_keys[i]);
        if (strIn == str.toLower()) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        CuteReport::ReportCore::log(LogError, MODULENAME, "sourceTypeFromString", QString("Value \\'%1\\' is not within range").arg(value));
        if (ok)
            *ok = false;
    }

    return ImageItem::SourceType(qtscript_ImageItem_SourceType_values[idx]);
}


QString ImageItem::scaleTypeToString(ImageItem::ScaleType value)
{
    int idx = -1;
    for (int i = 0; i < ImageItem_ScaleType_num; ++i) {
        if (qtscript_ImageItem_ScaleType_values[i] == value) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        CuteReport::ReportCore::log(LogError, MODULENAME, "scaleTypeToString", QString("Value \\'%1\\' is not within range").arg((int)value));
        return QString();
    }

    return QString(qtscript_ImageItem_ScaleType_keys[idx]);
}


ImageItem::ScaleType ImageItem::scaleTypeFromString(const QString &value, bool *ok)
{
    QString strIn = value.toLower();
    int idx = -1;
    if (ok) {*ok = true;}
    for (int i = 0; i < ImageItem_ScaleType_num; ++i) {
        QString str(qtscript_ImageItem_ScaleType_keys[i]);
        if (strIn == str.toLower()) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        CuteReport::ReportCore::log(LogError, MODULENAME, "scaleTypeFromString", QString("Value \\'%1\\' is not within range").arg(value));
        if (ok)
            *ok = false;
    }

    return ImageItem::ScaleType(qtscript_ImageItem_ScaleType_values[idx]);
}


void ImageItem::tryToLoadDynamicImage()
{
    Q_D(ImageItem);
    CuteReport::ReportInterface * report = reportObject();
    if (!report)
        return;

    switch (d->sourceType) {
        case ImageItem::Storage: {
                //QImage image;
                //image.loadFromData( reportCore()->loadObject(d->source, report) );
                //setImage(image);
                d->image.loadFromData( reportCore()->loadObject(d->source, report) );
            }
            break;
        case ImageItem::Dataset:
            break;
        case ImageItem::Static :
            break;
    }
}


QStringList ImageItem::_sourceType_variants() const
{
    return ImageItem_SourceType_variants();
}


QStringList ImageItem::_scaleType_variants() const
{
    return ImageItem_ScaleType_variants();
}


QString ImageItem::_current_property_description() const
{
    QString propertyName = metaObject()->property(m_currentProperty).name();

    if (propertyName == "sourceType")
        return tr("Defines the source of the image data.\n"
                  "Static: use \"image\" to load blob\n"
                  "Storage: you can define \'source\' field in format:\n"
                  "file://[data.\"name\"] to take file with runtime name\n"
                  "res://objectname.png to take file from resource\n"
                  "Dataset: [data.\"name\"]\n");
    else if (propertyName == "source")
        return tr("Set source for sourceTypes: Script and Dataset");
    else if (propertyName == "image")
        return tr("Set image blob for sourceTypes: Static");
    else if (propertyName == "scaleType")
        return tr("Type of Image layout and scale in a frame");
    else
        return ItemInterface::_current_property_description();
}


//====================================================================================================================

RenderedImageItem::RenderedImageItem(CuteReport::BaseItemInterface * item, CuteReport::BaseItemInterfacePrivate *itemPrivateData, CuteReport::RendererPublicInterface * renderer)
    :RenderedItemInterface(item, itemPrivateData)
{
    ImageItemPrivate * const d = static_cast<ImageItemPrivate*>(itemPrivateData);
    ImageItem* image = reinterpret_cast<ImageItem*>(item);
    switch (d->sourceType) {
        case ImageItem::Storage:
            d->source = renderer->processString(CuteReport::ScriptString(item->reportObject(), image, d->source ));
            d->image.loadFromData( renderer->getStorageObject(d->source).toByteArray() );
            break;
        case ImageItem::Dataset: {
                QString str = d->source.remove('[').remove(']').remove('"');
                CuteReport::DatasetInterface * dataset = renderer->dataset(str.section('.', 0, 0));
                if (dataset)
                    d->image.loadFromData( dataset->getValue(str.section('.', 1, 1)).toByteArray() );
                else
                    d->image = QImage();
            }
            break;
        case ImageItem::Static :
            break;
    }
}


void RenderedImageItem::paint(QPainter * painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    ImageItem::paint(painter, option, d_ptr, rect(), CuteReport::RenderingReport);
}


#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(ImageItem, ImageItem)
#endif
