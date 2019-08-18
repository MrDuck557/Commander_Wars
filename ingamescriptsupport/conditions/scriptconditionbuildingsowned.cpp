#include "scriptconditionbuildingsowned.h"

#include "ingamescriptsupport/scriptdata.h"
#include "ingamescriptsupport/scripteditor.h"

#include "ingamescriptsupport/genericbox.h"

#include "resource_management/fontmanager.h"

#include "coreengine/mainapp.h"

#include "objects/spinbox.h"

ScriptConditionBuildingsOwned::ScriptConditionBuildingsOwned()
    : ScriptCondition(ConditionType::buildingsOwned)
{

}

qint32 ScriptConditionBuildingsOwned::getCount() const
{
    return m_count;
}

void ScriptConditionBuildingsOwned::setCount(const qint32 &count)
{
    m_count = count;
}

qint32 ScriptConditionBuildingsOwned::getPlayer() const
{
    return m_player;
}

void ScriptConditionBuildingsOwned::setPlayer(const qint32 &player)
{
    m_player = player;
}

void ScriptConditionBuildingsOwned::readCondition(QTextStream& rStream)
{
    QString line = rStream.readLine().simplified();
    QStringList items = line.replace("if (map.getPlayer(", "")
                            .replace(").getBuildingCount() >= ", ",")
                            .replace(" && ", ",").split(",");
    if (items.size() >= 2)
    {
        m_player = items[0].toInt();
        m_count = items[1].toInt();
    }
    while (!rStream.atEnd())
    {
        qint64 pos = rStream.pos();
        line = rStream.readLine().simplified();
        if (line.endsWith(ConditionBuildingsOwned + " End"))
        {
            break;
        }
        else
        {
            rStream.seek(pos);
        }
        if (subCondition.get() == nullptr)
        {
            setSubCondition(createReadCondition(rStream));
        }
        spScriptEvent event = ScriptEvent::createReadEvent(rStream);
        if (event.get() != nullptr)
        {
            events.append(event);
        }
    }
}

void ScriptConditionBuildingsOwned::writePreCondition(QTextStream& rStream)
{
    m_executed = ScriptData::getVariableName();
    rStream << "        var " << m_executed << " = variables.createVariable(\"" << m_executed << "\");\n";
    if (subCondition.get() != nullptr)
    {
        subCondition->writePreCondition(rStream);
    }
}

void ScriptConditionBuildingsOwned::writeCondition(QTextStream& rStream)
{
    rStream << "        if (map.getPlayer(" << QString::number(m_player) << ").getBuildingCount() >= " << QString::number(m_count)
            << " && " << m_executed << ".readDataBool() === false) {"
            << "// " << ConditionBuildingsOwned << "\n";
    if (subCondition.get() != nullptr)
    {
        subCondition->writeCondition(rStream);
    }
    else if (pParent != nullptr)
    {
        pParent->writePostCondition(rStream);
        for (qint32 i = 0; i < events.size(); i++)
        {
            events[i]->writeEvent(rStream);
        }
        rStream << "            " << m_executed << ".writeDataBool(true);\n";
    }
    else
    {
        for (qint32 i = 0; i < events.size(); i++)
        {
            events[i]->writeEvent(rStream);
        }
        rStream << "            " << m_executed << ".writeDataBool(true);\n";
    }
    rStream << "        } // " + ConditionBuildingsOwned + " End\n";
}

void ScriptConditionBuildingsOwned::writePostCondition(QTextStream& rStream)
{
    ScriptCondition::writePostCondition(rStream);
    for (qint32 i = 0; i < events.size(); i++)
    {
        events[i]->writeEvent(rStream);
    }
    rStream << "            " << m_executed << ".writeDataBool(true);\n";
}

void ScriptConditionBuildingsOwned::showEditCondition(spScriptEditor pScriptEditor)
{
    spGenericBox pBox = new GenericBox();

    oxygine::TextStyle style = FontManager::getMainFont();
    style.color = oxygine::Color(255, 255, 255, 255);
    style.vAlign = oxygine::TextStyle::VALIGN_TOP;
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    style.multiline = false;

    qint32 width = 300;

    oxygine::spTextField pText = new oxygine::TextField();
    pText->setStyle(style);
    pText->setHtmlText(tr("Count: ").toStdString().c_str());
    pText->setPosition(30, 30);
    pBox->addItem(pText);
    spSpinBox spinBox = new SpinBox(150, 0, 99999);
    spinBox->setPosition(width, 30);
    spinBox->setCurrentValue(m_count);
    connect(spinBox.get(), &SpinBox::sigValueChanged,
            [=](qreal value)
    {
        setCount(static_cast<qint32>(value));
    });
    pBox->addItem(spinBox);

    pText = new oxygine::TextField();
    pText->setStyle(style);
    pText->setHtmlText(tr("Player: ").toStdString().c_str());
    pText->setPosition(30, 70);
    pBox->addItem(pText);
    spinBox = new SpinBox(150, 1, 99999);
    spinBox->setPosition(width, 70);
    spinBox->setCurrentValue(m_player + 1);
    connect(spinBox.get(), &SpinBox::sigValueChanged,
            [=](qreal value)
    {
        setPlayer(static_cast<qint32>(value) - 1);
    });
    pBox->addItem(spinBox);

    pScriptEditor->addChild(pBox);
    connect(pBox.get(), &GenericBox::sigFinished, pScriptEditor.get(), &ScriptEditor::updateConditios, Qt::QueuedConnection);
}
