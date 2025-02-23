//+------------------------------------------------------------------+
#property copyright "Copyright 2024, Mogyo Software Corp."
#property link      "http://www.mogyo.com"
#property version   "1.0"

#property description "Linear regression channel"
#property description "1."
#property description "2."

#property indicator_separate_window
#property indicator_buffers 4
#property indicator_plots   4

#property indicator_label1 "0"
#property indicator_type1 DRAW_LINE
#property indicator_color1 clrBlack
#property indicator_style1 STYLE_SOLID
#property indicator_width1 1

#property indicator_label2 "width"
#property indicator_type2 DRAW_LINE
#property indicator_color2 clrRed
#property indicator_style2 STYLE_SOLID
#property indicator_width2 2

#property indicator_label3 "a"
#property indicator_type3 DRAW_LINE
#property indicator_color3 clrGreen
#property indicator_style3 STYLE_SOLID
#property indicator_width3 1

#property indicator_label4 "b"
#property indicator_type4 DRAW_LINE
#property indicator_color4 clrBlue
#property indicator_style4 STYLE_SOLID
#property indicator_width4 1

input int offset = 0;

//--- indicator buffers
double widthBuffer[];
double zeroBuffer[];
double aBuffer[];
double bBuffer[];

//+------------------------------------------------------------------+
template <typename T> class pair
{
private:
    T m0_, m1_;
public:
    pair(const T a, const T b) : m0_(a), m1_(b) {}
    pair(const pair &p2): m0_(p2.m0_), m1_(p2.m1_) {}
    ~pair() {}

    T get0(void) const { return m0_; }
    T get1(void) const { return m1_; }
};
//+------------------------------------------------------------------+
int OnInit()
{
    SetIndexBuffer(0, zeroBuffer,  INDICATOR_DATA);
    SetIndexBuffer(1, widthBuffer, INDICATOR_DATA);
    SetIndexBuffer(2, aBuffer,     INDICATOR_DATA);
    SetIndexBuffer(3, bBuffer,     INDICATOR_DATA);
    
    string short_name = StringFormat("lrCgh %d", 10);
    IndicatorSetString(INDICATOR_SHORTNAME, short_name);
    return (INIT_SUCCEEDED);
}
//+------------------------------------------------------------------+
int OnCalculate(const int       rates_total,
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

    if (rates_total < 3000) return 0;

    int cnt = rates_total;
    if (cnt > runLen) cnt = runLen;

    ArraySetAsSeries(open,        false);   
    ArraySetAsSeries(widthBuffer, false);   
    ArraySetAsSeries(zeroBuffer,  false);   
    ArraySetAsSeries(aBuffer,     false);   
    ArraySetAsSeries(bBuffer,     false);   

    for (int i = 0; i < rates_total; i++) {
        widthBuffer[i] = 0;
        zeroBuffer[i] = 0;
        aBuffer[i] = 0;
        bBuffer[i] = 0;
    }

    int startIndex = rates_total - 1 - offset;
    for (int i = 3; i < runLen; i++) {
        int endIndex   = rates_total - i;
        widthBuffer[endIndex] = getSpread(startIndex, endIndex, open);
    }

    return rates_total;
}
//+------------------------------------------------------------------+
double getSpread(const int startIndex, const int endIndex, const double& price[])
{
    double maxUpDiff   = 0;
    double maxDownDiff = 0;

    const pair<double> a_b = simpleLinRegr(startIndex, endIndex, price);
    const double a = a_b.get0();
    const double b = a_b.get1();

    aBuffer[endIndex] = a / 1000;
    bBuffer[endIndex] = b * 20;

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
{
}
//+------------------------------------------------------------------+
