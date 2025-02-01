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

    ZigZag(const string bufferName, const int depth = 15, const int deviation = 5, const int backstep = 3) :
          Indicator(0, bufferName, iCustom(NULL, PERIOD_CURRENT, "myZigZag", depth, deviation, backstep)),
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
