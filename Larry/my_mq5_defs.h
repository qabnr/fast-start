typedef unsigned int  uint;
typedef unsigned long ulong;
struct string  { int x; };
const int NULL = 0;

enum ENUM_TRADE_REQUEST_ACTIONS
{
    TRADE_ACTION_DEAL,     // Place a trade order for an immediate execution with the specified parameters (market order)
    TRADE_ACTION_PENDING,  // Place a trade order for the execution under specified conditions (pending order)
    TRADE_ACTION_SLTP,     // Modify Stop Loss and Take Profit values of an opened position
    TRADE_ACTION_MODIFY,   // Modify the parameters of the order placed previously
    TRADE_ACTION_REMOVE,   // Delete the pending order placed previously
    TRADE_ACTION_CLOSE_BY, // Close a position by an opposite one
};

enum ENUM_ORDER_TYPE {
    ORDER_TYPE_BUY,             // Market Buy order
    ORDER_TYPE_SELL,            // Market Sell order
    ORDER_TYPE_BUY_LIMIT,       // Buy Limit pending order
    ORDER_TYPE_SELL_LIMIT,      // Sell Limit pending order
    ORDER_TYPE_BUY_STOP,        // Buy Stop pending order
    ORDER_TYPE_SELL_STOP,       // Sell Stop pending order
    ORDER_TYPE_BUY_STOP_LIMIT,  // Upon reaching the order price, a pending Buy Limit order is placed at the StopLimit price
    ORDER_TYPE_SELL_STOP_LIMIT, // Upon reaching the order price, a pending Sell Limit order is placed at the StopLimit price
    ORDER_TYPE_CLOSE_BY,        // Order to close a position by an opposite one
};

enum ENUM_ORDER_TYPE_FILLING {
    ORDER_FILLING_FOK, /*
Fill or Kill
	

An order can be executed in the specified volume only.

 

If the necessary amount of a financial instrument is currently unavailable in the market, the order will not be executed.

 

The desired volume can be made up of several available offers.

 

The possibility of using FOK orders is determined at the trade server.
	
*/

    ORDER_FILLING_IOC, /*
Immediate or Cancel
	

A trader agrees to execute a deal with the volume maximally available in the market within that indicated in the order.

 

If the request cannot be filled completely, an order with the available volume will be executed, and the remaining volume will be canceled.

 

The possibility of using IOC orders is determined at the trade server.

*/ 
	
    ORDER_FILLING_BOC,


/* Passive (Book or Cancel)
	

The BoC order assumes that the order can only be placed in the Depth of Market and cannot be immediately executed. If the order can be executed immediately when placed, then it is canceled.

 

In fact, the BOC policy guarantees that the price of the placed order will be worse than the current market. BoC orders are used to implement passive trading, so that the order is not executed immediately when placed and does not affect current liquidity.

 

Only limit and stop limit orders are supported (ORDER_TYPE_BUY_LIMIT, ORDER_TYPE_SELL_LIMIT, ORDER_TYPE_BUY_STOP_LIMIT, ORDER_TYPE_SELL_STOP_LIMIT).
	 */

    ORDER_FILLING_RETURN,
/* Return
	

In case of partial filling, an order with remaining volume is not canceled but processed further.

 

Return orders are not allowed in the Market Execution mode (market execution — SYMBOL_TRADE_EXECUTION_MARKET).
 */	

}


struct MqlTradeRequest
  {
   ENUM_TRADE_REQUEST_ACTIONS    action;           // Trade operation type
   ulong                         magic;            // Expert Advisor ID (magic number)
   ulong                         order;            // Order ticket
   string                        symbol;           // Trade symbol
   double                        volume;           // Requested volume for a deal in lots
   double                        price;            // Price
   double                        stoplimit;        // StopLimit level of the order
   double                        sl;               // Stop Loss level of the order
   double                        tp;               // Take Profit level of the order
   ulong                         deviation;        // Maximal possible deviation from the requested price
   ENUM_ORDER_TYPE               type;             // Order type
   ENUM_ORDER_TYPE_FILLING       type_filling;     // Order execution type
   ENUM_ORDER_TYPE_TIME          type_time;        // Order expiration type
   datetime                      expiration;       // Order expiration time (for the orders of ORDER_TIME_SPECIFIED type)
   string                        comment;          // Order comment
   ulong                         position;         // Position ticket
   ulong                         position_by;      // The ticket of an opposite position
  };

  struct MqlTradeResult
  {
   uint     retcode;          // Operation return code
   ulong    deal;             // Deal ticket, if it is performed
   ulong    order;            // Order ticket, if it is placed
   double   volume;           // Deal volume, confirmed by broker
   double   price;            // Deal price, confirmed by broker
   double   bid;              // Current Bid price
   double   ask;              // Current Ask price
   string   comment;          // Broker comment to operation (by default it is filled by description of trade server return code)
   uint     request_id;       // Request ID set by the terminal during the dispatch 
   int      retcode_external; // Return code of an external trading system
  };


  struct MqlDateTime
  {
   int year;           // Year
   int mon;            // Month
   int day;            // Day
   int hour;           // Hour
   int min;            // Minutes
   int sec;            // Seconds
   int day_of_week;    // Day of week (0-Sunday, 1-Monday, ... ,6-Saturday)
   int day_of_year;    // Day number of the year (January 1st is assigned the number value of zero)
  };


enum ENUM_MA_METHOD {
    MODE_SMA,   // Simple averaging
    MODE_EMA,   // Exponential averaging
    MODE_SMMA,  // Smoothed averaging
    MODE_LWMA,  // Linear-weighted averaging
};


enum ENUM_APPLIED_PRICE {
    PRICE_CLOSE,    // Close price
    PRICE_OPEN,     // Open price
    PRICE_HIGH,     // The maximum price for the period
    PRICE_LOW,      // The minimum price for the period
    PRICE_MEDIAN,   // Median price, (high + low)/2
    PRICE_TYPICAL,  // Typical price, (high + low + close)/3
    PRICE_WEIGHTED, // Average price, (high + low + close + close)/4
};

int iMA(string               symbol,            // symbol name
        ENUM_TIMEFRAMES      period,            // period
        int                  ma_period,         // averaging period
        int                  ma_shift,          // horizontal shift
        ENUM_MA_METHOD       ma_method,         // smoothing type
        ENUM_APPLIED_PRICE   applied_price      // type of price or handle
   );
int iCustom(string           symbol,     // symbol name
            ENUM_TIMEFRAMES  period,     // period
            string           name        // folder/custom_indicator_name
            ...                          // list of indicator input parameters
   );
void  ZeroMemory(void & variable      // reset variable
   );
bool  OrderSend(MqlTradeRequest&  request,      // query structure
                MqlTradeResult&   result        // structure of the answer
   );

const int Buffer_0   = 0;
const int Buffer_1   = 1;
const int StartPos_0 = 0;
int  CopyBuffer(int       indicator_handle,     // indicator handle
                int       buffer_num,           // indicator buffer number
                int       start_pos,            // start position
                int       count,                // amount to copy
                double    buffer[]              // target array to copy
   );
int    OrdersTotal(void);
ulong  OrderGetTicket(int  index      // Number in the list of orders);
   