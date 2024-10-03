//+------------------------------------------------------------------+
//|                                                  iustom_test.mq5 |
//|                                                               GM |
//|                                             https://www.mql5.com |
//+------------------------------------------------------------------+
#property copyright "GM"
#property link      "https://www.gm.com"
#property version   "1.00"
#property indicator_separate_window 
#property indicator_buffers 1 
#property indicator_plots   1 
//---- plot Label1 
#property indicator_label1  "Label1" 
#property indicator_type1   DRAW_LINE 
#property indicator_color1  clrRed 
#property indicator_style1  STYLE_SOLID 
#property indicator_width1  1 

//--- input parameters 
input int MA_Period=21; 
input int MA_Shift=0; 
input ENUM_MA_METHOD MA_Method=MODE_SMA; 

//--- indicator buffers 
double         Label1Buffer[]; 
//--- Handle of the Custom Moving Average.mq5 custom indicator 
int MA_handle; 
//+------------------------------------------------------------------+ 
//| Custom indicator initialization function                         | 
//+------------------------------------------------------------------+ 
int OnInit() 
  { 
//--- indicator buffers mapping 
   SetIndexBuffer(0,Label1Buffer,INDICATOR_DATA); 
   ResetLastError(); 
   MA_handle=iCustom(NULL,0,"Examples\\Custom Moving Average", 
                     MA_Period, 
                     MA_Shift, 
                     MA_Method, 
                     PRICE_CLOSE // using the close prices 
                     ); 
   Print("MA_handle = ",MA_handle,"  error = ",GetLastError()); 
//--- 
   return(INIT_SUCCEEDED); 
  } 
//+------------------------------------------------------------------+ 
//| Custom indicator iteration function                              | 
//+------------------------------------------------------------------+ 
int OnCalculate(const int rates_total, 
                const int prev_calculated, 
                const datetime &time[], 
                const double &open[], 
                const double &high[], 
                const double &low[], 
                const double &close[], 
                const long &tick_volume[], 
                const long &volume[], 
                const int &spread[]) 
  {
    if (rates_total <= prev_calculated) { return (rates_total); }
//--- Copy the values of the indicator Custom Moving Average to our indicator buffer 
   int copy=CopyBuffer(MA_handle,0,0,rates_total,Label1Buffer); 
   Print("copy = ",copy,"    rates_total = ",rates_total); 
//--- If our attempt has failed - Report this 
   if(copy<=0) 
      Print("An attempt to get the values if Custom Moving Average has failed"); 
//--- return value of prev_calculated for next call 
   return(rates_total); 
  } 
//+------------------------------------------------------------------+
