#include <QFile>

#include "game/gamemap.h"

#include "coreengine/mainapp.h"

#include "resource_management/terrainmanager.h"
#include "resource_management/buildingspritemanager.h"
#include "resource_management/unitspritemanager.h"
#include "resource_management/movementtablemanager.h"
#include "resource_management/weaponmanager.h"
#include "resource_management/gamemanager.h"
#include "resource_management/cospritemanager.h"

#include "game/terrain.h"

#include "game/unit.h"

#include "game/building.h"

#include "game/player.h"
#include "game/co.h"
#include "game/gameanimationfactory.h"

#include "menue/gamemenue.h"

const QString GameMap::m_JavascriptName = "map";
const QString GameMap::m_GameAnimationFactory = "GameAnimationFactory";
const qint32 GameMap::frameTime = 125;
GameMap* GameMap::m_pInstance = nullptr;



GameMap::GameMap(qint32 width, qint32 heigth, qint32 playerCount)
{
    loadMapData();
    newMap(width, heigth, playerCount);

}

GameMap::GameMap(QString map, bool gamestart, bool onlyLoad)
{
    loadMapData();
    QFile file(map);
    file.open(QIODevice::ReadOnly);
    QDataStream pStream(&file);
    deserialize(pStream);
    if (!onlyLoad)
    {
        updateSprites();
        qint32 heigth = getMapHeight();
        qint32 width = getMapWidth();
        centerMap(width / 2, heigth / 2);
    }
}

void GameMap::loadMapData(bool reload)
{
    m_pInstance = this;
    Interpreter* pInterpreter = Mainapp::getInstance()->getInterpreter();
    pInterpreter->setGlobal(m_JavascriptName, pInterpreter->newQObject(this));
    pInterpreter->setGlobal(m_GameAnimationFactory, pInterpreter->newQObject(GameAnimationFactory::getInstance()));

    TerrainManager* pTerrainManager = TerrainManager::getInstance();
    if (reload || (pTerrainManager->getTerrainCount() == 0))
    {
        pTerrainManager->loadAll();
    }
    BuildingSpriteManager* pBuildingSpriteManager = BuildingSpriteManager::getInstance();
    if (reload || (pBuildingSpriteManager->getBuildingCount() == 0))
    {
        pBuildingSpriteManager->loadAll();
    }
    UnitSpriteManager* pUnitspritemanager = UnitSpriteManager::getInstance();
    if (reload || (pUnitspritemanager->getUnitCount() == 0))
    {
        pUnitspritemanager->loadAll();
    }
    MovementTableManager* pMovementTableManager = MovementTableManager::getInstance();
    if (reload || (pMovementTableManager->getMovementTableCount() == 0))
    {
        pMovementTableManager->loadAll();
    }
    GameManager* pGameManager = GameManager::getInstance();
    if (reload || !pGameManager->getLoaded())
    {
        pGameManager->loadAll();
    }
    WeaponManager* pWeaponManager = WeaponManager::getInstance();
    if (reload || (pWeaponManager->getWeaponCount() == 0))
    {
        pWeaponManager->loadAll();
    }
    COSpriteManager* pCOSpriteManager = COSpriteManager::getInstance();
    if (reload || (pCOSpriteManager->getCOCount() == 0))
    {
        pCOSpriteManager->loadAll();
    }
}



GameMap::~GameMap()
{
    m_pInstance = nullptr;
    // remove us from the interpreter again
    Interpreter* pInterpreter = Mainapp::getInstance()->getInterpreter();
    pInterpreter->deleteObject(m_JavascriptName);
    // clean up session
    for (qint32 y = 0; y < fields.size(); y++)
    {
        fields.at(y)->clear();
        delete fields.at(y);
    }
    fields.clear();
}

spTerrain GameMap::getSpTerrain(qint32 x, qint32 y)
{
    if (onMap(x, y))
    {
        return fields.at(y)->at(x);
    }
    else
    {
        return nullptr;
    }
}

Terrain* GameMap::getTerrain(qint32 x, qint32 y)
{
    if (onMap(x, y))
    {
        return fields.at(y)->at(x).get();
    }
    else
    {
        return nullptr;
    }
}

spPlayer GameMap::getspPlayer(qint32 player)
{
    if (player >= 0 && player < players.size())
    {
        return players[player];
    }
    else
    {
        return nullptr;
    }
}

Player* GameMap::getPlayer(qint32 player)
{
    if (player >= 0 && player < players.size())
    {
        return players[player].get();
    }
    else
    {
        return nullptr;
    }
}

Player* GameMap::getCurrentPlayer()
{
    return m_CurrentPlayer.get();
}

spPlayer GameMap::getSpCurrentPlayer()
{
    return m_CurrentPlayer;
}

void GameMap::setCurrentPlayer(qint32 player)
{
    if ((player >= 0) && (player < players.size()))
    {
        m_CurrentPlayer = players[player];
    }
}

void GameMap::updateSprites(qint32 xInput, qint32 yInput)
{
    if ((xInput < 0) && (yInput < 0))
    {
        // update terrain sprites
        qint32 heigth = getMapHeight();
        qint32 width = getMapWidth();
        for (qint32 y = 0; y < heigth; y++)
        {
            for (qint32 x = 0; x < width; x++)
            {
                fields.at(y)->at(x)->loadSprites();
                if (fields.at(y)->at(x)->getUnit() != nullptr)
                {
                    fields.at(y)->at(x)->getUnit()->updateSprites();
                }
                if (fields.at(y)->at(x)->getBuilding() != nullptr)
                {
                    fields.at(y)->at(x)->getBuilding()->updateBuildingSprites();
                }
            }
        }
    }
    else
    {
        // more optimized for a single terrain :)
        for (qint32 y = yInput -1; y <= yInput + 1; y++)
        {
            for (qint32 x = xInput -1; x <= xInput + 1; x++)
            {
                if (onMap(x, y))
                {
                    fields.at(y)->at(x)->loadSprites();
                    if (fields.at(y)->at(x)->getUnit() != nullptr)
                    {
                        fields.at(y)->at(x)->getUnit()->updateSprites();
                    }
                    if (fields.at(y)->at(x)->getBuilding() != nullptr)
                    {
                        fields.at(y)->at(x)->getBuilding()->updateBuildingSprites();
                    }
                }
            }
        }
    }
    qint32 heigth = getMapHeight();
    qint32 width = getMapWidth();
    for (qint32 y = 0; y < heigth; y++)
    {
        for (qint32 x = 0; x < width; x++)
        {
            fields.at(y)->at(x)->syncAnimation();
        }
    }
}

Unit* GameMap::spawnUnit(qint32 x, qint32 y, QString unitID, Player* owner, qint32 range)
{
    qint32 heigth = getMapHeight();
    qint32 width = getMapWidth();
    if (range < 0)
    {
        range = width + heigth;
    }
    spPlayer pPlayer = nullptr;
    for (qint32 i = 0; i < players.size(); i++)
    {
        if (owner == players[i].get())
        {
            pPlayer = players[i];
            break;
        }
        else if (i == players.size() - 1)
        {
            // cancel since we have no owner for the unit
            return nullptr;
        }
    }
    spUnit pUnit = new Unit(unitID, pPlayer.get());
    MovementTableManager* pMovementTableManager = MovementTableManager::getInstance();
    QString movementType = pUnit->getMovementType();
    if (onMap(x, y))
    {
        spTerrain pTerrain = getTerrain(x, y);
        if ((pTerrain->getUnit() == nullptr) &&
            (pMovementTableManager->getBaseMovementPoints(movementType, pTerrain->getID()) > 0))
        {
            pTerrain->setUnit(pUnit);
            return pUnit.get();
        }
    }

    qint32 currentRadius = 0;
    qint32 x2 = 0;
    qint32 y2 = 0;
    bool found = false;
    while ((found == false) && (currentRadius <= range) && (range > 0))
    {
        currentRadius += 1;
        x2 = -currentRadius;
        y2 = 0;
        for (qint32 i = 0; i < currentRadius; i++)
        {
            x2 += 1;
            y2 += 1;
            if (onMap(x + x2, y + y2))
            {
                spTerrain pTerrain = getTerrain(x + x2 - currentRadius, y + y2);
                if ((pTerrain->getUnit() == nullptr) &&
                    (pMovementTableManager->getBaseMovementPoints(movementType, pTerrain->getID()) > 0))
                {
                    pTerrain->setUnit(pUnit);
                    return pUnit.get();
                }
            }
        }
        for (qint32 i = 0; i < currentRadius; i++)
        {
            x2 += 1;
            y2 -= 1;
            if (onMap(x + x2, y + y2))
            {
                spTerrain pTerrain = getTerrain(x + x2, y + y2);
                if ((pTerrain->getUnit() == nullptr) &&
                    (pMovementTableManager->getBaseMovementPoints(movementType, pTerrain->getID()) > 0))
                {
                    pTerrain->setUnit(pUnit);
                    return pUnit.get();
                }
            }
        }
        for (qint32 i = 0; i < currentRadius; i++)
        {
            x2 -= 1;
            y2 -= 1;
            if (onMap(x + x2, y + y2))
            {
                spTerrain pTerrain = getTerrain(x + x2, y + y2);
                if ((pTerrain->getUnit() == nullptr) &&
                    (pMovementTableManager->getBaseMovementPoints(movementType, pTerrain->getID()) > 0))
                {
                    pTerrain->setUnit(pUnit);
                    return pUnit.get();
                }
            }
        }
        for (qint32 i = 0; i < currentRadius; i++)
        {
            x2 -= 1;
            y2 += 1;
            if (onMap(x + x2, y + y2))
            {
                spTerrain pTerrain = getTerrain(x + x2, y + y2);
                if ((pTerrain->getUnit() == nullptr) &&
                    (pMovementTableManager->getBaseMovementPoints(movementType, pTerrain->getID()) > 0))
                {
                    pTerrain->setUnit(pUnit);
                    return pUnit.get();
                }
            }
        }

        if (currentRadius > getMapWidth() && currentRadius > getMapHeight())
        {
            break;
        }
    }
    return nullptr;
}

qint32 GameMap::getMapWidth() const
{
    return fields.at(0)->size();
}

qint32 GameMap::getMapHeight() const
{
    return fields.size();
}

qint32 GameMap::getBuildingCount(QString buildingID)
{
    qint32 ret = 0;
    qint32 width = getMapWidth();
    qint32 heigth = getMapHeight();
    for (qint32 x = 0; x < width; x++)
    {
        for (qint32 y = 0; y < heigth; y++)
        {
            Building* pBuilding = getTerrain(x, y)->getBuilding();
            if (pBuilding != nullptr)
            {
                if (pBuilding->getBuildingID() == buildingID)
                {
                    ret++;
                }
            }
        }
    }
    return ret;
}

void GameMap::getField(qint32& x, qint32& y, GameEnums::Directions direction)
{
    switch (direction)
    {
        case GameEnums::Directions_North:
        {
            y--;
            break;
        }
        case GameEnums::Directions_NorthEast:
        {
            x++;
            y--;
            break;
        }
        case GameEnums::Directions_East:
        {
            x++;
            break;
        }
        case GameEnums::Directions_SouthEast:
        {
            y++;
            x++;
            break;
        }
        case GameEnums::Directions_South:
        {
            y++;
            break;
        }
        case GameEnums::Directions_SouthWest:
        {
            y++;
            x--;
            break;
        }
        case GameEnums::Directions_West:
        {
            x--;
            break;
        }
        case GameEnums::Directions_NorthWest:
        {
            y--;
            x--;
            break;
        }
        default:
        {
            // do nothing
        }
    }
}

bool GameMap::onMap(qint32 x, qint32 y)
{
    qint32 heigth = getMapHeight();
    qint32 width = getMapWidth();
    if ((x >= 0) &&
        (y >= 0) &&
        (x < width) &&
        (y < heigth))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void GameMap::centerMap(qint32 x, qint32 y)
{
    Mainapp* pApp = Mainapp::getInstance();
    // draw point
    this->setPosition(pApp->getSettings()->getWidth() / 2.0f - x * this->getZoom() * Imagesize - Imagesize / 2.0f,
                      pApp->getSettings()->getHeight() / 2.0f - y * this->getZoom() * Imagesize - Imagesize / 2.0f);
}

void GameMap::moveMap(qint32 x, qint32 y)
{
    qint32 heigth = getMapHeight();
    qint32 width = getMapWidth();
    // draw point
    qint32 resX = this->getPosition().x + x;
    qint32 resY = this->getPosition().y + y;
    qint32 minVisible = 16 / m_zoom;
    Mainapp* pApp = Mainapp::getInstance();
    if (resX > pApp->getSettings()->getWidth()  - minVisible * m_zoom * Imagesize)
    {
        resX = pApp->getSettings()->getWidth() - minVisible * m_zoom * Imagesize;
    }
    if (resX < -m_zoom * Imagesize * width + minVisible * m_zoom * Imagesize)
    {
        resX = -m_zoom * Imagesize * width + minVisible * m_zoom * Imagesize;
    }
    if (resY > pApp->getSettings()->getHeight() - minVisible * m_zoom * Imagesize)
    {
        resY = pApp->getSettings()->getHeight() - minVisible * m_zoom * Imagesize;
    }
    if (resY < -m_zoom * Imagesize * heigth + minVisible * m_zoom * Imagesize)
    {
        resY = -m_zoom * Imagesize * heigth + minVisible * m_zoom * Imagesize;
    }

    this->setPosition(resX, resY);
}

void GameMap::zoom(float zoom)
{
    m_zoom += zoom * 0.125;
    // limit zoom
    if (m_zoom > 4.0f)
    {
        m_zoom = 4.0f;
    }
    else if (m_zoom < 0.5f)
    {
        m_zoom = 0.5f;
    }
    else
    {
        // all fine
    }
    this->setScale(m_zoom);
    moveMap(0, 0);
}

void GameMap::replaceTerrain(const QString& terrainID, qint32 x, qint32 y, bool useTerrainAsBaseTerrain, bool updateSprites)
{
    if (onMap(x, y))
    {
        spTerrain pTerrain = Terrain::createTerrain(terrainID, x, y);
        if (useTerrainAsBaseTerrain)
        {
            spTerrain pTerrainOld = fields.at(y)->at(x);
            this->removeChild(pTerrainOld);
            pTerrain->setBaseTerrain(pTerrainOld);
            fields.at(y)->replace(x, pTerrain);
            this->addChild(pTerrain);
            pTerrain->setPosition(x * Imagesize, y * Imagesize);
            pTerrain->setPriority(static_cast<qint32>(Mainapp::ZOrder::Terrain) + y);
        }
        else
        {
            spTerrain pTerrainOld = fields.at(y)->at(x);
            this->removeChild(pTerrainOld);
            fields.at(y)->replace(x, pTerrain);
            this->addChild(pTerrain);
            pTerrain->setPosition(x * Imagesize, y * Imagesize);
            pTerrain->setPriority(static_cast<qint32>(Mainapp::ZOrder::Terrain) + y);
        }
        if (updateSprites)
        {
            updateTerrain(x, y);
            this->updateSprites(x, y);
        }
    }
}


void GameMap::updateTerrain(qint32 x, qint32 y)
{
    for (qint32 xPos = x - 1; xPos <= x + 1; xPos++)
    {
        for (qint32 yPos = y - 1; yPos <= y + 1; yPos++)
        {
            if (!((xPos == x) && (yPos == y)))
            {
                if (onMap(xPos, yPos))
                {
                    if (!canBePlaced(fields.at(yPos)->at(xPos)->getTerrainID(), xPos, yPos))
                    {
                        replaceTerrain(fields.at(yPos)->at(xPos)->getBaseTerrainID(), xPos, yPos, false, true);
                    }
                }
            }
        }
    }
}

bool GameMap::canBePlaced(const QString& terrainID, qint32 x, qint32 y)
{
    Mainapp* pApp = Mainapp::getInstance();
    QString function = "canBePlaced";
    QJSValueList args;
    args << QJSValue(x);
    args << QJSValue(y);
    QJSValue placeable = pApp->getInterpreter()->doFunction(terrainID, function, args);
    if (placeable.isBool())
    {
        return placeable.toBool();
    }
    else
    {
        return false;
    }
}

void GameMap::serialize(QDataStream& pStream)
{
    qint32 heigth = getMapHeight();
    qint32 width = getMapWidth();
    // store header
    pStream << getVersion();
    pStream << mapName;
    pStream << width;
    pStream << heigth;
    pStream << getPlayerCount();
    qint32 currentPlayerIdx = 0;
    for (qint32 i = 0; i < players.size(); i++)
    {
        if (m_CurrentPlayer.get() == players[i].get())
        {
            currentPlayerIdx = i;
        }
        players[i]->serialize(pStream);
    }


    pStream << currentPlayerIdx;
    pStream << currentDay;
    // store map
    for (qint32 y = 0; y < heigth; y++)
    {
        for (qint32 x = 0; x < width; x++)
        {
            // serialize
            fields.at(y)->at(x)->serialize(pStream);
        }
    }
}

void GameMap::clearMap()
{
    // delete all data
    for (qint32 y = 0; y < fields.size(); y++)
    {
        for (qint32 x = 0; x < fields.at(y)->size(); x++)
        {
            this->removeChild(fields.at(y)->at(x));
        }
        fields.at(y)->clear();
    }
    fields.clear();
    players.clear();
}

QString GameMap::readMapName(QDataStream& pStream)
{
    QString name;
    // restore map header
    qint32 version = 0;
    pStream >> version;
    if (version > 1)
    {
        pStream >> name;
    }
    return name;
}

void GameMap::deserialize(QDataStream& pStream)
{
    clearMap();

    // restore map header
    qint32 version = 0;
    pStream >> version;
    if (version > 1)
    {
        pStream >> mapName;
    }
    qint32 heigth = 0;
    qint32 width = 0;
    pStream >> width;
    pStream >> heigth;
    qint32 playerCount = 0;
    pStream >> playerCount;
    for (qint32 i = 0; i < playerCount; i++)
    {
        // create new player
        players.append(new Player());
        // get player data from stream
        players[i]->deserialize(pStream);
    }

    qint32 currentPlayerIdx = 0;
    if (version > 1)
    {
        pStream >> currentPlayerIdx;
        pStream >> currentDay;
    }

    // restore map
    for (qint32 y = 0; y < heigth; y++)
    {
        fields.append(new QVector<spTerrain>());
        for (qint32 x = 0; x < width; x++)
        {
            spTerrain pTerrain = Terrain::createTerrain("", x, y);
            fields[y]->append(pTerrain);
            pTerrain->deserialize(pStream);
            this->addChild(pTerrain);
            pTerrain->setPosition(x * Imagesize, y * Imagesize);
            pTerrain->setPriority(static_cast<qint32>(Mainapp::ZOrder::Terrain) + y);
        }
    }    
    setCurrentPlayer(currentPlayerIdx);
}

qint32 GameMap::getImageSize()
{
    return Imagesize;
}

void GameMap::enableUnits(Player* pPlayer)
{
    qint32 heigth = getMapHeight();
    qint32 width = getMapWidth();
    for (qint32 y = 0; y < heigth; y++)
    {
        for (qint32 x = 0; x < width; x++)
        {
            spUnit pUnit = fields.at(y)->at(x)->getSpUnit();
            if (pUnit.get() != nullptr)
            {
                if (pUnit->getOwner() == pPlayer)
                {
                    pUnit->setHasMoved(false);
                }
            }
        }
    }
}

void GameMap::nextPlayer()
{
    for (qint32 i = 0; i < players.size(); i++)
    {
        if (players.at(i).get() == m_CurrentPlayer.get())
        {
            if (i == players.size() - 1)
            {
                m_CurrentPlayer = players[0];
                break;
            }
            else
            {
                m_CurrentPlayer = players[i + 1];
                break;
            }
        }
    }
}

void GameMap::startOfTurn(Player* pPlayer)
{
    qint32 heigth = getMapHeight();
    qint32 width = getMapWidth();
    for (qint32 y = 0; y < heigth; y++)
    {
        for (qint32 x = 0; x < width; x++)
        {
            spUnit pUnit = fields.at(y)->at(x)->getSpUnit();
            if (pUnit.get() != nullptr)
            {
                if (pUnit->getOwner() == pPlayer)
                {
                    pUnit->startOfTurn();
                }
            }
            spBuilding pBuilding = fields.at(y)->at(x)->getSpBuilding();
            if (pBuilding.get() != nullptr)
            {
                if (pBuilding->getOwner() == pPlayer)
                {
                    pBuilding->startOfTurn();
                }
            }
        }
    }
    m_CurrentPlayer->startOfTurn();
}

void GameMap::checkFuel(Player* pPlayer)
{
    qint32 heigth = getMapHeight();
    qint32 width = getMapWidth();
    for (qint32 y = 0; y < heigth; y++)
    {
        for (qint32 x = 0; x < width; x++)
        {
            spUnit pUnit = fields.at(y)->at(x)->getSpUnit();
            if (pUnit.get() != nullptr)
            {
                if ((pUnit->getOwner() == pPlayer) &&
                    (pUnit->getFuel() < 0))
                {
                    pUnit->killUnit();
                }
            }
        }
    }
}

QmlVectorUnit* GameMap::getUnits(Player* pPlayer)
{
    qint32 heigth = getMapHeight();
    qint32 width = getMapWidth();
    QmlVectorUnit* ret = new QmlVectorUnit();
    for (qint32 y = 0; y < heigth; y++)
    {
        for (qint32 x = 0; x < width; x++)
        {
            spUnit pUnit = fields.at(y)->at(x)->getSpUnit();
            if (pUnit.get() != nullptr)
            {
                if ((pUnit->getOwner() == pPlayer))
                {
                   ret->append(pUnit.get());
                }
            }
        }
    }
    return ret;
}

QmlVectorBuilding* GameMap::getBuildings(Player* pPlayer)
{
    qint32 heigth = getMapHeight();
    qint32 width = getMapWidth();
    QmlVectorBuilding* ret = new QmlVectorBuilding();
    for (qint32 y = 0; y < heigth; y++)
    {
        for (qint32 x = 0; x < width; x++)
        {
            spBuilding pBuilding = fields.at(y)->at(x)->getSpBuilding();
            if (pBuilding.get() != nullptr)
            {
                if ((pBuilding->getOwner() == pPlayer))
                {
                   ret->append(pBuilding.get());
                }
            }
        }
    }
    return ret;
}

QString GameMap::getMapName() const
{
    return mapName;
}

void GameMap::setMapName(const QString &value)
{
    mapName = value;
}

void GameMap::nextTurn()
{
    enableUnits(m_CurrentPlayer.get());
    Mainapp* pApp = Mainapp::getInstance();
    pApp->getAudioThread()->clearPlayList();
    nextPlayer();
    m_CurrentPlayer->earnMoney();
    startOfTurn(m_CurrentPlayer.get());
    checkFuel(m_CurrentPlayer.get());
    GameMenue::getInstance()->updatePlayerinfo();

    pApp->getAudioThread()->playRandom();
}
