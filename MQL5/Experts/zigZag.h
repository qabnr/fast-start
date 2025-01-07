//+------------------------------------------------------------------+
//|                                                       zigZag.mqh |
//|                                                            mogyo |
//|                                             http://www.mogyo.com |
//+------------------------------------------------------------------+
#ifndef ZIGZAG_H
#define ZIGZAG_H

#property copyright "mogyo"
#property link      "http://www.mogyo.com"
#property version   "1.00"
//+------------------------------------------------------------------+
#include "indicator.h"
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
    ~ZigZag() {
        // LOGF("");
    }

    bool copyBuffers(const int count) {
        return
            ZigZagBuffer .copy(count) &&
            HighMapBuffer.copy(count) &&
            LowMapBuffer .copy(count);
    }
};
//+------------------------------------------------------------------+
#endif
