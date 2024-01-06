//+------------------------------------------------------------------+
//| fast-start-example.mq5
//+------------------------------------------------------------------+
#property copyright "Copyright 2012, MetaQuotes Software Corp."
#property link      "http://www.mql5.com"
#property version   "1.00"

#property tester_indicator "myATR_TR_STOP.ex5"

#include <Trade\Trade.mqh>
#include <Trade\PositionInfo.mqh>

//+------------------------------------------------------------------+
struct minMaxT {
   double min;
   double max;
};
//+------------------------------------------------------------------+
class buffer
{
private:
    string  name;
    int     handle;
    int     buffNum;
    double  buff[];
    int     nrCopied;
    int     objSerialNum;

    static  int objCount;

public:
    buffer(): handle(INVALID_HANDLE), buffNum(0), nrCopied(0) {
        ArraySetAsSeries(buff, true);
        objCount++;
        objSerialNum = objCount;
    }
    ~buffer() {};
    void addHandleAndBuffNum(string _name, int _handle, int _buffNum) {
         name    =  _name;
         handle  = _handle;
         buffNum = _buffNum;

Print("Adding (", objSerialNum, ") ",name, " buff# ", buffNum);
    }
    bool copy(int count) {
        nrCopied = CopyBuffer(handle, buffNum, 0, count, buff);
        if (nrCopied <= 0)
        {   Print("Failed to copy data from buffer: ", buffNum, " handle: ", name, "(", objSerialNum, ")");    }
        return nrCopied > 0;
    }
    double get(int index) {
        return buff[index];
    }
    int getNrCopied() {
        return nrCopied;
    }
};

int buffer::objCount = 0;

//+------------------------------------------------------------------+
class ATR_TR_STOP {
private:
    int     handle;
    buffer  ST_buffer;
    buffer  color_buffer;

public:
    ATR_TR_STOP() {}

    void add(int ATRperiod, double mult) {
        handle  = iCustom(NULL, PERIOD_CURRENT, "myATR_TR_STOP", ATRperiod, mult);
        if (handle == INVALID_HANDLE)
        {
            Print("Failed to get the the ATR indicator(", ATRperiod, ", ", mult, ") handle");
            return;
        }
        ST_buffer   .addHandleAndBuffNum("ATR_ST", handle, 0);
        color_buffer.addHandleAndBuffNum("ATR_ST", handle, 1);
    }

    bool copy(int count) {
        return ST_buffer.copy(count) && color_buffer.copy(count);
    }
};
//+------------------------------------------------------------------+
class ATR_TR_STOP_List
{
private:
    ATR_TR_STOP atrTrSt[];

public:
    ATR_TR_STOP_List() { ArrayResize(atrTrSt, 0, 100); }
    ~ATR_TR_STOP_List() {};

    void add(int ATRper, double mult) {
Print("atrTrSt size: ", ArraySize(atrTrSt));
        ArrayResize(atrTrSt, ArraySize(atrTrSt) + 1, 100);
        atrTrSt[ArraySize(atrTrSt)-1].add(ATRper, mult);
Print("atrTrSt size: ", ArraySize(atrTrSt));
    }

    bool copy(int count) {
        for (int i = 0; i < ArraySize(atrTrSt); i++)
        {   if (!atrTrSt[i].copy(count)) return false;
        }
        return true;
    }
};

//************************************************************************

ATR_TR_STOP_List ATR_list;

int MACD1_handle;
int MACD2_handle;

buffer MACD1_Buffer, Signal1_Buffer, OsMA1_Buffer, osMA_Color1_Buffer;
buffer MACD2_Buffer, Signal2_Buffer, OsMA2_Buffer, osMA_Color2_Buffer;

buffer decPeriod_MACD1_Buffer;
buffer incPeriod_MACD1_Buffer;
buffer decPeriod_OsMA1_Buffer;
buffer incPeriod_OsMA1_Buffer;

buffer decPeriod_MACD2_Buffer;
buffer incPeriod_MACD2_Buffer;
buffer decPeriod_OsMA2_Buffer;
buffer incPeriod_OsMA2_Buffer;

string        my_symbol;
CTrade        m_Trade;
CPositionInfo m_Position;

double vol;

const int buffSize = 300;

//+------------------------------------------------------------------+
//| Expert initialization function
//+------------------------------------------------------------------+
int OnInit()
{

ATR_list.add(10, 3.0);

    my_symbol = Symbol();

    MACD1_handle  = iCustom(NULL, PERIOD_CURRENT, "myMACD",        12,  26,  9);
    MACD2_handle  = iCustom(NULL, PERIOD_CURRENT, "myMACD",        84, 182, 63);

    if (MACD1_handle  == INVALID_HANDLE ||
        MACD2_handle  == INVALID_HANDLE)
    {
        Print("Failed to get the one of the indicator handles");
        return (-1);
    }

    decPeriod_MACD1_Buffer.addHandleAndBuffNum("MACD1", MACD1_handle, 0);
    incPeriod_MACD1_Buffer.addHandleAndBuffNum("MACD1", MACD1_handle, 1);
    OsMA1_Buffer          .addHandleAndBuffNum("MACD1", MACD1_handle, 2);
    osMA_Color1_Buffer    .addHandleAndBuffNum("MACD1", MACD1_handle, 3);
    Signal1_Buffer        .addHandleAndBuffNum("MACD1", MACD1_handle, 4);
    MACD1_Buffer          .addHandleAndBuffNum("MACD1", MACD1_handle, 5);
    decPeriod_OsMA1_Buffer.addHandleAndBuffNum("MACD1", MACD1_handle, 6);
    incPeriod_OsMA1_Buffer.addHandleAndBuffNum("MACD1", MACD1_handle, 7);
    
    decPeriod_MACD2_Buffer.addHandleAndBuffNum("MACD1", MACD2_handle, 0);
    incPeriod_MACD2_Buffer.addHandleAndBuffNum("MACD1", MACD2_handle, 1);
    OsMA2_Buffer          .addHandleAndBuffNum("MACD2", MACD2_handle, 2);
    osMA_Color2_Buffer    .addHandleAndBuffNum("MACD2", MACD2_handle, 3);
    Signal2_Buffer        .addHandleAndBuffNum("MACD2", MACD2_handle, 4);
    MACD2_Buffer          .addHandleAndBuffNum("MACD2", MACD2_handle, 5);
    decPeriod_OsMA2_Buffer.addHandleAndBuffNum("MACD1", MACD2_handle, 6);
    incPeriod_OsMA2_Buffer.addHandleAndBuffNum("MACD1", MACD2_handle, 7);

ATR_list.add(10, 4.0);

    vol = SymbolInfoDouble(Symbol(), SYMBOL_VOLUME_MAX);

    return (0);
}
//+------------------------------------------------------------------+
//| Expert deinitialization function
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
    IndicatorRelease(MACD1_handle);
    IndicatorRelease(MACD2_handle);
}
//+------------------------------------------------------------------+
//| Expert tick function
//+------------------------------------------------------------------+
int runLen   = 0; 

void OnTick()
{
    if (copyBuffers() == false)
    {   Print("Failed to copy data from buffer"); return; }
    
    // minMaxT mM = findMinMax(&Signal1_Buffer);
    // Print("m: ", mM.min, " M: ", mM.max);

    bool BuyNow  = false;
    bool SellNow = false;

if (TimeCurrent() > D'2022.04.27')
{    
    if (osMA_Color2_Buffer.get(2) != osMA_Color2_Buffer.get(1))
    {
        // Print(" X- ", runLen, " -- ", osMA_Color2_Buffer.get(2), " --- ", osMA_Color2_Buffer.get(1), "-----------------");
    }
    else
    {
        runLen += 1;
        // Print(" X- ", runLen);        
    }
    if (osMA_Color2_Buffer.get(2) > osMA_Color2_Buffer.get(1))
    {
            if (runLen > 10)
            {
                BuyNow = true;
            }
            runLen = 0;
    }
    else
    if (osMA_Color2_Buffer.get(2) < osMA_Color2_Buffer.get(1))
    {
            if (runLen > 10)
            {
                SellNow = true;
            }
            runLen = 0;
    }
}

    if (BuyNow)
    {
        if (m_Position.Select(my_symbol)) 
        {
            if (m_Position.PositionType() == POSITION_TYPE_SELL)
                m_Trade.PositionClose(my_symbol);
            if (m_Position.PositionType() == POSITION_TYPE_BUY)
                return;
        }
        m_Trade.Buy(vol, my_symbol);
    }
    else if (SellNow)
    {
        if (m_Position.Select(my_symbol))
        {
            if (m_Position.PositionType() == POSITION_TYPE_BUY)
                m_Trade.PositionClose(my_symbol);
            if (m_Position.PositionType() == POSITION_TYPE_SELL)
                return;
        }
        m_Trade.Sell(vol, my_symbol);
    }
}
//+------------------------------------------------------------------+
//| 
//+------------------------------------------------------------------+
bool copyBuffers()
{
    if (!MACD1_Buffer       .copy(buffSize) ||
        !Signal1_Buffer     .copy(buffSize) ||
        !OsMA1_Buffer       .copy(buffSize) ||
        !osMA_Color1_Buffer .copy(buffSize) ||
        !MACD2_Buffer       .copy(buffSize) ||
        !Signal2_Buffer     .copy(buffSize) ||
        !OsMA2_Buffer       .copy(buffSize) ||
        !osMA_Color2_Buffer .copy(buffSize) ||
        !ATR_list.copy(buffSize)  )
    {
        Print("Failed to copy data from buffer");
        return false;
    }
    return true;
}

//+------------------------------------------------------------------+
//| 
//+------------------------------------------------------------------+

minMaxT findMinMax(buffer &buff)
{
    double min =  9e99;
    double max = -9e99;

    int size = buff.getNrCopied();

    for (int i = 0; i < size; i++)
    {
        if (buff.get(i) < min) min = buff.get(i);
        if (buff.get(i) > max) max = buff.get(i);
    }
    
    minMaxT r {min, max};

    return r;
}

//+------------------------------------------------------------------+
//| 
//+------------------------------------------------------------------+

//+------------------------------------------------------------------+
//| 
//+------------------------------------------------------------------+

//+------------------------------------------------------------------+
//| 
//+------------------------------------------------------------------+

