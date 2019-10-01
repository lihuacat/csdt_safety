#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "verify.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <fcntl.h>
#endif

#include "cstd_safety.h"
#ifndef WIN32
#define LOG_DEBUG(format, arg...) printf("%s,%d:" format "\n", __FILE__, __LINE__, ##arg);
#else
#define LOG_DEBUG printf("\n");printf
#endif

    static inline double calc_slight_injury_combustion_heat(double combustion_dur);
    static inline double calc_serious_injury_combustion_heat(double combustion_dur);
    static inline double calc_death_combustion_heat(double combustion_dur);

    static const double air_const = 8.314; //气体常数，J/mol*k
    static double precision = 1e-6;
    static const double E_value = 2.718281;
    static const double pi = 3.14;
    static const double air_density = 3.14;
    static const double g_value = 9.8;
    static int verified = 0;

    //计算至死亡热通量
    static inline double calc_death_combustion_heat(double combustion_dur)
    {
        double index = 0.0;
        double ret = 0.0;
        index = (5.0 + 37.23) / 2.56;
        ret = pow(E_value, index);
        ret = pow(ret / combustion_dur, 0.75);

        return ret;
    }

    //计算至重伤热通量
    static inline double calc_serious_injury_combustion_heat(double combustion_dur)
    {
        double index = 0.0;
        double ret = 0.0;
        index = (5.0 + 43.14) / 3.0188;
        ret = pow(E_value, index);
        ret = pow(ret / combustion_dur, 0.75);

        return ret;
    }

    //计算至轻伤热通量
    static inline double calc_slight_injury_combustion_heat(double combustion_dur)
    {
        double index = 0.0;
        double ret = 0.0;
        index = (5.0 + 39.83) / 3.0186;
        ret = pow(E_value, index);
        ret = pow(ret / combustion_dur, 0.75);

        return ret;
    }

    //double型比较
    static int double_cmp(double *x1, double *x2)
    {
        if (*x1 - *x2 > precision)
        {
            return 1;
        }
        else if (*x2 - *x1 > precision)
        {
            return -1;
        }
        else if (*x2 - *x1 <= precision && *x1 - *x2 <= precision)
        {
            return 0;
        }
        return 1;
    }

    int CSTD_safety_gas_leak_calc(CSTD_safety_gas_leak_t *gas_leak)
    {
        double r = 0.0;  //泄漏气体的绝热指数，定压热容/定容热容
        double p1 = 0.0; //环境压强与容器压强之比
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

        if (!verified)
            return CSTD_safety_err_no_auth;

        if (0.0 == gas_leak->const_vol_spec_heat || 0.0 == gas_leak->vessel_pressure)
        {
            return CSTD_safety_err_zero_divide;
        }
        r = gas_leak->const_press_spec_heat / gas_leak->const_vol_spec_heat; //泄漏气体的绝热指数，定压热容/定容热容
        p1 = CSTD_safety_atmos / gas_leak->vessel_pressure;

        if (0.0 == r - 1.0)
            return CSTD_safety_err_zero_divide;

        index = r / (r - 1.0);
        ret1 = 2 / (r + 1.0);
        ret3 = pow(ret1, index);
        ret2 = double_cmp(&p1, &ret3);
        if (ret2 == -1 || ret2 == 0)
        {
            gas_leak->leak_type = CSTD_safety_supersonic_leak;
        }
        else
        {
            gas_leak->leak_type = CSTD_safety_subsonic_leak;
        }

        if (gas_leak->leak_type == CSTD_safety_supersonic_leak)
        {
            index = (r + 1.0) / (r - 1.0);
            ret = pow(ret1, index);
            gas_leak->leak_rate = gas_leak->leak_coeff * gas_leak->gap_area * gas_leak->vessel_pressure * pow((gas_leak->mol_wt * r) / (air_const * gas_leak->gas_temperature) * ret, 0.5);
        }
        else
        {
            p11 = CSTD_safety_atmos / gas_leak->vessel_pressure;
            index = (r - 1.0) / r;
            Y4 = pow(p11, index);
            Y4 = 1 - Y4;
            index = 2 / r;
            Y3 = pow(p1, index);
            index = (r + 1.0) / (r - 1.0);
            Y2 = pow(((r + 1.0) / 2.0), index);
            Y1 = 1.0 / (r - 1.0);
            Y = pow(Y1 * Y2 * Y3 * Y4, 0.5);

            index = (r + 1.0) / (r - 1.0);
            ret = pow(ret1, index);
            index = 1.0 / (r - 1.0);
            ret3 = pow(2.0 / (r + 1.0), index);
            ret = pow(((gas_leak->mol_wt * r) / (air_const * gas_leak->gas_temperature)) * ret * ret3, 0.5);
            gas_leak->leak_rate = Y * gas_leak->leak_coeff * gas_leak->gap_area * gas_leak->vessel_pressure * ret;
        }

        return CSTD_safety_err_noerror;
    }

    int CSTD_safety_liquid_leak_calc(CSTD_safety_liquid_leak_t *liquid_leak)
    {
        double ret = 0.0;
        double ret1 = 0.0;

        if (!verified)
            return CSTD_safety_err_no_auth;

        if (liquid_leak->pressure == 0.0 || liquid_leak->density == 0)
        {
            return CSTD_safety_err_zero_divide;
        }

        ret = liquid_leak->leak_coeff * liquid_leak->gap_area * liquid_leak->density;
        ret1 = 2 * (liquid_leak->pressure - CSTD_safety_atmos) / liquid_leak->pressure + 2 * 9.8 * liquid_leak->height_diff;
        liquid_leak->leak_rate = ret1 * pow(ret1, 0.5);

        ret1 = pow(2 * liquid_leak->pressure / liquid_leak->density + 2 * 9.8 * liquid_leak->height_diff, 0.5) - (liquid_leak->density * 9.8 * liquid_leak->gap_area * liquid_leak->gap_area) * liquid_leak->leak_time;
        liquid_leak->instantaneous_rate = ret * ret1;

        return CSTD_safety_err_noerror;
    }

    int CSTD_safety_solid_fire_calc(CSTD_safety_solid_fire_t *solid_fire)
    {
        double burn_time = 0.0;
        double left_data = 0.0;
        double index = 0.0;
        double ret = 0.0;
        double ret1 = 0.0;

        if (!verified)
            return CSTD_safety_err_no_auth;

        if (0.0 == solid_fire->combustion_rate)
            return CSTD_safety_err_zero_divide;
        burn_time = solid_fire->mass / solid_fire->combustion_rate;

        //计算死亡半径
        index = (5.0 + 37.23) / 2.56;
        left_data = pow(E_value, index);
        left_data = left_data / burn_time;
        left_data = pow(left_data, 0.75);
        ret = 0.25 * solid_fire->mass * solid_fire->combustion_heat;
        ret1 = 4 * 3.14 * left_data;
        solid_fire->death_r = pow(ret / ret1, 0.5);

        //计算重伤半径
        index = (5.0 + 43.14) / 3.0188;
        left_data = pow(E_value, index);
        left_data = left_data / burn_time;
        left_data = pow(left_data, 0.75);
        ret1 = 4 * 3.14 * left_data;
        solid_fire->serious_injury_r = pow(ret / ret1, 0.5);

        //计算轻伤半径
        index = (5.0 + 39.83) / 3.0186;
        left_data = pow(E_value, index);
        left_data = left_data / burn_time;
        left_data = pow(left_data, 0.75);
        ret1 = 4 * 3.14 * left_data;
        solid_fire->slight_injury_r = pow(ret / ret1, 0.5);

        //计算财产损失半径
        left_data = 6730 * pow(burn_time, -0.8) + 25400;
        ret1 = 4 * 3.14 * left_data;
        solid_fire->property_damage_r = pow(ret / ret1, 0.5);

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

        if (!verified)
            return CSTD_safety_err_no_auth;

        if (pool_fire->combustion_rate == 0.0)
        {
            return CSTD_safety_err_zero_divide;
        }

        R = 2 * pow(pool_fire->pool_area / pi, 0.5);
        ret = pool_fire->combustion_rate / (air_density * pow(g_value * R, 0.5));
        ret = pow(ret, 0.61);
        fire_height = 42 * R * ret;
        ret = 0.25 * pi * pow(R, 2);
        ret = ret + pi * R * fire_height;
        ret = ret * pool_fire->combustion_rate * pool_fire->combustion_heat * 0.25;
        ret1 = 72 * pow(pool_fire->combustion_rate, 0.61) + 1;
        fire_heat_flux = ret / ret1;

        burn_dur = pool_fire->mass / pool_fire->combustion_rate;
        //计算死亡半径
        index = (5.0 + 37.23) / 2.56;
        left_data = pow(E_value, index);
        left_data = left_data / burn_dur;
        left_data = pow(left_data, 0.75);
        ret1 = left_data * 4 * pi;
        pool_fire->death_r = pow(fire_heat_flux / ret1, 0.5);

        //计算重伤半径
        index = (5.0 + 43.14) / 3.0188;
        left_data = pow(E_value, index);
        left_data = left_data / burn_dur;
        left_data = pow(left_data, 0.75);
        ret1 = left_data * 4 * pi;
        pool_fire->serious_injury_r = pow(fire_heat_flux / ret1, 0.5);

        //计算轻伤半径
        index = (5.0 + 39.83) / 3.0186;
        left_data = pow(E_value, index);
        left_data = left_data / burn_dur;
        left_data = pow(left_data, 0.75);
        ret1 = left_data * 4 * pi;
        pool_fire->slight_injury_r = pow(fire_heat_flux / ret1, 0.5);

        //计算财产损失半径
        left_data = 6730 * pow(burn_dur, -0.8) + 25400;
        ret1 = 4 * pi * left_data;
        pool_fire->property_damage_r = pow(fire_heat_flux / ret1, 0.5);

        return CSTD_safety_err_noerror;
    }

    int CSTD_safety_jet_fire_calc(CSTD_safety_jet_fire_t *jet_fire)
    {
        double left_data = 0.0;
        double ret = 0.0;
        double ret1 = 0.0;
        double index = 0.0;

        if (!verified)
            return CSTD_safety_err_no_auth;

        jet_fire->gas_leak.leak_coeff = 1.0;
        jet_fire->gas_leak.gap_area = pi * pow(jet_fire->diameter, 2.0) / 4.0;

        //计算气体泄露模型
        CSTD_safety_gas_leak_calc(&jet_fire->gas_leak);

        //计算死亡半径
        index = (5.0 + 37.23) / 2.56;
        left_data = pow(E_value, index);
        left_data = pow(left_data / jet_fire->combustion_dur, 0.75);
        ret = 0.2 * 5 * 0.35 * jet_fire->gas_leak.leak_rate * jet_fire->combustion_heat;
        ret1 = 4 * pi * left_data;
        jet_fire->death_r = pow(ret / ret1, 0.5);

        //计算重伤半径
        index = (5.0 + 43.14) / 3.0188;
        left_data = pow(E_value, index);
        left_data = pow(left_data / jet_fire->combustion_dur, 0.75);
        ret = 0.2 * 5 * 0.35 * jet_fire->gas_leak.leak_rate * jet_fire->combustion_heat;
        ret1 = 4 * pi * left_data;
        jet_fire->serious_injury_r = pow(ret / ret1, 0.5);

        //计算轻伤半径
        index = (5.0 + 39.83) / 3.0186;
        left_data = pow(E_value, index);
        left_data = pow(left_data / jet_fire->combustion_dur, 0.75);
        ret = 0.2 * 5 * 0.35 * jet_fire->gas_leak.leak_rate * jet_fire->combustion_heat;
        ret1 = 4 * pi * left_data;
        jet_fire->slight_injury_r = pow(ret / ret1, 0.5);

        //计算财产损失半径
        left_data = 6730 * pow(jet_fire->combustion_dur, -0.8) + 25400;
        ret1 = 4 * pi * left_data;
        ret = 0.2 * 5 * 0.35 * jet_fire->gas_leak.leak_rate * jet_fire->combustion_heat;
        jet_fire->property_damage_r = pow(ret / ret1, 0.5);

        return CSTD_safety_err_noerror;
    }

    int CSTD_safety_BLEVE_fire_calc(CSTD_safety_BLEVE_fire_t *BLEVE_fire)
    {
        double r = 10.0;
        double deta_r = 0.0;
        double q_r = 0.0; //热辐射通量
        // double scale = 0.0;
        double r2 = 0.0;
        double property_damage_q = 0.0;
        double combustion_heat = 0.0;
        int cmp_ret = 0;
        int cmp_ret1 = 0;

        if (!verified)
            return CSTD_safety_err_no_auth;

        BLEVE_fire->fireball_r = 2.0 * pow(BLEVE_fire->mass, 0.3333);
        BLEVE_fire->combustion_dur = 0.45 * pow(BLEVE_fire->mass, 0.3333);
        r2 = pow(BLEVE_fire->fireball_r, 2);
        //计算死亡半径
        r = BLEVE_fire->fireball_r * 2;
        deta_r = BLEVE_fire->fireball_r;
        combustion_heat = calc_death_combustion_heat(BLEVE_fire->combustion_dur);

        do
        {
            q_r = BLEVE_fire->radiant_heat_flux * r2 * r * (1 - 0.058 * log(r)) / pow(r2 + r * r, 1.5);
            cmp_ret = double_cmp(&q_r, &combustion_heat);
            if (cmp_ret == 0)
            {
                BLEVE_fire->death_r = r;
                break;
            }
            else if (cmp_ret > 0)
            {
                if (cmp_ret > 0 && cmp_ret1 < 0)
                {
                    deta_r = deta_r / 2;
                }
                r += deta_r;
            }
            else
            {
                if (cmp_ret1 > 0 && cmp_ret < 0)
                {
                    deta_r = deta_r / 2;
                }
                r -= deta_r;
            }
            cmp_ret1 = cmp_ret;
        } while (1);

        //计算重伤半径
        combustion_heat = calc_serious_injury_combustion_heat(BLEVE_fire->combustion_dur);
        deta_r = r = BLEVE_fire->death_r;
        do
        {
            q_r = BLEVE_fire->radiant_heat_flux * r2 * r * (1 - 0.0058 * log(r)) / pow(r2 + r * r, 1.5);
            cmp_ret = double_cmp(&q_r, &combustion_heat);
            if (cmp_ret == 0)
            {
                BLEVE_fire->serious_injury_r = r;
                break;
            }
            else if (cmp_ret > 0)
            {
                if (cmp_ret > 0 && cmp_ret1 < 0)
                {
                    deta_r = deta_r / 2;
                }
                r += deta_r;
            }
            else
            {
                if (cmp_ret1 > 0 && cmp_ret < 0)
                {
                    deta_r = deta_r / 2;
                }
                r -= deta_r;
            }
            cmp_ret1 = cmp_ret;
        } while (1);

        //计算轻伤半径
        combustion_heat = calc_slight_injury_combustion_heat(BLEVE_fire->combustion_dur);
        deta_r = r = BLEVE_fire->serious_injury_r;
        do
        {
            q_r = BLEVE_fire->radiant_heat_flux * r2 * r * (1 - 0.058 * log(r)) / pow(r2 + r * r, 1.5);
            cmp_ret = double_cmp(&q_r, &combustion_heat);
            if (cmp_ret == 0)
            {
                BLEVE_fire->slight_injury_r = r;
                break;
            }
            else if (cmp_ret > 0)
            {
                if (cmp_ret > 0 && cmp_ret1 < 0)
                {
                    deta_r = deta_r / 2;
                }
                r += deta_r;
            }
            else
            {
                if (cmp_ret1 > 0 && cmp_ret < 0)
                {
                    deta_r = deta_r / 2;
                }
                r -= deta_r;
            }
            cmp_ret1 = cmp_ret;
        } while (1);

        //计算财产损失半径
        property_damage_q = 6730 * pow(BLEVE_fire->combustion_dur, -0.8) + 25400;
        r = BLEVE_fire->fireball_r * 2;
        deta_r = BLEVE_fire->fireball_r;
        do
        {
            q_r = BLEVE_fire->radiant_heat_flux * r2 * r * (1 - 0.058 * log(r)) / pow(r2 + r * r, 1.5);

            cmp_ret = double_cmp(&q_r, &property_damage_q);
            if (cmp_ret == 0)
            {
                BLEVE_fire->property_damage_r = r;
                break;
            }
            else if (cmp_ret > 0)
            {
                if (cmp_ret > 0 && cmp_ret1 < 0)
                {
                    deta_r = deta_r / 2;
                }
                r += deta_r;
            }
            else
            {
                if (cmp_ret1 > 0 && cmp_ret < 0)
                {
                    deta_r = deta_r / 2;
                }
                r -= deta_r;
            }
            cmp_ret1 = cmp_ret;
        } while (1);

        return CSTD_safety_err_noerror;
    }

    static double explosion_damage_level[6] = {0.0, 3.8, 4.6, 9.6, 28, 56};
    int CSTD_safety_vcloud_explo_calc(CSTD_safety_vcloud_explo_t *vcloud_explosion)
    {
        double deta_p = 0.0;
        double Z = 0.0;
        double q_r = 0.0;
        double deta_r = 0.0;
        double r = 0.0;
        int cmp_ret = 0;
        int cmp_ret1 = 0;

        if (!verified)
            return CSTD_safety_err_no_auth;

        if (vcloud_explosion->property_damage_level > 5 || vcloud_explosion->property_damage_level < 1)
        {
            printf("property_damage_level illegal.\n");
            return CSTD_safety_err_para_error;
        }

        vcloud_explosion->explosion_energy = 1.8 * 0.04 * vcloud_explosion->mass * vcloud_explosion->combustion_heat;
        vcloud_explosion->TNT_equivalent = vcloud_explosion->explosion_energy / 4520.0;
        vcloud_explosion->death_r = 13.6 * pow(vcloud_explosion->TNT_equivalent / 1000.0, 0.37);

        //计算重伤半径
        deta_p = 44000 / CSTD_safety_atmos;
        // deta_p = 44000.0;
        deta_r = r = vcloud_explosion->death_r;
        do
        {
            Z = r / pow(vcloud_explosion->explosion_energy * 1000 / CSTD_safety_atmos, 0.333333);
            q_r = 0.137 * pow(Z, -3) + 0.119 * pow(Z, -2) + 0.269 * pow(Z, -1) - 0.019;
            cmp_ret = double_cmp(&q_r, &deta_p);
            if (cmp_ret == 0)
            {
                vcloud_explosion->serious_injury_r = r;
                break;
            }
            else if (cmp_ret > 0)
            {
                if (cmp_ret > 0 && cmp_ret1 < 0)
                {
                    deta_r = deta_r / 2;
                }
                r += deta_r;
            }
            else
            {
                if (cmp_ret1 > 0 && cmp_ret < 0)
                {
                    deta_r = deta_r / 2;
                }
                r -= deta_r;
            }
            cmp_ret1 = cmp_ret;
        } while (1);

        //计算轻伤半径
        deta_p = 17000 / CSTD_safety_atmos;
        // deta_p = 17000;
        deta_r = r = vcloud_explosion->serious_injury_r;
        do
        {
            Z = r / pow(vcloud_explosion->explosion_energy * 1000 / CSTD_safety_atmos, 0.333333);
            q_r = 0.137 * pow(Z, -3) + 0.119 * pow(Z, -2) + 0.269 * pow(Z, -1) - 0.019;
            cmp_ret = double_cmp(&q_r, &deta_p);
            if (cmp_ret == 0)
            {
                vcloud_explosion->slight_injury_r = r;
                break;
            }
            else if (cmp_ret > 0)
            {
                if (cmp_ret > 0 && cmp_ret1 < 0)
                {
                    deta_r = deta_r / 2;
                }
                r += deta_r;
            }
            else
            {
                if (cmp_ret1 > 0 && cmp_ret < 0)
                {
                    deta_r = deta_r / 2;
                }
                r -= deta_r;
            }
            cmp_ret1 = cmp_ret;
        } while (1);

        vcloud_explosion->property_damage_r = explosion_damage_level[vcloud_explosion->property_damage_level] * pow(vcloud_explosion->TNT_equivalent, 0.333333) / pow(1 + (pow(3175 / vcloud_explosion->TNT_equivalent, 2)), 1 / 6);

        return 0;
    }

    int CSTD_safety_explosive_explo_calc(CSTD_safety_explosive_explo_t *explosive_explo)
    {
        double deta_p = 0.0;
        double Z = 0.0;
        double q_r = 0.0;
        double deta_r = 0.0;
        double r = 0.0;
        int cmp_ret = 0;
        int cmp_ret1 = 0;

        if (!verified)
            return CSTD_safety_err_no_auth;

        if (explosive_explo->property_damage_level > 5 || explosive_explo->property_damage_level < 1)
        {
            printf("property_damage_level illegal.\n");
            return CSTD_safety_err_para_error;
        }

        explosive_explo->explosion_energy = 1.8 * explosive_explo->mass * explosive_explo->explosion_heat;
        explosive_explo->TNT_equivalent = explosive_explo->explosion_energy / 4520.0;

        //死亡半径
        explosive_explo->death_r = 13.6 * pow(explosive_explo->TNT_equivalent / 1000.0, 0.37);

        //计算重伤半径
        deta_p = 44000 / CSTD_safety_atmos;
        deta_r = r = explosive_explo->death_r;
        do
        {
            Z = r / pow(explosive_explo->explosion_energy * 1000 / CSTD_safety_atmos, 0.333333);
            q_r = 0.137 * pow(Z, -3) + 0.119 * pow(Z, -2) + 0.269 / Z - 0.019;
            cmp_ret = double_cmp(&q_r, &deta_p);
            if (cmp_ret == 0)
            {
                explosive_explo->serious_injury_r = r;
                break;
            }
            else if (cmp_ret > 0)
            {
                if (cmp_ret > 0 && cmp_ret1 < 0)
                {
                    deta_r = deta_r / 2;
                }
                r += deta_r;
            }
            else
            {
                if (cmp_ret1 > 0 && cmp_ret < 0)
                {
                    deta_r = deta_r / 2;
                }
                r -= deta_r;
            }
            cmp_ret1 = cmp_ret;
        } while (1);

        //计算轻伤半径
        deta_p = 17000 / CSTD_safety_atmos;
        deta_r = r = explosive_explo->serious_injury_r;
        do
        {
            Z = r / pow(explosive_explo->explosion_energy * 1000 / CSTD_safety_atmos, 0.333333);
            q_r = 0.137 * pow(Z, -3) + 0.119 * pow(Z, -2) + 0.269 / Z - 0.019;
            cmp_ret = double_cmp(&q_r, &deta_p);
            if (cmp_ret == 0)
            {
                explosive_explo->slight_injury_r = r;
                break;
            }
            else if (cmp_ret > 0)
            {
                if (cmp_ret > 0 && cmp_ret1 < 0)
                {
                    deta_r = deta_r / 2;
                }
                r += deta_r;
            }
            else
            {
                if (cmp_ret1 > 0 && cmp_ret < 0)
                {
                    deta_r = deta_r / 2;
                }
                r -= deta_r;
            }
            cmp_ret1 = cmp_ret;
        } while (1);

        explosive_explo->property_damage_r = explosion_damage_level[explosive_explo->property_damage_level] * pow(explosive_explo->TNT_equivalent, 0.333333) / pow(1 + ((3175 * 3175) / (explosive_explo->TNT_equivalent * explosive_explo->TNT_equivalent)), 1 / 6);

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

        if (!verified)
            return CSTD_safety_err_no_auth;

        if (press_explo->property_damage_level > 5 || press_explo->property_damage_level < 1)
        {
            printf("property_damage_level illegal.\n");
            return CSTD_safety_err_para_error;
        }

        if (press_explo->adiabatic_ind == 0.0 || press_explo->abs_press == 0.0)
        {
            return CSTD_safety_err_zero_divide;
        }
        index = (press_explo->adiabatic_ind - 1) / press_explo->adiabatic_ind;
        ret = pow(0.1013 / press_explo->abs_press, index);
        ret = 1 - ret;
        press_explo->explosion_energy = 1000 * press_explo->abs_press * press_explo->volume * ret / (press_explo->adiabatic_ind - 1);
        press_explo->TNT_equivalent = press_explo->explosion_energy / 4520.0;

        //死亡半径
        press_explo->death_r = 13.6 * pow(press_explo->TNT_equivalent / 1000.0, 0.37);
        //计算重伤半径
        deta_p = 44000 / CSTD_safety_atmos;
        deta_r = r = press_explo->death_r;

        do
        {
            Z = r / pow(press_explo->explosion_energy * 1000 / CSTD_safety_atmos, 0.333333);
            q_r = 0.137 * pow(Z, -3) + 0.119 * pow(Z, -2) + 0.269 / Z - 0.019;
            cmp_ret = double_cmp(&q_r, &deta_p);
            if (cmp_ret == 0)
            {
                press_explo->serious_injury_r = r;
                break;
            }
            else if (cmp_ret > 0)
            {
                if (cmp_ret > 0 && cmp_ret1 < 0)
                {
                    deta_r = deta_r / 2;
                }
                r += deta_r;
            }
            else
            {
                if (cmp_ret1 > 0 && cmp_ret < 0)
                {
                    deta_r = deta_r / 2;
                }
                r -= deta_r;
            }
            cmp_ret1 = cmp_ret;
        } while (1);

        //计算轻伤半径
        deta_p = 17000 / CSTD_safety_atmos;
        deta_r = r = press_explo->serious_injury_r;
        do
        {
            Z = r / pow(press_explo->explosion_energy * 1000 / CSTD_safety_atmos, 0.333333);
            q_r = 0.137 * pow(Z, -3) + 0.119 * pow(Z, -2) + 0.269 / Z - 0.019;
            cmp_ret = double_cmp(&q_r, &deta_p);
            if (cmp_ret == 0)
            {
                press_explo->slight_injury_r = r;
                break;
            }
            else if (cmp_ret > 0)
            {
                if (cmp_ret > 0 && cmp_ret1 < 0)
                {
                    deta_r = deta_r / 2;
                }
                r += deta_r;
            }
            else
            {
                if (cmp_ret1 > 0 && cmp_ret < 0)
                {
                    deta_r = deta_r / 2;
                }
                r -= deta_r;
            }
            cmp_ret1 = cmp_ret;
        } while (1);

        press_explo->property_damage_r = explosion_damage_level[press_explo->property_damage_level] * pow(press_explo->TNT_equivalent, 0.333333) / pow(1 + ((3175 * 3175) / (press_explo->TNT_equivalent * press_explo->TNT_equivalent)), 1 / 6);

        return CSTD_safety_err_noerror;
    }

    int CSTD_safety_lpress_vessel_explo_calc(CSTD_safety_lpress_vessel_explo_t *press_explo)
    {
        double r = 0.0;
        double Z = 0.0;
        double q_r = 0.0;
        double deta_p = 0.0;
        double deta_r = 0.0;
        int cmp_ret = 0;
        int cmp_ret1 = 0;

        if (!verified)
            return CSTD_safety_err_no_auth;

        if (press_explo->property_damage_level > 5 || press_explo->property_damage_level < 1)
        {
            printf("property_damage_level illegal.\n");
            return CSTD_safety_err_para_error;
        }

        press_explo->explosion_energy = pow(press_explo->abs_press - 0.1013, 2) * press_explo->volume * press_explo->compress_coeff * 50000;

        press_explo->TNT_equivalent = press_explo->explosion_energy / 4520.0;

        //死亡半径
        press_explo->death_r = 13.6 * pow(press_explo->TNT_equivalent / 1000.0, 0.37);
        //计算重伤半径
        deta_p = 44000 / CSTD_safety_atmos;
        deta_r = r = press_explo->death_r;
        do
        {
            Z = r / pow(press_explo->explosion_energy * 1000 / CSTD_safety_atmos, 0.333333);
            q_r = 0.137 * pow(Z, -3) + 0.119 * pow(Z, -2) + 0.269 / Z - 0.019;
            cmp_ret = double_cmp(&q_r, &deta_p);
            if (cmp_ret == 0)
            {
                press_explo->serious_injury_r = r;
                break;
            }
            else if (cmp_ret > 0)
            {
                if (cmp_ret > 0 && cmp_ret1 < 0)
                {
                    deta_r = deta_r / 2;
                }
                r += deta_r;
            }
            else
            {
                if (cmp_ret1 > 0 && cmp_ret < 0)
                {
                    deta_r = deta_r / 2;
                }
                r -= deta_r;
            }
            cmp_ret1 = cmp_ret;
        } while (1);

        //计算轻伤半径
        deta_p = 17000 / CSTD_safety_atmos;
        deta_r = r = press_explo->serious_injury_r;
        do
        {
            Z = r / pow(press_explo->explosion_energy * 1000 / CSTD_safety_atmos, 0.333333);
            q_r = 0.137 * pow(Z, -3) + 0.119 * pow(Z, -2) + 0.269 / Z - 0.019;
            cmp_ret = double_cmp(&q_r, &deta_p);
            if (cmp_ret == 0)
            {
                press_explo->slight_injury_r = r;
                break;
            }
            else if (cmp_ret > 0)
            {
                if (cmp_ret > 0 && cmp_ret1 < 0)
                {
                    deta_r = deta_r / 2;
                }
                r += deta_r;
            }
            else
            {
                if (cmp_ret1 > 0 && cmp_ret < 0)
                {
                    deta_r = deta_r / 2;
                }
                r -= deta_r;
            }
            cmp_ret1 = cmp_ret;
        } while (1);

        press_explo->property_damage_r = explosion_damage_level[press_explo->property_damage_level] * pow(press_explo->TNT_equivalent, 0.333333) / pow(1 + ((3175 * 3175) / (press_explo->TNT_equivalent * press_explo->TNT_equivalent)), 1 / 6);

        return CSTD_safety_err_noerror;
    }

    int CSTD_safety_lgpress_vessel_explo_calc(CSTD_safety_lgpress_vessel_explo_t *press_explo)
    {
        double r = 0.0;
        double Z = 0.0;
        double q_r = 0.0;
        double deta_p = 0.0;
        double deta_r = 0.0;
        int cmp_ret = 0;
        int cmp_ret1 = 0;

        if (!verified)
            return CSTD_safety_err_no_auth;

        if (press_explo->property_damage_level > 5 || press_explo->property_damage_level < 1)
        {
            printf("property_damage_level illegal.\n");
            return CSTD_safety_err_para_error;
        }

        press_explo->explosion_energy = ((press_explo->ethalpy - press_explo->normal_ethalpy) - (press_explo->ehtropy - press_explo->normal_ehtropy) * press_explo->boiling_point) * press_explo->mass;

        press_explo->TNT_equivalent = press_explo->explosion_energy / 4520.0;

        //死亡半径
        press_explo->death_r = 13.6 * pow(press_explo->TNT_equivalent / 1000.0, 0.37);
        //计算重伤半径
        deta_p = 44000 / CSTD_safety_atmos;
        deta_r = r = press_explo->death_r;
        do
        {
            Z = r / pow(press_explo->explosion_energy * 1000 / CSTD_safety_atmos, 0.333333);
            q_r = 0.137 * pow(Z, -3) + 0.119 * pow(Z, -2) + 0.269 / Z - 0.019;
            cmp_ret = double_cmp(&q_r, &deta_p);
            if (cmp_ret == 0)
            {
                press_explo->serious_injury_r = r;
                break;
            }
            else if (cmp_ret > 0)
            {
                if (cmp_ret > 0 && cmp_ret1 < 0)
                {
                    deta_r = deta_r / 2;
                }
                r += deta_r;
            }
            else
            {
                if (cmp_ret1 > 0 && cmp_ret < 0)
                {
                    deta_r = deta_r / 2;
                }
                r -= deta_r;
            }
            cmp_ret1 = cmp_ret;
        } while (1);

        //计算轻伤半径
        deta_p = 17000 / CSTD_safety_atmos;
        deta_r = r = press_explo->serious_injury_r;
        do
        {
            Z = r / pow(press_explo->explosion_energy * 1000 / CSTD_safety_atmos, 0.333333);
            q_r = 0.137 * pow(Z, -3) + 0.119 * pow(Z, -2) + 0.269 / Z - 0.019;
            cmp_ret = double_cmp(&q_r, &deta_p);
            if (cmp_ret == 0)
            {
                press_explo->slight_injury_r = r;
                break;
            }
            else if (cmp_ret > 0)
            {
                if (cmp_ret > 0 && cmp_ret1 < 0)
                {
                    deta_r = deta_r / 2;
                }
                r += deta_r;
            }
            else
            {
                if (cmp_ret1 > 0 && cmp_ret < 0)
                {
                    deta_r = deta_r / 2;
                }
                r -= deta_r;
            }
            cmp_ret1 = cmp_ret;
        } while (1);

        press_explo->property_damage_r = explosion_damage_level[press_explo->property_damage_level] * pow(press_explo->TNT_equivalent, 0.333333) / pow(1 + ((3175 * 3175) / (press_explo->TNT_equivalent * press_explo->TNT_equivalent)), 1 / 6);

        return CSTD_safety_err_noerror;
    }

    int CSTD_safety_satwater_vessel_explo_calc(CSTD_safety_satwater_vessel_explo_t *press_explo)
    {
        double r = 0.0;
        double Z = 0.0;
        double q_r = 0.0;
        double deta_p = 0.0;
        double deta_r = 0.0;
        int cmp_ret = 0;
        int cmp_ret1 = 0;

        if (!verified)
            return CSTD_safety_err_no_auth;

        if (press_explo->property_damage_level > 5 || press_explo->property_damage_level < 1)
        {
            printf("property_damage_level illegal.\n");
            return CSTD_safety_err_para_error;
        }

        press_explo->explosion_energy = press_explo->coeff * press_explo->volume;

        press_explo->TNT_equivalent = press_explo->explosion_energy / 4520.0;

        //死亡半径
        press_explo->death_r = 13.6 * pow(press_explo->TNT_equivalent / 1000.0, 0.37);
        //计算重伤半径
        deta_p = 44000 / CSTD_safety_atmos;
        deta_r = r = press_explo->death_r;
        do
        {
            Z = r / pow(press_explo->explosion_energy * 1000 / CSTD_safety_atmos, 0.333333);
            q_r = 0.137 * pow(Z, -3) + 0.119 * pow(Z, -2) + 0.269 / Z - 0.019;
            cmp_ret = double_cmp(&q_r, &deta_p);
            if (cmp_ret == 0)
            {
                press_explo->serious_injury_r = r;
                break;
            }
            else if (cmp_ret > 0)
            {
                if (cmp_ret > 0 && cmp_ret1 < 0)
                {
                    deta_r = deta_r / 2;
                }
                r += deta_r;
            }
            else
            {
                if (cmp_ret1 > 0 && cmp_ret < 0)
                {
                    deta_r = deta_r / 2;
                }
                r -= deta_r;
            }
            cmp_ret1 = cmp_ret;
        } while (1);

        //计算轻伤半径
        deta_p = 17000 / CSTD_safety_atmos;
        deta_r = r = press_explo->serious_injury_r;
        do
        {
            Z = r / pow(press_explo->explosion_energy * 1000 / CSTD_safety_atmos, 0.333333);
            q_r = 0.137 * pow(Z, -3) + 0.119 * pow(Z, -2) + 0.269 / Z - 0.019;
            cmp_ret = double_cmp(&q_r, &deta_p);
            if (cmp_ret == 0)
            {
                press_explo->slight_injury_r = r;
                break;
            }
            else if (cmp_ret > 0)
            {
                if (cmp_ret > 0 && cmp_ret1 < 0)
                {
                    deta_r = deta_r / 2;
                }
                r += deta_r;
            }
            else
            {
                if (cmp_ret1 > 0 && cmp_ret < 0)
                {
                    deta_r = deta_r / 2;
                }
                r -= deta_r;
            }
            cmp_ret1 = cmp_ret;
        } while (1);

        press_explo->property_damage_r = explosion_damage_level[press_explo->property_damage_level] * pow(press_explo->TNT_equivalent, 0.333333) / pow(1 + ((3175 * 3175) / (press_explo->TNT_equivalent * press_explo->TNT_equivalent)), 1 / 6);

        return CSTD_safety_err_noerror;
    }

    int CSTD_authorize(const char *file_name)
    {
        if (verify_auth(file_name) == 0)
        {
            verified = 1;
            return 0;
        }

        return CSTD_safety_err_no_auth;
    }

    /*********************************************************************************
                                     扩散参数表
sigma_y  σ=γ1xα1
稳定度        α1            γ1          下风距离(m)
A            0.901074     0.425809      0~1000
             0.850934     0.602052      ＞1000
B            0.914370     0.281846      0~1000
             0.865014     0.396353      ＞1000
B－C         0.919325     0.229500      0~1000
             0.875086     0.314238      ＞1000
C            0.924279     0.177154      1~1000
             0.885157     0.232123      ＞1000
C－D         0.926849     0.143940      1~1000
             0.886940     0.189396      ＞1000
D            0.929418     0.110726      1~1000
             0.888723     0.146669      ＞1000
D－E         0.925118     0.0985631     1~1000
             0.892794     0.124308      ＞1000
E            0.920818     0.0864001     1~1000
             0.896864     0.101947      ＞1000
F            0.929418     0.0553634     0~1000
             0.888723     0.0733348     ＞1000

sigma_z  σz=γ2xα2 
稳定度        α2            γ2          下风距离(m)
A            1.12154      0.0799904     0~300
             1.51360      0.00854771    300~500
			 2.10881      0.000211545   >500
B            0.964435     0.127190      0~500
             1.09356      0.057025      >500
B－C         0.941015     0.114682      0~500
             1.00770      0.0757182     >500
C            0.917595     0.106803      >0
C－D         0.838628     0.126152      0~2000
             0.756410     0.235667      2000~10000
			 0.815575     0.136659      >10000
D            0.826212     0.104634      1~1000
             0.632023     0.400167      1000~10000
			 0.55536      0.810763      >10000
D－E         0.776864     0.111771      0~2000
             0.572347     0.5289922     2000~10000
			 0.499149     1.03810       >10000
E            0.788370     0.0927529     0~1000
             0.565188     0.433384      1000~10000
			 0.414743     1.73421       >10000
F            0.784400     0.0620765     0~1000
             0.525969     0.370015      1000~10000
			 0.322659     2.40691       >10000
**********************************************************************************/
    int stab_y_steps[] = {1000};
    int stab_z_steps[] = {300, 500, 2000, 10000, 1000, 10000};
    double atmo_stab_a_y[] = {0.901074, 0.850934, 0.914370, 0.865014, 0.919325, 0.875086, 0.924279, 0.885157, 0.926849, 0.886940, 0.929418, 0.888723, 0.925118, 0.892794, 0.920818, 0.896864, 0.929418, 0.888723};
    double atmo_stab_r_y[] = {0.425809, 0.602052, 0.281846, 0.396353, 0.229500, 0.314238, 0.177154, 0.232123, 0.143940, 0.189396, 0.110726, 0.146669, 0.0985631, 0.124308, 0.0864001, 0.101947, 0.0553634, 0.0733348};
    static int get_sigma_y(int atmo_stab, int distance, double *sigma_y)
    {
        int ind1 = -1;
        int ind2 = -1;

        if (distance <= 0 || atmo_stab > CSTD_atmo_stab_F || atmo_stab < CSTD_atmo_stab_A)
        {
            return -1;
        }
        if (distance > 1000)
        {
			ind2 = 1;
        }
        else
        {
            ind2 = 0;
        }
        ind1 = 2 * atmo_stab + ind2;
        *sigma_y = pow(atmo_stab_r_y[ind1], atmo_stab_a_y[ind1]);

        return 0;
    }

    //double atmo_stab_a_z[] = {1.12154, 1.51360, 2.10881, 0.964435, 1.09356, 0.941015, 1.00770, 0.917595, 0.838628, 0.756410, 0.815575, 0.826212, 0.632023, 0.55536, 0.776864, 0.572347, 0.499149, 0.788370, 0.565188, 0.414743, 0.784400, 0.525969, 0.322659};
    //double atmo_stab_r_z[] = {0.0799904, 0.00854771, 0.000211545, 0.127190, 0.057025, 0.114682, 0.0757182, 0.106803, 0.126152, 0.235667, 0.136659, 0.104634, 0.400167, 0.810763, 0.111771, 0.5289922, 1.03810, 0.0927529, 0.433384, 1.73421, 0.0620765, 0.370015, 2.40691};
    static int get_sigma_z(int atmo_stab, int distance, double *sigma_z)
    {
        int index = -1;
        if (distance <= 0 || atmo_stab > CSTD_atmo_stab_F || atmo_stab < CSTD_atmo_stab_A)
        {
            return -1;
        }

        switch (atmo_stab)
        {
        case CSTD_atmo_stab_A:
            if (distance <= 300 && distance > 0)
            {
                index = 0;
            }
            else if (distance <= 500 && distance > 300)
            {
                index = 1;
            }
            else
            {
                index = 2;
            }
            break;
        case CSTD_atmo_stab_B:
            if (distance <= 500 && distance > 0)
            {
                index = 3;
            }
            else
            {
                index = 4;
            }
            break;

        case CSTD_atmo_stab_BC:
            if (distance <= 500 && distance > 0)
            {
                index = 5;
            }
            else
            {
                index = 6;
            }
            break;

        case CSTD_atmo_stab_C:
            index = 7;
            break;

        case CSTD_atmo_stab_CD:
            if (distance <= 2000 && distance > 0)
            {
                index = 8;
            }
            else if (distance <= 10000 && distance > 2000)
            {
                index = 9;
            }
            else
            {
                index = 10;
            }
            break;

        case CSTD_atmo_stab_D:
            if (distance <= 1000 && distance > 0)
            {
                index = 11;
            }
            else if (distance <= 10000 && distance > 1000)
            {
                index = 12;
            }
            else
            {
                index = 13;
            }
            break;

        case CSTD_atmo_stab_DE:
            if (distance <= 2000 && distance > 0)
            {
                index = 14;
            }
            else if (distance <= 10000 && distance > 2000)
            {
                index = 15;
            }
            else
            {
                index = 16;
            }
            break;

        case CSTD_atmo_stab_E:
            if (distance <= 1000 && distance > 0)
            {
                index = 17;
            }
            else if (distance <= 10000 && distance > 1000)
            {
                index = 18;
            }
            else
            {
                index = 19;
            }
            break;

        case CSTD_atmo_stab_F:
            if (distance <= 1000 && distance > 0)
            {
                index = 20;
            }
            else if (distance <= 10000 && distance > 1000)
            {
                index = 21;
            }
            else
            {
                index = 22;
            }
            break;

        default:
            break;
        }

        *sigma_z = pow(atmo_stab_r_y[index], atmo_stab_a_y[index]);

        return 0;
    }
/* 
    int static get_atmo_stab_y_distance_steps(int atmo_stab, int **steps)
    {
		*steps = &stab_y_steps[0]; return 1;
    }

    int static get_atmo_stab_z_distance_steps(int atmo_stab, int **steps)
    {
        if (atmo_stab > CSTD_atmo_stab_F || atmo_stab < CSTD_atmo_stab_A)
        {
            return -1;
        }

        switch (atmo_stab)
        {
        case CSTD_atmo_stab_A:
			*steps = &stab_z_steps[0]; return 2;

        case CSTD_atmo_stab_B:
			*steps = &stab_z_steps[1]; return 1;

        case CSTD_atmo_stab_BC:
			*steps = &stab_z_steps[1]; return 1;

        case CSTD_atmo_stab_C:
            return 0;

        case CSTD_atmo_stab_CD:
			*steps = &stab_z_steps[2]; return 2;

        case CSTD_atmo_stab_D:
			*steps = &stab_z_steps[4]; return 2;

        case CSTD_atmo_stab_DE:
			*steps = &stab_z_steps[2]; return 2;

        case CSTD_atmo_stab_E:
			*steps = &stab_z_steps[4]; return 2;

        case CSTD_atmo_stab_F:
			*steps = &stab_z_steps[4]; return 2;

        default:
            break;
        }

        return -1;
    }
*/
    CSTD_point_2d_t *CSTD_safety_Gaussian_plume_calc(double conce, int stability, double emission_rate, int height, int wind_speed, int *ret)
    {
        // int *steps = NULL;
        // int ret1 = 0;
        int distance_x = 0;
        int distance_y = 0;
        int i = 0;
        double sigma_y = 0.0;
        double sigma_z = 0.0;
        double c;
        double num1;
        double num2;
        int in = 0;
        CSTD_point_2d_t *points = NULL;
        //get z step;
        // ret1 = get_atmo_stab_z_distance_steps(stability, &steps);

        int point_len = 2;
        int point_used = 0;

        // for( i=0; i<ret1; i++ ) //遍历steps
        // {
        for (distance_x = 1; distance_x <= 50000; distance_x++ ) //距离不超过50千米
        {
            LOG_DEBUG("distance_x:%d", distance_x);
            //取到sigma_y, sigma_z
            get_sigma_y(stability, distance_x, &sigma_y);
            get_sigma_z(stability, distance_x, &sigma_z);
            // LOG_DEBUG("sigma_y:%f", sigma_y);
            // LOG_DEBUG("sigma_z:%f", sigma_z);
            num1 = (pi * sigma_z * sigma_y * wind_speed);
            num2 = -(height * height) / (sigma_z * sigma_z * 2);
            LOG_DEBUG("num1:%f", num1);
            LOG_DEBUG("num2:%f", num2);
            LOG_DEBUG("emission_rate / num1:%f", emission_rate / num1);
            LOG_DEBUG("pow(E_value, num2):%f", pow(E_value, num2));
            c = (emission_rate / num1) * pow(E_value, num2);
            LOG_DEBUG("conce:%f", conce);
            LOG_DEBUG("c:%f", c);
            if (in == 0 && double_cmp(&conce, &c) == 1)
            {
                continue;
            }
            in = 1;
            if (in == 1 && double_cmp(&conce, &c) == 1)
            {
                break;
            }
            //找y方向的边界
            for (distance_y = 0; distance_y < 5000; distance_y++ )
            {
                c = ( emission_rate / num1 ) * exp( -( (distance_y * distance_y) / (2 * sigma_y * sigma_y) ) + num2 );
                LOG_DEBUG("distance_y:%d", distance_y);
                if ( double_cmp(&c, &conce) > 0 )
                {
                    continue;
                }
                if (!points)
                {
                    points = calloc(2 , sizeof(CSTD_point_2d_t));
                    memset(points, 0x00, 2 * sizeof(CSTD_point_2d_t) );
                }
                points[point_used].x = distance_x;
                points[point_used].y = distance_y;
                LOG_DEBUG("distance_x:%d", distance_x);
                LOG_DEBUG("distance_y:%d", distance_y);
                point_used++;
                if ( point_len - point_used < 2 )
                {
                    CSTD_point_2d_t * new_points = calloc(2* point_len , sizeof(CSTD_point_2d_t));
                    memset(new_points, 0x00, 2* point_len * sizeof(CSTD_point_2d_t));
                    memcpy(new_points, points, point_len * sizeof(CSTD_point_2d_t));
                    free(points);
                    points = new_points;
                    point_len *= 2;
                }
                break;
            }
        }

        // }

        int n = point_used;
        for (i = n-1; i>=0; i--)
        {
            if (points[i].y > 0)
            {
                points[point_used].x = points[i].x;
                points[point_used].y = -points[i].y;
                point_used++;
                if (point_len - point_used < 2)
                {
                    CSTD_point_2d_t * new_points = calloc(2* point_len , sizeof(CSTD_point_2d_t));
                    memset(new_points, 0x00, 2* point_len * sizeof(CSTD_point_2d_t));
                    memcpy(new_points, points, point_len * sizeof(CSTD_point_2d_t));
                    free(points);
                    points = new_points;
                    point_len *= 2;
                }
            }
        }

        *ret = point_used;

        return points;
    }

    void CSTD_safety_free( void *p)
    {
        if( p != NULL ) {
            free( p );
        }
    }

#ifdef __cplusplus
}
#endif
