/*
 #property copyright "Denis Zyatkevich"
#property description "This indicator calculates TakeProfit levels"
#property description "using the average market volatility. It uses the values"
#property description "of Average True Range (ATR) indicator, calculated"
#property description "on daily price data. Indicator values are calculated"
#property description "using maximal and minimal price values per day."
#property version   "1.00"
#property indicator_chart_window
#property indicator_buffers 2
#property indicator_plots   2
#property indicator_type1   DRAW_LINE
#property indicator_color1  C'127,191,127'
#property indicator_style1  STYLE_SOLID
#property indicator_label1  "Buy TP"
#property indicator_type2   DRAW_LINE
#property indicator_color2  C'191,127,127'
#property indicator_style2  STYLE_SOLID
#property indicator_label2  "Sell TP"
 */
/* input */ int ATR_period = 5;                        // ATR Period
/* input */ ENUM_TIMEFRAMES ATR_timeframe = PERIOD_D1; // Indicator timeframe

double buBuffer[], bd[];
int hATR;

void OnInit()
{
    SetIndexBuffer(0, buBuffer, INDICATOR_DATA);
    SetIndexBuffer(1, bd, INDICATOR_DATA);
    hATR = iATR(NULL, ATR_timeframe, ATR_period);
}

int OnCalculate(const int rates_total,
                const int prev_calculated,
                const datetime &time[],
                const double &open[],
                const double &high[],
                const double &low[],
                const double &close[],
                const long   &tick_volume[],
                const long   &volume[],
                const int    &spread[])
{
    int i, day_n = 0, day_time = 0;
    double atr[], day_hi = 0, day_lo = 999999.0;

    CopyBuffer(hATR, 0, 0, 2, atr);
    ArraySetAsSeries(atr, true);

    for (i = prev_calculated; i < rates_total; i++)
    {   day_time = (int)time[i] / PeriodSeconds(ATR_timeframe);
        if (day_n  < day_time)
        {   day_n  = day_time;
            day_lo = low[i];
            day_hi = high[i];
        }
        else
        {   if (high[i] > day_hi)   day_hi = high[i];
            if (low[i]  < day_lo)   day_lo = low[i];
        }
        buBuffer[i] = day_lo + atr[1];
        bd[i] = day_hi - atr[1];
    }
    return (rates_total);
}

void OnDeinit(const int reason)
{
    IndicatorRelease(hATR);
}
