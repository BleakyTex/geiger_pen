  ; <-- Read and clear TIM3 counter
  ; <-- TIM3_CNTRH must be read first to prevent rollover
  LD A, TIM3_CNTRH;
  CLR   TIM3_CNTRH;
  PUSH TIM3_CNTRL;
  CLR  TIM3_CNTRL;
  PUSH A;
  LDW X, (0x01, SP);

  ; <-- Apply dead time correction: CP5S = CP5S + ((CP5S / 32)^2 / 20)
  ; <-- Formula is a polynomial approximation of x = x / (1 - x*38e-6)
  LD  A, #32;
  DIV X, A;
  LD  A, XL;
  MUL X, A;
  LD  A, #20;
  DIV X, A;
  ADDW X, (0x01, SP);
  LDW (0x01, SP), X;

  ; <-- Update radiation rate
  ; <-- Calculate CPM/2 with a rolling sum
  ; <-- Equivalent to:
  ; <-- cpm += cp5s - event_buf[event_idx];
  ; <-- rad_rate = cpm / 2;
  CLR  A;
  LDW  X, s:event_idx;
  LDW  Y, X;
  LDW  X, (s:event_buf, X);
  NEGW X;
  ADDW X, (0x01, SP);
  JRPL _zero_a;
    DEC A;
  _zero_a:
  ADDW X, s:cpm + 1;
  ADC  A, s:cpm + 0;
  LDW  s:cpm + 1, X;
  LD   s:cpm + 0, A;
  SRA  A;
  RRCW X;
  LDW  s:rad_rate, X;

  ; <-- Multiply CPM by 0.52 to find radiation rate
  ; <-- CPM * 0.52 = CPM * 0.5 + CPM * 0.02 = CPM/2 + (CPM/2)/25
  LD A, #25;
  DIV X, A;
  ADDW X, s:rad_rate;
  LDW  s:rad_rate, X;

  ; <-- Update rolling sum buffer
  POPW X;
  LDW  (s:event_buf, Y), X;
  DEC s:event_idx + 1;
  DEC s:event_idx + 1;
  JRPL _event_idx_nonneg;
    MOV s:event_idx + 1, #22;
  _event_idx_nonneg:

  ; <-- Update radiation dose
  ADDW X, s:dose + 1;
  JRNC _dose_no_carry;
    INC s:dose + 0;
  _dose_no_carry:
  LDW s:dose + 1, X;

  ; <-- Prepare the output dose value
  LD A, s:mode_dose            ; 1
  SLL A                        ; 1
  ADD A, s:mode_dose           ; 1
  ADD A, #modes                ; 1
  CLRW Y                       ; 1
  LD YL, A                     ; 1
  LDW X, s:dose + 0            ; 2
  CPW X, (Y)                   ; 2
  JRULT _exit_prep_dose_near   ; 1
  JRUGT _nocmp_dose            ; 1
    LDW X, s:dose + 1          ; 2
    CPW X, (0x01, Y)           ; 2
    JRUGE _nocmp_dose          ; 1
  _exit_prep_dose_near:        ;
    jp_reloc(_exit_prep_dose)  ; 1
  _nocmp_dose:                 ;
    CP A, #12                  ; 1
    JRULE _mode4minus          ; 1
      LDW X, s:dose + 1        ; 2 <-- mode 5
      PUSHW Y                  ; 2
      LDW Y, #1153             ; 2
      DIVW X, Y                ; 16
      LD A, YL                 ; 1
      ADD A, s:dd123 + 1       ; 1
      LD s:dd123 + 1, A        ; 1
      POPW Y                   ; 2
      LD A, #115               ; 1
      DIV X, A                 ; 16
      ADD A, s:dd123 + 0       ; 1
      LDW s:dose + 1, X        ; 2
      JP _carry                ; 1
    _mode4minus:               ;
    JRULT _mode3minus          ; 1
      LDW X, s:dose + 1        ; 2 <-- mode 4
      PUSHW Y                  ; 2
      LDW Y, #1153             ; 2
      DIVW X, Y                ; 16
      LD A, YL                 ; 1
      ADD A, s:dd123 + 0       ; 1
      POPW Y                   ; 2
      LDW s:dose + 1, X        ; 2
      JP _carry                ; 1
    _mode3minus:               ;
      LD A, s:dose + 2         ; 1 <-- mode 3--
      SUB A, (0x02, Y)         ; 1
      LD s:dose + 2, A         ; 1
      LD A, s:dose + 1         ; 1
      SBC A, (0x01, Y)         ; 1
      LD s:dose + 1, A         ; 1
      LD A, s:dose + 0         ; 1
      SBC A, (Y)               ; 1
      INC A                    ; 1 <-- +0x010000
      LD s:dose + 0, A         ; 1
      LD A, s:dd123 + 0        ; 1
      INC A                    ; 1
    _carry:                    ;   <-- A = updated d1
                               ;
    LD s:dd123 + 0, A          ; 1
    CP A, #10                  ; 1
    JRULT _exit_prep_dose      ; 1
      SUB A, #10               ; 1
      LD s:dd123 + 0, A        ; 1
                               ;
      LD A, s:mode_dose        ; 1
      ADD A, #mantissae        ; 1
      CLRW X                   ; 1
      LD XL, A                 ; 1
      LD A, (0x01, X)          ; 1
      PUSH A                   ; 1
      INC s:dd123 + 1          ; 1
      LD A, s:dd123 + 1        ; 1
      CP A, #10                ; 1
      PUSH CC                  ; 1
      JRULT _nod2              ; 1
        SUB A, #10             ; 1
        LD s:dd123 + 1, A      ; 1
        LD A, (X)              ; 1
        ADD A, (0x02, SP)      ; 1
        LD (0x02, SP), A       ; 1
      _nod2:                   ;
      LD A, s:dose + 2         ; 1
      SUB A, (0x02, SP)        ; 1
      LD s:dose + 2, A         ; 1
      LD A, s:dose + 1         ; 1
      SBC A, #0                ; 1
      LD s:dose + 1, A         ; 1
      LD A, s:dose + 0         ; 1
      SBC A, #0                ; 1
      LD s:dose + 0, A         ; 1
      POP CC                   ; 1
      POP A                    ; 1
      JRULT _exit_prep_dose    ; 1
        INC s:dd123 + 2        ; 1
        LD A, s:dd123 + 2      ; 1
        CP A, #10              ; 1
        JRULT _exit_prep_dose  ; 1
          SUB A, #10           ; 1
          PUSH A               ; 1
          PUSH s:dd123 + 1     ; 1
          LD A, s:dd123 + 0    ; 1
          POP s:dd123 + 0      ; 1
          POP s:dd123 + 1      ; 1
          MOV s:dd123 + 2, #1  ; 1
          JREQ _mulhalt        ; 1
            LDW X, Y           ; 1
            LDW X, (0x01, X)   ; 2 <-- [0th byte]
            MUL X, A           ; 4
            ADDW X, s:dose + 1 ; 2
            LDW s:dose + 1, X  ; 2
            LDW Y, (Y)         ; 2 <-- [1st byte]
            MUL Y, A           ; 4
            LD A, YL           ; 1
            ADD A, s:dose + 1  ; 1
            LD s:dose + 1, A   ; 1
          _mulhalt:            ;
          DEC s:mode_dose      ; 1
  _exit_prep_dose:             ;

  ; <-- Update day
  TNZ RTC_DR1;
  JREQ _rtc_zero;       <-- RTC_DR1 cannot get >1 since previous tick
    MOV RTC_WPR, #0xCA; <-- Disable write protection to RTC registers
    MOV RTC_WPR, #0x53;
    BSET RTC_ISR1, #7;  <-- Enter initialization mode
    BTJF RTC_ISR1, #6, $;
    CLR RTC_DR1;        <-- Clear date register
    BRES RTC_ISR1, #7;  <-- Exit initialization mode
    MOV RTC_WPR, #0xFF; <-- Enable write protection to RTC registers

    LDW X, s:td_mt;
    JRNE _rtc_nonzero;
      LDW X, #0x1000;
    _rtc_nonzero:
    LD A, XL
    ADD A, #0x07
    JRH _half_lo
      SUB A, #0x06
    _half_lo:
    ADD A, #0x60
    JRC _full_lo
      SUB A, #0x60
    _full_lo:
    LD s:td_mt + 1, A
    LD A, XH
    ADC A, #0x06
    JRH _half_hi
      SUB A, #0x06
    _half_hi:
    LD s:td_mt + 0, A
  _rtc_zero:
