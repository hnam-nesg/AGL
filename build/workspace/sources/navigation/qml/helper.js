
function roundNumber(number, digits)
{
    var multiple = Math.pow(10, digits);
    return Math.round(number * multiple) / multiple;
}

function formatTime(sec)
{
    var totalSeconds = Number(sec)
    if (!isFinite(totalSeconds) || totalSeconds < 0)
        return ""

    var totalMinutes = Math.max(1, Math.round(totalSeconds / 60))
    var hours = Math.floor(totalMinutes / 60)
    var minutes = totalMinutes % 60
    return hours > 0 ? hours + "h " + minutes + "m" : minutes + " min"
}

function formatDistance(meters)
{
    var dist = Number(meters)
    if (!isFinite(dist) || dist < 0)
        return ""

    if (dist >= 1000) {
        var km = dist / 1000
        return (km >= 100 ? Math.round(km) : Math.round(km * 10) / 10) + " km"
    }

    return Math.round(dist) + " m"
}
