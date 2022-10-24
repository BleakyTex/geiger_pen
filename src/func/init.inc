  ; <-- Clock Setup - Enable LSE

  BSET CLK_SWCR, #1;    CLK_SWCR_bit.SWEN = 1;   // Prepare to change clk source
  MOV CLK_SWR, #0x08;   CLK_SWR = 0x08;          // Change clk source to LSE

  BTJT CLK_SWCR, #0, $; while (CLK_SWCR_bit.SWBSY); // Wait until it starts up

  CLR CLK_CKDIVR;       CLK_CKDIVR = 0x00;       // Drop prescaler from /8 to /1
  BRES CLK_ICKCR, #2;   CLK_ICKCR_bit.HSION = 0; // Disable internal oscillators
  BRES CLK_ICKCR, #0;   CLK_ICKCR_bit.LSION = 0;
  BRES CLK_PCKENR2, #7; CLK_PCKENR2_bit.PCKEN27 = 0;// Disable Boot ROM


  ; <-- VREF_Init

  BTJF PWR_CSR2, #0, $; while (!PWR_CSR2_bit.VREFINTF); // Wait for stable vref

  BSET CLK_PCKENR2, #5; CLK_PCKENR2_bit.PCKEN25 = 1; // Enable clock for COMP
                    ;   // Disable Schmitt trigger on PD7 to reduce leakage
  BSET COMP_CSR5, #1;   COMP_CSR5_bit.VREFTRIG  = 2;
  BSET COMP_CSR3, #0;   COMP_CSR3_bit.VREFOUTEN = 1; // Enable VREF output
  BSET RI_IOSR2, #2;    RI_IOSR2_bit.CH8E = 1;       // Connect VREFOUT to PD7
  BRES CLK_PCKENR2, #5; CLK_PCKENR2_bit.PCKEN25 = 0; // Disable clock for COMP


  ; <-- TIM1 SETUP (SYSCLK/2 GATED BY EXTERNAL SIGNAL)

  BSET PD_DDR, #4;      PD_DDR_bit.DDR4 = 1;         // PD4 as output (TIM1_CH2)
  BSET PD_CR1, #4;      PD_CR1_bit.C14 = 1;          // Push-pull mode for PD4
  BSET CLK_PCKENR2, #1; CLK_PCKENR2_bit.PCKEN21 = 1; // Enable TIM1

  CLR TIM1_ARRH;        TIM1_ARRH = 0;
  MOV TIM1_ARRL, #1;    TIM1_ARRL = 1;               // Frequency
  CLR TIM1_CCR2H;       TIM1_CCR2H = 0;
  MOV TIM1_CCR2L, #1;   TIM1_CCR2L = 1;              // Duty cycle

  BSET TIM1_BKR, #4;    TIM1_BKR_bit.BKE = 1;        // Enable output break
  MOV TIM1_CCMR2, #0x60;TIM1_CCMR2_bit.OC2M = 6;     // Set PWM mode
  BSET TIM1_CR1, #0;    TIM1_CR1_bit.CEN = 1;        // Enable counter
  BSET TIM1_CCER1, #4;  TIM1_CCER1_bit.CC2E = 1;     // CC2 channel enable
                   ;    // Auto output enable (to recover from output break)
  BSET TIM1_BKR, #6;    TIM1_BKR_bit.AOE = 1;


  ; <-- TIM2 / TIM3 / TIM4

                     ;  CLK_PCKENR1_bit.PCKEN10 = 1; // Enable TIM2
  MOV CLK_PCKENR1, #7;  CLK_PCKENR1_bit.PCKEN11 = 1; // Enable TIM3
                     ;  CLK_PCKENR1_bit.PCKEN12 = 1; // Enable TIM4


  ; <-- TIM2 SETUP (ONE-PULSE MODE, EXT. TRIGGER)

  BSET PB_DDR, #2;      PB_DDR_bit.DDR2 = 1;     // PB2 as output (TIM2_CH2)
  BSET PB_CR1, #2;      PB_CR1_bit.C12 = 1;      // Push-pull mode for PB2

  CLR TIM2_ARRH;        TIM2_ARRH = 0;
  MOV TIM2_ARRL, #6;    TIM2_ARRL = 6;           // Pulse width = 30.52uS * ARRL
  CLR TIM2_CCR2H;       TIM2_CCR2H = 0;
  MOV TIM2_CCR2L, #1;   TIM2_CCR2L = 1;

  MOV TIM2_CCMR2, #0x70;TIM2_CCMR2_bit.OC2M = 7; // Set PWM mode 2
  BSET TIM2_CR1, #3;    TIM2_CR1_bit.OPM  = 1;   // One-pulse mode

  BSET TIM2_ETR, #7;    TIM2_ETR_bit.ETP  = 1;   // Trigger on falling edge
  MOV TIM2_SMCR, #0x76; TIM2_SMCR_bit.TS  = 7;   // Trigger source - external
                      ; TIM2_SMCR_bit.SMS = 6;   // Start timer on ext. trigger
  BSET TIM2_CR2, #5;    TIM2_CR2_bit.MMS = 2;    // Upd. event as trigger output

  BSET TIM2_CCER1, #4;  TIM2_CCER1_bit.CC2E = 1; // Enable channel 2 output
  BSET TIM2_BKR, #7;    TIM2_BKR_bit.MOE = 1;    // Enable main output


  ; <-- TIM3 SETUP (TIM2 PULSE COUNTER)

  MOV TIM3_SMCR, #0x37; TIM3_SMCR_bit.TS  = 3;   // Trigger source - TIM2
                      ; TIM3_SMCR_bit.SMS = 7;   // Trigger as timer clock
  BSET TIM3_CR1, #0;    TIM3_CR1_bit.CEN = 1;    // Start the timer


  ; <-- TIM4 SETUP (5-SEC INTERRUPTS)

  MOV TIM4_PSCR, #0x0E; TIM4_PSCR_bit.PSC = 0x0E; // Div by 16384 -> 0.5 sec
                  ;     // 5 second period (ARR = 2*sec - 1)
  MOV TIM4_ARR, #9;     TIM4_ARR = 9; 
                   ;    // Update timer regs to apply prescaler value (optional)
  BSET TIM4_EGR, #0;    TIM4_EGR_bit.UG = 1;
  BSET TIM4_CR1, #0;    TIM4_CR1_bit.CEN = 1;    // Start the timer


  ; <-- RTC_Init

  BSET CLK_CRTCR, #4;   CLK_CRTCR_bit.RTCSEL3   = 1; // LSE -> RTC clock source
                      ; // Enable RTC clock (also required for LCD to work)
  BSET CLK_PCKENR2, #2; CLK_PCKENR2_bit.PCKEN22 = 1;

  MOV RTC_WPR, #0xCA;   RTC_WPR = 0xCA; // Disable write protection to RTC regs
  MOV RTC_WPR, #0x53;   RTC_WPR = 0x53;

  BSET RTC_ISR1, #7;    RTC_ISR1_bit.INIT = 1; // Enter initialization mode
  BTJF RTC_ISR1, #6, $; while (!RTC_ISR1_bit.INITF);

  BSET RTC_CR1, #5;     RTC_CR1_bit.RATIO = 1; // fSYSCLK = fRTC
  CLR RTC_DR3;          RTC_DR3 = 0;
  CLR RTC_DR2;          RTC_DR2 = 0;
  CLR RTC_DR1;          RTC_DR1 = 0;
  CLR RTC_TR3;          RTC_TR3 = 0;
  CLR RTC_TR2;          RTC_TR2 = 0;
  CLR RTC_TR1;          RTC_TR1 = 0;

  BRES RTC_ISR1, #7;    RTC_ISR1_bit.INIT = 0; // Exit initialization mode
  MOV RTC_WPR, #0xFF;   RTC_WPR = 0xFF; // Reenable write protection to RTC regs


  ; <-- LCD_Init

  BSET CLK_PCKENR2, #3; CLK_PCKENR2_bit.PCKEN23 = 1; // Enable LCD clock
                 ;      LCD_CR1_bit.B2    = 0; // LCD bias = 1/3
  MOV LCD_CR1, #6;      LCD_CR1_bit.DUTY0 = 1;
                 ;      LCD_CR1_bit.DUTY1 = 1; // Duty = 1/4
  MOV LCD_FRQ, #0x30;   LCD_FRQ = 3 << 4; // Set frame rate, 4 << 4 is slowest
  MOV LCD_PM1, #0xCF;   LCD_PM1 = 0xCF;
  MOV LCD_PM2, #0x0B;   LCD_PM2 = 0x0B;   // Set pins that drive LCD segments
  BSET LCD_CR3, #6;     LCD_CR3_bit.LCDEN = 1; // Turn on the LCD driver


  ; <-- IRQ_Init

  BSET TIM4_IER, #0;  TIM4_IER_bit.UIE = 1; // Enable TIM4_UIF interrupt
  BSET PC_CR2, #0;    PC_CR2_bit.C20 = 1; // Interrupt from PC0 (on power loss)
  MOV PA_CR2, #0x0C;  PA_CR2_bit.C22 = 1; // Interrupt from PA2 (on mode select)
                   ;  PA_CR2_bit.C23 = 1; // Interrupt from PA3 (on dose reset)
  MOV EXTI_CR1, #0xA0;EXTI_CR1_bit.P2IS = 2; // Interrupt on PORTx2 falling edge
                     ;EXTI_CR1_bit.P3IS = 2; // Interrupt on PORTx3 falling edge
                    ; WFE_CR1_bit.EXTI_EV0 = 1; // WFE for PORTx0 interrupts
  MOV WFE_CR1, #0xD0; WFE_CR1_bit.EXTI_EV2 = 1; // WFE for PORTx2 interrupts
                    ; WFE_CR1_bit.EXTI_EV3 = 1; // WFE for PORTx3 interrupts
  BSET WFE_CR3, #2;   WFE_CR3_bit.TIM4_EV  = 1; // WFE for TIM4 interrupts


  ; <-- the rest of initializations

                   ;  // The reset value for PA_CR1 is 0x01
  MOV PA_CR1, #0x0D;  PA_CR1_bit.C12 = 1;      // Pull up PA2
                   ;  PA_CR1_bit.C13 = 1;      // Pull up PA3
  MOV PC_CR1, #0x1E;  PC_CR1 = 0x1E;           // Pull up unused pins
  BSET FLASH_CR1, #2; FLASH_CR1_bit.WAITM = 1; // Disable Flash during wait mode
  BSET FLASH_CR2, #6; FLASH_CR2_bit.WPRG  = 1; // EEPROM write blocks are 4-byte
