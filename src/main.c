#include <stdio.h>
#include "main.h"
#include "init.h"
#include "lcd.h"
#include "apps.h"
#include "iostm8l152k4.h"

#define SAVESIZE 8
#define EEPROM_START 0x1000

uint8_t scr_mode=0;
uint8_t *eeprom_addr; // 0x1000 .. 0x13FF

void load_state(void)
{
  int8_t sec, min, hrs, buf;
  uint16_t ddisp=0; // displayed numbers of dose
  eeprom_addr = (uint8_t *) EEPROM_START;
  
  /* Data unpacking */
  ddisp = *eeprom_addr;                           ++eeprom_addr;
  ddisp |= ((uint16_t)(*eeprom_addr & 0xC0) << 2);
  dd1  = ddisp % 10; ddisp /= 10;
  dd2  = ddisp % 10; ddisp /= 10;
  dd3  = ddisp % 10;
  buf  = *eeprom_addr & 0x3F;
  min  = (buf % 10) | (((buf / 10) << 4) & 0x70); ++eeprom_addr;
  dose =   (uint32_t)(*eeprom_addr);              ++eeprom_addr;
  dose |= ((uint32_t)*eeprom_addr << 8);          ++eeprom_addr;
  buf = *eeprom_addr;                            
  dose |= ((uint32_t)buf << 16);                  ++eeprom_addr;
  if (buf == 0xFF) dose |= ((uint32_t)0xFF << 24); // extend sign
  mode = *eeprom_addr & 0x7;
  buf = (*eeprom_addr >> 3) & 0x1F;               ++eeprom_addr;
  hrs  = (buf % 10) | (((buf / 10) << 4) & 0x70);
  time_days  =  *eeprom_addr;                     ++eeprom_addr;
  time_days |= ((uint16_t)(*eeprom_addr & 0xC0) << 2);
  buf = *eeprom_addr & 0x3F;
  sec  = (buf % 10) | (((buf / 10) << 4) & 0x70);
  
  /* Restore RTC state */
  RTC_WPR = 0xCA;        // Disable write protection to RTC registers
  RTC_WPR = 0x53;
  
  RTC_ISR1_bit.INIT = 1; // Enter initialization mode
  while (RTC_ISR1_bit.INITF == 0);
  RTC_TR3 = hrs;
  RTC_TR2 = min;
  RTC_TR1 = sec;
  RTC_ISR1_bit.INIT = 0;   // Exit initialization mode      
  RTC_WPR = 0xFF;  // Enable write protection to RTC registers  
  
  if (time_days>0) mode_time=1; 
  if (time_days>999) mode_time=2;  // Restore time display mode
}

void shutdown(void)
{
  /* Disable peripherals */
  PD_DDR_bit.DDR4 = 0; // disable boost converter driving pin
  PB_DDR_bit.DDR2 = 0; // disable piezo speaker driving pin
  CLK_PCKENR2_bit.PCKEN23 = 0; 
  CLK_PCKENR2_bit.PCKEN21 = 0; 
  CLK_PCKENR1 = 0;              // disable peripherals
  
  /* To compress data, variables are packed in bit fields with redundant bits removed */
  /* Bit fields are slower than bit arithmetics in IAR compiler */
  uint8_t sec, min, hrs;
  uint16_t ddisp=0; // displayed numbers of dose
  ddisp = dd1 + 10*dd2 + 100*dd3;
  
  sec = RTC_TR1; 
  min = RTC_TR2; 
  hrs = RTC_TR3;
  CLK_PCKENR2_bit.PCKEN22 = 0; // disable RTC
  
  sec = 10*((sec >> 4) & 0x7) + (sec & 0xF);
  min = 10*((min >> 4) & 0x7) + (min & 0xF);
  hrs = 10*((hrs >> 4) & 0x3) + (hrs & 0xF);
  if ((RTC_DR1 > 0) && (time_days < 1000)) time_days++;
  
  /* Save state */
  eeprom_addr = (uint8_t *) EEPROM_START;
    // if EEPROM is locked
  if (!FLASH_IAPSR_bit.DUL)
  { // then unlock EEPROM
    FLASH_DUKR = 0xAE;
    FLASH_DUKR = 0x56;
  }
  while (!FLASH_IAPSR_bit.DUL);
  
  *eeprom_addr++ = (uint8_t) (ddisp);                               
  *eeprom_addr++ = (uint8_t) (min & 0x3F) | ((ddisp >> 2) & 0xC0);  
  *eeprom_addr++ = (uint8_t) (dose);                                
  *eeprom_addr++ = (uint8_t) (dose >> 8);                           
  while (!FLASH_IAPSR_bit.HVOFF); // EEPROM is written in 4-byte blocks
  *eeprom_addr++ = (uint8_t) (dose >> 16);                          
  *eeprom_addr++ = (uint8_t) ((hrs << 3) & 0xF8) | (mode & 0x7);    
  *eeprom_addr++ = (uint8_t) (time_days);                                
  *eeprom_addr   = (uint8_t) (sec & 0x3F) | ((time_days >> 2) & 0xC0);     
  while (!FLASH_IAPSR_bit.HVOFF); // During write, must wait for HVOFF, EOP is for erasing
  
  FLASH_IAPSR_bit.DUL = 0; // lock EEPROM 
  PWR_CSR2_bit.ULP    = 0; 
  asm("HALT");             // shut down remaining peripherals
}   

void reset_state(void)
{
  /* Clear variables */
  dd1=0; dd2=0; dd3=0; dose=0; mode=0;     // reset dose
  dt1=0; dt2=0; dt3=0; dt4=0; time_days=0; // reset time                   
  
  RTC_WPR = 0xCA;        // Disable write protection to RTC registers
  RTC_WPR = 0x53;
  
  RTC_ISR1_bit.INIT = 1; // Enter initialization mode
  while (RTC_ISR1_bit.INITF == 0);
  RTC_DR3 = 0;
  RTC_DR2 = 0;
  RTC_DR1 = 0;
  RTC_TR3 = 0;
  RTC_TR2 = 0;
  RTC_TR1 = 0;
  RTC_ISR1_bit.INIT = 0;   // Exit initialization mode      
  RTC_WPR = 0xFF;  // Enable write protection to RTC registers  
        
  /* Clear EEPROM */
    // if EEPROM is locked
  if (!FLASH_IAPSR_bit.DUL)
  {
    // then unlock EEPROM
    FLASH_DUKR = 0xAE;
    FLASH_DUKR = 0x56;
  }
  while (!FLASH_IAPSR_bit.DUL);
  
  eeprom_addr = (uint8_t *) EEPROM_START;
  for (uint8_t i=0; i<SAVESIZE; i+=4) {
    *(eeprom_addr+i) = 0; 
    *(eeprom_addr+i+1) = 0;
    *(eeprom_addr+i+2) = 0;
    *(eeprom_addr+i+3) = 0;
    while (!FLASH_IAPSR_bit.EOP); // When erasing, it's faster to wait for EOP rather than HVOFF
  }
  
  FLASH_IAPSR_bit.DUL = 0; // lock EEPROM
}
    
int main(void)
{
  Clock_Init();
  VREF_Init();
  TIM_Init();
  RTC_Init();
  LCD_Init();
  IRQ_Init();
  
  PA_CR1_bit.C12 = 1; // Pull up PA2
  PA_CR1_bit.C13 = 1; // Pull up PA3
  PC_CR1 = 0x1E;      // Pull up unused pins
  FLASH_CR1_bit.WAITM = 1;   // Disable Flash memory during wait mode
  FLASH_CR2_bit.WPRG  = 1;   // EEPROM is written in 4-byte blocks
  load_state();
  
  while(1)
  {
    if (EXTI_SR1_bit.P0F == 1) { // Save state and shut down if power is lost
      EXTI_SR1_bit.P0F = 1;
      shutdown();
    }
    
    if (TIM4_SR1_bit.UIF == 1) { // Update rate, dose and time every 5 sec
      TIM4_SR1_bit.UIF = 0;
      app_readGeiger();
      app_updateTime();
    }
    
    if (EXTI_SR1_bit.P2F == 1) { // Switch modes on PA2 button press 
      EXTI_SR1_bit.P2F = 1;
      ++scr_mode;
      if (scr_mode>2) scr_mode=0;
    }
    
    if (EXTI_SR1_bit.P3F == 1) { // Reset dose and time on PA3 button press 
      EXTI_SR1_bit.P3F = 1;
      reset_state();
    }
    
    if (scr_mode==0) print_rate(rad_rate); // radiation rate
    else if (scr_mode==1) print_dose();    // radiation dose
    else print_time();                     // total counting time
    asm("WFE");
  }
}
