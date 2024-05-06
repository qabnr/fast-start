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

#property indicator_label1 "width"
#property indicator_type1 DRAW_LINE
#property indicator_color1 clrBlack
#property indicator_style1 STYLE_SOLID
#property indicator_width1 2

#property indicator_label2 "0"
#property indicator_type2 DRAW_LINE
#property indicator_color2 clrRed
#property indicator_style2 STYLE_SOLID
#property indicator_width2 2

input uint offset = 0;

//--- indicator buffers
double widthBuffer[];
double zeroBuffer[];

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

    T get0(void) const { return a_; }
    T get1(void) const { return b_; }
};
//+------------------------------------------------------------------+
int OnInit()
//+------------------------------------------------------------------+
{
    SetIndexBuffer(0, zeroBuffer, INDICATOR_DATA);
    SetIndexBuffer(1, widthBuffer, INDICATOR_DATA);
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
    static const int runLen = 500;

    if (rates_total < runLen) {
        ArraySetAsSeries(widthBuffer, true);   
        ArraySetAsSeries(zeroBuffer, true);   
        for (int i = 0; i < rates_total; i++)
        widthBuffer[i] = zeroBuffer[i] = 0;
        return 0;
    }

    int cnt = rates_total;
    if (cnt > runLen) cnt = runLen;

    ArraySetAsSeries(open, false);   
    ArraySetAsSeries(widthBuffer, false);   
    ArraySetAsSeries(zeroBuffer, false);   

    for (int i = 0; i < rates_total; i++) {
        widthBuffer[i] = 0;
        zeroBuffer[i] = 0;
    }

    int startIndex = rates_total - 1 - offset;
    for (int i = 3; i < runLen; i++) {
        int endIndex   = rates_total - i;
        int buffIdx = rates_total - i;
        const double spread = getSpread(startIndex, endIndex, open);

        widthBuffer[buffIdx] = spread;
        zeroBuffer [buffIdx] = 0;
    }

    return rates_total;
}
//+------------------------------------------------------------------+
double getSpread(const int startIndex, const int endIndex, const double& price[]) {
    double maxUpDiff   = 0;
    double maxDownDiff = 0;

    const pair<double> a_b = simpleLinRegr(startIndex, endIndex, price);
    const double a = a_b.get0();
    const double b = a_b.get1();

    for (int x = startIndex; x >= endIndex; x--) {
        const double y = a + b * x;
        double diff = price[x] - y;
        if (diff > 0 && diff > maxUpDiff) { maxUpDiff = diff; }
        else
        if (diff < 0 && -diff > maxDownDiff) { maxDownDiff = -diff; }
    }

    return maxUpDiff + maxDownDiff;
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
