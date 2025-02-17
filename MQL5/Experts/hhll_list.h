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

        void change2higher() {
            isHigher = true;
            timeStamp = TimeCurrent();
        }

        void change2lower() {
            isHigher = false;
            timeStamp = TimeCurrent();
        }

        void updateTimestamp() {
            timeStamp = TimeCurrent();
        }

        string toStr(int with = none) const {
            if (!isValid) return "None";
            string result = (isHigher ? "H" : "L") + (isHigh ? "H" : "L");
            if (with == none || with > 200000) {
                return result;
            } else if (with == withTimeStamp) {
                return result + "(" + StringSubstr((string)timeStamp, 2, 14) + ")";
            } else {
                return result + "(" + string(with) + ")";
            }
        }
    };

    HHLL_data data[arrSize];

    void prepNew() {
        latest = (latest + 1) % arrSize;
        if (oldest == latest) {
            oldest = (oldest + 1) % arrSize;
        }
        data[latest].isValid   = true;
        data[latest].timeStamp = TimeCurrent();
    }

    void addHH() {
        prepNew(); data[latest].isHigh = true;  data[latest].isHigher = true;
    }
    void addHL() {
        prepNew(); data[latest].isHigh = false; data[latest].isHigher = true;
    }
    void addLH() {
        prepNew(); data[latest].isHigh = true;  data[latest].isHigher = false;
    }
    void addLL() {
        prepNew(); data[latest].isHigh = false; data[latest].isHigher = false;
    }
    void updateTimestamp() {
        data[latest].updateTimestamp();
    }
    string toStr(int with = none) const {
        string retval = "";
        for (uint i = latest; ; i = (i + arrSize - 1) % arrSize) {
            if (!data[i].isValid) break;
            if (with == withTimeDiff) {
                retval += data[i].toStr((int)(data[i].timeStamp - data[(i + arrSize - 1) % arrSize].timeStamp) / 3600) + "-";
            } else {
                retval += data[i].toStr(with) + "-";
            }
            if (i == oldest) break;
        }
        return retval;
    }
    bool isHigh() const {
        return data[latest].isValid && data[latest].isHigh;
    }
    bool isLow() const {
        return data[latest].isValid && !data[latest].isHigh;
    }
    void change2higher() {
        if (!data[latest].isValid) return;
        data[latest].change2higher();
    }
    void change2lower() {
        if (!data[latest].isValid) return;
        data[latest].change2lower();
    }

public:
    static double lastHH;
    static double lastMax;
    static int    lastMaxTickCnt;
    static double lastLL;
    static double lastMin;
    static int    lastMinTickCnt;

    HHLLlist() : latest(arrSize - 1), oldest(latest), lastPrice(SymbolInfoDouble(_Symbol, SYMBOL_LAST)),
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
        {   
            double currH = g::zigZag.HighMapBuffer.get(1);
            if (currH > 0) {
                lastMax = currH;
                lastMaxTickCnt = tickCnt - 1;
                for (int i = 2; i < prevLookBack; i++) {
                    double backH = g::zigZag.HighMapBuffer.get(i);
                    if (backH > 0) {
                        prevH = backH;
                        if (backH > currH) {
                            lastMax = backH;
                            lastMaxTickCnt = tickCnt - i;
                        }
                        break;
                    }
                }
                bool HH = currH > prevH;
                if (HH) {
                    lastHH = currH;
                    if (isHigh()) {  // was xH-...
                        change2higher();  // Now: HH-...
                    } else {  // was xL
                        addHH();  // Now: HH-...
                    }
                }
                else { // LH
                    if (isLow()) {  // was xL...
                        addLH();
                    } else {  // was xH, must be LH
                        updateTimestamp();
                    }
                }
                LOG(SF("ZZ:H: %.2f (%+.1f%%) %s (%s)", currH, (currH/prevL-1)*100, HH ? "HH" : "LH", toStr()));
                LOG(SF("ZZ:H: %.2f (%+.1f%%) %s (%s)", currH, (currH/prevL-1)*100, HH ? "HH" : "LH", toStr(withTimeDiff)));
                // LOG(SF("ZZ:H: %.2f (%+.1f%%) %s (%s)", currH, (currH/prevL-1)*100, HH ? "HH" : "LH", toStr(withTimeStamp)));
                prevH = currH;
            }
        }
        {   
            double currL = g::zigZag.LowMapBuffer.get(1);
            if (currL > 0) {
                lastMin = currL;
                lastMinTickCnt = tickCnt - 1;
                for (int i = 2; i < prevLookBack; i++) {
                    double backL = g::zigZag.LowMapBuffer.get(i);
                    if (backL > 0) {
                        prevL = backL;
                        if (backL < currL) {
                            lastMin = backL;
                            lastMinTickCnt = tickCnt - i;
                        }
                        break;
                    }
                }
                bool LL = currL < prevL;
                if (LL) {
                    lastLL = currL;
                    if (isLow()) {
                        change2lower();
                    } else {
                        addLL();
                    }
                } else {
                    if (isHigh()) {
                        addHL();
                    } else {
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

// Initialize static members
double HHLLlist::lastHH = 0;
double HHLLlist::lastMax = 0;
int HHLLlist::lastMaxTickCnt = 0;
double HHLLlist::lastLL = 0;
double HHLLlist::lastMin = 0;
int HHLLlist::lastMinTickCnt = 0;

#endif // HHLL_LIST_H