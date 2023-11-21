
union esr {
  struct {
    u32 iss : 25;
    u32 il  : 1;
    u32 ec  : 6;
    u32 res : 32;
  };
  u64 val;
} __packed;


// Exception context
// Has more info about exception than normal context
struct econtext {
  union esr esr;
  u64 far;
  struct context ctx;
}__packed;

