//+------------------------------------------------------------------+
//|                                                   Demo_iMACD.mq5 |
//|                        Copyright 2011, MetaQuotes Software Corp. |
//|                                             https://www.mql5.com |
//+------------------------------------------------------------------+
#property copyright "Copyright 2011, MetaQuotes Software Corp."
#property link "https://www.mql5.com"
#property version "1.00"
#property description "The indicator demonstrates how to obtain data"
#property description "of indicator buffers for the iMACD technical indicator."
#property description "A symbol and timeframe used for calculation of the indicator,"
#property description "are set by the symbol and period parameters."
#property description "All other parameters like in the standard MACD."

#property indicator_separate_window
#property indicator_buffers 4
#property indicator_plots 3

//--- the OsMA plot
#property indicator_label1 "OsMA"
#property indicator_type1 DRAW_COLOR_HISTOGRAM
#property indicator_color1 clrGreen, clrDarkOrange
#property indicator_style1 STYLE_SOLID
#property indicator_width1 1
//--- the Signal plot
#property indicator_label2 "Signal"
#property indicator_type2 DRAW_LINE
#property indicator_color2 clrRed
#property indicator_style2 STYLE_SOLID
#property indicator_width2 2
//--- the MACD plot
#property indicator_label3 "MACD"
#property indicator_type3 DRAW_LINE
#property indicator_color3 clrPurple
#property indicator_style3 STYLE_SOLID
#property indicator_width3 2

//+------------------------------------------------------------------+
//| Enumeration of the methods of handle creation                    |
//+------------------------------------------------------------------+
enum Creation
{
    Call_iMACD,          // use iMACD
    Call_IndicatorCreate // use IndicatorCreate
};
//--- input parameters
input int fast_ema_period = 12;                         // period of fast ma
input int slow_ema_period = 26;                         // period of slow ma
input int signal_period = 9;                            // period of averaging of difference
input ENUM_APPLIED_PRICE applied_price = PRICE_TYPICAL; // type of price
input string symbol = " ";                              // symbol
input ENUM_TIMEFRAMES period = PERIOD_CURRENT;          // timeframe

//--- indicator buffers
double MACDBuffer[];
double SignalBuffer[];
double OsMABuffer[];
double colorBuffer[];


//--- variable for storing the handle of the iMACD indicator
int handle;
//--- variable for storing
string name = symbol;
//--- name of the indicator on a chart
string short_name;
//--- we will keep the number of values in the Moving Averages Convergence/Divergence indicator
int bars_calculated = 0;
//+------------------------------------------------------------------+
//| Custom indicator initialization function                         |
//+------------------------------------------------------------------+
int OnInit()
{
    //--- assignment of arrays to indicator buffers
    SetIndexBuffer(0, OsMABuffer,   INDICATOR_DATA);
    SetIndexBuffer(1, colorBuffer,  INDICATOR_COLOR_INDEX);
    SetIndexBuffer(2, MACDBuffer,   INDICATOR_DATA);
    SetIndexBuffer(3, SignalBuffer, INDICATOR_DATA);
    
    //--- determine the symbol the indicator is drawn for
    name = symbol;
    //--- delete spaces to the right and to the left
    StringTrimRight(name);
    StringTrimLeft(name);
    //--- if it results in zero length of the 'name' string
    if (StringLen(name) == 0)
    {
        //--- take the symbol of the chart the indicator is attached to
        name = _Symbol;
    }
    //--- create handle of the indicator
        handle = iMACD(name, period, fast_ema_period, slow_ema_period, signal_period, applied_price);
    //--- if the handle is not created
    if (handle == INVALID_HANDLE)
    {
        //--- tell about the failure and output the error code
        PrintFormat("Failed to create handle of the iMACD indicator for the symbol %s/%s, error code %d",
                    name,
                    EnumToString(period),
                    GetLastError());
        //--- the indicator is stopped early
        return (INIT_FAILED);
    }
    //--- show the symbol/timeframe the Moving Average Convergence/Divergence indicator is calculated for
    short_name = StringFormat("iMACD(%s/%s,%d,%d,%d,%s)", name, EnumToString(period),
                              fast_ema_period, slow_ema_period, signal_period, EnumToString(applied_price));
    IndicatorSetString(INDICATOR_SHORTNAME, short_name);
    //--- normal initialization of the indicator
    return (INIT_SUCCEEDED);
}
//+------------------------------------------------------------------+
//| Custom indicator iteration function                              |
//+------------------------------------------------------------------+
int OnCalculate(const int rates_total,
                const int prev_calculated,
                const datetime &time[],
                const double &open[],
                const double &high[],
                const double &low[],
                const double &close[],
                const long &tick_volume[],
                const long &volume[],
                const int &spread[])
{
    //--- number of values copied from the iMACD indicator
    int values_to_copy;
    //--- determine the number of values calculated in the indicator
    int calculated = BarsCalculated(handle);
    if (calculated <= 0)
    {
        //PrintFormat("BarsCalculated() returned %d, error code %d", calculated, GetLastError());
        return (0);
    }
    //--- if it is the first start of calculation of the indicator or if the number of values in the iMACD indicator changed
    //---or if it is necessary to calculated the indicator for two or more bars (it means something has changed in the price history)
    if (prev_calculated == 0 || calculated != bars_calculated || rates_total > prev_calculated + 1)
    {
        //--- if the MACDBuffer array is greater than the number of values in the iMACD indicator for symbol/period, then we don't copy everything
        //--- otherwise, we copy less than the size of indicator buffers
        if (calculated > rates_total)
            values_to_copy = rates_total;
        else
            values_to_copy = calculated;
    }
    else
    {
        //--- it means that it's not the first time of the indicator calculation, and since the last call of OnCalculate()
        //--- for calculation not more than one bar is added
        values_to_copy = (rates_total - prev_calculated) + 1;
    }
    //--- fill the arrays with values of the iMACD indicator
    //--- if FillArraysFromBuffer returns false, it means the information is nor ready yet, quit operation
    if (!FillArraysFromBuffers(values_to_copy))
        return (0);
    //--- form the message
    string comm = StringFormat("%s ==>  Updated value in the indicator %s: %d",
                               TimeToString(TimeCurrent(), TIME_DATE | TIME_SECONDS),
                               short_name,
                               values_to_copy);
    //--- display the service message on the chart
    Comment(comm);
    //--- memorize the number of values in the Moving Averages indicator Convergence/Divergence
    bars_calculated = calculated;
    //--- return the prev_calculated value for the next call
    return (rates_total);
}
//+------------------------------------------------------------------+
//| Filling indicator buffers from the iMACD indicator               |
//+------------------------------------------------------------------+
bool FillArraysFromBuffers(int amount)
{
    //--- reset error code
    ResetLastError();
    //--- fill a part of the iMACDBuffer array with values from the indicator buffer that has 0 index
    if (CopyBuffer(handle, 0, 0, amount, MACDBuffer) < 0)
    {
        //--- if the copying fails, tell the error code
        PrintFormat("Failed to copy data from the iMACD indicator, error code %d", GetLastError());
        //--- quit with zero result - it means that the indicator is considered as not calculated
        return (false);
    }

    //--- fill a part of the SignalBuffer array with values from the indicator buffer that has index 1
    if (CopyBuffer(handle, 1, 0, amount, SignalBuffer) < 0)
    {
        //--- if the copying fails, tell the error code
        PrintFormat("Failed to copy data from the iMACD indicator, error code %d", GetLastError());
        //--- quit with zero result - it means that the indicator is considered as not calculated
        return (false);
    }
    for (int i = 0; i < amount; i++)
    {
        OsMABuffer[i] = MACDBuffer[i] - SignalBuffer[i];
        colorBuffer[i] = 0;
        if (i > 0 && OsMABuffer[i] < OsMABuffer[i - 1])
        {
            colorBuffer[i] = 1;
        }
    }
    //--- everything is fine
    return (true);
}
//+------------------------------------------------------------------+
//| Indicator deinitialization function                              |
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
    if (handle != INVALID_HANDLE)
        IndicatorRelease(handle);
    //--- clear the chart after deleting the indicator
    Comment("");
}