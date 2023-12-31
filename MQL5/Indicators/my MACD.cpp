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
#property indicator_buffers 6
#property indicator_plots 5

//--- the MACD decreasing period len
#property indicator_label1 "decMACD"
#property indicator_type1 DRAW_LINE
#property indicator_color1 clrGray
#property indicator_style1 STYLE_SOLID
#property indicator_width1 1

//--- the MACD increasing period len
#property indicator_label2 "incMACD"
#property indicator_type2 DRAW_LINE
#property indicator_color2 clrBlueViolet
#property indicator_style2 STYLE_SOLID
#property indicator_width2 1

//--- the OsMA plot
#property indicator_label3 "OsMA"
#property indicator_type3 DRAW_COLOR_HISTOGRAM
#property indicator_color3 clrGreen, clrDarkOrange
#property indicator_style3 STYLE_SOLID
#property indicator_width3 1
//--- the Signal plot
#property indicator_label4 "Signal"
#property indicator_type4 DRAW_LINE
#property indicator_color4 clrRed
#property indicator_style4 STYLE_SOLID
#property indicator_width4 2
//--- the MACD plot
#property indicator_label5 "MACD"
#property indicator_type5 DRAW_LINE
#property indicator_color5 clrPurple
#property indicator_style5 STYLE_SOLID
#property indicator_width5 2

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

double decPeriod_MACDBuffer[];
double incPeriod_MACDBuffer[];

//--- variable for storing the handle of the iMACD indicator
int iMACD_handle;
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
    SetIndexBuffer(0, decPeriod_MACDBuffer,   INDICATOR_DATA);
    SetIndexBuffer(1, incPeriod_MACDBuffer,   INDICATOR_DATA);
    SetIndexBuffer(2, OsMABuffer,   INDICATOR_DATA);
    SetIndexBuffer(3, colorBuffer,  INDICATOR_COLOR_INDEX);
    SetIndexBuffer(4, SignalBuffer, INDICATOR_DATA);
    SetIndexBuffer(5, MACDBuffer,   INDICATOR_DATA);
    
    name = symbol;
    StringTrimRight(name);
    StringTrimLeft(name);
    if (StringLen(name) == 0)
    {
        name = _Symbol;
    }
    iMACD_handle = iMACD(name, period, fast_ema_period, slow_ema_period, signal_period, applied_price);
    if (iMACD_handle == INVALID_HANDLE)
    {
        PrintFormat("Failed to create iMACD_handle of the iMACD indicator for the symbol %s/%s, error code %d",
                    name,
                    EnumToString(period),
                    GetLastError());
        return (INIT_FAILED);
    }
    short_name = StringFormat("iMACD(%s/%s,%d,%d,%d,%s)", name, EnumToString(period),
                              fast_ema_period, slow_ema_period, signal_period, EnumToString(applied_price));
    IndicatorSetString(INDICATOR_SHORTNAME, short_name);
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
    int nr_values_to_copy;
    //--- determine the number of values calculated in the indicator
    int calculated = BarsCalculated(iMACD_handle);

    if (calculated <= 0)
    {
//        PrintFormat("BarsCalculated() returned %d, error code %d", calculated, GetLastError());

        MACDBuffer[0] = 0;
        SignalBuffer[0] = 0;
        OsMABuffer[0] = 0;
        colorBuffer[0] = 0;

        return (0);
    }

    //--- if it is the first start of calculation of the indicator or if the number of values in the iMACD indicator changed
    //---or if it is necessary to calculated the indicator for two or more bars (it means something has changed in the price history)
    if (prev_calculated == 0 || calculated != bars_calculated || rates_total > prev_calculated + 1)
    {
        //--- if the MACDBuffer array is greater than the number of values in the iMACD indicator for symbol/period, then we don't copy everything
        //--- otherwise, we copy less than the size of indicator buffers
        if (calculated > rates_total)
            nr_values_to_copy = rates_total;
        else
            nr_values_to_copy = calculated;
    }
    else
    {
        //--- it means that it's not the first time of the indicator calculation, and since the last call of OnCalculate()
        //--- for calculation not more than one bar is added
        nr_values_to_copy = (rates_total - prev_calculated) + 1;
    }
    //--- fill the arrays with values of the iMACD indicator
    //--- if FillArraysFromBuffer returns false, it means the information is nor ready yet, quit operation
    if (!FillArraysFromBuffers(nr_values_to_copy))
        return (0);

//Print("calc: ", calculated, " pr_c: ", prev_calculated, " bars_c: ", bars_calculated, " rts_tot ", rates_total, " nr_vl_tcpy: ", nr_values_to_copy, " minP:", decPeriod_MACDBuffer[0]);

    //--- form the message
    // string comm = StringFormat("%s ==>  Updated value in the indicator %s: %d",
    //                            TimeToString(TimeCurrent(), TIME_DATE | TIME_SECONDS),
    //                            short_name,
    //                            nr_values_to_copy);
    //--- display the service message on the chart
    // Comment(comm);
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
    if (CopyBuffer(iMACD_handle, 0, 0, amount, MACDBuffer) < 0)
    {
        //--- if the copying fails, tell the error code
        PrintFormat("Failed to copy data from the iMACD indicator, error code %d", GetLastError());
        //--- quit with zero result - it means that the indicator is considered as not calculated
        return (false);
    }

    //--- fill a part of the SignalBuffer array with values from the indicator buffer that has index 1
    if (CopyBuffer(iMACD_handle, 1, 0, amount, SignalBuffer) < 0)
    {
        //--- if the copying fails, tell the error code
        PrintFormat("Failed to copy data from the iMACD indicator, error code %d", GetLastError());
        //--- quit with zero result - it means that the indicator is considered as not calculated
        return (false);
    }
//Print("amount: ", amount);

    OsMABuffer[0] = 0;
    decPeriod_MACDBuffer[0] = 0;
    incPeriod_MACDBuffer[0] = 0;

    for (int i = 0; i < amount; i++)
    {
        OsMABuffer[i] = MACDBuffer[i] - SignalBuffer[i];
//if (i > amount - 7) Print(i, " ", OsMABuffer[i], " ", MACDBuffer[i], " ", SignalBuffer[i]);
        colorBuffer[i] = 0;
        if (i > 0 && OsMABuffer[i] < OsMABuffer[i - 1])
        {
            colorBuffer[i] = 1;
        }

        if (i > 0 && MACDBuffer[i] < MACDBuffer[i-1])
        {   decPeriod_MACDBuffer[i] = decPeriod_MACDBuffer[i-1] - 0.05;    }
        else
        {  decPeriod_MACDBuffer[i] = 0; }

        if (i > 0 && MACDBuffer[i] > MACDBuffer[i-1])
        {   incPeriod_MACDBuffer[i] = incPeriod_MACDBuffer[i-1] + 0.05;    }
        else
        {  incPeriod_MACDBuffer[i] = 0; }

    }
    return (true);
}
//+------------------------------------------------------------------+
// 
//+------------------------------------------------------------------+


//+------------------------------------------------------------------+
//| Indicator deinitialization function                              |
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
    if (iMACD_handle != INVALID_HANDLE)
        IndicatorRelease(iMACD_handle);
    //--- clear the chart after deleting the indicator
    // Comment("");
}