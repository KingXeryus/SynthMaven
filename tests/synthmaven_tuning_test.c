/* SPDX-License-Identifier: GPL-3.0-only */
#define PERSISTENCE_TEST_NO_MAIN
#include "persistence_test.c"
int main(void)
{
    int bad=0;reset();voice_t v={0};
    sm_tuning=(sm_tuning_t){.enabled=1,.scale=1,.octave=3,.apply=15,.mask=0xfff};
    for(uint32_t p=0;p<4;p++){
        trk[p].eng_req=ENGI_HYBRID;
        voice_scale_pitch(&trk[p],&v,52);
        bad+=check("RAST adjusts E3 by -50 cents on every enabled audio part",v.pitch16==824 && !v.scale_fine);
        voice_scale_pitch(&trk[p],&v,40);
        bad+=check("notes below C3 retain equal temperament",v.pitch16==640 && !v.scale_fine);
        voice_scale_pitch(&trk[p],&v,49);
        bad+=check("unadjusted chromatic key retains its original pitch",v.pitch16==784 && !v.scale_fine);
    }
    sm_tuning.apply=13;voice_scale_pitch(&trk[1],&v,52);
    bad+=check("individual part can bypass global tuning",v.pitch16==832 && !v.scale_fine);
    sm_tuning.apply=15;sm_tuning.quantize=1;
    bad+=check("MIDI quantization follows global scale without white-key remapping",midi_map(&trk[0],49)==48 && midi_map(&trk[0],47)==47);
    sm_tuning.root=2;voice_scale_pitch(&trk[0],&v,54);
    bad+=check("root transposes adjusted chromatic keys",v.pitch16==856 && !v.scale_fine);
    sm_tuning.scale=SM_TUNE_CUSTOM;sm_tuning.cents[4]=-37;
    bad+=check("custom cents retain one-cent resolution",sm_tuning_cents(&sm_tuning,0,54)==-37);
    uint8_t bytes[SM_TUNE_WIRE];sm_tuning_encode(bytes,&sm_tuning);sm_tuning_t copy={0};
    bad+=check("tagged tuning settings round-trip exactly",sm_tuning_decode(&copy,bytes) && !memcmp(&copy,&sm_tuning,sizeof copy));
    bytes[14]=127;sm_tuning_t before=copy;
    bad+=check("out-of-range tuning rejects the whole update",!sm_tuning_decode(&copy,bytes) && !memcmp(&copy,&before,sizeof copy));
    persist_t saved={0};settings_export(&saved);sm_tuning=(sm_tuning_t){0};
    bad+=check("global tuning persists within unchanged settings record",sizeof saved==604u && settings_import(&saved,sizeof saved) && sm_tuning.cents[4]==-37 && sm_tuning.root==2 && sm_tuning.octave==3);
    project_save(0);sm_tuning.root=7;project_load(0);
    bad+=check("ordinary project loads do not replace global tuning",sm_tuning.root==7 && sm_tuning.cents[4]==-37);
    for(uint32_t scale=1;scale<SM_TUNE_CUSTOM;scale++)for(uint32_t root=0;root<12;root++){
        sm_tuning.scale=scale;sm_tuning.root=root;
        for(uint32_t n=48;n<96;n++){
            int32_t a=sm_tuning_cents(&sm_tuning,0,n),b=sm_tuning_cents(&sm_tuning,0,n+12);
            if(a!=b || a<-100 || a>100 || !sm_tuning_mask(&sm_tuning))bad++;
        }
    }
    printf("SynthMaven tuning: 46 maqam profiles, all roots, octave threshold, bypass and settings: %s\n",bad?"FAIL":"PASS");
    return bad?1:0;
}
