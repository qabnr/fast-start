//+------------------------------------------------------------------+
//|                                                         macd.mqh |
//|                                                            mogyo |
//|                                             http://www.mogyo.com |
//+------------------------------------------------------------------+
#ifndef MACD_H
#define MACD_H
#property copyright "mogyo"
#property link      "http://www.mogyo.com"
#property version   "1.00"
//+------------------------------------------------------------------+
#include "indicator.h"
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
        if (handle == INVALID_HANDLE) {
            Print("Failed to get indicator handle");
        }
    }

    virtual ~MACD_base() {
        // LOGF("");
    };

    virtual bool copyBuffers(const int count) {
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
    {}
    ~MACD() {
        // LOGF("");
    }

    bool copyBuffers(const int count) {
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

    ~myMACD2() {
        // LOGF("");
    }
};
//+------------------------------------------------------------------+
#endif
