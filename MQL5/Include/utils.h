#ifndef UTILS_H
#define UTILS_H

#property copyright "Copyright 2024, Mogyo Software Corp."
#property link      "http://www.mogyo.com"

//+------------------------------------------------------------------+
#define SF StringFormat
//+------------------------------------------------------------------+
bool isLOG() {
// if (TimeCurrent() > D'2022.06.13')
// if (TimeCurrent() < D'2022.12.01')
    return true;
    // return MQLInfoInteger(MQL_TESTER) != 0;
}
void LOG_Naked(const string s) {
    if (isLOG()) Print(s);
}
#define LOG(s) if (isLOG()) PrintFormat("%d: %s", __LINE__, s)
#define LOGF(s) if (isLOG()) PrintFormat("%d: %s: %s", __LINE__, __FUNCTION__, s)
//+------------------------------------------------------------------+
#define DAYS *24*60*60
#define HOURS *60*60
//+------------------------------------------------------------------+
string d2str(const double d, bool human = true) {
    const string ThSep = ",";

    if (d < 0) return "-" + d2str(-d, human);
    ulong i = (ulong)MathFloor(d);

    if (i < 1000) {
        return (string)i;
    }
    ulong thousands = i / 1000;
    int u = int(i - thousands * 1000);
    if (thousands < 1000) {
        return (string)thousands + ThSep + SF("%03u", u);
    }
    uint millions = uint(thousands / 1000);
    thousands -= millions * 1000;
    if (millions < 1000) {
        if (human) return (string)millions + ThSep + SF("%02d", thousands/10) + "m";
        return (string)millions + ThSep + SF("%03d", thousands) + ThSep + SF("%03u", u);
    }
    uint trillions = millions / 1000;
    millions -= trillions * 1000;
    if (human) return (string)trillions + ThSep + SF("%02d", millions/10) + "tr";
    return (string)trillions + ThSep + SF("%03u", millions) + ThSep + SF("%03u", thousands) + ThSep + SF("%03u", u);
}
//+------------------------------------------------------------------+
bool isNewMinute() {
    static datetime oldTime = 0;

    datetime now = TimeCurrent();

    datetime nowMinutes = now / 60;
    datetime oldMinutes = oldTime / 60;

    oldTime = now;

    return nowMinutes != oldMinutes;
}
//+------------------------------------------------------------------+
string secToStr(const int totalSeconds) {
    int minutes = totalSeconds / 60;
    int hours   = minutes / 60;
    int days    = hours / 24;
    int seconds = totalSeconds - minutes*60;
    minutes -= hours * 60;
    hours   -= days * 24;

    if (days > 0 && hours > 0) return SF("%2dd %dh", days, hours);
    if (days > 0) return SF("%2dd", days);

    if (hours > 0 && seconds > 0) return SF("%2dh %dm %ds", hours, minutes, seconds);
    if (hours > 0 && minutes > 0) return SF("%2dh %dm", hours, minutes);
    if (hours > 0) return SF("%2dh", hours);
    if (minutes > 0 && seconds > 0) return SF("%2dm %ds", minutes, seconds);
    if (minutes > 0) return SF("%2dm", minutes);
    return SF("%2ds", seconds);
}
//+------------------------------------------------------------------+
int timeDiff(datetime const &then) {
    return int(TimeCurrent() - then);
}
//+------------------------------------------------------------------+
string timeDiffToStr(datetime const &then) {
    return secToStr(timeDiff(then));
}
//+------------------------------------------------------------------+
#endif