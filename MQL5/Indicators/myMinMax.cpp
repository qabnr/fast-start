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
#property indicator_type1 DRAW_SECTION
#property indicator_color1 clrOrangeRed
#property indicator_style1 STYLE_SOLID
#property indicator_width1 2

#property indicator_label2 "maxLen"
#property indicator_type2 DRAW_SECTION
#property indicator_color2 clrBlack
#property indicator_style2 STYLE_SOLID
#property indicator_width2 2

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

    PlotIndexSetDouble(0, PLOT_EMPTY_VALUE, 0.0);
    PlotIndexSetDouble(1, PLOT_EMPTY_VALUE, 0.0);

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
    for (int i = prev_calculated; i < rates_total; i++)
    {
        int j;
        minBuffer[i] = 0;

        for (j = i-1; j > 0; j--) {
            if (low[j] < low[i]) {
                minBuffer[i] = f(i-1-j);
                break;
            }
        }
        if (j == 0) minBuffer[i] = f(i);

        maxBuffer[i] = 0;
        for (j = i-1; j > 0; j--) {
            if (high[j] > high[i]) {
                maxBuffer[i] = f(i-1-j);
                break;
            }
        }
        if (j == 0) maxBuffer[i] = f(i);
    }

    //--- return the prev_calculated value for the next call
    return (rates_total);
}
//+------------------------------------------------------------------+
double f(const double x) 
{
/*/
    if (x < 5) return 0;
/*/
    if (x < 5) return MathLog(2.0);
/**/
    return MathLog(x+2);
}
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
//+------------------------------------------------------------------+
{
    Print("mMm: OnDeinit");
}
//+------------------------------------------------------------------+
