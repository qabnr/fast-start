//+------------------------------------------------------------------+
//| fast-start-example.mq5
//+------------------------------------------------------------------+
#property copyright "Copyright 2024, Mogyo Software Corp."
#property link      "http://www.mogyo.com"
#property version   "3.4"

#include <Trade\Trade.mqh>
#include <Trade\PositionInfo.mqh>

input int    MACD1_fast_MA_period     = 12;
input int    MACD1_slow_MA_period     = 26;
input int    MACD1_avg_diff_period    = 9;
input int    MACD2_fast_MA_period     = 96;
input int    MACD2_slow_MA_period     = 208;
input int    MACD2_avg_diff_period    = 72;
input double OsMA_limit               = 0.28;
input double decP_OsMa_limit          = 2.13;
input int    minMaxBAckTrack          = 5;
input double profitPerBalanceLimit    = 0.48;
input double profitLossPerBalLimit    = 0.46;
input int    maxTransactions          = 1000;
input double equityTradeLimit         = 1.00;
input double tradeSizeFraction        = 1.00;
input int    LastChangeOfSignMinLimit = 98810;
input int    LastChangeOfSignMaxLimit = 250950;
input double profitPerPriceLimit      = 1.73;
input double cummPrLossPerPriceLimit  = 29.26;
input double profitLossPerPriceLimit  = 7.98;
input double maxRelDrawDownLimit      = 0.70;

//+------------------------------------------------------------------+
void printInputParams() {
    PrintFormat("input int    MACD1_fast_MA_period     = %d;", MACD1_fast_MA_period);
    PrintFormat("input int    MACD1_slow_MA_period     = %d;", MACD1_slow_MA_period);
    PrintFormat("input int    MACD1_avg_diff_period    = %d;", MACD1_avg_diff_period);
    PrintFormat("input int    MACD2_fast_MA_period     = %d;", MACD2_fast_MA_period);
    PrintFormat("input int    MACD2_slow_MA_period     = %d;", MACD2_slow_MA_period);
    PrintFormat("input int    MACD2_avg_diff_period    = %d;", MACD2_avg_diff_period);
    PrintFormat("input double OsMA_limit               = %.2f;", OsMA_limit);
    PrintFormat("input double decP_OsMa_limit          = %.2f;", decP_OsMa_limit);
    PrintFormat("input int    minMaxBAckTrack          = %d;", minMaxBAckTrack);
    PrintFormat("input double profitPerBalanceLimit    = %.2f;", profitPerBalanceLimit);
    PrintFormat("input double profitLossPerBalLimit    = %.2f;", profitLossPerBalLimit);
    PrintFormat("input int    maxTransactions          = %d;", maxTransactions);
    PrintFormat("input double equityTradeLimit         = %.2f;", equityTradeLimit);
    PrintFormat("input double tradeSizeFraction        = %.2f;", tradeSizeFraction);
    PrintFormat("input int    LastChangeOfSignMinLimit = %d;", LastChangeOfSignMinLimit);
    PrintFormat("input int    LastChangeOfSignMaxLimit = %d;", LastChangeOfSignMaxLimit);
    PrintFormat("input double profitPerPriceLimit      = %.2f;", profitPerPriceLimit);
    PrintFormat("input double cummPrLossPerPriceLimit  = %.2f;", cummPrLossPerPriceLimit);
    PrintFormat("input double profitLossPerPriceLimit  = %.2f;", profitLossPerPriceLimit);
    PrintFormat("input double maxRelDrawDownLimit      = %.2f;", maxRelDrawDownLimit);
}
//+------------------------------------------------------------------+
#define SF StringFormat
//+------------------------------------------------------------------+
bool isLOG() {
// if (TimeCurrent() > D'2022.06.13')
// if (TimeCurrent() < D'2022.12.01')
        return true;
    return false;
}
void LOG_Naked(const string s) {
    if (isLOG()) Print(s);
}
#define LOG(s) if (isLOG()) Print(SF("%d: %s", __LINE__, s))
#define LOGF(s) if (isLOG()) Print(SF("%d: %s: %s", __LINE__, __FUNCTION__, s))
//+------------------------------------------------------------------+
#define DAYS *24*60*60
#define HOURS *60*60
//+------------------------------------------------------------------+
string d2str(const double d, bool human = true) {
    const string ThSep = ",";

    if (d < 0) return "-" + d2str(-d, human);
    ulong i = (ulong)MathFloor(d);

    if (i < 1000) {
        return (string)i;
    }
    ulong thousands = i / 1000;
    int u = int(i - thousands * 1000);
    if (thousands < 1000) {
        return (string)thousands + ThSep + SF("%03u", u);
    }
    uint millions = uint(thousands / 1000);
    thousands -= millions * 1000;
    if (millions < 1000) {
        if (human) return (string)millions + ThSep + SF("%02d", thousands/10) + "m";
        return (string)millions + ThSep + SF("%03d", thousands) + ThSep + SF("%03u", u);
    }
    uint trillions = millions / 1000;
    millions -= trillions * 1000;
    if (human) return (string)trillions + ThSep + SF("%02d", millions/10) + "tr";
    return (string)trillions + ThSep + SF("%03u", millions) + ThSep + SF("%03u", thousands) + ThSep + SF("%03u", u);
}
//+------------------------------------------------------------------+
bool isNewMinute() {
    static datetime oldTime = 0;

    datetime now = TimeCurrent();

    datetime nowMinutes = now / 60;
    datetime oldMinutes = oldTime / 60;

    oldTime = now;

    return nowMinutes != oldMinutes;
}
//+------------------------------------------------------------------+
string secToStr(int totalSeconds) {
    int minutes = totalSeconds / 60;
    int hours   = minutes / 60;
    int days    = hours / 24;
    int seconds = totalSeconds - minutes*60;
    minutes -= hours * 60;
    hours   -= days * 24;

    if (days > 0 && hours > 0) return SF("%2dd %dh", days, hours);
    if (days > 0) return SF("%2dd", days);

    if (hours > 0 && seconds > 0) return SF("%2dh %dm %ds", hours, minutes, seconds);
    if (hours > 0 && minutes > 0) return SF("%2dh %dm", hours, minutes);
    if (hours > 0) return SF("%2dh", hours);
    if (minutes > 0 && seconds > 0) return SF("%2dm %ds", minutes, seconds);
    if (minutes > 0) return SF("%2dm", minutes);
    return SF("%2ds", seconds);
}
//+------------------------------------------------------------------+
int timeDiff(datetime &then) {
    return int(TimeCurrent() - then);
}
//+------------------------------------------------------------------+
string timeDiffToStr(datetime &then) {
    return secToStr(timeDiff(then));
}
//+------------------------------------------------------------------+
class Buffer
{
private:
    string  indicatorName;
    int     handle;
    int     buffNum;
    double  buff[];
    int     nrCopied;

    void init() {
        ArraySetAsSeries(buff, true);
        LOG(SF("New buffer: %d, handle: %d, indicator: %s", buffNum, handle, indicatorName));
        if (handle == INVALID_HANDLE) {
            Print("Failed to get handle for buffer: ", buffNum, ", indicator: ", indicatorName);
        }
    }

public:
    Buffer(const int _buffNumber, const Buffer &otherBuffer)
        : indicatorName(otherBuffer.indicatorName), handle(otherBuffer.handle), buffNum(_buffNumber), nrCopied(0)
    {
        init();
    }
    Buffer(const int _buffNumber, const string _indicatorName, const int _handle)
        : indicatorName(_indicatorName), handle(_handle), buffNum(_buffNumber), nrCopied(0)
    {
        init();
    }
    ~Buffer() { LOGF(SF("buffer: %d, handle: %d, indicator: %s", buffNum, handle, indicatorName)); };

    int getHandle() const {
        return handle;
    }

    string getName() const {
        return indicatorName;
    }

    bool copy(const int count) {
        nrCopied = CopyBuffer(handle, buffNum, 0, count, buff);
        if (nrCopied <= 0)
        {   Print("Failed to copy data from buffer: ", buffNum, " handle: ", indicatorName, " (", handle, ")");
        }
        return nrCopied > 0;
    }
    double get(const int index) const {
        return buff[index];
    }
    int getNrCopied() const {
        return nrCopied;
    }
};
//+------------------------------------------------------------------+
class Indicator {
protected:
    int    handle;
public:
    Buffer buffer;

    Indicator(int _buffNumber, string _indicatorName, const int _handle)
        : handle(_handle),
          buffer(_buffNumber, _indicatorName, _handle)
    {}
    virtual ~Indicator() { LOGF(""); };

    string getName() const {
        return buffer.getName();
    }

    virtual bool copyBuffers(const int count) {
        return buffer.copy(count);
    }
};
//+------------------------------------------------------------------+
class IndicatorList {
private:
    Indicator* list[];

public:
    IndicatorList() {}
    ~IndicatorList() {
        for (int i = 0; i < ArraySize(list); i++) {
            delete list[i];
        }
    }

    void add(Indicator* indicator) {
        LOG(SF("IndLst: adding %s", indicator.getName()));
        ArrayResize(list, ArraySize(list) + 1, 100);
        list[ArraySize(list)-1] = indicator;
    }

    bool copyBuffers(const int count) {
        int len = ArraySize(list);
        for (int i = 0; i < len; i++) {
            if (!list[i].copyBuffers(count)) {
                return false;
            }
        }
        return true;
    } 
};
//+------------------------------------------------------------------+
class ATR_TR_STOP : public Indicator
{
private:
    Buffer *stopBuffer;
    Buffer stopColorBuffer;

    Buffer buyBuffer;
    Buffer buyColorBuffer;

    Buffer sellBuffer;
    Buffer sellColorBuffer;

    string indName;

public:
    ~ATR_TR_STOP() { LOGF(indName); }
  
    ATR_TR_STOP(const int ATRperiod, const double mult)
        : indName(SF("ATR_TR_ST(%d, %.1f)", ATRperiod, mult)),
          Indicator(0, SF("ATR_TR_ST(%d, %.1f)", ATRperiod, mult),
                    iCustom(NULL, PERIOD_CURRENT, "myATR_TR_STOP", ATRperiod, mult)),
          stopBuffer     (  &buffer),
          stopColorBuffer(1, buffer),
          buyBuffer      (2, buffer),
          buyColorBuffer (3, buffer),
          sellBuffer     (4, buffer),
          sellColorBuffer(5, buffer)
    {}

    bool copyBuffers(const int count) {
        return
           stopBuffer.copy(count) && stopColorBuffer.copy(count)
        && buyBuffer .copy(count) && buyColorBuffer .copy(count)
        && sellBuffer.copy(count) && sellColorBuffer.copy(count);
    }

    bool isBuyNow(const double price) const {
        return buyBuffer.get(1) < price;
    }

    bool isSellNow(const double price) const {
        return sellBuffer.get(1) > price;
    }
};
//+------------------------------------------------------------------+
class ATR_TR_STOP_List
{
private:
    ATR_TR_STOP* atrTrStList[];

public:
    ATR_TR_STOP_List() { ArrayResize(atrTrStList, 0, 100); }
    ~ATR_TR_STOP_List() { LOGF(""); }

    void add(int ATRper, double mult) {
        ArrayResize(atrTrStList, ArraySize(atrTrStList) + 1, 100);
        ATR_TR_STOP *new_stop = new ATR_TR_STOP(ATRper, mult);
        atrTrStList[ArraySize(atrTrStList)-1] = new_stop;
        g::indicatorList.add(new_stop);
    }

    bool isBuyNow(const double price) const {
        for (int i = 0; i < ArraySize(atrTrStList); i++)
        {   if (atrTrStList[i].isBuyNow(price)) return true;
        }
        return false;
    }
    bool isSellNow(const double price) const {
        for (int i = 0; i < ArraySize(atrTrStList); i++)
        {   if (atrTrStList[i].isSellNow(price)) return true;
        }
        return false;
    }
};
//+------------------------------------------------------------------+
class MACD_base : public Indicator
{
protected:
    Buffer signal_Buffer;
    Buffer osMA_Buffer;
    Buffer osMA_Color_Buffer;

public:
    MACD_base(const string customIndicatorName, 
              const string bufferName,
              const int fast_MA_period,
              const int slow_MA_period,
              const int avg_diff_period,
              const int MACD_Buffer_buffNum,
              const int Signal_Buffer_buffNum,
              const int OsMA_Buffer_buffNum,
              const int osMA_Color_Buffer_buffNum)
      : Indicator        (MACD_Buffer_buffNum,       bufferName,
                          iCustom(NULL, PERIOD_CURRENT, customIndicatorName, 
                                  fast_MA_period, slow_MA_period, avg_diff_period)),
        signal_Buffer    (Signal_Buffer_buffNum,     bufferName, handle),
        osMA_Buffer      (OsMA_Buffer_buffNum,       bufferName, handle),
        osMA_Color_Buffer(osMA_Color_Buffer_buffNum, bufferName, handle)
    {
        if (handle == INVALID_HANDLE)
        {
            Print("Failed to get indicator handle");
        }
    }

    virtual ~MACD_base() { LOGF(""); };

    virtual bool copyBuffers(const int count)
    {
        return
            buffer           .copy(count) &&
            signal_Buffer    .copy(count) &&
            osMA_Buffer      .copy(count) &&
            osMA_Color_Buffer.copy(count);
    }

    bool justChangedToDownTrend() const {
        return osMA_Buffer.get(0) < osMA_Buffer.get(1)
            && osMA_Buffer.get(1) > osMA_Buffer.get(2);
    }

    bool justChangedToUpTrend() const {
        return osMA_Buffer.get(0) > osMA_Buffer.get(1)
            && osMA_Buffer.get(1) < osMA_Buffer.get(2);
    }

    bool OsMA_justChangedPositive() const {
        return osMA_Buffer.get(0) >  0
            && osMA_Buffer.get(1) <= 0
            && buffer.get(0) < 0;
    }

    bool OsMA_justChangedNegative() const {
        return osMA_Buffer.get(0) <  0
            && osMA_Buffer.get(1) >= 0
            && buffer.get(0) > 0;
    }
};
//+------------------------------------------------------------------+
class MACD : public MACD_base
{
protected:
    Buffer decPeriod_Buffer;
    Buffer incPeriod_Buffer;
    Buffer decPeriod_OsMA_Buffer;
    Buffer incPeriod_OsMA_Buffer;

public:
    MACD(const string bufferName, const int fast_MA_period, const int slow_MA_period, const int avg_diff_period)
        : MACD_base("myMACD", bufferName, fast_MA_period, slow_MA_period, avg_diff_period,
        5, 4 , 2, 3),
        decPeriod_Buffer     (0, bufferName, handle),
        incPeriod_Buffer     (1, bufferName, handle),
        decPeriod_OsMA_Buffer(6, bufferName, handle),
        incPeriod_OsMA_Buffer(7, bufferName, handle)
    { }
    ~MACD() { LOGF(""); }

    bool copyBuffers(const int count)
    {
        return
            MACD_base::copyBuffers    (count) &&
            decPeriod_Buffer     .copy(count) &&
            incPeriod_Buffer     .copy(count) &&
            decPeriod_OsMA_Buffer.copy(count) &&
            incPeriod_OsMA_Buffer.copy(count);
    }
};
//+------------------------------------------------------------------+
class myMACD2 : public MACD_base
{
public:
    myMACD2(const string bufferName, const int fast_MA_period, const int slow_MA_period, const int avg_diff_period)
        : MACD_base("myMACD2", bufferName, fast_MA_period, slow_MA_period, avg_diff_period,
                    0, 1, 2, 3)
    {}

    ~myMACD2() { LOGF(""); }
};
//+------------------------------------------------------------------+
class LinRegrChannel : public Indicator
{
public:
    LinRegrChannel(string bufferName)
        : Indicator(0, bufferName, iCustom(NULL, PERIOD_CURRENT, "linRegrChannel"))
    {}
    ~LinRegrChannel() { LOGF(""); }
};
//+------------------------------------------------------------------+
class ZigZag : public Indicator
{
public:
    Buffer *ZigZagBuffer;
    Buffer HighMapBuffer;
    Buffer LowMapBuffer;

    ZigZag(const string bufferName)
        : Indicator(0, bufferName, iCustom(NULL, PERIOD_CURRENT, "myZigZag", 12, 5, 3)),
          ZigZagBuffer (  &buffer),
          HighMapBuffer(1, buffer),
          LowMapBuffer (2, buffer)
    {}
    ~ZigZag() { LOGF(""); }

    bool copyBuffers(const int count) {
        return
            ZigZagBuffer .copy(count) &&
            HighMapBuffer.copy(count) &&
            LowMapBuffer .copy(count);
    }
};
//+------------------------------------------------------------------+
#define enum2str_CASE(c) case  c: return #c
#define enum2str_DEFAULT default: return "<UNKNOWN>"
#define DEF_IS_METHOD(state_)  bool is##state_  () { return state == state_; }

namespace Reason
{
    enum ReasonCode {
        Bought,
        Sold,
        peakNr1,
        peakNr2,
        valleyNr1,
        valleyNr2,
        decOSMA_gt_limit,
        decOSMA_lt_limit,
        ATR_high,
        ATR_low,
        chDir_profitPerBalanceLimit,
        chDir_profitLossPerBalLimit,
        chDir_profitPerPriceLimit,
        chDir_profitLossPerPriceLimit,
        chDir_cummPrLossPerPriceLimit,
        changeOfSign_neg,
        changeOfSign_pos,
        OsMA_pos,
        OsMA_neg,
        size
    };

    string toStr(int r) {
        switch (r) {
            case Bought:            return "Bought";
            case Sold:              return "Sold";
            case peakNr1:           return "1st peak";
            case peakNr2:           return "2nd peak";
            case valleyNr1:         return "1st valley";
            case valleyNr2:         return "2nd valley";
            case decOSMA_gt_limit:  return "decOsMA > limit";
            case decOSMA_lt_limit:  return "decOsMA < -limit";
            case ATR_high:          return "ATR high";
            case ATR_low:           return "ATR low";
            case chDir_profitPerBalanceLimit:    return SF("chDir: profitPerBalanceLimit (-%.0f%%)",   profitPerBalanceLimit  *100);
            case chDir_profitLossPerBalLimit:    return SF("chDir: profitLossPerBalLimit (-%.0f%%)",   profitLossPerBalLimit  *100);
            case chDir_profitPerPriceLimit:      return SF("chDir: profitPerPriceLimit (-%.0f%%)",     profitPerPriceLimit    *100);
            case chDir_profitLossPerPriceLimit:  return SF("chDir: profitLossPerPriceLimit (-%.0f%%)", profitLossPerPriceLimit*100);
            case chDir_cummPrLossPerPriceLimit:  return SF("chDir: cummPrLossPerPriceLimit (-%.0f%%)", cummPrLossPerPriceLimit*100);
            case changeOfSign_neg:  return "Change of sign: (-)";
            case changeOfSign_pos:  return "Change of sign: (+)";
            case OsMA_pos:          return "OsMA (+)";
            case OsMA_neg:          return "OsMA (-)";

            enum2str_DEFAULT;
        }
    }
};
//+------------------------------------------------------------------+
class List {
private:
    double arr_[];
    int    last_;
    double sum_;

public:
    List(): last_(-1), sum_(0) { ArrayResize(arr_, 0, 1000); }
    ~List() {}

    void push(double val) {
        sum_ += val;
        last_++;
        ArrayResize(arr_, last_+1, 1000);
        arr_[last_] = val;
    }

    double sum() const { return sum_; }
    double avg() const { return last_ <= 0 ? 0: sum_/last_; }

    double variance() const {
        int size = ArraySize(arr_);
        if (size == 0) return 0;

        double sum = 0;
        for (int i = 0; i < size; i++) { sum += arr_[i]; }
        double avg = sum / size;
        double diffSqSum = 0;
        for (int i = 0; i < size; i++) {
            double diff = avg - arr_[i];
            diffSqSum += diff * diff;
        }
        return diffSqSum / size;
    }

    double stdDev() const { return MathSqrt(variance()); }
};
//+------------------------------------------------------------------+
class Stats {
public:
    enum Operation {
        buy,
        sell,
        close,
        size
    };

private:
    int  cntOp[Operation::size][Reason::ReasonCode::size];
    List profitList[Reason::ReasonCode::size];

public:
    string op2str(int op) const {
        switch (op)
        {
            case Operation::buy:     return "Buy";
            case Operation::sell:    return "Sell";
            case Operation::close:   return "Clse";
            default:                 return "<UNKNOWN>";
        }
    }

    void addOpReason(Operation op, Reason::ReasonCode r) {
        cntOp[op, r]++;
    }
  
    void addProfit(double profit, Reason::ReasonCode reason) {
        profitList[reason].push(profit);
    }
  
    void print() {
        string line = SF("%41s", "");
        for (int op = 0; op < Operation::size; op++) {
            line += SF("%7s", op2str(op));
        }
        LOG("");
        line += "   Profit   avgPrf   stdDev";
        LOG(line);
        string lineDiv;
        StringInit(lineDiv, StringLen(line), '-');
        LOG(lineDiv);
        double sumSumProfit = 0;
        for (int r = 0; r < Reason::ReasonCode::size; r++) {
            line = SF("%40s", Reason::toStr(r));
            for (int op = 0; op < Operation::size; op++) {
                line += SF("%7d", cntOp[op][r]);
            }
            LOG(SF("%s %8s %8.1f %8.1f",
                line,
                d2str(profitList[r].sum(), false),
                profitList[r].avg(),
                profitList[r].stdDev()));
            sumSumProfit += profitList[r].sum();
        }
        LOG(lineDiv);
        LOG(SF("                                                              %8s", d2str(sumSumProfit, false)));
    }
};
//+------------------------------------------------------------------+
class SellOrBuy {
public:
    enum State {
        None,
        GetReadyToBuy,
        BuyNow,
        GetReadyToSell,
        SellNow
    };

private:
    State  state;
    Reason::ReasonCode reason;

public:

    SellOrBuy(): state(None) {}
    ~SellOrBuy() {}

    Reason::ReasonCode getReason() { return reason; }

    void set(State state_, Reason::ReasonCode reason_, int lineNo = 0) {
        if (state != state_) {
            state  = state_;
            reason = reason_;
            LOG_Naked(SF("%d: ==> %s <== %s", lineNo, State2str(), Reason2str()));
        }
    }

    DEF_IS_METHOD(None)
    DEF_IS_METHOD(GetReadyToBuy)
    DEF_IS_METHOD(BuyNow)
    DEF_IS_METHOD(GetReadyToSell)
    DEF_IS_METHOD(SellNow)

    string Reason2str() const { return Reason::toStr(reason); }

    string State2str() const { return State2str(state); }

    string State2str(State s) const {
        switch (s) {
            enum2str_CASE(None);
            enum2str_CASE(GetReadyToBuy);
            enum2str_CASE(BuyNow);
            enum2str_CASE(GetReadyToSell);
            enum2str_CASE(SellNow);
            enum2str_DEFAULT;
        }
    }
};
//+------------------------------------------------------------------+
class TradePosition {
private:
    const string        my_symbol;
    CTrade        m_Trade;
    CPositionInfo m_Position;

    int           posType;
    double        volume;
    double        totalPricePaid;
    Stats         stats;

public:
    TradePosition(): my_symbol(Symbol()),
        volume(MathFloor(SymbolInfoDouble(my_symbol, SYMBOL_VOLUME_MAX) / tradeSizeFraction)),
        posType(-1), totalPricePaid(0.01)
    {}
    ~TradePosition() { stats.print(); }

    bool select()                    { return m_Position.Select(my_symbol); }
    bool isTypeSELL()          const { return posType == POSITION_TYPE_SELL; }
    bool isTypeBUY()           const { return posType == POSITION_TYPE_BUY; }
    double getTotalPricePaid() const { return totalPricePaid; }

    void close(Reason::ReasonCode reason, double profit) {
LOG(SF("Close, profit: %+.1f%%", (profit)));

        stats.addOpReason(stats.close,  reason);
        stats.addProfit(profit, reason);

        uint cnt = 0;
        while(m_Trade.PositionClose(my_symbol)) {
            cnt++;
// LOG(SF("Close: Ticket: %u  Price = %.2f", m_Trade.ResultDeal(), m_Trade.ResultPrice()));
            if (cnt == 1) {
                LOG(SF("Close: %s (%d)", m_Trade.ResultRetcodeDescription(), m_Trade.ResultRetcode()));
            }
        }
        if (cnt == 0) {
            LOG(SF("Close: %s (%d)", m_Trade.ResultRetcodeDescription(), m_Trade.ResultRetcode()));
        }
    }

    bool buy(Reason::ReasonCode reason) {
        stats.addOpReason(stats.buy, reason);

        double freeMarginBeforeTrade = AccountInfoDouble(ACCOUNT_FREEMARGIN);

        double executionPrice   = 0.0;
        double stopLoss         = 0.0;
        double takeProfit       = 0.0;

        for (int i = maxTransactions; i > 0; i--)
        {
            bool res = m_Trade.Buy(volume, my_symbol, executionPrice, stopLoss, takeProfit);
            if (!res) {
                LOG(m_Trade.ResultRetcodeDescription());
                break;
            }
// LOG(SF("Buy:   Ticket: %u  Price = %.2f", m_Trade.ResultDeal(), m_Trade.ResultPrice()));
            posType = POSITION_TYPE_BUY;
            if (AccountInfoDouble(ACCOUNT_FREEMARGIN) < freeMarginBeforeTrade * equityTradeLimit) {
                break;
            }
        }
        LOG(SF("Buy: %s (%d)", m_Trade.ResultRetcodeDescription(), m_Trade.ResultRetcode()));
        if (m_Trade.ResultRetcode() != TRADE_RETCODE_MARKET_CLOSED) {
            totalPricePaid = freeMarginBeforeTrade - AccountInfoDouble(ACCOUNT_FREEMARGIN);
            LOG(SF("BUY for %s at %.2f each", d2str(totalPricePaid), m_Trade.ResultPrice()));
            return true;
        }
        return false;
    }

    bool sell(Reason::ReasonCode reason) {
        stats.addOpReason(stats.sell, reason);

        double freeMarginBeforeTrade = AccountInfoDouble(ACCOUNT_FREEMARGIN);

        double executionPrice   = 0.0;
        double stopLoss         = 0.0;
        double takeProfit       = 0.0;

        for (int i = maxTransactions; i > 0; i--) {
            bool res = m_Trade.Sell(volume, my_symbol, executionPrice, stopLoss, takeProfit);
            if (!res) {
                LOG(m_Trade.ResultRetcodeDescription());
                break;
            }
// LOG(SF("Sell:  Ticket: %u  Price = %.2f", m_Trade.ResultDeal(), m_Trade.ResultPrice()));          
            posType = POSITION_TYPE_SELL;
            if (AccountInfoDouble(ACCOUNT_FREEMARGIN) < freeMarginBeforeTrade * equityTradeLimit) {
                break;
            }
        }
        LOG(SF("Sell: %s (%d)", m_Trade.ResultRetcodeDescription(), m_Trade.ResultRetcode()));
        if (m_Trade.ResultRetcode() != TRADE_RETCODE_MARKET_CLOSED) {
            totalPricePaid = freeMarginBeforeTrade - AccountInfoDouble(ACCOUNT_FREEMARGIN);
            LOG(SF("SELL for %s at %.2f each", d2str(totalPricePaid), m_Trade.ResultPrice()));
            return true;
        }
        return false;
}
};
//+------------------------------------------------------------------+
namespace g
{
    ATR_TR_STOP_List ATR_list;
    myMACD2         *MACD1;
    myMACD2         *MACD2;
    ZigZag          *zigZag;
    TradePosition    pPos;
    LinRegrChannel  *linRegrChannel;
    SellOrBuy        sellOrBuy;
    double           maxRelDrawDown = 0;

    IndicatorList   indicatorList;
};
//+------------------------------------------------------------------+
int OnInit()
{
    printInputParams();
    
    g::indicatorList.add(g::MACD1  = new myMACD2("MACD1", MACD1_fast_MA_period, MACD1_slow_MA_period, MACD1_avg_diff_period));
    g::indicatorList.add(g::MACD2  = new myMACD2("MACD2", MACD2_fast_MA_period, MACD2_slow_MA_period, MACD2_avg_diff_period));
    g::indicatorList.add(g::zigZag = new ZigZag ("ZZ"));

    // g::linRegrChannel = new LinRegrChannel("LRCh");

/*/
    g::ATR_list.add(10, 1.0);
    g::ATR_list.add(10, 2.0);
    g::ATR_list.add(10, 3.0);
/*/
    g::ATR_list.add(10, 4.0);
 //   g::ATR_list.add(10, 6.0);

    return (0);
}
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
}
//+------------------------------------------------------------------+
void changeDirection(const Reason::ReasonCode reason, const int lineNo) {
    if (g::pPos.isTypeBUY()) {
        g::sellOrBuy.set(SellOrBuy::State::SellNow, reason, lineNo);
    }
    else if (g::pPos.isTypeSELL()) {
        g::sellOrBuy.set(SellOrBuy::State::BuyNow, reason, lineNo);
    }
}
//+------------------------------------------------------------------+
class MACD_PeaksAndValleys
{
private:
    myMACD2  *macd;
    int      sign;
    datetime TimeOfLastChangeOfSign;
    int      numOfmins, numOfMaxs;
    double   lastMin,   lastMax;
    datetime TimeOfLastMin, TimeOfLastMax;
    bool     is1stPeakAlreadyFound,   is2ndPeakAlreadyFound;
    bool     is1stValleyAlreadyFound, is2ndValleyAlreadyFound;

    bool isMin() const {
//LOG(SF("(%d) %.3f  %.3f", 1, macd.buffer.get(1), macd.buffer.get(2)));
        if (macd.buffer.get(1) < macd.buffer.get(2)) return false;
        for (int i = 2; i < minMaxBAckTrack; i++) {
//LOG(SF("(%d) %.3f  %.3f", i, macd.buffer.get(i), macd.buffer.get(i+1)));
            if (macd.buffer.get(i) > macd.buffer.get(i+1)) return false;
        }
// LOG("MIN found");
        return true;
    };

    bool isMax() const {
//LOG(SF("(%d) %.3f  %.3f", 1, macd.buffer.get(1), macd.buffer.get(2)));
        if (macd.buffer.get(1) > macd.buffer.get(2)) return false;
        for (int i = 2; i < minMaxBAckTrack; i++) {
//LOG(SF("(%d) %.3f  %.3f", i, macd.buffer.get(i), macd.buffer.get(i+1)));
            if (macd.buffer.get(i) < macd.buffer.get(i+1)) return false;
        }
// LOG("MAX found");
        return true;
    };

    void initValues(int _sign) {

        if (sign != _sign) {
// LOG(SF("(%s) Last change of sign: %s ago XXXXXXXXXXXXXXXXX", _sign > 0 ? "+" : _sign < 0 ? "-" : "0", timeDiffToStr(TimeOfLastChangeOfSign)));
            TimeOfLastChangeOfSign = TimeCurrent();
        }
        sign = _sign;
        numOfmins = numOfMaxs = 0;
        lastMin = 0.00001;
        lastMax = 999999;
        is1stPeakAlreadyFound = false;
        is2ndPeakAlreadyFound = false;
        is1stValleyAlreadyFound = false;
        is2ndValleyAlreadyFound = false;
    };

public:
    MACD_PeaksAndValleys(myMACD2 *macd_) :
        macd(macd_),
        sign(-2),
        TimeOfLastChangeOfSign(TimeCurrent()),
        TimeOfLastMin(0),
        TimeOfLastMax(0)
    {   initValues(0); }
    ~MACD_PeaksAndValleys() {};

    void LogMACD_Last(const int cnt) const {
        if (!isLOG()) return;
        string s;
        for (int i = 0; i < cnt; i++) {
            s += SF("%.2f ", macd.buffer.get(i));
        }
        Print(s);
    }

    void process() {

//LogMACD_Last(minMaxBAckTrack+1);

static datetime barTime = 0;
datetime currBarTime = iTime(_Symbol, _Period, 0);
if(barTime != currBarTime) {
    barTime = currBarTime;
}
else {
    return;
}

        double macd0 = macd.buffer.get(1);
        if (macd0 < 0) {
            if (sign >= 0) {
if (timeDiff(TimeOfLastChangeOfSign) > LastChangeOfSignMinLimit)
if (timeDiff(TimeOfLastChangeOfSign) < LastChangeOfSignMaxLimit)
    g::sellOrBuy.set(SellOrBuy::State::SellNow, Reason::changeOfSign_neg, __LINE__);
                initValues(-1);
            }
            if (isMin()) {
// LOG(SF("MIN,  time since last: %s", timeDiffToStr(TimeOfLastMin)));
                if (macd0  * (1 - 0.05) < lastMax) {
//                    if (numOfmins == numOfMaxs)
if (timeDiff(TimeOfLastMin) > 25 HOURS)
                        numOfmins++;
                    lastMin = macd0;
                    TimeOfLastMin = TimeCurrent();
// string LOGtxt = SF("min: %.2f, nOf: %d", macd0, numOfmins);
// if (numOfmins == 1) LOGtxt += SF(", Last change of sign: %s ago XXXXXXXXXXXXXXXXX", timeDiffToStr(TimeOfLastChangeOfSign));
// LOG(LOGtxt);
                }
            }
            else if (isMax()) {
// LOG(SF("MAX,  time since last: %s", timeDiffToStr(TimeOfLastMax)));
                if ((macd0 / lastMin) < 1 - 0.05) {
//                    if (numOfmins > numOfMaxs)
                        numOfMaxs++;
                    lastMax = macd0;
                    TimeOfLastMax = TimeCurrent();
// string LOGtxt = (SF("Max: %.2f, nOf: %d", macd0, numOfMaxs));
// if (numOfMaxs == 1) LOGtxt += (SF(", Last change of sign: %s ago XXXXXXXXXXXXXXXXX", timeDiffToStr(TimeOfLastChangeOfSign)));
// LOG(LOGtxt);
                }
            }
        }
        else {
            if (sign <= 0) {
if (timeDiff(TimeOfLastChangeOfSign) > LastChangeOfSignMinLimit)
if (timeDiff(TimeOfLastChangeOfSign) < LastChangeOfSignMaxLimit)
    g::sellOrBuy.set(SellOrBuy::State::BuyNow, Reason::changeOfSign_pos, __LINE__);
                initValues(1);
            }
            if (isMax()) {
// LOG(SF("MAX,  time since last: %s", timeDiffToStr(TimeOfLastMax)));
//                if (numOfmins == numOfMaxs)
                    numOfMaxs++;
                lastMax = macd0;
                TimeOfLastMax = TimeCurrent();
// string LOGtxt = (SF("Max: %.2f, nOf: %d", macd0, numOfMaxs));
// if (numOfMaxs == 1) LOGtxt += (SF(", Last change of sign: %s ago XXXXXXXXXXXXXXXXX", timeDiffToStr(TimeOfLastChangeOfSign)));
// LOG(LOGtxt);
            }
            else if (isMin()){
// LOG(SF("MIN,  time since last: %s", timeDiffToStr(TimeOfLastMin)));
                if ((macd0 / lastMax) < 1 - 0.05) {
//                    if (numOfmins < numOfMaxs)
if (timeDiff(TimeOfLastMin) > 25 HOURS)
                        numOfmins++;
                    lastMin = macd0;
                    TimeOfLastMin = TimeCurrent();
// string LOGtxt = (SF("min: %.2f, nOf: %d", macd0, numOfmins));
// if (numOfmins == 1) LOGtxt += (SF(", Last change of sign: %s ago XXXXXXXXXXXXXXXXX", timeDiffToStr(TimeOfLastChangeOfSign)));
// LOG(LOGtxt);
                }
            }
        }
    }

    bool is1stPeak() {
// LOG(SF("%s: %s  MAX: %d  min: %d", __FUNCTION__, is1stPeakAlreadyFound?"T":"F", numOfMaxs,numOfmins));
        if (is1stPeakAlreadyFound) return false;
        // if (numOfMaxs == 1 && numOfmins == 0) {
        if (sign > 0 && numOfMaxs == 1 && numOfmins == 0) {
            is1stPeakAlreadyFound = true;
            return true;
        }
        return false;
    }
    bool is2ndPeak() {
//LOG(SF("%s: %s  MAX: %d  min: %d", __FUNCTION__, is2ndPeakAlreadyFound?"T":"F", numOfMaxs, numOfmins));
        if (is2ndPeakAlreadyFound) return false;
        // if (numOfMaxs == 2 && numOfmins == 1) {
        // if (numOfMaxs == 2 && numOfmins <= 1) {
        if (sign > 0 && numOfMaxs == 2 && numOfmins <= 1) {
            is2ndPeakAlreadyFound = true;
            return true;
        }
        return false;
    }

    bool is1stValley() {
//LOG(SF("%s: %s  MAX: %d  min: %d", __FUNCTION__, is1stValleyAlreadyFound?"T":"F", numOfMaxs, numOfmins));
        if (is1stValleyAlreadyFound) return false;
        // if (numOfmins == 1 && numOfMaxs == 0) {
        if (sign < 0 && numOfmins == 1 && numOfMaxs == 0) {
            is1stValleyAlreadyFound = true;
            return true;
        }
        return false;
    }

    bool is2ndValley() {
//LOG(SF("%s: %s  MAX: %d  min: %d", __FUNCTION__, is2ndValleyAlreadyFound?"T":"F", numOfMaxs, numOfmins));
        if (is2ndValleyAlreadyFound) return false;
//        if (numOfmins == 2 && numOfMaxs == 1) {
        // if (numOfmins == 2 && numOfMaxs <= 1) {
        if (sign < 0 && numOfmins == 2 && numOfMaxs <= 1) {
            is2ndValleyAlreadyFound = true;
            return true;
        }
        return false;
    }
};
//+------------------------------------------------------------------+
void logHHLL() {
    static string HHLL = "";
    {   double H = g::zigZag.HighMapBuffer.get(1);
        static double prevH = 99999;
        if (H > 0) {
            for (int i = 2; i < 50; i++) {
                double H = g::zigZag.HighMapBuffer.get(i);
                if (H > 0) {
                    LOG(SF("Prev H[%d]: %.2f", i, H));
                    prevH = H;
                    break;
                }
            }
            string hhlh = H > prevH ? "HH" : "LH";
            if (H > prevH && StringSubstr(HHLL, 0, 2) == "LH") {
                StringSetCharacter(HHLL, 0, 'H');
            }
            else if (StringSubstr(HHLL, 0, 2) != hhlh) {
                HHLL = hhlh + "-" + HHLL;
                if (StringLen(HHLL) > 75) {
                    HHLL = HHLL.Substr(0, 50) + "...";
                }
            }
            LOG(SF("ZZ:H: %.2f: %s (%s)", H, hhlh, HHLL));
            prevH = H;
        }
    }
    {   double L = g::zigZag.LowMapBuffer.get(1);
        static double prevL = 0;
        if (L > 0) {
            for (int i = 2; i < 50; i++) {
                double L = g::zigZag.LowMapBuffer.get(i);
                if (L > 0) {
                    LOG(SF("Prev L[%d]: %.2f", i, L));
                    prevL = L;
                    break;
                }
            }
            string llhl = L > prevL ? "HL" : "LL";
            if (L < prevL && StringSubstr(HHLL, 0, 2) == "HL") {
                StringSetCharacter(HHLL, 0, 'L');
            }
            else if (StringSubstr(HHLL, 0, 2) != llhl) {
                HHLL = llhl + "-" + HHLL;
            }
            LOG(SF("ZZ:L: %.2f: %s (%s)", L, llhl, HHLL));
            prevL = L;
        }
    }
}
//+------------------------------------------------------------------+
class ProfitEtc
{
private:
    int    logCnt;

public:
    double totalPricePaid;
    double maxProfit;
    double maxEquity;
    double maxBalance;
    double cummPrLossPerPrice;

    double profit;
    double balance;
    double equity;
    double profitPerBalance;
    double profitLossPerBal;
    double profitPerPrice;
    double profitLossPerPrice;

    void newLogHeader() { logCnt = 0; }

    void setValues() {
        totalPricePaid = g::pPos.getTotalPricePaid();
        balance = AccountInfoDouble(ACCOUNT_BALANCE);
        equity  = AccountInfoDouble(ACCOUNT_EQUITY);
        profit  = equity - balance;

        if (profit  > maxProfit)  { maxProfit = profit; }
        if (equity  > maxEquity)  { maxEquity = equity; }
        if (balance > maxBalance) { maxBalance = balance; }

        profitPerBalance   = profit / balance;
        profitLossPerBal   = (profit-maxProfit) / balance;
        profitPerPrice     = profit / totalPricePaid;
        profitLossPerPrice = (profit-maxProfit) / totalPricePaid;

        if (profitPerPrice < 0) { cummPrLossPerPrice += profitPerPrice; }
        else { cummPrLossPerPrice = 0; }

        double relDrawDown = 1 - balance / maxBalance;
        if (relDrawDown > g::maxRelDrawDown) { g::maxRelDrawDown = relDrawDown; }
    }

    void logHeader() {
        LOG("--  Pro    PrLs/Bal PrLs/Pri  Pro/Bal  Pro/Pri  CmPr/Pr     Eq     Eq/EqMx    Bal   RlDrDn");
    }
    void log() {
        logCnt++;
        if (logCnt % 20 == 1) { logHeader(); }

        LOG(SF("%8s %+7.1f%%  %+7.1f%%   %+6.1f%%  %+6.1f%%  %+6.1f%%  %7s  %+6.1f%%  %7s %6.1f%%",
            d2str(profit),
            profitLossPerBal * 100,
            profitLossPerPrice * 100,
            profitPerBalance * 100.0,
            profitPerPrice * 100.0,
            cummPrLossPerPrice * 100,
            d2str(equity),
            (equity-maxEquity) / maxEquity * 100,
            d2str(balance),
            g::maxRelDrawDown * 100)
        );
    }
};
//+------------------------------------------------------------------+
void OnTick()
{
    static bool stopToRespond = false;

    if (stopToRespond) return;

    //if (MQLInfoInteger(MQL_TESTER) && g::maxRelDrawDown > maxRelDrawDownLimit) return;

    static ProfitEtc p;

    p.setValues();
  
    if (p.totalPricePaid == 0) return;

    if (g::indicatorList.copyBuffers(300) == false) {
        LOG("Failed to copy data from buffer");
        stopToRespond = true;
        return;
    }

    if (isNewMinute()) {
        p.log();
        logHHLL();
    }

    if (p.profitPerBalance < -profitPerBalanceLimit) {
        changeDirection(Reason::chDir_profitPerBalanceLimit, __LINE__);
    }
    else if (p.maxProfit > 0 && p.profitLossPerBal < -profitLossPerBalLimit) {
        changeDirection(Reason::chDir_profitLossPerBalLimit, __LINE__);
    }
    else if (p.profitPerPrice < -profitPerPriceLimit) {
        changeDirection(Reason::chDir_profitPerPriceLimit, __LINE__);
    }
    else if (p.profitLossPerPrice < -profitLossPerPriceLimit) {
        changeDirection(Reason::chDir_profitLossPerPriceLimit, __LINE__);
    }
    else if (p.cummPrLossPerPrice < -cummPrLossPerPriceLimit) {
        changeDirection(Reason::chDir_cummPrLossPerPriceLimit, __LINE__);
    }
    else
    {
        static MACD_PeaksAndValleys MACD1peaksAndValleys(g::MACD1);

        MACD1peaksAndValleys.process();

        if (g::MACD1.OsMA_justChangedPositive()) {
            g::sellOrBuy.set(SellOrBuy::State::BuyNow, Reason::OsMA_pos, __LINE__);
        } else
        if (g::MACD1.OsMA_justChangedNegative()) {
                    g::sellOrBuy.set(SellOrBuy::State::SellNow, Reason::OsMA_neg, __LINE__);
        } else
        if (MACD1peaksAndValleys.is1stPeak()) {
            // LOG(SF("1st Peak: p.profit = %.2f eq = %.2f bal = %.2f   pr/bal = %.2f %%  --------------------", p.profit, p.equity, p.balance, p.profitPerBalance * 100));
            if (p.profitPerBalance > 0.5) {
                //    g::sellOrBuy.set(SellOrBuy::State::SellNow, Reason::peakNr1, __LINE__);
            }
        } else 
        if (MACD1peaksAndValleys.is2ndPeak()) {
            g::sellOrBuy.set(SellOrBuy::State::SellNow, Reason::peakNr2, __LINE__);
        } else 
        if (MACD1peaksAndValleys.is1stValley()) {
            // LOG(SF("1st Valley: p.profit = %.2f eq = %.2f bal = %.2f   pr/bal = %.2f %%  --------------------", p.profit, p.equity, p.balance, p.profitPerBalance*100));
            if (p.profitPerBalance > 0.5) {
            //    g::sellOrBuy.set(SellOrBuy::State::BuyNow, Reason::valleyNr1, __LINE__);
            }
        } else 
        if (MACD1peaksAndValleys.is2ndValley()) {
            g::sellOrBuy.set(SellOrBuy::State::BuyNow, Reason::valleyNr2, __LINE__);
        }
    }

    if (g::sellOrBuy.isGetReadyToBuy()) {
        if (g::ATR_list.isBuyNow(getPrice().low)) {
            g::sellOrBuy.set(SellOrBuy::State::BuyNow, Reason::ATR_low, __LINE__);
        }
    }
    else
    if (g::sellOrBuy.isGetReadyToSell()) {
        if (g::ATR_list.isSellNow(getPrice().high)) {
            g::sellOrBuy.set(SellOrBuy::State::SellNow, Reason::ATR_high, __LINE__);
        }
    }

    if (g::sellOrBuy.isBuyNow()) {
        Reason::ReasonCode reason = g::sellOrBuy.getReason();
        if (g::pPos.select())
        {
            if (g::pPos.isTypeSELL()) {
                g::pPos.close(reason, p.profitPerBalance*100);
            }
            else if (g::pPos.isTypeBUY()) {
                g::sellOrBuy.set(SellOrBuy::State::None, Reason::Bought, __LINE__);
                LOG(" Already bought");
                return;
            }
        }
        if (g::pPos.buy(reason)) {
            g::sellOrBuy.set(SellOrBuy::State::None, Reason::Bought, __LINE__);
            p.cummPrLossPerPrice = 0;
            p.maxProfit = 0;
            p.newLogHeader();
        }
    }
    else if (g::sellOrBuy.isSellNow()) {
        Reason::ReasonCode reason = g::sellOrBuy.getReason();
        if (g::pPos.select())
        {
            if (g::pPos.isTypeBUY()) {
                g::pPos.close(reason, p.profitPerBalance*100);
            }
            else if (g::pPos.isTypeSELL()) {
                g::sellOrBuy.set(SellOrBuy::State::None, Reason::Sold, __LINE__);
                LOG(" Already sold");
                return;
            }
        }
        if (g::pPos.sell(reason)) {
            g::sellOrBuy.set(SellOrBuy::State::None, Reason::Sold, __LINE__);
            p.cummPrLossPerPrice = 0;
            p.maxProfit = 0;
            p.newLogHeader();
        }
    }
}
//+------------------------------------------------------------------+
MqlRates getPrice()
{
    MqlRates bar[2];
    if(CopyRates(_Symbol,_Period, 0, 2, bar) > 0) {
    }
    return bar[1];
}
//+------------------------------------------------------------------+
double OnTester()
{
    if (g::maxRelDrawDown > maxRelDrawDownLimit) return 0;
    if (g::maxRelDrawDown < 0.01) return 0;
    if (AccountInfoDouble(ACCOUNT_BALANCE) < 10) return 0;
    // return AccountInfoDouble(ACCOUNT_BALANCE);
    return AccountInfoDouble(ACCOUNT_BALANCE) / g::maxRelDrawDown;
}
//+------------------------------------------------------------------+

//+------------------------------------------------------------------+

