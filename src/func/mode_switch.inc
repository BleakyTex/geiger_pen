  LD A, s:mode_scr;
  JRPL _mode_scr_nonneg;
    DEC A;
    NEG A;
  _mode_scr_nonneg:
  DEC A;
  LD s:mode_scr, A;
