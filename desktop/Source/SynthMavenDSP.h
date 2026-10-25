/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct sm_context sm_context;
typedef struct { const char *name; int minimum, maximum, initial, choices; } sm_parameter;
sm_context *sm_create(void);
void sm_destroy(sm_context *);
int sm_engine_count(void);
const char *sm_engine_name(int);
int sm_program_count(void);
const char *sm_program_name(int);
int sm_program_engine(int);
void sm_select_program(sm_context *,int);
int sm_parameter_count(void);
sm_parameter sm_parameter_info(int engine,int parameter);
const char *sm_parameter_choice(int engine,int parameter,int value);
float sm_get_parameter(sm_context *,int);
void sm_set_parameter(sm_context *,int,float);
void sm_midi(sm_context *,const uint8_t *,int);
void sm_render(sm_context *,float *interleaved,int frames);
void sm_panic(sm_context *);
void sm_tempo(sm_context *,double);
int sm_control_period(void);
int sm_custom_patch(sm_context *,int kind,const uint8_t *,int);
int sm_context_bytes(void);
#ifdef __cplusplus
}
#endif
