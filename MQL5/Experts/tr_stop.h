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
    Buffer *buyBuffer;
    Buffer buyColorBuffer;

    Buffer sellBuffer;
    Buffer sellColorBuffer;

    Buffer stopBuffer;
    Buffer stopColorBuffer;

    string indName;

public:
    ~TR_STOP() {
        // LOGF(indName);
    }
  
    TR_STOP(const int lookBackPeriod, const double offset)
        : indName(SF("TR_ST(%d, %.1f)", lookBackPeriod, offset)),
          Indicator(0, SF("TR_ST(%d, %.1f)", lookBackPeriod, offset),
                    iCustom(NULL, PERIOD_CURRENT, "myTR_STOP", lookBackPeriod, offset)),
          buyBuffer      (  &buffer),
          buyColorBuffer (1, buffer),
          sellBuffer     (2, buffer),
          sellColorBuffer(3, buffer),
          stopBuffer     (4, buffer),
          stopColorBuffer(5, buffer)
    {}

    bool copyBuffers(const int count) {
        return
           stopBuffer.copy(count) && stopColorBuffer.copy(count)
        && buyBuffer .copy(count) && buyColorBuffer .copy(count)
        && sellBuffer.copy(count) && sellColorBuffer.copy(count);
    }

    int isBuyNow(const double price) const {
        if (price < stopBuffer.get(1)) return -1;
        if (price > stopBuffer.get(2)) return -1;
        
        const int size = stopBuffer.getSize();
        int cnt = 0;
        for (int i = 2; i < size; i++) {
            if (buyBuffer.get(i) != stopBuffer.get(i)) break;
            cnt++;
        }
// LOG(SF("%s: BuyNow: %d", indName, cnt));
        return cnt;
    }

    int isSellNow(const double price) const {
        if (price > stopBuffer.get(1)) return -1;
        if (price < stopBuffer.get(2)) return -1;
        
        const int size = stopBuffer.getSize();
        int cnt = 0;
        for (int i = 2; i < size; i++) {
            if (sellBuffer.get(i) != stopBuffer.get(i)) break;
            cnt++;
        }
// LOG(SF("%s: SellNow: %d", indName, cnt));
        return cnt;
    }
};
//+------------------------------------------------------------------+
class TR_STOP_SIGNAL : public Indicator
{
private:
    Buffer *diff_buffer;
    Buffer len_buffer;
    Buffer comb_buffer;

    string indName;

public:
    ~TR_STOP_SIGNAL() {
        // LOGF(indName);
    }
  
    TR_STOP_SIGNAL(const int lookBackPeriod, const double priceOffset, const double weight)
        : indName(SF("TRSTS(%d, %.2f)", lookBackPeriod, priceOffset, weight)),
          Indicator(0, SF("TRSTS(%d, %.2f)", lookBackPeriod, priceOffset, weight),
                    iCustom(NULL, PERIOD_CURRENT, "myTR_STOP_Signal", lookBackPeriod, priceOffset, weight)),
          diff_buffer(  &buffer),
          len_buffer (1, buffer),
          comb_buffer(2, buffer)
    {}

    bool copyBuffers(const int count) {
        return
           diff_buffer.copy(count) && len_buffer.copy(count) && comb_buffer .copy(count);
    }
};
//+------------------------------------------------------------------+
class TR_STOP_List
{
private:
    TR_STOP* trStList[];

public:
    TR_STOP_List() { ArrayResize(trStList, 0, 100); }
    ~TR_STOP_List() {
        // LOGF("");
    }

    void add(int lookBackPeriod, double offset) {
        ArrayResize(trStList, ArraySize(trStList) + 1, 100);
        TR_STOP *new_stop = new TR_STOP(lookBackPeriod, offset);
        trStList[ArraySize(trStList)-1] = new_stop;
        g::indicatorList.add(new_stop);
    }

    int isBuyNow(const double price) const {
        int minLen = 999999;
        for (int i = 0; i < ArraySize(trStList); i++) {
            int len = trStList[i].isBuyNow(price);
            if (len < 0) return -1;
            minLen = MathMin(minLen, len);
        }
        return minLen;
    }
    int isSellNow(const double price) const {
        int minLen = 999999;
        for (int i = 0; i < ArraySize(trStList); i++) {
            int len = trStList[i].isSellNow(price);
            if (len < 0) return -1;
            minLen = MathMin(minLen, len);
        }
        return minLen;
    }
};
//+------------------------------------------------------------------+
#endif
