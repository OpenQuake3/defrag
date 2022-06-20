#ifndef DEFRAG_H
#define DEFRAG_H

#include "q_shared.h"

typedef struct {
  char const* name;

  uint32_t crc32sum;

  int32_t pps_offset;

  int32_t cg_draw2d_defrag;
  int32_t cg_draw2d_vanilla;
} defrag_t;

qboolean init_defrag(uint32_t crc32sum);

defrag_t const* defrag(void);

#endif // DEFRAG_H
