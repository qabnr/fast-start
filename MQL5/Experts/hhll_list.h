//+------------------------------------------------------------------+
// HHLL_list.h
//+------------------------------------------------------------------+

#ifndef HHLL_LIST_H
#define HHLL_LIST_H

class HHLLlist
{
private:
    #define arrSize 25

    uint latest;
    uint oldest;

    double lastPrice;
    double prevL;
    double prevH;

    enum { 
        none = 0,
        withTimeStamp,
        withTimeDiff
    };

    struct HHLL_data
    {
        bool isValid;
        bool isHigh;
        bool isHigher;
        datetime timeStamp;

        void change2higher(void) {
            isHigher = true;
            timeStamp = TimeCurrent();
        }

        void change2lower(void) {
            isHigher = false;
            timeStamp = TimeCurrent();
        }

        void updateTimestamp(void) {
            timeStamp = TimeCurrent();
        }

        string toStr(int with = none) {
            if (!isValid) return "None";
            switch (with)
            {
                case none:
                    return (isHigher ? "H" : "L") + (isHigh ? "H" : "L");
                    break;
                case withTimeStamp: {
                    string ts = (string)timeStamp;
                    return toStr() + "(" + StringSubstr(ts, 2, 14) + ")";
                    break;
                }
                default:
                    if (with > 200000) return toStr();
                    return toStr() + "(" + string(with) + ")";
                    break;
            }
        }
    };

    HHLL_data data[arrSize];

    void prepNew(void) {
        latest = (latest+1) % arrSize;
        if (oldest == latest) {
            oldest = (oldest+1) % arrSize;
        }
        data[latest].isValid   = true;
        data[latest].timeStamp = TimeCurrent();
    }

    void addHH(void) {
        prepNew(); data[latest].isHigh   = true;  data[latest].isHigher = true;
    }
    void addHL(void) {
        prepNew(); data[latest].isHigh   = false; data[latest].isHigher = true;
    }
    void addLH(void) {
        prepNew(); data[latest].isHigh   = true;  data[latest].isHigher = false;
    }
    void addLL(void) {
        prepNew(); data[latest].isHigh   = false; data[latest].isHigher = false;
    }
    void updateTimestamp(void) {
        data[latest].updateTimestamp();
    }
    string toStr (int with = none) {
        string retval = "";
        for (uint i = latest; ; i = (i + arrSize - 1) % arrSize ) {
            if (!data[i].isValid) break;
            if (with == withTimeDiff) {
                retval += data[i].toStr((int)(data[i].timeStamp - data[(i+arrSize-1)%arrSize].timeStamp)/3600) + "-";
            } else {
                retval += data[i].toStr(with) + "-";
            }
            if (i == oldest) break;
        }
        return retval;
    }
    bool isHigh(void) {
        return data[latest].isValid && data[latest].isHigh;
    }
    bool isLow(void) {
        return data[latest].isValid && !data[latest].isHigh;
    }
    void change2higher(void) {
        if (!data[latest].isValid) return;
        data[latest].change2higher();
    }
    void change2lower(void) {
        if (!data[latest].isValid) return;
        data[latest].change2lower();
    }
public:
    HHLLlist() : latest(arrSize-1), oldest(latest), lastPrice(SymbolInfoDouble(_Symbol, SYMBOL_LAST)),
        prevL(lastPrice), prevH(lastPrice)
    {
        for (uint i = 0; i < arrSize; i++) {
            data[i].isValid = false;
            data[i].timeStamp = 0;
        }
    }
    void log(const int tickCnt)
    {
        const int prevLookBack = 100;
        {   double currH = g::zigZag.HighMapBuffer.get(1);
            if (currH > 0) {
                g::lastMax = currH;
                g::lastMaxTickCnt = tickCnt - 1;
                for (int i = 2; i < prevLookBack; i++) {
                    double backH = g::zigZag.HighMapBuffer.get(i);
                    if (backH > 0) {
                        prevH = backH;
                        if (backH > currH) {
                            g::lastMax = backH;
                            g::lastMaxTickCnt = tickCnt - i;
                        }
                        break;
                    }
                }
                bool HH = currH > prevH;
                if (HH) {
                    g::lastHH = currH;
                    if (isHigh()) {  // was xH-...
                        change2higher();  // Now: HH-...
                    }
                    else {  // was xL
                        addHH();  // Now: HH-...
                    }
                }
                else { // LH
                    if (isLow()) {  // was xL...
                        addLH();
                    }
                    else {  // was xH, must be LH
                        updateTimestamp();
                    }
                }
                LOG(SF("ZZ:H: %.2f (%+.1f%%) %s (%s)", currH, (currH/prevL-1)*100, HH ? "HH" : "LH", toStr()));
                LOG(SF("ZZ:H: %.2f (%+.1f%%) %s (%s)", currH, (currH/prevL-1)*100, HH ? "HH" : "LH", toStr(withTimeDiff)));
                // LOG(SF("ZZ:H: %.2f (%+.1f%%) %s (%s)", currH, (currH/prevL-1)*100, HH ? "HH" : "LH", toStr(withTimeStamp)));
                prevH = currH;
            }
        }
        {   double currL = g::zigZag.LowMapBuffer.get(1);
            if (currL > 0) {
                g::lastMin = currL;
                g::lastMinTickCnt = tickCnt - 1;
                for (int i = 2; i < prevLookBack; i++) {
                    double backL = g::zigZag.LowMapBuffer.get(i);
                    if (backL > 0) {
                        prevL = backL;
                        if (backL < currL) {
                            g::lastMin = backL;
                            g::lastMinTickCnt = tickCnt - i;
                        }
                        break;
                    }
                }
                bool LL = currL < prevL;
                if (LL) {
                    g::lastLL = currL;
                    // if (secondChar == "L") {  // was xL
                    if (isLow()) {  // was xL
                        change2lower();
                    }
                    else {  // was xH
                        addLL();
                    }
                }
                else {  // HL
                    if (isHigh()) {  // was xH
                        addHL();
                    }
                    else {  // was xL, must be HL
                        updateTimestamp();
                    }
                }
                LOG(SF("ZZ:L: %.2f (%.1f%%) %s (%s)", currL, (currL/prevH-1)*100, LL ? "LL" : "HL", toStr()));
                LOG(SF("ZZ:L: %.2f (%.1f%%) %s (%s)", currL, (currL/prevH-1)*100, LL ? "LL" : "HL", toStr(withTimeDiff)));
                // LOG(SF("ZZ:L: %.2f (%.1f%%) %s (%s)", currL, (currL/prevH-1)*100, LL ? "LL" : "HL", toStr(withTimeStamp)));
                prevL = currL;
            }
        }
    }
};

#endif // HHLL_LIST_H
