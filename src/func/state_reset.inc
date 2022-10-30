  ; <-- Clear variables
  CLR A;
  CLRW X;
  LDW s:dose + 1, X;
  MOV s:dose + 0, #1;
  MOV s:mode_dose, #5;
  LDW s:dd123 + 1, X;
  LD  s:dd123 + 0, A;
  LDW s:td_mt + 0, X;

  MOV RTC_WPR, #0xCA; <-- Disable write protection to RTC registers
  MOV RTC_WPR, #0x53;
  BSET RTC_ISR1, #7;  <-- Enter initialization mode
  BTJF RTC_ISR1, #6, $;
  LD  RTC_DR3, A;
  LDW RTC_DR1, X;
  LDW RTC_TR2, X;
  LD  RTC_TR1, A;
  BRES RTC_ISR1, #7;  <-- Exit initialization mode
  MOV RTC_WPR, #0xFF; <-- Enable write protection to RTC registers
