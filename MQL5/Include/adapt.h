#define input


typedef char* string;
typedef int datetime;
typedef unsigned uint

const double DBL_MAX = 9999999.999;

string StringFormat    (...);
void SetIndexBuffer    (...);
void IndicatorSetString(...);
void IndicatorRelease  (...);
void Print             (...);
void Print             (...);
bool PrintFormat       (...);

int iCustom        (...);
int ArrayResize    (...);
int ArraySize      (...);
char*  Symbol(...);

double MathMax(...);
double MathMin(...);
double MathSqrt(double);
double SymbolInfoDouble(...);

enum mql5_consts {
    NULL,
    INDICATOR_DATA,
    INDICATOR_COLOR_INDEX,
    INDICATOR_SHORTNAME,
    PERIOD_CURRENT,
    _Symbol,
    SYMBOL_LAST,
};
