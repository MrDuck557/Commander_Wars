#include "perkselectiondialog.h"

#include "resource_management/objectmanager.h"
#include "resource_management/fontmanager.h"

#include "objects/base/label.h"
#include "objects/base/dropdownmenu.h"
#include "objects/dialogs/dialogtextinput.h"

#include "game/gamemap.h"

#include "coreengine/filesupport.h"

PerkSelectionDialog::PerkSelectionDialog(GameMap* pMap, Player* pPlayer, qint32 maxPerkcount, bool banning, QStringList hiddenList)
    : m_pPlayer(pPlayer),
      m_banning(banning),
      m_pMap(pMap)
{
    setObjectName("PerkSelectionDialog");
    Interpreter::setCppOwnerShip(this);
    Mainapp* pApp = Mainapp::getInstance();
    moveToThread(pApp->getWorkerthread());

    ObjectManager* pObjectManager = ObjectManager::getInstance();
    oxygine::spBox9Sprite pSpriteBox = oxygine::spBox9Sprite::create();
    oxygine::ResAnim* pAnim = pObjectManager->getResAnim("codialog");
    pSpriteBox->setResAnim(pAnim);
    pSpriteBox->setSize(Settings::getWidth(), Settings::getHeight());
    addChild(pSpriteBox);
    pSpriteBox->setPosition(0, 0);
    pSpriteBox->setPriority(static_cast<qint32>(Mainapp::ZOrder::Objects));
    setPriority(static_cast<qint32>(Mainapp::ZOrder::Dialogs));

    // ok button
    m_OkButton = pObjectManager->createButton(tr("Ok"), 150);
    m_OkButton->setPosition(Settings::getWidth() / 2 - m_OkButton->getWidth() / 2, Settings::getHeight() - 30 - m_OkButton->getHeight());
    pSpriteBox->addChild(m_OkButton);

    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
    style.color = FontManager::getFontColor();
    style.vAlign = oxygine::TextStyle::VALIGN_DEFAULT;
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;

    CO* firstCO = nullptr;
    if (!banning)
    {
        spLabel pLabel = spLabel::create(200);
        pLabel->setStyle(style);
        pLabel->setHtmlText("Perk's of CO:");
        pLabel->setPosition(30, 30);
        pSpriteBox->addChild(pLabel);
        QStringList list;
        CO* pCO = pPlayer->getCO(0);

        if (pCO != nullptr)
        {
            firstCO = pCO;
            list.append(pCO->getCOName());
        }
        pCO = pPlayer->getCO(1);
        if (pCO != nullptr)
        {
            list.append(pCO->getCOName());
            if (firstCO == nullptr)
            {
                firstCO = pCO;
            }
        }
        spDropDownmenu pDropDownmenu = spDropDownmenu::create(250, list);
        pDropDownmenu->setPosition(210, 30);
        pSpriteBox->addChild(pDropDownmenu);
        connect(pDropDownmenu.get(), &DropDownmenu::sigItemChanged, this, &PerkSelectionDialog::changeCO, Qt::QueuedConnection);

        pLabel = spLabel::create(100);
        pLabel->setStyle(style);
        pLabel->setHtmlText("Fill:");
        pLabel->setPosition(pDropDownmenu->getX() + pDropDownmenu->getWidth() + 10, 30);
        pSpriteBox->addChild(pLabel);
        m_randomFillCheckbox = spCheckbox::create();
        m_randomFillCheckbox->setTooltipText(tr("If checked clicking the random button. The selected perks are filled up to the maximum."));
        m_randomFillCheckbox->setPosition(pLabel->getX() + pLabel->getWidth() + 10, 30);
        pSpriteBox->addChild(m_randomFillCheckbox);
        oxygine::spButton randomButton = pObjectManager->createButton(tr("Random"), 150);
        randomButton->setPosition(m_randomFillCheckbox->getX() + m_randomFillCheckbox->getWidth() + 10, 30);
        pSpriteBox->addChild(randomButton);
        randomButton->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event*)
        {
            emit sigSelectRandomPerks();
        });
        connect(this, &PerkSelectionDialog::sigSelectRandomPerks, this, &PerkSelectionDialog::selectRandomPerks, Qt::QueuedConnection);
    }

    QSize size(Settings::getWidth() - 60, Settings::getHeight() - 40 * 3 - m_OkButton->getHeight());
    m_pPanel = spPanel::create(true, size, size);
    if (banning)
    {
        m_pPanel->setPosition(30, 30);
    }
    else
    {
        m_pPanel->setPosition(30, 75);
    }
    m_pPerkSelection = spPerkSelection::create(firstCO, Settings::getWidth() - 80, maxPerkcount, banning, hiddenList, m_pMap);
    m_pPanel->addItem(m_pPerkSelection);
    m_pPanel->setContentHeigth(m_pPerkSelection->getHeight() + 40);
    m_pPanel->setContentWidth(m_pPerkSelection->getWidth());
    pSpriteBox->addChild(m_pPanel);

    if (banning)
    {
        m_OkButton->setPosition(Settings::getWidth() - m_OkButton->getWidth() - 30, Settings::getHeight() - 30 - m_OkButton->getHeight());

        m_CancelButton = pObjectManager->createButton(tr("Cancel"), 150);
        m_CancelButton->setPosition(30, Settings::getHeight() - 30 - m_CancelButton->getHeight());
        pSpriteBox->addChild(m_CancelButton);
        m_CancelButton->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event*)
        {
            emit sigCancel();
        });

        oxygine::spButton pSave = pObjectManager->createButton(tr("Save"), 150);
        pSave->setPosition(Settings::getWidth() / 2 - pSave->getWidth() / 2, Settings::getHeight() - 30 - m_CancelButton->getHeight());
        pSave->addClickListener([this](oxygine::Event*)
        {
            emit sigShowSavePerklist();
        });
        pSpriteBox->addChild(pSave);
        connect(this, &PerkSelectionDialog::sigShowSavePerklist, this, &PerkSelectionDialog::showSavePerklist, Qt::QueuedConnection);

        m_ToggleAll = pObjectManager->createButton(tr("Un/Select All"), 180);
        m_ToggleAll->setPosition(Settings::getWidth() / 2 + 60 , Settings::getHeight() - 75 - m_ToggleAll->getHeight());
        pSpriteBox->addChild(m_ToggleAll);
        m_ToggleAll->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event*)
        {
            m_toggle = !m_toggle;
            emit sigToggleAll(m_toggle);
        });
        connect(this, &PerkSelectionDialog::sigToggleAll, m_pPerkSelection.get(), &PerkSelection::toggleAll, Qt::QueuedConnection);
        auto items = getNameList("data/perkbannlist/");
        m_PredefinedLists = spDropDownmenu::create(260, items);

        m_PredefinedLists->setPosition(Settings::getWidth() / 2 + 40 - m_PredefinedLists->getWidth(), Settings::getHeight() - 75 - m_ToggleAll->getHeight());
        pSpriteBox->addChild(m_PredefinedLists);
        connect(m_PredefinedLists.get(), &DropDownmenu::sigItemChanged, this, &PerkSelectionDialog::setPerkBannlist, Qt::QueuedConnection);
    }
    else
    {
        m_OkButton->setPosition(Settings::getWidth() - m_OkButton->getWidth() - 30, Settings::getHeight() - 30 - m_OkButton->getHeight());

        oxygine::spButton pSave = pObjectManager->createButton(tr("Save"), 150);
        pSave->setPosition(Settings::getWidth() / 2 + 60 , Settings::getHeight() - 30 - pSave->getHeight());
        pSave->addClickListener([this](oxygine::Event*)
        {
            emit sigShowSavePerklist();
        });
        pSpriteBox->addChild(pSave);
        connect(this, &PerkSelectionDialog::sigShowSavePerklist, this, &PerkSelectionDialog::showSavePerklist, Qt::QueuedConnection);
        auto items = getNameList("data/perkselection/");
        m_PredefinedLists = spDropDownmenu::create(260, items);

        m_PredefinedLists->setPosition(Settings::getWidth() / 2 + 40 - m_PredefinedLists->getWidth(), Settings::getHeight() - 30 - pSave->getHeight());
        pSpriteBox->addChild(m_PredefinedLists);
        connect(m_PredefinedLists.get(), &DropDownmenu::sigItemChanged, this, &PerkSelectionDialog::setPerkBannlist, Qt::QueuedConnection);
    }

    auto* pPerkSelection = m_pPerkSelection.get();
    m_OkButton->addEventListener(oxygine::TouchEvent::CLICK, [this, pPerkSelection](oxygine::Event*)
    {
        emit editFinished(pPerkSelection->getPerks());
        emit sigFinished();
    });

    connect(this, &PerkSelectionDialog::sigCancel, this, &PerkSelectionDialog::remove, Qt::QueuedConnection);
    connect(this, &PerkSelectionDialog::sigFinished, this, &PerkSelectionDialog::remove, Qt::QueuedConnection);
}

void PerkSelectionDialog::remove()
{
    detach();
}

void PerkSelectionDialog::setPerkBannlist(qint32)
{
    if (m_banning)
    {
        QString file = m_PredefinedLists->getCurrentItemText();
        auto fileData = Filesupport::readList(file + ".bl", "data/perkbannlist/");
        m_pPerkSelection->setPerks(fileData.items);
    }
    else
    {
        QString file = m_PredefinedLists->getCurrentItemText();
        auto fileData = Filesupport::readList(file + ".bl", "data/perkselection/");
        QStringList perks = fileData.items;
        qint32 i = 0;
        
        while (i < perks.size())
        {
            if (m_pMap->getGameRules()->getAllowedPerks().contains(perks[i]))
            {
                ++i;
            }
            else
            {
                perks.removeAt(i);
            }
        }
        qint32 maxPerkCount = m_pMap->getGameRules()->getMaxPerkCount();
        while (perks.size() > maxPerkCount)
        {
            perks.removeLast();
        }
        m_pPerkSelection->setPerks(perks);
    }
}

void PerkSelectionDialog::changeCO(qint32 index)
{
    CO* pCO = nullptr;
    if (index == 0)
    {
        pCO = m_pPlayer->getCO(index);
        if (pCO == nullptr)
        {
            pCO = m_pPlayer->getCO(1);
        }
    }
    else
    {
        pCO = m_pPlayer->getCO(index);
    }
    m_pPerkSelection->updatePerksView(pCO);
}

void PerkSelectionDialog::showSavePerklist()
{    
    spDialogTextInput pSaveInput = spDialogTextInput::create(tr("Perklist Name"), true, "");
    connect(pSaveInput.get(), &DialogTextInput::sigTextChanged, this, &PerkSelectionDialog::savePerklist, Qt::QueuedConnection);
    addChild(pSaveInput);
}

QStringList PerkSelectionDialog::getNameList(QString path)
{
    QStringList items;
    QStringList filters;
    filters << "*.bl";
    QDirIterator dirIter(path, filters, QDir::Files, QDirIterator::IteratorFlag::NoIteratorFlags);
    while (dirIter.hasNext())
    {
        dirIter.next();
        QString file = dirIter.fileInfo().absoluteFilePath();
        auto data = Filesupport::readList(file);
        items.append(data.name);
    }
    return items;
}

void PerkSelectionDialog::savePerklist(QString filename)
{
    QString path;
    if (m_banning)
    {
        path = "data/perkbannlist/";
    }
    else
    {
        path = "data/perkselection/";
    }
    Filesupport::storeList(filename, m_pPerkSelection->getPerks(), path);
    auto items = getNameList(path);
    m_PredefinedLists->changeList(items);
}

void PerkSelectionDialog::selectRandomPerks()
{
    m_pPerkSelection->selectRandomPerks(m_randomFillCheckbox->getChecked());
}
