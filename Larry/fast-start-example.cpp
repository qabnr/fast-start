//+------------------------------------------------------------------+
//| fast-start-example.mq5
//+------------------------------------------------------------------+
#property copyright "Copyright 2024, Mogyo Software Corp."
#property link      "http://www.mogyo.com"
#property version   "3.4"

#include <Trade\Trade.mqh>
#include <Trade\PositionInfo.mqh>

input int MACD2_fast_MA_period  = 71;  // 84;
input int MACD2_slow_MA_period  = 154; // 182;
input int MACD2_avg_diff_period = 86;  // 63;

input double OsMA_limit =      0.52;   // 0.19;
input double decP_OsMa_limit = 0.97;   // 0.79;

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

        string short_name = StringFormat("ATR_TR_ST(%d, %.1f)", ATRperiod, mult);
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
        volume(SymbolInfoDouble(symbol, SYMBOL_VOLUME_MAX) / 1)
    {}
    ~TradePosition() {}

    bool select()
    { return m_Position.Select(my_symbol); }
    ENUM_POSITION_TYPE positionType ()
    { return m_Position.PositionType(); }
    void positionClose()
    { m_Trade.PositionClose(my_symbol); }
    void buy()
    { m_Trade.Buy(volume, my_symbol); }
    void sell()
    { m_Trade.Sell(volume, my_symbol); }
};
//+------------------------------------------------------------------+
class Global
{
public:
    ATR_TR_STOP_List ATR_list;
    MACD *MACD1, *MACD2;
    TradePosition *pPos;
};

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
class SellOrBuy {
#define enum2str_CASE(c) case  c: return #c
#define enum2str_DEFAULT default: return "<UNKNOWN>"
#define DEF_SET_METHOD(c) void set##c () { state = c; Print("--> ", toStr());  }
#define DEF_IS_METHOD(c)  bool is##c  () { return state == c; }

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
string Osma2str(int idx) {
   return StringFormat("[%.2f %.2f]",
                  g.MACD2.decPeriod_OsMA_Buffer.get(idx),
                  g.MACD2.OsMA_Buffer.get(idx));

}
//+------------------------------------------------------------------+
void PrintDecOsMa() {
    string s;
    for (int i=0; i < 8; i++) {
      s = s + Osma2str(i) + " ";
    }
Print(s);
}
//+------------------------------------------------------------------+
void OnTick()
{
    if (copyBuffers() == false)
    {   Print("Failed to copy data from buffer"); return; }

    static SellOrBuy sellOrBuy;

//if (TimeCurrent() > D'2023.08.05')
if (TimeCurrent() > D'2022.05.23')
{
    //if (sellOrBuy.isNone())
     {
        if (g.MACD2.decPeriod_OsMA_Buffer.get(0) <  0.001
        &&  g.MACD2.decPeriod_OsMA_Buffer.get(0) > -0.001
        &&  g.MACD2.decPeriod_OsMA_Buffer.get(1) > decP_OsMa_limit
        &&  g.MACD2.OsMA_Buffer.get(0) > OsMA_limit
        ) {
            sellOrBuy.setGetReadyToSell();
PrintDecOsMa();

        }
        else
        if (g.MACD2.decPeriod_OsMA_Buffer.get(0) <  0.001
        &&  g.MACD2.decPeriod_OsMA_Buffer.get(0) > -0.001
        &&  g.MACD2.decPeriod_OsMA_Buffer.get(1) < -decP_OsMa_limit
        &&  g.MACD2.OsMA_Buffer.get(0) < -OsMA_limit
        ) {
            sellOrBuy.setGetReadyToBuy();
PrintDecOsMa();
        }
    }
}
    if (sellOrBuy.isGetReadyToBuy()) {
        if (g.ATR_list.isBuyNow(getPrice().low)) {
            sellOrBuy.setBuyNow();
        }
    }
    else
    if (sellOrBuy.isGetReadyToSell()) {
        if (g.ATR_list.isSellNow(getPrice().high)) {
            sellOrBuy.setSellNow();
        }
    }

    if (sellOrBuy.isBuyNow()) {
        if (g.pPos.select())
        {
            if (g.pPos.positionType() == POSITION_TYPE_SELL)
                g.pPos.positionClose();
            if (g.pPos.positionType() == POSITION_TYPE_BUY) {
//Print(__LINE__, " Already bought");
                return;
            }
        }
        g.pPos.buy();
        sellOrBuy.setNone();
    }
    else if (sellOrBuy.isSellNow()) {
        if (g.pPos.select())
        {
            if (g.pPos.positionType() == POSITION_TYPE_BUY) {
                g.pPos.positionClose();
            }
            if (g.pPos.positionType() == POSITION_TYPE_SELL) {
//Print(__LINE__, " Already sold");
                return;
            }
        }
        g.pPos.sell();
        sellOrBuy.setNone();
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

//+------------------------------------------------------------------+

//+------------------------------------------------------------------+

