//+------------------------------------------------------------------+
//| myMinMax.mq5
//+------------------------------------------------------------------+
#property copyright "Copyright 2024, Mogyo Software Corp."
#property link      "http://www.mogyo.com"
#property version   "1.0"

#property description "For how long the present value is local min or max"

#property indicator_separate_window
#property indicator_buffers 3
#property indicator_plots   3

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

#property indicator_label3 "diff"
#property indicator_type3 DRAW_SECTION
#property indicator_color3 clrBlue
#property indicator_style3 STYLE_SOLID
#property indicator_width3 1

double minBuffer[];
double maxBuffer[];
double diffBuffer[];

#define SF StringFormat


//+------------------------------------------------------------------+
int OnInit()
//+------------------------------------------------------------------+
{
    Print("mMm: OnInit");

    SetIndexBuffer(0,  minBuffer, INDICATOR_DATA);
    SetIndexBuffer(1,  maxBuffer, INDICATOR_DATA);
    SetIndexBuffer(2, diffBuffer, INDICATOR_DATA);

    string short_name = "MaxMin";
    IndicatorSetString(INDICATOR_SHORTNAME, short_name);

    //PlotIndexSetDouble(0, PLOT_EMPTY_VALUE, 0.0);
    //PlotIndexSetDouble(1, PLOT_EMPTY_VALUE, 0.0);
    //PlotIndexSetDouble(2, PLOT_EMPTY_VALUE, 0.0);

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
        const bool isMin = false;
        findLen(minBuffer,  low, i, isMin);
        findLen(maxBuffer, high, i, !isMin );

        diffBuffer[i] = 0;
        if (minBuffer[i] > 0) {
            for (int j = i-1; j >= 0; j--) {
                if (maxBuffer[j] > 0) {
                    diffBuffer[i] = (high[j] - low[i]) / (high[j] + low[i]) * 2 * 100;
                    break;
                }

                if (low[i] < low[j]) {
                    minBuffer[j]  = 0;
                    diffBuffer[j] = 0;
                }
            }
        }
        if (maxBuffer[i] > 0) {
            for (int j = i-1; j >= 0; j--) {
                if (minBuffer[j] > 0) {
                    diffBuffer[i] = (high[i] - low[j]) / (high[i] + low[j]) * 2 * 100;
                    break;
                }

                if (high[i] > high[j]) {
                    maxBuffer[j] = 0;
                    diffBuffer[j] = 0;
                }
            }
        }
    }

    //--- return the prev_calculated value for the next call
    return (rates_total);
}
//+------------------------------------------------------------------+
void findDiff(double &buff[], const double &price[], const int i, const bool isMax)
{
    int j, j0 = i-1;
    buff[i] = 0;
    for (j = j0; j > 0; j--) {
        if ((isMax && price[j] > price[i]) || (!isMax && price[j] < price[i])) {
            buff[i] = f(j0-j);
            break;
        }
    }
    if (j == 0) buff[i] = f(i);
}
//+------------------------------------------------------------------+
void findLen(double &buff[], const double &price[], const int i, const bool isMax)
{
    int j, j0 = i-1;
    buff[i] = 0;
    for (j = j0; j > 0; j--) {
        if ((isMax && price[j] > price[i]) || (!isMax && price[j] < price[i])) {
            buff[i] = f(j0-j);
            break;
        }
    }
    if (j == 0) buff[i] = f(i);
}
//+------------------------------------------------------------------+
double f(const double x) 
{
    if (x < 5) return 0;
    return MathLog(x+2);
}
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
//+------------------------------------------------------------------+
{
    Print("mMm: OnDeinit");
}
//+------------------------------------------------------------------+
