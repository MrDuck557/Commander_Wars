#ifndef SCRIPTCONDITION_H
#define SCRIPTCONDITION_H

#include <QObject>

#include <QVector>

#include <QTextStream>

#include "oxygine-framework.h"

#include "ingamescriptsupport/events/scriptevent.h"

class ScriptCondition;
typedef oxygine::intrusive_ptr<ScriptCondition> spScriptCondition;

class ScriptEditor;
typedef oxygine::intrusive_ptr<ScriptEditor> spScriptEditor;

class ScriptCondition : public QObject, public oxygine::ref_counter
{
    Q_OBJECT
public:
    enum class ConditionType
    {
        startOfTurn,
        victory,
        eachDay,
        unitDestroyed,
        buildingDestroyed,
        buildingCaptured,
        playerDefeated,
        unitsDestroyed,
        buildingsOwned,
        playerReachedArea,
        unitReachedArea
    };
    static const QString ConditionVictory;
    static const QString ConditionStartOfTurn;
    static const QString ConditionEachDay;
    static const QString ConditionUnitDestroyed;
    static const QString ConditionBuildingDestroyed;
    static const QString ConditionBuildingCaptured;
    static const QString ConditionPlayerDefeated;
    static const QString ConditionUnitsDestroyed;
    static const QString ConditionBuildingsOwned;
    static const QString ConditionPlayerReachedArea;
    static const QString ConditionUnitReachedArea;

    /**
     * @brief createCondition
     * @param type
     * @return
     */
    static ScriptCondition* createCondition(ConditionType type);
    /**
     * @brief createReadCondition
     * @param rStream
     * @return
     */
    static ScriptCondition* createReadCondition(QTextStream& rStream);
    /**
     * @brief readCondition
     * @param rStream
     */
    virtual void readCondition(QTextStream& rStream) = 0;
    /**
     * @brief writeCondition
     * @param rStream
     */
    virtual void writeCondition(QTextStream& rStream) = 0;
    /**
     * @brief writePreCondition
     * @param rStream
     */
    virtual void writePreCondition(QTextStream& rStream);
    /**
     * @brief writePostCondition
     * @param rStream
     */
    virtual void writePostCondition(QTextStream& rStream);
    /**
     * @brief addEvent
     * @param event
     */
    void addEvent(spScriptEvent event);
    /**
     * @brief getEvent
     * @param index
     * @return
     */
    spScriptEvent getEvent(qint32 index);
    /**
     * @brief getEventSize
     * @return
     */
    qint32 getEventSize()
    {
        return events.size();
    }
    /**
     * @brief removeEvent
     * @param pEvent
     */
    void removeEvent(spScriptEvent pEvent);
    /**
     * @brief getType
     * @return
     */
    ConditionType getType() const;
    /**
     * @brief getDescription
     * @return
     */
    virtual QString getDescription() = 0;
    /**
     * @brief showEditCondition
     */
    virtual void showEditCondition(spScriptEditor pScriptEditor) = 0;
    /**
     * @brief getSubCondition
     * @return
     */
    spScriptCondition getSubCondition() const;
    /**
     * @brief setSubCondition
     * @param value
     */
    void setSubCondition(const spScriptCondition &value);
    /**
     * @brief sameConditionGroup
     * @param type1
     * @param type2
     * @return
     */
    static bool sameConditionGroup(ConditionType type1, ConditionType type2);
    /**
     * @brief getParent
     * @return
     */
    ScriptCondition *getParent() const;
    /**
     * @brief setParent
     * @param value
     */
    void setParent(ScriptCondition *value);
    /**
     * @brief getVersion
     * @return
     */
    virtual qint32 getVersion() = 0;
    /**
     * @brief readSubCondition
     * @param rStream
     * @param id
     * @return
     */
    bool readSubCondition(QTextStream& rStream, QString id);
protected:
    explicit ScriptCondition(ConditionType type);
    QVector<spScriptEvent> events;
    spScriptCondition subCondition;
    ScriptCondition* pParent{nullptr};

    ConditionType m_Type;
};

#endif // SCRIPTCONDITION_H
