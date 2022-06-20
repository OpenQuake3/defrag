#define MAX_NADES 10

typedef struct
{
  /** the entity number of the nade being tracked, -1 if not tracking */
  int id;
  /** predicted time of when the nade will explode */
  int explode_time;
  /** flag for whether this nade was seen in current snapshot */
  int seen;
} nade_info_t;

extern nade_info_t nades[MAX_NADES];
