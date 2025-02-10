//+--------------+
//| myZigZag2.mq5 |
//+--------------+

#ifndef __cplusplus
#property copyright "Copyright 2024, Mogyo Software Corp."
#property link      "http://www.mogyo.com"
#property version   "1.0"

#property indicator_separate_window
#property indicator_buffers 5
#property indicator_plots   2

#property indicator_label1  "Diff"
#property indicator_type1   DRAW_SECTION
#property indicator_color1  clrRed
#property indicator_style1  STYLE_SOLID
#property indicator_width1  2

#property indicator_label2  "MinMax"
#property indicator_type2   DRAW_SECTION
#property indicator_color2  clrGreen
#property indicator_style2  STYLE_SOLID
#property indicator_width2  1

#property indicator_label3  "ZigZag"
#property indicator_type3   DRAW_SECTION
#property indicator_color3  clrCadetBlue
#property indicator_style3  STYLE_SOLID
#property indicator_width3  1
#else
#include <adapt.h>
#endif

//+------------------------------------------------------------------+
input int InpDepth_I     = 12;  // Depth
input int InpDeviation_I = 5;   // Deviation
input int InpBackstep_I  = 3;   // Back Step

int InpDepth     = InpDepth_I    * PeriodSeconds(PERIOD_H1) / PeriodSeconds(PERIOD_CURRENT);
int InpDeviation = InpDeviation_I;
int InpBackstep  = InpBackstep_I * PeriodSeconds(PERIOD_H1) / PeriodSeconds(PERIOD_CURRENT);

//+------------------------------------------------------------------+
double   PrcDiffBuffer[];     // diff between open price and last ZigZag value
double   MinMaxBuffer[];      // MinMax buffer
double   ZigZagBuffer[];      // ZigZag buffer
double   HighMapBuffer[];     // ZigZag high extremes (peaks)
double   LowMapBuffer[];      // ZigZag low extremes (bottoms)

int      ExtRecalc = 3;       // number of last extremes for recalculation
enum     SearchMode {
    Any_Extremum =  0, // search for the first extremum
    Peak         =  1, // search for the next ZigZag peak
    Bottom       = -1  // search for the next ZigZag bottom
};
//+------------------------------------------------------------------+
void OnInit() {
     SetIndexBuffer(0, PrcDiffBuffer, INDICATOR_DATA);
     SetIndexBuffer(1, MinMaxBuffer,  INDICATOR_DATA);
     SetIndexBuffer(2, ZigZagBuffer,  INDICATOR_CALCULATIONS);
     SetIndexBuffer(3, HighMapBuffer, INDICATOR_CALCULATIONS);
     SetIndexBuffer(4, LowMapBuffer,  INDICATOR_CALCULATIONS);

    IndicatorSetString(INDICATOR_SHORTNAME,
        StringFormat("ZigZag(%d, %d, %d)", InpDepth, InpDeviation, InpBackstep));

    PlotIndexSetString(0, PLOT_LABEL, "Diff");
    // PlotIndexSetDouble(0, PLOT_EMPTY_VALUE, 0.0);

    PlotIndexSetString(1, PLOT_LABEL, "MinMax");
    // PlotIndexSetDouble(1, PLOT_EMPTY_VALUE, 0.0);

    PlotIndexSetString(2, PLOT_LABEL, "ZZ");
    // PlotIndexSetDouble(2, PLOT_EMPTY_VALUE, 0.0);

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
    if (rates_total < 100) {
        return 0;
    }
    int    start = 0;
    int    lastHighIdx = 0, lastLowIdx = 0;
    double curlow = 0, curhigh = 0, last_high = 0, last_low = 0;
    SearchMode extreme_search = Any_Extremum;

    if (prev_calculated == 0) {
     ArrayInitialize(MinMaxBuffer,  0.0);
     ArrayInitialize(PrcDiffBuffer, 0.0);
     ArrayInitialize(ZigZagBuffer,  0.0);
     ArrayInitialize(HighMapBuffer, 0.0);
     ArrayInitialize(LowMapBuffer,  0.0);
        start = InpDepth;
    }
    else {
        start = rates_total - 1;
        //--- search for the third extremum from the last uncompleted bar
        for(int extreme_counter = 0;
             (extreme_counter < ExtRecalc) && (start > rates_total - 100);
             start--)
        {
            if (ZigZagBuffer[start] != 0.0) {
                extreme_counter++;
            }
        }
        start++;

        //--- what type of extremum we search for
        if (LowMapBuffer[start] != 0.0) {
            curlow = LowMapBuffer[start];
            extreme_search = Peak;
        }
        else {
            curhigh = HighMapBuffer[start];
            extreme_search = Bottom;
        }
        //--- clear indicator values
        for(int i = start + 1; i < rates_total && !IsStopped(); i++) {
            MinMaxBuffer [i] = 0.0;
            PrcDiffBuffer[i] = 0.0;
            ZigZagBuffer [i] = 0.0;
            HighMapBuffer[i] = 0.0;
            LowMapBuffer [i] = 0.0;
        }
    }

//--- search for high and low extremes
    for(int i = start; i < rates_total && !IsStopped(); i++) {
        { //--- low
            double low_val = low[getLowestIndex(low, InpDepth, i)];
            if (low_val == last_low) {
                low_val = 0.0;
            } 
            else {
                last_low = low_val;
                if ((low[i] - low_val) > InpDeviation * _Point) {
                    low_val = 0.0;
                }
                else {
                    for(int back = 1; back <= InpBackstep && i >= back; back++) {
                        if (LowMapBuffer[i - back] > low_val) {
                             LowMapBuffer[i - back] = 0.0;
                        }
                    }
                }
            }
            if (low[i] == low_val) {
                LowMapBuffer[i] = low_val;
                for (int j = i-1; j >= 0; j--) {
                    if (LowMapBuffer[j] != 0) {
                        break;
                    }
                }
            }
            else {
                LowMapBuffer[i] = 0.0;
            }
        }
        { //--- high
            double hi_val = high[getHighestIndex(high, InpDepth, i)];
            if (hi_val == last_high) {
                hi_val = 0.0;
            }
            else {
                last_high = hi_val;
                if ((hi_val - high[i]) > InpDeviation * _Point) {
                    hi_val = 0.0;
                }
                else {
                    for(int back = 1; back <= InpBackstep && i >= back; back++) {
                        if (HighMapBuffer[i - back] < hi_val) {
                            HighMapBuffer[i - back] = 0.0;
                        }
                    }
                }
            }
            if (high[i] == hi_val) {
                HighMapBuffer[i] = hi_val;
            }
            else {
                HighMapBuffer[i] = 0.0;
            }
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
    for(int i = start; i < rates_total && !IsStopped(); i++) {
    //  MinMaxBuffer[i]  = lastHighIdx < lastLowIdx ? last_high : last_low;
     PrcDiffBuffer[i] = lastHighIdx < lastLowIdx ? open[i] - last_high : open[i] - last_low;
        switch(extreme_search) {
            case Any_Extremum: {
                if (last_low == 0.0 && last_high == 0.0) {
                    if (HighMapBuffer[i] != 0) {
                        last_high = high[i];
                        lastHighIdx = i;
                        extreme_search = Bottom;
                        ZigZagBuffer[i] = last_high;
                    }
                    if (LowMapBuffer[i] != 0.0) {
                        last_low = low[i];
                        lastLowIdx = i;
                        extreme_search = Peak;
                        ZigZagBuffer[i] = last_low;
                    }
                }
                break;
            }
            case Peak: {
                if (LowMapBuffer[i] != 0.0 && LowMapBuffer[i] < last_low && HighMapBuffer[i] == 0.0) {
                    ZigZagBuffer[lastLowIdx] = 0.0;
                    lastLowIdx = i;
                    last_low = LowMapBuffer[i];
                    ZigZagBuffer[i] = last_low;
                    zeroOut(i);
                }
                if (HighMapBuffer[i] != 0.0 && LowMapBuffer[i] == 0.0) {
                    last_high = HighMapBuffer[i];
                    lastHighIdx = i;
                    ZigZagBuffer[i] = last_high;
                    zeroOut(i);
                    extreme_search = Bottom;
                }
                break;
            }
            case Bottom: {
                if (HighMapBuffer[i] != 0.0 && HighMapBuffer[i] > last_high && LowMapBuffer[i] == 0.0) {
                    ZigZagBuffer[lastHighIdx] = 0.0;
                    lastHighIdx = i;
                    last_high = HighMapBuffer[i];
                    ZigZagBuffer[i] = last_high;
                    zeroOut(i);
                }
                if (LowMapBuffer[i] != 0.0 && HighMapBuffer[i] == 0.0) {
                    last_low = LowMapBuffer[i];
                    lastLowIdx = i;
                    ZigZagBuffer[i] = last_low;
                    zeroOut(i);
                    extreme_search = Peak;
                }
                break;
            }
            default:
                return rates_total;
        }
    }
    return rates_total;
}
//+------------------------------------------------------------------+
void zeroOut(const int i)
{
    for (int back = i-1; back >= 0; back--) {
        if (ZigZagBuffer[back] == 0) {
            HighMapBuffer[back] = 0;
            LowMapBuffer [back] = 0;
        }
    }
}
//+------------------------------------------------------------------+
//|  Search for the index of the highest bar                         |
//+------------------------------------------------------------------+
int getHighestIndex(const double &array[], const int depth, const int start) {
    if (start < 0) {
        return 0;
    }
    double max = array[start];
    int    index = start;

    for(int i = start - 1; i > start - depth && i >= 0; i--) {
        if (array[i] > max) {
            index = i;
            max = array[i];
        }
    }
    return index;
}
//+------------------------------------------------------------------+
//|  Search for the index of the lowest bar                          |
//+------------------------------------------------------------------+
int getLowestIndex(const double &array[], const int depth, const int start) {
    if (start < 0) {
        return 0;
    }
    double min = array[start];
    int    index = start;

    for(int i = start - 1; i > start - depth && i >= 0; i--) {
        if (array[i] < min) {
            index = i;
            min = array[i];
        }
    }
    return index;
}
//+------------------------------------------------------------------+
