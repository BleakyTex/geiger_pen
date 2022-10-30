  TNZ s:mode_scr;
  JREQ _rad_rate;
    jp_reloc(_not_rad_rate);
  _rad_rate:

    LDW Y, #0x0800;        <-- dot12 = 0x08, dot11 = 0x00
    LDW X, s:rad_rate;
    CPW X, #9999;
    JRULE _not_div_ten;
      LDW Y, #0x0080;      <-- dot12 = 0x00, dot11 = 0x80
      LD A, #10;
      DIV X, A;
    _not_div_ten:

    LD A, YL;
    PUSH A;                <-- dot_11
    SUB SP, #5;
    LD A, YH;
    LD (0x02, SP), A;      <-- dot_12

    LD A, #10;             <-- prepare division base 10
    DIV X, A;              <-- divide X by 10
    LD (0x03, SP), A;      <-- copy D4 to stack, not to top but 2 bytes lower
    LD A, #10;             <-- prepare division base 10
    DIV X, A;              <-- divide X by 10
    PUSH A;                <-- push D3 to stack top
    LD A, #10;             <-- prepare division base 10
    DIV X, A;              <-- divide X by 10
    CLRW Y;                <-- zero-out Y
    LD YL, A;              <-- copy D2 to Y
    LD A, #10;             <-- prepare division base 10
    DIV X, A;              <-- divide X by 10
    LD XL, A;              <-- copy D1 to X; no need zeroing after /= 10000

    ; now this is how stack looks like:
    ; SP|__|     <-- stack top, nothing here
    ; +1|D3|     <-- third digit
    ; +2|??|     <-- reserved space #1
    ; +3|dot_12| <-- dot_12 + space #2
    ; +4|D4|     <-- fourth digit
    ; +5|??|     <-- reserved space #3
    ; +6|??|     <-- reserved space #4
    ; +7|dot_11| <-- dot_11

    LD A, (S:NUM1_2, X);   <-- A  = NUM1_2[D1]
    OR A, (S:NUM2_2, Y);   <-- A |= NUM2_2[D2]
    LD LCD_RAM2, A;        <-- LCD_RAM2 = A

    LD A, (S:NUM1_9, X);   <-- A  = NUM1_9[D1]
    OR A, (S:NUM2_9, Y);   <-- A |= NUM2_9[D2]
    LD LCD_RAM9, A;        <-- LCD_RAM9 = A

    LD A, (0x03, SP);      <-- A  = dot_12
    OR A, (S:NUM1_C, X);   <-- A |= NUM1_C[D1]
    LD (0x03, SP), A;      <-- copy A to stack, filling reserved space #2
    LD A, (S:NUM1_5, X);   <-- A  = NUM1_5[D1]
    LD (0x02, SP), A;      <-- copy A to stack, filling reserved space #1

    POP A;                 <-- pop D3 from stack top
    LD XL, A;              <-- X, previously holding D1, now holds D3

    POP A;                 <-- pop NUM1_5[D1] (reserved space #1) from stack top
    OR A, (S:NUM2_5, Y);   <-- A |= NUM2_5[D2]
    OR A, (S:NUM3_5, X);   <-- A |= NUM3_5[D3]
    LD LCD_RAM5, A;        <-- LCD_RAM5 = A

    POP A;                 <-- pop NUM1_C[D1] (reserved space #2) from stack top
    OR A, (S:NUM2_C, Y);   <-- A |= NUM2_C[D2]
    OR A, (S:NUM3_C, X);   <-- A |= NUM3_C[D3]
    LD LCD_RAM12, A;       <-- LCD_RAM12 = A

    LD A, (S:NUM2_8, Y);   <-- A  = NUM2_8[D2]
    LD (0x03, SP), A;      <-- copy A to stack, filling reserved space #4
    LD A, (S:NUM2_1, Y);   <-- A  = NUM2_1[D2]
    LD (0x02, SP), A;      <-- copy A to stack, filling reserved space #3

    POP A;                 <-- pop D4 from stack top
    LD YL, A;              <-- Y, previously holding D2, now holds D4

    POP A;                 <-- pop NUM2_1[D2] (reserved space #3) from stack top
    OR A, (S:NUM4_1, Y);   <-- A |= NUM4_1[D4]
    OR A, (S:NUM3_1, X);   <-- A |= NUM3_1[D3]
    LD LCD_RAM1, A;        <-- LCD_RAM1 = A

    POP A;                 <-- pop NUM2_8[D2] (reserved space #4) from stack top
    OR A, (S:NUM4_8, Y);   <-- A |= NUM4_8[D4]
    OR A, (S:NUM3_8, X);   <-- A |= NUM3_8[D3]
    LD LCD_RAM8, A;        <-- LCD_RAM8 = A

    LD A, (S:NUM4_4, Y);   <-- A  = NUM4_4[D4]
    OR A, (S:NUM3_4, X);   <-- A |= NUM3_4[D3]
    LD LCD_RAM4, A;        <-- LCD_RAM4 = A

    POP A;                 <-- pop dot_11 from stack top
    OR A, (S:NUM4_B, Y);   <-- A |= NUM4_B[D4]
    LD LCD_RAM11, A;       <-- LCD_RAM11 = A

    WFE;
    jp_reloc(_endless_loop);


  _not_rad_rate:
  JRPL _rad_dose;
    jp_reloc(_total_time);
  _rad_dose:

    LD A, s:mode_dose;
    JRMI _print_ol;

    SUB SP, #2;
    PUSH #3;               <-- push D4 to stack top
    CP A, #3;
    JRULT _md_less_three;
      DEC (0x01, SP);
      SUB A, #3;
    _md_less_three:
    LD XL, A;
    MUL X, A;
    LD A, XL;
    SWAP A;
    SRL A;
    PUSH A;                <-- dot_12
    PUSH A;
    PUSH s:dd123 + 0;      <-- push D3 to stack top

    CLRW Y;                <-- zero-out Y
    LD A, s:dd123 + 1;
    LD YL, A;              <-- copy D2 to Y
    LD A, s:dd123 + 2;
    LD XL, A;              <-- copy D1 to X; no need zeroing after the MUL above

    ; now this is how stack looks like:
    ; SP|__|     <-- stack top, nothing here
    ; +1|D3|     <-- third digit
    ; +2|??|     <-- reserved space #1
    ; +3|dot_12| <-- dot_12 + space #2
    ; +4|D4|     <-- fourth digit
    ; +5|??|     <-- reserved space #3
    ; +6|??|     <-- reserved space #4

    LD A, (S:NUM1_2, X);   <-- A  = NUM1_2[D1]
    OR A, (S:NUM2_2, Y);   <-- A |= NUM2_2[D2]
    LD LCD_RAM2, A;        <-- LCD_RAM2 = A

    LD A, (S:NUM1_9, X);   <-- A  = NUM1_9[D1]
    OR A, (S:NUM2_9, Y);   <-- A |= NUM2_9[D2]
    LD LCD_RAM9, A;        <-- LCD_RAM9 = A

    LD A, (0x03, SP);      <-- A  = dot_12
    OR A, (S:NUM1_C, X);   <-- A |= NUM1_C[D1]
    LD (0x03, SP), A;      <-- copy A to stack, filling reserved space #2
    LD A, (S:NUM1_5, X);   <-- A  = NUM1_5[D1]
    LD (0x02, SP), A;      <-- copy A to stack, filling reserved space #1

    POP A;                 <-- pop D3 from stack top
    LD XL, A;              <-- X, previously holding D1, now holds D3

    POP A;                 <-- pop NUM1_5[D1] (reserved space #1) from stack top
    OR A, (S:NUM2_5, Y);   <-- A |= NUM2_5[D2]
    OR A, (S:NUM3_5, X);   <-- A |= NUM3_5[D3]
    LD LCD_RAM5, A;        <-- LCD_RAM5 = A

    POP A;                 <-- pop NUM1_C[D1] (reserved space #2) from stack top
    OR A, (S:NUM2_C, Y);   <-- A |= NUM2_C[D2]
    OR A, (S:NUM3_C, X);   <-- A |= NUM3_C[D3]
    LD LCD_RAM12, A;       <-- LCD_RAM12 = A

    LD A, (S:NUM2_8, Y);   <-- A  = NUM2_8[D2]
    LD (0x03, SP), A;      <-- copy A to stack, filling reserved space #4
    LD A, (S:NUM2_1, Y);   <-- A  = NUM2_1[D2]
    LD (0x02, SP), A;      <-- copy A to stack, filling reserved space #3

    POP A;                 <-- pop D4 from stack top
    LD YL, A;              <-- Y, previously holding D2, now holds D4

    POP A;                 <-- pop NUM2_1[D2] (reserved space #3) from stack top
    OR A, (S:SYM4_1, Y);   <-- A |= SYM4_1[D4]
    OR A, (S:NUM3_1, X);   <-- A |= NUM3_1[D3]
    LD LCD_RAM1, A;        <-- LCD_RAM1 = A

    POP A;                 <-- pop NUM2_8[D2] (reserved space #4) from stack top
    OR A, (S:SYM4_8, Y);   <-- A |= SYM4_8[D4]
    OR A, (S:NUM3_8, X);   <-- A |= NUM3_8[D3]
    LD LCD_RAM8, A;        <-- LCD_RAM8 = A

    LD A, (S:SYM4_4, Y);   <-- A  = SYM4_4[D4]
    OR A, (S:NUM3_4, X);   <-- A |= NUM3_4[D3]
    LD LCD_RAM4, A;        <-- LCD_RAM4 = A

    LD A, (S:SYM4_B, Y);   <-- A  = SYM4_B[D4]
    LD LCD_RAM11, A;       <-- LCD_RAM11 = A

    WFE;
    jp_reloc(_endless_loop);


  _print_ol:

    CLR A;
    LD LCD_RAM2, A;
    LD LCD_RAM9, A;
    MOV LCD_RAM5,  NUM3_5 + 0; <-- avoid 'MOV L:DST, S:SRC' (compiles as S:DST)
    MOV LCD_RAM12, NUM3_C + 0; <-- avoid 'MOV L:DST, S:SRC' (compiles as S:DST)
    LD A, S:NUM3_1 + 0;
    OR A, S:SYM4_1 + 0;
    LD LCD_RAM1, A;
    LD A, S:NUM3_8 + 0;
    OR A, S:SYM4_8 + 0;
    LD LCD_RAM8, A;
    LD A, S:NUM3_4 + 0;
    OR A, S:SYM4_4 + 0;
    LD LCD_RAM4, A;
    MOV LCD_RAM11, SYM4_B + 0; <-- avoid 'MOV L:DST, S:SRC' (compiles as S:DST)

    WFE;
    jp_reloc(_endless_loop);


  _total_time:

    LD A, s:td_mt + 0;
    AND A, #0xF0;
    CP A, #0x10;
    JRUGT _print_ol;
    JRULT _get_time;
      LDW X, s:td_mt;
      JRA _decode;
    _get_time:
      LDW X, RTC_TR1;      <-- XH := sec, XL := min
      LD A, RTC_TR3;       <-- A := hrs
      JREQ _sec_min;
        RLWA X, A;         <-- A := sec, XH := min, XL := hrs
      _sec_min:
      TNZ RTC_DR3;         <-- just finish reading time/date
      LD A, XH;
      SWAP A;
      AND A, #0x0F;
      PUSH A;
      PUSH #0;
      SLLW X;
      SLLW X;
      SLLW X;
      SLLW X;
      ADDW X, (0x01, SP);
      ADD SP, #2;

    _decode:
    SUB SP, #5;
    LD A, XH;
    SWAP A;
    AND A, #0x0F;
    LD (0x03, SP), A;      <-- copy D4 to stack, not to top but 2 bytes lower
    LD A, XL;
    AND A, #0x0F;
    PUSH A;                <-- push D3 to stack top
    LD A, XL;
    SWAP A;
    AND A, #0x0F;
    CLRW Y;                <-- zero-out Y
    LD YL, A;              <-- copy D2 to Y
    LD A, XH;
    AND A, #0x0F;
    CLRW X;                <-- zero-out X
    LD XL, A;              <-- copy D1 to X

    ; now this is how stack looks like:
    ; SP|__|     <-- stack top, nothing here
    ; +1|D3|     <-- third digit
    ; +2|??|     <-- reserved space #1
    ; +3|??|     <-- reserved space #2
    ; +4|D4|     <-- fourth digit
    ; +5|??|     <-- reserved space #3
    ; +6|??|     <-- reserved space #4

    LD A, (S:NUM1_2, X);   <-- A  = NUM1_2[D1]
    OR A, (S:NUM2_2, Y);   <-- A |= NUM2_2[D2]
    LD LCD_RAM2, A;        <-- LCD_RAM2 = A

    LD A, (S:NUM1_9, X);   <-- A  = NUM1_9[D1]
    OR A, (S:NUM2_9, Y);   <-- A |= NUM2_9[D2]
    LD LCD_RAM9, A;        <-- LCD_RAM9 = A

    LD A, (S:NUM1_C, X);   <-- A  = NUM1_C[D1]
    LD (0x03, SP), A;      <-- copy A to stack, filling reserved space #2
    LD A, (S:NUM1_5, X);   <-- A  = NUM1_5[D1]
    LD (0x02, SP), A;      <-- copy A to stack, filling reserved space #1

    POP A;                 <-- pop D3 from stack top
    LD XL, A;              <-- X, previously holding D1, now holds D3

    POP A;                 <-- pop NUM1_5[D1] (reserved space #1) from stack top
    OR A, (S:NUM2_5, Y);   <-- A |= NUM2_5[D2]
    OR A, (S:NUM3_5, X);   <-- A |= NUM3_5[D3]
    LD LCD_RAM5, A;        <-- LCD_RAM5 = A

    POP A;                 <-- pop NUM1_C[D1] (reserved space #2) from stack top
    OR A, (S:NUM2_C, Y);   <-- A |= NUM2_C[D2]
    OR A, (S:NUM3_C, X);   <-- A |= NUM3_C[D3]
    LD LCD_RAM12, A;       <-- LCD_RAM12 = A

    LD A, (S:NUM2_8, Y);   <-- A  = NUM2_8[D2]
    LD (0x03, SP), A;      <-- copy A to stack, filling reserved space #4
    LD A, (S:NUM2_1, Y);   <-- A  = NUM2_1[D2]
    LD (0x02, SP), A;      <-- copy A to stack, filling reserved space #3

    POP A;                 <-- pop D4 from stack top
    LD YL, A;              <-- Y, previously holding D2, now holds D4

    TNZ s:td_mt + 0;
    JRNE _print_date;
      POP A;               <-- pop NUM2_1[D2] (reserved space #3) from stack top
      OR A, (S:NUM4_1, Y); <-- A |= NUM4_1[D4]
      OR A, (S:NUM3_1, X); <-- A |= NUM3_1[D3]
      LD LCD_RAM1, A;      <-- LCD_RAM1 = A

      POP A;               <-- pop NUM2_8[D2] (reserved space #4) from stack top
      OR A, (S:NUM4_8, Y); <-- A |= NUM4_8[D4]
      OR A, (S:NUM3_8, X); <-- A |= NUM3_8[D3]
      LD LCD_RAM8, A;      <-- LCD_RAM8 = A

      LD A, (S:NUM4_4, Y); <-- A  = NUM4_4[D4]
      OR A, (S:NUM3_4, X); <-- A |= NUM3_4[D3]
      LD LCD_RAM4, A;      <-- LCD_RAM4 = A

      LD A, (S:NUM4_B, Y); <-- A  = NUM4_B[D4]
      OR A, #0x20;         <-- A |= dot_11
      LD LCD_RAM11, A;     <-- LCD_RAM11 = A

      WFE;
      jp_reloc(_endless_loop);

    _print_date:
      POP A;               <-- pop NUM2_1[D2] (reserved space #3) from stack top
      OR A, (S:SYM4_1, Y); <-- A |= SYM4_1[D4]
      OR A, (S:NUM3_1, X); <-- A |= NUM3_1[D3]
      LD LCD_RAM1, A;      <-- LCD_RAM1 = A

      POP A;               <-- pop NUM2_8[D2] (reserved space #4) from stack top
      OR A, (S:SYM4_8, Y); <-- A |= SYM4_8[D4]
      OR A, (S:NUM3_8, X); <-- A |= NUM3_8[D3]
      LD LCD_RAM8, A;      <-- LCD_RAM8 = A

      LD A, (S:SYM4_4, Y); <-- A  = SYM4_4[D4]
      OR A, (S:NUM3_4, X); <-- A |= NUM3_4[D3]
      LD LCD_RAM4, A;      <-- LCD_RAM4 = A

      LD A, (S:SYM4_B, Y); <-- A  = SYM4_B[D4]
      LD LCD_RAM11, A;     <-- LCD_RAM11 = A

      WFE;
      jp_reloc(_endless_loop);
