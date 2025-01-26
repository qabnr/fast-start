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
input double OsMA_limit               = 0.336;
input double decP_OsMa_limit          = 5.325;
input int    minMaxBAckTrack          = 11;
input double profitPerBalanceLimit    = 1.632;
input double profitLossPerBalLimit    = 0.410;
input int    maxTransactions          = 11;
input double freeMarginTradeLimit     = 71.00;
input double tradeSize                = 0; // > 0: execute trades in MT5
input int    LastChangeOfSignMinLimit = 139810;
input int    LastChangeOfSignMaxLimit = 330950;
input double profitPerPriceLimit      = 14.705;
input double cummPrLossPerPriceLimit  = 289.674;
input double profitLossPerPriceLimit  = 56.658;
input double maxRelDrawDownLimit      = 0.70;


//+------------------------------------------------------------------+
void printInputParams()
{
    PrintFormat("input int    MACD1_fast_MA_period     = %d;",   MACD1_fast_MA_period);
    PrintFormat("input int    MACD1_slow_MA_period     = %d;",   MACD1_slow_MA_period);
    PrintFormat("input int    MACD1_avg_diff_period    = %d;",   MACD1_avg_diff_period);
    PrintFormat("input int    MACD2_fast_MA_period     = %d;",   MACD2_fast_MA_period);
    PrintFormat("input int    MACD2_slow_MA_period     = %d;",   MACD2_slow_MA_period);
    PrintFormat("input int    MACD2_avg_diff_period    = %d;",   MACD2_avg_diff_period);
    PrintFormat("input double OsMA_limit               = %.3f;", OsMA_limit);
    PrintFormat("input double decP_OsMa_limit          = %.3f;", decP_OsMa_limit);
    PrintFormat("input int    minMaxBAckTrack          = %d;",   minMaxBAckTrack);
    PrintFormat("input double profitPerBalanceLimit    = %.3f;", profitPerBalanceLimit);
    PrintFormat("input double profitLossPerBalLimit    = %.3f;", profitLossPerBalLimit);
    PrintFormat("input int    maxTransactions          = %d;",   maxTransactions);
    PrintFormat("input double freeMarginTradeLimit     = %.3f;", freeMarginTradeLimit);
    PrintFormat("input double tradeSize                = %.3f;", tradeSize);
    PrintFormat("input int    LastChangeOfSignMinLimit = %d;",   LastChangeOfSignMinLimit);
    PrintFormat("input int    LastChangeOfSignMaxLimit = %d;",   LastChangeOfSignMaxLimit);
    PrintFormat("input double profitPerPriceLimit      = %.3f;", profitPerPriceLimit);
    PrintFormat("input double cummPrLossPerPriceLimit  = %.3f;", cummPrLossPerPriceLimit);
    PrintFormat("input double profitLossPerPriceLimit  = %.3f;", profitLossPerPriceLimit);
    PrintFormat("input double maxRelDrawDownLimit      = %.3f;", maxRelDrawDownLimit);
}
//+------------------------------------------------------------------+
namespace Reason
{
    enum Code {
        None,
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
        Size
    };

    string toStr(int r) {
        switch (r) {
            case None:              return "None";
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
            default:                return "<UNKNOWN>";
        }
    }
};
//+------------------------------------------------------------------+
class StatList
{
private:
    double sum_;
    double sumSq_;
    int    last_;

public:
    StatList(): sum_(0), sumSq_(0), last_(-1) {}
    ~StatList() {}

    void push(double val) {
        last_++;
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
        Buy,
        Sell,
        Close,
        Size
    };

private:
    int      cntOp[Operation::Size][Reason::Code::Size];
    StatList profitList[Reason::Code::Size];

public:
    string op2str(Operation op) const {
        switch (op) {
            case Operation::Buy:     return "Buy";
            case Operation::Sell:    return "Sell";
            case Operation::Close:   return "Clse";
            default:                 return "<UNKNOWN>";
        }
    }

    void addOpReason(Operation op, Reason::Code r) {
        cntOp[op][r]++;
    }

    void addProfit(double profit, Reason::Code reason) {
        profitList[reason].push(profit);
    }

    void print() {
        string line = SF("%41s", "");
        for (Operation op = 0; op < Operation::Size; op++) {
            line += SF("%7s", op2str(op));
        }
        LOG("");
        line += "   Profit   avgPrf   stdDev";
        LOG(line);
        string lineDiv;
        StringInit(lineDiv, StringLen(line), '-');
        LOG(lineDiv);
        double sumSumProfit = 0;
        for (int r = 0; r < Reason::Code::Size; r++) {
            line = SF("%40s", Reason::toStr(r));
            for (int op = 0; op < Operation::Size; op++) {
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
class NextTrade
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
    Reason::Code reason;

public:

    NextTrade(): state(State::None) {}
    ~NextTrade() {}

    Reason::Code getReason() { return reason; }

    void set(State newState, Reason::Code newReason, int lineNo = 0) {
        if (state != newState) {
            reason = newReason;
            if (newReason != Reason::None) {
                LOG_LINENO(lineNo, SF("==> %s <== %s (%s)", State2str(newState), State2str(state), Reason2str()));
            }
            state  = newState;
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
        // LOG(SF("Cash: %9.2f + %9.2f = %9.2f", cash, amount, cash + amount));
         cash += amount; }

    double getCash(void)    { return cash; }
    double getBalance(void) { return cash + g::pPos.getTotalPricePaid(); }
    double getEquity(void)  { return cash + g::pPos.getValOfOpenPos(); }
    double getFreeMargin()  { return cash; }

    void log(void) {
        LOG(SF("Open pos value: %9s  balance: %9s  equity: %9s  cash: %9s",
               d2str(g::pPos.getValOfOpenPos()), d2str(g::account.getBalance()), d2str(g::account.getEquity()), d2str(cash)));
    }
};
//+------------------------------------------------------------------+
class MTrade
{
private:
    CTrade c_trade;
    string symbol;
    uint   retcode;

public:
    MTrade(const string &_symbol) : symbol(_symbol) {}
    ~MTrade() {}

    bool ExecuteTrade(bool isBuy, double volume, double price) {
        if (g::account.getCash() > volume * price) {
            if (g::execTradeinMT()) {
                double executionPrice = 0.0;
                double stopLoss = 0.0;
                double takeProfit = 0.0;

                if (isBuy) {
                    return c_trade.Buy(volume, symbol, executionPrice, stopLoss, takeProfit);
                } else {
                    return c_trade.Sell(volume, symbol, executionPrice, stopLoss, takeProfit);
                }
            }
            retcode = TRADE_RETCODE_DONE;
        } else {
            retcode = TRADE_RETCODE_NO_MONEY;
        }
        return retcode == TRADE_RETCODE_DONE;
    }

    bool Buy(double volume, double price) {
        return ExecuteTrade(true, volume, price);
    }

    bool Sell(double volume, double price) {
        return ExecuteTrade(false, volume, price);
    }

    bool PositionClose() {
        if (g::execTradeinMT()) {
            return c_trade.PositionClose(symbol);
        }
        return true;
    }

    string ResultRetcodeDescription() const {
        if (g::execTradeinMT()) {
            return c_trade.ResultRetcodeDescription();
        }
        switch (retcode)
        {
        case TRADE_RETCODE_MARKET_CLOSED:
            return "Market is closed";
        case TRADE_RETCODE_NO_MONEY:
            return "Not enough money";
        case TRADE_RETCODE_DONE:
            return "Done";
        default:
            return "<UNKNOWN>";
        }
    }

    uint ResultRetcode() const {
        if (g::execTradeinMT()) {
            return c_trade.ResultRetcode();
        }
        return retcode;
    }
};
//+------------------------------------------------------------------+
class TradePosition
{
public:
    enum Type {
        None = -1,
        Sell,
        Buy,
    };

private:
    const string  my_symbol;
    const double  maxVolume;

    MTrade  m_Trade;

    Type    posType;
    double  posTotalPricePaid;
    double  posTotalVolume;
    Stats   stats;

    Reason::Code  openReason;

public:
    TradePosition() : my_symbol(Symbol()),
                      maxVolume(SymbolInfoDouble(my_symbol, SYMBOL_VOLUME_MAX)),
                      m_Trade(my_symbol),
                      posType(None), posTotalPricePaid(0.01)
    {}
    ~TradePosition() {
        if (posType != Type::None) {
            close(Reason::None,  (getValOfOpenPos() - getTotalPricePaid()) / g::account.getBalance() * 100);
        }
        stats.print();
        LOG("***********************************");
        LOG(SF("*  Final balance: %13s   *", d2str(g::account.getBalance())));
        LOG(SF("*  Max relative drawdown: %5.2f%%  *", g::maxRelDrawDown * 100));
        LOG("***********************************");
    }

    bool select()                    { return true; /*PositionSelect(my_symbol);*/ }
    int getType()              const { return posType; }
    double getTotalPricePaid() const { return posTotalPricePaid; }

    double lastPrice() {
        return SymbolInfoDouble(my_symbol, SYMBOL_LAST);
    }

    double getValOfOpenPos() {
        if (posType == Type::Buy)  return posTotalVolume * lastPrice();
        if (posType == Type::Sell) return 2 * posTotalPricePaid - posTotalVolume * lastPrice();
        return 0;
    }

    bool checkFreeMarginTradeLimit() {
        return g::account.getFreeMargin() >= g::account.getEquity() * freeMarginTradeLimit / 100;
    }

    void close(Reason::Code closeReason, double profit) {
        LOG(SF("Close, profit: %+.1f%%, %s reason: %s",
                profit,
                posType == Type::Buy ? "Buy" : posType == Type::Sell ? "Sell" : "?",
                Reason::toStr(openReason)));
        g::account.log();

        stats.addOpReason(stats.Close,  openReason);
        stats.addProfit(profit, openReason);

        uint cnt = 0;
        if (g::execTradeinMT()) {
            for (cnt = 0; m_Trade.PositionClose(); cnt++) {
            }
            LOG(SF("Close: %s (%d)", m_Trade.ResultRetcodeDescription(), m_Trade.ResultRetcode()));
        }
        else {
            cnt = 1;
        }
        if (cnt > 0) {
            g::account.addToCash(getValOfOpenPos());
            LOG(SF("CLOSE: %s = %.0f x %.2f", d2str(getValOfOpenPos()), posTotalVolume, lastPrice()));
            posTotalPricePaid = 0.001;
            posTotalVolume    = 0;
            posType           = Type::None;
        }
        g::account.log();
    }

    bool buy(Reason::Code reason) {
        openReason = reason;
        stats.addOpReason(stats.Buy, openReason);

        double freeMarginBeforeTransaction = g::account.getFreeMargin();
        double price = lastPrice();
        double maxTotalVolume = MathFloor(freeMarginBeforeTransaction * freeMarginTradeLimit / 100 / price);
        double volume = MathMin(maxVolume, maxTotalVolume);

        double adjVolume = MathFloor(volume);
        for(posTotalVolume = 0; posTotalVolume < maxTotalVolume;) {
            bool res = m_Trade.Buy(adjVolume, price);
            if (!res) {
                LOG(SF("BUY: %.2f = %.0f x %.2f", adjVolume * price, adjVolume, price));
                LOG(m_Trade.ResultRetcodeDescription());
                break;
            }
            posTotalVolume += adjVolume;
            g::account.addToCash(-adjVolume * price);
            posType = Type::Buy;
            if (checkFreeMarginTradeLimit()) {
                break;
            }
        }
        switch (m_Trade.ResultRetcode()) {
        case TRADE_RETCODE_MARKET_CLOSED:
        case TRADE_RETCODE_NO_MONEY:
            LOG(SF("Buy: %s (%d)", m_Trade.ResultRetcodeDescription(), m_Trade.ResultRetcode()));
            return false;
        }

        posTotalPricePaid = freeMarginBeforeTransaction - g::account.getFreeMargin();
        LOG(SF("BUY: %s = %.0f x %.2f", d2str(posTotalPricePaid), posTotalVolume, price));
        g::account.log();

        return true;
    }

    bool sell(Reason::Code reason) {
        openReason = reason;
        stats.addOpReason(stats.Sell, openReason);

        double freeMarginBeforeTransaction = g::account.getFreeMargin();
        double price = lastPrice();
        double maxTotalVolume = MathFloor(freeMarginBeforeTransaction * freeMarginTradeLimit / 100 / price);
        double volume = MathMin(maxVolume, maxTotalVolume);

        for(posTotalVolume = 0; posTotalVolume < maxTotalVolume;) {
            bool res = m_Trade.Sell(volume, price);
            if (!res) {
                LOG(SF("SELL: %.2f = %.0f x %.2f", volume * price, volume, price));
                LOG(m_Trade.ResultRetcodeDescription());
                break;
            }
            posTotalVolume += volume;
            g::account.addToCash(-volume * price);
            posType = Type::Sell;
            if (checkFreeMarginTradeLimit()) {
                break;
            }
        }
        switch (m_Trade.ResultRetcode()) {
        case TRADE_RETCODE_MARKET_CLOSED:
        case TRADE_RETCODE_NO_MONEY:
            LOG(SF("Sell: %s (%d)", m_Trade.ResultRetcodeDescription(), m_Trade.ResultRetcode()));
            return false;
        }

        posTotalPricePaid = freeMarginBeforeTransaction - g::account.getFreeMargin();
        LOG(SF("SELL: %s = %.0f x %.2f", d2str(posTotalPricePaid), posTotalVolume, price));
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
    NextTrade        nextTrade;
    double           maxRelDrawDown = 0;

    IndicatorList   indicatorList;

    double lastHH;
    double lastMax;
    int    lastMaxTickCnt;
    double lastLL;
    double lastMin;
    int    lastMinTickCnt;

    Account account;

    bool execTradeinMT() { return tradeSize > 0; }
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
void changeDirection(const Reason::Code reason, const int lineNo)
{
    if (g::pPos.getType() == TradePosition::Type::Buy) {
        g::nextTrade.set(NextTrade::State::SellNow, reason, lineNo);
    }
    else if (g::pPos.getType() == TradePosition::Type::Sell) {
        g::nextTrade.set(NextTrade::State::BuyNow, reason, lineNo);
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
                        g::nextTrade.set(NextTrade::State::SellNow, Reason::changeOfSign_neg, __LINE__);
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
                        g::nextTrade.set(NextTrade::State::BuyNow, Reason::changeOfSign_pos, __LINE__);
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
            else if (isMin()) {
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
//        static HHLLlist hhll;
//        hhll.log(tickCnt);
        // p.log(tickCnt);
    }

    double price = g::pPos.lastPrice();
    handleTradeSignals(p, price);
    handleBuySell(p, price);
}
//+------------------------------------------------------------------+
void handleTradeSignals(ProfitEtc& p, const double price)
{
    int len = 0;
    if ((len = g::TR_ST_list.isBuyNow(price)) > 0) {
        // LOG(SF("TRST: BuyNow: %d", len));
    }
    else if ((len = g::TR_ST_list.isSellNow(price)) > 0) {
        // LOG(SF("TRST: SellNow: %d", len));
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
void handleMACDSignals(ProfitEtc &p) {
    static MACD_PeaksAndValleys MACD1peaksAndValleys(g::MACD1);
    MACD1peaksAndValleys.process();

    if (g::MACD1.OsMA_justChangedPositive()) {
        g::nextTrade.set(NextTrade::State::BuyNow, Reason::OsMA_pos, __LINE__);
    } else if (g::MACD1.OsMA_justChangedNegative()) {
//???        g::nextTrade.set(NextTrade::State::SellNow, Reason::OsMA_neg, __LINE__);
    } else if (MACD1peaksAndValleys.is1stPeak()) {
        if (p.profitPerBalance > 0.5) {
            // g::nextTrade.set(NextTrade::State::SellNow, Reason::peakNr1, __LINE__);
        }
    } else if (MACD1peaksAndValleys.is2ndPeak()) {
//???        g::nextTrade.set(NextTrade::State::SellNow, Reason::peakNr2, __LINE__);
    } else if (MACD1peaksAndValleys.is1stValley()) {
        if (p.profitPerBalance > 0.5) {
            // g::nextTrade.set(NextTrade::State::BuyNow, Reason::valleyNr1, __LINE__);
        }
    } else if (MACD1peaksAndValleys.is2ndValley()) {
        g::nextTrade.set(NextTrade::State::BuyNow, Reason::valleyNr2, __LINE__);
    }
}
//+------------------------------------------------------------------+
void handleBuySell(ProfitEtc &p, const double price) {
    if (g::nextTrade.isGetReadyToBuy()) {
        if (g::ATR_list.isBuyNow(price)) {
            g::nextTrade.set(NextTrade::State::BuyNow, Reason::ATR_low, __LINE__);
        }
    }
    else if (g::nextTrade.isGetReadyToSell()) {
        if (g::ATR_list.isSellNow(price)) {
            g::nextTrade.set(NextTrade::State::SellNow, Reason::ATR_high, __LINE__);
        }
    }

    if (g::nextTrade.isBuyNow()) {
        executeTrade(p, TradePosition::Type::Buy, g::nextTrade.getReason());
    }
    else if (g::nextTrade.isSellNow()) {
        executeTrade(p, TradePosition::Type::Sell, g::nextTrade.getReason());
    }
}
//+------------------------------------------------------------------+
void executeTrade(ProfitEtc& p, int type, Reason::Code reason)
{
    if (g::pPos.select()) {
        if (g::pPos.getType() == type) {
            // LOG(type == TradePosition::Type::Buy ? " Already bought" : " Already sold");
            return;
        }
        g::pPos.close(reason, p.profitPerBalance * 100);
    }
    if (type == TradePosition::Type::Buy ? g::pPos.buy(reason) : g::pPos.sell(reason)) {
        g::nextTrade.set(NextTrade::State::None, Reason::None, __LINE__);
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
    //*
    return g::account.getBalance();
    /*/
    return g::account.getBalance() / g::maxRelDrawDown;
    /**/
}
//+------------------------------------------------------------------+

//+------------------------------------------------------------------+
