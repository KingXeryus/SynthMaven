/* Compare native and packed WAVE buffers, including in-place decode/output overlap. */
#define UI_PAGES_HARNESS_ONLY
#include "ui_pages_test.c"
int main(void) {
 uint32_t t,f,i,h;
 for(t=0;t<WT_NFACTORY;t++) {wt_have=-1;for(f=0;f<WT_NF;f++)wt_build_frame(t,f);h=2166136261u;
  for(i=0;i<sizeof wt_mip;i++)h=(h^((uint8_t *)wt_mip)[i])*16777619u;
  printf("%u %08x\n",t,h);
 }return 0;
}
