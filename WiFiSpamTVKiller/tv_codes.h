#pragma once
#include <stdint.h>

enum TvProto : uint8_t {
  P_NEC, P_SONY, P_RC5, P_RC6, P_SAMSUNG, P_LG, P_PANASONIC, P_JVC
};

struct TvCode {
  const char* brand;
  uint8_t     proto;
  uint64_t    code;
  uint8_t     bits;
};

static const TvCode TV_CODES[] = {
  { "Samsung",      P_SAMSUNG,   0xE0E040BFULL,      32 },
  { "LG",           P_NEC,       0x20DF10EFULL,      32 },
  { "Sony",         P_SONY,      0x00000A90ULL,      12 },
  { "Philips",      P_RC5,       0x0000000CULL,      12 },
  { "Panasonic",    P_PANASONIC, 0x40040100BCBDULL,  48 },
  { "Toshiba",      P_NEC,       0x02FD48B7ULL,      32 },
  { "Sharp",        P_NEC,       0x5B5B48B7ULL,      32 },
  { "Hitachi",      P_NEC,       0x00FEA857ULL,      32 },
  { "Vizio",        P_NEC,       0x20DF10EFULL,      32 },
  { "TCL",          P_NEC,       0x00FEA857ULL,      32 },
  { "Hisense",      P_NEC,       0x00FF00FFULL,      32 },
  { "Grundig",      P_NEC,       0x00FEA857ULL,      32 },
  { "Telefunken",   P_NEC,       0x00FEA857ULL,      32 },
  { "Medion",       P_NEC,       0x00FEA857ULL,      32 },
  { "Sanyo",        P_NEC,       0xCC0000FFULL,      32 },
  { "Acer",         P_NEC,       0x10C8E11EULL,      32 },
  { "RCA / Generic",P_NEC,       0x00FF00FFULL,      32 },
  { "Sony OFF",     P_SONY,      0x00000F50ULL,      12 },
  { "Samsung OFF",  P_SAMSUNG,   0xE0E019E6ULL,      32 },
  { "Samsung ON",   P_SAMSUNG,   0xE0E09966ULL,      32 }
};
static const size_t TV_COUNT = sizeof(TV_CODES) / sizeof(TV_CODES[0]);
