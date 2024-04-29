//+------------------------------------------------------------------+
//| fast-start-example.mq5
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

    for (int i = 0; i < cnt; i++) {
        widthBuffer[i] = simpleLinRegr(i+10, open).get1();
    }

    return rates_total;
}
//+------------------------------------------------------------------+
pair simpleLinRegr(const int n, const double& y[])
//+------------------------------------------------------------------+
{
    double sumX  = 0;
    double sumX2 = 0;
    double sumY  = 0;
    double sumY2 = 0;
    double sumXY = 0;

    for (int x = 0; x < n; x++) {
        sumX  += x;
        sumX2 += x * x;
        sumY  += y[x];
        sumY2 += y[x] * y[x];
        sumXY += x * y[x];
    }

    double b = (sumXY - sumX * sumY / n) / (sumX2 - sumX * sumX / n);
    double a = sumY/n - b * sumX / n;
Print("n = ", n, "  b = ", b);
    return pair(a, b);
}
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
//+------------------------------------------------------------------+
{
}
//+------------------------------------------------------------------+
