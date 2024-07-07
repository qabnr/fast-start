//+------------------------------------------------------------------+
//|                                                        tr_stop.h |
//|                                                            mogyo |
//|                                             http://www.mogyo.com |
//+------------------------------------------------------------------+
#ifndef TR_STOP_H
#define TR_STOP_H

#property copyright "mogyo"
#property link      "http://www.mogyo.com"
#property version   "1.00"
//+------------------------------------------------------------------+
#include "indicator.h"
//+------------------------------------------------------------------+
class TR_STOP : public Indicator
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
    ~TR_STOP() { LOGF(indName); }
  
    TR_STOP(const int lookBackPeriod, const double offset)
        : indName(SF("TR_ST(%d, %.1f)", lookBackPeriod, offset)),
          Indicator(0, SF("TR_ST(%d, %.1f)", lookBackPeriod, offset),
                    iCustom(NULL, PERIOD_CURRENT, "myTR_STOP", lookBackPeriod, offset)),
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
class TR_STOP_List
{
private:
    TR_STOP* trStList[];

public:
    TR_STOP_List() { ArrayResize(trStList, 0, 100); }
    ~TR_STOP_List() { LOGF(""); }

    void add(int lookBackPeriod, double offset) {
        ArrayResize(trStList, ArraySize(trStList) + 1, 100);
        TR_STOP *new_stop = new TR_STOP(lookBackPeriod, offset);
        trStList[ArraySize(trStList)-1] = new_stop;
        g::indicatorList.add(new_stop);
    }

    bool isBuyNow(const double price) const {
        for (int i = 0; i < ArraySize(trStList); i++) {
            if (trStList[i].isBuyNow(price)) return true;
        }
        return false;
    }
    bool isSellNow(const double price) const {
        for (int i = 0; i < ArraySize(trStList); i++) {
            if (trStList[i].isSellNow(price)) return true;
        }
        return false;
    }
};
//+------------------------------------------------------------------+
#endif
