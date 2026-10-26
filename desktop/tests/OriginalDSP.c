/* SPDX-License-Identifier: GPL-3.0-only */
/* Test reference: original single-instance firmware storage, not contextualized. */
#define SM_ENTER(x) ((void)(x))
#include "../Source/DSPUnity.c"
struct sm_context{int unused;};
sm_context *sm_create(void){sm_context*c=calloc(1,sizeof *c);sm_init_voice();int flat=0;for(int e=0;e<12;e++)flat+=ENGINES[e]->npresets;sm_select_program(c,flat);return c;}
void sm_destroy(sm_context*c){free(c);}
int sm_context_bytes(void){return 0;}
