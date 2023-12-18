//+------------------------------------------------------------------+
//| fast-start-example.mq5
//+------------------------------------------------------------------+
#property copyright "Copyright 2012, MetaQuotes Software Corp."
#property link      "http://www.mql5.com"
#property version   "1.00"

#include <Trade\Trade.mqh>
#include <Trade\PositionInfo.mqh>

struct minMaxT {
   double min;
   double max;
};

class buffer
{
private:
    int     handle;
    int     buffNum;
    double  buff[];
    int     nrCopied;

public:
    buffer(): handle(INVALID_HANDLE), buffNum(0), nrCopied(0) {
        ArraySetAsSeries(buff, true);
    }
    ~buffer() {};
    void addHandleAndBuffNum(int _handle, int _buffNum) {
         handle  = _handle;
         buffNum = _buffNum;
    }
    bool copy(int count) {
        nrCopied = CopyBuffer(handle, buffNum, 0, count, buff);
        return nrCopied > 0;
    }
    double get(int index) {
        return buff[index];
    }
    int getNrCopied() {
        return nrCopied;
    }
};


int ATR_ST_handle;
int MACD1_handle;
int MACD2_handle;

buffer ATR_ST_buffer, ATR_Color_buffer;
buffer MACD1_Buffer, Signal1_Buffer, OsMA1_Buffer, osMA_Color1_Buffer;
buffer MACD2_Buffer, Signal2_Buffer, OsMA2_Buffer, osMA_Color2_Buffer;

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
    my_symbol = Symbol();

    ATR_ST_handle = iCustom(NULL, PERIOD_CURRENT, "myATR_TR_STOP", 100,   2);
    MACD1_handle  = iCustom(NULL, PERIOD_CURRENT, "my MACD",        12,  26,  9);
    MACD2_handle  = iCustom(NULL, PERIOD_CURRENT, "my MACD",        84, 182, 63);

    if (ATR_ST_handle == INVALID_HANDLE
    ||  MACD1_handle  == INVALID_HANDLE
    ||  MACD2_handle  == INVALID_HANDLE)
    {
        Print("Failed to get the one of the indicator handles");
        return (-1);
    }

    ATR_ST_buffer       .addHandleAndBuffNum(ATR_ST_handle, 0);
    ATR_Color_buffer    .addHandleAndBuffNum(ATR_ST_handle, 1);

    MACD1_Buffer        .addHandleAndBuffNum(MACD1_handle, 2);
    Signal1_Buffer      .addHandleAndBuffNum(MACD1_handle, 3);
    OsMA1_Buffer        .addHandleAndBuffNum(MACD1_handle, 0);
    osMA_Color1_Buffer  .addHandleAndBuffNum(MACD1_handle, 1);
    
    MACD2_Buffer        .addHandleAndBuffNum(MACD2_handle, 2);
    Signal2_Buffer      .addHandleAndBuffNum(MACD2_handle, 3);
    OsMA2_Buffer        .addHandleAndBuffNum(MACD2_handle, 0);
    osMA_Color2_Buffer  .addHandleAndBuffNum(MACD2_handle, 1);
    
    vol = SymbolInfoDouble(Symbol(), SYMBOL_VOLUME_MAX);

    return (0);
}
//+------------------------------------------------------------------+
//| Expert deinitialization function
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
    IndicatorRelease(ATR_ST_handle);
    IndicatorRelease(MACD1_handle);
    IndicatorRelease(MACD2_handle);
}
//+------------------------------------------------------------------+
//| Expert tick function
//+------------------------------------------------------------------+
void OnTick()
{
    if (copyBuffers() == false)
    {   Print("Failed to copy data from buffer"); return; }
    
    minMaxT mM = findMinMax(&Signal1_Buffer);
    Print("m: ", mM.min, " M: ", mM.max);

    bool BuyNow  = false;
    bool SellNow = false;

    if (ATR_Color_buffer.get(0) > 1.5)
    {
        if (ATR_Color_buffer.get(1) < 0.5)
        {
            Print("Sell");
            SellNow = true;
        }
        else
        {
            Print("Buy");
            BuyNow = true;
        }
    }
    
    if (osMA_Color2_Buffer.get(2) != osMA_Color2_Buffer.get(1))
    {
        Print("-----------------", osMA_Color2_Buffer.get(2), " --- ", osMA_Color2_Buffer.get(1), "-----------------");
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
    if (!ATR_ST_buffer      .copy(buffSize) ||
        !ATR_Color_buffer   .copy(buffSize) ||
        !MACD1_Buffer       .copy(buffSize) ||
        !Signal1_Buffer     .copy(buffSize) ||
        !OsMA1_Buffer       .copy(buffSize) ||
        !osMA_Color1_Buffer .copy(buffSize) ||
        !MACD2_Buffer       .copy(buffSize) ||
        !Signal2_Buffer     .copy(buffSize) ||
        !OsMA2_Buffer       .copy(buffSize) ||
        !osMA_Color2_Buffer .copy(buffSize)   )
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

