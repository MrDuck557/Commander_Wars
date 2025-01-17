#ifndef VIEWPLAYER_H
#define VIEWPLAYER_H

#include "game/player.h"
#include "gameinput/humanplayerinput.h"

class GameMenue;
class Viewplayer;
using spViewplayer = std::shared_ptr<Viewplayer>;

class Viewplayer final : public Player
{
    Q_OBJECT
public:
    enum ViewType
    {
        CurrentTeam = -3,
        AllTeams  = -2,
        Map  = -1,
    };

    explicit Viewplayer(GameMenue* pMenu, GameMap* pMap);
    virtual ~Viewplayer() = default;
    /**
     * @brief getColorTableAnim
     * @return
     */
    virtual oxygine::spResAnim getColorTableAnim() const override;
    /**
     * @brief getFieldVisible
     * @param x
     * @param y
     */
    Q_INVOKABLE virtual bool getFieldVisible(qint32 x, qint32 y) override;
    /**
     * @brief getFieldVisibleType
     * @param x
     * @param y
     * @return
     */
    Q_INVOKABLE virtual GameEnums::VisionType getFieldVisibleType(qint32 x, qint32 y) const override;
    /**
     * @brief getFieldDirectVisible
     * @param x
     * @param y
     * @return if true a stealthed unit will be made visible by this players view
     */
    Q_INVOKABLE virtual bool getFieldDirectVisible(qint32 x, qint32 y) override;
    /**
     * @brief getViewType
     * @return
     */
    Q_INVOKABLE qint32 getViewType() const;
    /**
     * @brief setViewType
     * @param ViewType
     */
    Q_INVOKABLE void setViewType(const qint32 ViewType);
private:
    qint32 m_ViewType{static_cast<qint32>(ViewType::CurrentTeam)};
    HumanPlayerInput m_input;
};

#endif // VIEWPLAYER_H
