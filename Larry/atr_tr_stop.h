//+------------------------------------------------------------------+
//|                                                  atr_tr_stop.mqh |
//|                                                            mogyo |
//|                                             http://www.mogyo.com |
//+------------------------------------------------------------------+
#ifndef ATR_TR_STOP_H
#define ATR_TR_STOP_H

#property copyright "mogyo"
#property link      "http://www.mogyo.com"
#property version   "1.00"
//+------------------------------------------------------------------+
#include "indicator.h"
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
        for (int i = 0; i < ArraySize(atrTrStList); i++) {
            if (atrTrStList[i].isBuyNow(price)) return true;
        }
        return false;
    }
    bool isSellNow(const double price) const {
        for (int i = 0; i < ArraySize(atrTrStList); i++) {
            if (atrTrStList[i].isSellNow(price)) return true;
        }
        return false;
    }
};
//+------------------------------------------------------------------+
#endif
