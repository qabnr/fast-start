//+------------------------------------------------------------------+
//|                                               TR_STOP_Signal.mq5 |
//+------------------------------------------------------------------+
#property copyright "GM"
#property link "https://www.gm.com"
#property description "Moving Average Convergence/Divergence"

#property indicator_separate_window
#property indicator_buffers 3
#property indicator_plots   3

#property indicator_label1  "Diff"
#property indicator_type1   DRAW_LINE
#property indicator_color1  clrPurple
#property indicator_width1  1

#property indicator_label2  "Len"
#property indicator_type2   DRAW_LINE
#property indicator_color2  clrRed
#property indicator_width2  1
 
#property indicator_label3  "Comb"
#property indicator_type3   DRAW_HISTOGRAM
#property indicator_color3  clrTeal
#property indicator_width3  2
 
#include "../Include/utils.h"

input int     lookBackPeriod_I = 10;
input double  priceOffset      = 1.6;
input double  weight           = 1.0;

int lookBackPeriod   = lookBackPeriod_I * PeriodSeconds(PERIOD_H1) / PeriodSeconds(PERIOD_CURRENT);

//--- indicator buffers
double diff_buffer[];
double len_buffer[];
double comb_buffer[];

// my TR stop buffers
double stopBuffer[];
double stopColorBuffer[];

int trStop_handle;

/*
    my TR stop buffers:

    SetIndexBuffer(0, buyBuffer,       INDICATOR_DATA);
    SetIndexBuffer(1, buyColorBuffer,  INDICATOR_COLOR_INDEX);

    SetIndexBuffer(2, sellBuffer,      INDICATOR_DATA);
    SetIndexBuffer(3, sellColorBuffer, INDICATOR_COLOR_INDEX);

    SetIndexBuffer(4, stopBuffer,      INDICATOR_DATA);
    SetIndexBuffer(5, stopColorBuffer, INDICATOR_COLOR_INDEX);

*/
const int trStop_buyBufferNumber       = 0;
const int trStop_buyColorBufferNumber  = 1;
const int trStop_sellBufferNumber      = 2;
const int trStop_sellColorBufferNumber = 3;
const int trStop_stopBufferNumber      = 4;
const int trStop_stopColorBufferNumber = 5;

//+------------------------------------------------------------------+
int OnInit()
{
    LOG(SF("lookBackPeriod_I = %d, priceOffset = %.2f, weight = %.2f", lookBackPeriod_I, priceOffset, weight));

    SetIndexBuffer(0, diff_buffer,  INDICATOR_DATA);
    SetIndexBuffer(1, len_buffer,   INDICATOR_DATA);
    SetIndexBuffer(2, comb_buffer,  INDICATOR_DATA);

    SetIndexBuffer(3, stopBuffer,      INDICATOR_CALCULATIONS);
    SetIndexBuffer(4, stopColorBuffer, INDICATOR_CALCULATIONS);

    trStop_handle = iCustom(NULL, PERIOD_CURRENT, "myTR_STOP", lookBackPeriod_I, priceOffset);

    if (trStop_handle == INVALID_HANDLE) {
        PrintFormat("Failed to create iCustom handle of the myTR_STOP indicator for the symbol %s/%s, error code %d",
                    _Symbol, EnumToString(PERIOD_CURRENT), GetLastError());
        return(INIT_FAILED);
    }

    string short_name = StringFormat("TRSTS (%d, %.2f)", lookBackPeriod, priceOffset);
    IndicatorSetString(INDICATOR_SHORTNAME, short_name);

    LOG(short_name + " initialized successfully");
    return(INIT_SUCCEEDED);
}
//+------------------------------------------------------------------+
int OnCalculate(const int       rates_total,
                const int       prev_calculated,
                const datetime &time[],
                const double   &open[],
                const double   &high[],
                const double   &low[],
                const double   &close[],
                const long     &tick_volume[],
                const long     &volume[],
                const int      &spread[])
{
    if (rates_total <= prev_calculated) { return (rates_total); }
// LOG(SF("OnCalculate(%d, %.2f, %.2f) : %d, %d", lookBackPeriod_I, priceOffset, weight, rates_total, prev_calculated));

    if (CopyBufferWithCheck(0, rates_total, trStop_stopBufferNumber,      stopBuffer) <= 0
     || CopyBufferWithCheck(0, rates_total, trStop_stopColorBufferNumber, stopColorBuffer) <= 0) {
        return prev_calculated;
    };

    for (int i = 0; i < rates_total; i++) {
        diff_buffer[i] = 0;
        len_buffer[i] =  10;
        comb_buffer[i] = 20;
        // comb_buffer[i] = stopColorBuffer[i];
    }

    // for (int start = rates_total - 1; start >= 0;) {
    for (int start = rates_total - 1; start > 0;) {
        int i;
        for (i = start; i > 0; i--) {
            if (stopColorBuffer[i] == 2) {
                break;
            }
        }
        for (int j = i; j <= start; j++) {
            int timeDiff  = ((int)time[j] - (int)time[i]) / 3600 ;
            len_buffer[j] = timeDiff;
            diff_buffer[j] = weight * (stopBuffer[j] - stopBuffer[i]);
            comb_buffer[j] = MathSqrt(timeDiff*timeDiff + diff_buffer[j]*diff_buffer[j]);
            if (diff_buffer[j] < 0) {
                comb_buffer[j] = -comb_buffer[j];
                len_buffer[j]  = -len_buffer[j];
            }
        }
        start = i-2;
    }
    return rates_total; 
}
//+------------------------------------------------------------------+
int CopyBufferWithCheck(int startPos, int cnt, int fromBuff, double &toBuff[]) {
    int copied = CopyBuffer(trStop_handle, fromBuff, startPos, cnt, toBuff);
    if(copied <= 0) {
        LOG(SF("CopyBuffer from myTR_STOP handle failed. Error %d", GetLastError()));
        LOG(SF("startPos: %d, cnt: %d, copied: %d", startPos, cnt, copied));
    }
    return copied;
}
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
    LOG("Deinit");
}
