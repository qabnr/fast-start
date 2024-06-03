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
input double profitPerBalanceLimit    = 10.19;
input double profitLossPerBalLimit    = 0.46;
input int    maxTransactions          = 1000;
input double equityTradeLimit         = 1.00;
input double tradeSizeFraction        = 1.00;
input int    LastChangeOfSignMinLimit = 98810;
input int    LastChangeOfSignMaxLimit = 250950;
input double profitPerPriceLimit      = 2.98;
input double cummPrLossPerPriceLimit  = 29.68;
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
    string  name;
    int     handle;
    int     buffNum;
    double  buff[];
    int     nrCopied;

public:
    Buffer(): name("<NO NAME>"), handle(INVALID_HANDLE), buffNum(0), nrCopied(0) {
        ArraySetAsSeries(buff, true);
    }
    Buffer(int _buffNum, string _name, int _handle): name(_name), handle(_handle), buffNum(_buffNum), nrCopied(0)
    {
        ArraySetAsSeries(buff, true);
Print("New buffer: ", buffNum, " handle: ", name, " (", handle, ")");
    }
    void addHandleAndBuffNum(string _name, int _handle, int _buffNum) {
         name    =  _name;
         handle  = _handle;
         buffNum = _buffNum;
Print("New buffer: ", buffNum, " handle: ", name, " (", handle, ")");
    }
    ~Buffer() {};

    bool copy(int count) {
        nrCopied = CopyBuffer(handle, buffNum, 0, count, buff);
        if (nrCopied <= 0)
        {   Print("Failed to copy data from buffer: ", buffNum, " handle: ", name, " (", handle, ")");  }
        return nrCopied > 0;
    }
    double get(int index) {
        return buff[index];
    }
    int getNrCopied() {
        return nrCopied;
    }
};
//+------------------------------------------------------------------+
class ATR_TR_STOP
{
private:
    int    handle;
    Buffer stopBuffer;
    Buffer stopColorBuffer;

    Buffer buyBuffer;
    Buffer buyColorBuffer;

    Buffer sellBuffer;
    Buffer sellColorBuffer;

public:
    ATR_TR_STOP() : handle(INVALID_HANDLE) {}
    ~ATR_TR_STOP() {}
  
    void init(int ATRperiod, double mult) {
        handle  = iCustom(NULL, PERIOD_CURRENT, "myATR_TR_STOP", ATRperiod, mult);
        if (handle == INVALID_HANDLE) {
            Print("Failed to get the the ATR indicator(", ATRperiod, ", ", mult, ") handle");
            return;
        }

        string short_name = SF("ATR_TR_ST(%d, %.1f)", ATRperiod, mult);
        stopBuffer     .addHandleAndBuffNum(short_name, handle, 0);
        stopColorBuffer.addHandleAndBuffNum(short_name, handle, 1);
        buyBuffer      .addHandleAndBuffNum(short_name, handle, 2);
        buyColorBuffer. addHandleAndBuffNum(short_name, handle, 3);
        sellBuffer     .addHandleAndBuffNum(short_name, handle, 4);
        sellColorBuffer.addHandleAndBuffNum(short_name, handle, 5);
    }

    bool copyBuffers(int count) {
        return
           stopBuffer.copy(count) && stopColorBuffer.copy(count)
        && buyBuffer .copy(count) && buyColorBuffer .copy(count)
        && sellBuffer.copy(count) && sellColorBuffer.copy(count);
    }

    bool isBuyNow(double price) {
//Print(__FUNCTION__, ": ", buyBuffer.get(1), ", ", price);
        return buyBuffer.get(1) < price;
    }

    bool isSellNow(double price) {
//Print(__FUNCTION__, ": ", sellBuffer.get(1), ", ", price);
        return sellBuffer.get(1) > price;
    }
};
//+------------------------------------------------------------------+
class ATR_TR_STOP_List
{
private:
    ATR_TR_STOP atrTrStList[];

public:
    ATR_TR_STOP_List() { ArrayResize(atrTrStList, 0, 100); }
    ~ATR_TR_STOP_List() {};

    void add(int ATRper, double mult) {
        ArrayResize(atrTrStList, ArraySize(atrTrStList) + 1, 100);
        atrTrStList[ArraySize(atrTrStList)-1].init(ATRper, mult);
    }

    bool copyBuffers(int count) {
        for (int i = 0; i < ArraySize(atrTrStList); i++)
        {   if (!atrTrStList[i].copyBuffers(count)) return false;
        }
        return true;
    }
    bool isBuyNow(double price) {
        for (int i = 0; i < ArraySize(atrTrStList); i++)
        {   if (atrTrStList[i].isBuyNow(price)) return true;
        }
        return false;
    }
    bool isSellNow(double price) {
        for (int i = 0; i < ArraySize(atrTrStList); i++)
        {   if (atrTrStList[i].isSellNow(price)) return true;
        }
        return false;
    }
};
//+------------------------------------------------------------------+
class MACD_base
{
private:
public:
    int handle;

    Buffer MACD_Buffer;
    Buffer Signal_Buffer;
    Buffer OsMA_Buffer;
    Buffer osMA_Color_Buffer;

    MACD_base(const string customIndicatorName,
              const string bufferName,
              const int fast_MA_period,
              const int slow_MA_period,
              const int avg_diff_period,
              const int MACD_Buffer_buffNum,
              const int Signal_Buffer_buffNum,
              const int OsMA_Buffer_buffNum,
              const int osMA_Color_Buffer_buffNum)
      : handle(iCustom(NULL, PERIOD_CURRENT, customIndicatorName, fast_MA_period, slow_MA_period, avg_diff_period)),
        MACD_Buffer      (MACD_Buffer_buffNum,       bufferName, handle),
        Signal_Buffer    (Signal_Buffer_buffNum,     bufferName, handle),
        OsMA_Buffer      (OsMA_Buffer_buffNum,       bufferName, handle),
        osMA_Color_Buffer(osMA_Color_Buffer_buffNum, bufferName, handle)
    {
        if (handle == INVALID_HANDLE)
        {
            Print("Failed to get indicator handle");
        }
    }

    virtual ~MACD_base() {};

    bool copyBuffers(const int count)
    {
        return
            MACD_Buffer      .copy(count) &&
            Signal_Buffer    .copy(count) &&
            OsMA_Buffer      .copy(count) &&
            osMA_Color_Buffer.copy(count);
    }

    bool justChangedToDownTrend() {
        return OsMA_Buffer.get(0) < OsMA_Buffer.get(1)
            && OsMA_Buffer.get(1) > OsMA_Buffer.get(2);
    }

    bool justChangedToUpTrend() {
        return OsMA_Buffer.get(0) > OsMA_Buffer.get(1)
            && OsMA_Buffer.get(1) < OsMA_Buffer.get(2);
    }

    bool OsMA_justChangedPositive() {
        return OsMA_Buffer.get(0) >  0
            && OsMA_Buffer.get(1) <= 0
            && MACD_Buffer.get(0) < 0;
    }

    bool OsMA_justChangedNegative() {
        return OsMA_Buffer.get(0) <  0
            && OsMA_Buffer.get(1) >= 0
            && MACD_Buffer.get(0) > 0;
    }
};
//+------------------------------------------------------------------+
class MACD : public MACD_base
{
public:
    Buffer decPeriod_Buffer;
    Buffer incPeriod_Buffer;
    Buffer decPeriod_OsMA_Buffer;
    Buffer incPeriod_OsMA_Buffer;

    MACD(const string bufferName, const int fast_MA_period, const int slow_MA_period, const int avg_diff_period)
        : MACD_base("myMACD", bufferName, fast_MA_period, slow_MA_period, avg_diff_period,
        5, 4 , 2, 3),
        decPeriod_Buffer     (0, bufferName, handle),
        incPeriod_Buffer     (1, bufferName, handle),
        decPeriod_OsMA_Buffer(6, bufferName, handle),
        incPeriod_OsMA_Buffer(7, bufferName, handle)
    { }
    ~MACD() {}

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
    { }

    ~myMACD2() {}
};
//+------------------------------------------------------------------+
class LinRegrChannel
{
private:
    int handle;

public:
    Buffer buffer;

    LinRegrChannel(string bufferName): handle(iCustom(NULL, PERIOD_CURRENT, "linRegrChannel")),
        buffer(0, bufferName, handle)
    {
        if(handle  == INVALID_HANDLE)
        {   Print("Failed to get indicator handle"); }
    }
    ~LinRegrChannel() {}

    bool copyBuffers(int count)
    {
        return buffer.copy(count);
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

    double sum() { return sum_; }
    double avg() { return last_ <= 0 ? 0: sum_/last_; }

    double variance() {
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

    double stdDev() { return MathSqrt(variance()); }
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
    string op2str(int op) {
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

    string Reason2str() { return Reason::toStr(reason); }

    string State2str() { return State2str(state); }

    string State2str(State s) {
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
    string        my_symbol;
    CTrade        m_Trade;
    CPositionInfo m_Position;

    int           posType;
    double        volume;
    double        totalPricePaid;
    Stats         stats;

public:
    TradePosition(string symbol): my_symbol(symbol),
        volume(MathFloor(SymbolInfoDouble(symbol, SYMBOL_VOLUME_MAX) / tradeSizeFraction)),
        posType(-1), totalPricePaid(0.01)
    {}
    ~TradePosition() { stats.print(); }

    bool select()     { return m_Position.Select(my_symbol); }
    bool isTypeSELL() { return posType == POSITION_TYPE_SELL; }
    bool isTypeBUY()  { return posType == POSITION_TYPE_BUY; }
    double getTotalPricePaid() { return totalPricePaid; }

    void close(Reason::ReasonCode reason, double profit) {
LOG(SF("Close, profit: %+.1f%%", (profit)));

        stats.addOpReason(stats.close,  reason);
        stats.addProfit(profit, reason);

        while(m_Trade.PositionClose(my_symbol)) {
// LOG(SF("Close: Ticket: %u  Price = %.2f", m_Trade.ResultDeal(), m_Trade.ResultPrice()));
        }
    }

    void buy(Reason::ReasonCode reason) {
        stats.addOpReason(stats.buy, reason);

        double freeMarginBeforeTrade = AccountInfoDouble(ACCOUNT_FREEMARGIN);

        double executionPrice   = 0.0;
        double stopLoss         = 0.0;
        double takeProfit       = 0.0;

        for (int i = maxTransactions; i > 0; i--)
        {
            bool res = m_Trade.Buy(volume, my_symbol, executionPrice, stopLoss, takeProfit);
            if (!res) break;
// LOG(SF("Buy:   Ticket: %u  Price = %.2f", m_Trade.ResultDeal(), m_Trade.ResultPrice()));
            posType = POSITION_TYPE_BUY;
            if (AccountInfoDouble(ACCOUNT_FREEMARGIN) < freeMarginBeforeTrade * equityTradeLimit) {
                break;
            }
        }
        totalPricePaid = freeMarginBeforeTrade - AccountInfoDouble(ACCOUNT_FREEMARGIN);
LOG(SF("BUY for %s at %.2f each", d2str(totalPricePaid), m_Trade.ResultPrice()));
    }

    void sell(Reason::ReasonCode reason) {
        stats.addOpReason(stats.sell, reason);

        double freeMarginBeforeTrade = AccountInfoDouble(ACCOUNT_FREEMARGIN);

        double executionPrice   = 0.0;
        double stopLoss         = 0.0;
        double takeProfit       = 0.0;

        for (int i = maxTransactions; i > 0; i--) {
            bool res = m_Trade.Sell(volume, my_symbol, executionPrice, stopLoss, takeProfit);
            if (!res) break;
// LOG(SF("Sell:  Ticket: %u  Price = %.2f", m_Trade.ResultDeal(), m_Trade.ResultPrice()));          
            posType = POSITION_TYPE_SELL;
            if (AccountInfoDouble(ACCOUNT_FREEMARGIN) < freeMarginBeforeTrade * equityTradeLimit) {
                break;
            }
        }
        totalPricePaid = freeMarginBeforeTrade - AccountInfoDouble(ACCOUNT_FREEMARGIN);
LOG(SF("SELL for %s at %.2f each", d2str(totalPricePaid), m_Trade.ResultPrice()));
    }
};
//+------------------------------------------------------------------+
namespace g
{
    ATR_TR_STOP_List ATR_list;
    myMACD2         *MACD1;
    myMACD2         *MACD2;
    TradePosition   *pPos;
    LinRegrChannel  *linRegrChannel;
    SellOrBuy        sellOrBuy;
    double           maxRelDrawDown = 0;
};
//+------------------------------------------------------------------+
int OnInit()
{
    printInputParams();
    
    g::MACD1 = new myMACD2("MACD1", MACD1_fast_MA_period, MACD1_slow_MA_period, MACD1_avg_diff_period);
    g::MACD2 = new myMACD2("MACD2", MACD2_fast_MA_period, MACD2_slow_MA_period, MACD2_avg_diff_period);

    // g::linRegrChannel = new LinRegrChannel("LRCh");

    g::pPos  = new TradePosition(Symbol());

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
    // delete g::linRegrChannel;
    delete g::pPos;
    delete g::MACD1;
    delete g::MACD2;
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
class MACD1_PeaksAndValleys
{
private:
    int      sign;
    datetime TimeOfLastChangeOfSign;
    int      numOfmins, numOfMaxs;
    double   lastMin,   lastMax;
    datetime TimeOfLastMin, TimeOfLastMax;
    bool     is1stPeakAlreadyFound,   is2ndPeakAlreadyFound;
    bool     is1stValleyAlreadyFound, is2ndValleyAlreadyFound;

    bool isMin() {
//LOG(SF("(%d) %.3f  %.3f", 1, g::MACD1.MACD_Buffer.get(1), g::MACD1.MACD_Buffer.get(2)));
        if (g::MACD1.MACD_Buffer.get(1) < g::MACD1.MACD_Buffer.get(2)) return false;
        for (int i = 2; i < minMaxBAckTrack; i++) {
//LOG(SF("(%d) %.3f  %.3f", i, g::MACD1.MACD_Buffer.get(i), g::MACD1.MACD_Buffer.get(i+1)));
            if (g::MACD1.MACD_Buffer.get(i) > g::MACD1.MACD_Buffer.get(i+1)) return false;
        }
// LOG("MIN found");
        return true;
    };

    bool isMax() {
//LOG(SF("(%d) %.3f  %.3f", 1, g::MACD1.MACD_Buffer.get(1), g::MACD1.MACD_Buffer.get(2)));
        if (g::MACD1.MACD_Buffer.get(1) > g::MACD1.MACD_Buffer.get(2)) return false;
        for (int i = 2; i < minMaxBAckTrack; i++) {
//LOG(SF("(%d) %.3f  %.3f", i, g::MACD1.MACD_Buffer.get(i), g::MACD1.MACD_Buffer.get(i+1)));
            if (g::MACD1.MACD_Buffer.get(i) < g::MACD1.MACD_Buffer.get(i+1)) return false;
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
    MACD1_PeaksAndValleys() :
        sign(-2),
        TimeOfLastChangeOfSign(TimeCurrent()),
        TimeOfLastMin(0),
        TimeOfLastMax(0)
    {   initValues(0); }
    ~MACD1_PeaksAndValleys() {};

    void LogMACD_Last(int cnt) {
        if (!isLOG()) return;
        string s;
        for (int i = 0; i < cnt; i++) {
            s += SF("%.2f ", g::MACD1.MACD_Buffer.get(i));
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

        double macd0 = g::MACD1.MACD_Buffer.get(1);
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
void OnTick()
{
    static int logCnt = 0;

    //if (MQLInfoInteger(MQL_TESTER) && g::maxRelDrawDown > maxRelDrawDownLimit) return;
  
    double totalPricePaid = g::pPos.getTotalPricePaid();

    if (totalPricePaid == 0) return;

    if (copyBuffers() == false)
    {   Print("Failed to copy data from buffer"); return; }

    static double maxProfit  = 0;
    static double maxEquity  = 0;
    static double maxBalance = 0;

    double profit, balance, equity;

    balance = AccountInfoDouble(ACCOUNT_BALANCE);
    equity  = AccountInfoDouble(ACCOUNT_EQUITY);
    profit  = equity - balance;

    if (profit  > maxProfit)  { maxProfit = profit; }
    if (equity  > maxEquity)  { maxEquity = equity; }
    if (balance > maxBalance) { maxBalance = balance; }

    double profitPerBalance       = profit / balance;
    double profitLossPerBal       = (profit-maxProfit) / balance;
    double profitPerPrice         = profit / totalPricePaid;

static double cummPrLossPerPrice = 0;

if (profitPerPrice < 0) {
    cummPrLossPerPrice += profitPerPrice;
}
else {
    cummPrLossPerPrice = 0;
}

    double profitLossPerPrice = (profit-maxProfit) / totalPricePaid;

    double relDrawDown = 1 - balance / maxBalance;
    if (relDrawDown > g::maxRelDrawDown) { g::maxRelDrawDown = relDrawDown; }

if (isNewMinute()) {
    logCnt++;
    if (logCnt % 20 == 1) {
        LOG("--  Pro    PrLs/Bal PrLs/Pri  Pro/Bal  Pro/Pri  CmPr/Pr     Eq     Eq/EqMx    Bal   RlDrDn");
    }
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
        g::maxRelDrawDown * 100));
}

    if (profitPerBalance < -profitPerBalanceLimit) {
        changeDirection(Reason::chDir_profitPerBalanceLimit, __LINE__);
    }
    else if (maxProfit > 0 && profitLossPerBal < -profitLossPerBalLimit) {
        changeDirection(Reason::chDir_profitLossPerBalLimit, __LINE__);
    }
    else if (profitPerPrice < -profitPerPriceLimit) {
        changeDirection(Reason::chDir_profitPerPriceLimit, __LINE__);
    }
    else if (profitLossPerPrice < -profitLossPerPriceLimit) {
        changeDirection(Reason::chDir_profitLossPerPriceLimit, __LINE__);
    }
    else if (cummPrLossPerPrice < -cummPrLossPerPriceLimit) {
        changeDirection(Reason::chDir_cummPrLossPerPriceLimit, __LINE__);
    }
    else
    {
        static MACD1_PeaksAndValleys MACD1peaksAndValleys;

        MACD1peaksAndValleys.process();

        if (g::MACD1.OsMA_justChangedPositive()) {
            g::sellOrBuy.set(SellOrBuy::State::BuyNow, Reason::OsMA_pos, __LINE__);
        } else
        if (g::MACD1.OsMA_justChangedNegative()) {
                    g::sellOrBuy.set(SellOrBuy::State::SellNow, Reason::OsMA_neg, __LINE__);
        } else
        if (MACD1peaksAndValleys.is1stPeak()) {
            // LOG(SF("1st Peak: profit = %.2f eq = %.2f bal = %.2f   pr/bal = %.2f %%  --------------------", profit, equity, balance, profitPerBalance * 100));
            if (profitPerBalance > 0.5) {
                //    g::sellOrBuy.set(SellOrBuy::State::SellNow, Reason::peakNr1, __LINE__);
            }
        } else 
        if (MACD1peaksAndValleys.is2ndPeak()) {
            g::sellOrBuy.set(SellOrBuy::State::SellNow, Reason::peakNr2, __LINE__);
        } else 
        if (MACD1peaksAndValleys.is1stValley()) {
            // LOG(SF("1st Valley: profit = %.2f eq = %.2f bal = %.2f   pr/bal = %.2f %%  --------------------", profit, equity, balance, profitPerBalance*100));
            if (profitPerBalance > 0.5) {
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
        g::sellOrBuy.set(SellOrBuy::State::None, Reason::Bought, __LINE__);
        if (g::pPos.select())
        {
            if (g::pPos.isTypeSELL()) {
                g::pPos.close(reason, profitPerBalance*100);
            }
            else if (g::pPos.isTypeBUY()) {
LOG(" Already bought");
                return;
            }
        }
        g::pPos.buy(reason);
        cummPrLossPerPrice = 0;
        maxProfit = 0;
        logCnt = 0;
    }
    else if (g::sellOrBuy.isSellNow()) {
        Reason::ReasonCode reason = g::sellOrBuy.getReason();
        g::sellOrBuy.set(SellOrBuy::State::None, Reason::Sold, __LINE__);
        if (g::pPos.select())
        {
            if (g::pPos.isTypeBUY()) {
                g::pPos.close(reason, profitPerBalance*100);
            }
            else if (g::pPos.isTypeSELL()) {
LOG(" Already sold");
                return;
            }
        }
        g::pPos.sell(reason);
        cummPrLossPerPrice = 0;
        maxProfit = 0;
        logCnt = 0;
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
bool copyBuffers()
{
    const int buffSize = 300;

    if (   !g::MACD1.         copyBuffers(buffSize)
        || !g::MACD2.         copyBuffers(buffSize)
        || !g::ATR_list.      copyBuffers(buffSize)
        // || !g::linRegrChannel.copyBuffers(buffSize)
        )
    {
        Print("Failed to copy data from buffer");
        return false;
    }
    return true;
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

