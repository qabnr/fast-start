//+------------------------------------------------------------------+
//| my iMACD.mq5
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
#property indicator_buffers 8
#property indicator_plots 7

//--- the MACD decreasing period len
#property indicator_label1 "decMACD"
#property indicator_type1 DRAW_LINE
#property indicator_color1 clrBlack
#property indicator_style1 STYLE_SOLID
#property indicator_width1 1

//--- the MACD increasing period len
#property indicator_label2 "incMACD"
#property indicator_type2 DRAW_LINE
#property indicator_color2 clrBlack
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
#property indicator_type4  DRAW_LINE
#property indicator_color4 clrRed
#property indicator_style4 STYLE_SOLID
#property indicator_width4 2
//--- the MACD plot
#property indicator_label5 "MACD"
#property indicator_type5  DRAW_LINE
#property indicator_color5 clrPurple
#property indicator_style5 STYLE_SOLID
#property indicator_width5 2

//--- the OsMA decreasing period len
#property indicator_label6 "decOsMA"
#property indicator_type6  DRAW_LINE
#property indicator_color6 clrBlack
#property indicator_style6 STYLE_SOLID
#property indicator_width6 1

//--- the OsMA increasing period len
#property indicator_label7 "incOsMA"
#property indicator_type7  DRAW_LINE
#property indicator_color7 clrBlack
#property indicator_style7 STYLE_SOLID
#property indicator_width7 1

//--- input parameters
input int fast_ema_period = 12;                         // period of fast ma
input int slow_ema_period = 26;                         // period of slow ma
input int signal_period = 9;                            // period of averaging of difference
/* input */ ENUM_APPLIED_PRICE applied_price = PRICE_TYPICAL; // type of price
/* input */ ENUM_TIMEFRAMES period = PERIOD_CURRENT;          // timeframe

//--- indicator buffers
double MACDBuffer[];
double SignalBuffer[];
double OsMABuffer[];
double colorBuffer[];

double decPeriod_MACDBuffer[];
double incPeriod_MACDBuffer[];
double decPeriod_OsMABuffer[];
double incPeriod_OsMABuffer[];

int iMACD_handle;

//+------------------------------------------------------------------+
int OnInit()
//+------------------------------------------------------------------+
{
    SetIndexBuffer(0, decPeriod_MACDBuffer, INDICATOR_DATA);
    SetIndexBuffer(1, incPeriod_MACDBuffer, INDICATOR_DATA);
    SetIndexBuffer(2, OsMABuffer,           INDICATOR_DATA);
    SetIndexBuffer(3, colorBuffer,          INDICATOR_COLOR_INDEX);
    SetIndexBuffer(4, SignalBuffer,         INDICATOR_DATA);
    SetIndexBuffer(5, MACDBuffer,           INDICATOR_DATA);
    SetIndexBuffer(6, decPeriod_OsMABuffer, INDICATOR_DATA);
    SetIndexBuffer(7, incPeriod_OsMABuffer, INDICATOR_DATA);
    
    iMACD_handle = iMACD(_Symbol, period, fast_ema_period, slow_ema_period, signal_period, applied_price);
    if (iMACD_handle == INVALID_HANDLE)
    {
        PrintFormat("Failed to create iMACD_handle of the iMACD indicator for the symbol %s/%s, error code %d",
                    _Symbol,
                    EnumToString(period),
                    GetLastError());
        return (INIT_FAILED);
    }
    // string short_name = StringFormat("iMACD(%s/%s,%d,%d,%d,%s)", _Symbol, EnumToString(period),
    //                           fast_ema_period, slow_ema_period, signal_period, EnumToString(applied_price));
    string short_name = StringFormat("myMACD(%d,%d,%d)",
                              fast_ema_period, slow_ema_period, signal_period);
    IndicatorSetString(INDICATOR_SHORTNAME, short_name);
    return (INIT_SUCCEEDED);
}
//+------------------------------------------------------------------+
int OnCalculate(const int       rates_total,
//+------------------------------------------------------------------+
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
    static int bars_calculated = 0;
    int nr_values_to_copy;
    int calculated = BarsCalculated(iMACD_handle);

    if (calculated <= 0)
    {
//        PrintFormat("BarsCalculated() returned %d, error code %d", calculated, GetLastError());
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
    if (!FillArraysFromBuffers(nr_values_to_copy, prev_calculated))
    {   return (0); }

    bars_calculated = calculated;
    //--- return the prev_calculated value for the next call
    return (rates_total);
}
//+------------------------------------------------------------------+
bool FillArraysFromBuffers(int amount, int prev_calculated)
//+------------------------------------------------------------------+
{
    ResetLastError();
    if (CopyBuffer(iMACD_handle, 0, 0, amount, MACDBuffer) < 0)
    {
        PrintFormat("Failed to copy data from the iMACD indicator, error code %d", GetLastError());
        return (false);
    }

    if (CopyBuffer(iMACD_handle, 1, 0, amount, SignalBuffer) < 0)
    {
        PrintFormat("Failed to copy data from the iMACD indicator, error code %d", GetLastError());
        return (false);
    }

    OsMABuffer[0] = 0;
    decPeriod_MACDBuffer[0] = 0;
    incPeriod_MACDBuffer[0] = 0;
    decPeriod_OsMABuffer[0] = 0;
    incPeriod_OsMABuffer[0] = 0;

    double tmp = 0;

    for (int i = prev_calculated; i < amount; i++)
    {
        OsMABuffer[i] = MACDBuffer[i] - SignalBuffer[i];
        colorBuffer[i] = 0;
        if (i > 0) {
            if (OsMABuffer[i] < OsMABuffer[i - 1]) {
                colorBuffer[i] = 1; 
            }
tmp = (OsMABuffer[i] - OsMABuffer[i-1]) * 10;
            updatePeriodBuffers(i, MACDBuffer, decPeriod_MACDBuffer, incPeriod_MACDBuffer);
incPeriod_MACDBuffer[i] = (OsMABuffer[i] - OsMABuffer[i-1]) * 10;
            updatePeriodBuffers(i, OsMABuffer, decPeriod_OsMABuffer, incPeriod_OsMABuffer);
        }
    }
    return (true);
}
//+------------------------------------------------------------------+
void updatePeriodBuffers(int i, double& buff[], double& decPerBuff[], double& incPerBuff[])
//+------------------------------------------------------------------+
{
    // if (i > 0 && buff[i] < buff[i-1])
    // {   decPerBuff[i] = decPerBuff[i-1] - 0.05; }
    // else
    // {  decPerBuff[i] = 0; }

    // if (i > 0 && buff[i] > buff[i-1])
    // {   incPerBuff[i] = incPerBuff[i-1] + 0.05; }
    // else
    // {  incPerBuff[i] = 0; }

    static double step = 0.02;
    static int  runLen = 5;

    if (i > 0) {
        if ((buff[i] > 0 && buff[i-1] <= 0)
        ||  (buff[i] < 0 && buff[i-1] >= 0)) {
                incPerBuff[i] = buff[i]/signal_period;
        }
        else {
            incPerBuff[i] = incPerBuff[i-1] + buff[i]/signal_period;
        }
        
        if( buff[i] < buff[i-1]) {                      // decreasing
            if (decPerBuff[i-1] <= 0)                   //      already in decreasing trend
            {   decPerBuff[i] = decPerBuff[i-1] - step; //          continue to decrease
            }
            else {
                decPerBuff[i] = 0;                      //          start new trend
                if (decPerBuff[i - runLen] < 0) {
                    for (int j = runLen; j > 0; j--) {
                        decPerBuff[i - j + 1] = decPerBuff[i - j] - step;
                    }
                }
            }
        }
        else {                                          // increasing
            if (decPerBuff[i-1] >= 0)                   //      already in increasing trend
            {   decPerBuff[i] = decPerBuff[i-1] + step; //          continue to increase
            }
            else {
                decPerBuff[i] = 0;                      //          start new trend
                if (decPerBuff[i - runLen] > 0) {
                    for (int j = runLen; j > 0; j--) {
                        decPerBuff[i - j + 1] = decPerBuff[i - j] + step;
                    }
                }
            }
        }
    }
}
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
//+------------------------------------------------------------------+
{
    if (iMACD_handle != INVALID_HANDLE)
        IndicatorRelease(iMACD_handle);
}
//+------------------------------------------------------------------+
