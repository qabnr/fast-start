//+------------------------------------------------------------------+
//|                                                    indicator.mqh |
//|                                                            mogyo |
//|                                             http://www.mogyo.com |
//+------------------------------------------------------------------+
#ifndef INDICATOR_H
#define INDICATOR_H
#property copyright "mogyo"
#property link      "http://www.mogyo.com"

#include <adapt.h>
#include <utils.h>
//+------------------------------------------------------------------+
class Buffer
{
private:
    string  indicatorName;
    int     handle;
    int     buffNum;
    double  buff[];
    int     nrCopied;

    void init() {
        ArraySetAsSeries(buff, true);
        LOG(SF("New buffer: %d, handle: %d, indicator: %s", buffNum, handle, indicatorName));
        if (handle == INVALID_HANDLE) {
            Print("Failed to get handle for buffer: ", buffNum, ", indicator: ", indicatorName);
        }
    }

public:
    Buffer(const int _buffNumber, const Buffer &otherBuffer)
        : indicatorName(otherBuffer.indicatorName), handle(otherBuffer.handle), buffNum(_buffNumber), nrCopied(0)
    {
        init();
    }
    Buffer(const int _buffNumber, const string _indicatorName, const int _handle)
        : indicatorName(_indicatorName), handle(_handle), buffNum(_buffNumber), nrCopied(0)
    {
        init();
    }
    ~Buffer() { LOGF(SF("buffer: %d, handle: %d, indicator: %s", buffNum, handle, indicatorName)); };

    int getHandle() const {
        return handle;
    }

    string getName() const {
        return indicatorName;
    }

    bool copy(const int count) {
        nrCopied = CopyBuffer(handle, buffNum, 0, count, buff);
        if (nrCopied <= 0) {
            Print("Failed to copy data from buffer: ", buffNum, " handle: ", indicatorName, " (", handle, ")");
        }
        return nrCopied > 0;
    }
    double get(const int index) const {
        return buff[index];
    }
    int getNrCopied() const {
        return nrCopied;
    }
};
//+------------------------------------------------------------------+
class Indicator {
protected:
    int    handle;
public:
    Buffer buffer;

    Indicator(int _buffNumber, string _indicatorName, const int _handle)
        : handle(_handle),
          buffer(_buffNumber, _indicatorName, _handle)
    {}
    virtual ~Indicator() { LOGF(""); };

    string getName() const {
        return buffer.getName();
    }

    virtual bool copyBuffers(const int count) {
        return buffer.copy(count);
    }
};
//+------------------------------------------------------------------+
class IndicatorList {
private:
    Indicator* list[];

public:
    IndicatorList() {}
    ~IndicatorList() {
        for (int i = 0; i < ArraySize(list); i++) {
            delete list[i];
        }
    }

    void add(Indicator* indicator) {
        LOG(SF("IndLst: adding %s", indicator.getName()));
        ArrayResize(list, ArraySize(list) + 1, 100);
        list[ArraySize(list)-1] = indicator;
    }

    bool copyBuffers(const int count) {
        int len = ArraySize(list);
        for (int i = 0; i < len; i++) {
            if (!list[i].copyBuffers(count)) {
                return false;
            }
        }
        return true;
    } 
};
//+------------------------------------------------------------------+
#endif
