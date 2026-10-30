/* Documentation-only host capture of the current firmware descriptors and renderer. */
#include "../build/guide/renderer.h"
static void js(const char *s){putchar('"');for(;s&&*s;s++){if(*s=='"'||*s=='\\')putchar('\\');if((unsigned char)*s>=32)putchar(*s);}putchar('"');}
static void desc(const param_desc_t *d){
 if(!d || !d->label || !d->label[0]){printf("null");return;}
 printf("{\"label\":");js(d->label);printf(",\"min\":%d,\"max\":%d,\"fmt\":%u,\"choices\":[",d->min,d->max,d->fmt);
 if(d->names && d->max-d->min<80)for(int v=d->min;v<=d->max;v++){if(v>d->min)putchar(',');js(d->names[v]);}
 char lo[64],hi[64];const char *lu,*hu;param_format(d,d->min,lo,&lu);param_format(d,d->max,hi,&hu);
 printf("],\"low\":");js(lo);printf(",\"high\":");js(hi);printf(",\"unit\":");js(hu);printf("}");
}
int main(void){int first=1;printf("[");
 for(unsigned e=0;e<NENGINES;e++)if(eng_ok(e))for(unsigned i=0;i<NPAGES;i++)for(unsigned fx=0;fx<(PAGES[i].fam==FAM_FX?FX_COUNT:1u);fx++){
  state();pal(0);eng(e);ui.home=0;ui.page=i;page_entered();
  if(fx){if(e!=ENGI_FM6 || fx==6)continue;for(unsigned n=0;n<3;n++){TSEL->p[P_FXTYPE+n*FX_PARAMS]=fx;song.g[G_MFXTYPE+n*FX_PARAMS]=fx;}}
  if(!page_visible(i))continue;
  const page_t *p=&PAGES[i];char file[80];snprintf(file,sizeof file,"ref_%u_%u_%u",e,i,fx);
  draw(-1);write_ppm("build/guide","NIGHT",file);
  if(!first)putchar(',');first=0;printf("{\"engine\":");js(ENGINES[e]->name);printf(",\"engine_id\":%u,\"page_id\":%u,\"scope\":%u,\"title\":",e,i,p->scope);js(p->title);
  printf(",\"effect\":");js(fx?N_FX[fx]:"");printf(",\"family\":%u,\"env\":%d,\"lfo\":%d,\"image\":",p->fam,native_page(i,FAM_ENV),native_page(i,FAM_LFO));js(file);printf(",\"controls\":[");
  for(unsigned k=0;k<4;k++){if(k)putchar(',');int16_t *v;const param_desc_t *d=page_desc(p,k,&v);
   if(p->scope==SC_SM_PREF){printf("{\"label\":");js(sm_pref_label(p->id[k]));printf(",\"pref\":%u}",p->id[k]);}else desc(d);
  }printf("],\"list\":[");
  const list_t *l=list_of(i);
  if(l)for(unsigned r=0;r<l->n;r++){if(r)putchar(',');const lrow_t *rw=&l->row[r];int16_t *v;desc(page_desc(&PAGES[page_titled(rw->page)],rw->slot,&v));}
  printf("]}");
 }
 printf("]\n");return 0;}
