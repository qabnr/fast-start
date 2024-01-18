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
    
    //Print("ATR TR ST per: ", ATRper, " Mult: ", Mult);
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
        const int holdingPeriod = 10;

        double atrI = (i > ATRper) ? atr[i - prev_calculated] : 0;
        
        double newSellStop = low[i]  - atrI * Mult;
        double newBuyStop  = high[i] + atrI * Mult;
        bool is_newBuyStop  = false;
        bool is_newSellStop = false;

        sellColorBuffer[i] = (maxSellStop == maxResetValue) ? 2 : 0;
        buyColorBuffer[i]  = (minBuyStop  == minResetValue) ? 2 : 1;

        if (newSellStop > maxSellStop)
        {
            sellBuffer[i] = newSellStop;
            is_newSellStop = true;
            maxSellStop = newSellStop;
        }
        else
        {
            if (i > 0) 
            { sellBuffer[i] = sellBuffer[i-1]; }
            else
            { sellBuffer[i] = 0; }

            int cnt = 0;
            for (int j = 0; j < holdingPeriod && j <= i; j++)
            {   if (sellBuffer[i-j] > low[i-j])
                {   cnt++; }
            }
            if (cnt == holdingPeriod)
            {   maxSellStop = maxResetValue; }
        }

        if (newBuyStop < minBuyStop)
        {
            buyBuffer[i] = newBuyStop;
            is_newBuyStop = true;
            minBuyStop = newBuyStop;
        }
        else
        {
            if (i > 0)
            { buyBuffer[i] = buyBuffer[i-1]; }
            else
            { buyBuffer[i] = 0; }

            int cnt = 0;
            for (int j = 0; j < holdingPeriod && j <= i; j++)
            {   if (buyBuffer[i-j] < high[i-j])
                {   cnt++; }
            }
            if (cnt == holdingPeriod)
            {   minBuyStop = minResetValue;  }
        }


        if (trend > 0)  // uptrend
        {
            trend++;
            stopColorBuffer[i] = trend > 2 ? 0 : 2;

            if (is_newSellStop)
            {   stopBuffer[i] = newSellStop;
            }
            else
            {   
                if (i > 0)
                {   stopBuffer[i] = stopBuffer[i-1];
                    if (stopBuffer[i]   > low[i])
                    if (stopBuffer[i-1] > low[i-1])
                    {   trend = -1;
                    }
                }
                else
                {   stopBuffer[i] = 0; }
            }
        }
        else  // downtrend
        {
            trend--;
            stopColorBuffer[i] = trend < -2 ? 1 : 2;

            if (is_newBuyStop)
            {    stopBuffer[i] = newBuyStop;
            }
            else
            {
                if (i > 0)
                {   stopBuffer[i] = stopBuffer[i-1];
                    if (stopBuffer[i]   < high[i])
                    if (stopBuffer[i-1] < high[i-1])
                    {   trend = 1;
                    }
                }
                else
                {   stopBuffer[i] = 0; }
            }
        }
    }    
    return (rates_total);
}

//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
    IndicatorRelease(hATR);
}
