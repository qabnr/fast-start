//+------------------------------------------------------------------+
//| fast-start-example.mq5
//+------------------------------------------------------------------+
#property copyright "Copyright 2012, MetaQuotes Software Corp."
#property link      "http://www.mql5.com"
#property version   "1.00"

#include <Trade\Trade.mqh>
#include <Trade\PositionInfo.mqh>

int ATR_ST_handle;
int MACD1_handle;
int MACD2_handle;

double ATR_ST_buf[],   ATR_Color_buf[];
double MACD1_Buffer[], Signal1_Buffer[], OsMA1_Buffer[], osMA_Color1_Buffer[];
double MACD2_Buffer[], Signal2_Buffer[], OsMA2_Buffer[], osMA_Color2_Buffer[];

string        my_symbol;
CTrade        m_Trade;
CPositionInfo m_Position;

double vol;

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
    ArraySetAsSeries(ATR_ST_buf,     true);
    ArraySetAsSeries(ATR_Color_buf,  true);
    
    ArraySetAsSeries(MACD1_Buffer,       true);
    ArraySetAsSeries(Signal1_Buffer,     true);
    ArraySetAsSeries(OsMA1_Buffer,       true);
    ArraySetAsSeries(osMA_Color1_Buffer, true);
    
    ArraySetAsSeries(MACD2_Buffer,       true);
    ArraySetAsSeries(Signal2_Buffer,     true);
    ArraySetAsSeries(OsMA2_Buffer,       true);
    ArraySetAsSeries(osMA_Color2_Buffer, true);
    
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

    ArrayFree(ATR_ST_buf);
    ArrayFree(ATR_Color_buf);
}
//+------------------------------------------------------------------+
//| Expert tick function
//+------------------------------------------------------------------+
void OnTick()
{
    if (copyBuffers() == false)
    {   Print("Failed to copy data from buffer"); return; }

    bool BuyNow  = false;
    bool SellNow = false;

    if (ATR_Color_buf[0] > 1.5)
    {
        if (ATR_Color_buf[1] < 0.5)
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
    
    if (osMA_Color2_Buffer[2] != osMA_Color2_Buffer[1])
    {
        Print("-----------------", osMA_Color2_Buffer[2], " --- ", osMA_Color2_Buffer[1], "-----------------");
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
    if (CopyBuffer(ATR_ST_handle, 0, 0, 3, ATR_ST_buf)    < 0
    ||  CopyBuffer(ATR_ST_handle, 1, 0, 3, ATR_Color_buf) < 0

    ||  CopyBuffer(MACD1_handle,  0, 0, 3, OsMA1_Buffer)       < 0
    ||  CopyBuffer(MACD1_handle,  1, 0, 3, osMA_Color1_Buffer) < 0
    ||  CopyBuffer(MACD1_handle,  2, 0, 3, MACD1_Buffer)       < 0
    ||  CopyBuffer(MACD1_handle,  3, 0, 3, Signal1_Buffer)     < 0

    ||  CopyBuffer(MACD2_handle,  0, 0, 3, OsMA2_Buffer)       < 0
    ||  CopyBuffer(MACD2_handle,  1, 0, 3, osMA_Color2_Buffer) < 0
    ||  CopyBuffer(MACD2_handle,  2, 0, 3, MACD2_Buffer)       < 0
    ||  CopyBuffer(MACD2_handle,  3, 0, 3, Signal2_Buffer)     < 0
    )
    {
        Print("Failed to copy data from buffer");
        return false;
    }
    return true;
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

//+------------------------------------------------------------------+
//| 
//+------------------------------------------------------------------+

