#include "gamemanager.h"

#include "coreengine/mainapp.h"

#include <QFileInfo>

#include <QDirIterator>

GameManager* GameManager::m_pInstance = nullptr;

GameManager::GameManager()
{
    oxygine::Resources::loadXML("resources/images/game/res.xml");
    // load game images
    for (qint32 i = 0; i < Settings::getMods().size(); i++)
    {
        if (QFile::exists(Settings::getMods().at(i) + "/images/game/res.xml"))
        {
            oxygine::Resources::loadXML(Settings::getMods().at(i) + "/images/game/res.xml");
        }
    }
}

GameManager* GameManager::getInstance()
{
    if (m_pInstance == nullptr)
    {
        m_pInstance = new GameManager();
    }
    return m_pInstance;
}

void GameManager::loadAll()
{
    loaded = true;
    QStringList searchPaths;
    searchPaths.append("resources/scripts/actions");
    // make sure to overwrite existing js stuff
    for (qint32 i = 0; i < Settings::getMods().size(); i++)
    {
        searchPaths.append(Settings::getMods().at(i) + "/scripts/actions");
    }
    for (qint32 i = 0; i < searchPaths.size(); i++)
    {
        QString path =  QCoreApplication::applicationDirPath() + "/" + searchPaths[i];
        QStringList filter;
        filter << "*.js";
        QDirIterator* dirIter = new QDirIterator(path, filter, QDir::Files, QDirIterator::Subdirectories);
        while (dirIter->hasNext())
        {
            dirIter->next();
            QString file = dirIter->fileInfo().fileName().split(".").at(0);
            if (!m_loadedActions.contains(file.toUpper()))
            {
                loadAction(file.toUpper());
            }
        }
    }
    m_loadedActions.sort();
}

bool GameManager::loadAction(QString actionID)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QStringList searchPaths;
    searchPaths.append("resources/scripts/actions");
    for (qint32 i = 0; i < Settings::getMods().size(); i++)
    {
        searchPaths.append(Settings::getMods().at(i) + "/scripts/actions");
    }
    bool bRet = false;
    for (qint32 i = 0; i < searchPaths.size(); i++)
    {
        QString file = searchPaths[i] + "/" + actionID + ".js";
        QFileInfo checkFile(file);
        if (checkFile.exists() && checkFile.isFile())
        {
            pInterpreter->openScript(file, true);
            if (!bRet)
            {
                m_loadedActions.append(actionID);
            }
            bRet = true;
        }
    }
    return bRet;
}
