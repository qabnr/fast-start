#property copyright "GM"
#property description "ATR Trailing Stop"

#property version "1.00"

#property indicator_chart_window

#property indicator_buffers 6
#property indicator_plots 3

#property indicator_type1  DRAW_COLOR_LINE
#property indicator_color1 clrRed, clrGreen, clrGray
#property indicator_style1 STYLE_SOLID
#property indicator_width1 2
#property indicator_label1 "ST"

#property indicator_type2  DRAW_COLOR_LINE
#property indicator_color2 clrRed, clrGreen, CLR_NONE
#property indicator_style2 STYLE_SOLID
#property indicator_width2 1
#property indicator_label2 "Buy ST"

#property indicator_type3  DRAW_COLOR_LINE
#property indicator_color3 clrRed, clrGreen, CLR_NONE
#property indicator_style3 STYLE_SOLID
#property indicator_width3 1
#property indicator_label3 "Sell ST"

input int    ATRper = 10;        // ATR Period
input double Mult   = 1;         // Multiplier

/* input */ENUM_TIMEFRAMES ATRtimeframe = PERIOD_CURRENT; // Timeframe

double stopBuffer[];
double stopColorBuffer[];

double buyBuffer[];
double buyColorBuffer[];

double sellBuffer[];
double sellColorBuffer[];

int hATR;
double atr[];

const double maxResetValue = 0.0;
const double minResetValue = DBL_MAX;

string short_name;
    
//+------------------------------------------------------------------+
void OnInit()
{
    SetIndexBuffer(0, stopBuffer,      INDICATOR_DATA);
    SetIndexBuffer(1, stopColorBuffer, INDICATOR_COLOR_INDEX);

    SetIndexBuffer(2, buyBuffer,       INDICATOR_DATA);
    SetIndexBuffer(3, buyColorBuffer,  INDICATOR_COLOR_INDEX);

    SetIndexBuffer(4, sellBuffer,      INDICATOR_DATA);
    SetIndexBuffer(5, sellColorBuffer, INDICATOR_COLOR_INDEX);

    hATR = iATR(NULL, ATRtimeframe, ATRper);

    short_name = StringFormat("ATR_TRST (%d, %.1f)", ATRper, Mult);
    IndicatorSetString(INDICATOR_SHORTNAME, short_name);
}

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

    CopyBuffer(hATR, 0, 0, rates_total - prev_calculated, atr);

    for (i = prev_calculated; i < rates_total; i++)
    {
        if (i == 0) {
            sellBuffer[i] = low[i];
            buyBuffer[i]  = high[i];
            stopBuffer[i] = low[i];

            sellColorBuffer[i] = 2;
            buyColorBuffer[i]  = 2;
            stopColorBuffer[i] = 2;

            continue;
        }
        const int holdingPeriod = int(Mult * 2);

        double atrI = (i > ATRper) ? atr[i - prev_calculated] : 0;
        
        double newSellStop = low[i-1]  - atrI * Mult;
        double newBuyStop  = high[i-1] + atrI * Mult;
        bool is_newBuyStop  = false;
        bool is_newSellStop = false;

        sellColorBuffer[i] = (maxSellStop == maxResetValue) ? 2 : 0;
        buyColorBuffer[i]  = (minBuyStop  == minResetValue) ? 2 : 1;

        if (newSellStop > maxSellStop) {
            is_newSellStop = true;
            sellBuffer[i] = newSellStop;
            maxSellStop = newSellStop;
        }
        else {
            sellBuffer[i] = sellBuffer[i-1];

            int cnt = 0;
            for (int j = 0; j < holdingPeriod && j <= i; j++) {
                if (sellBuffer[i-j] > low[i-j])
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
                if (buyBuffer[i-j] < high[i-j])
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
            }
            else {
                stopBuffer[i] = stopBuffer[i-1];
                if (stopBuffer[i] > low[i-1]) {
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
            }
            else {
                stopBuffer[i] = stopBuffer[i-1];
                if (stopBuffer[i] < high[i-1]) {
                    trend = 1;
                    stopBuffer[i] = sellBuffer[i];
                    stopColorBuffer[i] = 2;
                }
            }
        }
//PrintFormat("%s: %s H: %.2f L: %.2f ST: %.2f", short_name,trend > 0 ? "up" : "down", high[i], low[i], stopBuffer[i]);    

//if (TimeCurrent() > D'2024.01.15')
//if (TimeCurrent() < D'2024.01.18')
//PrintFormat("%s: %s: %s H: %.2f L: %.2f ST: %.2f", TimeToString(time[i]),
//short_name,trend > 0 ? " up " : "down", high[i-1], low[i-1], stopBuffer[i]);    
    }
    
    
    return (rates_total);
}

//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
    IndicatorRelease(hATR);
}
