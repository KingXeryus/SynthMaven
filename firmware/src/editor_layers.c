/* SPDX-License-Identifier: GPL-3.0-only */
#define ED_LAYERS 111u
#define ED_ROUTES 118u
static int ed_layers_handle(uint32_t cmd,const uint8_t *a,uint32_t n)
{
 if(cmd==ED_ROUTES){
  uint32_t op=n?a[0]:127u,rc=0;uint8_t raw[SM_ROUTES_WIRE];sm_routes_t next;
  if(!n||op>1u||(!op&&n!=1u))rc=1;
  else if(op){if(ed_unpack7(a+1,n-1,raw,sizeof raw)!=sizeof raw||!sm_routes_decode(&next,raw))rc=1;
  else {uint32_t guard=motion_guard();sm_routes=next;panic_req|=15u;motion_unguard(guard);settings_save();ui.force=1;}}
  track_t *drum=sm_drum_part();
  ed_b(op);ed_b(rc);ed_b(drum?trk_index(drum):127u);sm_routes_encode(raw,&sm_routes);ed_bk_pack(raw,sizeof raw);return 1;
 }
 if(cmd!=ED_LAYERS)return 0;
 uint32_t op=n?a[0]:127u,rc=0;uint8_t raw[SM_LAYERS_WIRE];sm_layers_t next;
 if(!n||op>1u||(!op&&n!=1u))rc=1;
 else if(op){if(ed_unpack7(a+1,n-1,raw,sizeof raw)!=sizeof raw||!sm_layers_decode(&next,raw))rc=1;
 else {uint32_t guard=motion_guard();sm_layers=next;motion_unguard(guard);settings_save();ui.force=1;}}
 ed_b(op);ed_b(rc);sm_layers_encode(raw,&sm_layers);ed_bk_pack(raw,sizeof raw);return 1;
}
