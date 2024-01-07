#property copyright "GM"
#property description "ATR Trailing Stop"

#property version "1.00"

#property indicator_chart_window
#property indicator_buffers 2
#property indicator_plots 1
#property indicator_type1 DRAW_COLOR_LINE
#property indicator_color1 clrRed, clrGreen, clrBlack
#property indicator_style1 STYLE_SOLID
#property indicator_width1 2
#property indicator_label1 "Buy TP"

input int    ATRper = 10;        // ATR Period
input double Mult   = 1;         // Multiplier

/* input */ENUM_TIMEFRAMES ATRtimeframe = PERIOD_CURRENT; // Timeframe

double atrTrStBuffer[];
double colorBuffer[];

int hATR;
double atr[];

const double maxResetValue = 0.0;
const double minResetValue = DBL_MAX;

void OnInit()
{
    SetIndexBuffer(0, atrTrStBuffer,    INDICATOR_DATA);
    SetIndexBuffer(1, colorBuffer, INDICATOR_COLOR_INDEX);
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

    static bool upTrend = true;

    int i, day_n = 0, day_t = 0;

    CopyBuffer(hATR, 0, 0, 2, atr);

    for (i = prev_calculated; i < rates_total; i++)
    {       
        if (upTrend)
        {
            colorBuffer[i] = maxLowStop > maxResetValue ? 0 : 2;

            double newLowStop = low[i] - atr[0] * Mult;
            if (newLowStop > maxLowStop)
            {
                atrTrStBuffer[i] = newLowStop;
                maxLowStop = newLowStop;
            }
            else
            {
                atrTrStBuffer[i] = atrTrStBuffer[i - 1];
            }

            if (atrTrStBuffer[i] > low[i])
            if (atrTrStBuffer[i-1] > low[i-1])
            {
                maxLowStop = maxResetValue;
                upTrend = false;
            }
        }
        else  // downtrend
        {
            colorBuffer[i] = minHiStop < minResetValue ? 1 : 2;

            double newLowStop = high[i] + atr[0] * Mult;
            if (newLowStop < minHiStop)
            {
                atrTrStBuffer[i] = newLowStop;
                minHiStop = newLowStop;
            }
            else
            {
                atrTrStBuffer[i] = atrTrStBuffer[i - 1];
            }
            if (atrTrStBuffer[i] < high[i])
            if (atrTrStBuffer[i-1] < high[i-1])
            {
                minHiStop = minResetValue;
                upTrend = true;
            }
        }
//Print("iATR: ", atrTrStBuffer[i], " clr: ", colorBuffer[i] < 0.5 ? "Red" : colorBuffer[i] < 1.5 ? "Green" : "Black");
//PrintFormat("iATR: %.2f, %s", atrTrStBuffer[i], colorBuffer[i] < 0.5 ? "Red" : colorBuffer[i] < 1.5 ? "Green" : "Black");
//Print(TimeCurrent());
    }    
    return (rates_total);
}

void OnDeinit(const int reason)
{
    IndicatorRelease(hATR);
}
