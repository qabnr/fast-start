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

#include ""

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
   hMA =     iMA(NULL, 0, MAper, 0, MODE_SMA, PRICE_CLOSE); +
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
   double high[], low[], ma[], atr_h[], atr_l[], lev_highestOfDay, lev_lowestOfDay, StopLoss, 
   StopLevel = _Point * SymbolInfoInteger(Symbol(), SYMBOL_TRADE_STOPS_LEVEL), 
   Spread    = NormalizeDouble(SymbolInfoDouble(Symbol(), SYMBOL_ASK) - SymbolInfoDouble(Symbol(), SYMBOL_BID), _Digits);
          
   request.symbol       = Symbol(); 
   request.volume       = Lots; 
   request.tp           = 0; 
   request.deviation    = 0; 
   request.type_filling = ORDER_FILLING_FOK; 

   TimeCurrent(dt); 
   i = (dt.hour+1)*60; 
   if (CopyTime(Symbol(), 0, 0, i, time) < i 
    || CopyHigh(Symbol(), 0, 0, i, high) < i
    || CopyLow (Symbol(), 0, 0, i, low) < i)
       { Print("Can't copy timeseries!");  return; }
   ArraySetAsSeries(time, true); 
   ArraySetAsSeries(high, true); 
   ArraySetAsSeries(low, true); 
   lev_highestOfDay = high[0]; 
   lev_lowestOfDay = low[0]; 
   for(i = 1; i < ArraySize(time) && MathFloor(time[i] / 86400) == MathFloor(time[0] / 86400); i++)
   {  if (high[i] > lev_highestOfDay) lev_highestOfDay = high[i]; 
      if (low[i]  < lev_lowestOfDay)  lev_lowestOfDay  = low[i]; 
   }
   lev_highestOfDay += Spread + _Point; 
   lev_lowestOfDay  -= _Point; 
   if (CopyBuffer(hMA, 0, 0, 2, ma)    < 2
    || CopyBuffer(hCI, 0, 0, 1, atr_h) < 1
    || CopyBuffer(hCI, 1, 0, 1, atr_l) < 1)
      {  Print("Can't copy indicator buffer!"); return; }
   ArraySetAsSeries(ma, true); 
   atr_l[0] += Spread; 

   // in this loop we're checking all opened positions
   for(i = 0; i < PositionsTotal(); i++)
   {  // process orders with "our" symbols only
      if (Symbol() == PositionGetSymbol(i))
      {  // we will change the values of StopLoss and TakeProfit
         request.action = TRADE_ACTION_SLTP; 
         // process long positions 
         if (PositionGetInteger(POSITION_TYPE) == POSITION_TYPE_BUY)
         {  // let's determine StopLoss
            if (ma[1]>PositionGetDouble(POSITION_PRICE_OPEN)) StopLoss = ma[1];  else StopLoss = lev_lowestOfDay; 
            // if StopLoss is not defined or lower than needed            
            if ((PositionGetDouble(POSITION_SL) == 0 || NormalizeDouble(StopLoss-PositionGetDouble(POSITION_SL), _Digits)>0
               // if TakeProfit is not defined or higer than needed
               || PositionGetDouble(POSITION_TP) == 0 || NormalizeDouble(PositionGetDouble(POSITION_TP)-atr_h[0], _Digits)>0)
               // is new StopLoss close to the current price?
               && NormalizeDouble(SymbolInfoDouble(Symbol(), SYMBOL_BID)-StopLoss-StopLevel, _Digits)>0
               // is new TakeProfit close to the current price?
               && NormalizeDouble(atr_h[0]-SymbolInfoDouble(Symbol(), SYMBOL_BID)-StopLevel, _Digits)>0)
            {
               request.sl = NormalizeDouble(StopLoss, _Digits); 
               request.tp = NormalizeDouble(atr_h[0], _Digits); 
               OrderSend(request, result); 
            }
         }
         // process short positions 
         if (PositionGetInteger(POSITION_TYPE) == POSITION_TYPE_SELL)
         {  // let's determine the value of StopLoss
            if (ma[1]+Spread<PositionGetDouble(POSITION_PRICE_OPEN)) StopLoss = ma[1]+Spread;  else StopLoss = lev_highestOfDay; 
            // if StopLoss is not defined or higher than needed
            if ((PositionGetDouble(POSITION_SL) == 0 || NormalizeDouble(PositionGetDouble(POSITION_SL)-StopLoss, _Digits)>0
               // if TakeProfit is not defined or lower than needed
               || PositionGetDouble(POSITION_TP) == 0 || NormalizeDouble(atr_l[0]-PositionGetDouble(POSITION_TP), _Digits)>0)
               // is new StopLoss close to the current price?
               && NormalizeDouble(StopLoss-SymbolInfoDouble(Symbol(), SYMBOL_ASK)-StopLevel, _Digits)>0
               // is new TakeProfit close to the current price?
               && NormalizeDouble(SymbolInfoDouble(Symbol(), SYMBOL_ASK)-atr_l[0]-StopLevel, _Digits)>0)
            {
               request.sl = NormalizeDouble(StopLoss, _Digits); 
               request.tp = NormalizeDouble(atr_l[0], _Digits); 
               OrderSend(request, result); 
            }
         }
         // if there is an opened position, return from here...
         return; 
      }
   }
   // in this loop we're checking all pending orders
   for(i = 0; i < OrdersTotal(); i++)
   {  // choos each order and get its ticket
      ticket = OrderGetTicket(i); 
      // process orders with "our" symbols only
      if (OrderGetString(ORDER_SYMBOL) == Symbol())
      {  // process Buy Stop orders
         if (OrderGetInteger(ORDER_TYPE) == ORDER_TYPE_BUY_STOP)
         {  // check if there is trading time and price movement is possible
            if (dt.hour >= StartHour && dt.hour<EndHour && lev_highestOfDay<atr_h[0])
            {  // if the opening price is lower than needed
               if ((NormalizeDouble(lev_highestOfDay - OrderGetDouble(ORDER_PRICE_OPEN), _Digits)>0
                  // if StopLoss is not defined or higher than needed
                  || OrderGetDouble(ORDER_SL) == 0 || NormalizeDouble(OrderGetDouble(ORDER_SL)-lev_lowestOfDay, _Digits) != 0)
                  // is opening price close to the current price?
                  && NormalizeDouble(lev_highestOfDay - SymbolInfoDouble(Symbol(), SYMBOL_ASK)-StopLevel, _Digits)>0)
               {
                  // pending order parameters will be changed
                  request.action = TRADE_ACTION_MODIFY; 
                  // put the ticket number to the structure
                  request.order = ticket; 
                  // put the new value of opening price to the structure
                  request.price = NormalizeDouble(lev_highestOfDay, _Digits); 
                  // put new value of StopLoss to the structure
                  request.sl = NormalizeDouble(lev_lowestOfDay, _Digits); 
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
            // setting the flag, that indicates the presence of Buy Stop order
            is_BuyStopOrder = true; 
         }
         // processing Sell Stop orders
         if (OrderGetInteger(ORDER_TYPE) == ORDER_TYPE_SELL_STOP)
         {  // check if there is trading time and price movement is possible
            if (dt.hour >= StartHour && dt.hour<EndHour && lev_lowestOfDay>atr_l[0])
            {  // if the opening price is higher than needed
               if ((NormalizeDouble(OrderGetDouble(ORDER_PRICE_OPEN)-lev_lowestOfDay, _Digits)>0
                  // if StopLoss is not defined or lower than need
                  || OrderGetDouble(ORDER_SL) == 0 || NormalizeDouble(lev_highestOfDay-OrderGetDouble(ORDER_SL), _Digits)>0)
                  // is opening price close to the current price?
                  && NormalizeDouble(SymbolInfoDouble(Symbol(), SYMBOL_BID)-lev_lowestOfDay-StopLevel, _Digits)>0)
               {
                  // pending order parameters will be changed
                  request.action = TRADE_ACTION_MODIFY; 
                  request.order = ticket; 
                  request.price = NormalizeDouble(lev_lowestOfDay, _Digits); 
                  request.sl = NormalizeDouble(lev_highestOfDay, _Digits); 
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
   if (dt.hour >= StartHour && dt.hour<EndHour)
   {
      if (is_BuyStopOrder == false && lev_highestOfDay<atr_h[0])
      {
         request.price = NormalizeDouble(lev_highestOfDay, _Digits); 
         request.sl = NormalizeDouble(lev_lowestOfDay, _Digits); 
         request.type = ORDER_TYPE_BUY_STOP; 
         OrderSend(request, result); 
      }
      if (is_SellStopOrder == false && lev_lowestOfDay>atr_l[0])
      {
         request.price = NormalizeDouble(lev_lowestOfDay, _Digits); 
         request.sl = NormalizeDouble(lev_highestOfDay, _Digits); 
         request.type = ORDER_TYPE_SELL_STOP; 
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
