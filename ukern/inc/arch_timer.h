
#define CNT_CTL_ISTATUS (1 << 2)
#define CNT_CTL_IMASK   (1 << 1)
#define CNT_CTL_ENABLE  (1 << 0)


enum arch_timer_reg {
	ARCH_TIMER_REG_P_CTRL,
	ARCH_TIMER_REG_P_CVAL,
	ARCH_TIMER_REG_P_TVAL,
	ARCH_TIMER_REG_V_CTRL,
	ARCH_TIMER_REG_V_CVAL,
	ARCH_TIMER_REG_V_TVAL,
};

static __always_inline void
arch_timer_reg_write(enum arch_timer_reg reg, u32 val)
{
  switch (reg) {
  case ARCH_TIMER_REG_P_CTRL:
    write_sysreg(val, cntp_ctl_el0);
    break;
  case ARCH_TIMER_REG_P_CVAL:
    write_sysreg(val, cntp_cval_el0);
    break;
  case ARCH_TIMER_REG_P_TVAL:
    write_sysreg(val, cntp_tval_el0);
    break;
  case ARCH_TIMER_REG_V_CTRL:
    write_sysreg(val, cntv_ctl_el0);
    break;
  case ARCH_TIMER_REG_V_CVAL:
    write_sysreg(val, cntv_cval_el0);
    break;
  case ARCH_TIMER_REG_V_TVAL:
    write_sysreg(val, cntv_tval_el0);
    break;
  }

  isb();
}

static __always_inline u32
arch_timer_reg_read(enum arch_timer_reg reg)
{
  switch (reg) {
  case ARCH_TIMER_REG_P_CTRL:
    return read_sysreg(cntp_ctl_el0);
  case ARCH_TIMER_REG_P_CVAL:
    return read_sysreg(cntp_cval_el0);
  case ARCH_TIMER_REG_P_TVAL:
    return read_sysreg(cntp_tval_el0);
  case ARCH_TIMER_REG_V_CTRL:
    return read_sysreg(cntv_ctl_el0);
  case ARCH_TIMER_REG_V_CVAL:
    return read_sysreg(cntv_cval_el0);
  case ARCH_TIMER_REG_V_TVAL:
    return read_sysreg(cntv_tval_el0);
  }
  return (u32)-1;
}

static __always_inline u32 
arch_timer_get_cntfrq(void)
{
	return read_sysreg(cntfrq_el0);
}

static inline u64 arch_counter_get_cntvct(void)
{
	isb();
	return read_sysreg(cntvct_el0);
}

static inline u64 arch_counter_get_cntpct(void)
{
	isb();
	return read_sysreg(cntpct_el0);
}
