//+------------------------------------------------------------------+
#property copyright "Copyright 2024, Mogyo Software Corp."
#property link      "http://www.mogyo.com"
#property version   "1.0"

#property description "Linear regression channel"
#property description "1."
#property description "2."

#property indicator_separate_window
#property indicator_buffers 2
#property indicator_plots   2

#property indicator_label1 "b"
#property indicator_type1 DRAW_LINE
#property indicator_color1 clrBlack
#property indicator_style1 STYLE_SOLID
#property indicator_width1 1

#property indicator_label2 "a"
#property indicator_type2 DRAW_LINE
#property indicator_color2 clrRed
#property indicator_style2 STYLE_SOLID
#property indicator_width2 1


//--- indicator buffers
double aBuffer[];
double bBuffer[];

//+------------------------------------------------------------------+
template <typename T>
class pair
{
private:
    T a_, b_;
public:
    pair(const T a, const T b) : a_(a), b_(b) {}
    pair(const pair &p2): a_(p2.a_), b_(p2.b_) {}
    ~pair() {}

    T get0(void) { return a_; }
    T get1(void) { return b_; }
};
//+------------------------------------------------------------------+
int OnInit()
//+------------------------------------------------------------------+
{
    SetIndexBuffer(0, bBuffer, INDICATOR_DATA);
    SetIndexBuffer(1, aBuffer, INDICATOR_DATA);
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
    static const int runLen = 100;

    if (rates_total < runLen) {
        ArraySetAsSeries(aBuffer, true);   
        ArraySetAsSeries(bBuffer, true);   
        for (int i = 0; i < rates_total; i++)
        aBuffer[i] = bBuffer[i] = 0;
        return 0;
    }

    int cnt = rates_total;
    if (cnt > runLen) cnt = runLen;

    ArraySetAsSeries(open, false);   
    ArraySetAsSeries(aBuffer, false);   
    ArraySetAsSeries(bBuffer, false);   

    for (int i = 0; i < rates_total; i++) {
        aBuffer[i] = 0;
        bBuffer[i] = 0;
    }

    int startIndex = rates_total-1;
    for (int i = 10; i < runLen; i++) {
        int endIndex   = rates_total - i;
        int buffIdx = rates_total - i;
        pair<double> val = simpleLinRegr(startIndex, endIndex, open);
        double a = val.get0();
        double b = val.get1();
// Print("[", buffIdx, "] = ", a);
        aBuffer[buffIdx] = a;
        bBuffer[buffIdx] = -5000*b;
    }

    return rates_total;
}
//+------------------------------------------------------------------+
pair<double> simpleLinRegr(const int startIdx, const int endIdx, const double& y[])
//+------------------------------------------------------------------+
{
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

    return pair<double>(a, b);
}
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
//+------------------------------------------------------------------+
{
}
//+------------------------------------------------------------------+
