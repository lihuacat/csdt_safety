#ifndef __CSTD_SAFETY_H__
#define __CSTD_SAFETY_H__

enum
{
    CSTD_safety_err_noerror = 0,
    CSTD_safety_err_zero_divide = -1,
    CSTD_safety_err_para_error = -2,
};

//气体泄漏模型
struct CSTD_safety_gas_leak
{
    double gap_area;                         //裂口面积，单位平方米
    double gas_temperature;                 //气体温度，单位K
    double const_press_spec_heat;           //定压热容
    double const_vol_spec_heat;              //定容热容
    double leak_coeff;                      //气体的泄漏系数
    double mol_wt;                         //气体分子量
    double vessel_pressure;               //容器内介质压力，单位pa

    int leak_type;                         //泄露类型
    double leak_rate;                     //泄露速度
};
typedef struct CSTD_safety_gas_leak  CSTD_safety_gas_leak_t;

//气体的泄漏系数
#define CSTD_safety_leak_coeff_round = (double)1.00     //圆形泄露孔
#define CSTD_safety_leak_coeff_triangle = (double)0.95   //三角形泄露孔
#define CSTD_safety_leak_coeff_rectangle = (double)0.90   //矩形泄露孔
//气体的泄漏系数 end

//标准大气压
#define CSTD_safety_atmos (double)101300.00

enum
{
    CSTD_safety_subsonic_leak = 0,     //亚音速泄露
    CSTD_safety_supersonic_leak = 1,    //超音速泄露
};
//气体泄漏模型 end

//液体泄露模型
struct CSTD_safety_liquid_leak
{
    double gap_area;                         //裂口面积，单位平方米
    double height_diff;                     //裂口之上液位高度，单位m
    double leak_coeff;                      //泄漏系数
    double leak_time;                       //泄漏时间，s
    double density;                         //泄漏液体密度，kg/m^3
    double vessel_CSA;                      //容器横截面积
    double pressure;                        //设备内物质压力，pa
    
    double instantaneous_rate;            //瞬时泄露速度
    double leak_rate;                     //泄露速度
};
typedef struct CSTD_safety_liquid_leak  CSTD_safety_liquid_leak_t;
//液体泄露模型 end

struct CSTD_safety_solid_fire
{
    double combustion_heat;       //燃烧热，J/kg
    double combustion_rate;       //燃烧速度，kg/s
    double mass;                  //质量，kg

    double death_r;           //死亡半径
    double serious_injury_r;  //重伤半径
    double slight_injury_r;   //轻伤半径
    double property_damage_r;  //财产损失半径
};
typedef struct CSTD_safety_solid_fire  CSTD_safety_solid_fire_t;

//池火模型
struct CSTD_safety_pool_fire
{
    double combustion_heat;       //燃烧热，J/kg
    double combustion_rate;       //燃烧速度，kg/s
    double mass;                  //质量，kg
    double pool_area;             //液池面积，m^2

    double death_r;           //死亡半径
    double serious_injury_r;  //重伤半径
    double slight_injury_r;   //轻伤半径
    double property_damage_r;  //财产损失半径
};
typedef struct CSTD_safety_pool_fire  CSTD_safety_pool_fire_t;

struct CSTD_safety_jet_fire
{
    CSTD_safety_gas_leak_t gas_leak;
    double combustion_heat;   //燃烧热
    double combustion_dur;   //燃烧时间
    double diameter;         //泄露口直径

    double death_r;           //死亡半径
    double serious_injury_r;  //重伤半径
    double slight_injury_r;   //轻伤半径
    double property_damage_r;  //财产损失半径
};
typedef struct CSTD_safety_jet_fire CSTD_safety_jet_fire_t;

//沸腾液体扩展蒸气爆炸模型
struct CSTD_safety_BLEVE_fire
{
    double mass;              //质量
    double radiant_heat_flux; //火球表面的热辐射通量，对柱形罐可取270kW/m^2；对球形罐可取200kW/m^2

    double fireball_r;         //火球半径，m
    double combustion_dur;     //燃烧时间
    double death_r;           //死亡半径
    double serious_injury_r;  //重伤半径
    double slight_injury_r;   //轻伤半径
    double property_damage_r;  //财产损失半径
};
typedef struct CSTD_safety_BLEVE_fire CSTD_safety_BLEVE_fire_t;
#define radiant_heat_flux_sphere (double)200.0
#define radiant_heat_flux_pillar (double)270.0

//蒸汽云（vapor cloud）爆炸模型
struct CSTD_safety_vcloud_explo
{
    double combustion_heat;   //燃烧热
    double mass;              //质量
    int property_damage_level; //要计算的财产损失等级，取值范围：{1,2,3,4,5},财产损失等级的划分表的第一列

    double explosion_energy;  //爆炸释放能量
    double TNT_equivalent;    //TNT当量
    double death_r;           //死亡半径
    double serious_injury_r;  //重伤半径
    double slight_injury_r;   //轻伤半径
    double property_damage_r;  //财产损失半径
};
typedef struct CSTD_safety_vcloud_explo CSTD_safety_vcloud_explo_t;
/*
财产损失等级的划分表
╔═════════════╦══════╦════════════════════════════════════════════════════════════════════╗
║damage level ║ 常数  ║  破坏状况                                                          ║
╠═════════════╬══════╬════════════════════════════════════════════════════════════════════╣
║1            ║  3.8 ║ 所有建筑物全部破坏                                                   ║
╠═════════════╬══════╬════════════════════════════════════════════════════════════════════╣
║2            ║  4.6 ║ 砖砌房外表50％～70％破损，墙壁下部危险                                ║
╠═════════════╬══════╬════════════════════════════════════════════════════════════════════╣
║3            ║  9.6 ║ 房屋不能再居住，屋基部分或全部破坏，外墙1～2个面部分破损，承重墙损失严重 ║
╠═════════════╬══════╬════════════════════════════════════════════════════════════════════╣
║4            ║  28  ║ 建筑物受到一定程度破坏，隔墙木机构要加固                               ║
╠═════════════╬══════╬════════════════════════════════════════════════════════════════════╣
║5            ║  56  ║ 房屋经修理可居住，天井瓷砖瓦管不同程度破坏，隔墙木结构要加固             ║  
╠═════════════╬══════╬════════════════════════════════════════════════════════════════════╣
║6            ║ 无穷大║ 房屋基本无破坏                                                      ║
╚═════════════╩══════╩════════════════════════════════════════════════════════════════════╝
*/
//蒸汽云（vapor cloud）爆炸模型 end

//凝聚相含能材料（爆炸物，如火药、炸药等）爆炸模型
struct CSTD_safety_explosive_explo
{
    double explosion_heat;   //爆炸热
    double mass;              //质量
    int property_damage_level; //要计算的财产损失等级，取值范围：{1,2,3,4,5},财产损失等级的划分表的第一列

    double explosion_energy;  //爆炸释放能量
    double TNT_equivalent;    //TNT当量
    double death_r;           //死亡半径
    double serious_injury_r;  //重伤半径
    double slight_injury_r;   //轻伤半径
    double property_damage_r;  //财产损失半径
};
typedef struct CSTD_safety_explosive_explo CSTD_safety_explosive_explo_t;

//盛装气体压力容器爆破模型
struct CSTD_safety_gpress_vessel_explo
{
    double abs_press;       //绝对压力Mpa
    double adiabatic_ind;   //气体绝热指数,双原子分子取1.4，三原子分子和四原子分子取1.2-1.3
    double volume;          //容器体积m^3
    int property_damage_level; //要计算的财产损失等级，取值范围：{1,2,3,4,5},财产损失等级的划分表的第一列

    double explosion_energy;  //爆炸释放能量
    double TNT_equivalent;    //TNT当量
    double death_r;           //死亡半径
    double serious_injury_r;  //重伤半径
    double slight_injury_r;   //轻伤半径
    double property_damage_r;  //财产损失半径
};
typedef struct CSTD_safety_gpress_vessel_explo CSTD_safety_gpress_vessel_explo_t;

//盛装液体压力容器爆破模型
struct CSTD_safety_lpress_vessel_explo
{
    double abs_press;       //绝对压力Mpa
    double compress_coeff;   //液体的压缩系数
    double volume;          //容器体积m^3
    int property_damage_level; //要计算的财产损失等级，取值范围：{1,2,3,4,5},财产损失等级的划分表的第一列

    double explosion_energy;  //爆炸释放能量
    double TNT_equivalent;    //TNT当量
    double death_r;           //死亡半径
    double serious_injury_r;  //重伤半径
    double slight_injury_r;   //轻伤半径
    double property_damage_r;  //财产损失半径
};
typedef struct CSTD_safety_lpress_vessel_explo CSTD_safety_lpress_vessel_explo_t;

//盛装液化气体容器爆破模型
struct CSTD_safety_lgpress_vessel_explo
{
    double mass;                //质量
    double boiling_point;       //介质在大气压力下的沸点，K
    double ethalpy;             //爆炸前液化气体的焓，kJ/kg
    double ehtropy;             //爆炸前饱和液体的熵，kJ/(kg.K)
    double normal_ethalpy;      //在大气压力下饱和液体的焓，kJ/kg
    double normal_ehtropy;      //在大气压力下饱和液体的熵，kJ/(kg.K)
    int property_damage_level; //要计算的财产损失等级，取值范围：{1,2,3,4,5},财产损失等级的划分表的第一列

    double explosion_energy;  //爆炸释放能量
    double TNT_equivalent;    //TNT当量
    double death_r;           //死亡半径
    double serious_injury_r;  //重伤半径
    double slight_injury_r;   //轻伤半径
    double property_damage_r;  //财产损失半径
};
typedef struct CSTD_safety_lgpress_vessel_explo CSTD_safety_lgpress_vessel_explo_t;

//盛装饱和水容器爆破模型
struct CSTD_safety_satwater_vessel_explo
{
    double volume;                //容器内饱和水所占容积m^3
    double coeff;                //饱和水爆破能量系数，kJ/m^3
    int property_damage_level;   //要计算的财产损失等级，取值范围：{1,2,3,4,5},财产损失等级的划分表的第一列

    double explosion_energy;  //爆炸释放能量
    double TNT_equivalent;    //TNT当量
    double death_r;           //死亡半径
    double serious_injury_r;  //重伤半径
    double slight_injury_r;   //轻伤半径
    double property_damage_r;  //财产损失半径
};
typedef struct CSTD_safety_satwater_vessel_explo CSTD_safety_satwater_vessel_explo_t;


//以下运算函数无错误返回0，出错返回<0
//气体泄露模型计算
int CSTD_safety_gas_leak_calc(CSTD_safety_gas_leak_t *gas_leak);
//液体泄露模型
int CSTD_safety_liquid_leak_calc(CSTD_safety_liquid_leak_t *liquid_leak);
//固体火灾模型计算
int CSTD_safety_solid_fire_calc(CSTD_safety_solid_fire_t *solid_fire);
//池火灾模型计算
int CSTD_safety_pool_fire_calc(CSTD_safety_pool_fire_t *pool_fire);
//喷射火模型计算
int CSTD_safety_jet_fire_calc( CSTD_safety_jet_fire_t *jet_fire );
//沸腾液体扩展蒸气爆炸模型计算
int CSTD_safety_BLEVE_fire_calc(CSTD_safety_BLEVE_fire_t *BLEVE_fire);
//蒸汽云爆炸模型计算
int CSTD_safety_vcloud_explo_calc( CSTD_safety_vcloud_explo_t *vcloud_explosion);
//凝聚相含能材料（爆炸物，如火药、炸药等）爆炸模型计算
int CSTD_safety_explosive_explo_calc(CSTD_safety_explosive_explo_t *explosive_explo);
//盛装气体压力容器爆破模型计算
int CSTD_safety_gpress_vessel_explo_calc(CSTD_safety_gpress_vessel_explo_t *press_explo);
//盛装液体压力容器爆破模型计算
int CSTD_safety_lpress_vessel_explo_calc(CSTD_safety_lpress_vessel_explo_t *press_explo);
//盛装液化气体容器爆破模型计算
int CSTD_safety_lgpress_vessel_explo_calc( CSTD_safety_lgpress_vessel_explo_t *press_explo );
//盛装饱和水容器爆破模型计算
int CSTD_safety_satwater_vessel_explo_calc( CSTD_safety_satwater_vessel_explo_t *press_explo );
#endif

