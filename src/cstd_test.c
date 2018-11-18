#include <stdio.h>

#include "cstd_safety.h"

#define LOG_DEBUG(format,arg...) printf("%s,%d:"format"\n",__FILE__,__LINE__,##arg);

int main(int argc, char** argv)
{
    int ret = 0;
    CSTD_safety_gas_leak_t gas_leak;
    gas_leak.gap_area=0.000078;
    gas_leak.gas_temperature = 10;
    gas_leak.const_press_spec_heat = 1231.0;
    gas_leak.const_vol_spec_heat = 33;
    gas_leak.leak_coeff = 1.0;
    gas_leak.mol_wt = 1233;
    gas_leak.vessel_pressure = 123000;

    LOG_DEBUG("==========CSTD_safety_gas_leak_calc test==========");
    ret = CSTD_safety_gas_leak_calc(&gas_leak);
    LOG_DEBUG("CSTD_safety_gas_leak_calc ret:%d",ret);
    LOG_DEBUG("leak type:%d", gas_leak.leak_type);
    LOG_DEBUG("leak speed:%f", gas_leak.leak_rate);

    LOG_DEBUG("==========CSTD_safety_liquid_leak_calc test==========");
    CSTD_safety_liquid_leak_t liquid_leak;
    liquid_leak.gap_area = 0.0058;
    liquid_leak.height_diff = 1.2;
    liquid_leak.leak_coeff = 0.5;
    liquid_leak.leak_time = 30;
    liquid_leak.density = 1.98;
    liquid_leak.vessel_CSA = 0.8;
    liquid_leak.pressure = 213100;
    CSTD_safety_liquid_leak_calc(&liquid_leak);
    LOG_DEBUG("instantaneous_rate:%f", liquid_leak.instantaneous_rate);
    LOG_DEBUG("leak_rate:%f", liquid_leak.leak_rate);

    LOG_DEBUG("==========CSTD_safety_solid_fire_calc test==========");
    CSTD_safety_solid_fire_t solid_fire;
    solid_fire.combustion_heat = 10000.0;
    solid_fire.combustion_rate = 100.0;
    solid_fire.mass = 500;
    ret = CSTD_safety_solid_fire_calc(&solid_fire);
    LOG_DEBUG("CSTD_safety_solid_fire_calc ret:%d",ret);
    LOG_DEBUG("death_r:%f", solid_fire.death_r);
    LOG_DEBUG("serious_injury_r:%f", solid_fire.serious_injury_r);
    LOG_DEBUG("slight_injury_r:%f", solid_fire.slight_injury_r);
    LOG_DEBUG("property_damage_r:%f", solid_fire.property_damage_r);

    LOG_DEBUG("==========CSTD_safety_pool_fire_calc test==========");
    CSTD_safety_pool_fire_t pool_fire;
    pool_fire.combustion_heat = 5200;
    pool_fire.combustion_rate = 500;
    pool_fire.mass = 5000;
    pool_fire.pool_area = 50;
    CSTD_safety_pool_fire_calc(&pool_fire);
    LOG_DEBUG("CSTD_safety_pool_fire_calc ret:%d",ret);
    LOG_DEBUG("death_r:%f", pool_fire.death_r);
    LOG_DEBUG("serious_injury_r:%f", pool_fire.serious_injury_r);
    LOG_DEBUG("slight_injury_r:%f", pool_fire.slight_injury_r);
    LOG_DEBUG("property_damage_r:%f", pool_fire.property_damage_r);

    LOG_DEBUG("==========CSTD_safety_jet_fire_calc test==========");
    CSTD_safety_jet_fire_t jet_fire;
    jet_fire.combustion_heat = 12455;
    jet_fire.combustion_dur = 100;
    jet_fire.diameter = 0.012;
    jet_fire.gas_leak.gas_temperature = 10;
    jet_fire.gas_leak.const_press_spec_heat = 1231.0;
    jet_fire.gas_leak.const_vol_spec_heat = 33;
    jet_fire.gas_leak.leak_coeff = 1.0;
    jet_fire.gas_leak.mol_wt = 1233;
    jet_fire.gas_leak.vessel_pressure = 123000;
    CSTD_safety_jet_fire_calc(&jet_fire);
    LOG_DEBUG("CSTD_safety_jet_fire_calc ret:%d",ret);
    LOG_DEBUG("death_r:%f", pool_fire.death_r);
    LOG_DEBUG("serious_injury_r:%f", jet_fire.serious_injury_r);
    LOG_DEBUG("slight_injury_r:%f", jet_fire.slight_injury_r);
    LOG_DEBUG("property_damage_r:%f", jet_fire.property_damage_r);

    LOG_DEBUG("==========CSTD_safety_BLEVE_fire_calc test==========");
    CSTD_safety_BLEVE_fire_t BLEVE_fire;
    BLEVE_fire.mass = 55650.60;
    BLEVE_fire.radiant_heat_flux = 270000.0;
    CSTD_safety_BLEVE_fire_calc(&BLEVE_fire);
    LOG_DEBUG("death_r:%f",BLEVE_fire.death_r);
    LOG_DEBUG("combustion_dur:%f",BLEVE_fire.combustion_dur);
    LOG_DEBUG("fireball_r:%f",BLEVE_fire.fireball_r);
    LOG_DEBUG("serious_injury_r:%f",BLEVE_fire.serious_injury_r);
    LOG_DEBUG("slight_injury_r:%f",BLEVE_fire.slight_injury_r);
    LOG_DEBUG("property_damage_r:%f",BLEVE_fire.property_damage_r);

    LOG_DEBUG("==========CSTD_safety_vcloud_explosion test==========");
    CSTD_safety_vcloud_explo_t vcloud_explosion;
    vcloud_explosion.combustion_heat = 43070.0;
    vcloud_explosion.mass = 37857.0;
    vcloud_explosion.property_damage_level = 1;
    CSTD_safety_vcloud_explo_calc(&vcloud_explosion);
    LOG_DEBUG("death_r:%f",vcloud_explosion.death_r);
    LOG_DEBUG("explosion_energy:%f",vcloud_explosion.explosion_energy);
    LOG_DEBUG("TNT_equivalent:%f",vcloud_explosion.TNT_equivalent);
    LOG_DEBUG("serious_injury_r:%f",vcloud_explosion.serious_injury_r);
    LOG_DEBUG("slight_injury_r:%f",vcloud_explosion.slight_injury_r);
    LOG_DEBUG("property_damage_r:%f",vcloud_explosion.property_damage_r);

    LOG_DEBUG("==========CSTD_safety_vcloud_explosion test==========");
    CSTD_safety_explosive_explo_t explosive_explo;
    explosive_explo.explosion_heat = 4520;
    explosive_explo.mass = 50;
    explosive_explo.property_damage_level = 1;
    CSTD_safety_explosive_explo_calc(&explosive_explo);
    LOG_DEBUG("death_r:%f",explosive_explo.death_r);
    LOG_DEBUG("explosion_energy:%f",explosive_explo.explosion_energy);
    LOG_DEBUG("TNT_equivalent:%f",explosive_explo.TNT_equivalent);
    LOG_DEBUG("serious_injury_r:%f",explosive_explo.serious_injury_r);
    LOG_DEBUG("slight_injury_r:%f",explosive_explo.slight_injury_r);
    LOG_DEBUG("property_damage_r:%f",explosive_explo.property_damage_r);

    return 0;
}