#include "iostm8l152k4.h"
PUBLIC __iar_program_start


SECTION `.eeprom.noinit`:DATA:ROOT(0)
  _eeprom_bgn:


SECTION `.tiny.noinit`:DATA:ROOT(0)

  ; <-- constants

  NUM1_2:     ds8 10;
  NUM1_5:     ds8 10;
  NUM1_9:     ds8 10;
  NUM1_C:     ds8 10;

  NUM2_1:     ds8 10;
  NUM2_2:     ds8 10;
  NUM2_5:     ds8 10;
  NUM2_8:     ds8 10;
  NUM2_9:     ds8 10;
  NUM2_C:     ds8 10;

  NUM3_1:     ds8 10;
  NUM3_4:     ds8 10;
  NUM3_5:     ds8 10;
  NUM3_8:     ds8 10;
  NUM3_C:     ds8 10;

  NUM4_1:     ds8 10;
  NUM4_4:     ds8 10;
  NUM4_8:     ds8 10;
  NUM4_B:     ds8 10;

  SYM4_1:     ds8 4;
  SYM4_4:     ds8 4;
  SYM4_8:     ds8 4;
  SYM4_B:     ds8 4;

  modes:      ds8 18;

  mantissae:  ds8 7;

  ; <-- variables

  dose:       ds8 3;
  mode_dose:  ds8 1;
  dd123:      ds8 3;
  mode_scr:   ds8 1;
  cpm:        ds8 3;
  td_mt:      ds8 2; <-- time_days BCD + mode_time (top BCD digit)
  rad_rate:   ds8 2;
  event_idx:  ds8 2;

  event_buf:  ; <-- had to be ds16 12 but there`s not enough room below 0x100
              ; <-- (still, this base address sure is below, so this is okay)


EXTERN CSTACK$$Limit
SECTION `.near_func.text`:CODE:ROOT(0)

_tiny_bgn:

  ; <-- constants

  db  0x0A, 0x00, 0x02, 0x02, 0x08, 0x0A, 0x0A, 0x02, 0x0A, 0x0A;
  db  0x20, 0x20, 0xA0, 0xA0, 0xA0, 0x80, 0x80, 0x20, 0xA0, 0xA0;
  db  0x0A, 0x02, 0x08, 0x02, 0x02, 0x02, 0x0A, 0x02, 0x0A, 0x02;
  db  0x80, 0x00, 0x80, 0x80, 0x00, 0x80, 0x80, 0x00, 0x80, 0x80;

  db  0x80, 0x00, 0x80, 0x80, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80;
  db  0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01;
  db  0x08, 0x08, 0x18, 0x18, 0x18, 0x10, 0x10, 0x08, 0x18, 0x18;
  db  0x80, 0x80, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80;
  db  0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00;
  db  0x10, 0x00, 0x10, 0x10, 0x00, 0x10, 0x10, 0x00, 0x10, 0x10;

  db  0x48, 0x00, 0x08, 0x08, 0x40, 0x48, 0x48, 0x08, 0x48, 0x48;
  db  0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x80;
  db  0x00, 0x00, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x04;
  db  0x48, 0x08, 0x40, 0x08, 0x08, 0x08, 0x48, 0x08, 0x48, 0x08;
  db  0x04, 0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00, 0x04, 0x04;

  db  0x06, 0x00, 0x02, 0x02, 0x04, 0x06, 0x06, 0x02, 0x06, 0x06;
  db  0x20, 0x20, 0x60, 0x60, 0x60, 0x40, 0x40, 0x20, 0x60, 0x60;
  db  0x06, 0x02, 0x04, 0x02, 0x02, 0x02, 0x06, 0x02, 0x06, 0x02;
  db  0x40, 0x00, 0x40, 0x40, 0x00, 0x40, 0x40, 0x00, 0x40, 0x40;
  ;    L     d     u     m
  db  0x04, 0x00, 0x00, 0x02;
  db  0x00, 0x60, 0x00, 0x40;
  db  0x04, 0x06, 0x06, 0x06;
  db  0x40, 0x40, 0x40, 0x00;

  db  0xB1,0x10,0x1D, 0x12,0x9B,0x36, 0x02,0xC2,0xB8;
  db  0x01,0x2D,0x12, 0x01,0x04,0x81, 0x01,0x00,0x73;

  db  0x03, 0x05, 0x01, 0x06, 0x04, 0x08, 0x03;

  ; <-- variables

  db  0x01,0x00,0x00;
  db  0x05;
  db  0x00,0x00,0x00;
  db  0x00;
  db  0x00,0x00,0x00;
  db  0x00,0x00;
  db  0x00,0x00;
  db  0x00,0x00;

  dw  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00;


  ; ==================== RAM CODE STARTS HERE ==================== ;

jp_reloc MACRO label;
  JP label - _tiny_bgn;
ENDM;

_main:
  BSET FLASH_CR1,  #3;        <-- Disable Flash/EEPROM
  BTJT CLK_REGCSR, #7, $;     <-- Wait until Flash/EEPROM switch off
  BSET CLK_REGCSR, #1;        <-- Configure low-power mode
  _endless_loop:
    BTJT EXTI_SR1, #0, _state_save;
      jp_reloc(_state_save_skip);
    _state_save:
      BRES CLK_REGCSR, #1;    <-- Configure high-power mode
      BTJF CLK_REGCSR, #0, $;
      BRES FLASH_CR1,  #3;    <-- Enable Flash/EEPROM
      BTJF CLK_REGCSR, #7, $; <-- Wait until Flash/EEPROM switch on
      #include "func/state_save.inc"
      BRES PWR_CSR2, #1;
      HALT;
    _state_save_skip:
    BTJT TIM4_SR1, #0, _rate_update;
      jp_reloc(_rate_update_skip);
    _rate_update:
      BRES TIM4_SR1, #0;
      #include "func/rate_update.inc"
    _rate_update_skip:
    BTJF EXTI_SR1, #2, _mode_switch_skip;
      BSET EXTI_SR1, #2;
      #include "func/mode_switch.inc"
    _mode_switch_skip:
    BTJF EXTI_SR1, #3, _state_reset_skip;
      BSET EXTI_SR1, #3;
      #include "func/state_reset.inc"
    _state_reset_skip:
  #include "func/display.inc"

  ; ===================== RAM CODE ENDS HERE ===================== ;


unroll_times EQU 60;
unroll_copy MACRO times;
  IF (times > 2);
    unroll_copy(times - 2);
  ENDIF;
  POPW Y;
  LDW (times - 2, X), Y;
ENDM;

__iar_program_start:
  LDW X, #(_tiny_bgn - 1);
  LDW SP, X;
  CLRW X;
  LD A, #((__iar_program_start - _tiny_bgn + unroll_times - 1) / unroll_times);
  _copy:
    unroll_copy(unroll_times);
    ADDW X, #unroll_times;
    DEC A;
  JRNE _copy; <-- exactly 20% faster (!!) than the built-in DMA
  #include "func/init.inc"
  #include "func/state_load.inc"
  LDW X, #(CSTACK$$Limit - 1);
  LDW SP, X;
  jp_reloc(_main);


END