#include "fs_player.hpp"
#include "util/util.hpp"

#include "simulationcraft.hpp"

namespace fellowship
{
namespace gunde
{

// Forward Declarations
class gunde_t;

enum class secondary_trigger
{
  NONE = 0U
};

namespace actions
{
struct gunde_attack_t;
struct gunde_heal_t;
struct gunde_spell_t;

struct melee_t;
}  // namespace actions

class gunde_td_t : public fs_player_td_t
{
public:
  struct dots_t
  {
    dot_t* rend;
    dot_t* slaughter;
  } dots;

  struct
  {
    buff_t* open_wounds;
  } debuffs;

  gunde_td_t( player_t* target, gunde_t* source );
};

struct gunde_buff_t : public fs_player_buff_t
{
  gunde_buff_t( player_t* p, util::string_view name ) : fs_player_buff_t( p, name )
  {
  }

  gunde_t* p()
  {
    return debug_cast<gunde_t*>( player );
  }

  const gunde_t* p() const
  {
    return debug_cast<const gunde_t*>( player );
  }
};

class gunde_t : public fellowship::fs_player_t
{
public:
  struct actions_t
  {
    action_t* auto_attack;
    actions::melee_t* melee_hit;
    //action_t* double_strike;
    //action_t* reavers_edge;
    action_t* rend;
    action_t* slaughter;
    //action_t* heart_splitter;
    action_t* heart_splitter_exsanguinate;
    //action_t* rupture;
    //action_t* blood_arc;
    //action_t* grim_carve;
    //action_t* owed_in_blood;
    //action_t* reign_in_blood;
    //action_t* slaughter;
    //action_t* bloodbound_spirit;
    action_t* bloodbound_spirit_bird;
    action_t* ravens_precision;
    action_t* bloodcraze;
  } actions;

  struct buffs_t
  {
    buff_t* serrated_edge;
    buff_t* reign_in_blood;
    buff_t* owed_in_blood;
    buff_t* deaths_arc;
    buff_t* grim_harvest;
    buff_t* harvesters_toll;
    buff_t* crimson_strikes;
    buff_t* massacre;
    buff_t* bloodcraze;
    buff_t* ancestral_insight;
    buff_t* bloodbath;
  } buffs;

  struct cooldowns_t
  {
    cooldown_t* reavers_edge;
    cooldown_t* heart_splitter;
    cooldown_t* rupture;
    cooldown_t* slaughter;
    cooldown_t* grim_carve;
    cooldown_t* blood_arc;
    cooldown_t* reign_in_blood;
  } cooldowns;

  struct gains_t
  {
    gain_t* spirit_procs;
  } gains;

  struct rng_objects_t
  {
    accumulated_rng_t* grim_harvest;
    accumulated_rng_t* deaths_arc;
    accumulated_rng_t* ancestral_insight;
  } rng_objects;

#define GUNDE_TALENT_LIST( X )                                         \
  X( DEATHS_ARC, "deaths_arc", "Deaths Arc" )                          \
  X( RAVENS_PRECISION, "ravens_precision", "Ravens Precision" )        \
  X( GRIM_HARVEST, "grim_harvest", "Grim Harvest" )                    \
  X( HARVESTERS_TOLL, "harvesters_toll", "Harvesters Toll" )           \
  X( CRIMSON_STRIKES, "crimson_strikes", "Crimson Strikes" )           \
  X( DARKENING_HEARTS, "darkening_hearts", "Darkening Hearts" )        \
  X( SUPERIOR_SERRATION, "superior_serration", "Superior Serration" )  \
  X( MURDER_OF_CROWS, "murder_of_crows", "Murder of Crows" )           \
  X( MASSACRE, "massacre", "Massacre" )                                \
  X( SLAYERS_GRIN, "slayers_grin", "Slayers Grin" )                    \
  X( FRENZIED_REGEN, "frenzied_regen", "Frenzied Regen" )              \
  X( DEEP_REND, "deep_rend", "Deep Rend" )                             \
  X( BLOODCRAZE, "bloodcraze", "Bloodcraze" )                          \
  X( OATHSHATTER, "oathshatter", "Oathshatter" )                       \
  X( CARNAGE, "carnage", "Carnage" )                                   \
  X( SUNDERED_FLESH, "sundered_flesh", "Sundered Flesh" )              \
  X( ANCESTRAL_INSTINCT, "ancestral_instinct", "Ancestral Instinct" )  \
  X( BLOODBATH, "bloodbath", "Bloodbath" )

  enum gunde_talent_index_t
  {
#define X( name, id, pretty ) name##_INDEX,
    GUNDE_TALENT_LIST( X )
#undef X
        GUNDE_TALENT_MAX
  };

  enum gunde_talents_t : unsigned long long
  {
    NONE = 0,
#define X( name, id, pretty ) name = 1ULL << name##_INDEX,
    GUNDE_TALENT_LIST( X )
#undef X
        MAX = 1ULL << GUNDE_TALENT_MAX
  };

  static constexpr talent_info GUNDE_TALENTS[] = {
#define X( name, id, pretty ) { gunde_talents_t::name, id, pretty },
      GUNDE_TALENT_LIST( X )
#undef X
  };

  constexpr std::string_view talent_name( long long t ) override
  {
    for ( const auto& talent : GUNDE_TALENTS )
      if ( talent.flag == t )
        return talent.id;

    return "unknown_talent";
  }

  constexpr std::string_view talent_name_formatted( long long t ) override
  {
    for ( const auto& talent : GUNDE_TALENTS )
      if ( talent.flag == t )
        return talent.pretty;

    return "Unknown Talent";
  }

  struct spell_const_t
  {
    // Gunde.AutoAttack.SwingTimer, 1.5
    timespan_t auto_attack_time = 1.5_s;
    // Gunde.MeleeAutoAttack.StrengthCoefficient, 0.30
    double auto_attack_coeff = 0.3;
    // Gunde.MeleeAutoAttack.HitVisualDelay, 0.51
    timespan_t melee_hit_visual_delay = 0.51_s;

    // Gunde.SpiritProc.SpiritPointsGain
    double spirit_refund_mul = 1.0; 
    
    // Gunde.SpiritProc.AmountOfOrbs
    int spirit_proc_orbs = 5;

    // Feathers Gunde.SpiritProc.SpawnMinRadius, 150.0 Gunde.SpiritProc.SpawnMaxRadius, 300.0; 

    // Gunde.PerTargetAccumulatedBleed.DamageToDotScaler
    double rend_accumulation = 0.5;
    // Gunde.PerTargetAccumulatedBleed.Dot.Duration
    timespan_t rend_duration = 30_s;
    // Gunde.PerTargetAccumulatedBleed.Dot.Period
    timespan_t rend_period   = 3_s;
    
    // Gunde.PerTargetAccumulatedBleed.TotalDamageToStackCountScaler, 0.0025 ; So 1/100 damage
        
    // Gunde.BoostedDotDamage.Debuff.BleedDamageScaler, 1.20
    double open_wounds_modifier = 1.2;
    // Gunde.BoostedDotDamage.Debuff.Duration, 18.0
    timespan_t open_wounds_duration = 18_s;
    
    // Gunde.ExtraDotDamageGain.BonusAmount, 0.2		;Blood Arc buff
    double blood_arc_buff_additional_rend = 0.2;
    // Gunde.ExtraDotDamageGain.MaxStacks, 1.0
    int blood_arc_buff_max_stacks      = 1;
    // Gunde.ExtraDotDamageGain.Duration, 8.0
    timespan_t blood_arc_buff_duration = 8_s;;

    //Gunde.BasicSingleTargetAttack.DamageStrengthCoefficientPerHit, 0.55         ; DOUBLE STRIKE WAS 1.0
    double double_strike_coeff = 0.55;
    // Gunde.BasicSingleTargetAttack.FirstVisualHitDelay, 0.1
    timespan_t double_strike_first_hit_delay = 0.1_s;
    // Gunde.BasicSingleTargetAttack.SecondVisualHitDelay, 0.45
    timespan_t double_trike_second_hit_delay = 0.45_s;   
    //Gunde.BasicSingleTargetAttack.MaxRange, 3000.0

    // Gunde.InstantWhirlwindAoe.VisualHitDelay, 0.45                               ; REAVER'S EDGE 
    timespan_t reavers_edge_delay = 0.45_s;
    //Gunde.InstantWhirlwindAoe.AoeRadius, 700.0
    //Gunde.InstantWhirlwindAoe.DamageStrengthCoefficient, 2.20                   ; WAS 1.616
    double reavers_edge_coeff = 2.20;
    //Gunde.InstantWhirlwindAoe.DamageScalingTargetCountThreshold, 8.0			 ; WAS 3.0
    double reavers_edge_target_threshold = 8.0;
    //Gunde.InstantWhirlwindAoe.Cooldown, 5.0
    timespan_t reavers_edge_cooldown = 5_s;
    //Gunde.InstantWhirlwindAoe.Talent.GatherOrbs.MaxRange, 700.0


    //Gunde.HeavyMeleeDotBoost.VisualHitDelay, 0.3                                 ; RUPTURE
    timespan_t rupture_visual_delay = 0.3_s;
    //Gunde.HeavyMeleeDotBoost.DamageStrengthCoefficient, 11.16                      ; WAS 8.4
    double rupture_coeff = 11.16;
    //Gunde.HeavyMeleeDotBoost.Cooldown, 60.0
    timespan_t rupture_cooldown = 60_s;
    //Gunde.HeavyMeleeDotBoost.Talent.IncreasedDotDamage.Duration, 10.0
    //Gunde.HeavyMeleeDotBoost.Talent.IncreasedDotDamage.DamageMultiplier, 1.12
    //Gunde.HeavyMeleeDotBoost.Talent.IncreasedCritAndNoneCritDamage.CritDamageMultiplier, 1.5
    //Gunde.HeavyMeleeDotBoost.Talent.IncreasedCritAndNoneCritDamage.NoneCritDamageMultiplier, 1.1
    //Gunde.HeavyMeleeDotBoost.Talent.ReducedCooldown.ReductionInSeconds, 1.0
    //Gunde.HeavyMeleeDotBoost.Talent.OrbDropper.NumOfOrbs, 8.0
    //Gunde.HeavyMeleeDotBoost.Talent.OrbDropper.MinRadius, 150.0
    //Gunde.HeavyMeleeDotBoost.Talent.OrbDropper.MaxRadius, 300.0
    //Gunde.HeavyMeleeDotBoost.Talent.ReducedCooldownPerOrb.ReductionInSeconds, 0.2

    //Gunde.TargetedAoeProjectile.ProjectileSpawnDelay, 0.14                        ; GRIM CARVE
    //Gunde.TargetedAoeProjectile.DamageStrengthCoefficientPerHit, 1.635            ; WAS 2.0
    //Gunde.TargetedAoeProjectile.DamageScalingTargetCountThreshold, 12.0
    //Gunde.TargetedAoeProjectile.Speed, 5000.0
    //Gunde.TargetedAoeProjectile.Cooldown, 15.0
    //Gunde.TargetedAoeProjectile.MaxRange, 3000.0
    //Gunde.TargetedAoeProjectile.Aoe.Radius, 700.0
    //Gunde.TargetedAoeProjectile.Aoe.Duration, 1.15
    //Gunde.TargetedAoeProjectile.Aoe.Period, 0.5
    //Gunde.TargetedAoeProjectile.Talent.Empowered.ProcChance, 0.16
    //Gunde.TargetedAoeProjectile.Talent.Empowered.DamageMultiplier, 1.40                                          ;WAS 2.0
    //Gunde.TargetedAoeProjectile.Talent.Empowered.Duration, 8.0  
    //Gunde.TargetedAoeProjectile.Talent.DamageCooldownReduction.DamageMultiplier, 1.25
    //Gunde.TargetedAoeProjectile.Talent.DamageCooldownReduction.CooldownReductionInSecondsPerHit, 1.0
    //Gunde.TargetedAoeProjectile.Talent.AdditionalTicks.Amount, 2.0

    //Gunde.DamageReductionSelfBuff.IncomingDamageMultiplier, 0.6                 ; RECKLESS ABANDON
    //Gunde.DamageReductionSelfBuff.Duration, 4.0
    //Gunde.DamageReductionSelfBuff.Cooldown, 30.0
    //Gunde.DamageReductionSelfBuff.VisualDelay, 0.24

    //Gunde.SingleTargetInterrupt.MaxRange, 500.0                                 ; HEADSPLITTER
    //Gunde.SingleTargetInterrupt.Duration, 4.0
    //Gunde.SingleTargetInterrupt.Cooldown, 16.0
    //Gunde.SingleTargetInterrupt.VisualDelay, 0.2

    //Gunde.ExtraDotDamageBuff.BonusDotDamage, 0.5                                ; REIGN IN BLOOD
    double reign_in_blood_additional_rend = 0.5;
    //Gunde.ExtraDotDamageBuff.Duration, 12.0
    timespan_t reign_in_blood_duration = 12.0_s;
    //Gunde.ExtraDotDamageBuff.Cooldown, 90.0
    timespan_t reign_in_blood_cooldown = 90.0_s;
    //Gunde.ExtraDotDamageBuff.VisualDelay, 0.2
    //Gunde.ExtraDotDamageBuff.Talent.Haste.AdditionalHaste, 0.25

    //Gunde.HeavyMeleeDotBased.Cooldown, 12.0                                     ; RECKONING
    timespan_t reckoning_cooldown = 12.0_s;
    //Gunde.HeavyMeleeDotBased.Damage.StrengthCoefficient, 3.70                   ; WAS 3.4
    double reckoning_coeff = 3.70;
    //Gunde.HeavyMeleeDotBased.Damage.DotCoefficient, 0.50						; WAS 0.10
    double reckoning_exsanguinate_coeff = 0.5;
    //Gunde.HeavyMeleeDotBased.VisualDelay, 0.35
    timespan_t reckoning_visual_delay = 0.35_s;
    //Gunde.HeavyMeleeDotBased.Talent.IncreasedCritChance.AdditionalCritChance, 0.3         ;WAS 0.4
    //Gunde.HeavyMeleeDotBased.Talent.CritToAoe.Radius, 500.0
    //Gunde.HeavyMeleeDotBased.Talent.CritToAoe.AdditionalTargets, 4.0	; OLD
    //Gunde.HeavyMeleeDotBased.Talent.CritToAoe.TargetThresholdScaling, 3.0
    //Gunde.HeavyMeleeDotBased.Talent.LowHealthTarget.AddedCriticalStrikeChance, 1.00
    //Gunde.HeavyMeleeDotBased.Talent.CooldownAcceleration.AdditionalCooldownRecovery, 0.5
    //Gunde.HeavyMeleeDotBased.Talent.CooldownAcceleration.Duration, 3.0
    //Gunde.HeavyMeleeDotBased.Talent.Charges.NumOfCharges, 2.0
    //Gunde.HeavyMeleeDotBased.Talent.Charges.DoubleStrike.Chance, 0.2
    //Gunde.HeavyMeleeDotBased.Talent.Charges.DoubleStrike.DelayBetweenStrikes, 0.4

    //Gunde.InstantAoeDot.Aoe.Radius, 1000.0                                      ; SLAUGHTER
    //Gunde.InstantAoeDot.Aoe.Height, 500.0
    //Gunde.InstantAoeDot.Dot.AccDotToNewDotScaler, 1.60                           ;WAS 1.30
    double slaughter_rend_multiplier = 1.60;
    //Gunde.InstantAoeDot.Dot.Duration, 3.0
    timespan_t slaughter_duration = 3.0_s;
    //Gunde.InstantAoeDot.Dot.Period, 0.3
    timespan_t slaughter_period = 0.3_s;
    //Gunde.InstantAoeDot.VisualDelay, 0.5
    //Gunde.InstantAoeDot.Cooldown, 30.0											;WAS 18.0	
    timespan_t slaughter_cooldown = 30.0_s;
    //Gunde.InstantAoeDot.Talent.IncreasedCritChance.AdditionalCritChance, 0.1
    //Gunde.InstantAoeDot.Talent.StacksToHaste.Duration, 8.0
    //Gunde.InstantAoeDot.Talent.StacksToHaste.AdditionalHastePerStack, 0.0025
    //Gunde.InstantAoeDot.Talent.StacksToHaste.MaxStacks, 100.0

    //Gunde.DotTransfer.MaxRange, 2000.0                                          ; OWED IN BLOOD
    //Gunde.DotTransfer.SelfBuff.Duration, 30.0
    //Gunde.DotTransfer.VisualDelay, 0.25
    //Gunde.DotTransfer.Target.Cooldown, 3.0
    //Gunde.DotTransfer.Talent.IncreasedDamageOnSelf.DamageScalePerStack, 0.0025
    //Gunde.DotTransfer.Talent.IncreasedDamageOnSelf.MaxStacks, 30.0
    //Gunde.DotTransfer.Talent.IncreasedDamageOnSelf.Duration, 10.0
    //Gunde.DotTransfer.Talent.Aoe.Cooldown, 20.0
    //Gunde.DotTransfer.Talent.Aoe.Radius, 1000.0
    //Gunde.DotTransfer.Talent.Aoe.MaxStacks, 80.0
    //Gunde.DotTransfer.Talent.Aoe.StackDuration, 10.0
    //Gunde.DotTransfer.Talent.ExplosionOnPickup.StrengthCoefficient, 0.34
    //Gunde.DotTransfer.Talent.ExplosionOnPickup.Radius, 700.0
    //Gunde.DotTransfer.Talent.ExplosionOnPickup.TargetThresholdScaling, 8.0
    //Gunde.DotTransfer.Talent.AoeBasedOnStacks.Radius, 700.0
    //Gunde.DotTransfer.Talent.AoeBasedOnStacks.BaseStrengthCoefficient, 0.20
    //Gunde.DotTransfer.Talent.AoeBasedOnStacks.DamageIncreasePerStack, 1.20
    //Gunde.DotTransfer.Talent.AoeBasedOnStacks.TargetThresholdScaling, 5.0
    //Gunde.DotTransfer.Talent.AoeBasedOnStacks.Period, 3.0
    //Gunde.DotTransfer.Talent.AoeBasedOnStacks.Duration, 6.0

    //Gunde.DotTransfer.Orb.Lifespan, 30.0                                        ;OWED IN BLOOD NEW FUNCTION
    //Gunde.DotTransfer.Orb.MaxActiveOrbPickups, 20.0
    //Gunde.DotTransfer.Orb.ActivationDelay, 0.5  								; Mainly to allow the pickups to spawn and linger for a while when the player is already in the trigger
    //Gunde.DotTransfer.Orb.Scale, 1.5                                            ;WAS 1.0
    //Gunde.DotTransfer.Orb.PickUpRadius, 360.0                                   ;WAS 150.0
    //Gunde.DotTransfer.Orb.NumOfStacksPerOrb, 1.0                                ;WAS 2.0
    //Gunde.DotTransfer.Orb.RelevantDropRadius, 5000.0	                        ; How far away an enemy can die and still drop an orb
    //Gunde.DotTransfer.Orb.SpawnChanceAtDeath, 0.25
    //Gunde.DotTransfer.Orb.SpawnChanceAtTick.A, 1.0
    //Gunde.DotTransfer.Orb.SpawnChanceAtTick.B, 0.50
    //Gunde.DotTransfer.Orb.SpawnChanceAtTick.C, 0.33
    //Gunde.DotTransfer.Orb.SpawnChanceAtTick.D, 0.25
    //Gunde.DotTransfer.Orb.SpawnChanceAtTick.E, 0.20
    //Gunde.DotTransfer.Orb.SpawnChanceAtTick.F, 0.1667
    //Gunde.DotTransfer.Orb.SpawnChanceAtTick.G, 0.15
    //Gunde.DotTransfer.Orb.SpawnInnerRadius, 100.0
    //Gunde.DotTransfer.Orb.SpawnOuterRadius, 300.0
    //Gunde.DotTransfer.MaxStacks, 150.0

    //Gunde.SingleTargetPull.Cooldown, 30.0                                       ; BUTCHER'S HOOK
    //Gunde.SingleTargetPull.MaxRange, 3000.0
    //Gunde.SingleTargetPull.MaxPullHeight, 1000.0                                ; How high in the air gunde can be while pulling an enemy towards a spot on the ground below him. (enemies never get pulled up into the air.)
    //Gunde.SingleTargetPull.PullDelay, 0.25
    //Gunde.SingleTargetPull.PullDuration, 0.2
    //Gunde.SingleTargetPull.StopDistance, 150.0                                  ; How far from gunde the enemy will end up at.
    //Gunde.SingleTargetPull.Damage.StrengthCoefficient, 0.37                     ;WAS 0.69
    //Gunde.SingleTargetPull.Talent.MultiTargets.Radius, 800.0
    //Gunde.SingleTargetPull.Talent.MultiTargets.AdditionalTargets, 3.0
    //Gunde.SingleTargetPull.Talent.MultiTargets.AdditionalCooldown, 15.0

    //Gunde.JumpToLocation.Cooldown, 20.0                                         ; WARBOUND
    //Gunde.JumpToLocation.NumCharges, 2.0
    //Gunde.JumpToLocation.MaxRangeHorizontal, 2000.0
    //Gunde.JumpToLocation.MaxRangeUp, 1000.0
    //Gunde.JumpToLocation.MaxRangeDown, 5000.0
    //Gunde.JumpToLocation.TargetingRadius, 50.0 ; purly visual right now
    //Gunde.JumpToLocation.JumpDuration, 0.3

    //Gunde.InstantAoeWithBuff.VisualHitDelay, 0.2                               ; BLOOD ARC
    //Gunde.InstantAoeWithBuff.AoeRadius, 700.0
    //Gunde.InstantAoeWithBuff.ConePieHeight, 500.0                               ;WAS 200
    //Gunde.InstantAoeWithBuff.ConePieMinRadius, 0.0
    //Gunde.InstantAoeWithBuff.ConePieAngleWidth, 120.0

    //Gunde.InstantAoeWithBuff.DamageStrengthCoefficient, 1.212                    ; WAS 2.0
    //Gunde.InstantAoeWithBuff.DamageScalingTargetCountThreshold, 5.0
    //Gunde.InstantAoeWithBuff.Cooldown, 9.0
    //Gunde.InstantAoeWithBuff.Talent.AdditionalStack.MaxStacks, 2.0
    //Gunde.InstantAoeWithBuff.Talent.ChanceCooldownReset.Chance, 0.25            ; WAS 0.15
    //Gunde.InstantAoeWithBuff.Talent.Empowered.AddedCriticalStrikeChance, 1.00
    //Gunde.InstantAoeWithBuff.Talent.Empowered.Duration, 12.0

    //Gunde.DurationalAoeDotAndBuff.Radius, 1000.0								; BLOODBOUND SPIRIT
    //Gunde.DurationalAoeDotAndBuff.Duration, 6.15
    //Gunde.DurationalAoeDotAndBuff.Period, 1.5
    //Gunde.DurationalAoeDotAndBuff.StrengthCoefficient, 4.95                      ; WAS 3.5
    //Gunde.DurationalAoeDotAndBuff.DamageScalingTargetCountThreshold, 8.0
    //Gunde.DurationalAoeDotAndBuff.CastTime, 1.0
    //Gunde.DurationalAoeDotAndBuff.SelfBuff.Duration, 20.0
    //Gunde.DurationalAoeDotAndBuff.SelfBuff.DamageMultiplier, 1.10               ; WAS 1.25

    //Gunde.DashAttack.StrengthCoefficient, 0.2									; WARBOUND NEW WAS 0.5
    //Gunde.DashAttack.DamageRadius, 250.0
    //Gunde.DashAttack.MaxCharges, 2.0
    //Gunde.DashAttack.MaxDistance, 1000.0
    //Gunde.DashAttack.InAirExitSpeed, 1000.0
    //Gunde.DashAttack.Speed, 2500
    //Gunde.DashAttack.CooldownDuration, 8.0
    //Gunde.DashAttack.Talent.DamageMultiplier, 1.15

    //Gunde.Talent.AbilityToBuffChance.ProcChance, 0.4
    //Gunde.Talent.AbilityToBuffChance.StrengthMultiplier, 1.06
    //Gunde.Talent.AbilityToBuffChance.Duration, 6.0

  } spell_const;

  struct talents_t
  {
    // Talent Deep Rend
    // Gunde.PerTargetAccumulatedBleed.Talent.ReducedTickSpeed.PeriodFrequencyIncrease
    double deep_rend_tick_speed_increase = 1.2;
    // Gunde.PerTargetAccumulatedBleed.Talent.IncreasedDropAmount.DropChance
    double deep_rend_proc_chance_st = 0.1;
    // Gunde.PerTargetAccumulatedBleed.Talent.IncreasedDropAmount.NewAmount
    int deep_rend_proc_feathers = 3;
  } talents;

  struct legendary_t
  {
    bool lego_1 = false;

    bool lego_2 = false;

    bool lego_3 = false;
  } legendary;

  struct options_t
  {
  } options;

  target_specific_t<gunde_td_t> target_data;

  const gunde_td_t* find_target_data( const player_t* target ) const override
  {
    return target_data[ target ];
  }

  gunde_td_t* get_target_data( player_t* target ) const override
  {
    gunde_td_t*& td = target_data[ target ];
    if ( !td )
    {
      td = new gunde_td_t( target, const_cast<gunde_t*>( this ) );
    }
    return td;
  }

  // Character Definition
  void init_spells() override;
  void init_base_stats() override;
  void init_talents() override;
  void init_gains() override;
  void init_procs() override;
  void init_scaling() override;
  void init_resources( bool force ) override;
  void init_items() override;
  void init_special_effects() override;
  void init_finished() override;
  void init_background_actions() override;
  void init_rng() override;

  void create_cooldowns();
  void create_buffs() override;
  void create_options() override;
  void parry_effects( action_state_t* s );

  void copy_from( player_t* source ) override;
  std::string create_profile( save_e stype ) override;
  void init_action_list() override;

  void reset() override;
  void activate() override;
  void arise() override;
  void combat_begin() override;
  action_t* create_action( util::string_view name, util::string_view options ) override;

  std::unique_ptr<expr_t> create_action_expression( action_t& action, std::string_view name_str ) override;
  std::unique_ptr<expr_t> create_expression( util::string_view name_str ) override;
  std::unique_ptr<expr_t> create_resource_expression( util::string_view name ) override;

  void regen( timespan_t periodicity ) override;
  resource_e primary_resource() const override
  {
    return RESOURCE_NONE;
  }
  role_e primary_role() const override
  {
    return ROLE_ATTACK;
  }
  stat_e convert_hybrid_stat( stat_e s ) const override;

  double composite_attribute_multiplier( attribute_e attr ) const override;
  double composite_melee_auto_attack_speed() const override;
  double composite_melee_haste() const override;
  double composite_melee_crit_chance() const override;
  double composite_spell_crit_chance() const override;
  double composite_spell_haste() const override;
  double composite_damage_versatility() const override;
  double composite_heal_versatility() const override;
  double composite_leech() const override;
  double matching_gear_multiplier( attribute_e attr ) const override;
  double composite_player_multiplier( school_e school ) const override;
  double composite_player_pet_damage_multiplier( const action_state_t*, bool ) const override;
  double composite_player_target_multiplier( player_t* target, school_e school ) const override;
  double composite_player_target_crit_chance( player_t* target ) const override;
  double composite_player_target_armor( player_t* target ) const override;
  double resource_regen_per_second( resource_e ) const override;
  double non_stacking_movement_modifier() const override;
  double stacking_movement_modifier() const override;
  void invalidate_cache( cache_e ) override;

  double resource_gain( resource_e r, double amount, gain_t* source = nullptr, action_t* a = nullptr ) override;

  void cancel_auto_attacks() override;

  std::string default_flask() const override
  {
    return "disabled";
  }

  // rogue_t::default_potion ==================================================

  std::string default_potion() const override
  {
    return "disabled";
  }

  // rogue_t::default_food ====================================================

  std::string default_food() const override
  {
    return "disabled";
  }

  // rogue_t::default_rune ====================================================

  std::string default_rune() const override
  {
    return "disabled";
  }

  // rogue_t::default_temporary_enchant =======================================

  std::string default_temporary_enchant() const override
  {
    return "disabled";
  }

  gunde_t( sim_t* sim, util::string_view name, race_e r = RACE_NONE )
    : fs_player_t( sim, name, r, GUNDE ), target_data()
  {
    resource_regeneration = regen_type::DYNAMIC;

    create_cooldowns();
    spirit_refund_mul = spell_const.spirit_refund_mul;
  }
};

namespace actions
{  // namespace actions

template <typename Base>
class gunde_action_t : public fellowship::actions::fs_player_action_t<Base>
{
protected:
  /// typedef for gunde_action_t<action_base_t>
  using base_t = gunde_action_t<fellowship::actions::fs_player_action_t<Base>>;

private:
  /// typedef for the templated action type, eg. spell_t, attack_t, heal_t
  using ab = fellowship::actions::fs_player_action_t<Base>;

public:
  bool _applies_rend;

  // Init =====================================================================

  gunde_action_t( util::string_view n, gunde_t* p, util::string_view options = {} )
    : ab( n, p, options ),
      _applies_rend( false )
  {
    ab::parse_options( options );
    ab::may_crit = ab::tick_may_crit = true;
    ab::school                       = SCHOOL_PHYSICAL;

    ab::gcd_type = gcd_haste_type::ATTACK_HASTE;
  }

  void init() override
  {
    ab::init();
  }

  gunde_t* p()
  {
    return debug_cast<gunde_t*>( ab::player );
  }

  const gunde_t* p() const
  {
    return debug_cast<const gunde_t*>( ab::player );
  }

  gunde_td_t* td( player_t* t ) const
  {
    return p()->get_target_data( t );
  }

public:
  // Ability triggers
  void trigger_auto_attack( const action_state_t* );
  void trigger_spirit_refund( const action_state_t* );
  void apply_rend( const action_state_t* );

  void execute() override
  {
    ab::execute();

    if ( ab::hit_any_target && !ab::background )
    {
      trigger_auto_attack( ab::execute_state );
    }
  }
};

// ==========================================================================
// Rogue Attack Classes
// ==========================================================================

struct gunde_heal_t : public gunde_action_t<heal_t>
{
protected:
  using base_t = gunde_action_t<heal_t>;

public:
  gunde_heal_t( util::string_view n, gunde_t* p, util::string_view o = {} ) : base_t( n, p, o )
  {
    harmful = false;
    set_target( p );
  }

  size_t available_targets( std::vector<player_t*>& target_list ) const override
  {
    target_list.clear();
    target_list.push_back( target );

    for ( const auto& t : sim->healing_no_pet_list )
    {
      if ( t != target && ( t->is_active() || ( t->type == HEALING_ENEMY && !t->is_sleeping() ) ) )
        target_list.push_back( t );
    }

    // Remove non Healing Enemy pets from valid target list
    for ( const auto& t : sim->healing_pet_list )
    {
      if ( t != target && ( ( t->type == HEALING_ENEMY && !t->is_sleeping() ) ) )
        target_list.push_back( t );
    }

    return target_list.size();
  }
};

struct gunde_spell_t : public gunde_action_t<spell_t>
{
protected:
  using base_t = gunde_action_t<spell_t>;

public:
  gunde_spell_t( util::string_view n, gunde_t* p, util::string_view o = {} ) : base_t( n, p, o )
  {
    school = SCHOOL_MAGIC;
  }
};

struct gunde_attack_t : public gunde_action_t<melee_attack_t>
{
protected:
  using base_t = gunde_action_t<melee_attack_t>;

public:
  gunde_attack_t( util::string_view n, gunde_t* p, util::string_view o = {} ) : base_t( n, p, o )
  {
    special = true;
    school  = SCHOOL_PHYSICAL;
  }

  void impact( action_state_t* s ) override
  {
    base_t::impact( s );

    if ( result_is_hit( s->result ) )
    {
      apply_rend( s );
    }
  }
};

struct gunde_absorb_t : public gunde_action_t<absorb_t>
{
protected:
  using base_t = gunde_action_t<absorb_t>;

public:
  gunde_absorb_t( util::string_view n, gunde_t* p, util::string_view o = {} ) : base_t( n, p, o )
  {
    may_crit      = false;
    tick_may_crit = false;
    may_miss      = false;
    target        = p;
  }

  size_t available_targets( std::vector<player_t*>& target_list ) const override
  {
    target_list.clear();
    target_list.push_back( target );

    for ( const auto& t : sim->healing_no_pet_list )
    {
      if ( t != target && ( t->is_active() || ( t->type == HEALING_ENEMY && !t->is_sleeping() ) ) )
        target_list.push_back( t );
    }

    // Remove non Healing Enemy pets from valid target list
    for ( const auto& t : sim->healing_pet_list )
    {
      if ( t != target && ( ( t->type == HEALING_ENEMY && !t->is_sleeping() ) ) )
        target_list.push_back( t );
    }

    return target_list.size();
  }

  /* absorb_buff_t* create_buff( const action_state_t* s ) override
 {
   if ( s->target == player )
   {
     if ( priest().buffs.power_word_shield->absorb_source != stats )
       priest().buffs.power_word_shield->set_absorb_source( stats );
     return priest().buffs.power_word_shield;
   }

   buff_t* b = buff_t::find( s->target, name_str, player );
   if ( b )
     return debug_cast<absorb_buff_t*>( b );

   auto buff = make_buff<buffs::power_word_shield_buff_t>( &priest(), s->target );
   buff->set_absorb_source( stats );

   return buff;
 }*/
};

struct melee_t : public gunde_attack_t
{
  bool first;
  bool canceled;
  timespan_t prev_scheduled_time;

  melee_t( const char* name, const char* reporting_name, gunde_t* p )
    : gunde_attack_t( name, p ), first( true ), canceled( false ), prev_scheduled_time( timespan_t::zero() )
  {
    background = repeating = may_glance = may_crit = true;
    may_miss                                       = true;
    allow_class_ability_procs = not_a_proc = true;
    special                                = false;

    school             = SCHOOL_PHYSICAL;
    trigger_gcd        = timespan_t::zero();
    name_str_reporting = reporting_name;

    attack_power_mod.direct = p->spell_const.auto_attack_coeff;
  }

  double miss_chance( double /* hit */, player_t* /* target */ ) const
  {
    return 1 - 0.95 * 0.95;
  }

  void reset() override
  {
    gunde_attack_t::reset();
    first               = true;
    canceled            = false;
    prev_scheduled_time = timespan_t::zero();
  }

  timespan_t execute_time() const override
  {
    timespan_t t = gunde_attack_t::execute_time();

    if ( first )
    {
      return timespan_t::zero();
    }

    // If we cancel the swing timer mid-fight, use the previous swing timer
    if ( canceled )
    {
      return std::min( t, std::max( prev_scheduled_time - p()->sim->current_time(), timespan_t::zero() ) );
    }

    return t;
  }

  void schedule_execute( action_state_t* state ) override
  {
    gunde_attack_t::schedule_execute( state );

    if ( first )
    {
      first = false;
      p()->sim->print_log( "{} schedules AA start {} with {} swing timer", *p(), *this, time_to_execute );
    }

    if ( canceled )
    {
      canceled            = false;
      prev_scheduled_time = timespan_t::zero();
      p()->sim->print_log( "{} schedules AA restart {} with {} swing timer remaining", *p(), *this, time_to_execute );
    }
  }
};

struct auto_melee_attack_t : public action_t
{
  auto_melee_attack_t( gunde_t* p, util::string_view options_str ) : action_t( ACTION_OTHER, "auto_attack", p )
  {
    trigger_gcd        = timespan_t::zero();
    name_str_reporting = "Auto Attack";

    background = true;

    p->actions.melee_hit = debug_cast<melee_t*>( p->find_action( "auto_attack_damage" ) );
    if ( !p->actions.melee_hit )
      p->actions.melee_hit = new melee_t( "auto_attack_damage", "Auto Attack", p );

    p->main_hand_attack                    = p->actions.melee_hit;
    p->main_hand_attack->base_execute_time = p->spell_const.auto_attack_time;
    p->main_hand_attack->id                = 1;

    id = 1;


    add_child( p->actions.melee_hit );
  }

  void execute() override
  {
    stats->add_execute( 0_ms, target );  // log AA timer resets

    player->main_hand_attack->schedule_execute();
  }

  bool ready() override
  {
    if ( player->is_moving() )
      return false;

    return ( player->main_hand_attack->execute_event == nullptr );  // not swinging
  }
};

struct double_strike_t : public gunde_attack_t
{
  struct double_strike_hit_t : public gunde_attack_t
  {
    double_strike_hit_t( util::string_view name, gunde_t* p )
      : gunde_attack_t( name, p )
    {
      background = dual       = true;
      id                      = 2;
      _applies_rend           = true;
      school                  = SCHOOL_PHYSICAL;
      attack_power_mod.direct = p->spell_const.double_strike_coeff;
      name_str_reporting      = "Double Strike Hit";

      ability_flags |= ability_type_e::ABILITY_BASIC;
    }
  };

  std::array<double_strike_hit_t*, 2> hits;

  double_strike_t( util::string_view name, gunde_t* p, util::string_view options_str = {} )
    : gunde_attack_t( name, p, options_str ), hits()
  {
    id = 2;
    _applies_rend = true;

    school                  = SCHOOL_PHYSICAL;

    name_str_reporting     = "Double Strike";

    ability_flags |= ability_type_e::ABILITY_BASIC;

    hits[ 0 ]               = new double_strike_hit_t( "double_strike_hit_1", p );
    hits[ 0 ]->travel_delay = p->spell_const.double_strike_first_hit_delay.total_seconds();
    hits[ 1 ]               = new double_strike_hit_t( "double_strike_hit_2", p );
    hits[ 1 ]->travel_delay = p->spell_const.double_trike_second_hit_delay.total_seconds();

    for ( auto& hit : hits )
    {
      add_child( hit );
    }
  }

  void impact( action_state_t* s ) override
  {
    gunde_attack_t::impact( s );

    for ( auto& hit : hits )
    {
      hit->set_target( s->target );
      action_state_t* damage_state = hit->get_state( s );

      // dmg_action->snapshot_state( damage_state, result_amount_type::DMG_DIRECT );
      // damage_state->persistent_multiplier = execute_state->persistent_multiplier;

      hit->finesse_n_active = finesse_n_active;

      if ( finesse_n_active )
      {
        fs_p()->fs_buffs.finesse_n->expire();
        finesse_n_active = false;
      }

      hit->schedule_execute( damage_state );
    }
  }

  //void execute() override
  //{
  //  gunde_attack_t::execute();
  //}
};
//
//struct blinding_slash_t : public gunde_attack_t
//{
//  blinding_slash_t( util::string_view name, gunde_t* p, util::string_view options_str = {} )
//    : gunde_attack_t( name, p, options_str )
//  {
//    id = 3;
//    _procs_golden_hour      = true;
//
//    school                  = SCHOOL_PHYSICAL;
//    attack_power_mod.direct = p->spell_const.blinding_slash_coeff;
//
//    name_str_reporting     = "Blinding Slash";
//    swift_reprieval_chance = p->spell_const.blinding_slash_resource_proc_chance;
//
//    aoe                 = -1;
//    reduced_aoe_targets = p->spell_const.blinding_slash_scaling_threshold;
//
//    cooldown->hasted   = true;
//    cooldown->duration = p->spell_const.blinding_slash_cd;
//    cooldown->charges  = 1;
//
//    ability_flags |= ability_type_e::ABILITY_BASIC;
//  }
//
//  void impact( action_state_t* s ) override
//  {
//    gunde_attack_t::impact( s );
//
//    p()->get_target_data( s->target )->debuffs.blind->trigger();
//  }
//};
//
//struct omnistrike_t : public gunde_attack_t
//{
//  struct omnistrike_action_state_t : public action_state_t
//  {
//  public:
//    int sunstruck_stacks;
//    omnistrike_action_state_t( action_t* action, player_t* target )
//      : action_state_t( action, target ), sunstruck_stacks( 0 )
//    {
//    }
//
//    void initialize() override
//    {
//      action_state_t::initialize();
//      sunstruck_stacks = 0;
//    }
//
//    std::ostringstream& debug_str( std::ostringstream& s ) override
//    {
//      action_state_t::debug_str( s ) << " sunstruck_stacks=" << sunstruck_stacks;
//      return s;
//    }
//
//    void copy_state( const action_state_t* s )
//    {
//      action_state_t::copy_state( s );
//      const omnistrike_action_state_t* rs = debug_cast<const omnistrike_action_state_t*>( s );
//      sunstruck_stacks                    = rs->sunstruck_stacks;
//    }
//  };
//
//  omnistrike_t( util::string_view name, gunde_t* p, util::string_view options_str = {} )
//    : gunde_attack_t( name, p, options_str )
//  {
//    id = 4;
//    _procs_golden_hour      = true;
//
//    school                  = SCHOOL_PHYSICAL;
//    attack_power_mod.direct = p->spell_const.omnistrike_coeff;
//
//    name_str_reporting     = "Omnistrike";
//    swift_reprieval_chance = p->spell_const.omnistrike_resource_proc_chance;
//
//    aoe                 = -1;
//    reduced_aoe_targets = p->spell_const.omnistrike_scaling_threshold;
//
//    cooldown->hasted   = true;
//    cooldown->duration = p->spell_const.omnistrike_cooldown;
//    cooldown->charges  = 1;
//
//    energize_type     = action_energize::ON_CAST;
//    energize_amount   = p->spell_const.omnistrike_spirit_gain;
//    energize_resource = RESOURCE_SPIRIT;
//    
//    ability_flags |= ability_type_e::ABILITY_CORE;
//  }
//
//  static const omnistrike_action_state_t* cast_state( const action_state_t* st )
//  {
//    return debug_cast<const omnistrike_action_state_t*>( st );
//  }
//
//  static omnistrike_action_state_t* cast_state( action_state_t* st )
//  {
//    return debug_cast<omnistrike_action_state_t*>( st );
//  }
//
//  action_state_t* new_state() override
//  {
//    return new omnistrike_action_state_t( this, target );
//  }
//
//  void impact( action_state_t* s ) override
//  {
//    gunde_attack_t::impact( s );
//
//    p()->get_target_data( s->target )->debuffs.blind->trigger();
//  }
//
//  double composite_da_multiplier( const action_state_t* s ) const override
//  {
//    double m = gunde_attack_t::composite_da_multiplier( s );
//    if ( p()->talents_enabled( gunde_t::GLEAMING_STRIKES ) && cast_state( s )->sunstruck_stacks > 0 )
//      m *= 1 + p()->talents.gleaming_strikes_omnistrike_sunstruck_amp * cast_state( s )->sunstruck_stacks;
//    return m;
//  }
//
//  void execute() override
//  {
//    int debuffs = 0;
//    for ( auto t : target_list() )
//    {
//      debuffs += p()->get_target_data( t )->debuffs.sunstruck->check();
//
//      p()->get_target_data( t )->debuffs.sunstruck->expire();
//    }
//
//    pre_execute_state                                 = get_state( pre_execute_state );
//    cast_state( pre_execute_state )->sunstruck_stacks = debuffs;
//    snapshot_state( pre_execute_state, amount_type( pre_execute_state ) );
//
//    gunde_attack_t::execute();
//
//    if ( debuffs > 0 )
//    {
//      auto mana_gained = p()->spell_const.sunstruck_mana_refund_base +
//                         ( debuffs - 1 ) * p()->spell_const.sunstruck_mana_refund_per_stack;
//      p()->resource_gain( RESOURCE_MANA, mana_gained, gain, this );
//    }
//
//    if ( p()->talents_enabled( gunde_t::RISING_SUN ) && p()->rng_objects.rising_sun->trigger() )
//    {
//      p()->cooldowns.solar_blades->reset( false, 1 );
//    }
//
//    if ( p()->buffs.golden_hour->check() )
//    {
//      p()->buffs.omega_reprieval->trigger();
//      p()->buffs.golden_hour->expire();
//    }
//  }
//};
//
//struct suns_verdict_t : public gunde_attack_t
//{
//  suns_verdict_t( gunde_t* p ) : gunde_attack_t( "suns_verdict", p )
//  {
//    id = 5;
//
//    background = true;
//
//    school                  = SCHOOL_MAGIC;
//    attack_power_mod.direct = p->talents.suns_verdict_coeff;
//
//    name_str_reporting = "Suns Verdict";
//
//    aoe                 = -1;
//    reduced_aoe_targets = p->talents.suns_verdict_scaling_threshold;
//    full_amount_targets = 1;
//  }
//};
//
//struct solar_blades_t : public gunde_attack_t
//{
//  solar_blades_t( util::string_view name, gunde_t* p, util::string_view options_str = {} )
//    : gunde_attack_t( name, p, options_str )
//  {
//    id = 6;
//    _procs_golden_hour = true;
//
//    use_off_gcd = true;
//
//    gcd_type    = gcd_haste_type::NONE;
//    trigger_gcd = 0_s;
//
//    attack_power_mod.direct = p->spell_const.solar_blades_coeff;
//
//    name_str_reporting     = "Solar Blades";
//    swift_reprieval_chance = p->spell_const.solar_blades_resource_chance;
//
//    aoe                 = -1;
//    reduced_aoe_targets = p->spell_const.solar_blades_scaling_threshold;
//    full_amount_targets = 1;
//
//    cooldown->hasted   = true;
//    cooldown->duration = p->spell_const.solar_blades_cd;
//    cooldown->charges  = 1;
//
//    resource_current            = RESOURCE_MANA;
//    base_costs[ RESOURCE_MANA ] = p->spell_const.solar_blades_mana_cost;
//    ability_flags |= ability_type_e::ABILITY_CORE;
//  }
//
//  void impact( action_state_t* s ) override
//  {
//    gunde_attack_t::impact( s );
//
//    if ( p()->legendary.lego_2 )
//    {
//      p()->get_target_data( s->target )->debuffs.lego_2->trigger();
//    }
//
//    if ( s->chain_target == 0 && p()->talents_enabled( gunde_t::SOLAR_BURN ) )
//    {
//      p()->get_target_data( s->target )->debuffs.sunburn->trigger();
//      // Also do the residual dot.
//    }
//  }
//
//  double composite_crit_chance() const override
//  {
//    double crit = gunde_attack_t::composite_crit_chance();
//
//    if ( p()->legendary.lego_2 && p()->rng().roll( p()->legendary.lego_2_dmg_chance ) )
//      crit += p()->legendary.lego_2_proc_added_cc;
//
//    return crit;
//  }
//
//  double composite_da_multiplier( const action_state_t* s ) const override
//  {
//    double m = gunde_attack_t::composite_da_multiplier( s );
//
//    if ( s->chain_target == 0 )
//      m *= p()->spell_const.solar_blades_main_target_mul;
//
//    return m;
//  }
//
//  void execute() override
//  {
//    gunde_attack_t::execute();
//  }
//};
//
//struct brilliant_flash_action_state_t : public action_state_t
//{
//public:
//  int omega_reprieval;
//
//  brilliant_flash_action_state_t( action_t* action, player_t* target )
//    : action_state_t( action, target ), omega_reprieval( 0 )
//  {
//  }
//
//  void initialize() override
//  {
//    action_state_t::initialize();
//    omega_reprieval = 0;
//  }
//
//  std::ostringstream& debug_str( std::ostringstream& s ) override
//  {
//    action_state_t::debug_str( s ) << " omega_reprieval=" << omega_reprieval;
//    return s;
//  }
//
//  void copy_state( const action_state_t* s )
//  {
//    action_state_t::copy_state( s );
//    const brilliant_flash_action_state_t* rs = debug_cast<const brilliant_flash_action_state_t*>( s );
//  }
//};
//
//template <typename Base>
//struct brilliant_flash_base_t : public gunde_action_t<Base>
//{
//  using ab = gunde_action_t<Base>;
//
//  brilliant_flash_base_t( util::string_view name, gunde_t* p, util::string_view options_str = {} )
//    : ab( name, p, options_str )
//  {
//    ab::id = 7;
//
//    ab::_procs_golden_hour = true;
//
//    ab::name_str_reporting = "Brilliant Flash";
//
//    // aoe                 = -1;
//    ab::reduced_aoe_targets = p->spell_const.omega_reprieval_falloff;
//    ab::full_amount_targets = 1;
//
//    if ( p->talents_enabled( gunde_t::GOLDEN_HOUR ) )
//    {
//      ab::base_dd_multiplier *= p->talents.golden_hour_brilliant_flash_amp;
//    }
//
//    ab::ability_flags |= ability_type_e::ABILITY_POWER;
//  }
//
//  static const brilliant_flash_action_state_t* cast_state( const action_state_t* st )
//  {
//    return debug_cast<const brilliant_flash_action_state_t*>( st );
//  }
//
//  static brilliant_flash_action_state_t* cast_state( action_state_t* st )
//  {
//    return debug_cast<brilliant_flash_action_state_t*>( st );
//  }
//
//  action_state_t* new_state() override
//  {
//    return new brilliant_flash_action_state_t( this, ab::target );
//  }
//
//  void snapshot_state( action_state_t* state, result_amount_type rt ) override
//  {
//    auto rs = cast_state( state );
//
//    rs->omega_reprieval = ab::p()->buffs.omega_reprieval->check();
//
//    ab::snapshot_state( state, rt );
//  }
//
//  void impact( action_state_t* s ) override
//  {
//    ab::impact( s );
//  }
//
//  int n_targets() const override
//  {
//    if ( ab::background )
//      return 1;
//
//    if ( ab::p()->buffs.omega_reprieval->check() )
//      return -1;
//
//    return 1;
//  }
//
//  double composite_da_multiplier( const action_state_t* s ) const override
//  {
//    double m = ab::composite_da_multiplier( s );
//
//    if ( !ab::background && cast_state( s )->omega_reprieval && s->chain_target == 0 && !ab::background )
//      m *= ab::p()->spell_const.omega_reprieval_main_target_mul;
//
//    return m;
//  }
//
//  bool ready() override
//  { 
//    if ( !ab::p()->buffs.omega_reprieval->check() && !ab::p()->buffs.swift_reprieval->check() )
//      return false;
//
//    return ab::ready();
//  }
//
//  void execute() override
//  {
//    ab::execute();
//    
//    if ( ab::background )
//      return;
//
//    if ( ab::p()->buffs.omega_reprieval->check() )
//    {
//      ab::p()->buffs.omega_reprieval->decrement();
//      ab::p()->resource_gain( RESOURCE_MANA,
//                              ab::p()->resources.max[ RESOURCE_MANA ] * ab::p()->spell_const.omega_reprieval_mana_pct,
//                              ab::p()->gains.omega_reprieval, this );
//    }
//    else
//    {
//      ab::p()->buffs.swift_reprieval->decrement();
//    }
//
//    if ( ab::p()->talents_enabled( gunde_t::SUNS_VERDICT ) && ab::p()->rng_objects.suns_verdict->trigger() )
//    {
//      ab::p()->buffs.suns_verdict->trigger();
//    }
//
//    if ( ab::p()->talents_enabled( gunde_t::CELESTIAL_FAVOR ) )
//    {
//      ab::p()->cooldowns.omega_reprieval->adjust( -ab::p()->talents.celestial_favor_cdr, false );
//    }
//  }
//};
//
//struct brilliant_flash_t : public brilliant_flash_base_t<spell_t>
//{
//private:
//  using ab = brilliant_flash_base_t<spell_t>;
//
//public:
//  brilliant_flash_t( util::string_view name, gunde_t* p, util::string_view options_str = {} )
//    : brilliant_flash_base_t<spell_t>( name, p, options_str )
//  {
//    attack_power_mod.direct = p->spell_const.brilliant_flash_dmg_coeff;
//  }
//
//  void execute() override
//  {
//    ab::execute();
//
//    p()->actions.brilliant_flash_heal->execute_on_target( p() );
//  }
//
//  double composite_da_multiplier( const action_state_t* s ) const override
//  {
//    double m = ab::composite_da_multiplier( s );
//
//    if ( s->target == p() )
//      m *= p()->spell_const.brilliant_flash_self_heal_mul;
//
//    return m;
//  }
//};
//
//struct brilliant_flash_heal_t : public brilliant_flash_base_t<heal_t>
//{
//  brilliant_flash_heal_t( util::string_view name, gunde_t* p, util::string_view options_str = {} )
//    : brilliant_flash_base_t<heal_t>( name, p, options_str )
//  {
//    attack_power_mod.direct = p->spell_const.brilliant_flash_heal_coeff;
//  }
//};
//
//struct omega_repreival_t : public gunde_spell_t
//{
//  omega_repreival_t( util::string_view name, gunde_t* p, util::string_view options_str = {} )
//    : gunde_spell_t( name, p, options_str )
//  {
//    id = 9;
//
//    use_off_gcd = true;
//
//    gcd_type    = gcd_haste_type::NONE;
//    trigger_gcd = 0_s;
//
//    name_str_reporting = "Omega Reprieval";
//
//    cooldown->hasted   = false;
//    cooldown->duration = p->spell_const.omega_reprieval_cd;
//    cooldown->charges  = 1;
//
//    ability_flags |= ability_type_e::ABILITY_MAJOR;
//  }
//
//  void execute() override
//  {
//    gunde_spell_t::execute();
//
//    p()->buffs.omega_reprieval->trigger( 2 );
//  }
//};
//
//struct vanguard_of_vengeance_t : public gunde_attack_t
//{
//  vanguard_of_vengeance_t( gunde_t* p ) : gunde_attack_t( "vanguard_of_vengeance", p )
//  {
//    id = 10;
//
//    background = true;
//    name_str_reporting = "Vanguard of Vengeance";
//
//    energize_resource = RESOURCE_MANA;
//    energize_amount   = p->resources.max[ RESOURCE_MANA ] * p->talents.vanguard_of_vengeance_mana_gain_pct;
//
//    attack_power_mod.direct = p->talents.vanguard_of_vengeance_coeff;
//  }
//
//  void execute() override
//  {
//    p()->buffs.vanguard_of_vengeance->trigger();
//    base_t::execute();
//  }
//};
//
//struct solaris_t : public gunde_spell_t
//{
//  solaris_t( gunde_t* p ) : gunde_spell_t( "solaris", p )
//  {
//    id                 = 11;
//    background         = true;
//    name_str_reporting = "Solaris";
//
//    may_crit = false;
//
//    aoe = -1;
//    reduced_aoe_targets = p->spell_const.solaris_target_scaling_threshold;
//  }
//
//  void init_finished()
//  {
//    snapshot_flags &= STATE_NO_MULTIPLIER;
//    snapshot_flags |= STATE_TARGET_NO_PET & ~STATE_TGT_CRIT;
//  }
//};
//
//struct decree_of_the_sun_t : public gunde_spell_t
//{
//  decree_of_the_sun_t( std::string_view name, gunde_t* p, std::string_view opt ) : gunde_spell_t( "decree_of_the_sun", p, opt )
//  {
//    id                 = 12;
//    name_str_reporting = "Decree of the Sun";
//
//    resource_current              = RESOURCE_SPIRIT;
//    base_costs[ RESOURCE_SPIRIT ] = 100;
//    ability_flags |= ability_type_e::ABILITY_SPIRIT;
//    
//    use_off_gcd = true;
//
//    gcd_type    = gcd_haste_type::NONE;
//    trigger_gcd = 0_s;
//  }
//
//  void execute() override
//  {
//    gunde_spell_t::execute();
//    p()->fs_buffs.spirit_of_heroism->trigger();
//    p()->buffs.decree_of_the_sun->trigger();
//    p()->buffs.decree_of_the_sun_invuln->trigger();
//    p()->used_ultimate();
//  }
//};
//
//struct decree_of_the_sun_dmg_t : public gunde_spell_t
//{
//  decree_of_the_sun_dmg_t( gunde_t* p ) : gunde_spell_t( "decree_of_the_sun_dmg", p )
//  {
//    id                      = 12;
//    background              = true;
//    name_str_reporting      = "Decree of the Sun Damage";
//    attack_power_mod.direct = p->spell_const.decree_of_the_sun_dmg_coeff;
//    aoe                     = -1;
//    reduced_aoe_targets     = p->spell_const.decree_of_the_sun_falloff;
//    ability_flags |= ability_type_e::ABILITY_SPIRIT;
//  }
//};
//
//struct decree_of_the_sun_heal_t : public gunde_heal_t
//{
//  decree_of_the_sun_heal_t( gunde_t* p ) : gunde_heal_t( "decree_of_the_sun_heal", p )
//  {
//    id                      = 12;
//    background              = true;
//    name_str_reporting      = "Decree of the Sun Heal";
//    attack_power_mod.direct = p->spell_const.decree_of_the_sun_heal_coeff;
//    aoe                     = -1;
//    ability_flags |= ability_type_e::ABILITY_SPIRIT;
//  }
//};

}  // namespace actions

gunde_td_t::gunde_td_t( player_t* target, gunde_t* source )
  : fellowship::fs_player_td_t( target, source ), dots(), debuffs()
{
  dots.rend      = target->get_dot( "rend", source );
  dots.slaughter = target->get_dot( "slaughter", source );
  //dots.sunburn    = target->get_dot( "sunburn", source );
  //dots.suns_touch = target->get_dot( "suns_touch", source );

  //debuffs.blind = make_buff( *this, "blind" )
  //                    ->set_duration( source->spell_const.blinding_slash_blind_duration )
  //                    ->set_max_stack( source->spell_const.blinding_slash_max_stacks )
  //                    ->set_default_value( source->spell_const.blinding_slash_parry_chance )
  //                    ->set_refresh_behavior( buff_refresh_behavior::DURATION );

  //debuffs.lego_2 = make_buff( *this, "lego_2" )
  //                          ->set_duration( source->legendary.lego_2_duration )
  //                          ->set_max_stack( 1 )
  //                          ->set_default_value( source->legendary.lego_2_dmg_taken_mul )
  //                          ->set_refresh_behavior( buff_refresh_behavior::DURATION );

  //debuffs.sunburn = make_buff( *this, "sunburn" )
  //                      ->set_duration( source->talents.solar_burn_duration )
  //                      ->set_period( source->talents.solar_burn_period )
  //                      ->set_default_value( source->talents.solar_burn_dmg_received_multiplier )
  //                      ->set_refresh_behavior( buff_refresh_behavior::DURATION );

  //debuffs.sunstruck = make_buff( *this, "sunstruck" )
  //                        ->set_duration( source->spell_const.sunstruck_duration )
  //                        ->set_max_stack( source->spell_const.suntruck_max_stacks )
  //                        ->set_refresh_behavior( buff_refresh_behavior::DURATION );
}

// gunde_t::composite_attribute_multiplier ==================================

double gunde_t::composite_attribute_multiplier( attribute_e a ) const
{
  double am = fs_player_t::composite_attribute_multiplier( a );

  return am;
}

// gunde_t::composite_melee_auto_attack_speed ===============================

double gunde_t::composite_melee_auto_attack_speed() const
{
  double h = fs_player_t::composite_melee_auto_attack_speed();

  return h;
}

// gunde_t::composite_melee_haste ==========================================

double gunde_t::composite_melee_haste() const
{
  double h = fs_player_t::composite_melee_haste();

  return h;
}

// gunde_t::composite_spell_haste ==========================================

double gunde_t::composite_spell_haste() const
{
  double h = fs_player_t::composite_spell_haste();

  return h;
}

// gunde_t::composite_melee_crit_chance =========================================

double gunde_t::composite_melee_crit_chance() const
{
  double crit = fs_player_t::composite_melee_crit_chance();

  return crit;
}

// gunde_t::composite_spell_crit_chance =========================================

double gunde_t::composite_spell_crit_chance() const
{
  double crit = fs_player_t::composite_spell_crit_chance();

  return crit;
}

// gunde_t::composite_damage_versatility ===================================

double gunde_t::composite_damage_versatility() const
{
  double cdv = fs_player_t::composite_damage_versatility();

  return cdv;
}

// gunde_t::composite_heal_versatility =====================================

double gunde_t::composite_heal_versatility() const
{
  double chv = fs_player_t::composite_heal_versatility();

  return chv;
}

// gunde_t::composite_leech ===============================================

double gunde_t::composite_leech() const
{
  double l = fs_player_t::composite_leech();

  return l;
}

// gunde_t::matching_gear_multiplier ========================================

double gunde_t::matching_gear_multiplier( attribute_e attr ) const
{
  return 0.0;
}

// gunde_t::composite_player_multiplier =====================================

double gunde_t::composite_player_multiplier( school_e school ) const
{
  double m = fs_player_t::composite_player_multiplier( school );

  return m;
}

// gunde_t::composite_player_pet_damage_multiplier ==========================

double gunde_t::composite_player_pet_damage_multiplier( const action_state_t* s, bool guardian ) const
{
  double m = fs_player_t::composite_player_pet_damage_multiplier( s, guardian );

  return m;
}

// gunde_t::composite_player_target_multiplier ==============================

double gunde_t::composite_player_target_multiplier( player_t* target, school_e school ) const
{
  double m = fs_player_t::composite_player_target_multiplier( target, school );

  return m;
}

// gunde_t::composite_player_target_crit_chance =============================

double gunde_t::composite_player_target_crit_chance( player_t* target ) const
{
  double c = fs_player_t::composite_player_target_crit_chance( target );

  return c;
}

// gunde_t::composite_player_target_armor ===================================

double gunde_t::composite_player_target_armor( player_t* target ) const
{
  return 0.0;

  double a = fs_player_t::composite_player_target_armor( target );

  return a;
}
// gunde_t::init_actions ====================================================

void gunde_t::init_action_list()
{
  if ( !action_list_str.empty() )
  {
    fs_player_t::init_action_list();
    return;
  }

  clear_action_priority_lists();

  use_default_action_list = true;

  fs_player_t::init_action_list();
}

// gunde_t::create_action  ==================================================

action_t* gunde_t::create_action( util::string_view name, util::string_view options_str )
{
  using namespace actions;

  if ( name == "double_strike" )
    return new double_strike_t( name, this, options_str );

  return fs_player_t::create_action( name, options_str );
}

// gunde_t::create_expression ===============================================

std::unique_ptr<expr_t> gunde_t::create_action_expression( action_t& action, std::string_view name_str )
{
  auto split = util::string_split<util::string_view>( name_str, "." );

  return fs_player_t::create_action_expression( action, name_str );
}

std::unique_ptr<expr_t> gunde_t::create_expression( util::string_view name_str )
{
  auto split = util::string_split<util::string_view>( name_str, "." );

  if ( util::str_compare_ci( split[ 0 ], "legendary" ) )
  {
    if ( split.size() == 2 )
    {
      if ( util::str_compare_ci( split[ 1 ], "lego_2" ) )
      {
        return make_ref_expr( name_str, legendary.lego_2 );
      }
      else if ( util::str_compare_ci( split[ 1 ], "lego_3" ) )
      {
        return make_ref_expr( name_str, legendary.lego_3 );
      }
      else if ( util::str_compare_ci( split[ 1 ], "lego_1" ) )
      {
        return make_ref_expr( name_str, legendary.lego_1 );
      }
    }
  }
  else if ( util::str_compare_ci( split[ 0 ], "talent" ) )
  {
    if ( split.size() == 2 )
    {
      for ( gunde_talents_t t = static_cast<gunde_talents_t>( 1U ); t < gunde_talents_t::MAX; t++ )
      {
        if ( util::str_compare_ci( split[ 1 ], talent_name( t ) ) )
        {
          return make_fn_expr( name_str, std::bind( std::mem_fn( &gunde_t::talents_enabled ), this, t ) );
        }
      }
    }
  }
  // Split expressions

  return fs_player_t::create_expression( name_str );
}

std::unique_ptr<expr_t> gunde_t::create_resource_expression( util::string_view name_str )
{
  return fs_player_t::create_resource_expression( name_str );
}

// gunde_t::init_base =======================================================

void gunde_t::init_base_stats()
{
  if ( base.distance < 1 )
    base.distance = 5;

  fs_player_t::init_base_stats();

  base.stats.attribute[ STAT_STRENGTH ] = 100;
  resources.base[ RESOURCE_HEALTH ]     = 1811;

  base.health_per_stamina = 53.332;

  base_gcd = timespan_t::from_seconds( 1.5 );
  min_gcd  = timespan_t::from_seconds( 0 );
  //min_gcd  = timespan_t::from_seconds( 0.75 );

  base.parry = 0.05;
  base.dodge = 0.05;
}

// gunde_t::init_spells =====================================================

void gunde_t::init_spells()
{
  fs_player_t::init_spells();

  actions.auto_attack = new actions::auto_melee_attack_t( this, "" );
}

// gunde_t::init_gains ======================================================

void gunde_t::init_gains()
{
  fs_player_t::init_gains();

  gains.spirit_procs    = get_gain( "Spirit Procs" );
}

// gunde_t::init_procs ======================================================

void gunde_t::init_procs()
{
  fs_player_t::init_procs();
}

// gunde_t::init_scaling ====================================================

void gunde_t::init_scaling()
{
  fs_player_t::init_scaling();

  scaling->disable( STAT_AGILITY );
  scaling->disable( STAT_INTELLECT );

  // Break out early if scaling is disabled on this player, or there's no
  // scaling stat
  if ( !scale_player || sim->scaling->scale_stat == STAT_NONE )
  {
    return;
  }
}

// gunde_t::init_resources =================================================

void gunde_t::init_resources( bool force )
{
  fs_player_t::init_resources( force );
}

// gunde_t::init_buffs ======================================================

void gunde_t::create_buffs()
{
  fs_player_t::create_buffs();

  //buffs.decree_of_the_sun =
  //    make_buff<gunde_buff_t>( this, "decree_of_the_sun" )
  //        ->set_duration( spell_const.decree_of_the_sun_dr_duration + spell_const.decree_of_the_sun_invuln_duration )
  //        ->set_max_stack( 1 )
  //        ->set_refresh_behavior( buff_refresh_behavior::DURATION )
  //        ->set_period( spell_const.decree_of_the_sun_pulse_period )
  //        ->set_tick_callback( [ this ]( buff_t*, int, timespan_t ) { actions.decree_of_the_sun_heal->execute(); } );

  //buffs.decree_of_the_sun_invuln = make_buff<gunde_buff_t>( this, "decree_of_the_sun_invuln" )
  //                                     ->set_duration( spell_const.decree_of_the_sun_invuln_duration )
  //                                     ->set_max_stack( 1 )
  //                                     ->set_refresh_behavior( buff_refresh_behavior::DURATION )
  //                                     ->add_stack_change_callback( [ this ]( buff_t* b, int old, int _new ) {
  //                                       if ( !_new )
  //                                       {
  //                                         actions.decree_of_the_sun_dmg->execute();
  //                                         buffs.decree_of_the_sun_dr->trigger();
  //                                       }
  //                                     } );

  //buffs.decree_of_the_sun_dr = make_buff<gunde_buff_t>( this, "decree_of_the_sun_dr" )
  //                                 ->set_duration( spell_const.decree_of_the_sun_dr_duration )
  //                                 ->set_max_stack( 1 )
  //                                 ->set_refresh_behavior( buff_refresh_behavior::DURATION );

  //buffs.vanguard_of_vengeance = make_buff<gunde_buff_t>( this, "vanguard_of_vengeance" )
  //                                  ->set_duration( talents.vanguard_of_vengeance_buff_duration )
  //                                  ->set_max_stack( talents.vanguard_of_vengeance_max_stacks )
  //                                  ->set_default_value( talents.vanguard_of_vengeance_spirit_per_stack )
  //                                  ->set_pct_buff_type( STAT_PCT_BUFF_MASTERY );

  //buffs.swift_reprieval = make_buff<gunde_buff_t>( this, "swift_reprieval" )
  //                            ->set_max_stack( spell_const.swift_reprieval_max_stacks )
  //                            ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

  //buffs.aura_of_solace = make_buff<gunde_buff_t>( this, "aura_of_solace" )
  //                           ->set_default_value( spell_const.aura_of_solace_parry )
  //                           ->add_stack_change_callback( [ this ]( buff_t* b, int old, int _new ) {
  //                             if ( _new > old )
  //                             {
  //                               b->player->current.parry += b->current_value;
  //                             }
  //                             else
  //                             {
  //                               b->player->current.parry -= b->current_value;
  //                             }
  //                           } )
  //                           ->add_invalidate( CACHE_PARRY );

  //buffs.suns_verdict = make_buff<gunde_buff_t>( this, "suns_verdict" )
  //                         ->set_max_stack( talents.suns_verdict_max_stacks )
  //                         ->set_duration( talents.suns_verdict_duration );

  //buffs.golden_hour = make_buff<gunde_buff_t>( this, "golden_hour" )
  //                        ->set_duration( talents.golden_hour_buff_duration )
  //                        ->set_max_stack( 1 )
  //                        ->set_refresh_behavior( buff_refresh_behavior::DURATION );

  //buffs.omega_reprieval = make_buff<gunde_buff_t>( this, "omega_reprieval" )
  //                            ->set_max_stack( spell_const.omega_reprieval_max_stacks )
  //                            ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );
}

// gunde_t::invalidate_cache =========================================

void gunde_t::invalidate_cache( cache_e c )
{
  fs_player_t::invalidate_cache( c );

  switch ( c )
  {
    case CACHE_HASTE:
      invalidate_cache( CACHE_AUTO_ATTACK_SPEED );
      break;
    default:
      break;
  }
}

void gunde_t::create_options()
{
  fs_player_t::create_options();
}

// gunde_t::copy_from =======================================================

void gunde_t::copy_from( player_t* source )
{
  gunde_t* gunde = static_cast<gunde_t*>( source );
  fs_player_t::copy_from( source );

  talents     = gunde->talents;
  legendary   = gunde->legendary;
  options     = gunde->options;
  spell_const = gunde->spell_const;
}

// gunde_t::create_profile  =================================================

std::string gunde_t::create_profile( save_e stype )
{
  std::string profile_str = fs_player_t::create_profile( stype );

  // Break out early if we are not saving everything, or gear
  if ( !( stype & SAVE_PLAYER ) && !( stype & SAVE_GEAR ) )
  {
    return profile_str;
  }

  std::string term = "\n";

  return profile_str;
}

// gunde_t::init_items ======================================================

void gunde_t::init_items()
{
  fs_player_t::init_items();
}

// gunde_t::init_special_effects ============================================

void gunde_t::init_special_effects()
{
  fs_player_t::init_special_effects();

  //{
  //  auto effect                   = new special_effect_t( this );
  //  effect->spell_id              = 107;
  //  effect->name_str              = "gunde_parry";
  //  effect->proc_flags_           = PF_DAMAGE_TAKEN;
  //  effect->proc_flags2_          = PF2_PARRY;
  //  effect->rppm_scale_           = rppm_scale_e::RPPM_NONE;
  //  effect->proc_chance_          = 1.0;
  //  effect->type                  = special_effect_e::SPECIAL_EFFECT_EQUIP;

  //  special_effects.push_back( effect );

  //  struct parry_cb_t : dbc_proc_callback_t
  //  {
  //    parry_cb_t( gunde_t* p, const special_effect_t& e ) : dbc_proc_callback_t( p, e )
  //    {
  //    }

  //    gunde_t* p() const
  //    {
  //      return static_cast<gunde_t*>( listener );
  //    }

  //    void execute( action_t*, action_state_t* s ) override
  //    {
  //      p()->parry_effects( s );
  //    }
  //  };

  //  auto cb = new parry_cb_t( this, *effect );
  //  cb->initialize();
  //  cb->activate();
  //}

  //{
  //  auto effect                   = new special_effect_t( this );
  //  effect->spell_id              = 108;
  //  effect->name_str              = "solaris";
  //  effect->proc_flags_           = PF_ALL_HEAL_TAKEN;
  //  effect->proc_flags2_          = PF2_ALL_HIT | PF2_PERIODIC_HEAL;
  //  effect->set_can_proc_from_procs( true );
  //  effect->rppm_scale_           = rppm_scale_e::RPPM_NONE;
  //  effect->proc_chance_          = 1.0;
  //  effect->type                  = special_effect_e::SPECIAL_EFFECT_EQUIP;

  //  special_effects.push_back( effect );

  //  struct solaris_cb_t : dbc_proc_callback_t
  //  {
  //    solaris_cb_t( gunde_t* p, const special_effect_t& e ) : dbc_proc_callback_t( p, e )
  //    {
  //    }

  //    gunde_t* p() const
  //    {
  //      return static_cast<gunde_t*>( listener );
  //    }

  //    void execute( action_t*, action_state_t* s ) override
  //    {
  //      auto overheal = s->result_total - s->result_amount;

  //      if ( overheal > 0 )
  //      {
  //        p()->actions.solaris->execute_on_target( p()->actions.solaris->target,
  //                                                 overheal * p()->spell_const.solaris_overheal_scaler );
  //      }
  //    }
  //  };

  //  auto cb = new solaris_cb_t( this, *effect );
  //  cb->initialize();
  //  cb->activate();
  //}
}

// gunde_t::init_finished ===================================================

void gunde_t::init_finished()
{
  fs_player_t::init_finished();
}

void gunde_t::init_talents()
{
  fs_player_t::init_talents();

  auto talents = util::string_split<std::string_view>( talents_str, "/" );
  for ( const auto talent : talents )
  {
    auto talent_split = util::string_split<std::string_view>( talent, ":" );
    if ( talent_split.size() != 2 )
    {
      sim->error( "Invalid talent string {}", talent );
      sim->cancel();
      return;
    }

    auto ranks = util::to_unsigned( talent_split[ 1 ] );

    for ( gunde_talents_t t = static_cast<gunde_talents_t>( 1U ); t < gunde_talents_t::MAX; t++ )
    {
      if ( util::str_compare_ci( talent_split[ 0 ], talent_name( t ) ) )
      {
        set_talent_points( t, ranks >= 1 );
        break;
      }
    }
  }
}

void gunde_t::init_background_actions()
{
  fs_player_t::init_background_actions();

  //actions.suns_verdict                     = new actions::suns_verdict_t( this );

  //actions.brilliant_flash_heal             = new actions::brilliant_flash_heal_t( "brilliant_flash_heal", this, {} );
  //actions.brilliant_flash_heal->background = true;

  //actions.vanguard_of_vengeance  = new actions::vanguard_of_vengeance_t( this );
  //actions.solaris                = new actions::solaris_t( this );
  //actions.decree_of_the_sun_dmg  = new actions::decree_of_the_sun_dmg_t( this );
  //actions.decree_of_the_sun_heal = new actions::decree_of_the_sun_heal_t( this );
}

void gunde_t::init_rng()
{
  fs_player_t::init_rng();

  //rng_objects.golden_hour = get_accumulated_rng( "golden_hour", rng::CfromP( talents.golden_hour_proc_chance ) );

  //rng_objects.rising_sun = get_accumulated_rng( "rising_sun", rng::CfromP( talents.rising_sun_chance ) );

  //rng_objects.suns_verdict = get_accumulated_rng( "suns_verdict", rng::CfromP( talents.suns_verdict_chance ) );
}

// gunde_t::reset ===========================================================

void gunde_t::reset()
{
  fs_player_t::reset();
}

// gunde_t::activate ========================================================

void gunde_t::activate()
{
  fs_player_t::activate();
}

// gunde_t::cancel_auto_attack ==============================================

void gunde_t::cancel_auto_attacks()
{
  if ( actions.melee_hit && actions.melee_hit->execute_event )
  {
    actions.melee_hit->canceled            = true;
    actions.melee_hit->prev_scheduled_time = actions.melee_hit->execute_event->occurs();
  }

  fs_player_t::cancel_auto_attacks();
}

// gunde_t::arise ===========================================================

void gunde_t::arise()
{
  fs_player_t::arise();
}

// gunde_t::combat_begin ====================================================

void gunde_t::combat_begin()
{
  fs_player_t::combat_begin();
}

// gunde_t::energy_regen_per_second =========================================

double gunde_t::resource_regen_per_second( resource_e r ) const
{
  double reg = fs_player_t::resource_regen_per_second( r );

  return reg;
}

double gunde_t::resource_gain( resource_e resource_type, double amount, gain_t* source, action_t* action )
{
  double actual_amount = fs_player_t::resource_gain( resource_type, amount, source, action );

  return actual_amount;
}

// gunde_t::non_stacking_movement_modifier ==================================

double gunde_t::non_stacking_movement_modifier() const
{
  double ms = fs_player_t::non_stacking_movement_modifier();

  return ms;
}

// gunde_t::stacking_movement_modifier===================================

double gunde_t::stacking_movement_modifier() const
{
  double ms = fs_player_t::stacking_movement_modifier();

  return ms;
}

// gunde_t::regen ===========================================================

void gunde_t::regen( timespan_t periodicity )
{
  fs_player_t::regen( periodicity );
}

template <typename Base>
void actions::gunde_action_t<Base>::trigger_auto_attack( const action_state_t* /* state */ )
{
  if ( !p()->main_hand_attack || p()->main_hand_attack->execute_event )
    return;

  p()->actions.auto_attack->schedule_execute();
}

template <typename Base>
void actions::gunde_action_t<Base>::trigger_spirit_refund( const action_state_t* state )
{
  p()->spirit_refund();
}

template <typename Base>
void actions::gunde_action_t<Base>::apply_rend( const action_state_t* state )
{
  if ( !_applies_rend )
    return;

   auto td = p()->get_target_data( state->target );

}


// gunde_t::convert_hybrid_stat ==============================================

stat_e gunde_t::convert_hybrid_stat( stat_e s ) const
{
  // this converts hybrid stats that either morph based on spec or only work
  // for certain specs into the appropriate "basic" stats
  switch ( s )
  {
    case STAT_AGI_INT:
      return STAT_NONE;
    case STAT_STR_AGI_INT:
    case STAT_STR_AGI:
    case STAT_STR_INT:
      return STAT_STRENGTH;
    case STAT_BONUS_ARMOR:
      return STAT_NONE;
    default:
      return s;
  }
}

void gunde_t::create_cooldowns()
{
  //cooldowns.blinding_slash  = get_cooldown( "blinding_slash" );
  //cooldowns.omega_reprieval = get_cooldown( "omega_reprieval" );
  //cooldowns.omnistrike      = get_cooldown( "omnistrike" );
  //cooldowns.solar_blades    = get_cooldown( "solar_blades" );
  //cooldowns.solar_shield    = get_cooldown( "solar_shield" );

    cooldowns.blood_arc      = get_cooldown( "blood_arc" );
  cooldowns.grim_carve     = get_cooldown( "grim_carve" );
  cooldowns.heart_splitter = get_cooldown( "heart_splitter" );
  cooldowns.reavers_edge   = get_cooldown( "reavers_edge" );
  cooldowns.reign_in_blood = get_cooldown( "reign_in_blood" );
  cooldowns.rupture        = get_cooldown( "rupture" );
  cooldowns.slaughter      = get_cooldown( "slaughter" );
}

class gunde_module_t : public module_t
{
public:
  gunde_module_t() : module_t( GUNDE )
  {
  }

  player_t* create_player( sim_t* sim, util::string_view name, race_e r = RACE_NONE ) const override
  {
    return new gunde_t( sim, name, r );
  }

  bool valid() const override
  {
    return true;
  }

  void static_init() const override
  {
  }

  void register_hotfixes() const override
  {
  }

  void init( player_t* ) const override
  {
  }
  void combat_begin( sim_t* ) const override
  {
  }
  void combat_end( sim_t* ) const override
  {
  }
};

}  // namespace gunde
}  // namespace fellowship

const module_t* module_t::gunde()
{
  static fellowship::gunde::gunde_module_t m;
  return &m;
}
