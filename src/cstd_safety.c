#include <math.h>
#include <stdio.h>
#include "cstd_safety.h"

#define LOG_DEBUG(format,arg...) printf("%s,%d:"format"\n",__FILE__,__LINE__,##arg);

static inline double calc_slight_injury_combustion_heat( double combustion_dur);
static inline double calc_serious_injury_combustion_heat( double combustion_dur);
static inline double calc_death_combustion_heat( double combustion_dur);

static const double air_const = 8.314; //气体常数，J/mol*k
static double precision = 1e-6;
static const double E_value = 2.718281;
static const double pi = 3.14;
static const double air_density = 3.14;
static const double g_value = 9.8;

//计算至死亡热通量
static inline double calc_death_combustion_heat( double combustion_dur)
{
    double index = 0.0;
    double ret = 0.0;
    index = (5.0+37.23)/2.56;
    ret = pow(E_value,index);
    ret = pow(ret/combustion_dur,0.75);

    return ret;
}

//计算至重伤热通量
static inline double calc_serious_injury_combustion_heat( double combustion_dur)
{
    double index = 0.0;
    double ret = 0.0;
    index = (5.0+43.14)/3.0188;
    ret = pow(E_value,index);
    ret = pow(ret/combustion_dur,0.75);

    return ret;
}

//计算至轻伤热通量
static inline double calc_slight_injury_combustion_heat( double combustion_dur)
{
    double index = 0.0;
    double ret = 0.0;
    index = (5.0+39.83)/3.0186;
    ret = pow(E_value,index);
    ret = pow(ret/combustion_dur,0.75);

    return ret;
}

//double型比较
static int double_cmp( double *x1, double *x2 )
{
    if( *x1-*x2>precision )
    {
        return 1;
    }
    else if( *x2-*x1>precision )
    {
        return -1;
    }
    else if( *x2-*x1<=precision && *x1-*x2<=precision )
    {
        return 0;
    }
    return 1;
}

int CSTD_safety_gas_leak_calc(CSTD_safety_gas_leak_t *gas_leak)
{
    double r = 0.0;           //泄漏气体的绝热指数，定压热容/定容热容
    double p1 = 0.0;        //环境压强与容器压强之比
    double p11 = 0.0;
    double index = 0.0; 
    double ret = 0.0;
    double ret1 = 0.0;
    double ret3 = 0.0;
    // double ret4 = 0.0;
    double Y = 0.0;
    double Y4 = 0.0;
    double Y3 = 0.0;
    double Y2 = 0.0;
    double Y1 = 0.0;
    int ret2 = 0;

    if( 0.0 == gas_leak->const_vol_spec_heat || 0.0 == gas_leak->vessel_pressure )
    {
        return CSTD_safety_err_zero_divide;
    }
    r=gas_leak->const_press_spec_heat/gas_leak->const_vol_spec_heat; //泄漏气体的绝热指数，定压热容/定容热容
    p1 = CSTD_safety_atmos/gas_leak->vessel_pressure;

    if( 0.0 == r-1.0 )
        return CSTD_safety_err_zero_divide;

    index = r/(r-1.0);
    ret1 = 2/(r+1.0);
    ret3 = pow(ret1,index);
    ret2 = double_cmp( &p1, &ret3 );
    if( ret2 == -1 || ret2 == 0 )
    {
        gas_leak->leak_type = CSTD_safety_supersonic_leak;
    }
    else
    {
        gas_leak->leak_type = CSTD_safety_subsonic_leak;
    }

    if( gas_leak->leak_type == CSTD_safety_supersonic_leak )
    {
        index = (r+1.0)/(r-1.0);
        ret = pow(ret1,index);
        gas_leak->leak_rate = gas_leak->leak_coeff * gas_leak->gap_area * gas_leak->vessel_pressure * pow( (gas_leak->mol_wt*r)/(air_const*gas_leak->gas_temperature) * ret , 0.5 );
    }
    else
    {
        p11 = CSTD_safety_atmos/gas_leak->vessel_pressure;
        index = (r-1.0)/r;
        Y4 = pow(p11,index);
        Y4 = 1-Y4;
        index = 2/r;
        Y3 = pow(p1,index);
        index = (r+1.0)/(r-1.0);
        Y2 = pow(((r+1.0)/2.0), index);
        Y1 = 1.0/(r-1.0);
        Y = pow( Y1*Y2*Y3*Y4, 0.5 );

        index = (r+1.0)/(r-1.0);
        ret = pow(ret1,index);
        index = 1.0/(r-1.0);
        ret3 = pow( 2.0/(r+1.0), index );
        ret = pow( ( (gas_leak->mol_wt*r)/(air_const*gas_leak->gas_temperature) ) * ret * ret3, 0.5 );
        gas_leak->leak_rate = Y*gas_leak->leak_coeff*gas_leak->gap_area*gas_leak->vessel_pressure*ret;
    }

    return CSTD_safety_err_noerror;

}

int CSTD_safety_liquid_leak_calc(CSTD_safety_liquid_leak_t *liquid_leak)
{
    double ret = 0.0;
    double ret1 = 0.0;

    if(liquid_leak->pressure==0.0||liquid_leak->density == 0)
    {
        return CSTD_safety_err_zero_divide;
    }

    ret = liquid_leak->leak_coeff*liquid_leak->gap_area*liquid_leak->density;
    ret1 = 2*(liquid_leak->pressure-CSTD_safety_atmos)/liquid_leak->pressure+2*9.8*liquid_leak->height_diff;
    liquid_leak->leak_rate = ret1*pow(ret1,0.5);

    ret1 = pow(2*liquid_leak->pressure/liquid_leak->density+2*9.8*liquid_leak->height_diff,0.5) - (liquid_leak->density*9.8*liquid_leak->gap_area*liquid_leak->gap_area)*liquid_leak->leak_time;
    liquid_leak->instantaneous_rate = ret*ret1;

    return CSTD_safety_err_noerror;
}

int CSTD_safety_solid_fire_calc(CSTD_safety_solid_fire_t *solid_fire)
{
    double burn_time = 0.0;
    double left_data = 0.0;
    double index = 0.0;
    double ret = 0.0;
    double ret1 = 0.0;
    
    if( 0.0 == solid_fire->combustion_rate )
        return CSTD_safety_err_zero_divide;
    burn_time = solid_fire->mass/solid_fire->combustion_rate;
    
    //计算死亡半径
    index = (5.0+37.23)/2.56;
    left_data = pow(E_value,index);
    left_data = left_data/burn_time;
    left_data = pow(left_data,0.75);
    ret = 0.25*solid_fire->mass*solid_fire->combustion_heat;
    ret1 = 4*3.14*left_data;
    solid_fire->death_r = pow(ret/ret1,0.5);
    
    //计算重伤半径
    index = (5.0+43.14)/3.0188;
    left_data = pow(E_value,index);
    left_data = left_data/burn_time;
    left_data = pow(left_data,0.75);
    ret1 = 4*3.14*left_data;
    solid_fire->serious_injury_r = pow(ret/ret1,0.5);

    //计算轻伤半径
    index = (5.0+39.83)/3.0186;
    left_data = pow(E_value,index);
    left_data = left_data/burn_time;
    left_data = pow(left_data,0.75);
    ret1 = 4*3.14*left_data;
    solid_fire->slight_injury_r = pow(ret/ret1,0.5);

    //计算财产损失半径
    left_data = 6730*pow(burn_time,-0.8) + 25400;
    ret1 = 4*3.14*left_data;
    solid_fire->property_damage_r = pow(ret/ret1,0.5);

    return CSTD_safety_err_noerror;
}

int CSTD_safety_pool_fire_calc(CSTD_safety_pool_fire_t *pool_fire)
{
    double R = 0.0;
    double fire_height = 0.0;
    double fire_heat_flux = 0.0;
    double ret = 0.0;
    double ret1 = 0.0;
    double index = 0.0;
    double left_data = 0.0;
    double burn_dur = 0.0;

    if(pool_fire->combustion_rate == 0.0 )
    {
        return CSTD_safety_err_zero_divide;
    }

    R = 2 * pow(pool_fire->pool_area/pi,0.5);
    ret = pool_fire->combustion_rate /( air_density * pow(g_value*R,0.5) );
    ret = pow(ret,0.61);
    fire_height = 42*R*ret;
    ret = 0.25*pi*pow(R,2);
    ret=ret+pi*R*fire_height;
    ret=ret*pool_fire->combustion_rate*pool_fire->combustion_heat*0.25;
    ret1=72*pow(pool_fire->combustion_rate,0.61)+1;
    fire_heat_flux = ret/ret1;

    burn_dur = pool_fire->mass/pool_fire->combustion_rate;
    //计算死亡半径
    index = (5.0+37.23)/2.56;
    left_data = pow(E_value,index);
    left_data = left_data/burn_dur;
    left_data = pow(left_data,0.75);
    ret1 = left_data*4*pi;
    pool_fire->death_r = pow(fire_heat_flux/ret1,0.5);

    //计算重伤半径
    index = (5.0+43.14)/3.0188;
    left_data = pow(E_value,index);
    left_data = left_data/burn_dur;
    left_data = pow(left_data,0.75);
    ret1 = left_data*4*pi;
    pool_fire->serious_injury_r = pow(fire_heat_flux/ret1,0.5);

    //计算轻伤半径
    index = (5.0+39.83)/3.0186;
    left_data = pow(E_value,index);
    left_data = left_data/burn_dur;
    left_data = pow(left_data,0.75);
    ret1 = left_data*4*pi;
    pool_fire->slight_injury_r = pow(fire_heat_flux/ret1,0.5);

    //计算财产损失半径
    left_data = 6730*pow(burn_dur,-0.8) + 25400;
    ret1 = 4*pi*left_data;
    pool_fire->property_damage_r = pow(fire_heat_flux/ret1,0.5);

    return CSTD_safety_err_noerror;
}

int CSTD_safety_jet_fire_calc( CSTD_safety_jet_fire_t *jet_fire )
{
    double left_data = 0.0;
    double ret = 0.0;
    double ret1 = 0.0;
    double index = 0.0;

    jet_fire->gas_leak.leak_coeff = 1.0;
    jet_fire->gas_leak.gap_area = pi*pow(jet_fire->diameter,2.0)/4.0;

    //计算气体泄露模型
    CSTD_safety_gas_leak_calc(&jet_fire->gas_leak);

    //计算死亡半径
    index = (5.0+37.23)/2.56;
    left_data = pow(E_value,index);
    left_data = pow(left_data/jet_fire->combustion_dur,0.75);
    ret = 0.2*5*0.35*jet_fire->gas_leak.leak_rate*jet_fire->combustion_heat;
    ret1 = 4*pi*left_data;
    jet_fire->death_r = pow(ret/ret1,0.5);

    //计算重伤半径
    index = (5.0+43.14)/3.0188;
    left_data = pow(E_value,index);
    left_data = pow(left_data/jet_fire->combustion_dur,0.75);
    ret = 0.2*5*0.35*jet_fire->gas_leak.leak_rate*jet_fire->combustion_heat;
    ret1 = 4*pi*left_data;
    jet_fire->serious_injury_r = pow(ret/ret1,0.5);

    //计算轻伤半径
    index = (5.0+39.83)/3.0186;
    left_data = pow(E_value,index);
    left_data = pow(left_data/jet_fire->combustion_dur,0.75);
    ret = 0.2*5*0.35*jet_fire->gas_leak.leak_rate*jet_fire->combustion_heat;
    ret1 = 4*pi*left_data;
    jet_fire->slight_injury_r = pow(ret/ret1,0.5);

    //计算财产损失半径
    left_data = 6730*pow(jet_fire->combustion_dur,-0.8) + 25400;
    ret1 = 4*pi*left_data;
    ret = 0.2*5*0.35*jet_fire->gas_leak.leak_rate*jet_fire->combustion_heat;
    jet_fire->property_damage_r = pow(ret/ret1,0.5);

    return CSTD_safety_err_noerror;
}

int CSTD_safety_BLEVE_fire_calc(CSTD_safety_BLEVE_fire_t *BLEVE_fire)
{
    double r = 10.0;
    double deta_r = 0.0;
    double q_r = 0.0;           //热辐射通量
    // double scale = 0.0;
    double r2 = 0.0;
    double property_damage_q = 0.0;
    double combustion_heat = 0.0;
    int cmp_ret=0;
    int cmp_ret1=0;

    BLEVE_fire->fireball_r = 2.0*pow(BLEVE_fire->mass,0.3333);
    BLEVE_fire->combustion_dur = 0.45*pow(BLEVE_fire->mass,0.3333);
    r2 = pow( BLEVE_fire->fireball_r, 2 );
    //计算死亡半径
    r = BLEVE_fire->fireball_r*2;
    deta_r = BLEVE_fire->fireball_r;
    combustion_heat = calc_death_combustion_heat(BLEVE_fire->combustion_dur);
    
    do{
        q_r = BLEVE_fire->radiant_heat_flux*r2*r*(1-0.058*log(r))/pow(r2+r*r,1.5);
        cmp_ret=double_cmp(&q_r,&combustion_heat);
        if(cmp_ret==0)
        {
            BLEVE_fire->death_r = r;
            break;
        }
        else if(cmp_ret>0)
        {
            if( cmp_ret>0 && cmp_ret1<0 )
            {
                deta_r = deta_r/2;
            }
            r+=deta_r;
        }
        else
        {
            if( cmp_ret1>0 && cmp_ret<0 )
            {
                deta_r = deta_r/2;
            }
            r-=deta_r;
        }
        cmp_ret1 = cmp_ret;
    }while(1);

    //计算重伤半径
    combustion_heat = calc_serious_injury_combustion_heat(BLEVE_fire->combustion_dur);
    deta_r = r = BLEVE_fire->death_r;
    do{
        q_r = BLEVE_fire->radiant_heat_flux*r2*r*(1-0.0058*log(r))/pow(r2+r*r,1.5);
        cmp_ret=double_cmp(&q_r,&combustion_heat);
        if(cmp_ret==0)
        {
            BLEVE_fire->serious_injury_r = r;
            break;
        }
        else if(cmp_ret>0)
        {
            if( cmp_ret>0 && cmp_ret1<0 )
            {
                deta_r = deta_r/2;
            }
            r+=deta_r;
        }
        else
        {
            if( cmp_ret1>0 && cmp_ret<0 )
            {
                deta_r = deta_r/2;
            }
            r-=deta_r;
        }
        cmp_ret1 = cmp_ret;
    }while(1);

    //计算轻伤半径
    combustion_heat = calc_slight_injury_combustion_heat(BLEVE_fire->combustion_dur);
    deta_r = r = BLEVE_fire->serious_injury_r;
    do{
        q_r = BLEVE_fire->radiant_heat_flux*r2*r*(1-0.058*log(r))/pow(r2+r*r,1.5);
        cmp_ret=double_cmp(&q_r, &combustion_heat);
        if(cmp_ret==0)
        {
            BLEVE_fire->slight_injury_r = r;
            break;
        }
        else if(cmp_ret>0)
        {
            if( cmp_ret>0 && cmp_ret1<0 )
            {
                deta_r = deta_r/2;
            }
            r+=deta_r;
        }
        else
        {
            if( cmp_ret1>0 && cmp_ret<0 )
            {
                deta_r = deta_r/2;
            }
            r-=deta_r;
        }
        cmp_ret1 = cmp_ret;
    }while(1);

    //计算财产损失半径
    property_damage_q = 6730*pow(BLEVE_fire->combustion_dur,-0.8)+25400;
    r = BLEVE_fire->fireball_r*2;
    deta_r = BLEVE_fire->fireball_r;
    do{
        q_r = BLEVE_fire->radiant_heat_flux*r2*r*(1-0.058*log(r))/pow(r2+r*r,1.5);
        
        cmp_ret=double_cmp(&q_r, &property_damage_q);
        if(cmp_ret==0)
        {
            BLEVE_fire->property_damage_r = r;
            break;
        }
        else if(cmp_ret>0)
        {
            if( cmp_ret>0 && cmp_ret1<0 )
            {
                deta_r = deta_r/2;
            }
            r+=deta_r;
        }
        else
        {
            if( cmp_ret1>0 && cmp_ret<0 )
            {
                deta_r = deta_r/2;
            }
            r-=deta_r;
        }
        cmp_ret1 = cmp_ret;
    }while(1);

    return CSTD_safety_err_noerror;
}

static double explosion_damage_level[6] = {0.0,3.8,4.6,9.6,28,56};
int CSTD_safety_vcloud_explo_calc( CSTD_safety_vcloud_explo_t *vcloud_explosion)
{
    double deta_p = 0.0;
    double Z = 0.0;
    double q_r = 0.0;
    double deta_r = 0.0;
    double r = 0.0;
    int cmp_ret=0;
    int cmp_ret1=0;

    if (vcloud_explosion->property_damage_level>5 || vcloud_explosion->property_damage_level<1 )
    {
        printf("property_damage_level illegal.\n");
        return CSTD_safety_err_para_error;
    }

    vcloud_explosion->explosion_energy = 1.8*0.04*vcloud_explosion->mass*vcloud_explosion->combustion_heat;
    vcloud_explosion->TNT_equivalent = vcloud_explosion->explosion_energy/4520.0;
    vcloud_explosion->death_r = 13.6*pow(vcloud_explosion->TNT_equivalent/1000.0,0.37);

    //计算重伤半径
    deta_p = 44000/CSTD_safety_atmos;
    // deta_p = 44000.0;
    deta_r = r = vcloud_explosion->death_r;
    do{
        Z = r/pow( vcloud_explosion->explosion_energy*1000/CSTD_safety_atmos, 0.333333 );
        q_r = 0.137*pow(Z,-3) + 0.119*pow(Z,-2) + 0.269*pow(Z,-1) - 0.019;
        cmp_ret=double_cmp(&q_r,&deta_p);
        if(cmp_ret==0)
        {
            vcloud_explosion->serious_injury_r = r;
            break;
        }
        else if(cmp_ret>0)
        {
            if( cmp_ret>0 && cmp_ret1<0 )
            {
                deta_r = deta_r/2;
            }
            r+=deta_r;
        }
        else
        {
            if( cmp_ret1>0 && cmp_ret<0 )
            {
                deta_r = deta_r/2;
            }
            r-=deta_r;
        }
        cmp_ret1 = cmp_ret;
    }while(1);

    //计算轻伤半径
    deta_p = 17000/CSTD_safety_atmos;
    // deta_p = 17000;
    deta_r = r = vcloud_explosion->serious_injury_r;
    do{
        Z = r/pow( vcloud_explosion->explosion_energy*1000/CSTD_safety_atmos, 0.333333 );
        q_r = 0.137*pow(Z,-3) + 0.119*pow(Z,-2) + 0.269*pow(Z,-1) - 0.019;
        cmp_ret=double_cmp(&q_r,&deta_p);
        if(cmp_ret==0)
        {
            vcloud_explosion->slight_injury_r = r;
            break;
        }
        else if(cmp_ret>0)
        {
            if( cmp_ret>0 && cmp_ret1<0 )
            {
                deta_r = deta_r/2;
            }
            r+=deta_r;
        }
        else
        {
            if( cmp_ret1>0 && cmp_ret<0 )
            {
                deta_r = deta_r/2;
            }
            r-=deta_r;
        }
        cmp_ret1 = cmp_ret;
    }while(1);

    vcloud_explosion->property_damage_r = explosion_damage_level[vcloud_explosion->property_damage_level]*pow(vcloud_explosion->TNT_equivalent,0.333333)/pow(1+( pow(3175/vcloud_explosion->TNT_equivalent,2) ), 1/6);

    return 0;
}

int CSTD_safety_explosive_explo_calc(CSTD_safety_explosive_explo_t *explosive_explo)
{
    double deta_p = 0.0;
    double Z = 0.0;
    double q_r = 0.0;
    double deta_r = 0.0;
    double r = 0.0;
    int cmp_ret=0;
    int cmp_ret1=0;

    if (explosive_explo->property_damage_level>5 || explosive_explo->property_damage_level<1 )
    {
        printf("property_damage_level illegal.\n");
        return CSTD_safety_err_para_error;
    }

    explosive_explo->explosion_energy = 1.8 * explosive_explo->mass * explosive_explo->explosion_heat;
    explosive_explo->TNT_equivalent = explosive_explo->explosion_energy/4520.0;

    //死亡半径
    explosive_explo->death_r = 13.6*pow(explosive_explo->TNT_equivalent/1000.0,0.37);

    //计算重伤半径
    deta_p = 44000/CSTD_safety_atmos;
    deta_r = r = explosive_explo->death_r;
    do{
        Z = r/pow( explosive_explo->explosion_energy*1000/CSTD_safety_atmos, 0.333333 );
        q_r = 0.137*pow(Z,-3) + 0.119*pow(Z,-2) + 0.269/Z - 0.019;
        cmp_ret=double_cmp(&q_r,&deta_p);
        if(cmp_ret==0)
        {
            explosive_explo->serious_injury_r = r;
            break;
        }
        else if(cmp_ret>0)
        {
            if( cmp_ret>0 && cmp_ret1<0 )
            {
                deta_r = deta_r/2;
            }
            r+=deta_r;
        }
        else
        {
            if( cmp_ret1>0 && cmp_ret<0 )
            {
                deta_r = deta_r/2;
            }
            r-=deta_r;
        }
        cmp_ret1 = cmp_ret;
    }while(1);

    //计算轻伤半径
    deta_p = 17000/CSTD_safety_atmos;
    deta_r = r = explosive_explo->serious_injury_r;
    do{
        Z = r/pow( explosive_explo->explosion_energy*1000/CSTD_safety_atmos, 0.333333 );
        q_r = 0.137*pow(Z,-3) + 0.119*pow(Z,-2) + 0.269/Z - 0.019;
        cmp_ret=double_cmp(&q_r,&deta_p);
        if(cmp_ret==0)
        {
            explosive_explo->slight_injury_r = r;
            break;
        }
        else if(cmp_ret>0)
        {
            if( cmp_ret>0 && cmp_ret1<0 )
            {
                deta_r = deta_r/2;
            }
            r+=deta_r;
        }
        else
        {
            if( cmp_ret1>0 && cmp_ret<0 )
            {
                deta_r = deta_r/2;
            }
            r-=deta_r;
        }
        cmp_ret1 = cmp_ret;
    }while(1);

    explosive_explo->property_damage_r = explosion_damage_level[explosive_explo->property_damage_level]*pow(explosive_explo->TNT_equivalent,0.333333)/pow(1+((3175*3175)/(explosive_explo->TNT_equivalent*explosive_explo->TNT_equivalent)),1/6);

    return CSTD_safety_err_noerror;
}

int CSTD_safety_gpress_vessel_explo_calc(CSTD_safety_gpress_vessel_explo_t *press_explo)
{
    double index = 0.0;
    double ret = 0.0;
    double r = 0.0;
    double Z = 0.0;
    double q_r = 0.0;
    double deta_p = 0.0;
    double deta_r = 0.0;
    int cmp_ret = 0;
    int cmp_ret1 = 0;

    if (press_explo->property_damage_level>5 || press_explo->property_damage_level<1 )
    {
        printf("property_damage_level illegal.\n");
        return CSTD_safety_err_para_error;
    }

    if(press_explo->adiabatic_ind==0.0||press_explo->abs_press==0.0)
    {
        return CSTD_safety_err_zero_divide;
    }
    index = (press_explo->adiabatic_ind-1)/press_explo->adiabatic_ind;
    ret = pow(0.1013/press_explo->abs_press,index);
    ret = 1-ret;
    press_explo->explosion_energy = 1000*press_explo->abs_press*press_explo->volume*ret/(press_explo->adiabatic_ind-1);
    press_explo->TNT_equivalent = press_explo->explosion_energy/4520.0;

    //死亡半径
    press_explo->death_r = 13.6*pow(press_explo->TNT_equivalent/1000.0,0.37);
    //计算重伤半径
    deta_p = 44000/CSTD_safety_atmos;
    deta_r = r = press_explo->death_r;
    
    do{
        Z = r/pow( press_explo->explosion_energy*1000/CSTD_safety_atmos, 0.333333 );
        q_r = 0.137*pow(Z,-3) + 0.119*pow(Z,-2) + 0.269/Z - 0.019;
        cmp_ret=double_cmp(&q_r,&deta_p);
        if(cmp_ret==0)
        {
            press_explo->serious_injury_r = r;
            break;
        }
        else if(cmp_ret>0)
        {
            if( cmp_ret>0 && cmp_ret1<0 )
            {
                deta_r = deta_r/2;
            }
            r+=deta_r;
        }
        else
        {
            if( cmp_ret1>0 && cmp_ret<0 )
            {
                deta_r = deta_r/2;
            }
            r-=deta_r;
        }
        cmp_ret1 = cmp_ret;
    }while(1);

    //计算轻伤半径
    deta_p = 17000/CSTD_safety_atmos;
    deta_r = r = press_explo->serious_injury_r;
    do{
        Z = r/pow( press_explo->explosion_energy*1000/CSTD_safety_atmos, 0.333333 );
        q_r = 0.137*pow(Z,-3) + 0.119*pow(Z,-2) + 0.269/Z - 0.019;
        cmp_ret=double_cmp(&q_r,&deta_p);
        if(cmp_ret==0)
        {
            press_explo->slight_injury_r = r;
            break;
        }
        else if(cmp_ret>0)
        {
            if( cmp_ret>0 && cmp_ret1<0 )
            {
                deta_r = deta_r/2;
            }
            r+=deta_r;
        }
        else
        {
            if( cmp_ret1>0 && cmp_ret<0 )
            {
                deta_r = deta_r/2;
            }
            r-=deta_r;
        }
        cmp_ret1 = cmp_ret;
    }while(1);

    press_explo->property_damage_r = explosion_damage_level[press_explo->property_damage_level]*pow(press_explo->TNT_equivalent,0.333333)/pow(1+((3175*3175)/(press_explo->TNT_equivalent*press_explo->TNT_equivalent)),1/6);

    return CSTD_safety_err_noerror;
}

int CSTD_safety_lpress_vessel_explo_calc( CSTD_safety_lpress_vessel_explo_t *press_explo )
{
    double r = 0.0;
    double Z = 0.0;
    double q_r = 0.0;
    double deta_p = 0.0;
    double deta_r = 0.0;
    int cmp_ret = 0;
    int cmp_ret1 = 0;

    if (press_explo->property_damage_level>5 || press_explo->property_damage_level<1 )
    {
        printf("property_damage_level illegal.\n");
        return CSTD_safety_err_para_error;
    }

    press_explo->explosion_energy = pow(press_explo->abs_press-0.1013,2)*press_explo->volume*press_explo->compress_coeff*50000;

    press_explo->TNT_equivalent = press_explo->explosion_energy/4520.0;

    //死亡半径
    press_explo->death_r = 13.6*pow(press_explo->TNT_equivalent/1000.0,0.37);
    //计算重伤半径
    deta_p = 44000/CSTD_safety_atmos;
    deta_r = r = press_explo->death_r;
    do{
        Z = r/pow( press_explo->explosion_energy*1000/CSTD_safety_atmos, 0.333333 );
        q_r = 0.137*pow(Z,-3) + 0.119*pow(Z,-2) + 0.269/Z - 0.019;
        cmp_ret=double_cmp(&q_r,&deta_p);
        if(cmp_ret==0)
        {
            press_explo->serious_injury_r = r;
            break;
        }
        else if(cmp_ret>0)
        {
            if( cmp_ret>0 && cmp_ret1<0 )
            {
                deta_r = deta_r/2;
            }
            r+=deta_r;
        }
        else
        {
            if( cmp_ret1>0 && cmp_ret<0 )
            {
                deta_r = deta_r/2;
            }
            r-=deta_r;
        }
        cmp_ret1 = cmp_ret;
    }while(1);

    //计算轻伤半径
    deta_p = 17000/CSTD_safety_atmos;
    deta_r = r = press_explo->serious_injury_r;
    do{
        Z = r/pow( press_explo->explosion_energy*1000/CSTD_safety_atmos, 0.333333 );
        q_r = 0.137*pow(Z,-3) + 0.119*pow(Z,-2) + 0.269/Z - 0.019;
        cmp_ret=double_cmp(&q_r,&deta_p);
        if(cmp_ret==0)
        {
            press_explo->slight_injury_r = r;
            break;
        }
        else if(cmp_ret>0)
        {
            if( cmp_ret>0 && cmp_ret1<0 )
            {
                deta_r = deta_r/2;
            }
            r+=deta_r;
        }
        else
        {
            if( cmp_ret1>0 && cmp_ret<0 )
            {
                deta_r = deta_r/2;
            }
            r-=deta_r;
        }
        cmp_ret1 = cmp_ret;
    }while(1);

    press_explo->property_damage_r = explosion_damage_level[press_explo->property_damage_level]*pow(press_explo->TNT_equivalent,0.333333)/pow(1+((3175*3175)/(press_explo->TNT_equivalent*press_explo->TNT_equivalent)),1/6);

    return CSTD_safety_err_noerror;
}

int CSTD_safety_lgpress_vessel_explo_calc( CSTD_safety_lgpress_vessel_explo_t *press_explo )
{
    double r = 0.0;
    double Z = 0.0;
    double q_r = 0.0;
    double deta_p = 0.0;
    double deta_r = 0.0;
    int cmp_ret = 0;
    int cmp_ret1 = 0;

    if (press_explo->property_damage_level>5 || press_explo->property_damage_level<1 )
    {
        printf("property_damage_level illegal.\n");
        return CSTD_safety_err_para_error;
    }

    press_explo->explosion_energy = ( (press_explo->ethalpy-press_explo->normal_ethalpy)-(press_explo->ehtropy-press_explo->normal_ehtropy)*press_explo->boiling_point)*press_explo->mass;

    press_explo->TNT_equivalent = press_explo->explosion_energy/4520.0;

    //死亡半径
    press_explo->death_r = 13.6*pow(press_explo->TNT_equivalent/1000.0,0.37);
    //计算重伤半径
    deta_p = 44000/CSTD_safety_atmos;
    deta_r = r = press_explo->death_r;
    do{
        Z = r/pow( press_explo->explosion_energy*1000/CSTD_safety_atmos, 0.333333 );
        q_r = 0.137*pow(Z,-3) + 0.119*pow(Z,-2) + 0.269/Z - 0.019;
        cmp_ret=double_cmp(&q_r,&deta_p);
        if(cmp_ret==0)
        {
            press_explo->serious_injury_r = r;
            break;
        }
        else if(cmp_ret>0)
        {
            if( cmp_ret>0 && cmp_ret1<0 )
            {
                deta_r = deta_r/2;
            }
            r+=deta_r;
        }
        else
        {
            if( cmp_ret1>0 && cmp_ret<0 )
            {
                deta_r = deta_r/2;
            }
            r-=deta_r;
        }
        cmp_ret1 = cmp_ret;
    }while(1);

    //计算轻伤半径
    deta_p = 17000/CSTD_safety_atmos;
    deta_r = r = press_explo->serious_injury_r;
    do{
        Z = r/pow( press_explo->explosion_energy*1000/CSTD_safety_atmos, 0.333333 );
        q_r = 0.137*pow(Z,-3) + 0.119*pow(Z,-2) + 0.269/Z - 0.019;
        cmp_ret=double_cmp(&q_r,&deta_p);
        if(cmp_ret==0)
        {
            press_explo->slight_injury_r = r;
            break;
        }
        else if(cmp_ret>0)
        {
            if( cmp_ret>0 && cmp_ret1<0 )
            {
                deta_r = deta_r/2;
            }
            r+=deta_r;
        }
        else
        {
            if( cmp_ret1>0 && cmp_ret<0 )
            {
                deta_r = deta_r/2;
            }
            r-=deta_r;
        }
        cmp_ret1 = cmp_ret;
    }while(1);

    press_explo->property_damage_r = explosion_damage_level[press_explo->property_damage_level]*pow(press_explo->TNT_equivalent,0.333333)/pow(1+((3175*3175)/(press_explo->TNT_equivalent*press_explo->TNT_equivalent)),1/6);

    return CSTD_safety_err_noerror;
}

int CSTD_safety_satwater_vessel_explo_calc( CSTD_safety_satwater_vessel_explo_t *press_explo )
{
    double r = 0.0;
    double Z = 0.0;
    double q_r = 0.0;
    double deta_p = 0.0;
    double deta_r = 0.0;
    int cmp_ret = 0;
    int cmp_ret1 = 0;

    if (press_explo->property_damage_level>5 || press_explo->property_damage_level<1 )
    {
        printf("property_damage_level illegal.\n");
        return CSTD_safety_err_para_error;
    }

    press_explo->explosion_energy = press_explo->coeff*press_explo->volume;

    press_explo->TNT_equivalent = press_explo->explosion_energy/4520.0;

    //死亡半径
    press_explo->death_r = 13.6*pow(press_explo->TNT_equivalent/1000.0,0.37);
    //计算重伤半径
    deta_p = 44000/CSTD_safety_atmos;
    deta_r = r = press_explo->death_r;
    do{
        Z = r/pow( press_explo->explosion_energy*1000/CSTD_safety_atmos, 0.333333 );
        q_r = 0.137*pow(Z,-3) + 0.119*pow(Z,-2) + 0.269/Z - 0.019;
        cmp_ret=double_cmp(&q_r,&deta_p);
        if(cmp_ret==0)
        {
            press_explo->serious_injury_r = r;
            break;
        }
        else if(cmp_ret>0)
        {
            if( cmp_ret>0 && cmp_ret1<0 )
            {
                deta_r = deta_r/2;
            }
            r+=deta_r;
        }
        else
        {
            if( cmp_ret1>0 && cmp_ret<0 )
            {
                deta_r = deta_r/2;
            }
            r-=deta_r;
        }
        cmp_ret1 = cmp_ret;
    }while(1);

    //计算轻伤半径
    deta_p = 17000/CSTD_safety_atmos;
    deta_r = r = press_explo->serious_injury_r;
    do{
        Z = r/pow( press_explo->explosion_energy*1000/CSTD_safety_atmos, 0.333333 );
        q_r = 0.137*pow(Z,-3) + 0.119*pow(Z,-2) + 0.269/Z - 0.019;
        cmp_ret=double_cmp(&q_r,&deta_p);
        if(cmp_ret==0)
        {
            press_explo->slight_injury_r = r;
            break;
        }
        else if(cmp_ret>0)
        {
            if( cmp_ret>0 && cmp_ret1<0 )
            {
                deta_r = deta_r/2;
            }
            r+=deta_r;
        }
        else
        {
            if( cmp_ret1>0 && cmp_ret<0 )
            {
                deta_r = deta_r/2;
            }
            r-=deta_r;
        }
        cmp_ret1 = cmp_ret;
    }while(1);

    press_explo->property_damage_r = explosion_damage_level[press_explo->property_damage_level]*pow(press_explo->TNT_equivalent,0.333333)/pow(1+((3175*3175)/(press_explo->TNT_equivalent*press_explo->TNT_equivalent)),1/6);

    return CSTD_safety_err_noerror;
}
