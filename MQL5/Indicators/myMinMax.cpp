//+------------------------------------------------------------------+
//| myMinMax.mq5
//+------------------------------------------------------------------+
#property copyright "Copyright 2024, Mogyo Software Corp."
#property link      "http://www.mogyo.com"
#property version   "1.0"

#property description "For how long the present value is local min or max"

#property indicator_separate_window
#property indicator_buffers 2
#property indicator_plots 2

#property indicator_label1 "minLen"
#property indicator_type1 DRAW_LINE
#property indicator_color1 clrBlack
#property indicator_style1 STYLE_SOLID
#property indicator_width1 2

#property indicator_label2 "maxLen"
#property indicator_type2 DRAW_LINE
#property indicator_color2 clrOrangeRed
#property indicator_style2 STYLE_SOLID
#property indicator_width2 1

double minBuffer[];
double maxBuffer[];


#define SF StringFormat


//+------------------------------------------------------------------+
int OnInit()
//+------------------------------------------------------------------+
{
    Print("mMm: OnInit");

    SetIndexBuffer(0, minBuffer, INDICATOR_DATA);
    SetIndexBuffer(1, maxBuffer, INDICATOR_DATA);
    
    string short_name = "MaxMin";
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
Print("mMm: rates_total: ", rates_total, "  prev_calculated: ", prev_calculated);

if (prev_calculated == 0) {
    string s = "";
    for (int j = 0; j < rates_total; j++) {
        s += SF("%7.2f", low[j]);
        if (j%20 == 19) {
            Print(s);
            s = "";
        }
    }
    if (s != "") Print(s);
}

    for (int i = prev_calculated; i < rates_total; i++)
    {
        int j;
        minBuffer[i] = 0;

if(i == 8) PrintFormat("mMm: low[%2d]: %4.2f", i, low[i]);
        for (j = i-1; j > 0; j--) {
if(i == 8) PrintFormat("mMm: low[%2d]: %4.2f", j, low[j]);
            if (low[j] < low[i]) {
                minBuffer[i] = i-1-j;
                break;
            }
        }
        if (j == 0) minBuffer[i] = i;
if(i < 40) PrintFormat("mMm: min[%4d]: %4.0f", i, minBuffer[i]);
// if(i < 4) PrintFormat("mMm: min[%4d]: %4.0f  %5.2f  %5.2f  %5.2f  %5.2f", i, minBuffer[i], low[i], low[i-1], low[i-2], low[i-3], low[i-4]);

        maxBuffer[i] = 0;
        for (j = i-1; j > 0; j--) {
            if (high[j] > high[i]) {
                maxBuffer[i] = i-1-j;
                break;
            }
        }
        if (j == 0) maxBuffer[i] = i;
    }

    //--- return the prev_calculated value for the next call
    return (rates_total);
}
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
//+------------------------------------------------------------------+
{
    Print("mMm: OnDeinit");
}
//+------------------------------------------------------------------+
