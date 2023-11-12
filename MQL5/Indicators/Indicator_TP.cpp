#property copyright "Denis Zyatkevich"
#property description "This indicator calculates TakeProfit levels"
#property description "using the average market volatility. It uses the values"
#property description "of Average True Range (ATR) indicator, calculated"
#property description "on daily price data. Indicator values are calculated"
#property description "using maximal and minimal price values per day."
#property version   "1.00"
#property indicator_chart_window
#property indicator_buffers 2
#property indicator_plots   2
#property indicator_type1   DRAW_LINE
#property indicator_color1  C'127,191,127'
#property indicator_style1  STYLE_SOLID
#property indicator_label1  "Buy TP"
#property indicator_type2   DRAW_LINE
#property indicator_color2  C'191,127,127'
#property indicator_style2  STYLE_SOLID
#property indicator_label2  "Sell TP"

input int             ATRper       = 5;         //ATR Period
input ENUM_TIMEFRAMES ATRtimeframe = PERIOD_D1; //Indicator timeframe

double buBuffer[],bd[];
int hATR;

void OnInit()
{
   SetIndexBuffer(0,buBuffer,INDICATOR_DATA);
   SetIndexBuffer(1,bd,INDICATOR_DATA);
   hATR=iATR(NULL,ATRtimeframe,ATRper);
}

int OnCalculate(const int rates_total,
                const int prev_calculated,
                const datetime& time[],
                const double& open[],
                const double& high[],
                const double& low[],
                const double& close[],
                const long& tick_volume[],
                const long& volume[],
                const int& spread[])

{
   int i,day_n = 0, day_t = 0;
   double atr[],h_day = 0,l_day = 999999.0;

   CopyBuffer(hATR,0,0,2,atr);
   ArraySetAsSeries(atr,true);

   for(i=prev_calculated;i<rates_total;i++)
   {
      day_t=time[i]/PeriodSeconds(ATRtimeframe);
      if(day_n<day_t)
      {
         day_n=day_t;
         l_day=low[i];
         h_day=high[i];
      }
      else
      {
         if(high[i]>h_day) h_day=high[i];
         if(low[i]<l_day) l_day=low[i];
        }
      buBuffer[i]=l_day+atr[1];
      bd[i]=h_day-atr[1];
   }
   return(rates_total);
}

void OnDeinit(const int reason)
{
   IndicatorRelease(hATR);
}

