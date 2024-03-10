//+------------------------------------------------------------------+
//| fast-start-example.mq5
//+------------------------------------------------------------------+
#property copyright "Copyright 2024, Mogyo Software Corp."
#property link      "http://www.mogyo.com"
#property version   "3.4"

#include <Trade\Trade.mqh>
#include <Trade\PositionInfo.mqh>

input int    MACD2_fast_MA_period     = 78;
input int    MACD2_slow_MA_period     = 152;
input int    MACD2_avg_diff_period    = 95;
input double OsMA_limit               = 0.59;
input double decP_OsMa_limit          = 0.93;
input int    minMaxBAckTrack          = 5;
input double profitLossLimit          = 0.258;
input int    maxTransactions          = 170;
input double equityTradeLimit         = 0.55;
input double tradeSizeFraction        = 1.17;
input int    LastChangeOfSignMinLimit = 70003;
input int    LastChangeOfSignMaxLimit = 403621;

input double maxRelDrawDownLimit      = 0.7;

//+------------------------------------------------------------------+
#define SF StringFormat
//+------------------------------------------------------------------+
bool isLOG() {
    // if (TimeCurrent() > D'2022.06.13')
    // if (TimeCurrent() < D'2022.12.01')
        return true;
    return false;
}
void LOG_Naked(const string s) {
    if (isLOG()) Print(s);
}
#define LOG(s) LOG_Naked(SF("%d: %s", __LINE__, s))
//+------------------------------------------------------------------+
#define DAYS *24*60*60
#define HOURS *60*60
//+------------------------------------------------------------------+
string d2str(const double d, const string ThSep = ",") {
    if (d < 0) return "-" + d2str(-d, ThSep);
    int i = (int)MathFloor(d);

    if (i < 1000) return (string)i;
    int thousands = i / 1000;
    int u = i - thousands * 1000;
    if (thousands < 1000) return (string)thousands + ThSep + SF("%03d", u);
    int millions = thousands / 1000;
    thousands -= millions * 1000;
    return (string)millions + ThSep + SF("%03d", thousands) + ThSep + SF("%03d", u);
}
//+------------------------------------------------------------------+
string secToStr(int totalSeconds) {
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
int timeDiff(datetime &then) {
    return int(TimeCurrent() - then);
}
//+------------------------------------------------------------------+
string timeDiffToStr(datetime &then) {
    return secToStr(timeDiff(then));
}
//+------------------------------------------------------------------+
class buffer
{
private:
    string  name;
    int     handle;
    int     buffNum;
    double  buff[];
    int     nrCopied;

public:
    buffer(): handle(INVALID_HANDLE), buffNum(0), nrCopied(0) {
        ArraySetAsSeries(buff, true);
    }
    buffer(int _buffNum, string _name, int _handle): name(_name), handle(_handle), buffNum(_buffNum)
    {
        ArraySetAsSeries(buff, true);
Print("New buffer: ", buffNum, " handle: ", name, " (", handle, ")");
    }
    void addHandleAndBuffNum(string _name, int _handle, int _buffNum) {
         name    =  _name;
         handle  = _handle;
         buffNum = _buffNum;
Print("New buffer: ", buffNum, " handle: ", name, " (", handle, ")");
    }
    ~buffer() {};

    bool copy(int count) {
        nrCopied = CopyBuffer(handle, buffNum, 0, count, buff);
        if (nrCopied <= 0)
        {   Print("Failed to copy data from buffer: ", buffNum, " handle: ", name, " (", handle, ")");  }
        return nrCopied > 0;
    }
    double get(int index) {
        return buff[index];
    }
    int getNrCopied() {
        return nrCopied;
    }
};
//+------------------------------------------------------------------+
class ATR_TR_STOP
{
private:
    int    handle;
    buffer stopBuffer;
    buffer stopColorBuffer;

    buffer buyBuffer;
    buffer buyColorBuffer;

    buffer sellBuffer;
    buffer sellColorBuffer;

public:
    ATR_TR_STOP() {}

    void init(int ATRperiod, double mult) {
        handle  = iCustom(NULL, PERIOD_CURRENT, "myATR_TR_STOP", ATRperiod, mult);
        if (handle == INVALID_HANDLE) {
            Print("Failed to get the the ATR indicator(", ATRperiod, ", ", mult, ") handle");
            return;
        }

        string short_name = SF("ATR_TR_ST(%d, %.1f)", ATRperiod, mult);
        stopBuffer     .addHandleAndBuffNum(short_name, handle, 0);
        stopColorBuffer.addHandleAndBuffNum(short_name, handle, 1);
        buyBuffer      .addHandleAndBuffNum(short_name, handle, 2);
        buyColorBuffer. addHandleAndBuffNum(short_name, handle, 3);
        sellBuffer     .addHandleAndBuffNum(short_name, handle, 4);
        sellColorBuffer.addHandleAndBuffNum(short_name, handle, 5);
    }

    bool copyBuffers(int count) {
        return
           stopBuffer.copy(count) && stopColorBuffer.copy(count)
        && buyBuffer .copy(count) && buyColorBuffer .copy(count)
        && sellBuffer.copy(count) && sellColorBuffer.copy(count);
    }

    bool isBuyNow(double price) {
//Print(__FUNCTION__, ": ", buyBuffer.get(1), ", ", price);
        return buyBuffer.get(1) < price;
    }

    bool isSellNow(double price) {
//Print(__FUNCTION__, ": ", sellBuffer.get(1), ", ", price);
        return sellBuffer.get(1) > price;
    }
};
//+------------------------------------------------------------------+
class ATR_TR_STOP_List
{
private:
    ATR_TR_STOP atrTrStList[];

public:
    ATR_TR_STOP_List() { ArrayResize(atrTrStList, 0, 100); }
    ~ATR_TR_STOP_List() {};

    void add(int ATRper, double mult) {
        ArrayResize(atrTrStList, ArraySize(atrTrStList) + 1, 100);
        atrTrStList[ArraySize(atrTrStList)-1].init(ATRper, mult);
    }

    bool copyBuffers(int count) {
        for (int i = 0; i < ArraySize(atrTrStList); i++)
        {   if (!atrTrStList[i].copyBuffers(count)) return false;
        }
        return true;
    }
    bool isBuyNow(double price) {
        for (int i = 0; i < ArraySize(atrTrStList); i++)
        {   if (atrTrStList[i].isBuyNow(price)) return true;
        }
        return false;
    }
    bool isSellNow(double price) {
        for (int i = 0; i < ArraySize(atrTrStList); i++)
        {   if (atrTrStList[i].isSellNow(price)) return true;
        }
        return false;
    }
};
//+------------------------------------------------------------------+
class MACD
{
private:
    //int handle;

public:
    buffer MACD_Buffer;
    buffer Signal_Buffer;
    buffer OsMA_Buffer;
    buffer osMA_Color_Buffer;
    buffer decPeriod_Buffer;
    buffer incPeriod_Buffer;
    buffer decPeriod_OsMA_Buffer;
    buffer incPeriod_OsMA_Buffer;

    MACD(string name, int _handle):
        MACD_Buffer          (5, name, _handle),
        Signal_Buffer        (4, name, _handle),
        OsMA_Buffer          (2, name, _handle),
        osMA_Color_Buffer    (3, name, _handle),
        decPeriod_Buffer     (0, name, _handle),
        incPeriod_Buffer     (1, name, _handle),
        decPeriod_OsMA_Buffer(6, name, _handle),
        incPeriod_OsMA_Buffer(7, name, _handle)
    {
        if(_handle  == INVALID_HANDLE)
        {   Print("Failed to get the one of the indicator handles"); }
    }
    ~MACD() {}

    bool copyBuffers(int count)
    {
        return
            MACD_Buffer          .copy(count) &&
            Signal_Buffer        .copy(count) &&
            OsMA_Buffer          .copy(count) &&
            osMA_Color_Buffer    .copy(count) &&
            decPeriod_Buffer     .copy(count) &&
            incPeriod_Buffer     .copy(count) &&
            decPeriod_OsMA_Buffer.copy(count) &&
            incPeriod_OsMA_Buffer.copy(count);
    }
};
//+------------------------------------------------------------------+
class TradePosition
{
private:
    string        my_symbol;
    CTrade        m_Trade;
    CPositionInfo m_Position;

    double        volume;

public:
    TradePosition(string symbol): my_symbol(symbol),
        volume(MathFloor(SymbolInfoDouble(symbol, SYMBOL_VOLUME_MAX) / tradeSizeFraction))
    {}
    ~TradePosition() {}

    bool select()        { return m_Position.Select(my_symbol); }
    bool isTypeSELL()    { return m_Position.PositionType() == POSITION_TYPE_SELL; }
    bool isTypeBUY()     { return m_Position.PositionType() == POSITION_TYPE_BUY; }
    void positionClose() { 
        while(m_Trade.PositionClose(my_symbol)) {}
    }
    void buy() {
        double freeMarginBeforeTrade = AccountInfoDouble(ACCOUNT_FREEMARGIN);
LOG(SF("FrMrgn: %s", d2str(freeMarginBeforeTrade)));
        for (int i = maxTransactions; i > 0 && m_Trade.Buy(volume, my_symbol); i--) {
            if (AccountInfoDouble(ACCOUNT_FREEMARGIN) < freeMarginBeforeTrade * equityTradeLimit) {
                break;
            }
        }
LOG(SF("BUY for %s", d2str(freeMarginBeforeTrade - AccountInfoDouble(ACCOUNT_FREEMARGIN))));
LOG(SF("FrMrgn: %s", d2str(AccountInfoDouble(ACCOUNT_FREEMARGIN))));
    }
    void sell() {
        double freeMarginBeforeTrade = AccountInfoDouble(ACCOUNT_FREEMARGIN);
LOG(SF("FrMrgn: %s", d2str(freeMarginBeforeTrade)));
        for (int i = maxTransactions; i > 0 && m_Trade.Sell(volume, my_symbol); i--) {
            if (AccountInfoDouble(ACCOUNT_FREEMARGIN) < freeMarginBeforeTrade * equityTradeLimit) {
                break;
            }
        }
LOG(SF("SOLD for %s", d2str(freeMarginBeforeTrade - AccountInfoDouble(ACCOUNT_FREEMARGIN))));
LOG(SF("FrMrgn: %s", d2str(AccountInfoDouble(ACCOUNT_FREEMARGIN))));
    }
};
//+------------------------------------------------------------------+
class SellOrBuy {
#define enum2str_CASE(c) case  c: return #c
#define enum2str_DEFAULT default: return "<UNKNOWN>"
#define DEF_SET_METHOD(_state) void set##_state (int n) { if (state != _state) { state = _state; LOG_Naked(SF("%d: ==> %s", n, toStr()));}  }
#define DEF_SET_METHOD2(_state) void set##_state (int n, string comment) { if (state != _state) { state = _state; LOG_Naked(SF("%d: ==> %s <== %s", n, toStr(), comment));}  }
#define DEF_IS_METHOD(_state)  bool is##_state  () { return state == _state; }

private:
    enum SellOrBuy_State {
        None,
        GetReadyToBuy,
        BuyNow,
        GetReadyToSell,
        SellNow
    };
    SellOrBuy_State state;

public:
    SellOrBuy(): state(None) {}
    ~SellOrBuy() {}

    DEF_SET_METHOD(None)
    DEF_SET_METHOD(GetReadyToBuy)
    DEF_SET_METHOD(BuyNow)
    DEF_SET_METHOD(GetReadyToSell)
    DEF_SET_METHOD(SellNow)

    DEF_SET_METHOD2(None)
    DEF_SET_METHOD2(GetReadyToBuy)
    DEF_SET_METHOD2(BuyNow)
    DEF_SET_METHOD2(GetReadyToSell)
    DEF_SET_METHOD2(SellNow)

    DEF_IS_METHOD(None)
    DEF_IS_METHOD(GetReadyToBuy)
    DEF_IS_METHOD(BuyNow)
    DEF_IS_METHOD(GetReadyToSell)
    DEF_IS_METHOD(SellNow)

    string toStr() { return toStr(state); }

    string toStr(SellOrBuy_State s) {
        switch (s) {
            enum2str_CASE(None);
            enum2str_CASE(GetReadyToBuy);
            enum2str_CASE(BuyNow);
            enum2str_CASE(GetReadyToSell);
            enum2str_CASE(SellNow);
            enum2str_DEFAULT;
        }
    }
};
//+------------------------------------------------------------------+
class Global
{
public:
    ATR_TR_STOP_List ATR_list;
    MACD *MACD1, *MACD2;
    TradePosition *pPos;
    SellOrBuy sellOrBuy;
    double maxRelDrawDown;

    Global(): maxRelDrawDown(0) {};
};
//+-----------
Global g;
//+------------------------------------------------------------------+
int OnInit()
{

    g.MACD1 = new MACD("MACD1", iCustom(NULL, PERIOD_CURRENT, "myMACD", 12,  26,  9));
    g.MACD2 = new MACD("MACD2", iCustom(NULL, PERIOD_CURRENT, "myMACD", MACD2_fast_MA_period, MACD2_slow_MA_period, MACD2_avg_diff_period));

    g.pPos  = new TradePosition(Symbol());

/*/
    g.ATR_list.add(10, 1.0);
    g.ATR_list.add(10, 2.0);
    g.ATR_list.add(10, 3.0);
/*/
    g.ATR_list.add(10, 4.0);
 //   g.ATR_list.add(10, 6.0);

    return (0);
}
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
    delete g.MACD1;
    delete g.MACD2;
}
//+------------------------------------------------------------------+
string Osma2str(int idx) {
   return SF("[%.2f %.2f]",
                  g.MACD2.decPeriod_OsMA_Buffer.get(idx),
                  g.MACD2.OsMA_Buffer.get(idx));

}
//+------------------------------------------------------------------+
void PrintDecOsMa(string prefix) {
    string s;
    for (int i=0; i < 8; i++) {
      s = s + Osma2str(i) + " ";
    }
Print(prefix, "[dcPer OsMA]: ", s);
}
//+-----------
void PrintDecOsMa() {
    PrintDecOsMa("");
}
//+------------------------------------------------------------------+
bool justChangedToDownTrend() {
    return g.MACD2.OsMA_Buffer.get(0) < g.MACD2.OsMA_Buffer.get(1)
        && g.MACD2.OsMA_Buffer.get(1) > g.MACD2.OsMA_Buffer.get(2);
}
//+-----------
bool justChangedToUpTrend() {
    return g.MACD2.OsMA_Buffer.get(0) > g.MACD2.OsMA_Buffer.get(1)
        && g.MACD2.OsMA_Buffer.get(1) < g.MACD2.OsMA_Buffer.get(2);
}
//+-----------
bool justChangedTrends() {
    return justChangedToDownTrend() || justChangedToUpTrend();
}
//+------------------------------------------------------------------+
bool getBalanceEquity(double &balance, double &equity) {
    for (int i = 0; i < PositionsTotal(); i--) {
        if (PositionSelectByTicket(PositionGetTicket(i))) {
            balance = AccountInfoDouble(ACCOUNT_BALANCE);
            equity  = AccountInfoDouble(ACCOUNT_EQUITY);
            return true;
        }
    }
    return false;
}
//+------------------------------------------------------------------+
void changeDirection() {
    if (g.pPos.isTypeBUY()) {
        g.sellOrBuy.setSellNow(__LINE__, __FUNCTION__);
    }
    else if (g.pPos.isTypeSELL()) {
        g.sellOrBuy.setBuyNow(__LINE__, __FUNCTION__);
    }
}
//+------------------------------------------------------------------+
class MACD1_PeaksAndValleys
{
private:
    int      sign;
    datetime TimeOfLastChangeOfSign;
    int      numOfmins, numOfMaxs;
    double   lastMin,   lastMax;
    datetime TimeOfLastMin, TimeOfLastMax;
    bool     is1stPeakAlreadyFound,   is2ndPeakAlreadyFound;
    bool     is1stValleyAlreadyFound, is2ndValleyAlreadyFound;

    bool isMin() {
//LOG(SF("(%d) %.3f  %.3f", 1, g.MACD1.MACD_Buffer.get(1), g.MACD1.MACD_Buffer.get(2)));
        if (g.MACD1.MACD_Buffer.get(1) < g.MACD1.MACD_Buffer.get(2)) return false;
        for (int i = 2; i < minMaxBAckTrack; i++) {
//LOG(SF("(%d) %.3f  %.3f", i, g.MACD1.MACD_Buffer.get(i), g.MACD1.MACD_Buffer.get(i+1)));
            if (g.MACD1.MACD_Buffer.get(i) > g.MACD1.MACD_Buffer.get(i+1)) return false;
        }
// LOG("MIN found");
        return true;
    };

    bool isMax() {
//LOG(SF("(%d) %.3f  %.3f", 1, g.MACD1.MACD_Buffer.get(1), g.MACD1.MACD_Buffer.get(2)));
        if (g.MACD1.MACD_Buffer.get(1) > g.MACD1.MACD_Buffer.get(2)) return false;
        for (int i = 2; i < minMaxBAckTrack; i++) {
//LOG(SF("(%d) %.3f  %.3f", i, g.MACD1.MACD_Buffer.get(i), g.MACD1.MACD_Buffer.get(i+1)));
            if (g.MACD1.MACD_Buffer.get(i) < g.MACD1.MACD_Buffer.get(i+1)) return false;
        }
// LOG("MAX found");
        return true;
    };

    void initValues(int _sign) {

        if (sign != _sign) {
LOG(SF("(%s) Last change of sign: %s ago XXXXXXXXXXXXXXXXX", _sign > 0 ? "+" : _sign < 0 ? "-" : "0", timeDiffToStr(TimeOfLastChangeOfSign)));
            TimeOfLastChangeOfSign = TimeCurrent();
        }
        sign = _sign;
        numOfmins = numOfMaxs = 0;
        lastMin = 0.00001;
        lastMax = 999999;
        is1stPeakAlreadyFound = false;
        is2ndPeakAlreadyFound = false;
        is1stValleyAlreadyFound = false;
        is2ndValleyAlreadyFound = false;
    };

public:
    MACD1_PeaksAndValleys() :
        sign(-2),
        TimeOfLastChangeOfSign(TimeCurrent()),
        TimeOfLastMin(0),
        TimeOfLastMax(0)
    {   initValues(0); }
    ~MACD1_PeaksAndValleys() {};

    void LogMACD_Last(int cnt) {
        if (!isLOG()) return;
        string s;
        for (int i = 0; i < cnt; i++) {
            s += SF("%.2f ", g.MACD1.MACD_Buffer.get(i));
        }
        Print(s);
    }

    void process() {

//LogMACD_Last(minMaxBAckTrack+1);

static datetime barTime = 0;
datetime currBarTime = iTime(_Symbol, _Period, 0);
if(barTime != currBarTime) {
    barTime = currBarTime;
}
else {
    return;
}

if(0)
{
double decMACD0 = g.MACD1.decPeriod_Buffer.get(0);
double decMACD1 = g.MACD1.decPeriod_Buffer.get(1);
double decMACD2 = g.MACD1.decPeriod_Buffer.get(2);

if (decMACD0 >= 0 && decMACD1 < 0) {
    LOG(SF("dcMCD: Min (%.2f)", decMACD1));
}
if (decMACD1 >= 0 && decMACD2 < 0) {
    LOG(SF("dcMCD: Min2 (%.2f)", decMACD2));
}

if (decMACD0 <= 0 && decMACD1 > 0) {
    LOG(SF("dcMCD: MAX (%.2f)", decMACD1));
}
if (decMACD1 <= 0 && decMACD2 > 0) {
    LOG(SF("dcMCD: MAX2 (%.2f)", decMACD2));
}
}
        double macd0 = g.MACD1.MACD_Buffer.get(1);
        if (macd0 < 0) {
            if (sign >= 0) {
if (timeDiff(TimeOfLastChangeOfSign) > LastChangeOfSignMinLimit) 
if (timeDiff(TimeOfLastChangeOfSign) < LastChangeOfSignMaxLimit)
    g.sellOrBuy.setSellNow(__LINE__, "Change of sign: (-)");
                initValues(-1);
            }
            if (isMin()) {
LOG(SF("MIN,  time since last: %s", timeDiffToStr(TimeOfLastMin)));
// LOG(SF("Lmt: %.2f  MACD: %.2f  LstMax: %.2f", macd0  * (1 - 0.05), macd0, lastMax));
                if (macd0  * (1 - 0.05) < lastMax) {
//                    if (numOfmins == numOfMaxs)
if (timeDiff(TimeOfLastMin) > 25 HOURS)
                        numOfmins++;
                    lastMin = macd0;
                    TimeOfLastMin = TimeCurrent();
//LogMACD_Last(minMaxBAckTrack+1);
string LOGtxt = SF("min: %.2f, nOf: %d", macd0, numOfmins);
if (numOfmins == 1) LOGtxt += SF(", Last change of sign: %s ago XXXXXXXXXXXXXXXXX", timeDiffToStr(TimeOfLastChangeOfSign));
LOG(LOGtxt);
                }
            }
            else if (isMax()) {
LOG(SF("MAX,  time since last: %s", timeDiffToStr(TimeOfLastMax)));
// LOG(SF("Lmt: %.2f%%  MACD: %.2f  LstMin: %.2f", (macd0 / lastMin)*100, macd0, lastMin));
                if ((macd0 / lastMin) < 1 - 0.05) {
//                    if (numOfmins > numOfMaxs)
                        numOfMaxs++;
                    lastMax = macd0;
                    TimeOfLastMax = TimeCurrent();
string LOGtxt = (SF("Max: %.2f, nOf: %d", macd0, numOfMaxs));
if (numOfMaxs == 1) LOGtxt += (SF(", Last change of sign: %s ago XXXXXXXXXXXXXXXXX", timeDiffToStr(TimeOfLastChangeOfSign)));
LOG(LOGtxt);
                }
            }
        }
        else {
            if (sign <= 0) {
if (timeDiff(TimeOfLastChangeOfSign) > LastChangeOfSignMinLimit)
if (timeDiff(TimeOfLastChangeOfSign) < LastChangeOfSignMaxLimit)
    g.sellOrBuy.setBuyNow(__LINE__, "Change of sign: (+)");
                initValues(1);
            }
            if (isMax()) {
LOG(SF("MAX,  time since last: %s", timeDiffToStr(TimeOfLastMax)));
//                if (numOfmins == numOfMaxs)
                    numOfMaxs++;
                lastMax = macd0;
                TimeOfLastMax = TimeCurrent();
//LogMACD_Last(minMaxBAckTrack+1);
string LOGtxt = (SF("Max: %.2f, nOf: %d", macd0, numOfMaxs));
if (numOfMaxs == 1) LOGtxt += (SF(", Last change of sign: %s ago XXXXXXXXXXXXXXXXX", timeDiffToStr(TimeOfLastChangeOfSign)));
LOG(LOGtxt);
            }
            else if (isMin()){
LOG(SF("MIN,  time since last: %s", timeDiffToStr(TimeOfLastMin)));
// LOG(SF("Lmt: %.2f%%  MACD: %.2f  LstMAX: %.2f", (macd0 / lastMax)*100, macd0, lastMax));
                if ((macd0 / lastMax) < 1 - 0.05) {
//                    if (numOfmins < numOfMaxs)
if (timeDiff(TimeOfLastMin) > 25 HOURS)
                        numOfmins++;
                    lastMin = macd0;
                    TimeOfLastMin = TimeCurrent();
string LOGtxt = (SF("min: %.2f, nOf: %d", macd0, numOfmins));
if (numOfmins == 1) LOGtxt += (SF(", Last change of sign: %s ago XXXXXXXXXXXXXXXXX", timeDiffToStr(TimeOfLastChangeOfSign)));
LOG(LOGtxt);
                }
            }
        }
    }

    bool is1stPeak() {
// LOG(SF("%s: %s  MAX: %d  min: %d", __FUNCTION__, is1stPeakAlreadyFound?"T":"F", numOfMaxs,numOfmins));
        if (is1stPeakAlreadyFound) return false;
        // if (numOfMaxs == 1 && numOfmins == 0) {
        if (sign > 0 && numOfMaxs == 1 && numOfmins == 0) {
            is1stPeakAlreadyFound = true;
            return true;
        }
        return false;
    }
    bool is2ndPeak() {
//LOG(SF("%s: %s  MAX: %d  min: %d", __FUNCTION__, is2ndPeakAlreadyFound?"T":"F", numOfMaxs, numOfmins));
        if (is2ndPeakAlreadyFound) return false;
//        if (numOfMaxs == 2 && numOfmins == 1) {
        // if (numOfMaxs == 2 && numOfmins <= 1) {
        if (sign > 0 && numOfMaxs == 2 && numOfmins <= 1) {
            is2ndPeakAlreadyFound = true;
            return true;
        }
        return false;
    }

    bool is1stValley() {
//LOG(SF("%s: %s  MAX: %d  min: %d", __FUNCTION__, is1stValleyAlreadyFound?"T":"F", numOfMaxs, numOfmins));
        if (is1stValleyAlreadyFound) return false;
        // if (numOfmins == 1 && numOfMaxs == 0) {
        if (sign < 0 && numOfmins == 1 && numOfMaxs == 0) {
            is1stValleyAlreadyFound = true;
            return true;
        }
        return false;
    }

    bool is2ndValley() {
//LOG(SF("%s: %s  MAX: %d  min: %d", __FUNCTION__, is2ndValleyAlreadyFound?"T":"F", numOfMaxs, numOfmins));
        if (is2ndValleyAlreadyFound) return false;
//        if (numOfmins == 2 && numOfMaxs == 1) {
        // if (numOfmins == 2 && numOfMaxs <= 1) {
        if (sign < 0 && numOfmins == 2 && numOfMaxs <= 1) {
            is2ndValleyAlreadyFound = true;
            return true;
        }
        return false;
    }
};
//+------------------------------------------------------------------+
void OnTick()
{

    if (g.maxRelDrawDown > maxRelDrawDownLimit) return;

    if (copyBuffers() == false)
    {   Print("Failed to copy data from buffer"); return; }

    double profit = 0, balance, equity;
    static double maxProfit  = 0;
    static double maxEquity  = 0;
    static double maxBalance = 0;

    if (getBalanceEquity(balance, equity)) {
        profit = equity - balance;
        if (profit  > maxProfit)  { maxProfit = profit; }
        if (equity  > maxEquity)  { maxEquity = equity; }
        if (balance > maxBalance) { maxBalance = balance; }

        // double relDrawDown = 1 - equity / maxEquity;
        double relDrawDown = 1 - balance / maxBalance;
        if (relDrawDown > g.maxRelDrawDown) { g.maxRelDrawDown = relDrawDown; }
        
        LOG(SF("P: %8s  PR: %+6.1f%%  E: %6s  E/Emx: %+6.1f%%  B: %s  DrDmx: %.1f%%",
            d2str(profit),
            profit/balance*100.0,
            d2str(equity),
            (equity-maxEquity)/maxEquity*100,
            d2str(balance),
            g.maxRelDrawDown * 100));

        if (profit/balance < -profitLossLimit) {
            changeDirection();
        }
    }

    //if (TimeCurrent() > D'2023.08.05')
    // if (TimeCurrent() > D'2022.05.23')
    {
        static MACD1_PeaksAndValleys MACD1peaksAndValleys;

        MACD1peaksAndValleys.process();

        if (MACD1peaksAndValleys.is1stPeak()) {
// LOG(SF("1st Peak: profit = %.2f eq = %.2f bal = %.2f   pr/bal = %.2f %%  --------------------", profit, equity, balance, profit/balance*100));
if (profit/balance > 0.5) {
//    g.sellOrBuy.setSellNow(__LINE__, "1st peak");
}
        }
        if (MACD1peaksAndValleys.is2ndPeak()) {
            g.sellOrBuy.setSellNow(__LINE__, "2nd peak");
        }
        else if (MACD1peaksAndValleys.is1stValley()) {
// LOG(SF("1st Valley: profit = %.2f eq = %.2f bal = %.2f   pr/bal = %.2f %%  --------------------", profit, equity, balance, profit/balance*100));
if (profit/balance > 0.5) {
//    g.sellOrBuy.setBuyNow(__LINE__, "1st valey");
}

        }
        else if (MACD1peaksAndValleys.is2ndValley()) {
            g.sellOrBuy.setBuyNow(__LINE__, "2nd valley");
        }
        else if (justChangedToDownTrend()) {
//PrintDecOsMa("v ");
            if (g.MACD2.decPeriod_OsMA_Buffer.get(1) > decP_OsMa_limit) {
                if (g.MACD2.OsMA_Buffer.get(0)       > OsMA_limit) {
                    g.sellOrBuy.setGetReadyToSell(__LINE__, "decOsMA > limit");
                }
            }
        }
        else if (justChangedToUpTrend()) {
//PrintDecOsMa("^ ");
            if (g.MACD2.decPeriod_OsMA_Buffer.get(1) < -decP_OsMa_limit) {
                if (g.MACD2.OsMA_Buffer.get(0)       < -OsMA_limit) {
                    g.sellOrBuy.setGetReadyToBuy(__LINE__, "decOsMA < -limit");
                }
            }
        }
    }

    if (g.sellOrBuy.isGetReadyToBuy()) {
        if (g.ATR_list.isBuyNow(getPrice().low)) {
            g.sellOrBuy.setBuyNow(__LINE__, "ATR low");
        }
    }
    else
    if (g.sellOrBuy.isGetReadyToSell()) {
        if (g.ATR_list.isSellNow(getPrice().high)) {
            g.sellOrBuy.setSellNow(__LINE__, "ATR high");
        }
    }

    if (g.sellOrBuy.isBuyNow()) {
        if (g.pPos.select())
        {
            if (g.pPos.isTypeSELL()) {
                g.pPos.positionClose();
            }
            if (g.pPos.isTypeBUY()) {
//Print(__LINE__, " Already bought");
                return;
            }
        }
        g.pPos.buy();
        maxProfit = 0;
        g.sellOrBuy.setNone(__LINE__, "Bought");
    }
    else if (g.sellOrBuy.isSellNow()) {
        if (g.pPos.select())
        {
            if (g.pPos.isTypeBUY()) {
                g.pPos.positionClose();
            }
            if (g.pPos.isTypeSELL()) {
//Print(__LINE__, " Already sold");
                return;
            }
        }
        g.pPos.sell();
        maxProfit = 0;
        g.sellOrBuy.setNone(__LINE__, "Sold");
    }
}
//+------------------------------------------------------------------+
MqlRates getPrice()
{
    MqlRates bar[2];
    if(CopyRates(_Symbol,_Period, 0, 2, bar) > 0) {
    }
    return bar[1];
}
//+------------------------------------------------------------------+
bool copyBuffers()
{
    const int buffSize = 300;

    if (!g.MACD1.   copyBuffers(buffSize) ||
        !g.MACD2.   copyBuffers(buffSize) ||
        !g.ATR_list.copyBuffers(buffSize)  )
    {
        Print("Failed to copy data from buffer");
        return false;
    }
    return true;
}
//+------------------------------------------------------------------+
double OnTester()
{
    // if (g.maxRelDrawDown > maxRelDrawDownLimit) return 0;
    // return AccountInfoDouble(ACCOUNT_BALANCE);
    if (g.maxRelDrawDown > maxRelDrawDownLimit) return 0;
    return AccountInfoDouble(ACCOUNT_BALANCE) / g.maxRelDrawDown;
}
//+------------------------------------------------------------------+

//+------------------------------------------------------------------+

