#include "iostm8l152k4.h"

void Clock_Init(void)
{
  /* Clock Setup - Enable LSE */
  CLK_SWCR_bit.SWEN = 1;         // Prepare to change the clock source
  CLK_SWR = 0x08;                // Change the clock source to LSE
  while(CLK_SWCR_bit.SWBSY==1);  // Wait until it starts up
  CLK_CKDIVR = 0x00;             // Change the clock prescaler from default /8 to /1
  CLK_ICKCR_bit.HSION = 0;       
  CLK_ICKCR_bit.LSION = 0;       // Disable internal oscillators
  CLK_PCKENR2_bit.PCKEN27 = 0;   // Disable Boot ROM
}

void RTC_Init(void)
{
  CLK_CRTCR_bit.RTCSEL3   = 1;   // Select LSE as RTC clock source
  CLK_PCKENR2_bit.PCKEN22 = 1;   // Enable RTC clock (also required for LCD to work)

  RTC_WPR = 0xCA;  // Disable write protection to RTC registers
  RTC_WPR = 0x53;
  
  RTC_ISR1_bit.INIT = 1; // Enter initialization mode
  while (RTC_ISR1_bit.INITF == 0);
  RTC_CR1_bit.RATIO = 1; // fSYSCLK = fRTC
  RTC_DR3 = 0;
  RTC_DR2 = 0;
  RTC_DR1 = 0;
  RTC_TR3 = 0;
  RTC_TR2 = 0;
  RTC_TR1 = 0;
  RTC_ISR1_bit.INIT = 0;   // Exit initialization mode      
  RTC_WPR = 0xFF;  // Enable write protection to RTC registers  
}

void LCD_Init(void)
{
  CLK_PCKENR2_bit.PCKEN23 = 1;   // Enable LCD clock

  LCD_CR2_bit.VSEL  = 0;         // LCD voltage source: Internal
  LCD_CR1_bit.B2    = 0;         // LCD bias = 1/3
  LCD_CR1_bit.DUTY0 = 1;         
  LCD_CR1_bit.DUTY1 = 1;         // Duty = 1/4
  
  LCD_FRQ = 3 << 4;              // Set LCD frame rate, 4 << 4 is slowest tolerable

  LCD_PM1 = 0xCF;
  LCD_PM2 = 0xB;                 // Set which pins are used as LCD segment drivers
  
  LCD_CR3_bit.LCDEN = 1;         // Turn on the LCD driver
}

void VREF_Init(void)
{
  while(!PWR_CSR2_bit.VREFINTF); // Wait for voltage reference to stabilize
  
  CLK_PCKENR2_bit.PCKEN25 = 1; // Enable clock for COMP
  COMP_CSR5_bit.VREFTRIG  = 2; // Disable Schmitt trigger on PD7 to reduce leakage
  COMP_CSR3_bit.VREFOUTEN = 1; // Enable VREF output
  RI_IOSR2_bit.CH8E = 1;       // Connect VREFOUT to PD7
  CLK_PCKENR2_bit.PCKEN25 = 0; // Disable clock for COMP, was only needed for init
}

void TIM_Init(void)
{
  /* TIM1 SETUP (SYSCLK/2 GATED BY EXTERNAL SIGNAL) */
  PD_DDR_bit.DDR4 = 1;         // Set PD4 as output (TIM1_CH2)
  PD_CR1_bit.C14 = 1;          // Set PD4 to push-pull mode
  CLK_PCKENR2_bit.PCKEN21 = 1; // Enable clock for TIM1
  
  TIM1_ARRH = 0; 
  TIM1_ARRL = 1;        // Frequency
  TIM1_CCR2H = 0; 
  TIM1_CCR2L = 1;       // Duty cycle
  
  TIM1_BKR_bit.BKE = 1;      // Enable output break
  TIM1_CCMR2_bit.OC2M = 0x6; // Set PWM mode
  TIM1_CR1_bit.CEN = 1;      // Enable counter
  TIM1_CCER1_bit.CC2E = 1;   // CC2 channel enable
  TIM1_BKR_bit.AOE = 1;      // Automatic output enable (to recover from output break)
  
  /* TIM2 SETUP (ONE-PULSE MODE, EXT. TRIGGER) */
  PB_DDR_bit.DDR2 = 1;         // Set PB2 as output (TIM2_CH2)
  PB_CR1_bit.C12 = 1;          // Set PB2 as push-pull
  CLK_PCKENR1_bit.PCKEN10 = 1; // Enable TIM2
  TIM2_ARRH = 0;
  TIM2_ARRL = 6;           // Pulse width = 30.52uS * ARRL
  TIM2_CCR2H = 0;
  TIM2_CCR2L = 1;          
  TIM2_CCMR2_bit.OC2M = 7; // PWM mode 2
  TIM2_CR1_bit.OPM  = 1;   // One-pulse mode
  
  TIM2_SMCR_bit.TS  = 7;   // Trigger source - external
  TIM2_ETR_bit.ETP  = 1;   // Trigger on falling edge
  TIM2_SMCR_bit.SMS = 6;   // Start counting on ext. trigger
  TIM2_CR2_bit.MMS = 2;    // Update event -> trigger output
  
  TIM2_CCER1_bit.CC2E = 1; // Enable channel 2 output
  TIM2_BKR_bit.MOE = 1;    // Enable main output
  
  /* TIM3 SETUP (TIM2 PULSE COUNTER) */
  CLK_PCKENR1_bit.PCKEN11 = 1; // Enable TIM3
  TIM3_SMCR_bit.TS  = 3;       // Trigger source - TIM2
  TIM3_SMCR_bit.SMS = 7;       // Trigger acts as timer clock
  TIM3_CR1_bit.CEN = 1;        // Start the timer
  
  /* TIM4 SETUP (5-SEC INTERRUPTS) */
  CLK_PCKENR1_bit.PCKEN12 = 1; // Enable TIM4
  TIM4_PSCR_bit.PSC = 0xE;     // Div by 16384 -> 0.5 sec resolution
  TIM4_ARR = 9;                // 5 second period (ARR = 2*sec - 1) 
  TIM4_EGR_bit.UG = 1;         // Update timer registers to apply prescaler value (optional)
  TIM4_CR1_bit.CEN = 1;        // Start the timer
}

void IRQ_Init(void)
{
  TIM4_IER_bit.UIE = 1;     // Enable TIM4_UIF interrupt
  PC_CR2_bit.C20 = 1;       // Enable interrupt from PC0 (power loss detect)
  PA_CR2_bit.C22 = 1;       // Enable interrupt from PA2 (mode select)
  PA_CR2_bit.C23 = 1;       // Enable interrupt from PA3 (dose reset)
  EXTI_CR1_bit.P2IS = 2;    // Interrupt on PORTx 2 falling edge
  EXTI_CR1_bit.P3IS = 2;    // Interrupt on PORTx 3 falling edge
  
  WFE_CR1_bit.EXTI_EV0 = 1; // Enable WFE for PORTx 0 interrupts
  WFE_CR1_bit.EXTI_EV2 = 1; // Enable WFE for PORTx 2 interrupts
  WFE_CR1_bit.EXTI_EV3 = 1; // Enable WFE for PORTx 3 interrupts
  WFE_CR3_bit.TIM4_EV  = 1; // Enable WFE for TIM4 interrupts
}
