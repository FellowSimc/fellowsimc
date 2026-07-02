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

  struct rend_tracker_t
  {
    uint8_t current_tick = 0;
    // Hardcoded to have enough buckets currently for Deep Rend.
    std::array<double, 11> tick_buckets = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  } rend_tracker, slaughter_tracker;

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
    action_t* rend;
    action_t* slaughter;
    action_t* ravens_precision;
    action_t* bloodcraze;
    action_t* heart_splitter;
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
    buff_t* ancestral_instinct;
    buff_t* bloodbath;
    buff_t* bloodbound_spirit;
    buff_t* murder_of_crows;
    buff_t* heartsplitter_slaughter;
    buff_t* carrion_onslaught;
    buff_t* carrion_onslaught_feathers;
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

  struct procs_t
  {
    proc_t* feathers;
  } procs;

  struct rng_objects_t
  {
    accumulated_rng_t* grim_harvest;
    accumulated_rng_t* deaths_arc;
    accumulated_rng_t* ancestral_instinct;
    accumulated_rng_t* deep_rend;
    accumulated_rng_t* heart_splitter_twice;
    accumulated_rng_t* heart_splitter_twice_2;
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
  X( FRENZIED_REIGN, "frenzied_reign", "Frenzied Reign" )              \
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
    timespan_t rend_period = 3_s;

    // Gunde.PerTargetAccumulatedBleed.TotalDamageToStackCountScaler, 0.0025 ; So 1/100 damage

    // Gunde.BoostedDotDamage.Debuff.BleedDamageScaler, 1.20
    double open_wounds_modifier = 1.2;
    // Gunde.BoostedDotDamage.Debuff.Duration, 18.0
    timespan_t open_wounds_duration = 18_s;

    // Gunde.ExtraDotDamageGain.BonusAmount, 0.2		;Blood Arc buff
    double blood_arc_buff_additional_rend = 0.2;
    // Gunde.ExtraDotDamageGain.MaxStacks, 1.0
    int blood_arc_buff_max_stacks = 1;
    // Gunde.ExtraDotDamageGain.Duration, 8.0
    timespan_t blood_arc_buff_duration = 8_s;

    // Gunde.BasicSingleTargetAttack.DamageStrengthCoefficientPerHit, 0.825         ; DOUBLE STRIKE WAS 1.0
    double double_strike_coeff = 0.825;
    // Gunde.BasicSingleTargetAttack.FirstVisualHitDelay, 0.1
    timespan_t double_strike_first_hit_delay = 0.1_s;
    // Gunde.BasicSingleTargetAttack.SecondVisualHitDelay, 0.45
    timespan_t double_trike_second_hit_delay = 0.45_s;
    // Gunde.BasicSingleTargetAttack.MaxRange, 3000.0

    // Gunde.InstantWhirlwindAoe.VisualHitDelay, 0.45                               ; REAVER'S EDGE
    timespan_t reavers_edge_delay = 0.45_s;
    // Gunde.InstantWhirlwindAoe.AoeRadius, 700.0
    // Gunde.InstantWhirlwindAoe.DamageStrengthCoefficient, 2.20                   ; WAS 1.616
    double reavers_edge_coeff = 2.0317;
    // Gunde.InstantWhirlwindAoe.DamageScalingTargetCountThreshold, 8.0			 ; WAS 3.0
    double reavers_edge_target_threshold = 8.0;
    // Gunde.InstantWhirlwindAoe.Cooldown, 5.0
    timespan_t reavers_edge_cooldown = 5_s;
    // Gunde.InstantWhirlwindAoe.Talent.GatherOrbs.MaxRange, 700.0

    // Gunde.HeavyMeleeDotBoost.VisualHitDelay, 0.3       ; Rupture
    timespan_t rupture_visual_delay = 0.3_s;
    // Gunde.HeavyMeleeDotBoost.DamageStrengthCoefficient, 21.26                      ; WAS 8.4 then 11.16
    double rupture_coeff = 21.26;
    // Gunde.HeavyMeleeDotBoost.Cooldown, 60.0
    timespan_t rupture_cooldown = 60_s;

    // Gunde.TargetedAoeProjectile.DamageStrengthCoefficientPerHit, 2.076           ; WAS 1.635
    double grim_carve_coeff = 2.076;
    // Gunde.TargetedAoeProjectile.DamageScalingTargetCountThreshold, 5.0			; was 12.0
    double grim_carve_falloff = 5;
    // Gunde.TargetedAoeProjectile.Aoe.Duration, 1.15
    timespan_t grim_carve_duration = 1.15_s;
    // Gunde.TargetedAoeProjectile.Aoe.Period, 0.5
    timespan_t grim_carve_period = 0.5_s;
    // Gunde.TargetedAoeProjectile.ProjectileSpawnDelay, 0.14                        ; GRIM CARVE
    timespan_t grim_carve_initial_delay = 0.14_s;
    // Gunde.TargetedAoeProjectile.Speed, 5000.0
    // Gunde.TargetedAoeProjectile.Cooldown, 15.0
    timespan_t grim_carve_cooldown = 15_s;
    // Gunde.TargetedAoeProjectile.MaxRange, 3000.0
    // Gunde.TargetedAoeProjectile.Aoe.Radius, 700.0

    // Gunde.DamageReductionSelfBuff.IncomingDamageMultiplier, 0.6                 ; RECKLESS ABANDON
    // Gunde.DamageReductionSelfBuff.Duration, 4.0
    // Gunde.DamageReductionSelfBuff.Cooldown, 30.0
    // Gunde.DamageReductionSelfBuff.VisualDelay, 0.24

    // Gunde.SingleTargetInterrupt.MaxRange, 500.0                                 ; HEADSPLITTER
    // Gunde.SingleTargetInterrupt.Duration, 4.0
    // Gunde.SingleTargetInterrupt.Cooldown, 16.0
    // Gunde.SingleTargetInterrupt.VisualDelay, 0.2

    // Gunde.ExtraDotDamageBuff.BonusDotDamage, 0.5                                ; REIGN IN BLOOD
    double reign_in_blood_additional_rend = 0.3;
    // Gunde.ExtraDotDamageBuff.Duration, 12.0
    timespan_t reign_in_blood_duration = 12.0_s;
    // Gunde.ExtraDotDamageBuff.Cooldown, 90.0
    timespan_t reign_in_blood_cooldown = 90.0_s;
    // Gunde.ExtraDotDamageBuff.VisualDelay, 0.2

    // Gunde.HeavyMeleeDotBased.Cooldown, 12.0                                     ; RECKONING / heart splitter
    timespan_t heart_splitter_cooldown = 12.0_s;
    // Gunde.HeavyMeleeDotBased.Damage.StrengthCoefficient, 3.70                   ; WAS 3.4
    double heart_splitter_coeff = 4.7;
    // Gunde.HeavyMeleeDotBased.Damage.DotCoefficient, 0.30						; WAS 0.10
    double heart_splitter_exsanguinate_coeff = 0.3;
    // Gunde.HeavyMeleeDotBased.VisualDelay, 0.35
    timespan_t heart_splitter_visual_delay = 0.35_s;
    double heart_splitter_additional_rend  = 0.5;

    // Gunde.InstantAoeDot.Aoe.Radius, 1000.0                                      ; SLAUGHTER
    // Gunde.InstantAoeDot.Aoe.Height, 500.0
    // Gunde.InstantAoeDot.Dot.AccDotToNewDotScaler, 1.60                           ;WAS 1.30
    double slaughter_rend_multiplier = 1.60;
    // Gunde.InstantAoeDot.Dot.Duration, 3.0
    timespan_t slaughter_duration = 3.0_s;
    // Gunde.InstantAoeDot.Dot.Period, 0.3
    timespan_t slaughter_period = 0.3_s;
    // Gunde.InstantAoeDot.VisualDelay, 0.5
    // Gunde.InstantAoeDot.Cooldown, 30.0
    // ;WAS 18.0
    timespan_t slaughter_cooldown = 30.0_s;
    // Gunde.InstantAoeDot.Talent.IncreasedCritChance.AdditionalCritChance, 0.1
    // Gunde.InstantAoeDot.Talent.StacksToHaste.Duration, 8.0
    // Gunde.InstantAoeDot.Talent.StacksToHaste.AdditionalHastePerStack, 0.0025
    // Gunde.InstantAoeDot.Talent.StacksToHaste.MaxStacks, 100.0

    // Gunde.DotTransfer.MaxRange, 2000.0                                          ; OWED IN BLOOD
    // Gunde.DotTransfer.SelfBuff.Duration, 45.0			;was 30
    timespan_t owed_in_blood_duration = 45_s;
    // Gunde.DotTransfer.VisualDelay, 0.25
    // Gunde.DotTransfer.Target.Cooldown, 3.0
    timespan_t owed_in_blood_cooldown = 3_s;

    // Gunde.DotTransfer.Orb.Lifespan, 30.0                                        ;OWED IN BLOOD NEW FUNCTION
    timespan_t feather_duration = 30_s;
    // Gunde.DotTransfer.Orb.MaxActiveOrbPickups, 20.0
    int feathers_max_on_ground = 20;
    // Gunde.DotTransfer.Orb.ActivationDelay, 0.5  								; Mainly
    // to allow the pickups to spawn and linger for a while when the player is already in the trigger
    // Gunde.DotTransfer.Orb.Scale, 1.5                                            ;WAS 1.0
    // Gunde.DotTransfer.Orb.PickUpRadius, 360.0                                   ;WAS 150.0
    // Gunde.DotTransfer.Orb.NumOfStacksPerOrb, 1.0                                ;WAS 2.0
    int feathers_per_orb = 1;
    // Gunde.DotTransfer.Orb.RelevantDropRadius, 5000.0	                        ; How far away an enemy can die and
    // still drop an orb Gunde.DotTransfer.Orb.SpawnChanceAtDeath, 0.25
    double feather_chance_on_kill = 0.25;
    // Gunde.DotTransfer.Orb.SpawnChanceAtTick.A, 1.0
    // Gunde.DotTransfer.Orb.SpawnChanceAtTick.B, 0.50
    // Gunde.DotTransfer.Orb.SpawnChanceAtTick.C, 0.33
    // Gunde.DotTransfer.Orb.SpawnChanceAtTick.D, 0.25
    // Gunde.DotTransfer.Orb.SpawnChanceAtTick.E, 0.20
    // Gunde.DotTransfer.Orb.SpawnChanceAtTick.F, 0.1667
    // Gunde.DotTransfer.Orb.SpawnChanceAtTick.G, 0.15
    std::array<double, 8> feather_chance_per_rends = {
        1.0,  1.0, 0.5,    0.33,
        0.25, 0.2, 0.1667, 0.15 };  // Pad it by an extra 1.0 so can index by active_targets clamped to max array size.
    // Gunde.DotTransfer.Orb.SpawnInnerRadius, 100.0
    // Gunde.DotTransfer.Orb.SpawnOuterRadius, 300.0
    // Gunde.DotTransfer.MaxStacks, 150.0
    int owed_in_blood_max_stacks = 150;

    // Gunde.SingleTargetPull.Cooldown, 30.0                                       ; BUTCHER'S HOOK
    // Gunde.SingleTargetPull.MaxRange, 3000.0
    // Gunde.SingleTargetPull.MaxPullHeight, 1000.0                                ; How high in the air gunde can be
    // while pulling an enemy towards a spot on the ground below him. (enemies never get pulled up into the air.)
    // Gunde.SingleTargetPull.PullDelay, 0.25
    // Gunde.SingleTargetPull.PullDuration, 0.2
    // Gunde.SingleTargetPull.StopDistance, 150.0                                  ; How far from gunde the enemy will
    // end up at. Gunde.SingleTargetPull.Damage.StrengthCoefficient, 0.37                     ;WAS 0.69
    // Gunde.SingleTargetPull.Talent.MultiTargets.Radius, 800.0
    // Gunde.SingleTargetPull.Talent.MultiTargets.AdditionalTargets, 3.0
    // Gunde.SingleTargetPull.Talent.MultiTargets.AdditionalCooldown, 15.0

    // Gunde.JumpToLocation.Cooldown, 20.0                                         ; WARBOUND
    // Gunde.JumpToLocation.NumCharges, 2.0
    // Gunde.JumpToLocation.MaxRangeHorizontal, 2000.0
    // Gunde.JumpToLocation.MaxRangeUp, 1000.0
    // Gunde.JumpToLocation.MaxRangeDown, 5000.0
    // Gunde.JumpToLocation.TargetingRadius, 50.0 ; purly visual right now
    // Gunde.JumpToLocation.JumpDuration, 0.3

    // Gunde.InstantAoeWithBuff.VisualHitDelay, 0.2                               ; BLOOD ARC
    // Gunde.InstantAoeWithBuff.AoeRadius, 700.0
    // Gunde.InstantAoeWithBuff.ConePieHeight, 500.0                               ;WAS 200
    // Gunde.InstantAoeWithBuff.ConePieMinRadius, 0.0
    // Gunde.InstantAoeWithBuff.ConePieAngleWidth, 120.0

    // Gunde.InstantAoeWithBuff.DamageStrengthCoefficient, 1.212                    ; WAS 2.0
    double blood_arc_coeff = 2.7092;
    // Gunde.InstantAoeWithBuff.DamageScalingTargetCountThreshold, 5.0
    double blood_arc_target_threshold = 8.0;
    // Gunde.InstantAoeWithBuff.Cooldown, 9.0
    timespan_t blood_arc_cooldown = 9_s;

    // Gunde.DurationalAoeDotAndBuff.Radius, 1000.0								;
    // BLOODBOUND SPIRIT Gunde.DurationalAoeDotAndBuff.Duration, 6.15 ;was 6.15
    timespan_t bloodbound_spirit_duration = 6.15_s;
    // Gunde.DurationalAoeDotAndBuff.Period, 1.5
    timespan_t bloodbound_spirit_period = 1.5_s;
    // Gunde.DurationalAoeDotAndBuff.StrengthCoefficient, 4.653                      ; WAS 4.95
    double bloodbound_spirit_coeff = 4.653;
    // Gunde.DurationalAoeDotAndBuff.DamageScalingTargetCountThreshold, 3.0		;was 8.0
    double bloodbound_spirit_falloff = 1.0;
    // Gunde.DurationalAoeDotAndBuff.CastTime, 1.0
    timespan_t bloodbound_spirit_cast_time = 1_s;
    // Gunde.DurationalAoeDotAndBuff.SelfBuff.Duration, 20.0
    timespan_t bloodbound_spirit_buff_duration = 20_s;
    // Gunde.DurationalAoeDotAndBuff.SelfBuff.DamageMultiplier, 1.15               ; WAS 1.25
    double bloodbound_spirit_buff_amp = 1.15;

    // Gunde.DashAttack.StrengthCoefficient, 0.2
    // ; WARBOUND NEW WAS 0.5 Gunde.DashAttack.DamageRadius, 250.0 Gunde.DashAttack.MaxCharges, 2.0
    // Gunde.DashAttack.MaxDistance, 1000.0
    // Gunde.DashAttack.InAirExitSpeed, 1000.0
    // Gunde.DashAttack.Speed, 2500
    // Gunde.DashAttack.CooldownDuration, 8.0
    // Gunde.DashAttack.Talent.DamageMultiplier, 1.15

    // Gunde.Talent.AbilityToBuffChance.ProcChance, 0.4
    // Gunde.Talent.AbilityToBuffChance.StrengthMultiplier, 1.06
    // Gunde.Talent.AbilityToBuffChance.Duration, 6.0

  } spell_const;

  struct talents_t
  {
    // Gunde.HeavyMeleeDotBoost.Talent.IncreasedDotDamage.Duration, 10.0
    // Gunde.HeavyMeleeDotBoost.Talent.IncreasedDotDamage.DamageMultiplier, 1.12
    // Gunde.HeavyMeleeDotBoost.Talent.IncreasedCritAndNoneCritDamage.CritDamageMultiplier, 1.5
    // Gunde.HeavyMeleeDotBoost.Talent.IncreasedCritAndNoneCritDamage.NoneCritDamageMultiplier, 1.1
    // Gunde.HeavyMeleeDotBoost.Talent.ReducedCooldown.ReductionInSeconds, 1.0
    // Gunde.HeavyMeleeDotBoost.Talent.OrbDropper.NumOfOrbs, 8.0
    // Gunde.HeavyMeleeDotBoost.Talent.ReducedCooldownPerOrb.ReductionInSeconds, 0.4
    timespan_t slayers_grin_cdr = 0.4_s;

    // Talent Deep Rend
    // Gunde.PerTargetAccumulatedBleed.Talent.ReducedTickSpeed.PeriodFrequencyIncrease
    double deep_rend_tick_speed_increase = 1.1;
    // Gunde.PerTargetAccumulatedBleed.Talent.IncreasedDropAmount.DropChance
    double deep_rend_proc_chance_st = 0.2;
    // Gunde.PerTargetAccumulatedBleed.Talent.IncreasedDropAmount.NewAmount
    int deep_rend_proc_feathers = 2;

    // Gunde.TargetedAoeProjectile.Talent.Empowered.ProcChance, 0.05
    double grim_harvest_chance = 0.06;
    // Gunde.TargetedAoeProjectile.Talent.Empowered.DamageMultiplier, 1.40 ;WAS 2.0
    double grim_harvest_cc = 1.0;
    // Gunde.TargetedAoeProjectile.Talent.Empowered.Duration, 8.0
    timespan_t grim_harvest_buff_duration = 8_s;
    // Gunde.TargetedAoeProjectile.Talent.DamageCooldownReduction.DamageMultiplier, 1.25
    double carnage_damage_multiplier = 1.25;
    // Gunde.TargetedAoeProjectile.Talent.DamageCooldownReduction.CooldownReductionInSecondsPerHit, 1.0
    timespan_t carnage_cdr_per_hit = 1_s;

    // Gunde.InstantAoeWithBuff.Talent.AdditionalStack.MaxStacks, 2.0
    //int superior_serration_blood_arcs = 2
    // Gunde.InstantAoeWithBuff.Talent.AddedTransfer.DotTransferBonus, 0.10	
    double superior_serration_rend_bonus = 0.1;

    // Gunde.HeavyMeleeDotBased.Talent.IncreasedCritChance.AdditionalCritChance, 0.3         ;WAS 0.4
    double oathshatter_additional_crit = 0.3;
    // Gunde.HeavyMeleeDotBased.Talent.CritToAoe.Radius, 500.0
    // Gunde.HeavyMeleeDotBased.Talent.CritToAoe.AdditionalTargets, 4.0	; OLD
    // Gunde.HeavyMeleeDotBased.Talent.CritToAoe.TargetThresholdScaling, 8.0
    double oathshatter_target_threshold = 5.0;
    // Gunde.HeavyMeleeDotBased.Talent.CritToAoe.AoeDamageScaler, 0.25
    double oathshatter_aoe_multiplier = 0.5;
    // Gunde.HeavyMeleeDotBased.Talent.LowHealthTarget.AddedCriticalStrikeChance, 1.00
    double darkening_hearts_execute_cc = 1.0;
    double darkening_hearts_damage_mul = 1.3;
    // Gunde.HeavyMeleeDotBased.Talent.CooldownAcceleration.AdditionalCooldownRecovery, 0.5
    double bloodbath_cooldown_recovery = 2.0;
    // Gunde.HeavyMeleeDotBased.Talent.CooldownAcceleration.Duration, 3.0
    timespan_t bloodbath_duration = 3_s;

    double crimson_strikes_inc = 0.15;

    
    // Gunde.ExtraDotDamageBuff.Talent.Haste.AdditionalHaste, 0.25	;old
    // Gunde.ExtraDotDamageBuff.Talent.AddedBonus.AddedTransfer, 0.25
    double frenzied_reign_extra_transfer = 0.25;

    
    // Gunde.InstantAoeWithBuff.Talent.ChanceCooldownReset.Chance, 0.25            ; WAS 0.15
    double deaths_arc_chance = 0.3;
    // Gunde.InstantAoeWithBuff.Talent.Empowered.AddedCriticalStrikeChance, 1.00
    double deaths_arc_added_cc = 1.0;
    // Gunde.InstantAoeWithBuff.Talent.Empowered.Duration, 12.0
    timespan_t deaths_arc_duration = 12_s;

    // Gunde.Talent.AbilityToBuffChance.ProcChance, 0.4
    double ancestral_instinct_chance = 0.4;
    // Gunde.Talent.AbilityToBuffChance.StrengthMultiplier, 1.06
    double ancestral_instinct_multiplier = 1.06;
    // Gunde.Talent.AbilityToBuffChance.Duration, 6.0
    timespan_t ancestral_instinct_duration = 6_s;

    // Gunde.InstantAoeDot.Talent.StacksToHaste.Duration, 8.0
    timespan_t massacre_duration = 8_s;
    // Gunde.InstantAoeDot.Talent.StacksToHaste.AdditionalHastePerStack, 0.0025
    double massacre_per_stack = 0.0025;
    // Gunde.InstantAoeDot.Talent.StacksToHaste.MaxStacks, 100.0
    int massacre_max_stacks = 100;

    // Gunde.DotTransfer.Talent.IncreasedDamageOnSelf.DamageScalePerStack, 0.0025
    double harvesters_toll_increase = 0.075;
    // Gunde.DotTransfer.Talent.IncreasedDamageOnSelf.MaxStacks, 30.0
    // Gunde.DotTransfer.Talent.IncreasedDamageOnSelf.Duration, 10.0
    timespan_t harvesters_toll_duration = 10_s;

    // Gunde.DotTransfer.Talent.Aoe.Cooldown, 20.0
    // Gunde.DotTransfer.Talent.Aoe.Radius, 1000.0
    // Gunde.DotTransfer.Talent.Aoe.MaxStacks, 80.0
    // Gunde.DotTransfer.Talent.Aoe.StackDuration, 10.0
 
    // Gunde.DotTransfer.Talent.ExplosionOnPickup.StrengthCoefficient, 0.34
    double ravens_precision_coeff = 0.374;
    // Gunde.DotTransfer.Talent.ExplosionOnPickup.Radius, 700.0
    // Gunde.DotTransfer.Talent.ExplosionOnPickup.TargetThresholdScaling, 8.0
    double ravens_precision_falloff = 5;
    // Gunde.DotTransfer.Talent.AoeBasedOnStacks.Radius, 700.0
    // Gunde.DotTransfer.Talent.AoeBasedOnStacks.BaseStrengthCoefficient, 0.20
    double bloodcraze_coeff = 0.2;
    // Gunde.DotTransfer.Talent.AoeBasedOnStacks.DamageIncreasePerStack, 0.90
    double bloodcraze_amp_per_stack = 1.7;
    // Gunde.DotTransfer.Talent.AoeBasedOnStacks.TargetThresholdScaling, 5.0
    double bloodcraze_falloff = 1.0;
    // Gunde.DotTransfer.Talent.AoeBasedOnStacks.Period, 3.0
    timespan_t bloodcraze_period = 3_s;
    // Gunde.DotTransfer.Talent.AoeBasedOnStacks.Duration, 6.15
    timespan_t bloodcraze_duration = 6.15_s;
  } talents;

  struct legendary_t
  {
    bool lego_1 = false;
    // Gunde.TargetedAoeProjectile.Talent.AdditionalTicks.Amount, 2.0
    int lego_1_additional_grim_carve_hits = 2;

    bool lego_2 = false;
    // Gunde.HeavyMeleeDotBased.Talent.Charges.NumOfCharges, 2.0
    int lego_2_charges = 2;
    // Gunde.HeavyMeleeDotBased.Talent.Charges.DoubleStrike.Chance, 0.2
    double lego_2_hit_chance = 0.25;
    // Gunde.HeavyMeleeDotBased.Talent.Charges.DoubleStrike.DelayBetweenStrikes, 0.4
    timespan_t lego_2_delay = 0.4_s;

    bool lego_3                             = false;
    double lego_3_base_blood_arc_amp        = 0.2;
    double lego_3_blood_arc_amp_per_stack   = 0.02;
    double lego_3_base_feathers                = 1;
    double lego_3_feathers                     = 1;
    double lego_3_feathers_stack_divisor       = 3;
    timespan_t carrion_onslaught_period     = 2_s;
    timespan_t carrion_onslaught_duration   = 8_s;
    int carrion_onslaught_max_feathers_per_tick = 10;

    bool lego_4              = false;
    int lego_4_charges       = 2;
    int lego_4_double_hits   = 1;
    double lego_4_hit_chance = 0.1;

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
    return RESOURCE_SPIRIT;
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

  double get_current_rend( player_t* t ) const;
  double get_current_target_rend() const
  {
    return get_current_rend( target );
  }

  void spawn_feathers( int quantity = 1 );

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



struct gunde_action_state_t : public action_state_t
{
private:
  double rend_coefficient;

public:
  gunde_action_state_t( action_t* action, player_t* target )
    : action_state_t( action, target ), rend_coefficient( 0 )
  {
  }

  void initialize() override
  {
    action_state_t::initialize();
    rend_coefficient = 0;
  }

  std::ostringstream& debug_str( std::ostringstream& s ) override
  {
    action_state_t::debug_str( s ) << " rend_coefficient=" << rend_coefficient;
    return s;
  }

  void copy_state( const action_state_t* s )
  {
    action_state_t::copy_state( s );
    const gunde_action_state_t* rs = debug_cast<const gunde_action_state_t*>( s );
    rend_coefficient               = rs->rend_coefficient;
  }

  double get_rend_coefficient() const
  {
    return rend_coefficient;
  }

  void set_rend_coefficient( double rend_coeff )
  {
    rend_coefficient = rend_coeff;
  }
};

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
  double base_rend_applied;
  bool _affected_by_serrated_edge;

  // Init =====================================================================

  gunde_action_t( util::string_view n, gunde_t* p, util::string_view options = {} )
    : ab( n, p, options ),
      base_rend_applied( p->spell_const.rend_accumulation ),
      _affected_by_serrated_edge( true )
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

  static const gunde_action_state_t* cast_state( const action_state_t* st )
  {
    return debug_cast<const gunde_action_state_t*>( st );
  }

  static gunde_action_state_t* cast_state( action_state_t* st )
  {
    return debug_cast<gunde_action_state_t*>( st );
  }

  action_state_t* new_state() override
  {
    return new gunde_action_state_t( this, ab::target );
  }

  void update_state( action_state_t* state, unsigned flags, result_amount_type rt ) override
  {
    ab::update_state( state, flags, rt );
  }

  void snapshot_state( action_state_t* state, result_amount_type rt ) override
  {
    auto rs = cast_state( state );
    rs->set_rend_coefficient( rend_coefficient() );

    ab::snapshot_state( state, rt );
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

  double composite_persistent_multiplier( const action_state_t* s ) const
  {
    auto mul = ab::composite_persistent_multiplier( s );

    mul *= 1.0 + p()->buffs.crimson_strikes->check_value();

    return mul;
  }

public:
  // Ability triggers
  void trigger_auto_attack( const action_state_t* );
  void trigger_spirit_refund( const action_state_t* );
  void apply_rend( const action_state_t* );

  double rend_coefficient()
  {
    if ( base_rend_applied <= 0.0 )
      return 0.0;

    auto rend_coeff = base_rend_applied + p()->buffs.reign_in_blood->check_value();

    if ( _affected_by_serrated_edge )
      rend_coeff += p()->buffs.serrated_edge->check_value();

    return rend_coeff;
  }

  void roll_grim_harvest()
  {
    if ( p()->talents_enabled( gunde_t::GRIM_HARVEST ) )
    {
      if ( p()->rng_objects.grim_harvest->trigger() )
      {
        p()->cooldowns.grim_carve->reset( true, 1 );
        p()->buffs.grim_harvest->trigger();
      }
    }
  }

  void execute() override
  {
    ab::execute();

    if ( ab::hit_any_target && !ab::background )
    {
      trigger_auto_attack( ab::execute_state );

      if ( rend_coefficient() > 0 )
      {
        auto spirit_refund = p()->rng().roll( p()->cache.mastery_value() );

        if ( !spirit_refund && p()->buffs.murder_of_crows->check() )
        {
          spirit_refund = true;
          p()->buffs.murder_of_crows->decrement();
        }

        if ( spirit_refund )
        {
          trigger_spirit_refund( ab::execute_state );
        }

        if ( _affected_by_serrated_edge )
        {
          p()->buffs.serrated_edge->decrement();
          p()->buffs.crimson_strikes->decrement();
        }
      }
    }
  }
};

// ==========================================================================
// Rogue Attack Classes
// ==========================================================================

struct gunde_heal_t : public gunde_action_t<heal_t>
{
protected:
  using base_t = gunde_heal_t;

private:
  using ab = gunde_action_t<heal_t>;

public:
  gunde_heal_t( util::string_view n, gunde_t* p, util::string_view o = {} ) : ab( n, p, o )
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
  using base_t = gunde_spell_t;

private:
  using ab = gunde_action_t<spell_t>;

public:
  gunde_spell_t( util::string_view n, gunde_t* p, util::string_view o = {} ) : ab( n, p, o )
  {
    school = SCHOOL_MAGIC;
  }
};

struct gunde_attack_t : public gunde_action_t<melee_attack_t>
{
protected:
  using base_t = gunde_attack_t;

private:
    using ab = gunde_action_t<melee_attack_t>;

public:
    gunde_attack_t( util::string_view n, gunde_t* p, util::string_view o = {} ) : ab( n, p, o )
  {
    special = true;
    school  = SCHOOL_PHYSICAL;
  }

  void impact( action_state_t* s ) override
  {
    ab::impact( s );

    if ( result_is_hit( s->result ) )
    {
      apply_rend( s );
    }
  }
};

struct gunde_absorb_t : public gunde_action_t<absorb_t>
{
protected:
  using base_t = gunde_absorb_t;

private:
    using ab = gunde_action_t<absorb_t>;

public:
    gunde_absorb_t( util::string_view n, gunde_t* p, util::string_view o = {} ) : ab( n, p, o )
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

struct melee_t : public attack_t
{
  bool first;
  bool canceled;
  timespan_t prev_scheduled_time;

  melee_t( const char* name, const char* reporting_name, gunde_t* p )
    : attack_t( name, p ), first( true ), canceled( false ), prev_scheduled_time( timespan_t::zero() )
  {
    background = repeating = may_glance = may_crit = true;
    may_miss = may_parry      = true;
    allow_class_ability_procs = not_a_proc = true;
    special                                = false;

    school             = SCHOOL_PHYSICAL;
    trigger_gcd        = timespan_t::zero();
    name_str_reporting = reporting_name;

    attack_power_mod.direct = p->spell_const.auto_attack_coeff;
  }

  void reset() override
  {
    attack_t::reset();
    first               = true;
    canceled            = false;
    prev_scheduled_time = timespan_t::zero();
  }

  timespan_t execute_time() const override
  {
    timespan_t t = attack_t::execute_time();

    if ( first )
    {
      return timespan_t::zero();
    }

    // If we cancel the swing timer mid-fight, use the previous swing timer
    if ( canceled )
    {
      return std::min( t, std::max( prev_scheduled_time - player->sim->current_time(), timespan_t::zero() ) );
    }

    return t;
  }

  void schedule_execute( action_state_t* state ) override
  {
    attack_t::schedule_execute( state );

    if ( first )
    {
      first = false;
      player->sim->print_log( "{} schedules AA start {} with {} swing timer", *player, *this, time_to_execute );
    }

    if ( canceled )
    {
      canceled            = false;
      prev_scheduled_time = timespan_t::zero();
      player->sim->print_log( "{} schedules AA restart {} with {} swing timer remaining", *player, *this,
                              time_to_execute );
    }
  }
};

struct auto_melee_attack_t : public action_t
{
  auto_melee_attack_t( gunde_t* p, util::string_view options_str ) : action_t( ACTION_OTHER, "auto_attack_hit", p )
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

struct rend_t : public gunde_attack_t
{
  double to_tick_multiplier;
  size_t rend_ticks;
  rend_t( gunde_t* p ) : gunde_attack_t( "rend", p ), to_tick_multiplier( 0 ), rend_ticks( 10 )
  {
    id = 2;

    name_str_reporting = "Rend";

    tick_may_crit = false;
    may_crit      = false;

    dot_duration           = p->spell_const.rend_duration;
    dot_behavior           = DOT_REFRESH_DURATION;
    base_tick_time         = p->spell_const.rend_period;
    hasted_ticks           = false;
    dot_allow_partial_tick = true;

    to_tick_multiplier = base_tick_time / dot_duration;

    if ( p->talents_enabled( gunde_t::DEEP_REND ) )
    {
      base_tick_time /= p->talents.deep_rend_tick_speed_increase;
      // dot_duration /= p->talents.deep_rend_tick_speed_increase;
    }

    rend_ticks = as<size_t>( dot_duration / base_tick_time );
  }

  void init_finished() override
  {
    gunde_attack_t::init_finished();
    snapshot_flags = 0;
    update_flags   = 0;
  }

  double base_ta( const action_state_t* s ) const override
  {
    auto& rend_obj = p()->get_target_data( s->target )->rend_tracker;

    return rend_obj.tick_buckets[ rend_obj.current_tick ];
  }

  double last_tick_factor( const dot_t* d, timespan_t time_to_tick, timespan_t duration ) const override
  {
    return 1.0;
  }

  void impact( action_state_t* s ) override
  {
    dot_t* dot = get_dot( s->target );

    auto& rend_obj = p()->get_target_data( s->target )->rend_tracker;

    if ( !dot->is_ticking() )
    {
      rend_obj.current_tick = 0;
      range::fill( rend_obj.tick_buckets, 0 );
    }

    if ( s->result_amount > 0 )
    {
      auto add_amount = std::round( s->result_amount * to_tick_multiplier );

      for ( size_t i = 0; i < rend_ticks; i++ )
        rend_obj.tick_buckets[ i ] += add_amount;
    }

    trigger_dot( s );

    if ( !dual )
      stats->add_execute( timespan_t::zero(), s->target );
  }

  void tick( dot_t* d ) override
  {
    gunde_attack_t::tick( d );

    auto& rend_obj = p()->get_target_data( d->state->target )->rend_tracker;

    rend_obj.tick_buckets[ rend_obj.current_tick++ ] = 0;
    if ( rend_obj.current_tick >= rend_ticks )
      rend_obj.current_tick = 0;

    auto active_dots =
        std::min<size_t>( p()->get_active_dots( d ), p()->spell_const.feather_chance_per_rends.size() - 1 );
    auto proc_chance = p()->spell_const.feather_chance_per_rends[ active_dots ];

    sim->print_debug( "{} has {} active rends. {} rend cap, this results in {} for variable. Chance to proc {}. ", *p(),
                      p()->get_active_dots( d ), p()->spell_const.feather_chance_per_rends.size(), active_dots,
                      proc_chance );

    if ( p()->rng().roll( proc_chance ) )
    {
      p()->spawn_feathers( 1 );

      //if ( p()->talents_enabled( gunde_t::DEEP_REND ) && p()->rng_objects.deep_rend->trigger() )
      //{
      //  p()->spawn_feathers( p()->talents.deep_rend_proc_feathers );
      //}
    }

    //if ( p()->talents_enabled( gunde_t::DEEP_REND ) && active_dots == 1 && p()->rng_objects.deep_rend->trigger() )
    //{
    //  p()->spawn_feathers( p()->talents.deep_rend_proc_feathers - 1 );
    //}
  }
};

struct slaughter_dot_t : public gunde_attack_t
{
  double to_tick_multiplier;
  size_t ticks;
  slaughter_dot_t( gunde_t* p ) : gunde_attack_t( "slaughter", p ), to_tick_multiplier( 0 ), ticks( 10 )
  {
    id = 3;

    name_str_reporting = "Slaughter (DoT)";

    tick_may_crit = false;
    may_crit      = false;

    dot_duration           = p->spell_const.slaughter_duration;
    dot_behavior           = DOT_REFRESH_DURATION;
    base_tick_time         = p->spell_const.slaughter_period;
    hasted_ticks           = false;
    dot_allow_partial_tick = true;

    to_tick_multiplier = base_tick_time / dot_duration;
    ticks              = as<size_t>( dot_duration / base_tick_time );
  }

  void init_finished() override
  {
    gunde_attack_t::init_finished();
    snapshot_flags = 0;
    update_flags   = 0;
  }

  double base_ta( const action_state_t* s ) const override
  {
    auto& rend_obj = p()->get_target_data( s->target )->slaughter_tracker;

    return rend_obj.tick_buckets[ rend_obj.current_tick ];
  }

  double last_tick_factor( const dot_t* d, timespan_t time_to_tick, timespan_t duration ) const override
  {
    return 1.0;
  }

  void impact( action_state_t* s ) override
  {
    dot_t* dot = get_dot( s->target );

    auto& rend_obj = p()->get_target_data( s->target )->slaughter_tracker;

    if ( !dot->is_ticking() )
    {
      rend_obj.current_tick = 0;
      range::fill( rend_obj.tick_buckets, 0 );
    }

    if ( s->result_amount > 0 )
    {
      auto add_amount = std::round( s->result_amount * to_tick_multiplier );

      for ( size_t i = 0; i < ticks; i++ )
        rend_obj.tick_buckets[ i ] += add_amount;
    }

    trigger_dot( s );

    if ( !dual )
      stats->add_execute( timespan_t::zero(), s->target );
  }

  void tick( dot_t* d ) override
  {
    gunde_attack_t::tick( d );

    auto& rend_obj                                   = p()->get_target_data( d->state->target )->slaughter_tracker;
    rend_obj.tick_buckets[ rend_obj.current_tick++ ] = 0;
    if ( rend_obj.current_tick >= ticks )
      rend_obj.current_tick = 0;
  }
};

struct double_strike_t : public gunde_attack_t
{
  struct double_strike_hit_t : public gunde_attack_t
  {
    double_strike_hit_t( util::string_view name, gunde_t* p ) : gunde_attack_t( name, p )
    {
      background              = true;
      id                      = 4;
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
    id = 4;

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

  void init() override
  {
    snapshot_flags |= STATE_MUL_DA | STATE_TGT_MUL_DA | STATE_MUL_PERSISTENT | STATE_VERSATILITY | STATE_AP;
    base_t::init();
  }

  void execute() override
  {
    // Manual Finesse N (Vehement) handling due to dual child.
    auto was_finesse_n_active = fs_p()->fs_buffs.finesse_n->at_max_stacks();

    gunde_attack_t::execute();
    
    for ( auto& hit : hits )
    {
      hit->set_target( target );
      action_state_t* damage_state = hit->get_state( execute_state );

      hit->finesse_n_active = was_finesse_n_active;

      if ( was_finesse_n_active )
      {
        fs_p()->fs_buffs.finesse_n->expire();
        was_finesse_n_active = false;
      }

      hit->schedule_execute( damage_state );
    }
  }
};

struct grim_carve_t : public gunde_attack_t
{
  struct grim_carve_hit_t : public gunde_attack_t
  {
    grim_carve_hit_t( util::string_view name, gunde_t* p ) : gunde_attack_t( name, p )
    {
      background = true;
      id                = 5;

      attack_power_mod.direct = p->spell_const.grim_carve_coeff;

      aoe                 = -1;
      reduced_aoe_targets = p->spell_const.grim_carve_falloff;

      name_str_reporting = "Grim Carve Hit";

      ability_flags |= ability_type_e::ABILITY_POWER;

      if ( p->talents_enabled( gunde_t::CARNAGE ) )
      {
        attack_power_mod.direct *= p->talents.carnage_damage_multiplier;
      }
    }

    double composite_crit_chance() const override
    {
      auto cc = base_t::composite_crit_chance();

      cc += p()->buffs.grim_harvest->check_value();

      return cc;
    }

    double composite_da_multiplier( const action_state_t* s ) const override
    {
      auto m = base_t::composite_da_multiplier( s );

      m *= 1.0 + p()->buffs.bloodbound_spirit->check_value();

      return m;
    }

    void execute() override
    {
      base_t::execute();

      if ( p()->talents_enabled( gunde_t::CARNAGE ) )
      {
        auto cooldowns = { p()->cooldowns.reavers_edge, p()->cooldowns.blood_arc, p()->cooldowns.heart_splitter,
                           p()->cooldowns.rupture };

        for ( auto& cd : cooldowns )
        {
          cd->adjust( -p()->talents.carnage_cdr_per_hit, false );
        }
      }
    }
  };

  grim_carve_hit_t* hit;
  int num_hits;
  double period_multiplier;
  timespan_t period;

  grim_carve_t( util::string_view name, gunde_t* p, util::string_view options_str = {} )
    : gunde_attack_t( name, p, options_str ),
      num_hits(
          static_cast<int>( std::floor( p->spell_const.grim_carve_duration / p->spell_const.grim_carve_period ) ) ),
      period_multiplier( 1.0 ),
      period( p->spell_const.grim_carve_period )
  {
    id                 = 5;
    name_str_reporting = "Grim Carve";

    ability_flags |= ability_type_e::ABILITY_POWER;

    cooldown->duration = p->spell_const.grim_carve_cooldown;
    cooldown->charges  = 1;
    cooldown->hasted   = true;

    hit = new grim_carve_hit_t( "grim_carve_hit", p );
    add_child( hit );

    if ( p->legendary.lego_1 )
    {
      auto old_hits = num_hits;
      num_hits += p->legendary.lego_1_additional_grim_carve_hits;

      period_multiplier *= as<double>( old_hits ) / num_hits;
    }
  }

  void init() override
  {
    snapshot_flags |= STATE_MUL_DA | STATE_TGT_MUL_DA | STATE_MUL_PERSISTENT | STATE_VERSATILITY | STATE_AP;
    base_t::init();
  }

  void execute() override
  {
    gunde_attack_t::execute();

    if ( p()->talents_enabled( gunde_t::ANCESTRAL_INSTINCT ) )
    {
      if ( p()->rng_objects.ancestral_instinct->trigger() )
      {
        p()->buffs.ancestral_instinct->trigger();
      }
    }

    if ( p()->talents_enabled( gunde_t::BLOODBATH ) )
    {
      p()->buffs.bloodbath->trigger();
    }

    const timespan_t tick_period = p()->spell_const.grim_carve_period;

    for ( int i = 0; i <= num_hits; ++i )
    {
      auto* damage_state = hit->get_state( execute_state );

      hit->snapshot_state( damage_state, result_amount_type::DMG_DIRECT );
      damage_state->persistent_multiplier = execute_state->persistent_multiplier;

      hit->set_target( target );

      make_event(
          *sim,
          tick_period * i * p()->cache.attack_haste() * period_multiplier + p()->spell_const.grim_carve_initial_delay,
          [ this, damage_state ]() { hit->schedule_execute( damage_state ); } );
    }

    p()->buffs.grim_harvest->expire();
  }
};

struct reavers_edge_t : public gunde_attack_t
{
  reavers_edge_t( util::string_view name, gunde_t* p, util::string_view options_str = {} )
    : gunde_attack_t( name, p, options_str )
  {
    id                 = 6;
    name_str_reporting = "Reavers Edge";

    ability_flags |= ability_type_e::ABILITY_BASIC;

    cooldown->duration = p->spell_const.reavers_edge_cooldown;
    cooldown->charges  = 1;
    cooldown->hasted   = true;

    attack_power_mod.direct = p->spell_const.reavers_edge_coeff;

    aoe                 = -1;
    reduced_aoe_targets = p->spell_const.reavers_edge_target_threshold;
  }

  void execute() override
  {
    base_t::execute();

    roll_grim_harvest();

    if ( p()->talents_enabled( gunde_t::ANCESTRAL_INSTINCT ) )
    {
      if ( p()->rng_objects.ancestral_instinct->trigger() )
      {
        p()->buffs.ancestral_instinct->trigger();
      }
    }
  }
};

struct blood_arc_t : public gunde_attack_t
{
  blood_arc_t( util::string_view name, gunde_t* p, util::string_view options_str = {} )
    : gunde_attack_t( name, p, options_str )
  {
    id                 = 7;
    name_str_reporting = "Blood Arc";

    ability_flags |= ability_type_e::ABILITY_CORE;

    cooldown->duration = p->spell_const.blood_arc_cooldown;
    cooldown->charges  = 1;
    cooldown->hasted   = true;

    attack_power_mod.direct = p->spell_const.blood_arc_coeff;

    aoe                 = -1;
    reduced_aoe_targets = p->spell_const.blood_arc_target_threshold;
  }

  double composite_crit_chance() const override
  {
    auto cc = base_t::composite_crit_chance();

    cc += p()->buffs.deaths_arc->check_value();

    return cc;
  }

  double composite_da_multiplier( const action_state_t* s ) const override
  {
    auto da = base_t::composite_da_multiplier( s );

    if ( p()->buffs.carrion_onslaught->check() )
    {
      da *= 1.0 + p()->legendary.lego_3_base_blood_arc_amp +
            p()->legendary.lego_3_blood_arc_amp_per_stack * p()->buffs.carrion_onslaught->check();
    }

    return da;
  }

  void execute() override
  {
    gunde_attack_t::execute();

    roll_grim_harvest();

    p()->buffs.serrated_edge->trigger();

    if ( p()->buffs.carrion_onslaught->check() )
    {
      p()->buffs.carrion_onslaught_feathers->expire();

      auto feathers = as<int>( std::round( std::max(
          p()->legendary.lego_3_base_feathers, p()->legendary.lego_3_feathers * p()->buffs.carrion_onslaught->check() /
                                                   p()->legendary.lego_3_feathers_stack_divisor ) ) );

      p()->buffs.carrion_onslaught_feathers->trigger( feathers );
      p()->buffs.carrion_onslaught->expire();
    }

    if ( p()->talents_enabled( gunde_t::CRIMSON_STRIKES ) )
    {
      p()->buffs.crimson_strikes->trigger();
    }

    p()->buffs.deaths_arc->decrement();

    if ( p()->talents_enabled( gunde_t::DEATHS_ARC ) )
    {
      if ( p()->rng_objects.deaths_arc->trigger() )
      {
        cooldown->reset( true, 1 );
        p()->buffs.deaths_arc->trigger();
      }
    }
  }
};

struct rupture_t : public gunde_attack_t
{
  rupture_t( util::string_view name, gunde_t* p, util::string_view options_str = {} )
    : gunde_attack_t( name, p, options_str )
  {
    id                 = 8;
    name_str_reporting = "Rupture";

    ability_flags |= ability_type_e::ABILITY_POWER;

    cooldown->duration = p->spell_const.rupture_cooldown;
    cooldown->charges  = 1;
    cooldown->hasted   = false;

    attack_power_mod.direct = p->spell_const.rupture_coeff;
  }

  void impact( action_state_t* s ) override
  {
    base_t::impact( s );

    p()->get_target_data( s->target )->debuffs.open_wounds->trigger();
  }

  void execute() override
  {
    if ( p()->talents_enabled( gunde_t::HARVESTERS_TOLL ) )
    {
      p()->buffs.harvesters_toll->trigger();
    }

    base_t::execute();

    roll_grim_harvest();

    if ( p()->talents_enabled( gunde_t::MURDER_OF_CROWS ) )
    {
      //p()->spawn_feathers( p()->talents.murder_of_crows_feathers );
      p()->buffs.murder_of_crows->trigger( 2 );
    }
  }
};


struct heart_splitter_t : public gunde_attack_t
{
  struct heart_splitter_exsanguinate_t : public gunde_attack_t
  {
    heart_splitter_exsanguinate_t( gunde_t* p, std::string_view parent_name, bool oathshatter = false )
      : gunde_attack_t( std::format( "{}_exsanguinate", parent_name ), p )
    {
      id                 = 9;
      name_str_reporting = "Heart Splitter Exsanguinate";
      base_rend_applied  = 0.0;
      background         = true;

      base_multiplier = p->spell_const.heart_splitter_exsanguinate_coeff;

      if ( oathshatter )
      {
        aoe                 = -1;
        reduced_aoe_targets = p->talents.oathshatter_target_threshold;
        base_aoe_multiplier = p->talents.oathshatter_aoe_multiplier;
        full_amount_targets = 1;
      }

      may_crit = false;
    }

    double composite_da_multiplier( const action_state_t* s ) const override
    {
      return base_multiplier;
    }

    void init_finished() override
    {
      gunde_attack_t::init_finished();

      snapshot_flags = STATE_MUL_SPELL_DA;
    }
  };

  action_t* exsanguinate;
  action_t* exsanguinate_oathshatter;
  heart_splitter_t( util::string_view name, gunde_t* p, util::string_view options_str = {} )
    : gunde_attack_t( name, p, options_str ),
      exsanguinate( nullptr ),
      exsanguinate_oathshatter( nullptr )
  {
    id                 = 9;
    name_str_reporting = "Heart Splitter";

    ability_flags |= ability_type_e::ABILITY_CORE;

    cooldown->duration = p->spell_const.heart_splitter_cooldown;
    cooldown->charges  = 1;
    cooldown->hasted   = true;

    attack_power_mod.direct = p->spell_const.heart_splitter_coeff;

    if ( p->talents_enabled( gunde_t::DARKENING_HEARTS ) )
      attack_power_mod.direct *= p->talents.darkening_hearts_damage_mul;


    base_rend_applied += p->spell_const.heart_splitter_additional_rend;

    exsanguinate = new heart_splitter_exsanguinate_t( p, name, false );
    add_child( exsanguinate );

    if ( p->legendary.lego_2 )
    {
      cooldown->charges = p->legendary.lego_2_charges;
    }

    if ( p->legendary.lego_4 )
    {
      cooldown->charges = p->legendary.lego_4_charges;
    }

    if ( p->talents_enabled( gunde_t::OATHSHATTER ) )
    {
      exsanguinate_oathshatter = new heart_splitter_exsanguinate_t( p, name, true );
      add_child( exsanguinate_oathshatter );
      base_crit += p->talents.oathshatter_additional_crit;
    }
  }

  void init_finished() override
  {
    base_t::init_finished();

    if ( !background )
    {
      add_child( p()->actions.heart_splitter );
    }
  }

  double composite_target_crit_chance( player_t* t ) const override
  {
    auto tcc = base_t::composite_target_crit_chance( t );

    if ( p()->talents_enabled( gunde_t::DARKENING_HEARTS ) )
    {
      if ( t->health_percentage() <= low_health_threshold )
      {
        tcc += p()->talents.darkening_hearts_execute_cc;
      }
    }

    return tcc;
  }

  double composite_da_multiplier( const action_state_t* s ) const override
  {
    auto dam = base_t::composite_da_multiplier( s );

    dam *= 1.0 + p()->buffs.bloodbound_spirit->check_value();

    return dam;
  }

  void execute() override
  {
    base_t::execute();

    if ( !background )
    {
      roll_grim_harvest();

      auto double_hit = p()->legendary.lego_2 && p()->rng_objects.heart_splitter_twice->trigger() ||
                        p()->buffs.heartsplitter_slaughter->check();

      if ( !double_hit && p()->legendary.lego_4 && p()->rng_objects.heart_splitter_twice_2->trigger() )
        double_hit = true;

      if ( double_hit )
      {
        auto action = p()->actions.heart_splitter;

        action->set_target( target );
        action_state_t* damage_state = action->get_state( execute_state );
        damage_state->target         = action->target;

        action->snapshot_state( damage_state, result_amount_type::DMG_DIRECT );
        damage_state->persistent_multiplier = execute_state->persistent_multiplier;
        cast_state( damage_state )->set_rend_coefficient( cast_state( execute_state )->get_rend_coefficient() );

        action->schedule_execute( damage_state );

        p()->buffs.heartsplitter_slaughter->decrement();
      }
    }
  }

  void impact( action_state_t* s ) override
  {
    gunde_attack_t::impact( s );

    if ( s->result_amount > 0 )
    {
      auto exsang_action =
          s->result == RESULT_CRIT && exsanguinate_oathshatter ? exsanguinate_oathshatter : exsanguinate;
      auto current_rend = p()->get_current_rend( s->target );

      if ( current_rend > 0 )
        exsang_action->execute_on_target( s->target, current_rend );
    }
  }
};

struct reign_in_blood_t : public gunde_spell_t
{
  reign_in_blood_t( util::string_view name, gunde_t* p, util::string_view options_str = {} )
    : gunde_spell_t( name, p, options_str )
  {
    id                = 10;
    base_rend_applied = 0.0;

    name_str_reporting = "Reign in Blood";

    trigger_gcd = timespan_t::zero();

    cooldown->duration = p->spell_const.reign_in_blood_cooldown;
    cooldown->hasted   = false;
    cooldown->charges  = 1;

    ability_flags |= ability_type_e::ABILITY_MAJOR;
  }
    
  double recharge_rate_multiplier( const cooldown_t& cd ) const override
  {
    auto rrm = base_t::recharge_rate_multiplier( cd );

    rrm = 1.0 / rrm;

    rrm += p()->buffs.bloodbath->check_value();

    rrm = 1.0 / rrm;

    return rrm;
  }

  void execute() override
  {
    base_t::execute();
    p()->buffs.reign_in_blood->trigger();
  }
};

struct slaughter_t : public gunde_spell_t
{
  slaughter_t( util::string_view name, gunde_t* p, util::string_view options_str = {} )
    : gunde_spell_t( name, p, options_str )
  {
    id                = 11;
    base_rend_applied = 0.0;

    trigger_gcd = timespan_t::zero();

    cooldown->duration = p->spell_const.slaughter_cooldown;
    cooldown->hasted   = false;
    cooldown->charges  = 1;

    aoe = -1;

    ability_flags |= ability_type_e::ABILITY_MAJOR;
  }

  void impact( action_state_t* s ) override
  {
    auto td = p()->get_target_data( s->target );

    auto current_rend = p()->get_current_rend( s->target );
    auto open_wounds  = td->debuffs.open_wounds->check_value();
    td->debuffs.open_wounds->decrement();

    int rend_stacks = static_cast<int>( ( current_rend / p()->cache.strength() ) );

    if ( p()->talent_enabled( gunde_t::MASSACRE ) )
    {
      p()->buffs.massacre->trigger( rend_stacks );
    }

    current_rend *= 1.0 + open_wounds;
    current_rend *= p()->spell_const.slaughter_rend_multiplier;

    for ( auto& bucket : td->rend_tracker.tick_buckets )
    {
      bucket = 0;
    }

    td->rend_tracker.current_tick = 0;
    td->dots.rend->cancel();

    auto action                     = p()->actions.slaughter;
    action_state_t* slaughter_state = action->get_state();
    slaughter_state->result_amount  = current_rend;
    slaughter_state->target         = s->target;
    slaughter_state->result         = RESULT_HIT;
    action->snapshot_state( slaughter_state, result_amount_type::DMG_OVER_TIME );
    action->schedule_travel( slaughter_state );
  }

  bool ready() override
  {
    if ( p()->legendary.lego_4 )
      return false;

    return base_t::ready();
  }
};

struct slaughter_splitter_t : public gunde_spell_t
{
  slaughter_splitter_t( util::string_view name, gunde_t* p, util::string_view options_str = {} )
    : gunde_spell_t( name, p, options_str )
  {
    id                = 11;
    base_rend_applied = 0.0;

    trigger_gcd = timespan_t::zero();

    cooldown->duration = p->spell_const.slaughter_cooldown;
    cooldown->hasted   = false;
    cooldown->charges  = 1;

    aoe = -1;

    ability_flags |= ability_type_e::ABILITY_MAJOR;
  }

  bool ready() override
  {
    if ( !p()->legendary.lego_4 )
      return false;

    return base_t::ready();
  }

  void impact( action_state_t* s ) override
  {
    auto td = p()->get_target_data( s->target );

    auto current_rend = p()->get_current_rend( s->target );

    int rend_stacks = static_cast<int>( ( current_rend / p()->cache.strength() ) );

    if ( p()->talent_enabled( gunde_t::MASSACRE ) )
    {
      p()->buffs.massacre->trigger( rend_stacks );
    }
  }

  void execute()
  {
    base_t::execute();
    p()->buffs.heartsplitter_slaughter->trigger();
  }
};


struct owed_in_blood_t : public gunde_spell_t
{
  owed_in_blood_t( util::string_view name, gunde_t* p, util::string_view options_str = {} )
    : gunde_spell_t( name, p, options_str )
  {
    id                = 12;
    base_rend_applied = 0.0;

    trigger_gcd = timespan_t::zero();

    cooldown->duration = p->spell_const.owed_in_blood_cooldown;
    cooldown->hasted   = false;
    cooldown->charges  = 1;

    aoe = -1;

    
    name_str_reporting = "Owed in Blood";
  }

  bool ready() override
  {
    if ( !p()->buffs.owed_in_blood->check() )
      return false;

    return base_t::ready();
  }
  
  void execute() override
  {
    base_t::execute();

    auto stacks = p()->buffs.owed_in_blood->check();

    if ( p()->legendary.lego_3 )
    {
      // Do not be dumb, this deletes itself
      p()->buffs.carrion_onslaught->expire();

      p()->buffs.carrion_onslaught->trigger( stacks );
    }

    auto rend_damage = stacks * p()->cache.strength();

    auto action = p()->actions.rend;
    action->set_target( target );

    action_state_t* rend_state = action->get_state();
    rend_state->result_amount  = rend_damage;
    rend_state->target         = target;
    rend_state->result         = RESULT_HIT;
    action->snapshot_state( rend_state, result_amount_type::DMG_OVER_TIME );
    action->schedule_travel( rend_state );

    p()->buffs.owed_in_blood->expire();

    if ( p()->talents_enabled( gunde_t::BLOODCRAZE ) )
    {
      p()->buffs.bloodcraze->expire();
      p()->buffs.bloodcraze->trigger( stacks );
    }
  }
};

struct bloodcraze_t : public gunde_spell_t
{
  bloodcraze_t( gunde_t* p ) : gunde_spell_t( "bloodcraze", p )
  {
    id                 = 13;
    name_str_reporting = "Bloodcraze";
    base_rend_applied  = 0.0;

    background = true;

    attack_power_mod.direct = p->talents.bloodcraze_coeff;

    aoe                 = -1;
    reduced_aoe_targets = p->talents.bloodcraze_falloff;
  }

  double composite_da_multiplier( const action_state_t* s ) const override
  {
    auto m = base_t::composite_da_multiplier( s );

    m *= ( 1 + p()->buffs.bloodcraze->check_stack_value() );

    return m;
  }
};

struct ravens_precision_t : public gunde_attack_t
{
  ravens_precision_t( gunde_t* p ) : gunde_attack_t( "ravens_precision", p )
  {
    id                 = 14;
    name_str_reporting = "Ravens Precision";
    base_rend_applied  = 0.0;

    background = true;

    attack_power_mod.direct = p->talents.ravens_precision_coeff;

    aoe                 = -1;
    reduced_aoe_targets = p->talents.ravens_precision_falloff;
    name_str_reporting  = "Ravens Precision";
  }
};

struct bloodbound_spirit_t : public gunde_attack_t
{
  struct bloodbound_spirit_hit_t : public gunde_attack_t
  {
    bloodbound_spirit_hit_t( util::string_view name, gunde_t* p ) : gunde_attack_t( name, p )
    {
      id = 15;

      _affected_by_serrated_edge = false;
      background = true;

      ability_flags |= ability_type_e::ABILITY_SPIRIT;
      attack_power_mod.direct = p->spell_const.bloodbound_spirit_coeff;
      aoe                     = -1;
      reduced_aoe_targets     = p->spell_const.bloodbound_spirit_falloff;

      name_str_reporting = "Bloodbound Spirit (Hit)";
    }
  };

  action_t* damage;
  bloodbound_spirit_t( util::string_view name, gunde_t* p, util::string_view options_str = {} )
    : gunde_attack_t( name, p, options_str ), damage( new bloodbound_spirit_hit_t( "bloodbound_spirit_hit", p ) )
  {
    id                = 15;

    _affected_by_serrated_edge = false;

    base_execute_time = p->spell_const.bloodbound_spirit_cast_time;

    resource_current              = RESOURCE_SPIRIT;
    base_costs[ RESOURCE_SPIRIT ] = 100;
    ability_flags |= ability_type_e::ABILITY_SPIRIT;
    
    name_str_reporting = "Bloodbound Spirit";

    add_child( damage );
  }

  void execute() override
  {
    base_t::execute();

    p()->fs_buffs.spirit_of_heroism->trigger();
    p()->buffs.bloodbound_spirit->trigger();
    p()->used_ultimate();

    make_event<ground_aoe_event_t>( *sim, p(),
                                    ground_aoe_params_t()
                                        .target( execute_state->target )
                                        .pulse_time( p()->spell_const.bloodbound_spirit_period )
                                        .duration( p()->spell_const.bloodbound_spirit_duration )
                                        .action( damage ),
                                    true );
  }
};
}  // namespace actions

gunde_td_t::gunde_td_t( player_t* target, gunde_t* source )
  : fellowship::fs_player_td_t( target, source ), dots(), debuffs()
{
  dots.rend      = target->get_dot( "rend", source );
  dots.slaughter = target->get_dot( "slaughter", source );

  debuffs.open_wounds = make_buff( *this, "open_wounds" )
                            ->set_duration( source->spell_const.open_wounds_duration )
                            ->set_default_value( source->spell_const.open_wounds_modifier - 1 );
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

  m *= 1.0 + buffs.harvesters_toll->check_stack_value();

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
  if ( name == "grim_carve" )
    return new grim_carve_t( name, this, options_str );
  if ( name == "reavers_edge" )
    return new reavers_edge_t( name, this, options_str );
  if ( name == "blood_arc" )
    return new blood_arc_t( name, this, options_str );
  if ( name == "rupture" )
    return new rupture_t( name, this, options_str );
  if ( name == "heart_splitter" )
    return new heart_splitter_t( name, this, options_str );
  if ( name == "reign_in_blood" )
    return new reign_in_blood_t( name, this, options_str );
  if ( name == "slaughter" )
    return new slaughter_t( name, this, options_str );
  if ( name == "slaughter_splitter" )
    return new slaughter_splitter_t( name, this, options_str );
  if ( name == "owed_in_blood" )
    return new owed_in_blood_t( name, this, options_str );
  if ( name == "bloodbound_spirit" )
    return new bloodbound_spirit_t( name, this, options_str );

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
      if ( util::str_compare_ci( split[ 1 ], "lego_1" ) )
      {
        return make_ref_expr( name_str, legendary.lego_1 );
      }
      else if ( util::str_compare_ci( split[ 1 ], "lego_2" ) )
      {
        return make_ref_expr( name_str, legendary.lego_2 );
      }
      else if ( util::str_compare_ci( split[ 1 ], "lego_3" ) )
      {
        return make_ref_expr( name_str, legendary.lego_3 );
      }
      else if ( util::str_compare_ci( split[ 1 ], "lego_4" ) )
      {
        return make_ref_expr( name_str, legendary.lego_4 );
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
  else if ( util::str_compare_ci( split[ 0 ], "rend_on_target" ) )
  {
    return make_fn_expr( name_str, std::bind( std::mem_fn( &gunde_t::get_current_target_rend ), this ) );
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

  gains.spirit_procs = get_gain( "Spirit Procs" );
}

// gunde_t::init_procs ======================================================

void gunde_t::init_procs()
{
  fs_player_t::init_procs();
  procs.feathers = get_proc( "Feathers" );
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

  buffs.murder_of_crows = make_buff<gunde_buff_t>( this, "murder_of_crows" )->set_max_stack( 4 )->set_duration( 12_s );

  buffs.bloodbound_spirit = make_buff<gunde_buff_t>( this, "bloodbound_spirit" )
                                ->set_duration( spell_const.bloodbound_spirit_buff_duration )
                                ->set_default_value( spell_const.bloodbound_spirit_buff_amp - 1 );

  buffs.bloodcraze = make_buff<gunde_buff_t>( this, "bloodcraze" )
                         ->set_duration( talents.bloodcraze_duration )
                         ->set_default_value( talents.bloodcraze_amp_per_stack )
                         ->set_period( talents.bloodcraze_period )
                         ->set_tick_zero( true )
                         ->set_freeze_stacks( true )
                         ->set_max_stack( spell_const.owed_in_blood_max_stacks )
                         ->set_tick_callback( [ this ]( buff_t* buff, int current_tick, timespan_t tick_time ) {
                           actions.bloodcraze->execute();
                         } );

  buffs.bloodbath = make_buff<gunde_buff_t>( this, "bloodbath" )
                        ->set_duration( talents.bloodbath_duration )
                        ->set_default_value( talents.bloodbath_cooldown_recovery )
                        ->add_stack_change_callback(
                            [ this ]( auto, auto, auto ) { cooldowns.reign_in_blood->adjust_recharge_multiplier(); } );

  buffs.harvesters_toll = make_buff<gunde_buff_t>( this, "harvesters_toll" )
                              ->set_default_value( talents.harvesters_toll_increase )
                              ->set_duration( talents.harvesters_toll_duration )
                              ->add_invalidate( CACHE_PLAYER_DAMAGE_MULTIPLIER );

  buffs.owed_in_blood = make_buff<gunde_buff_t>( this, "owed_in_blood" )
                            ->set_duration( spell_const.owed_in_blood_duration )
                            ->set_max_stack( spell_const.owed_in_blood_max_stacks )
                            ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

  buffs.massacre = make_buff<gunde_buff_t>( this, "massacre" )
                       ->set_duration( talents.massacre_duration )
                       ->set_default_value( talents.massacre_per_stack )
                       ->set_max_stack(talents.massacre_max_stacks )
                       ->set_pct_buff_type( STAT_PCT_BUFF_MASTERY );

  buffs.grim_harvest = make_buff<gunde_buff_t>( this, "grim_harvest" )
                           ->set_duration( talents.grim_harvest_buff_duration )
                           ->set_default_value( talents.grim_harvest_cc );

  buffs.ancestral_instinct = make_buff<gunde_buff_t>( this, "ancestral_instinct" )
                                ->set_duration( talents.ancestral_instinct_duration )
                                ->set_default_value( talents.ancestral_instinct_multiplier - 1 )
                                ->set_pct_buff_type( STAT_PCT_BUFF_STRENGTH );

  buffs.deaths_arc = make_buff<gunde_buff_t>( this, "deaths_arc" )
                         ->set_duration( talents.deaths_arc_duration )
                         ->set_default_value( talents.deaths_arc_added_cc );

  buffs.reign_in_blood = make_buff<gunde_buff_t>( this, "reign_in_blood" )
                             ->set_default_value( spell_const.reign_in_blood_additional_rend )
                             ->set_duration( spell_const.reign_in_blood_duration );

  if ( talents_enabled( gunde_t::FRENZIED_REIGN ) )
  {
    buffs.reign_in_blood->default_value += talents.frenzied_reign_extra_transfer;
  }

  buffs.serrated_edge = make_buff<gunde_buff_t>( this, "serrated_edge" )
                            ->set_duration( spell_const.blood_arc_buff_duration )
                            ->set_max_stack( spell_const.blood_arc_buff_max_stacks )
                            ->set_default_value( spell_const.blood_arc_buff_additional_rend );

  if ( talents_enabled( gunde_t::SUPERIOR_SERRATION ) )
  {
    //buffs.serrated_edge->set_max_stack( talents.superior_serration_blood_arcs );
    buffs.serrated_edge->default_value += talents.superior_serration_rend_bonus;
  }

  buffs.serrated_edge->set_initial_stack( buffs.serrated_edge->max_stack() );

  buffs.crimson_strikes = make_buff<gunde_buff_t>( this, "crimson_strikes" )
                              ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT )
                              ->set_default_value( talents.crimson_strikes_inc )
                              ->set_max_stack( 1 );

  buffs.heartsplitter_slaughter = make_buff<gunde_buff_t>( this, "heartsplitter_slaughter" )
                                      ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT )
                                      ->set_max_stack( legendary.lego_4_double_hits )
                                      ->set_initial_stack( legendary.lego_4_double_hits );

  buffs.carrion_onslaught = make_buff<gunde_buff_t>( this, "carrion_onslaught" )
                                ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT )
                                ->set_max_stack( spell_const.owed_in_blood_max_stacks )
                                ->set_default_value( legendary.lego_3_base_feathers );

  buffs.carrion_onslaught_feathers = make_buff<gunde_buff_t>( this, "carrion_onslaught_feathers" )
                                         ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT )
                                         ->set_max_stack( spell_const.owed_in_blood_max_stacks )
                                         ->set_period( legendary.carrion_onslaught_period )
                                         ->set_tick_behavior( buff_tick_behavior::CLIP )
                                         ->set_tick_zero( true )
                                         ->set_freeze_stacks( true )
                                         ->set_reverse_stack_count( legendary.carrion_onslaught_max_feathers_per_tick )
                                         ->set_reverse( true )
                                         ->set_tick_callback( [ this ]( buff_t* buff, int, timespan_t ) {
                                           auto feathers =
                                               std::min( buff->reverse_stack_reduction, buff->current_stack );
                                           spawn_feathers( feathers );
                                           buff->decrement( buff->reverse_stack_reduction );
                                         } );
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

  add_option( opt_bool( "legendary.lego_1", legendary.lego_1 ) );
  add_option( opt_bool( "legendary.lego_2", legendary.lego_2 ) );
  add_option( opt_bool( "legendary.lego_3", legendary.lego_3 ) );
  add_option( opt_bool( "legendary.lego_4", legendary.lego_4 ) );
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

  actions.rend                               = new actions::rend_t( this );
  actions.slaughter                          = new actions::slaughter_dot_t( this );
  actions.bloodcraze                         = new actions::bloodcraze_t( this );
  actions.ravens_precision                   = new actions::ravens_precision_t( this );
  actions.heart_splitter                     = new actions::heart_splitter_t( "heart_splitter_repeat", this );
  actions.heart_splitter->name_str_reporting = "Heart Splitter (Legendary)";
  actions.heart_splitter->background         = true;
  actions.heart_splitter->cooldown->duration = 0_s;
}

void gunde_t::init_rng()
{
  fs_player_t::init_rng();

  rng_objects.deaths_arc   = get_accumulated_rng( "deaths_arc", rng::CfromP( talents.deaths_arc_chance ) );
  rng_objects.grim_harvest = get_accumulated_rng( "grim_harvest", rng::CfromP( talents.grim_harvest_chance ) );
  rng_objects.ancestral_instinct =
      get_accumulated_rng( "ancestral_instinct", rng::CfromP( talents.ancestral_instinct_chance ) );
  rng_objects.deep_rend = get_accumulated_rng( "deep_rend", rng::CfromP( talents.deep_rend_proc_chance_st ) );
  rng_objects.heart_splitter_twice =
      get_accumulated_rng( "heart_splitter_twice", rng::CfromP( legendary.lego_2_hit_chance ) );
  rng_objects.heart_splitter_twice_2 =
      get_accumulated_rng( "heart_splitter_twice_2", rng::CfromP( legendary.lego_4_hit_chance ) );
}

// gunde_t::reset ===========================================================

void gunde_t::reset()
{
  fs_player_t::reset();
  for ( gunde_td_t* td : target_data.get_entries() )
  {
    if ( !td )
      continue;

    td->slaughter_tracker.current_tick = 0;
    td->rend_tracker.current_tick      = 0;

    for ( auto& bucket : td->slaughter_tracker.tick_buckets )
    {
      bucket = 0;
    }
    for ( auto& bucket : td->rend_tracker.tick_buckets )
    {
      bucket = 0;
    }
  }
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

double gunde_t::get_current_rend( player_t* t ) const
{
  auto& rend_obj = get_target_data( t )->rend_tracker;
  return std::accumulate( rend_obj.tick_buckets.begin(), rend_obj.tick_buckets.end(), 0.0 );
}

void gunde_t::spawn_feathers( int quantity )
{
  buffs.owed_in_blood->trigger( quantity );
  procs.feathers->occur( quantity );
  
  if ( talents_enabled( gunde_t::SLAYERS_GRIN ) )
  {
    for ( int i = 0; i < quantity; i++ )
      cooldowns.rupture->adjust( -talents.slayers_grin_cdr );
  }

  if ( talents_enabled( gunde_t::RAVENS_PRECISION ) )
  {
    for ( int i = 0; i < quantity; i++ )
      actions.ravens_precision->execute();
  }
}

// gunde_t::arise ===========================================================

void gunde_t::arise()
{
  fs_player_t::arise();
  sim->print_debug( "{} arises. Current max hp is: {}, current is: {}, base: {}, base_mul: {}, init_mul: {}", *this,
                    resources.max[ RESOURCE_HEALTH ], resources.current[ RESOURCE_HEALTH ],
                    resources.base[ RESOURCE_HEALTH ], resources.base_multiplier[ RESOURCE_HEALTH ],
                    resources.initial_multiplier[ RESOURCE_HEALTH ] );
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
  p()->spawn_feathers( p()->spell_const.spirit_proc_orbs );
}

template <typename Base>
void actions::gunde_action_t<Base>::apply_rend( const action_state_t* state )
{
  // todo get from state.
  auto rs = cast_state( state );
  if ( rs->get_rend_coefficient() <= 0 || state->result_amount <= 0 )
    return;

  auto action       = p()->actions.rend;
  action->set_target( state->target );

  action_state_t* s = action->get_state();
  s->result_amount = state->result_amount * rs->get_rend_coefficient();
  s->target = state->target;
  s->result = RESULT_HIT;
  action->snapshot_state( s, result_amount_type::DMG_OVER_TIME );
  action->schedule_travel( s );
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
