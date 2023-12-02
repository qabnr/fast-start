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

double            ATR_ST_buf[];
double            ATR_Color_buf[];

int ATR_ST_handle;

string            my_symbol;                                                //variable for storing the symbol
ENUM_TIMEFRAMES   my_timeframe;                                             //variable for storing the time frame

CTrade            m_Trade;                                                  //structure for execution of trades
CPositionInfo     m_Position;                                               //structure for obtaining information of positions

double vol;

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int OnInit()
{
    my_symbol = Symbol();                                                    // save the current chart symbol for further operation of the EA on this very symbol
    my_timeframe = PERIOD_CURRENT;                                           // save the current time frame of the chart for further operation of the EA on this very time frame    

    ATR_ST_handle = iCustom(NULL, PERIOD_CURRENT, "myATR_TR_STOP", 100, 2); 
    if (ATR_ST_handle == INVALID_HANDLE)
    {
        Print("Failed to get the ATR TR STOP indicator handle");
        return (-1);
    }
    ArraySetAsSeries(ATR_ST_buf, true);
    ArraySetAsSeries(ATR_Color_buf, true);
    
    vol = SymbolInfoDouble(Symbol(), SYMBOL_VOLUME_MAX);

    return (0);                                                             // return 0, initialization complete
}
//+------------------------------------------------------------------+
//| Expert deinitialization function                                 |
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
    IndicatorRelease(ATR_ST_handle);
    ArrayFree(ATR_ST_buf);
    ArrayFree(ATR_Color_buf);
}
//+------------------------------------------------------------------+
//| Expert tick function                                             |
//+------------------------------------------------------------------+
void OnTick()
{
    if (CopyBuffer(ATR_ST_handle, 0, 0, 2, ATR_ST_buf) < 0
    ||  CopyBuffer(ATR_ST_handle, 1, 0, 2, ATR_Color_buf) < 0
    )
    {
        Print("Failed to copy data from the indicator buffer or price chart buffer"); // then print the relevant error message into the log file
        return;                                                                       // and exit the function
    }


   bool BuyNow = false;
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

   if (BuyNow)
   {
       if (m_Position.Select(my_symbol)) 
       {
           if (m_Position.PositionType() == POSITION_TYPE_SELL)
               m_Trade.PositionClose(my_symbol);
           if (m_Position.PositionType() == POSITION_TYPE_BUY)
               return;
       }
       m_Trade.Buy(vol, my_symbol); // if we got here, it means there is no position; then we open it
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
       m_Trade.Sell(vol, my_symbol); // if we got here, it means there is no position; then we open it
   }
}
//+------------------------------------------------------------------+
