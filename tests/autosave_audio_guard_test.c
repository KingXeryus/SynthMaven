/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdint.h>
#include <stdio.h>
#define FELUCCA_FLASH 1
#define SYNTHMAVEN_SYNTH 1
#define AUTOSAVE_IDLE 2500u
#define AUTOSAVE_GAP 20000u
#define OBJ_AUTOSAVE 7
#define AUTOSAVE_SIZE 4
static struct {int playing;} song;
static struct {int menu;} ui;
static struct {struct {uint32_t sum;} program;} autosave_work;
#define autosave_buf autosave_work.program
static uint32_t fm1_ms=100000,ui_input_ms,autosave_ms,autosave_checked,autosave_hash;
static int flash_ok=1;
static int transport_req,rec_wait,ft_on,quiet,captures,writes;
static uint32_t desired=42;
static int audio_quiet(void){return quiet;}
static void program_capture(void*p){captures++;*(uint32_t*)p=desired;}
static int st_save(int id,void*p,uint32_t n){(void)id;(void)p;(void)n;writes++;return 0;}
#include "../firmware/src/autosave_tick.h"
#define CHECK(x) do{if(!(x)){printf("FAIL line %d\n",__LINE__);return 1;}}while(0)
int main(void){
 quiet=0;autosave_tick();CHECK(captures==0&&writes==0);
 fm1_ms+=1100;quiet=1;autosave_tick();CHECK(captures==1&&writes==1&&autosave_hash==42);
 fm1_ms+=21000;desired=43;quiet=0;autosave_tick();CHECK(captures==1&&writes==1);
 fm1_ms+=1100;quiet=1;autosave_tick();CHECK(captures==2&&writes==2&&autosave_hash==43);
 fm1_ms+=21000;autosave_tick();CHECK(captures==3&&writes==2);
 fm1_ms+=21000;ui_input_ms=fm1_ms;desired=44;autosave_tick();CHECK(captures==3&&writes==2);
 puts("Autosave takes no IRQ-masking snapshot while audio is active; quiet persistence and deduplication PASS");return 0;
}
