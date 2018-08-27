#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include "rendererprocessor.h"
#include "plugins_common.h"
#include "scriptengineinterface.h"

#include <QScriptEngine>

namespace CuteReport {
class RendererPublicInterface;
}


USING_SUIT_NAMESPACE

SUIT_BEGIN_NAMESPACE
class ScriptEngine : public CuteReport::ScriptEngineInterface
{
    Q_OBJECT
public:
    explicit ScriptEngine(RendererProcessor *processor);
    ~ScriptEngine();

    virtual CuteReport::RendererPublicInterface *rendererItemInterface();

    RendererProcessor * processor() {return m_processor;}

    virtual void sendLog(CuteReport::LogLevel level, const QString &senderName,
                         const QString &shortMessage, const QString &fullMessage);
private:
    RendererProcessor *m_processor;
};


/// ------------------  template for additional metatypes
template <typename Tp>
QScriptValue qScriptValueFromQObject(QScriptEngine *engine, Tp const &qobject)
{
    return engine->newQObject(qobject);
}

template <typename Tp>
void qScriptValueToQObject(const QScriptValue &value, Tp &qobject)
{
    qobject = qobject_cast<Tp>(value.toQObject());
}

template <typename Tp>
int qScriptRegisterQObjectMetaType( QScriptEngine *engine, const QScriptValue &prototype = QScriptValue()
#ifndef qdoc
    , Tp * /* dummy */ = 0
#endif
    )
{
    return qScriptRegisterMetaType<Tp>(engine, qScriptValueFromQObject,
                                       qScriptValueToQObject, prototype);
}
/// --------------------------




SUIT_END_NAMESPACE

#endif // SCRIPTENGINE_H
