/* Filter capability map: source engine controls, no added filter DSP. */
typedef struct {int16_t model,cut,res,env,drive;const char *fixed;} sm_filter_cap_t;
static const sm_filter_cap_t SM_FILTER_CAP[NENGINES]={
 {P_FMODEL,P_E4,P_E5,P_ED_FLT,P_E6,"multi"}, /* ANALOG */
 {-1,-1,-1,-1,-1,"none"}, /* DIGITAL */
 {-1,-1,-1,-1,-1,"none"}, /* PHASE */
 {-1,P_E7,-1,P_ED_FLT,-1,"ONE POLE"}, /* LOFI */
 {-1,P_E4,-1,P_ED_FLT,-1,"ONE POLE"}, /* SAMPLE */
 {-1,-1,-1,-1,-1,"none"}, /* VOICE */
 {P_E4,P_E5,P_E6,P_ED_FLT,-1,"TRIO"}, /* TRIO */
 {-1,-1,-1,-1,-1,"none"}, /* WHEEL */
 {-1,P_E7,-1,P_ED_FLT,-1,"ONE POLE"}, /* GRAIN */
 {-1,-1,-1,-1,-1,"none"}, /* FM6 */
 {-1,-1,-1,-1,-1,"none"}, /* PHYS */
 {-1,P_E2,P_E3,P_ED_FLT,-1,"SVF"}, /* NOISE */
 {-2,P_E0,P_E1,P_E2,-1,"HYBRID"}, /* HYBRID */
 {P_FMODEL,P_E4,P_E5,P_ED_FLT,P_E6,"multi"}, /* WAVE */
 {-1,P_E1,P_E2,P_E3,P_E6,"LADDER"}, /* ACID */
 {-1,P_E4,P_E7,P_ED_FLT,-1,"SVF"}, /* STRINGS */
 {-1,P_E4,-1,P_ED_FLT,-1,"LPG"}, /* MACRO */
 {-1,P_E4,P_E5,P_ED_FLT,-1,"SVF"}, /* VPS */
 {-1,P_E6,P_E7,P_ED_FLT,-1,"SVF"}, /* TERRAIN */
 {-1,-1,-1,-1,-1,"none"}, /* FMX */
 {-1,P_E6,P_E7,P_ED_FLT,-1,"SVF"}, /* WSEQ */
 {-1,P_E5,P_E6,P_ED_FLT,-1,"LADDER"}, /* JUNO */
 {P_FMODEL,P_E7,-1,P_ED_FLT,-1,"multi"}, /* VECTOR */
};
static uint8_t sm_filter_comp_mask;
static int32_t sm_filter_comp_steps(const track_t *t){uint32_t i=(uint32_t)(t-trk);if(i>=4||!(sm_filter_comp_mask&(1u<<i)))return 0;const sm_filter_cap_t *c=&SM_FILTER_CAP[t->eng_req%NENGINES];int32_t r=c->res>=0?clamp(t->p[c->res],0,127):0,d=c->drive>=0?clamp(t->p[c->drive],0,127):0;if(t->eng_req==ENGI_HYBRID){const hy_patch_t *p=hy_gen[i]!=hy_gen_seen[i]?&hy_in[i]:&hy_cur[i];r=p->flt[0].res;d=p->drive;}return (r*12+d*12)/127;}
