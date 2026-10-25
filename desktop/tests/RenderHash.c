#include "SynthMavenDSP.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
int main(int argc,char**argv){sm_context*c=sm_create();sm_select_program(c,argc>1?atoi(argv[1]):0);uint8_t midi[3]={0x90,60,100};sm_midi(c,midi,3);float b[512];uint64_t hash=1469598103934665603ULL;for(int n=0;n<32;n++){sm_render(c,b,256);uint8_t*bytes=(uint8_t*)b;for(size_t i=0;i<sizeof b;i++)hash=(hash^bytes[i])*1099511628211ULL;}printf("%016llx\n",(unsigned long long)hash);sm_destroy(c);}
