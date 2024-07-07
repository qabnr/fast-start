//+------------------------------------------------------------------+
//|                                                 linRegrChannel.h |
//|                                                            mogyo |
//|                                             http://www.mogyo.com |
//+------------------------------------------------------------------+
#ifndef LIN_REGR_CHANNEL_H
#define LIN_REGR_CHANNEL_H

#property copyright "mogyo"
#property link      "http://www.mogyo.com"
#property version   "1.00"
//+------------------------------------------------------------------+
#include "indicator.h"
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
#endif
