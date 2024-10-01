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
#property indicator_type3   DRAW_LINE
#property indicator_color3  C'0,255,8'
#property indicator_width3  1
 
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

string short_name;

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

    short_name = StringFormat("TRSTS (%d, %.2f)", lookBackPeriod, priceOffset);
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
    if (rates_total < lookBackPeriod) {
        LOG(SF("Not enough data. rates_total = %d, lookBackPeriod = %d", rates_total, lookBackPeriod));
        return (0);
    }
    //--- not all data may be calculated
    int calculated = BarsCalculated(trStop_handle);
    if (calculated < rates_total) {
        LOG(SF("Not all data of fastMA_handle is calculated (", calculated, " bars). Error ", GetLastError()));
        return (0);
    }

    int to_copy;
    if (prev_calculated > rates_total || prev_calculated < 0){
        to_copy = rates_total;
    }
    else {
        to_copy = rates_total - prev_calculated;
        if (prev_calculated > 0)
            to_copy++;
    }

    const int start_pos = prev_calculated;

    CopyBufferWithCheck(start_pos, to_copy, trStop_buyBufferNumber,       diff_buffer);
    CopyBufferWithCheck(start_pos, to_copy, trStop_buyColorBufferNumber,  len_buffer);
    CopyBufferWithCheck(start_pos, to_copy, trStop_stopColorBufferNumber, comb_buffer);

    CopyBufferWithCheck(start_pos, to_copy, trStop_stopBufferNumber,      stopBuffer);
    CopyBufferWithCheck(start_pos, to_copy, trStop_stopColorBufferNumber, stopColorBuffer);

    for (int i = prev_calculated; i < rates_total; i++) {
        //len_buffer[i] = 30;
        //comb_buffer[i] = 20;
    }
    return rates_total; 

    if (IsStopped()) { return (0); }
    if (CopyBuffer(trStop_handle, 4, 0, to_copy, stopBuffer) <= 0) {
        LOG(SF("Getting stop buffer data failed! Error ", GetLastError()));
        return (0);
    }
    if (CopyBuffer(trStop_handle, 5, 0, to_copy, stopColorBuffer) <= 0) {
        LOG(SF("Getting stop color buffer data failed! Error ", GetLastError()));
        return (0);
    }

    const int start = (prev_calculated == 0) ? 0 : prev_calculated - 1;
 LOG(SF("start: %d", start));

    //--- find first stop value with a different color
    int i = start;
    for (i = start; i < rates_total && !IsStopped(); i++) {
        if (stopColorBuffer[i] != stopColorBuffer[start]) {
            break;
        }
    }
LOG(SF("i: %d", i));
LOG(SF("stopBuffer[%d] = %.5f", i, stopBuffer[i]));
LOG(SF("stopBuffer[%d] = %.5f", start, stopBuffer[start]));
    // diff_buffer[start] = stopBuffer[i] - stopBuffer[start];
    diff_buffer[start] = 0;
LOG(SF("diff: %.5f", diff_buffer[start]));
    // len_buffer[start]  = i - start;
    // comb_buffer[start] = weight * diff_buffer[start] * diff_buffer[start]
    //  + len_buffer[start] * len_buffer[start];
    len_buffer[start]  = 0;
    // comb_buffer[start] = 0;
LOG(SF("rates_total: %d", rates_total));
    return (rates_total);
}
//+------------------------------------------------------------------+
void CopyBufferWithCheck(int startPos, int cnt, int fromBuff, double &toBuff[]) {
    int copy = CopyBuffer(trStop_handle, fromBuff, startPos, cnt, toBuff);
    if(copy <= 0) PrintFormat("An attempt to get the values if Custom Moving Average has failed. Error %d", GetLastError());
}
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
    LOG("Deinit");
}
