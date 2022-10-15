#include "iostm8l152k4.h"

void print_dose(void);

const uint8_t NUM1_2[] = {0x0A, 0x00, 0x02, 0x02, 0x08, 0x0A, 0x0A, 0x02, 0x0A, 0x0A};
const uint8_t NUM1_5[] = {0x20, 0x20, 0xA0, 0xA0, 0xA0, 0x80, 0x80, 0x20, 0xA0, 0xA0};
const uint8_t NUM1_9[] = {0x0A, 0x02, 0x08, 0x02, 0x02, 0x02, 0x0A, 0x02, 0x0A, 0x02};
const uint8_t NUM1_C[] = {0x80, 0x00, 0x80, 0x80, 0x00, 0x80, 0x80, 0x00, 0x80, 0x80};

const uint8_t NUM2_1[] = {0x80, 0x00, 0x80, 0x80, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80};
const uint8_t NUM2_2[] = {0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01};
const uint8_t NUM2_5[] = {0x08, 0x08, 0x18, 0x18, 0x18, 0x10, 0x10, 0x08, 0x18, 0x18};
const uint8_t NUM2_8[] = {0x80, 0x80, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
const uint8_t NUM2_9[] = {0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00};
const uint8_t NUM2_C[] = {0x10, 0x00, 0x10, 0x10, 0x00, 0x10, 0x10, 0x00, 0x10, 0x10};

const uint8_t NUM3_1[] = {0x48, 0x00, 0x08, 0x08, 0x40, 0x48, 0x48, 0x08, 0x48, 0x48};
const uint8_t NUM3_4[] = {0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x80};
const uint8_t NUM3_5[] = {0x00, 0x00, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x04};
const uint8_t NUM3_8[] = {0x48, 0x08, 0x40, 0x08, 0x08, 0x08, 0x48, 0x08, 0x48, 0x08};
const uint8_t NUM3_C[] = {0x04, 0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00, 0x04, 0x04};

const uint8_t NUM4_1[] = {0x06, 0x00, 0x02, 0x02, 0x04, 0x06, 0x06, 0x02, 0x06, 0x06};
const uint8_t NUM4_4[] = {0x20, 0x20, 0x60, 0x60, 0x60, 0x40, 0x40, 0x20, 0x60, 0x60};
const uint8_t NUM4_8[] = {0x06, 0x02, 0x04, 0x02, 0x02, 0x02, 0x06, 0x02, 0x06, 0x02};
const uint8_t NUM4_B[] = {0x40, 0x00, 0x40, 0x40, 0x00, 0x40, 0x40, 0x00, 0x40, 0x40};

                       //  u     m     S     d     L
const uint8_t SYM4_1[] = {0x00, 0x02, 0x06, 0x00, 0x04};
const uint8_t SYM4_4[] = {0x00, 0x40, 0x00, 0x60, 0x00};
const uint8_t SYM4_8[] = {0x06, 0x06, 0x02, 0x06, 0x04};
const uint8_t SYM4_B[] = {0x40, 0x00, 0x40, 0x40, 0x40};

const uint8_t DOT11[]  = {0x00, 0x80};
const uint8_t DOT12[]  = {0x08, 0x00};

/*--------------------------------------
* Function to print out of range message
*--------------------------------------*/
void print_oL (void)
{
  LCD_RAM2  = 0;                 
  LCD_RAM9  = 0;                 
  LCD_RAM5  = NUM3_5[0];   
  LCD_RAM12 = NUM3_C[0];
  LCD_RAM1  = SYM4_1[4] | NUM3_1[0];   
  LCD_RAM8  = SYM4_8[4] | NUM3_8[0];   
  LCD_RAM4  = SYM4_4[4] | NUM3_4[0];   
  LCD_RAM11 = SYM4_B[4];       
}

/*--------------------------------------
* Function to print radiation rate
*--------------------------------------*/
uint8_t d1, d2, d3, d4, dot;
void print_rate(uint16_t value)
{
  if (value > 9999) {
    value /= 10;
    dot = 1;
  }
  else dot = 0;
  
  d4 = value % 10;
  value /= 10;
  d3 = value % 10;
  value /= 10;
  d2 = value % 10;
  value /= 10;
  d1 = value % 10;
  value /= 10;
  
  LCD_RAM2  = NUM1_2[d1] | NUM2_2[d2];                
  LCD_RAM9  = NUM1_9[d1] | NUM2_9[d2];               
  LCD_RAM5  = NUM1_5[d1] | NUM2_5[d2] | NUM3_5[d3];   
  LCD_RAM12 = NUM1_C[d1] | NUM2_C[d2] | NUM3_C[d3] | DOT12[dot];   
  LCD_RAM1  = NUM4_1[d4] | NUM2_1[d2] | NUM3_1[d3];  
  LCD_RAM8  = NUM4_8[d4] | NUM2_8[d2] | NUM3_8[d3];   
  LCD_RAM4  = NUM4_4[d4]              | NUM3_4[d3];   
  LCD_RAM11 = NUM4_B[d4] | DOT11[dot];                            
}


/*--------------------------------------
* Function to print radiation dose
*--------------------------------------*/
extern uint8_t dd1, dd2, dd3, dd4, mode;
void print_dose(void)
{
  if (mode==0) {
    dot = 0x20;  // set dot position
    dd4 = 0;     // 0 = 'u', 1 = 'm' letter
  }
  else if (mode==1) {
    dot = 0x08;
    dd4 = 0;
  }
  else if (mode==2) {
    dot = 0x00;
    dd4 = 0;
  }
  else if (mode==3) {
    dot = 0x20;
    dd4 = 1;                
  }
  else if (mode==4) {
    dot = 0x08;
    dd4 = 1;
  }
  else {
    print_oL();
    return;
  }
  
  LCD_RAM2  = NUM1_2[dd3] | NUM2_2[dd2];                 
  LCD_RAM9  = NUM1_9[dd3] | NUM2_9[dd2];                 
  LCD_RAM5  = NUM1_5[dd3] | NUM2_5[dd2] | NUM3_5[dd1];   
  LCD_RAM12 = NUM1_C[dd3] | NUM2_C[dd2] | NUM3_C[dd1] | dot;
  LCD_RAM1  = SYM4_1[dd4] | NUM2_1[dd2] | NUM3_1[dd1];   
  LCD_RAM8  = SYM4_8[dd4] | NUM2_8[dd2] | NUM3_8[dd1];   
  LCD_RAM4  = SYM4_4[dd4]               | NUM3_4[dd1];   
  LCD_RAM11 = SYM4_B[dd4];                               
}


/*-----------------------------------------
* Function to print total measurement time
*-----------------------------------------*/
uint8_t dt1, dt2, dt3, dt4;
uint8_t mode_time=0;  // hours/minutes, days or overflow

void print_time(void)
{
  if (mode_time == 2){
    print_oL();
  }
  
  else if (mode_time == 1) {
    LCD_RAM2  = NUM1_2[dt1] | NUM2_2[dt2];                 
    LCD_RAM9  = NUM1_9[dt1] | NUM2_9[dt2];                 
    LCD_RAM5  = NUM1_5[dt1] | NUM2_5[dt2] | NUM3_5[dt3];   
    LCD_RAM12 = NUM1_C[dt1] | NUM2_C[dt2] | NUM3_C[dt3];   
    LCD_RAM1  = SYM4_1[dt4] | NUM2_1[dt2] | NUM3_1[dt3];   
    LCD_RAM8  = SYM4_8[dt4] | NUM2_8[dt2] | NUM3_8[dt3];   
    LCD_RAM4  = SYM4_4[dt4]               | NUM3_4[dt3];   
    LCD_RAM11 = SYM4_B[dt4];                               
  }
  
  else {
    LCD_RAM2  = NUM1_2[dt1] | NUM2_2[dt2];                 
    LCD_RAM9  = NUM1_9[dt1] | NUM2_9[dt2];                 
    LCD_RAM5  = NUM1_5[dt1] | NUM2_5[dt2] | NUM3_5[dt3];   
    LCD_RAM12 = NUM1_C[dt1] | NUM2_C[dt2] | NUM3_C[dt3];   
    LCD_RAM1  = NUM4_1[dt4] | NUM2_1[dt2] | NUM3_1[dt3];   
    LCD_RAM8  = NUM4_8[dt4] | NUM2_8[dt2] | NUM3_8[dt3];   
    LCD_RAM4  = NUM4_4[dt4]               | NUM3_4[dt3];   
    LCD_RAM11 = NUM4_B[dt4];                               
    LCD_RAM11_bit.S309 = 1; // colon
  }
}
