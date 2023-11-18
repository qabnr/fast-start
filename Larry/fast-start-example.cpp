//+------------------------------------------------------------------+
//|                                           fast-start-example.mq5 |
//|                        Copyright 2012, MetaQuotes Software Corp. |
//|                                              http://www.mql5.com |
//+------------------------------------------------------------------+
#property copyright "Copyright 2012, MetaQuotes Software Corp."
#property link "http://www.mql5.com"
#property version "1.00"
//+------------------------------------------------------------------+
//| Expert initialization function                                   |
//+------------------------------------------------------------------+
#include <Trade\Trade.mqh>                                                  //include the library for execution of trades
#include <Trade\PositionInfo.mqh>                                           //include the library for obtaining information on positions

int               iMA_handle;                                               //variable for storing the indicator handle
double            iMA_buf[];                                                //dynamic array for storing indicator values
double            ATR_ST_buf[];
double            ATR_Color_buf[];
double            Close_buf[];                                              //dynamic array for storing the closing price of each bar

int ATR_ST_handle;

string            my_symbol;                                                //variable for storing the symbol
ENUM_TIMEFRAMES   my_timeframe;                                             //variable for storing the time frame

CTrade            m_Trade;                                                  //structure for execution of trades
CPositionInfo     m_Position;                                               //structure for obtaining information of positions

double min_volume;

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int OnInit()
{
    my_symbol = Symbol();                                                    // save the current chart symbol for further operation of the EA on this very symbol
    my_timeframe = PERIOD_CURRENT;                                           // save the current time frame of the chart for further operation of the EA on this very time frame
    iMA_handle = iMA(my_symbol, PERIOD_CURRENT, 40, 0, MODE_SMA, PRICE_CLOSE); // apply the indicator and get its handle
    if (iMA_handle == INVALID_HANDLE)                                        // check the availability of the indicator handle
    {
        Print("Failed to get the MA indicator handle");                     // if the handle is not obtained, print the relevant error message into the log file
        return (-1);                                                        // complete handling the error
    }
    ArraySetAsSeries(iMA_buf, true);                                        // set iMA_buf array indexing as time series
    ArraySetAsSeries(Close_buf, true);                                      // set Close_buf array indexing as time series
    

    ATR_ST_handle = iCustom(NULL, PERIOD_CURRENT, "myATR_TR_STOP", 100, 2); 
    if (ATR_ST_handle == INVALID_HANDLE)
    {
        Print("Failed to get the ATR TR STOP indicator handle");
        return (-1);
    }
    ArraySetAsSeries(ATR_ST_buf, true);
    
    min_volume = SymbolInfoDouble(Symbol(), SYMBOL_VOLUME_MIN);

    return (0);                                                             // return 0, initialization complete
}
//+------------------------------------------------------------------+
//| Expert deinitialization function                                 |
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
    IndicatorRelease(iMA_handle);                                           // deletes the indicator handle and deallocates the memory space it occupies
    IndicatorRelease(ATR_ST_handle);
    ArrayFree(iMA_buf);                                                     // free the dynamic array iMA_buf of data
    ArrayFree(ATR_ST_buf);
    ArrayFree(ATR_Color_buf);
    ArrayFree(Close_buf);                                                   // free the dynamic array Close_buf of data
}
//+------------------------------------------------------------------+
//| Expert tick function                                             |
//+------------------------------------------------------------------+
void OnTick()
{
    int retC1;                                                           // variable for storing the results of working with the indicator buffer
    int retC2;                                                           // variable for storing the results of working with the price chart
    int retC3;
    int retC4;

    retC1 = CopyBuffer(iMA_handle, 0, 1, 2, iMA_buf);                        // copy data from the indicator array into the dynamic array iMA_buf for further work with them
    retC3 = CopyBuffer(ATR_ST_handle, 0, 1, 2, ATR_ST_buf);
    retC4 = CopyBuffer(ATR_ST_handle, 1, 1, 2, ATR_Color_buf);
    retC2 = CopyClose(my_symbol, PERIOD_CURRENT, 1, 2, Close_buf);             // copy the price chart data into the dynamic array Close_buf for further work with them
    if (retC1 < 0 || retC2 < 0 || retC3 < 0)
    {
        Print("Failed to copy data from the indicator buffer or price chart buffer"); // then print the relevant error message into the log file
        return;                                                                       // and exit the function
    }


   bool BuyNow = false;
   bool SellNow = false;

//Print("ATR: ", NormalizeDouble(ATR_ST_buf[0], 2), " Col: ", ATR_Color_buf[0] < 0.5 ? "Red" : ATR_Color_buf[0] < 1.5 ? "Green" : "Black");
if (ATR_Color_buf[0] > 1.5)
{
   if (ATR_Color_buf[1] < 0.5)
   {
      Print("Buy");
      BuyNow = true;
   }
   else
   {
      Print("Sell");
      SellNow = true;
   }
}

//    if (iMA_buf[1] > Close_buf[1] && iMA_buf[0] < Close_buf[0])             // if the indicator values were greater than the closing price and became smaller
    if (BuyNow)
    {
        if (m_Position.Select(my_symbol))                                   // if the position for this symbol already exists
        {
            if (m_Position.PositionType() == POSITION_TYPE_SELL)
                m_Trade.PositionClose(my_symbol);                           // and this is a Sell position, then close it
            if (m_Position.PositionType() == POSITION_TYPE_BUY)
                return;                                                     // or else, if this is a Buy position, then exit
        }
        //else
        m_Trade.Buy(min_volume, my_symbol);                                 // if we got here, it means there is no position; then we open it
    }
//    if (iMA_buf[1] < Close_buf[1] && iMA_buf[0] > Close_buf[0])             // if the indicator values were less than the closing price and became greater
    else
    if (SellNow)
    {
        if (m_Position.Select(my_symbol))                                   // if the position for this symbol already exists
        {
            if (m_Position.PositionType() == POSITION_TYPE_BUY)
                m_Trade.PositionClose(my_symbol);                           // and this is a Buy position, then close it
            if (m_Position.PositionType() == POSITION_TYPE_SELL)
                return;                                                     // or else, if this is a Sell position, then exit
        }
        //else
        m_Trade.Sell(min_volume, my_symbol);                                // if we got here, it means there is no position; then we open it
    }
}
//+------------------------------------------------------------------+
