#include "iostm8l152k4.h"

// Addition table, bit 7 is carry
const uint8_t LUT[] = {
    0x00,  // 0
    0x01,  // 1
    0x02,  // 2
    0x03,  // 3
    0x04,  // 4
    0x05,  // 5
    0x06,  // 6
    0x07,  // 7
    0x08,  // 8
    0x09,  // 9
    0x80,  // 10
    0x81,  // 11
    0x82,  // 12
    0x83,  // 13
    0x84,  // 14
    0x85,  // 15
    0x86,  // 16
    0x87,  // 17
    0x88,  // 18
    0x89,  // 19
};

// LUT2 and 3 are calculated from 60/k, where k=0.0052 - uSv/h to CPM coefficient
const int32_t LUT2[] = {11538, 115384, 1153846, 11538461};
const uint8_t LUT3[] = {8, 4, 6, 1, 5, 3}; // 60/0.0052 = 1153[846153]

uint16_t cp5s;                 // Registered events during 5 sec period
uint8_t  cpm[3] = {0};         // 24 bit int presented as array
uint16_t cpm_half;
uint16_t rad_rate;

uint16_t event_buf[12] = {0};  // Circular buffer, 12x5 sec periods - 1 min
uint8_t event_idx = 0;         // Index of current buffer element

int32_t dose=0;  
uint8_t dd1=0, dd2=0, dd3=0, dd4, b1, b2, carry;
uint8_t mode=0;  // 6 dose display modes, see notes

int16_t xi=25; // xi =  1/(200*k – 1), where k=0.0052 - uSv/h to CPM coefficient

void app_readGeiger(void) 
{
  /*--------------------------------------
  *   Read CP5S
  *--------------------------------------*/
  
  // Read and clear TIM3 counter
  // TIM3_CNTRH must be read first to prevent rollover
  asm("LD   A, 0x528C"); // 0x528C - TIM3_CNTRH
  asm("CLR  0x528C");
  asm("PUSH 0x528D");    // 0x528D - TIM3_CNTRL
  asm("CLR  0x528D");
  asm("PUSH A");
  asm("LDW  X, (0x01, SP)");
  
  // Apply dead time correction: CP5S = CP5S + (CP5S^2 / 20480)
  // Formula is a polynomial approximation of x = x / (1 - x*38e-6)
  asm("LD   A, #32");
  asm("DIV  X, A");
  asm("LD   A, XL");
  asm("MUL  X, A");
  asm("LD   A, #20");
  asm("DIV  X, A");
  asm("ADDW X, (0x01, SP)");
  asm("POPW Y");
  asm("LDW  cp5s, X");

  /*--------------------------------------
  *   Update radiation rate
  *--------------------------------------*/
  
  // Calculate CPM/2 with a rolling sum
  // Equivalent to:
  // cpm += cp5s - event_buf[event_idx];
  // cpm_half = cpm / 2;
  asm("CLRW  X                ");
  asm("LD    A, event_idx     ");
  asm("SLL   A                "); 
  asm("LD    XL, A            ");
  asm("CLR   A                ");
  asm("LDW   X, (event_buf, X)");
  asm("NEGW  X                "); 
  asm("ADDW  X, cp5s          "); 
  asm("JRPL _zeroA            \n"
      "LD    A, #0xFF         \n"
      "_zeroA:                \n"
      "ADDW  X, cpm + 1       \n"
      "ADC   A, cpm + 0       \n"
      "LDW   cpm + 1, X       \n"
      "LD    cpm + 0, A       \n"
      "SRA   A                \n"
      "RRCW  X                \n"
      "LDW   cpm_half, X      ");
  
  // Update rolling sum buffer
  event_buf[event_idx] = cp5s;
  ++event_idx;
  if (event_idx > 11) event_idx = 0;
  
  // Multiply CPM by 0.52 to find radiation rate
  // CPM * 0.52 = CPM/2 + (CPM/2)/25
  rad_rate = cpm_half + cpm_half/xi; 
  
  /*--------------------------------------
  *   Update radiation dose
  *--------------------------------------*/
  dose += cp5s;
  
  if (mode > 5) return;
  
  else if (mode==0) {
    b2 = (uint16_t)dose / 1153; 
    dose = (uint16_t)dose % 1153;
    b1 = (uint16_t)dose / 115;  
    dose = (uint16_t)dose % 115;
    
    dd1 += b1;         
    carry = LUT[dd1] >> 7; 
    dd1 = LUT[dd1] & 0xF;
    
    dd2 += b2 + carry; 
    carry = LUT[dd2] >> 7; 
    dd2 = LUT[dd2] & 0xF;
    if (carry) {
      dd3 += carry; 
      dose -= LUT3[0];
        
      if (dd3 == 10) {
        dose += dd1*115;                // Compensate for removing d1 from screen
        dd1 = dd2; dd2 = 0; dd3 = 1;    // Draw 1|0|d1
        ++mode;
      }
    }
  }
  
  else if (mode==1) {
    b1 = (uint16_t)dose / 1153; 
    dose = (uint16_t)dose % 1153;
    dd1 += b1; carry = LUT[dd1] >> 7; dd1 = LUT[dd1] & 0xF;
    if (carry) {dd2 += carry; dose -= LUT3[0];} // Compensation for incrementing at 11530 instead of 11538
    carry = LUT[dd2] >> 7; dd2 = LUT[dd2] & 0xF;
    if (carry) {
      dd3 += carry; 
      dose -= LUT3[1];       // Compensation for incrementing at 115380 instead of 115384
      
      if (dd3 == 10) {
        while(dd1) {dose += 1153; --dd1;}
        dd1 = dd2; dd2 = 0; dd3 = 1;
        ++mode;
      }
    } 
  }
  
  else {
    if (dose >= LUT2[mode-2]) {
      dose -= LUT2[mode-2];
      ++dd1; carry = LUT[dd1] >> 7; dd1 = LUT[dd1] & 0xF;
      if (carry) {dd2 += carry; dose -= LUT3[mode-1];}
      carry = LUT[dd2] >> 7; dd2 = LUT[dd2] & 0xF;
      if (carry) {
        dd3 += carry; 
        dose -= LUT3[mode];
        
        if (dd3 == 10) {
          dd1 = dd2; dd2 = 0; dd3 = 1;
          ++mode;
        }
      }
    }
  }
}

/*---------------------------------------------
* Function to update total dose counting time
*---------------------------------------------*/

uint16_t time_days=0; // 0..999 
      
void app_updateTime(void)
{
  uint16_t days_buf;
  uint8_t  sec, min, hrs, day;

  if (mode_time==2) return;
  
  else if (mode_time == 1) day = RTC_DR1 & 0x0F;
  else {
    sec = RTC_TR1; min = RTC_TR2; hrs = RTC_TR3; day = RTC_DR1; // Time/date registers are locked when RTC_TR1 is read
  }

  volatile uint8_t dummy = RTC_DR3;                             // Time/date registers are unlocked when RTC_DR3 is read
  (void) dummy;

  if (day > 0) {
    mode_time=1;
    RTC_WPR = 0xCA;  // Disable write protection to RTC registers
    RTC_WPR = 0x53;
    RTC_ISR1_bit.INIT = 1; // Enter initialization mode
    while (RTC_ISR1_bit.INITF == 0);
    RTC_DR1 = 0;     // Clear date register
    RTC_ISR1_bit.INIT = 0;   // Exit initialization mode  
    RTC_WPR = 0xFF;  // Enable write protection to RTC registers
    
    ++time_days;
    if (time_days >= 999) mode_time=2;
  }

 if (mode_time==1) {
    days_buf = time_days;
    dt4 = 3;             // print letter 'd'
    dt3 = days_buf % 10;
    days_buf /= 10;
    dt2 = days_buf % 10;
    days_buf /= 10;
    dt1 = days_buf % 10;
    days_buf /= 10;
  }

  else if (hrs > 0) { // if <1 day
    dt1 = (hrs & 0x30) >> 4;
    dt2 =  hrs & 0x0F;
    dt3 = (min & 0xF0) >> 4;
    dt4 =  min & 0x0F;
  }

  else { // if <1 hour

    dt1 = (min & 0xF0) >> 4;
    dt2 =  min & 0x0F;
    dt3 = (sec & 0xF0) >> 4;
    dt4 =  sec & 0x0F;
  }
}
