#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

uint8_t *__afl_bbmap_ptr = NULL;

/* Uninspired gcc plugin instrumentation */
void __afl_trace_bb(const uint32_t x) {
  __afl_bbmap_ptr[x / 8] |= 1 << (x & 7);
}

__attribute__((constructor())) void __afl_auto_init(void) {
  const char *bb_shmid_ptr = getenv("__AFL_BB_SHM_ID");
  if (bb_shmid_ptr && !__afl_bbmap_ptr) {
    int shmid = atoi(bb_shmid_ptr);
    __afl_bbmap_ptr = shmat(shmid, NULL, 0);
  }
}
