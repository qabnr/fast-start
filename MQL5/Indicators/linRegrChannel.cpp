//+------------------------------------------------------------------+
#property copyright "Copyright 2024, Mogyo Software Corp."
#property link      "http://www.mogyo.com"
#property version   "1.0"

#property description "Linear regression channel"
#property description "1."
#property description "2."

#property indicator_separate_window
#property indicator_buffers 1
#property indicator_plots 1

#property indicator_label1 "Width"
#property indicator_type1 DRAW_LINE
#property indicator_color1 clrBlack
#property indicator_style1 STYLE_SOLID
#property indicator_width1 1


//--- input parameters
/* input */ ENUM_APPLIED_PRICE applied_price = PRICE_TYPICAL; // type of price
/* input */ ENUM_TIMEFRAMES period = PERIOD_CURRENT;          // timeframe

//--- indicator buffers
double widthBuffer[];

//+------------------------------------------------------------------+
class pair
{
private:
    double a_, b_;
public:
    pair(double a, double b) : a_(a), b_(b) {};
    pair(const pair &p2) { a_ = p2.a_; b_ = p2.b_; }
    ~pair() {};

    double get0(void) { return a_; }
    double get1(void) { return b_; }
};
//+------------------------------------------------------------------+
int OnInit()
//+------------------------------------------------------------------+
{
    SetIndexBuffer(0, widthBuffer, INDICATOR_DATA);
    string short_name = StringFormat("lrCgh %d", 10);
    IndicatorSetString(INDICATOR_SHORTNAME, short_name);
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
    if (rates_total < 100) return 0;

    int cnt = rates_total;
    if (cnt > 100) cnt = 100;

    ArraySetAsSeries(open, false);   
    ArraySetAsSeries(widthBuffer, false);   

    int startIndex = rates_total-1;
    for (int i = 10, disp = 1; i < 100; i++, disp++) {
        int endIndex   = rates_total - i;
        int buffIdx = rates_total - disp;
        double val = simpleLinRegr(startIndex, endIndex, open).get1();
Print("[", buffIdx, "] = ", val);
        widthBuffer[buffIdx] = val;
    }

    return rates_total;
}
//+------------------------------------------------------------------+
pair simpleLinRegr(const int startIdx, const int endIdx, const double& y[])
//+------------------------------------------------------------------+
{
// Print(startIdx, " --> ", endIdx);
    static double sumX  = 0;
    static double sumX2 = 0;
    static double sumY  = 0;
    static double sumY2 = 0;
    static double sumXY = 0;
    static int n = 0;
    static int lastStartIdx = 0;
    static int maxX = 0;

    if (startIdx != lastStartIdx) {
        sumX  = 0;
        sumX2 = 0;
        sumY  = 0;
        sumY2 = 0;
        sumXY = 0;
        n     = 0;
        maxX  = startIdx;
        lastStartIdx = startIdx;
    }

// Print(maxX, " --> ", endIdx);
    for (int x = maxX; x >= endIdx; x--) {
        n++;
        sumX  += x;
        sumX2 += x * x;
        sumY  += y[x];
        sumY2 += y[x] * y[x];
        sumXY += x * y[x];
    }
    maxX = endIdx-1;

    double b = (sumXY - sumX * sumY / n) / (sumX2 - sumX * sumX / n);
    double a = sumY/n - b * sumX / n;
// Print("n = ", n, "  b = ", b);
    return pair(a, b);
}
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
//+------------------------------------------------------------------+
{
}
//+------------------------------------------------------------------+
