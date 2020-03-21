var Constructor = function()
{
    this.getCOStyles = function()
    {
        // string array containing the endings of the alternate co style
        // e.g. co_adder has ["+alt"]
        return ["+alt"];
    };

    this.init = function(co)
    {
        co.setPowerStars(2);
        co.setSuperpowerStars(4);
    };

    this.activatePower = function(co)
    {
        var dialogAnimation = co.createPowerSentence();
        var powerNameAnimation = co.createPowerScreen(GameEnums.PowerMode_Power);
        dialogAnimation.queueAnimation(powerNameAnimation);

        var animation = GameAnimationFactory.createAnimation(0, 0);
        animation.addSprite2("white_pixel", 0, 0, 3200, map.getMapWidth(), map.getMapHeight());
        animation.addTweenColor(0, "#00FFFFFF", "#FFFFFFFF", 3000, true);
        powerNameAnimation.queueAnimation(animation);

        var player = co.getOwner();
        var funds = player.getFunds();
        if (funds > 0)
        {
            player.addFunds(player.getFunds() * 0.5);
        }
        audio.clearPlayList();
        CO_COLIN.loadCOMusic(co);
        audio.playRandom();
    };

    this.activateSuperpower = function(co, powerMode)
    {
        var dialogAnimation = co.createPowerSentence();
        var powerNameAnimation = co.createPowerScreen(powerMode);
        powerNameAnimation.queueAnimationBefore(dialogAnimation);

        var units = co.getOwner().getUnits();
        var animations = [];
        var counter = 0;
        units.randomize();
        for (var i = 0; i < units.size(); i++)
        {
            var unit = units.at(i);

            var animation = GameAnimationFactory.createAnimation(unit.getX(), unit.getY());

            if (animations.length < 5)
            {
                animation.addSprite("power12", -map.getImageSize() * 2, -map.getImageSize() * 2, 0, 1.5, globals.randInt(0, 400));
                powerNameAnimation.queueAnimation(animation);
                animations.push(animation);
            }
            else
            {
                animation.addSprite("power12", -map.getImageSize() * 2, -map.getImageSize() * 2, 0, 1.5);
                animations[counter].queueAnimation(animation);
                animations[counter] = animation;
                counter++;
                if (counter >= animations.length)
                {
                    counter = 0;
                }
            }
        }
        units.remove();

        audio.clearPlayList();
        CO_COLIN.loadCOMusic(co);
        audio.playRandom();
    };

    this.loadCOMusic = function(co)
    {
        // put the co music in here.
        switch (co.getPowerMode())
        {
            case GameEnums.PowerMode_Power:
                audio.addMusic("resources/music/cos/power.mp3", 992, 45321);
                break;
            case GameEnums.PowerMode_Superpower:
                audio.addMusic("resources/music/cos/superpower.mp3", 1505, 49515);
                break;
            case GameEnums.PowerMode_Tagpower:
                audio.addMusic("resources/music/cos/tagpower.mp3", 14611, 65538);
                break;
            default:
                audio.addMusic("resources/music/cos/colin.mp3", 6900, 75513)
                break;
        }
    };

    this.getCOUnitRange = function(co)
    {
        return 1;
    };
    this.getCOArmy = function()
    {
        return "BM";
    };
    this.getCostModifier = function(co, id, baseCost)
    {
        return -baseCost * 0.2;
    };
    this.getOffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                 defender, defPosX, defPosY, isDefender)
    {
        switch (co.getPowerMode())
        {
            case GameEnums.PowerMode_Tagpower:
            case GameEnums.PowerMode_Superpower:
                var bonus = attacker.getOwner().getFunds() / 1000 * 3.33 - 5;
                return bonus;
            case GameEnums.PowerMode_Power:
                return -5;
            default:
                break;
        }
        if (co.inCORange(Qt.point(atkPosX, atkPosY), attacker))
        {
            return -5;
        }
        return -15;
    };

    this.getDeffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                       defender, defPosX, defPosY, isDefender)
    {
        if (co.inCORange(Qt.point(defPosX, defPosY), defender) ||
                co.getPowerMode() > GameEnums.PowerMode_Off)
        {
            return 10;
        }
        return 0;
    };

    // CO - Intel
    this.getBio = function()
    {
        return qsTr("Blue Moon's rich boy CO and Sasha's little brother. A gifted CO with a sharp, if insecure, mind.");
    };
    this.getHits = function()
    {
        return qsTr("Olaf and Grit");
    };
    this.getMiss = function()
    {
        return qsTr("Black Hole");
    };
    this.getCODescription = function()
    {
        return qsTr("The heir to a vast fortune who can purchase units at bargain-basement prices. Troops' low firepower stems from his lack of confidence.");
    };
    this.getLongCODescription = function()
    {
        return qsTr("\nGlobal Effect: \nUnits are 20% cheaper and have 15% less firepower.") +
               qsTr("\n\nCO Zone Effect: \nUnits have only 5% weaker firepower.");
    };
    this.getPowerDescription = function()
    {
        return qsTr("Increases current funds by 50 percent.");
    };
    this.getPowerName = function()
    {
        return qsTr("Gold Rush");
    };
    this.getSuperPowerDescription = function()
    {
        return qsTr("Uses wealth to increase the strength of units. The more funds available, the more firepower his units receive.");
    };
    this.getSuperPowerName = function()
    {
        return qsTr("Power of Money");
    };
    this.getPowerSentences = function()
    {
        return [qsTr("Let me show you the power of money!"),
                qsTr("This is not a drill... I won't give up so easily!"),
                qsTr("People of Blue Moon!  I need your help!"),
                qsTr("I'll give it everything I've got!"),
                qsTr("You're not getting away with this!"),
                qsTr("I'll show you what I can do!")];
    };
    this.getVictorySentences = function()
    {
        return [qsTr("I w-won! Whew!"),
                qsTr("I'll win if I try my best!"),
                qsTr("Whew... I won! I really won!")];
    };
    this.getDefeatSentences = function()
    {
        return [qsTr("If only Commander Olaf were here..."),
                qsTr("Wait till I tell my sis! Then you'll be sorry!")];
    };
    this.getName = function()
    {
        return qsTr("Colin");
    };
}

Constructor.prototype = CO;
var CO_COLIN = new Constructor();
