#define input
typedef int string;
typedef int datetime;

const double DBL_MAX = 9999999.999;

string StringFormat    (...);
void SetIndexBuffer    (...);
void IndicatorSetString(...);
void IndicatorRelease  (...);
bool CopyBuffer        (...);
double MathMax(...);
double MathMin(...);

enum mql5_consts {
    INDICATOR_DATA,
    INDICATOR_COLOR_INDEX,
    INDICATOR_SHORTNAME,
};
