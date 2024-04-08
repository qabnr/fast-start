//+------------------------------------------------------------------+
//| fast-start-example.mq5
//+------------------------------------------------------------------+
#property copyright "Copyright 2024, Mogyo Software Corp."
#property link      "http://www.mogyo.com"
#property version   "3.4"

#include <Trade\Trade.mqh>
#include <Trade\PositionInfo.mqh>

// input int    MACD2_fast_MA_period        = 64;
// input int    MACD2_slow_MA_period        = 157;
// input int    MACD2_avg_diff_period       = 86;
// input double OsMA_limit                  = 0.47;
// input double decP_OsMa_limit             = 0.98;
// input int    minMaxBAckTrack             = 5;
// input double profitLossLimit             = 0.21;
// input double profitPerMaxProfitLossLimit = 0.40;
input int    maxTransactions             = 150;
//  int    maxTransactions             = 150;
// input double equityTradeLimit            = 0.75;
// input double tradeSizeFraction           = 1.18;
 double tradeSizeFraction           = 1.18;
// input int    LastChangeOfSignMinLimit    = 27100;
// input int    LastChangeOfSignMaxLimit    = 390300;

// //input double profitRate_paidLimit        = 1.0;

// input double maxRelDrawDownLimit         = 0.7;

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
#define LOG(s) if (isLOG()) Print(SF("%d: %s", __LINE__, s))
//+------------------------------------------------------------------+
#define DAYS *24*60*60
#define HOURS *60*60
//+------------------------------------------------------------------+
string d2str(const double d, const string ThSep = ",") {
    if (d < 0) return "-" + d2str(-d, ThSep);
    int i = (int)MathFloor(d);

    if (i < 1000) {
        return (string)i;
    }
    int thousands = i / 1000;
    int u = i - thousands * 1000;
    if (thousands < 1000) {
        return (string)thousands + ThSep + SF("%03d", u);
    }
    int millions = thousands / 1000;
    thousands -= millions * 1000;
    if (millions < 1000) {
        return (string)millions + ThSep + SF("%03d", thousands) + ThSep + SF("%03d", u);
    }
    int trillions = millions / 1000;
    millions -= trillions * 1000;
    return (string)trillions + ThSep + SF("%03d", millions) + ThSep + SF("%03d", thousands) + ThSep + SF("%03d", u);
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

    int           posType;
    double        pricePaid;

    double        getVolume(double freeMarginBeforeTrade)
    {
        double maxVol = SymbolInfoDouble(my_symbol, SYMBOL_VOLUME_MAX);
        if (freeMarginBeforeTrade / maxVol < 10)
            return MathFloor(maxVol / 3);
        return maxVol;
    }

public:
    TradePosition(string symbol): my_symbol(symbol),
        // volume(MathFloor(SymbolInfoDouble(symbol, SYMBOL_VOLUME_MAX) / tradeSizeFraction)),
        posType(-1), pricePaid(0)
    {}
    ~TradePosition() {}

    bool select()         { return m_Position.Select(my_symbol); }
    // bool isTypeSELL()    { return m_Position.PositionType() == POSITION_TYPE_SELL; }
    // bool isTypeBUY()     { return m_Position.PositionType() == POSITION_TYPE_BUY; }
    bool isTypeSELL()     { return posType == POSITION_TYPE_SELL; }
    bool isTypeBUY()      { return posType == POSITION_TYPE_BUY; }
    double getPricePaid() { return pricePaid; }
    void positionClose() {
        while(m_Trade.PositionClose(my_symbol)) {}
    }
    void buy() {
        double freeMarginBeforeTrade = AccountInfoDouble(ACCOUNT_FREEMARGIN);
        double volume = getVolume(freeMarginBeforeTrade);
LOG(SF("FrMrgn: %s", d2str(freeMarginBeforeTrade)));
        for (int i = maxTransactions; i > 0 && m_Trade.Buy(volume, my_symbol); i--) {
            posType = POSITION_TYPE_BUY;
            // if (AccountInfoDouble(ACCOUNT_FREEMARGIN) < freeMarginBeforeTrade * equityTradeLimit) {
            //     break;
            // }
        }
        pricePaid = freeMarginBeforeTrade - AccountInfoDouble(ACCOUNT_FREEMARGIN);
LOG(SF("BUY for %s", d2str(pricePaid)));
LOG(SF("FrMrgn: %s", d2str(AccountInfoDouble(ACCOUNT_FREEMARGIN))));
    }
    void sell() {
        double freeMarginBeforeTrade = AccountInfoDouble(ACCOUNT_FREEMARGIN);
        double volume = getVolume(freeMarginBeforeTrade);
LOG(SF("FrMrgn: %s", d2str(freeMarginBeforeTrade)));
        for (int i = maxTransactions; i > 0 && m_Trade.Sell(volume, my_symbol); i--) {
            posType = POSITION_TYPE_SELL;
            // if (AccountInfoDouble(ACCOUNT_FREEMARGIN) < freeMarginBeforeTrade * equityTradeLimit) {
            //     break;
            // }
        }
        pricePaid = freeMarginBeforeTrade - AccountInfoDouble(ACCOUNT_FREEMARGIN);
LOG(SF("SELL for %s", d2str(pricePaid)));
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
    // ATR_TR_STOP_List ATR_list;
    // MACD *MACD1, *MACD2;
    TradePosition *pPos;
    SellOrBuy sellOrBuy;
    double maxRelDrawDown;

    int    zzHandle;
    int    minMaxHandle;
    double zzBuff[];

    Global(): maxRelDrawDown(0) {};
};
//+-----------
Global g;
//+------------------------------------------------------------------+
int OnInit()
{

    // g.MACD1 = new MACD("MACD1", iCustom(NULL, PERIOD_CURRENT, "myMACD", 12,  26,  9));
    // g.MACD2 = new MACD("MACD2", iCustom(NULL, PERIOD_CURRENT, "myMACD", MACD2_fast_MA_period, MACD2_slow_MA_period, MACD2_avg_diff_period));

    g.minMaxHandle = iCustom(NULL, PERIOD_CURRENT, "myMinMax");

    g.zzHandle = iCustom(NULL, PERIOD_CURRENT, "Examples/ZigZag", 20,  5,  20);

    g.pPos  = new TradePosition(Symbol());

/*/
    g.ATR_list.add(10, 1.0);
    g.ATR_list.add(10, 2.0);
    g.ATR_list.add(10, 3.0);
/*/
    // g.ATR_list.add(10, 4.0);
 //   g.ATR_list.add(10, 6.0);

    LOG("OnInit");

    return (0);
}
//+------------------------------------------------------------------+
void copyZZbuff(void)
{
    int zzBuffNo = 0;
    int startPos = 0;
    int count = 999999;

    // ArrayFree(g.zzBuff);
    // ArraySetAsSeries(g.zzBuff, true);
    int copied = CopyBuffer(g.zzHandle, zzBuffNo, startPos, count, g.zzBuff);

    LOG("Cp'd: " + IntegerToString(copied));

    // if (copied == 3581) {
    //     printZZbuff();
    // }
}
void printZZbuff(void)
{
    ArraySetAsSeries(g.zzBuff, true);

LOG(SF("zzBuff size: %d", ArraySize(g.zzBuff)));

    for (int i = 0; i < ArraySize(g.zzBuff); i++) {
        if (g.zzBuff[i] > 0.01 && g.zzBuff[i] < 999999.99)
        {
            datetime time  = iTime(Symbol(),Period(), i);
            // LOG(SF("%4d: %10s %6.2f", i, TimeToString(time, TIME_DATE|TIME_SECONDS), g.zzBuff[i]));
        }
    }    
}
//+------------------------------------------------------------------+
void changeDirection(const int lineNo, const string comment) {
    if (g.pPos.isTypeBUY()) {
        g.sellOrBuy.setSellNow(lineNo, __FUNCTION__ + " <== " + comment);
    }
    else if (g.pPos.isTypeSELL()) {
        g.sellOrBuy.setBuyNow(lineNo, __FUNCTION__ + " <== " + comment);
    }
}
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
    LOG("Deinitialization reason code = " + IntegerToString(reason));

    // delete g.MACD1;
    // delete g.MACD2;

    copyZZbuff();
    printZZbuff();
}
//+------------------------------------------------------------------+
void BS(void)
{
    // if (TimeCurrent() == D'2022.03.14 22:00:00') { g.sellOrBuy.setBuyNow(__LINE__, "1"); }
    // if (TimeCurrent() == D'2022.03.29 22:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.04.18 19:00:00') { g.sellOrBuy.setBuyNow(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.04.21 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.04.27 17:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.04.28 22:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.05.02 21:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.05.04 22:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.05.12 21:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.05.17 22:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.05.20 20:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.06.01 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.06.02 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.06.02 22:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.06.14 22:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.06.15 22:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.06.16 21:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.06.27 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.06.30 17:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.07.08 18:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.07.14 17:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.07.22 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.07.26 22:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.08.08 17:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.08.09 19:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.08.16 21:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.08.24 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.08.26 17:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.09.06 17:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.09.12 17:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.09.16 19:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.09.21 21:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.09.23 21:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.09.28 22:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.09.30 22:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.11.15 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.11.22 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.11.23 22:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.11.29 18:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.12.01 17:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.12.07 17:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.12.13 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.12.20 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.12.21 21:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2022.12.28 22:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.01.03 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.01.06 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.01.18 17:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.01.19 18:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.02.02 21:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.02.10 19:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.02.15 22:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.03.02 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.03.06 18:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.03.13 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.03.22 21:00:00') { changeDirection(__LINE__, "1"); }   
    if (TimeCurrent() == D'2023.03.28 20:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.04.04 17:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.04.12 22:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.04.18 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.04.25 22:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.05.01 20:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.05.04 17:00:00') { changeDirection(__LINE__, "1"); }

    if (TimeCurrent() == D'2023.06.16 16:00:00') { changeDirection(__LINE__, "1"); }

    if (TimeCurrent() == D'2023.06.26 22:00:00') { changeDirection(__LINE__, "1"); }

    if (TimeCurrent() == D'2023.07.05 17:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.07.10 19:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.07.19 19:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.07.24 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.07.31 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.08.18 17:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.08.24 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.08.25 18:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.09.01 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.09.07 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.09.14 22:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.09.27 20:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.09.29 17:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.10.03 22:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.10.12 19:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.10.26 20:00:00') { changeDirection(__LINE__, "1"); }

    // if (TimeCurrent() == D'2023.11.09 22:00:00') { changeDirection(__LINE__, "1"); }
    // if (TimeCurrent() == D'2023.11.22 16:00:00') { changeDirection(__LINE__, "1"); }

    if (TimeCurrent() == D'2023.11.29 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2023.12.04 17:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2024.01.24 19:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2024.01.31 22:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2024.02.12 18:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2024.02.21 21:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2024.02.23 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2024.02.28 16:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2024.03.01 21:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2024.03.05 22:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2024.03.08 17:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2024.03.15 19:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2024.03.21 19:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2024.03.27 19:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2024.04.01 17:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2024.04.02 18:00:00') { changeDirection(__LINE__, "1"); }
    if (TimeCurrent() == D'2024.04.04 20:00:00') { changeDirection(__LINE__, "1"); }
}
//+------------------------------------------------------------------+
void OnTick()
{
    double mmBuff[2];
    if(CopyBuffer(g.minMaxHandle, 0, 0, 2, mmBuff) > 0) {
/*/
        LOG(SF("mM: %.0f  %.0f", mmBuff[1], mmBuff[0]));
/**/
    } 

    static int tickCnt = 0;

    tickCnt++;
    // LOG("tickCnt: " + IntegerToString(tickCnt));
    // if (tickCnt == 3512) {
    // if (tickCnt == 3512)
    // if (tickCnt >= 3500)
    {
        // copyZZbuff();
        // printZZbuff();
    }


    //if (MQLInfoInteger(MQL_TESTER) && g.maxRelDrawDown > maxRelDrawDownLimit) return;

    if (copyBuffers() == false)
    {   Print("Failed to copy data from buffer"); return; }

    static double maxProfit  = 0;
    static double maxEquity  = 0;
    static double maxBalance = 0;

    double profit, balance, equity;

    balance = AccountInfoDouble(ACCOUNT_BALANCE);
    equity  = AccountInfoDouble(ACCOUNT_EQUITY);
    profit  = equity - balance;

    if (profit  > maxProfit)  { maxProfit = profit; }
    if (equity  > maxEquity)  { maxEquity = equity; }
    if (balance > maxBalance) { maxBalance = balance; }

    double profitRate     = profit / balance;
    double profitLossRate = (profit-maxProfit) / balance;
    double profitRate_paid     = profit / g.pPos.getPricePaid();
    double profitLossRate_paid = (profit-maxProfit) / g.pPos.getPricePaid();

    double relDrawDown = 1 - balance / maxBalance;
    if (relDrawDown > g.maxRelDrawDown) { g.maxRelDrawDown = relDrawDown; }

/*/
if (isNewMinute())
LOG(SF("P: %11s  P/Pmx: %+6.1f%%  (%+6.1f%%)  PR: %+6.1f%%  (%+6.1f%%)  E: %6s  Eq/EqMx: %+6.1f%%  Blc: %s  DrDmx: %.1f%%",
        d2str(profit),
        profitLossRate * 100,
        profitLossRate_paid * 100,
        profitRate * 100.0,
        profitRate_paid * 100.0,
        d2str(equity),
        (equity-maxEquity) / maxEquity * 100,
        d2str(balance),
        g.maxRelDrawDown * 100));
/**/

    // if (profitRate < -profitLossLimit) {
    //     changeDirection(__LINE__, "profitLossLimit");
    // }
    // else if (maxProfit > 0 && (profitRate < -profitLossLimit/2) && profitLossRate < -profitPerMaxProfitLossLimit) {
    //     changeDirection(__LINE__, "profitPerMaxProfitLossLimit");
    // }
    // else if (profitRate_paid < -profitRate_paidLimit) {
    //     changeDirection(__LINE__, "profitRate_paidLimit");
    // }

BS();

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
    // const int buffSize = 300;

    // if (!g.MACD1.   copyBuffers(buffSize) ||
    //     !g.MACD2.   copyBuffers(buffSize) ||
    //     !g.ATR_list.copyBuffers(buffSize)  )
    // {
    //     Print("Failed to copy data from buffer");
    //     return false;
    // }
    return true;
}
//+------------------------------------------------------------------+
double OnTester()
{
    // if (g.maxRelDrawDown > maxRelDrawDownLimit) return 0;
    // return AccountInfoDouble(ACCOUNT_BALANCE);
//    if (g.maxRelDrawDown > maxRelDrawDownLimit) return 0;
    return AccountInfoDouble(ACCOUNT_BALANCE) / g.maxRelDrawDown;
}
//+------------------------------------------------------------------+

//+------------------------------------------------------------------+

