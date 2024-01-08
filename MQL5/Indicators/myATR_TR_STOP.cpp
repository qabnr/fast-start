#property copyright "GM"
#property description "ATR Trailing Stop"

#property version "1.00"

#property indicator_chart_window

#property indicator_buffers 6
#property indicator_plots 3

#property indicator_type1  DRAW_COLOR_LINE
#property indicator_color1 clrRed, clrGreen, CLR_NONE
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
            sellBuffer[i] = newLowStop ;
            is_newLowStop = true;
        }
        else
        {
            sellBuffer[i] = sellBuffer[i-1];
            if (sellBuffer[i] > low[i])
            if (sellBuffer[i-1] > low[i-1])
            if (sellBuffer[i-2] > low[i-2])
            if (sellBuffer[i-3] > low[i-3])
            if (sellBuffer[i-4] > low[i-4])
            if (sellBuffer[i-5] > low[i-5])
            if (sellBuffer[i-6] > low[i-6])
            if (sellBuffer[i-7] > low[i-7])
            if (sellBuffer[i-8] > low[i-8])
            if (sellBuffer[i-9] > low[i-9])
            {
                maxLowStop = maxResetValue;
            }
        }

        if (newHiStop < minHiStop)
        {
            buyBuffer[i] = newHiStop ;
        }
        else
        {
            buyBuffer[i] = buyBuffer[i - 1];

            if (buyBuffer[i] < high[i])
            if (buyBuffer[i-1] < high[i-1])
            if (buyBuffer[i-2] < high[i-2])
            if (buyBuffer[i-3] < high[i-3])
            if (buyBuffer[i-4] < high[i-4])
            if (buyBuffer[i-5] < high[i-5])
            if (buyBuffer[i-6] < high[i-6])
            if (buyBuffer[i-7] < high[i-7])
            if (buyBuffer[i-8] < high[i-8])
            if (buyBuffer[i-9] < high[i-9])
            {
                minHiStop = minResetValue;
            }
        }


        if (trend > 0)
        {
            trend++;
            stopColorBuffer[i] = maxLowStop > maxResetValue ? 0 : 2;

            if (is_newLowStop)
            {
                stopBuffer[i] = newLowStop;
                maxLowStop = newLowStop;
            }
            else
            {
                stopBuffer[i] = stopBuffer[i - 1];

                if (stopBuffer[i] > low[i])
                if (stopBuffer[i-1] > low[i-1])
                {
                    trend = -1;
                }
            }
        }
        else  // downtrend
        {
            trend--;
            stopColorBuffer[i] = minHiStop < minResetValue ? 1 : 2;

            if (newHiStop < minHiStop)
            {
                stopBuffer[i] = newHiStop;
                minHiStop = newHiStop;
            }
            else
            {
                stopBuffer[i] = stopBuffer[i - 1];
                if (stopBuffer[i] < high[i])
                if (stopBuffer[i-1] < high[i-1])
                {
                    trend = 1;
                }
            }
        }

//Print("iATR: ", stopBuffer[i], " clr: ", stopColorBuffer[i] < 0.5 ? "Red" : stopColorBuffer[i] < 1.5 ? "Green" : "Black");
//PrintFormat("iATR: %.2f, %s", stopBuffer[i], stopColorBuffer[i] < 0.5 ? "Red" : stopColorBuffer[i] < 1.5 ? "Green" : "Black");
//Print(TimeCurrent());
    }    
    return (rates_total);
}

void OnDeinit(const int reason)
{
    IndicatorRelease(hATR);
}
