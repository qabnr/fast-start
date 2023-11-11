/*
#property copyright   "Denis Zyatkevich"
#property version     "1.00"
#property description "This Expert Advisor places pending orders"
#property description "during StartHour to EndHour at distance"
#property description "1 point out of the daily range. StopLoss price"
#property description "of each order is placed on the opposite side"
#property description "of the price range. After order execution"
#property description "it places TakeProfit at price, calculated by"
#property description "'indicator_TP', StopLoss is placed to SMA, "
#property description "in case of the profitable zone."
*/

#include "my_mq5_defs.h"

/* input */ int    StartHour  =  7; 
/* input */ int    EndHour    =  19; 
/* input */ int    MAper      =  240; 
/* input */ double Lots       =  0.1; 

int hMA, hCI; 
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void OnInit()
{
   hMA =     iMA(NULL, 0, MAper, 0, MODE_SMA, PRICE_CLOSE);
   hCI = iCustom(NULL, 0, "indicator_TP"); 
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void OnTick()
{
   MqlTradeRequest request; 
   MqlTradeResult result; 
   MqlDateTime dt; 
   ZeroMemory(request); 
   bool is_BuyStopOrder = false, is_SellStopOrder = false; 
   int i; 
   ulong ticket; 
   datetime time[];
   double bid = SymbolInfoDouble(Symbol(), SYMBOL_BID);
   double ask = SymbolInfoDouble(Symbol(), SYMBOL_ASK);
   double high[], low[], movingAvg[], atr_hi[], atr_lo[], highestPriceOfDay, lowestPriceOfDay, StopLoss, 
   StopLevel = _Point * SymbolInfoInteger(Symbol(), SYMBOL_TRADE_STOPS_LEVEL), 
   Spread    = NormalizeDouble(ask - bid, _Digits);
          
   request.symbol       = Symbol(); 
   request.volume       = Lots; 
   request.tp           = 0; 
   request.deviation    = 0; 
   request.type_filling = ORDER_FILLING_FOK; 

   TimeCurrent(dt); 
   i = (dt.hour+1)*60; 
   if (CopyTime(Symbol(), /* current time frame */ 0, /* StartPos */ 0, /* cnt */ i, time) < i 
    || CopyHigh(Symbol(), /* current time frame */ 0, /* StartPos */ 0, /* cnt */ i, high) < i
    || CopyLow (Symbol(), /* current time frame */ 0, /* StartPos */ 0, /* cnt */ i, low ) < i)
       { Print("Can't copy timeseries!");  return; }
   ArraySetAsSeries(time, true); 
   ArraySetAsSeries(high, true); 
   ArraySetAsSeries(low,  true); 
   highestPriceOfDay = high[0]; 
   lowestPriceOfDay  = low[0]; 
   for(i = 1; i < ArraySize(time)
        && MathFloor(time[i] / 86400) == MathFloor(time[0] / 86400); // same day
        i++)
   {  if (high[i] > highestPriceOfDay) highestPriceOfDay = high[i]; 
      if (low[i]  < lowestPriceOfDay)  lowestPriceOfDay  = low[i]; 
   }
   highestPriceOfDay += Spread + _Point; 
   lowestPriceOfDay  -= _Point; 
   if (CopyBuffer(hMA, /* Buffer */ 0, /* StartPos */ 0, /* Len */ 2, movingAvg) < 2
    || CopyBuffer(hCI, /* Buffer */ 0, /* StartPos */ 0, /* Len */ 1, atr_hi) < 1
    || CopyBuffer(hCI, /* Buffer */ 1, /* StartPos */ 0, /* Len */ 1, atr_lo) < 1)
      {  Print("Can't copy indicator buffer!"); return; }
   ArraySetAsSeries(movingAvg, true); 
   atr_lo[0] += Spread; 

   for(i = 0; i < PositionsTotal(); i++)    // check all opened positions
   {  if (Symbol() == PositionGetSymbol(i)) // process orders with "our" symbols only
      {  // we will change the values of StopLoss and TakeProfit
         request.action = TRADE_ACTION_SLTP; 
         if (PositionGetInteger(POSITION_TYPE) == POSITION_TYPE_BUY)  // process long positions 
         {  // let's determine StopLoss
            if (movingAvg[1] > PositionGetDouble(POSITION_PRICE_OPEN)) 
            { StopLoss = movingAvg[1]; }
              else
            { StopLoss = lowestPriceOfDay; }
            
            double pos_StopLoss   = PositionGetDouble(POSITION_SL);
            double pos_TakeProfit = PositionGetDouble(POSITION_TP);
            if (  // if StopLoss is not defined or lower than needed
                  (  pos_StopLoss == 0   || pos_StopLoss < StopLoss
                  // if TakeProfit is not defined or higer than needed
                  || pos_TakeProfit == 0 || pos_TakeProfit > atr_hi[0]
                  )
                  // is new StopLoss close to the current price?
                  && NormalizeDouble(bid - StopLoss - StopLevel, _Digits) > 0
                  // is new TakeProfit close to the current price?
                  && NormalizeDouble(atr_hi[0] - bid - StopLevel, _Digits) > 0
               )
            {
               request.sl = NormalizeDouble(StopLoss,  _Digits); 
               request.tp = NormalizeDouble(atr_hi[0], _Digits);
               OrderSend(request, result); 
            }
         }
         else 
         if (PositionGetInteger(POSITION_TYPE) == POSITION_TYPE_SELL) // process short positions
         {  // let's determine the value of StopLoss
            if (movingAvg[1] + Spread < PositionGetDouble(POSITION_PRICE_OPEN))
            { StopLoss = movingAvg[1] + Spread; }
              else 
            { StopLoss = highestPriceOfDay; }
            // if StopLoss is not defined or higher than needed
            if ((pos_StopLoss == 0    || pos_StopLoss > StopLoss
               // if TakeProfit is not defined or lower than needed
               || pos_TakeProfit == 0 || atr_lo[0] > pos_TakeProfit
               // is new StopLoss close to the current price?
               && NormalizeDouble(StopLoss - ask - StopLevel, _Digits) > 0
               // is new TakeProfit close to the current price?
               && NormalizeDouble(ask - atr_lo[0] - StopLevel, _Digits) > 0)
            {
               request.sl = NormalizeDouble(StopLoss, _Digits); 
               request.tp = NormalizeDouble(atr_lo[0], _Digits); 
               OrderSend(request, result); 
            }
         }
         // if there is an opened position, return from here...
         return; 
      }
   }

   for(i = 0; i < OrdersTotal(); i++) // check all pending orders
   {  // choose each order and get its ticket
      ticket = OrderGetTicket(i); 
      // process orders with "our" symbols only
      if (OrderGetString(ORDER_SYMBOL) == Symbol())
      {  // process Buy Stop orders
         if (OrderGetInteger(ORDER_TYPE) == ORDER_TYPE_BUY_STOP)
         {  // check if there is trading time and price movement is possible
            if (dt.hour >= StartHour && dt.hour<EndHour && highestPriceOfDay<atr_hi[0])
            {  // if the opening price is lower than needed
               if ((highestPriceOfDay > OrderGetDouble(ORDER_PRICE_OPEN)
                  // if StopLoss is not defined or higher than needed
                  || OrderGetDouble(ORDER_SL) == 0 || (OrderGetDouble(ORDER_SL)-lowestPriceOfDay, _Digits) != 0)
                  // is opening price close to the current price?
                  && NormalizeDouble(highestPriceOfDay - ask-StopLevel, _Digits) > 0)
               {
                  request.action = TRADE_ACTION_MODIFY; 
                  request.order  = ticket; 
                  request.price  = NormalizeDouble(highestPriceOfDay, _Digits); 
                  request.sl     = NormalizeDouble(lowestPriceOfDay, _Digits); 
                  OrderSend(request, result); 
                  return; 
               }
            }
            // if there is no trading time or the average trade range has been passed
            else
            {  // we will delete this pending order
               request.action = TRADE_ACTION_REMOVE; 
               request.order  = ticket; 
               OrderSend(request, result); 
               return; 
            }
            // setting the flag, that indicates the presence of Buy Stop order
            is_BuyStopOrder = true; 
         }
         // processing Sell Stop orders
         if (OrderGetInteger(ORDER_TYPE) == ORDER_TYPE_SELL_STOP)
         {  // check if there is trading time and price movement is possible
            if (dt.hour >= StartHour && dt.hour<EndHour && lowestPriceOfDay > atr_lo[0])
            {  // if the opening price is higher than needed
               if ((NormalizeDouble(OrderGetDouble(ORDER_PRICE_OPEN)-lowestPriceOfDay, _Digits) > 0
                  // if StopLoss is not defined or lower than need
                  || OrderGetDouble(ORDER_SL) == 0 || NormalizeDouble(highestPriceOfDay-OrderGetDouble(ORDER_SL), _Digits) > 0)
                  // is opening price close to the current price?
                  && NormalizeDouble(bid - lowestPriceOfDay-StopLevel, _Digits) > 0)
               {
                  // pending order parameters will be changed
                  request.action = TRADE_ACTION_MODIFY; 
                  request.order = ticket; 
                  request.price = NormalizeDouble(lowestPriceOfDay, _Digits); 
                  request.sl = NormalizeDouble(highestPriceOfDay, _Digits); 
                  OrderSend(request, result); 
                  return; 
               }
            }
            // if there is no trading time or the average trade range has been passed
            else
            {  // we will delete this pending order
               request.action = TRADE_ACTION_REMOVE; 
               request.order = ticket; 
               OrderSend(request, result); 
               return; 
            }
            // set the flag that indicates the presence of Sell Stop order
            is_SellStopOrder = true; 
         }
      }
   }
   request.action = TRADE_ACTION_PENDING; 
   if (dt.hour >= StartHour && dt.hour < EndHour)
   {
      if (is_BuyStopOrder == false && highestPriceOfDay < atr_hi[0])
      {
         request.price = NormalizeDouble(highestPriceOfDay, _Digits); 
         request.sl    = NormalizeDouble(lowestPriceOfDay,  _Digits); 
         request.type  = ORDER_TYPE_BUY_STOP; 
         OrderSend(request, result); 
      }
      if (is_SellStopOrder == false && lowestPriceOfDay > atr_lo[0])
      {
         request.price = NormalizeDouble(lowestPriceOfDay,  _Digits); 
         request.sl    = NormalizeDouble(highestPriceOfDay, _Digits); 
         request.type  = ORDER_TYPE_SELL_STOP; 
         OrderSend(request, result); 
      }
   }
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
   IndicatorRelease(hCI); 
   IndicatorRelease(hMA); 
}
//+------------------------------------------------------------------+
