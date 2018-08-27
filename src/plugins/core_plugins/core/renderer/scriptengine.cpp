#include "scriptengine.h"

SUIT_BEGIN_NAMESPACE

ScriptEngine::ScriptEngine(RendererProcessor *processor) :
    CuteReport::ScriptEngineInterface(processor), m_processor(processor)
{
}


ScriptEngine::~ScriptEngine()
{

}


CuteReport::RendererPublicInterface *ScriptEngine::rendererItemInterface()
{
    if (!m_processor)
        return 0;
    return m_processor->rendererItemInterface();
}


void ScriptEngine::sendLog(CuteReport::LogLevel level, const QString &senderName, const QString &shortMessage, const QString &fullMessage)
{
    m_processor->sendLog(level, shortMessage, fullMessage);
}

SUIT_END_NAMESPACE
