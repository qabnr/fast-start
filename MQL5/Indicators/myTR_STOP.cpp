#property copyright "GM"
#property description "Trailing Stop"

#property version "1.00"

#property indicator_chart_window

#property indicator_buffers 6
#property indicator_plots 3

#property indicator_type1  DRAW_COLOR_LINE
#property indicator_color1 clrSpringGreen, clrNONE
#property indicator_style1 STYLE_SOLID
#property indicator_width1 1
#property indicator_label1 "Buy ST"

#property indicator_type2  DRAW_COLOR_LINE
#property indicator_color2 clrOrange, clrNONE
#property indicator_style2 STYLE_SOLID
#property indicator_width2 1
#property indicator_label2 "Sell ST"

#property indicator_type3  DRAW_COLOR_LINE
#property indicator_color3 clrRed, clrGreen, clrGray
#property indicator_style3 STYLE_SOLID
#property indicator_width3 3
#property indicator_label3 "ST"

#include <adapt.h>

input int     lookBackPeriod_I = 20;  // Look Back Period
input double  priceOffset      = 0.20;// Price Offset

int lookBackPeriod   = lookBackPeriod_I * PeriodSeconds(PERIOD_H1) / PeriodSeconds(PERIOD_CURRENT);

//+------------------------------------------------------------------+
#define SF StringFormat
#define LOG(s) Print(SF("%d: %s", __LINE__, s))
//+------------------------------------------------------------------+
double stopBuffer[];
double stopColorBuffer[];

double buyBuffer[];
double buyColorBuffer[];

double sellBuffer[];
double sellColorBuffer[];

const double maxResetValue = 0.0;
const double minResetValue = DBL_MAX;

string short_name;
//+------------------------------------------------------------------+
void OnInit()
{
    SetIndexBuffer(0, buyBuffer,       INDICATOR_DATA);
    SetIndexBuffer(1, buyColorBuffer,  INDICATOR_COLOR_INDEX);

    SetIndexBuffer(2, sellBuffer,      INDICATOR_DATA);
    SetIndexBuffer(3, sellColorBuffer, INDICATOR_COLOR_INDEX);

    SetIndexBuffer(4, stopBuffer,      INDICATOR_DATA);
    SetIndexBuffer(5, stopColorBuffer, INDICATOR_COLOR_INDEX);

    short_name = StringFormat("TRST (%d, %.1f)", lookBackPeriod, priceOffset);
    IndicatorSetString(INDICATOR_SHORTNAME, short_name);
}
//+------------------------------------------------------------------+
#define LOW_PRICE(i)  (low[i] + close[i]) / 2 
#define HIGH_PRICE(i) (high[i] + close[i]) / 2  
//+------------------------------------------------------------------+
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
    static double minBuyStop  = minResetValue;
    static double maxSellStop = maxResetValue;

    static int trend = 1;

    int i, day_n = 0, day_t = 0;

    for (i = prev_calculated; i < rates_total; i++)
    {
        if (i < lookBackPeriod) {
            sellBuffer[i] = LOW_PRICE(i);
            buyBuffer [i] = HIGH_PRICE(i);
            stopBuffer[i] = LOW_PRICE(i);

            sellColorBuffer[i] = 2;
            buyColorBuffer [i] = 1;
            stopColorBuffer[i] = 1;

            continue;
        }
        const int holdingPeriod = 6;

        double highMax  = 0.0;
        double maxHdiff = 0.0;

        double lowMin   = DBL_MAX;
        double maxLdiff = 0.0;

        for (int back = MathMin(i, lookBackPeriod); back > 0; back--) {
            maxHdiff = MathMax(highMax - LOW_PRICE(i-back), maxHdiff);
            highMax  = MathMax(highMax, HIGH_PRICE(i-back));

            maxLdiff = MathMax(HIGH_PRICE(i-back) - lowMin, maxLdiff);
            lowMin   = MathMin(lowMin, LOW_PRICE(i-back));
        }
        double newSellStop = highMax - maxHdiff - priceOffset;
        double newBuyStop  = lowMin  + maxLdiff + priceOffset;
        bool is_newBuyStop  = false;
        bool is_newSellStop = false;

        sellColorBuffer[i] = (maxSellStop == maxResetValue) ? 1 : 0;
        buyColorBuffer[i]  = (minBuyStop  == minResetValue) ? 1 : 0;

        if (newSellStop > maxSellStop) {
            is_newSellStop = true;
            sellBuffer[i] = newSellStop;
            maxSellStop = newSellStop;
        }
        else {
            sellBuffer[i] = sellBuffer[i-1];

            int cnt = 0;
            for (int j = 0; j < holdingPeriod && j <= i; j++) {
                if (sellBuffer[i-j] > LOW_PRICE(i-j))
                {   cnt++; }
            }
            if (cnt == holdingPeriod) {
                maxSellStop = maxResetValue;
            }
        }

        if (newBuyStop < minBuyStop) {
            is_newBuyStop = true;
            buyBuffer[i] = newBuyStop;
            minBuyStop = newBuyStop;
        }
        else {
            buyBuffer[i] = buyBuffer[i-1];

            int cnt = 0;
            for (int j = 0; j < holdingPeriod && j <= i; j++) {
                if (buyBuffer[i-j] < HIGH_PRICE(i-j))
                {   cnt++; }
            }
            if (cnt == holdingPeriod)  {
                minBuyStop = minResetValue;
            }
        }


        if (trend > 0)  // uptrend
        {
            trend++;
            stopColorBuffer[i] = 0;

            if (is_newSellStop) {
                stopBuffer[i] = newSellStop;
                // LOG(SF("new Sell Stop: %.2f", newSellStop));
            }
            else {
                stopBuffer[i] = stopBuffer[i-1];
                if (stopBuffer[i] > LOW_PRICE(i-1)) {
                    trend = -1;
                    stopBuffer[i] = buyBuffer[i];
                    stopColorBuffer[i] = 2;
                }
            }
        }
        else  // downtrend
        {
            trend--;
            stopColorBuffer[i] = 1;

            if (is_newBuyStop) {
                stopBuffer[i] = newBuyStop;
                // LOG(SF("new Buy Stop: %.2f", newBuyStop));
            }
            else {
                stopBuffer[i] = stopBuffer[i-1];
                if (stopBuffer[i] < HIGH_PRICE(i-1)) {
                    trend = 1;
                    stopBuffer[i] = sellBuffer[i];
                    stopColorBuffer[i] = 2;
                }
            }
        }
        //PrintFormat("%s: %s H: %.2f L: %.2f ST: %.2f", short_name,trend > 0 ? "up" : "down", HIGH_PRICE(i), LOW_PRICE(i), stopBuffer[i]);
        //if (TimeCurrent() > D'2024.01.15')
        //if (TimeCurrent() < D'2024.01.18')
        //PrintFormat("%s: %s: %s H: %.2f L: %.2f ST: %.2f", TimeToString(time[i]),
        //short_name,trend > 0 ? " up " : "down", HIGH_PRICE(i-1), LOW_PRICE(i-1), stopBuffer[i]);
    }
    return (rates_total);
}

//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
}
