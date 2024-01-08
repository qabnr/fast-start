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
#property indicator_label2 "Buy TP"

#property indicator_type3  DRAW_COLOR_LINE
#property indicator_color3 clrRed, clrGreen, CLR_NONE
#property indicator_style3 STYLE_SOLID
#property indicator_width3 1
#property indicator_label3 "Sell TP"

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

void OnInit()
{
    SetIndexBuffer(0, stopBuffer, INDICATOR_DATA);
    SetIndexBuffer(1, stopColorBuffer,   INDICATOR_COLOR_INDEX);

    SetIndexBuffer(2, buyBuffer, INDICATOR_DATA);
    SetIndexBuffer(3, buyColorBuffer, INDICATOR_COLOR_INDEX);

    SetIndexBuffer(4, sellBuffer, INDICATOR_DATA);
    SetIndexBuffer(5, sellColorBuffer, INDICATOR_COLOR_INDEX);

    hATR = iATR(NULL, ATRtimeframe, ATRper);
    ArraySetAsSeries(atr, true);
    
    //Print("ATR TR ST per: ", ATRper, " Mult: ", Mult);
}

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
    static double minHiStop  = minResetValue;
    static double maxLowStop = maxResetValue;

    static int trend = 1;

    int i, day_n = 0, day_t = 0;

    CopyBuffer(hATR, 0, 0, rates_total - prev_calculated, atr);

    for (i = prev_calculated; i < rates_total; i++)
    {       
        double newLowStop = low[i] - atr[i] * Mult;
        double newHiStop = high[i] + atr[i] * Mult;
        bool is_newHiStop  = false;
        bool is_newLowStop = false;

        sellColorBuffer[i] = 0;
        buyColorBuffer[i]  = 1;

        if (newLowStop > maxLowStop)
        {
            sellBuffer[i] = newLowStop;
            is_newLowStop = true;
            if (trend > 0)
            {
                maxLowStop = newLowStop;
                stopColorBuffer[i] = 2;
            }
        }
        else
        {
            sellBuffer[i] = sellBuffer[i-1];

            int cnt = 0;
            for (int j = 0; j < 10 && j <= i; j++)
            {   if (sellBuffer[i-j] > low[i-j])
                {   cnt++; }
            }
            if (cnt == 10)
            {   maxLowStop = maxResetValue; }
        }

        if (newHiStop < minHiStop)
        {
            buyBuffer[i] = newHiStop;
            is_newHiStop = true;
            if (trend < 0)
            {   minHiStop = newHiStop;
                stopColorBuffer[i] = 2;
            }
        }
        else
        {
            buyBuffer[i] = buyBuffer[i - 1];

            int cnt = 0;
            for (int j = 0; j < 10 && j <= i; j++)
            {   if (buyBuffer[i-j] < high[i-j])
                {   cnt++; }
            }
            if (cnt == 10)
            {   minHiStop = minResetValue;  }
        }


        if (trend > 0)
        {
            trend++;
            stopColorBuffer[i] = trend > 2 ? 0 : 2;

            if (is_newLowStop)
            {   stopBuffer[i] = newLowStop;
            }
            else
            {   stopBuffer[i] = stopBuffer[i - 1];

                if (stopBuffer[i]   > low[i])
                if (stopBuffer[i-1] > low[i-1])
                {   trend = -1;
                }
            }
        }
        else  // downtrend
        {
            trend--;
            stopColorBuffer[i] = trend < -2 ? 1 : 2;

            if (is_newHiStop)
            {    stopBuffer[i] = newHiStop;
            }
            else
            {   stopBuffer[i] = stopBuffer[i - 1];

                if (stopBuffer[i]   < high[i])
                if (stopBuffer[i-1] < high[i-1])
                {   trend = 1;
                }
            }
        }
    }    
    return (rates_total);
}

void OnDeinit(const int reason)
{
    IndicatorRelease(hATR);
}
