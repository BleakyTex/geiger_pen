  BRES PD_DDR, #4;         <-- disable boost converter driving pin
  BRES PB_DDR, #2;         <-- disable piezo speaker driving pin
  BRES CLK_PCKENR2, #3;    <-- disable LCD
  BRES CLK_PCKENR2, #1;    <-- disable TIM1
  CLR CLK_PCKENR1;         <-- disable peripherals

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


  ; <-- Write to EEPROM
  BTJT FLASH_IAPSR, #3, _eeprom_unlocked_write;
    MOV FLASH_DUKR, #0xAE;
    MOV FLASH_DUKR, #0x56;
  _eeprom_unlocked_write:


  LD A, s:td_mt + 1;
  AND A, #0xF0;
  SRL A;
  PUSH A;
  SRL A;
  SRL A;
  ADD A, (0x01, SP);
  LD (0x01, SP), A;
  LD A, s:td_mt + 1;
  AND A, #0x0F;
  ADD A, (0x01, SP);
  LD (0x01, SP), A;

  LD A, RTC_TR1;           <-- A = seconds
  LDW Y, RTC_TR2;          <-- YH = minutes, YL = hours
  PUSH A;
  PUSH A;
  AND A, #0xF0;
  SRL A;
  LD (0x02, SP), A;
  POP A;
  AND A, #0x0F;
  ADD A, (0x01, SP);
  SLL A;
  SLL A;
  ADD A, (0x01, SP);
  LD (0x01, SP), A;

  LD A, s:td_mt + 0;
  CP A, #0x20;
  JRULT _no_days_overflow;
    LD A, #10;
    LD (0x02, SP), A;      <-- overflow! setting 1010 days (<1024)
  _no_days_overflow:
  AND A, #0x0F;
  LD XL, A;
  LD A, #100;
  MUL X, A;
  ADDW X, (0x01, SP);
  TNZ RTC_DR1;             <-- days
  BRES CLK_PCKENR2, #2;    <-- disable RTC
  JREQ _days_add_zero;
    INCW X;
  _days_add_zero:
  LDW (0x01, SP), X;


  LDW X, s:dd123 + 0;      <-- XH = +0, [XL = +1]
  LD A, #10;
  MUL X, A;
  PUSHW X;
  LD A, s:dd123 + 0;
  ADD A, (0x02, SP);
  LD (0x02, SP), A;

  LD A, YH;                <-- A = minutes
  PUSH A;
  AND A, #0xF0;
  SRL A;
  LD (0x02, SP), A;
  POP A;
  AND A, #0x0F;
  ADD A, (0x01, SP);
  SLL A;
  SLL A;
  ADD A, (0x01, SP);
  LD (0x01, SP), A;

  LDW X, s:dd123 + 1;      <-- XH = +1, [XL = +2]
  LD A, #100;
  MUL X, A;
  ADDW X, (0x01, SP);
  ADD SP, #2;

  BTJF FLASH_IAPSR, #3, $;
  LDW _eeprom_bgn + 0, X;  <-- min:6, ddisp:10
  POPW X;
  LDW _eeprom_bgn + 2, X;  <-- sec:6, days:10
  BTJF FLASH_IAPSR, #6, $; <-- wait for HVOFF


  LD A, s:dose + 0;
  DEC A;
  PUSH A;
  LD A, YL;                <-- A = hours
  AND A, #0xF0;
  PUSH A;
  LD A, YL;
  AND A, #0x0F;
  SLL A;
  ADD A, (0x01, SP);
  SLL A;
  SLL A;
  ADD A, (0x01, SP);
  LD (0x01, SP), A;
  LD A, s:mode_dose;
  NEG A;
  ADD A, #5;
  AND A, #0x07;
  ADD A, (0x01, SP);
  LD (0x01, SP), A;

  POPW X;
  LDW _eeprom_bgn + 4, X;
  LDW X, s:dose + 1;
  LDW _eeprom_bgn + 6, X;
  BTJF FLASH_IAPSR, #6, $; <-- wait for HVOFF

  BRES FLASH_IAPSR, #3;    <-- lock EEPROM
