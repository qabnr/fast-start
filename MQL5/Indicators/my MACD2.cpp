//+------------------------------------------------------------------+
//|                                                         MACD.mq5 |
//|                             Copyright 2000-2024, MetaQuotes Ltd. |
//|                                             https://www.mql5.com |
//+------------------------------------------------------------------+
#property copyright "Copyright 2000-2024, MetaQuotes Ltd."
#property link "https://www.mql5.com"
#property description "Moving Average Convergence/Divergence"

#property indicator_separate_window
#property indicator_buffers 6
#property indicator_plots   4

#property indicator_label1  "MACD"
#property indicator_type1   DRAW_LINE
#property indicator_color1  clrPurple
#property indicator_width1  1

#property indicator_label2  "Signal"
#property indicator_type2   DRAW_LINE
#property indicator_color2  clrRed
#property indicator_width2  1
 
#property indicator_label3  "OsMA"
#property indicator_type3   DRAW_COLOR_HISTOGRAM
#property indicator_color3  clrGreen, clrDarkOrange
#property indicator_style3  STYLE_SOLID
#property indicator_width3  5

input int fastEMA_period_I   = 96;
input int slowEMA_period_I   = 208; 
input int signalSMA_period_I = 72; 
input ENUM_APPLIED_PRICE appliedPrice = PRICE_TYPICAL; 

//--- indicator buffers
double MACD_buffer[];
double Signal_buffer[];
double FastMA_buffer[];
double SlowMA_buffer[];

double OsMA_buffer[];
double OsMAcolor_buffer[];

int fastEMA_period   = fastEMA_period_I   * PeriodSeconds(PERIOD_H1) / PeriodSeconds(PERIOD_CURRENT);
int slowEMA_period   = slowEMA_period_I   * PeriodSeconds(PERIOD_H1) / PeriodSeconds(PERIOD_CURRENT); 
int signalSMA_period = signalSMA_period_I * PeriodSeconds(PERIOD_H1) / PeriodSeconds(PERIOD_CURRENT); 

int fastMA_handle;
int slowMA_handle;

//+------------------------------------------------------------------+
void OnInit()
{
    //--- indicator buffers mapping
    SetIndexBuffer(0, MACD_buffer,      INDICATOR_DATA);
    SetIndexBuffer(1, Signal_buffer,    INDICATOR_DATA);
    SetIndexBuffer(2, OsMA_buffer,      INDICATOR_DATA);
    SetIndexBuffer(3, OsMAcolor_buffer, INDICATOR_COLOR_INDEX);
    SetIndexBuffer(4, FastMA_buffer,    INDICATOR_CALCULATIONS);
    SetIndexBuffer(5, SlowMA_buffer,    INDICATOR_CALCULATIONS);

    PlotIndexSetInteger(1, PLOT_DRAW_BEGIN, signalSMA_period - 1);
    PlotIndexSetInteger(2, PLOT_DRAW_BEGIN, signalSMA_period - 1);

    string short_name = StringFormat("myMACD2(%d,%d,%d)", fastEMA_period, slowEMA_period, signalSMA_period);
    IndicatorSetString(INDICATOR_SHORTNAME, short_name);

    int shift = 0;
    fastMA_handle = iMA(NULL, PERIOD_CURRENT, fastEMA_period, shift, MODE_EMA, appliedPrice);
    slowMA_handle = iMA(NULL, PERIOD_CURRENT, slowEMA_period, shift, MODE_EMA, appliedPrice);
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
    if (rates_total < signalSMA_period) {
        return (0);
    }
    //--- not all data may be calculated
    int calculated = BarsCalculated(fastMA_handle);
    if (calculated < rates_total) {
        // Print("Not all data of fastMA_handle is calculated (", calculated, " bars). Error ", GetLastError());
        return (0);
    }
    calculated = BarsCalculated(slowMA_handle);
    if (calculated < rates_total) {
        // Print("Not all data of slowMA_handle is calculated (", calculated, " bars). Error ", GetLastError());
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
    if (IsStopped()) {
        return (0);
    }
    if (CopyBuffer(fastMA_handle, 0, 0, to_copy, FastMA_buffer) <= 0) {
        Print("Getting fast EMA is failed! Error ", GetLastError());
        return (0);
    }

    if (IsStopped()) {
        return (0);
    }
    if (CopyBuffer(slowMA_handle, 0, 0, to_copy, SlowMA_buffer) <= 0) {
        Print("Getting slow SMA is failed! Error ", GetLastError());
        return (0);
    }

    int start = (prev_calculated == 0) ? 0 : prev_calculated - 1;

    for (int i = start; i < rates_total && !IsStopped(); i++) {
        MACD_buffer[i] = FastMA_buffer[i] - SlowMA_buffer[i];
    }
    //--- calculate Signal
    SimpleMAOnBuffer(rates_total, prev_calculated);

    return (rates_total);
}
//+------------------------------------------------------------------+
void SimpleMAOnBuffer(const int rates_total, const int prev_calculated)
{
    //--- check period
    if (signalSMA_period <= 1 || signalSMA_period > rates_total) {
        return;
    }

    int start_position;

    if (prev_calculated == 0) {
        start_position = signalSMA_period;
        for (int i = 0; i < start_position; i++) {
            Signal_buffer[i] = 0.0;
            OsMA_buffer[i] = OsMAcolor_buffer[i] = 0;
        }
        double first_value = 0;
        for (int i = 0; i < start_position; i++) {
            first_value += MACD_buffer[i];
        }
        Signal_buffer[start_position - 1] = first_value / signalSMA_period;
    }
    else {
        start_position = prev_calculated - 1;
    }
    
    for (int i = start_position; i < rates_total; i++) {
        Signal_buffer[i] = Signal_buffer[i - 1] + (MACD_buffer[i] - MACD_buffer[i - signalSMA_period]) / signalSMA_period;

        OsMA_buffer[i] = MACD_buffer[i] - Signal_buffer[i];
        OsMAcolor_buffer[i] = 0;
        if (i > 0) {
            if (OsMA_buffer[i] < OsMA_buffer[i - 1]) {
                OsMAcolor_buffer[i] = 1; 
            }
        }
    }
}
