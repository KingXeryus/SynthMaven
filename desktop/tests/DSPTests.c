#include "SynthMavenDSP.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
static void note(sm_context*c,int on){uint8_t b[3]={(uint8_t)(on?0x90:0x80),60,100};sm_midi(c,b,3);}
static void *thread_render(void *arg){sm_context*c=arg;float b[512];for(int i=0;i<200;i++){note(c,1);sm_render(c,b,256);for(int j=0;j<512;j++)if(!isfinite(b[j]))abort();}return 0;}
int main(void){printf("%d engines; %d presets; %d parameters; %d context bytes\n",sm_engine_count(),sm_program_count(),sm_parameter_count(),sm_context_bytes());fflush(stdout);
 for(int p=0;p<sm_parameter_count();p++){sm_parameter d=sm_parameter_info(0,p);if(!d.name||d.maximum<d.minimum)return 1;}
 for(int e=0;e<sm_engine_count();e++)for(int p=0;p<sm_parameter_count();p++){sm_parameter d=sm_parameter_info(e,p);if(d.choices)for(int v=d.minimum;v<=d.maximum;v++){const char*s=sm_parameter_choice(e,p,v);if(!s||strlen(s)>100)return 5;}}
 sm_context *a=sm_create(),*b=sm_create(),*ref=sm_create();if(!a||!b||!ref)return 2;
 float x[1024],y[1024],z[1024];note(a,1);note(ref,1);sm_select_program(b,0);note(b,1);
 for(int i=0;i<80;i++){sm_render(a,x,512);sm_render(b,y,512);sm_render(ref,z,512);if(memcmp(x,z,sizeof x)){fprintf(stderr,"Isolation mismatch block %d\n",i);return 3;}}
 sm_destroy(a);sm_destroy(b);sm_destroy(ref);
 for(int p=0;p<sm_program_count();p++){a=sm_create();sm_select_program(a,p);note(a,1);double energy=0;for(int i=0;i<12;i++){sm_render(a,x,512);for(int j=0;j<1024;j++){if(!isfinite(x[j]))return 4;energy+=fabs(x[j]);}}printf("%3d %-10s %-16s %.3f\n",p,sm_engine_name(sm_program_engine(p)),sm_program_name(p),energy);fflush(stdout);sm_destroy(a);}
 a=sm_create();sm_select_program(a,0);sm_set_parameter(a,37,1.f/5);sm_set_parameter(a,38,90.f/127);sm_set_parameter(a,41,1);note(a,1);sm_render(a,x,512);uint8_t high[3]={0x90,72,100};sm_midi(a,high,3);sm_render(a,x,64);double pitch=sm_voice_pitch(a);if(!(pitch>60&&pitch<72)){fprintf(stderr,"Portamento did not glide: %.4f\n",pitch);return 6;}for(int i=0;i<1200;i++)sm_render(a,x,512);if(fabs(sm_voice_pitch(a)-72)>.1)return 7;sm_destroy(a);puts("PASS: portamento starts between notes and reaches target pitch");
 a=sm_create();b=sm_create();pthread_t t1,t2;pthread_create(&t1,0,thread_render,a);pthread_create(&t2,0,thread_render,b);pthread_join(t1,0);pthread_join(t2,0);sm_destroy(a);sm_destroy(b);puts("PASS: metadata, every factory preset, interleaved and concurrent instances");return 0;}
