#include "coreengine/interpreter.h"
#include "coreengine/console.h"
#include "coreengine/mainapp.h"
#include "coreengine/globalutils.h"
#include "coreengine/audiothread.h"
#include "coreengine/userdata.h"
#include "resource_management/fontmanager.h"
#include "resource_management/cospritemanager.h"
#include "resource_management/terrainmanager.h"
#include "resource_management/buildingspritemanager.h"
#include "resource_management/coperkmanager.h"
#include "resource_management/unitspritemanager.h"

#include <QDir>
#include <QQmlEngine>
#include <QTextStream>
#include <QThread>

spInterpreter Interpreter::m_pInstance;
QString Interpreter::m_runtimeData;

Interpreter::Interpreter()
    : QQmlEngine()
{
    setObjectName("Interpreter");
    setCppOwnerShip(this);
    connect(this, &Interpreter::sigNetworkGameFinished, this, &Interpreter::networkGameFinished, Qt::QueuedConnection);
}

void Interpreter::reloadInterpreter(const QString & runtime)
{
    m_pInstance = nullptr;
    m_pInstance = spInterpreter::create();
    m_pInstance->init();
    m_pInstance->loadScript(runtime, "Interpreter Runtime");
}

Interpreter::~Interpreter()
{
    // free memory
    collectGarbage();
}

void Interpreter::release()
{
    m_pInstance = nullptr;
}

void Interpreter::init()
{
    Mainapp* pApp = Mainapp::getInstance();
    moveToThread(pApp->getWorkerthread());
    setOutputWarningsToStandardError(false);
    setIncubationController(nullptr);

    QJSValue globals = newQObject(GlobalUtils::getInstance());
    globalObject().setProperty("globals", globals);
    QJSValue audio = newQObject(pApp->getAudioThread());
    globalObject().setProperty("audio", audio);
    QJSValue console = newQObject(Console::getInstance().get());
    globalObject().setProperty("GameConsole", console);
    globalObject().setProperty("console", console);
    QJSValue fontManager = newQObject(FontManager::getInstance());
    globalObject().setProperty("FontManager", fontManager);
    QJSValue settings = newQObject(Settings::getInstance());
    globalObject().setProperty("settings", settings);
    QJSValue userdata = newQObject(Userdata::getInstance());
    globalObject().setProperty("userdata", userdata);
    QJSValue coSpriteManager = newQObject(COSpriteManager::getInstance());
    globalObject().setProperty("coSpriteManager", coSpriteManager);
    QJSValue unitSpriteManager = newQObject(UnitSpriteManager::getInstance());
    globalObject().setProperty("unitSpriteManager", unitSpriteManager);
    QJSValue buildingspritemanager = newQObject(BuildingSpriteManager::getInstance());
    globalObject().setProperty("buildingSpriteManager", buildingspritemanager);
    QJSValue terrainSpriteManager = newQObject(TerrainManager::getInstance());
    globalObject().setProperty("terrainSpriteManager", terrainSpriteManager);
    QJSValue coPerkSpriteManager = newQObject(COPerkManager::getInstance());
    globalObject().setProperty("coPerkSpriteManager", coPerkSpriteManager);

    GameEnums::registerEnums();

    installExtensions(QJSEngine::Extension::AllExtensions);
}

QString Interpreter::getRuntimeData()
{
    return m_runtimeData;
}

void Interpreter::openScript(const QString & script, bool setup)
{
    QFile scriptFile(script);
    if (!scriptFile.open(QIODevice::ReadOnly))
    {
        QString error = "Error: attemp to read file " + script + " which could not be opened.";
        CONSOLE_PRINT(error, Console::eERROR);
    }
    else if (!scriptFile.exists())
    {
        QString error = "Error: unable to open non existing file " + script + ".";
        CONSOLE_PRINT(error, Console::eERROR);
    }
    else
    {
        CONSOLE_PRINT("Loading script " + script, Console::eDEBUG);
        QTextStream stream(&scriptFile);
        QString contents = stream.readAll();
        if (setup)
        {
            stream.seek(0);
            while (!stream.atEnd())
            {
                QString line = stream.readLine().simplified();
                m_runtimeData += line + "\n";
            }
        }
        scriptFile.close();
        QJSValue value = evaluate(contents, script);
        if (value.isError())
        {
            QString error = value.toString() + " in File:" + script + " in File: " +
                            value.property("fileName").toString() + " at Line: " +
                            value.property("lineNumber").toString();
            CONSOLE_PRINT(error, Console::eERROR);
        }
    }
}

void Interpreter::loadScript(const QString & content, const QString & script)
{
    CONSOLE_PRINT("Interpreter::loadScript: " + script, Console::eDEBUG);
    QJSValue value = evaluate(content, script);
    if (value.isError())
    {
        QString error = value.toString() + " in script " + script + " in File: " +
                        value.property("fileName").toString() + " at Line: " +
                        value.property("lineNumber").toString();
        CONSOLE_PRINT(error, Console::eERROR);
    }
}


QJSValue Interpreter::doFunction(const QString & func, const QJSValueList & args)
{
    QJSValue ret;
    QJSValue funcPointer = globalObject().property(func);
#ifdef GAMEDEBUG
    Q_ASSERT(Mainapp::getInstance()->getWorkerthread() == QThread::currentThread());
    if (funcPointer.isCallable())
    {
#endif
        ret = funcPointer.call(args);
        if (ret.isError())
        {
            QString error = ret.toString() + " in File: " +
                            ret.property("fileName").toString() + " at Line: " +
                            ret.property("lineNumber").toString();
            CONSOLE_PRINT(error, Console::eERROR);
        }
#ifdef GAMEDEBUG
    }
    else
    {
        QString error = "Error: attemp to call a non function value. Call:" + func;
        CONSOLE_PRINT(error, Console::eERROR);
    }
#endif
    return ret;
}

QJSValue Interpreter::doFunction(const QString & obj, const QString & func, const QJSValueList & args)
{
    QJSValue ret;
    QJSValue objPointer = globalObject().property(obj);
#ifdef GAMEDEBUG
    Q_ASSERT(Mainapp::getInstance()->getWorkerthread() == QThread::currentThread());
    if (objPointer.isObject())
    {
#endif
        QJSValue funcPointer = objPointer.property(func);
#ifdef GAMEDEBUG
        if (funcPointer.isCallable())
        {
#endif
            ret = funcPointer.call(args);
            if (ret.isError())
            {
                QString error = ret.toString() + " in File: " +
                                ret.property("fileName").toString() + " at Line: " +
                                ret.property("lineNumber").toString();
                CONSOLE_PRINT(error, Console::eERROR);
            }
#ifdef GAMEDEBUG
        }
        else
        {
            QString error = "Error: attemp to call a non function value. Call:" + obj + "." + func;
            CONSOLE_PRINT(error, Console::eERROR);
        }
    }
    else
    {
        QString error = "Error: attemp to call a non object value in order to call a function. Call:" + obj + "." + func;
        CONSOLE_PRINT(error, Console::eERROR);
    }
#endif
    return ret;
}

QJSValue Interpreter::doString(const QString & task)
{
#ifdef GAMEDEBUG
    Q_ASSERT(Mainapp::getInstance()->getWorkerthread() == QThread::currentThread());
#endif
    QJSValue value = evaluate(task, "GameCode");
    if (value.isError())
    {
        CONSOLE_PRINT(value.toString(), Console::eERROR);
    }
    return value;
}

void Interpreter::pushInt(const QString & name, qint32 value)
{
    globalObject().setProperty(name, value);
}

void Interpreter::pushDouble(const QString & name, double value)
{
    globalObject().setProperty(name, value);
}

void Interpreter::pushString(const QString & name, const QString & value)
{
    globalObject().setProperty(name, value);
}

void Interpreter::pushObject(const QString & name, QObject* object)
{
    QJSValue newQObj = newQObject(object);
    globalObject().setProperty(name, newQObj);
}

QJSValue Interpreter::newQObject(QObject* object)
{
    QJSValue newQObj = QQmlEngine::newQObject(object);
    // make sure js never deletes our qobject since that's kinda not what we want
    QQmlEngine::setObjectOwnership(object, QQmlEngine::ObjectOwnership::CppOwnership);
    return newQObj;
}

void Interpreter::setCppOwnerShip(QObject* object)
{
    QQmlEngine::setObjectOwnership(object, QQmlEngine::ObjectOwnership::CppOwnership);
}

void Interpreter::cleanMemory()
{
    collectGarbage();
}

qint32 Interpreter::getGlobalInt(const QString & var)
{
    qint32 ret = 0;
    QJSValue value = globalObject().property(var);
    if (!value.isNumber())
    {
        QString error = "Error: attemp to read " + var + "which is not from type number.";
        CONSOLE_PRINT(error, Console::eERROR);
    }
    else
    {
        ret = value.toInt();
    }
    return ret;
}

bool Interpreter::getGlobalBool(const QString & var)
{
    bool ret = 0;
    QJSValue value = globalObject().property(var);
    if (!value.isBool())
    {
        QString error = "Error: attemp to read " + var + "which is not from type bool.";
        CONSOLE_PRINT(error, Console::eERROR);
    }
    else
    {
        ret = value.toBool();
    }
    return ret;
}

double Interpreter::getGlobalDouble(const QString & var)
{
    double ret = 0;
    QJSValue value = globalObject().property(var);
    if (!value.isNumber())
    {
        QString error = "Error: attemp to read " + var + "which is not from type number.";
        CONSOLE_PRINT(error, Console::eERROR);
    }
    else
    {
        ret = value.toNumber();
    }
    return ret;
}

QString Interpreter::getGlobalString(const QString & var)
{
    QString ret = "";
    QJSValue value = globalObject().property(var);
    if (!value.isString())
    {
        QString error = "Error: attemp to read " + var + "which is not from type QString.";
        CONSOLE_PRINT(error, Console::eERROR);
    }
    else
    {
        ret = value.toString();
    }
    return ret;
}

QJSValue Interpreter::getGlobal(const QString & var)
{
    QJSValue value = globalObject().property(var);
    return value;
}

void Interpreter::setGlobal(const QString & var, const QJSValue & obj)
{
    globalObject().setProperty(var, obj);
}

void Interpreter::deleteObject(const QString & name)
{
    QString order = "Global[\"" + name + "\"] = null;\nGlobal[\"" + name + "\"] = undefined;";
    doString(order);
    collectGarbage();
}

bool Interpreter::exists(const QString & object, const QString & function)
{
    QJSValue objPointer = globalObject().property(object);
    if (objPointer.isObject())
    {
        QJSValue funcPointer = objPointer.property(function);
        if (funcPointer.isCallable())
        {
            return true;
        }
    }
    return false;
}

bool Interpreter::exists(const QString & object)
{
    QJSValue objPointer = globalObject().property(object);
    if (objPointer.isObject())
    {
        return true;
    }
    return false;
}

void Interpreter::networkGameFinished(qint32 value, QString id)
{
    QString obj = "Init";
    QString func = "onRemoteGameFinished";
    if (exists(obj, func))
    {
        QJSValueList args;
        args << value;
        args << id;
        doFunction(obj, func, args);
    }
}
