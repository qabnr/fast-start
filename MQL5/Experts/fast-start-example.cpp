//+------------------------------------------------------------------+
//| fast-start-example.mq5
//+------------------------------------------------------------------+
#ifndef __cplusplus
#property copyright "Copyright 2024, Mogyo Software Corp."
#property link      "http://www.mogyo.com"
#property version   "3.4"
#endif

#include <Trade/Trade.mqh>

#include <adapt.h>
#include <utils.h>

#include "atr_tr_stop.h"
#include "macd.h"
#include "linRegrChannel.h"
#include "zigZag.h"
#include "tr_stop.h"
#include "hhll_list.h"

input int    MACD1_fast_MA_period     = 12;
input int    MACD1_slow_MA_period     = 26;
input int    MACD1_avg_diff_period    = 9;
input int    MACD2_fast_MA_period     = 96;
input int    MACD2_slow_MA_period     = 208;
input int    MACD2_avg_diff_period    = 72;
input double OsMA_limit               = 0.73;
input double decP_OsMa_limit          = 17.08;
input int    minMaxBAckTrack          = 5;
input double profitPerBalanceLimit    = 1.76;
input double profitLossPerBalLimit    = 3.20;
input int    maxTransactions          = 791;
input double equityTradeLimit         = 38.00;
input double tradeSizeFraction        = 2650.00;
input int    LastChangeOfSignMinLimit = 139810;
input int    LastChangeOfSignMaxLimit = 330950;
input double profitPerPriceLimit      = 14.71;
input double cummPrLossPerPriceLimit  = 289.67;
input double profitLossPerPriceLimit  = 56.66;
input double maxRelDrawDownLimit      = 0.70;


//+------------------------------------------------------------------+
void printInputParams()
{
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
namespace Reason
{

#define enum2str_DEFAULT default: return "<UNKNOWN>"

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
class List
{
protected:
    double arr_[];
    int    last_;

public:
    List(): last_(-1) { ArrayResize(arr_, 0, 1000); }
    ~List() {}

    void push(double val) {
        last_++;
        ArrayResize(arr_, last_+1, 1000);
        arr_[last_] = val;
    }
};
//+------------------------------------------------------------------+
class StatList : List
{
private:
    double sum_;
    double sumSq_;

public:
    StatList(): sum_(0), sumSq_(0) {}
    ~StatList() {}

    void push(double val) {
        List::push(val);
        sum_   += val;
        sumSq_ += val * val;
    }

    double sum() const { return sum_; }
    double avg() const { return last_ <= 0 ? 0: sum_/last_; }
    double variance() const {
        return last_ < 0 ? 0 : (sumSq_ - sum_*sum_/(last_+1)) / (last_+1);
    }

    double stdDev() const { return MathSqrt(variance()); }
};
//+------------------------------------------------------------------+
class Stats
{
public:
    enum Operation {
        buy,
        sell,
        close,
        size
    };

private:
    int      cntOp[Operation::size][Reason::ReasonCode::size];
    StatList profitList[Reason::ReasonCode::size];

public:
    string op2str(int op) const {
        switch (op) {
            case Operation::buy:     return "Buy";
            case Operation::sell:    return "Sell";
            case Operation::close:   return "Clse";
            default:                 return "<UNKNOWN>";
        }
    }

    void addOpReason(Operation op, Reason::ReasonCode r) {
        cntOp[op][r]++;
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
class SellOrBuy
{
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

    SellOrBuy(): state(State::None) {}
    ~SellOrBuy() {}

    Reason::ReasonCode getReason() { return reason; }

    void set(State state_, Reason::ReasonCode reason_, int lineNo = 0) {
        if (state != state_) {
            state  = state_;
            reason = reason_;
            LOG_Naked(SF("%5d: ==> %s <== %s", lineNo, State2str(), Reason2str()));
        }
    }

    bool isNone()           const { return state == State::None; }
    bool isGetReadyToBuy()  const { return state == State::GetReadyToBuy; }
    bool isBuyNow()         const { return state == State::BuyNow; }
    bool isGetReadyToSell() const { return state == State::GetReadyToSell; }
    bool isSellNow()        const { return state == State::SellNow; }
    string Reason2str()     const { return Reason::toStr(reason); }
    string State2str()      const { return State2str(state); }

    string State2str(State s) const {
        switch (s) {
            case State::None:           return "None";
            case State::GetReadyToBuy:  return "GetReadyToBuy";
            case State::BuyNow:         return "BuyNow";
            case State::GetReadyToSell: return "GetReadyToSell";
            case State::SellNow:        return "SellNow";
            default:                    return "<UNKNOWN>";
        }
    }
};
//+------------------------------------------------------------------+
class Account
{
    double cash;

public:
    Account(): cash(10000) {}

    void addToCash(double amount) {
        cash += amount;
    }

    double getBalance(void) { 
        return cash + g::pPos.getTotalPricePaid();
    }

    double getEquity(void) {
        return cash + g::pPos.valueOpenPos();
    }
    
    double getFreeMargin() {
        return cash;
    }

    void log(void) {
        LOG(SF("Open pos value: %9.2f, balance: %9.2f, equity: %9.2f, cash: %9.2f", 
               g::pPos.valueOpenPos(), g::account.getBalance(), g::account.getEquity(), cash));
    }
};
//+------------------------------------------------------------------+
class MTrade
{
private:
    CTrade m_trade;
    string symbol;

public:
    MTrade(const string &_symbol) : symbol(_symbol) {}
    ~MTrade() {}

    bool Buy(double volume) {
        double executionPrice   = 0.0;
        double stopLoss         = 0.0;
        double takeProfit       = 0.0;

        return m_trade.Buy(volume, symbol, executionPrice, stopLoss, takeProfit);
    }

    bool Sell(double volume) {
        double executionPrice   = 0.0;
        double stopLoss         = 0.0;
        double takeProfit       = 0.0;

        return m_trade.Sell(volume, symbol, executionPrice, stopLoss, takeProfit);
    }

    bool PositionClose() {
        return m_trade.PositionClose(symbol);
    }

    string ResultRetcodeDescription() const {
        return m_trade.ResultRetcodeDescription();
    }

    uint ResultRetcode() const {
        return m_trade.ResultRetcode();
    }

    double ResultPrice() const {
        return m_trade.ResultPrice();
    }
};
//+------------------------------------------------------------------+

class TradePosition
{
public:
    enum positionType{
        UNKNOWN = -1,
        POSITION_TYPE_SELL,
        POSITION_TYPE_BUY,
    };

private:
    const string    my_symbol;
    MTrade          m_Trade;

    positionType    posType;
    double          volume;
    double          totalPricePaid;
    double          totalVolume;
    Stats           stats;

public:
    TradePosition(): my_symbol(Symbol()),
        volume(MathFloor(1000000.0 / tradeSizeFraction)),
        m_Trade(my_symbol),
        posType(UNKNOWN), totalPricePaid(0.01)
    {}
    ~TradePosition() { stats.print(); }

    bool select()                    { return true; /*PositionSelect(my_symbol);*/ }
    int getType()              const { return posType; }
    double getTotalPricePaid() const { return totalPricePaid; }

    double lastPrice()    { return SymbolInfoDouble(my_symbol, SYMBOL_LAST); }
    double valueOpenPos() {
        if (posType == POSITION_TYPE_BUY)  return totalVolume * lastPrice();
        if (posType == POSITION_TYPE_SELL) return 2 * totalPricePaid - totalVolume * lastPrice();
        return 0;
    }

    MqlRates getPrice()
    {
        MqlRates bar[2];
        if(CopyRates(my_symbol,_Period, 0, 2, bar) > 0) {
        }
        return bar[1];
    }

    void close(Reason::ReasonCode reason, double profit) {
        LOG(SF("Close, profit: %+.1f%%", (profit)));
        g::account.log();

        stats.addOpReason(stats.close,  reason);
        stats.addProfit(profit, reason);

        uint cnt = 0;
        while(m_Trade.PositionClose()) {
            cnt++;
            if (cnt == 1) {
                LOG(SF("Close: %s (%d)", m_Trade.ResultRetcodeDescription(), m_Trade.ResultRetcode()));
            }
        }
        if (cnt == 0) {
            LOG(SF("Close: %s (%d)", m_Trade.ResultRetcodeDescription(), m_Trade.ResultRetcode()));
        }
        else {
            g::account.addToCash(valueOpenPos());
            LOG(SF("CLOSE: %s = %.0f x %.2f", d2str(valueOpenPos()), totalVolume, lastPrice()));
            totalPricePaid = 0.01;
            totalVolume = 0;
        }
    }

    bool buy(Reason::ReasonCode reason) {
        g::account.log();

        stats.addOpReason(stats.buy, reason);

        double freeMarginBeforeBuy = g::account.getFreeMargin();
        double price = lastPrice();

        totalVolume = 0;
        for (int i = maxTransactions; i > 0; i--) {
            bool res = m_Trade.Buy(volume);
            if (!res) {
                LOG(m_Trade.ResultRetcodeDescription());
                break;
            }
            totalVolume += volume;
            g::account.addToCash(-volume * price);
            posType = POSITION_TYPE_BUY;
            if (g::account.getFreeMargin() < freeMarginBeforeBuy * equityTradeLimit) {
                break;
            }
        }
        LOG(SF("Buy: %s (%d)", m_Trade.ResultRetcodeDescription(), m_Trade.ResultRetcode()));
        switch (m_Trade.ResultRetcode()) {
            case TRADE_RETCODE_MARKET_CLOSED:
            case TRADE_RETCODE_NO_MONEY:
               return false;
        }

        totalPricePaid = freeMarginBeforeBuy - g::account.getFreeMargin();
        LOG(SF("BUY for %s = %.0f x %.2f", d2str(totalPricePaid), totalVolume, m_Trade.ResultPrice()));
        LOG(SF("Value of open positions: %s", d2str(valueOpenPos())));

        g::account.log();

        return true;
    }

    bool sell(Reason::ReasonCode reason) {
        g::account.log();

        stats.addOpReason(stats.sell, reason);

        double freeMarginBeforeSell = g::account.getFreeMargin();
        double price = lastPrice();

        totalVolume = 0;
        for (int i = maxTransactions; i > 0; i--) {
            bool res = m_Trade.Sell(volume);
            if (!res) {
                LOG(m_Trade.ResultRetcodeDescription());
                break;
            }
            totalVolume += volume;
            g::account.addToCash(-volume * price);
            posType = POSITION_TYPE_SELL;
            if (g::account.getFreeMargin() < freeMarginBeforeSell * equityTradeLimit) {
                break;
            }  
        }
        LOG(SF("Sell: %s (%d)", m_Trade.ResultRetcodeDescription(), m_Trade.ResultRetcode()));
        switch (m_Trade.ResultRetcode()) {
            case TRADE_RETCODE_MARKET_CLOSED:
            case TRADE_RETCODE_NO_MONEY:
               return false;
        }

        totalPricePaid = freeMarginBeforeSell - g::account.getFreeMargin();
        LOG(SF("SELL for %s = %.0f x %.2f", d2str(totalPricePaid), totalVolume, lastPrice()));
        LOG(SF("Value of open positions: %s", d2str(valueOpenPos())));

        g::account.log();

        return true;
    }
};
//+------------------------------------------------------------------+
namespace g
{
    ATR_TR_STOP_List ATR_list;
    TR_STOP_List     TR_ST_list;
    myMACD2         *MACD1;
    myMACD2         *MACD2;
    myMACD2         *MACD3;
    ZigZag          *zigZag;

    TR_STOP_SIGNAL  *TR_STS;

    TradePosition    pPos;
    LinRegrChannel  *linRegrChannel;
    SellOrBuy        sellOrBuy;
    double           maxRelDrawDown = 0;

    IndicatorList   indicatorList;

    double lastHH;
    double lastMax;
    int    lastMaxTickCnt;
    double lastLL;
    double lastMin;
    int    lastMinTickCnt;

    Account account;
};
//+------------------------------------------------------------------+
int OnInit()
{
    printInputParams();
    
    g::indicatorList.add(g::MACD1  = new myMACD2("MACD1", MACD1_fast_MA_period,   MACD1_slow_MA_period,   MACD1_avg_diff_period  ));
    g::indicatorList.add(g::MACD2  = new myMACD2("MACD2", MACD2_fast_MA_period,   MACD2_slow_MA_period,   MACD2_avg_diff_period  ));
    g::indicatorList.add(g::MACD3  = new myMACD2("MACD3", MACD2_fast_MA_period/2, MACD2_slow_MA_period/2, MACD2_avg_diff_period/2));
    g::indicatorList.add(g::zigZag = new ZigZag ("ZZ"));

    g::indicatorList.add(g::TR_STS = new TR_STOP_SIGNAL(10, 1.61, 1.0));    
    g::indicatorList.add(g::TR_STS = new TR_STOP_SIGNAL(10, 1.80, 2.0));    

    // g::TR_ST_list.add(10, 0.0);
    g::TR_ST_list.add(10, 0.4);
    g::TR_ST_list.add(10, 0.8);
    g::TR_ST_list.add(10, 1.2);
    g::TR_ST_list.add(10, 1.6);

    return 0;
}
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
}
//+------------------------------------------------------------------+
void changeDirection(const Reason::ReasonCode reason, const int lineNo)
{
    if (g::pPos.getType() == TradePosition::POSITION_TYPE_BUY) {
        g::sellOrBuy.set(SellOrBuy::State::SellNow, reason, lineNo);
    }
    else if (g::pPos.getType() == TradePosition::POSITION_TYPE_SELL) {
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
        balance = g::account.getBalance();
        equity  = g::account.getEquity();
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
        LOG("--  Pro    PrLs/Bal PrLs/Pri  Pro/Bal  Pro/Pri  CmPr/Pr     Eq"
        "     Eq/EqMx    Bal   RlDrDn   Pri     H-         L+");
    }
    void log(const int currTickCnt) {
        logCnt++;
        if (logCnt % 20 == 1) { logHeader(); }

        double lastPrice = g::pPos.lastPrice();

        double lastMaxDiffPct = g::lastMax == 0 ? 0 : (lastPrice / g::lastMax - 1) * 100;
        double lastMinDiffPct = g::lastMin == 0 ? 0 : (lastPrice / g::lastMin - 1) * 100;

        LOG(SF("%8s %+7.1f%%  %+7.1f%%   %+6.1f%%  %+6.1f%%  %+6.1f%%  %7s  %+6.1f%%  %7s %6.1f%% %6.2f H%+0.1f%%(%2d) L%+0.1f%%(%2d)",
            d2str(profit),
            profitLossPerBal * 100,
            profitLossPerPrice * 100,
            profitPerBalance * 100.0,
            profitPerPrice * 100.0,
            cummPrLossPerPrice * 100,
            d2str(equity),
            (equity-maxEquity) / maxEquity * 100,
            d2str(balance),
            g::maxRelDrawDown * 100,
            lastPrice,
            lastMaxDiffPct,
            currTickCnt - g::lastMaxTickCnt,
            lastMinDiffPct,
            currTickCnt - g::lastMinTickCnt
        ));
    }
};
//+------------------------------------------------------------------+
void OnTick()
{
    static bool stopToRespond = false;
    if (stopToRespond) return;
    if (!isNewPeriod()) return;

    static int tickCnt = 0;
    tickCnt++;
    static ProfitEtc p;

    p.setValues();
    if (p.totalPricePaid == 0) return;
    if (g::indicatorList.copyBuffers(300) == false) {
        LOG("Failed to copy data from buffer");
        stopToRespond = true;
        return;
    }

    if (isNewMinute()) {
        // static HHLLlist hhll;
        // hhll.log(tickCnt);
        // p.log(tickCnt);
    }

    MqlRates price = g::pPos.getPrice();
    handleTradeSignals(p, price);
    handleBuySell(p, price);
}
//+------------------------------------------------------------------+
void handleTradeSignals(ProfitEtc& p, const MqlRates& price)
{
    int len = 0;
    if ((len = g::TR_ST_list.isBuyNow(price.open)) > 0) {
        LOG(SF("TRST: BuyNow: %d", len));
    }
    else if ((len = g::TR_ST_list.isSellNow(price.open)) > 0) {
        LOG(SF("TRST: SellNow: %d", len));
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
    else {
        handleMACDSignals(p);
    }
}
//+------------------------------------------------------------------+
void handleMACDSignals(ProfitEtc& p)
{
    static MACD_PeaksAndValleys MACD1peaksAndValleys(g::MACD1);
    MACD1peaksAndValleys.process();

    if (g::MACD1.OsMA_justChangedPositive()) {
        g::sellOrBuy.set(SellOrBuy::State::BuyNow, Reason::OsMA_pos, __LINE__);
    } else if (g::MACD1.OsMA_justChangedNegative()) {
        g::sellOrBuy.set(SellOrBuy::State::SellNow, Reason::OsMA_neg, __LINE__);
    } else if (MACD1peaksAndValleys.is1stPeak()) {
        if (p.profitPerBalance > 0.5) {
            // g::sellOrBuy.set(SellOrBuy::State::SellNow, Reason::peakNr1, __LINE__);
        }
    } else if (MACD1peaksAndValleys.is2ndPeak()) {
        g::sellOrBuy.set(SellOrBuy::State::SellNow, Reason::peakNr2, __LINE__);
    } else if (MACD1peaksAndValleys.is1stValley()) {
        if (p.profitPerBalance > 0.5) {
            // g::sellOrBuy.set(SellOrBuy::State::BuyNow, Reason::valleyNr1, __LINE__);
        }
    } else if (MACD1peaksAndValleys.is2ndValley()) {
        g::sellOrBuy.set(SellOrBuy::State::BuyNow, Reason::valleyNr2, __LINE__);
    }
}
//+------------------------------------------------------------------+
void handleBuySell(ProfitEtc& p, const MqlRates& price)
{
    if (g::sellOrBuy.isGetReadyToBuy()) {
        if (g::ATR_list.isBuyNow(price.low)) {
            g::sellOrBuy.set(SellOrBuy::State::BuyNow, Reason::ATR_low, __LINE__);
        }
    }
    else if (g::sellOrBuy.isGetReadyToSell()) {
        if (g::ATR_list.isSellNow(price.high)) {
            g::sellOrBuy.set(SellOrBuy::State::SellNow, Reason::ATR_high, __LINE__);
        }
    }

    if (g::sellOrBuy.isBuyNow()) {
        executeTrade(p, TradePosition::POSITION_TYPE_BUY, Reason::Bought);
    }
    else if (g::sellOrBuy.isSellNow()) {
        executeTrade(p, TradePosition::POSITION_TYPE_SELL, Reason::Sold);
    }
}
//+------------------------------------------------------------------+
void executeTrade(ProfitEtc& p, int type, Reason::ReasonCode newReason)
{
    Reason::ReasonCode oldReason = g::sellOrBuy.getReason();
    if (g::pPos.select()) {
        if (g::pPos.getType() == type) {
            g::sellOrBuy.set(SellOrBuy::State::None, newReason, __LINE__);
            LOG(type == TradePosition::POSITION_TYPE_BUY ? " Already bought" : " Already sold");
            return;
        }
        g::pPos.close(oldReason, p.profitPerBalance * 100);
    }
    if (type == TradePosition::POSITION_TYPE_BUY ? g::pPos.buy(oldReason) : g::pPos.sell(oldReason)) {
        g::sellOrBuy.set(SellOrBuy::State::None, newReason, __LINE__);
        p.cummPrLossPerPrice = 0;
        p.maxProfit = 0;
        p.newLogHeader();
    }
}
//+------------------------------------------------------------------+
double OnTester()
{
    if (g::maxRelDrawDown > maxRelDrawDownLimit) return 0;
    if (g::maxRelDrawDown < 0.01) return 0;
    if (g::account.getBalance() < 10) return 0;
    // return g::account.getBalance();
    return g::account.getBalance() / g::maxRelDrawDown;
}
//+------------------------------------------------------------------+

//+------------------------------------------------------------------+
