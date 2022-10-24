  ; struct {
  ;   uint16_t min:6;     // dose measurement minutes, 0..59
  ;   uint16_t ddisp:10;  // displayed digits of dose, 0..999
  ; 
  ;   uint16_t sec:6;     // dose measurement seconds, 0..59
  ;   uint16_t days:10;   // dose measurement days, 0..999
  ; 
  ;   uint16_t hrs:5;     // dose measurement hours, 0..23
  ;   uint16_t mode:3;    // dose display mode, 0..6
  ;   uint16_t dose_h:8;  // radiation events counter (high 1 byte), 0..11538461
  ; 
  ;   uint16_t dose_l:16; // radiation events counter (low 2 bytes), 0..11538461
  ; } state;

  LDW X, _eeprom_bgn + 0;
  LD A, XH;
  SRL A;
  SRL A;
  PUSH A;             <-- minutes
  LD A, XH;
  AND A, #0x03;
  LD XH, A;

  LD A, #10;
  DIV X, A;
  LD s:dd123 + 0, A;
  LD A, #10;
  DIV X, A;
  LD s:dd123 + 1, A;
  LD A, #10;
  DIV X, A;
  LD s:dd123 + 2, A;

  LDW X, _eeprom_bgn + 2;
  LD A, XH;
  SRL A;
  SRL A;
  PUSH A;             <-- seconds
  LD A, XH;
  AND A, #0x03;
  LD XH, A;

  LD A, #10;
  DIV X, A;
  LD s:td_mt + 1, A;
  LD A, #10;
  DIV X, A;
  SWAP A;
  OR A, s:td_mt + 1;
  LD s:td_mt + 1, A;
  LD A, #10;
  DIV X, A;
  JRNE _days_lessthan_1k;
    ADD A, #0x10;
  _days_lessthan_1k:
  LD s:td_mt + 0, A;
  LDW X, s:td_mt;
  JREQ _days_zero;
    ADD A, #0x10;
    LD s:td_mt + 0, A;
  _days_zero:

  LDW X, _eeprom_bgn + 4;
  LD A, XH;
  AND A, #0x07;
  NEG A;
  ADD A, #5;
  LD s:mode_dose, A;
  LD A, XH;
  SRL A;
  SRL A;
  SRL A;              <-- hours

  MOV RTC_WPR, #0xCA; <-- Disable write protection to RTC registers
  MOV RTC_WPR, #0x53;
  BSET RTC_ISR1, #7;  <-- Enter initialization mode
  BTJF RTC_ISR1, #6, $;
  LD RTC_TR3, A;      <-- Set hours
  POP RTC_TR1;        <-- Set seconds
  POP RTC_TR2;        <-- Set minutes
  BRES RTC_ISR1, #7;  <-- Exit initialization mode
  MOV RTC_WPR, #0xFF; <-- Enable write protection to RTC registers

  LD A, XL;
  INC A;
  LD s:dose + 0, A;
  LDW X, _eeprom_bgn + 6;
  LDW s:dose + 1, X;
