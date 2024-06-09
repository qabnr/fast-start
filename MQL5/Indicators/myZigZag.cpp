//+--------------+
//| myZigZag.mq5 |
//+--------------+
#property copyright "Copyright 2024, Mogyo Software Corp."
#property link      "http://www.mogyo.com"
#property version   "1.0"

#property indicator_chart_window
#property indicator_buffers 3
#property indicator_plots   3

#property indicator_label1  "ZigZag"
#property indicator_type1   DRAW_SECTION
#property indicator_color1  clrCadetBlue
#property indicator_style1  STYLE_SOLID
#property indicator_width1  1

#property indicator_label2  "H"
#property indicator_type2   DRAW_SECTION
#property indicator_color2  clrRed
#property indicator_style2  STYLE_SOLID
#property indicator_width2  1

#property indicator_label3  "L"
#property indicator_type3   DRAW_SECTION
#property indicator_color3  clrGreen
#property indicator_style3  STYLE_SOLID
#property indicator_width3  1

//--- input parameters
input int InpDepth     = 12;  // Depth
input int InpDeviation = 5;   // Deviation
input int InpBackstep  = 3;   // Back Step

double   ZigZagBuffer[];      // main buffer
double   HighMapBuffer[];     // ZigZag high extremes (peaks)
double   LowMapBuffer[];      // ZigZag low extremes (bottoms)

int       ExtRecalc = 3;         // number of last extremes for recalculation

enum EnSearchMode {
   Any_Extremum = 0, // searching for the first extremum
   Peak = 1,         // searching for the next ZigZag peak
   Bottom = -1       // searching for the next ZigZag bottom
};
//+------------------------------------------------------------------+
void OnInit() {
//--- indicator buffers mapping
   SetIndexBuffer(0, ZigZagBuffer,  INDICATOR_DATA);
   SetIndexBuffer(1, HighMapBuffer, INDICATOR_DATA);
   SetIndexBuffer(2, LowMapBuffer,  INDICATOR_DATA);

   IndicatorSetString(INDICATOR_SHORTNAME,
      StringFormat("ZigZag(%d, %d, %d)", InpDepth, InpDeviation, InpBackstep));

   PlotIndexSetString(0, PLOT_LABEL, "ZZ");
   PlotIndexSetString(1, PLOT_LABEL, "H");
   PlotIndexSetString(2, PLOT_LABEL, "L");

   PlotIndexSetDouble(0, PLOT_EMPTY_VALUE, 0.0);
   PlotIndexSetDouble(1, PLOT_EMPTY_VALUE, 0.0);
   PlotIndexSetDouble(2, PLOT_EMPTY_VALUE, 0.0);

   IndicatorSetInteger(INDICATOR_DIGITS, _Digits);
}
//+------------------------------------------------------------------+
int OnCalculate(const int        rates_total,
                const int        prev_calculated,
                const datetime   &time[],
                const double     &open[],
                const double     &high[],
                const double     &low[],
                const double     &close[],
                const long       &tick_volume[],
                const long       &volume[],
                const int        &spread[])
{
   if (rates_total < 100)
      return 0;

   int    start = 0, extreme_counter = 0, extreme_search = Any_Extremum;
   int    shift = 0, back = 0, last_high_pos = 0, last_low_pos = 0;
   double curlow = 0, curhigh = 0, last_high = 0, last_low = 0;

   if (prev_calculated == 0) {
      ArrayInitialize(ZigZagBuffer,  0.0);
      ArrayInitialize(HighMapBuffer, 0.0);
      ArrayInitialize(LowMapBuffer,  0.0);
      start = InpDepth;
   }
   else {
      int i;
      //--- searching for the third extremum from the last uncompleted bar
      for(i = rates_total - 1;
          (extreme_counter < ExtRecalc) && (i > rates_total - 100);
          i--)
      {
         if (ZigZagBuffer[i] != 0.0)
            extreme_counter++;
         i--;
      }
      i++;
      start = i;

      //--- what type of exremum we search for
      if (LowMapBuffer[i] != 0.0) {
         curlow = LowMapBuffer[i];
         extreme_search = Peak;
      }
      else {
         curhigh = HighMapBuffer[i];
         extreme_search = Bottom;
      }
      //--- clear indicator values
      for(i = start + 1; i < rates_total && !IsStopped(); i++) {
         ZigZagBuffer[i]  = 0.0;
         LowMapBuffer[i]  = 0.0;
         HighMapBuffer[i] = 0.0;
      }
   }

//--- searching for high and low extremes
   for(shift = start; shift < rates_total && !IsStopped(); shift++) {
      { //--- low
         double low_val = low[Lowest(low, InpDepth, shift)];
         if (low_val == last_low)
            low_val = 0.0;
         else {
            last_low = low_val;
            if ((low[shift] - low_val) > InpDeviation * _Point) {
               low_val = 0.0;
            }
            else {
               for(back = 1; back <= InpBackstep && shift >= back; back++) {
                  double res = LowMapBuffer[shift - back];
                  if ((res != 0) && (res > low_val))
                     LowMapBuffer[shift - back] = 0.0;
               }
            }
         }
         if (low[shift] == low_val)
            LowMapBuffer[shift] = low_val;
         else
            LowMapBuffer[shift] = 0.0;
      }
      { //--- high
         double hi_val = high[Highest(high, InpDepth, shift)];
         if (hi_val == last_high)
            hi_val = 0.0;
         else {
            last_high = hi_val;
            if ((hi_val - high[shift]) > InpDeviation * _Point)
               hi_val = 0.0;
            else {
               for(back = 1; back <= InpBackstep && shift >= back; back++) {
                  double res = HighMapBuffer[shift - back];
                  if ((res != 0) && (res < hi_val))
                     HighMapBuffer[shift - back] = 0.0;
               }
            }
         }
         if (high[shift] == hi_val)
            HighMapBuffer[shift] = hi_val;
         else
            HighMapBuffer[shift] = 0.0;
      }
   }

   if (extreme_search == Any_Extremum) {
      last_low  = 0.0;
      last_high = 0.0;
   }
   else {
      last_low  = curlow;
      last_high = curhigh;
   }

//--- final selection of extreme points for ZigZag
   for(shift = start; shift < rates_total && !IsStopped(); shift++) {
      switch(extreme_search) {
         case Any_Extremum: {
            if (last_low == 0.0 && last_high == 0.0) {
               if (HighMapBuffer[shift] != 0) {
                  last_high = high[shift];
                  last_high_pos = shift;
                  extreme_search = Bottom;
                  ZigZagBuffer[shift] = last_high;
               }
               if (LowMapBuffer[shift] != 0.0) {
                  last_low = low[shift];
                  last_low_pos = shift;
                  extreme_search = Peak;
                  ZigZagBuffer[shift] = last_low;
               }
            }
            break;
         }
         case Peak: {
            if (LowMapBuffer[shift] != 0.0 && LowMapBuffer[shift] < last_low && HighMapBuffer[shift] == 0.0) {
               ZigZagBuffer[last_low_pos] = 0.0;
               last_low_pos = shift;
               last_low = LowMapBuffer[shift];
               ZigZagBuffer[shift] = last_low;
            }
            if (HighMapBuffer[shift] != 0.0 && LowMapBuffer[shift] == 0.0) {
               last_high = HighMapBuffer[shift];
               last_high_pos = shift;
               ZigZagBuffer[shift] = last_high;
               extreme_search = Bottom;
            }
            break;
         }
         case Bottom: {
            if (HighMapBuffer[shift] != 0.0 && HighMapBuffer[shift] > last_high && LowMapBuffer[shift] == 0.0) {
               ZigZagBuffer[last_high_pos] = 0.0;
               last_high_pos = shift;
               last_high = HighMapBuffer[shift];
               ZigZagBuffer[shift] = last_high;
            }
            if (LowMapBuffer[shift] != 0.0 && HighMapBuffer[shift] == 0.0) {
               last_low = LowMapBuffer[shift];
               last_low_pos = shift;
               ZigZagBuffer[shift] = last_low;
               extreme_search = Peak;
            }
            break;
         }
         default:
            return(rates_total);
      }
   }

//--- return value of prev_calculated for next call
   return(rates_total);
}
//+------------------------------------------------------------------+
//|  Search for the index of the highest bar                         |
//+------------------------------------------------------------------+
int Highest(const double &array[], const int depth, const int start) {
   if (start < 0)
      return(0);

   double max = array[start];
   int    index = start;
//--- start searching
   for(int i = start - 1; i > start - depth && i >= 0; i--) {
      if (array[i] > max) {
         index = i;
         max = array[i];
      }
   }
//--- return index of the highest bar
   return(index);
}
//+------------------------------------------------------------------+
//|  Search for the index of the lowest bar                          |
//+------------------------------------------------------------------+
int Lowest(const double &array[], const int depth, const int start) {
   if (start < 0)
      return(0);

   double min = array[start];
   int    index = start;
//--- start searching
   for(int i = start - 1; i > start - depth && i >= 0; i--) {
      if (array[i] < min) {
         index = i;
         min = array[i];
      }
   }
//--- return index of the lowest bar
   return(index);
}
//+------------------------------------------------------------------+
