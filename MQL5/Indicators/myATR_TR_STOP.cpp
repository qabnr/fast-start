#property copyright "Denis Zyatkevich"
#property description "This indicator calculates TakeProfit levels"
#property description "using the average market volatility. It uses the values"
#property description "of Average True Range (ATR) indicator, calculated"
#property description "on daily price data. Indicator values are calculated"
#property description "using maximal and minimal price values per day."
#property version "1.00"
#property indicator_chart_window
#property indicator_buffers 2
#property indicator_plots 1
#property indicator_type1 DRAW_COLOR_LINE
#property indicator_color1 clrRed, clrGreen, clrBlack
#property indicator_style1 STYLE_SOLID
#property indicator_width1 2
#property indicator_label1 "Buy TP"

input int    ATRper = 100;                           // ATR Period
input double Mult   = 1;                             // Multiplier

/* input */ENUM_TIMEFRAMES ATRtimeframe = PERIOD_CURRENT; // Indicator timeframe

double buBuffer[];
double colorBuffer[];

int hATR;

void OnInit()
{
    SetIndexBuffer(0, buBuffer,    INDICATOR_DATA);
    SetIndexBuffer(1, colorBuffer, INDICATOR_COLOR_INDEX);
    hATR = iATR(NULL, ATRtimeframe, ATRper);
    
    Print("iATR per: ", ATRper, " Mult: ", Mult);
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
    static double buMin = DBL_MAX;
    static double bdMax = 0.0;

    static bool upTrend = true;

    int i, day_n = 0, day_t = 0;
    double atr[];

    CopyBuffer(hATR, 0, 0, 2, atr);
    ArraySetAsSeries(atr, true);

    for (i = prev_calculated; i < rates_total; i++)
    {       
        if (upTrend)
        {
            colorBuffer[i] = bdMax > 0 ? 0 : 2;

            double newBD = low[i] - atr[0] * Mult;
            if (newBD > bdMax)
            {
                buBuffer[i] = newBD;
                bdMax = newBD;
            }
            else
            {
                buBuffer[i] = buBuffer[i - 1];
            }

            if (buBuffer[i] > low[i])
            if (buBuffer[i-1] > low[i-1])
            {
                bdMax = 0;
                upTrend = false;
            }
        }
        else
        {
            colorBuffer[i] = buMin < DBL_MAX ? 1 : 2;

            double newBU = high[i] + atr[0] * Mult;
            if (newBU < buMin)
            {
                buBuffer[i] = newBU;
                buMin = newBU;
            }
            else
            {
                buBuffer[i] = buBuffer[i - 1];
            }
            if (buBuffer[i] < high[i])
            if (buBuffer[i-1] < high[i-1])
            {
                buMin = DBL_MAX;
                upTrend = true;
            }
        }
//Print("iATR: ", buBuffer[i], " clr: ", colorBuffer[i] < 0.5 ? "Red" : colorBuffer[i] < 1.5 ? "Green" : "Black");
//PrintFormat("iATR: %.2f, %s", buBuffer[i], colorBuffer[i] < 0.5 ? "Red" : colorBuffer[i] < 1.5 ? "Green" : "Black");
//Print(TimeCurrent());
    }    
    return (rates_total);
}

void OnDeinit(const int reason)
{
    IndicatorRelease(hATR);
}
