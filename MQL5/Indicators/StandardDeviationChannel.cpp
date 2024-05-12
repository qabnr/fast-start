//+------------------------------------------------------------------+
//|                                     StandardDeviationChannel.mq5 |
//|                                                   Sergey Greecie |
//|                                               sergey1294@list.ru |
//+------------------------------------------------------------------+
#property copyright "Sergey Greecie"
#property link "sergey1294@list.ru"
#property version "1.00"

#property indicator_chart_window
#property indicator_buffers 5
#property indicator_plots   3
#property indicator_type1   DRAW_LINE
#property indicator_type2   DRAW_LINE
#property indicator_type3   DRAW_LINE
#property indicator_color1  DodgerBlue
#property indicator_color2  DodgerBlue
#property indicator_color3  Blue
#property indicator_style3  STYLE_DOT


input int                Inp_MAPeriod     = 14;
input ENUM_MA_METHOD     Inp_MAMethod     = MODE_SMA;
input ENUM_APPLIED_PRICE Inp_AppliedPrice = PRICE_CLOSE;
input double             Inp_Deviation    = 2.0;

double                   ExtUpBuffer[];
double                   ExtDownBuffer[];
double                   ExtMiddBuffer[];
double                   ExtMABuffer[];
double                   ExtStdDevBuffer[];

int                      ExtMAHandle;
int                      ExtStdDevMAHandle;

//+------------------------------------------------------------------+
//| Custom indicator initialization function                         |
//+------------------------------------------------------------------+
int OnInit()
{
    //--- indicator buffers mapping
    SetIndexBuffer(0, ExtUpBuffer,     INDICATOR_DATA);
    SetIndexBuffer(1, ExtDownBuffer,   INDICATOR_DATA);
    SetIndexBuffer(2, ExtMiddBuffer,   INDICATOR_DATA);
    SetIndexBuffer(3, ExtMABuffer,     INDICATOR_CALCULATIONS);
    SetIndexBuffer(4, ExtStdDevBuffer, INDICATOR_CALCULATIONS);
    //--- set first bar from what index will be drawn
    PlotIndexSetInteger(0, PLOT_DRAW_BEGIN, Inp_MAPeriod - 1);
    PlotIndexSetInteger(1, PLOT_DRAW_BEGIN, Inp_MAPeriod - 1);
    PlotIndexSetInteger(2, PLOT_DRAW_BEGIN, Inp_MAPeriod - 1);
    //--- get handles
    ExtMAHandle       = iMA    (NULL, 0, Inp_MAPeriod, 0, Inp_MAMethod, Inp_AppliedPrice);
    ExtStdDevMAHandle = iStdDev(NULL, 0, Inp_MAPeriod, 0, Inp_MAMethod, Inp_AppliedPrice);
    return (0);
}
//+------------------------------------------------------------------+
//| Custom indicator iteration function                              |
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
    if (rates_total < Inp_MAPeriod) return (0);

    int calculated = BarsCalculated(ExtMAHandle);
    if (calculated < rates_total) {
        Print("Not all data of ExtMAHandle is calculated (", calculated, "bars ). Error", GetLastError());
        return (0);
    }
    calculated = BarsCalculated(ExtStdDevMAHandle);
    if (calculated < rates_total) {
        Print("Not all data of ExtStdDevMAHandle is calculated (", calculated, "bars ). Error", GetLastError());
        return (0);
    }

    int to_copy;
    if (prev_calculated > rates_total || prev_calculated < 0) {
        to_copy = rates_total;
    }
    else {
        to_copy = rates_total - prev_calculated;
        if (prev_calculated > 0) to_copy++;
    }
    //--- get MA buffer
    if (CopyBuffer(ExtMAHandle, 0, 0, to_copy, ExtMABuffer) <= 0) {
        Print("Getting fast MA failed! Error", GetLastError());
        return (0);
    }
    //--- get StdDev buffer
    if (CopyBuffer(ExtStdDevMAHandle, 0, 0, to_copy, ExtStdDevBuffer) <= 0) {
        Print("Getting slow StdDev failed! Error", GetLastError());
        return (0);
    }
    //---
    int limit;
    if (prev_calculated == 0) { limit = 0; }
    else { limit = prev_calculated - 1; }

    for (int i = limit; i < rates_total; i++) {
        ExtMiddBuffer[i] = ExtMABuffer[i];
        ExtUpBuffer  [i] = ExtMABuffer[i] + (Inp_Deviation * ExtStdDevBuffer[i]);
        ExtDownBuffer[i] = ExtMABuffer[i] - (Inp_Deviation * ExtStdDevBuffer[i]);
    }

    return rates_total;
}
//+------------------------------------------------------------------+
