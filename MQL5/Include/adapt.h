#define input

typedef char* string;
typedef int datetime;
typedef unsigned uint;

const double DBL_MAX = 9999999.999;

string StringFormat    (...);
void SetIndexBuffer    (...);
void IndicatorSetString(...);
void IndicatorRelease  (...);
void Print             (...);
void StringInit        (...);


bool PrintFormat       (...);

int iCustom        (...);
int ArrayResize    (...);
int ArraySize      (...);
int MQLInfoInteger(...);
int CopyRates(...);
int StringLen(...);
int iTime    (...);

datetime TimeCurrent();

char*  Symbol(...);

double MathMax(...);
double MathMin(...);
double MathFloor(...);
double MathSqrt(double);
double SymbolInfoDouble(...);
double AccountInfoDouble(...);

struct MqlRates {
    double open;
    double close;
    double high;
    double low;
};

enum mql5_consts {
    NULL,
    INDICATOR_DATA,
    INDICATOR_COLOR_INDEX,
    INDICATOR_SHORTNAME,
    PERIOD_CURRENT,
    _Symbol,
    _Period,
    SYMBOL_LAST,
    MQL_TESTER,
    ACCOUNT_BALANCE,
    ACCOUNT_EQUITY,
    ACCOUNT_FREEMARGIN,
    TRADE_RETCODE_MARKET_CLOSED,
    TRADE_RETCODE_NO_MONEY,
    TRADE_RETCODE_DONE,
    SYMBOL_VOLUME_MAX
};
