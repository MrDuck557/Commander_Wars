FIRE.getVision = function(player)
{
    if (map.getGameRules().getCurrentWeather().getWeatherId() === "WEATHER_RAIN" &&
        !player.getWeatherImmune())
    {
        return -1;
    }
    else
    {
        return 5;
    }
};
