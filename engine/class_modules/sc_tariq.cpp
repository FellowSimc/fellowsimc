#include "fs_player.hpp"
#include "util/util.hpp"

#include "simulationcraft.hpp"
#include <unordered_set>

namespace fellowship
{
namespace tariq
{

// Forward Declarations
class tariq_t;

enum class secondary_trigger
{
  NONE = 0U,
  BLOOD_AND_THUNDER = 1U,
  ACE_OF_SPADES = 2U
};

namespace actions
{
struct tariq_attack_t;
struct tariq_heal_t;
struct tariq_spell_t;

struct melee_t;
}  // namespace actions

class tariq_td_t : public fs_player_td_t
{
public:
  struct
  {
    buff_t* slayers_mosh;
  } debuffs;

  tariq_td_t( player_t* target, tariq_t* source );
};

struct tariq_buff_t : public fs_player_buff_t
{
  tariq_buff_t( player_t* p, util::string_view name ) : fs_player_buff_t( p, name )
  {
  }

  tariq_t* p()
  {
    return debug_cast<tariq_t*>( player );
  }

  const tariq_t* p() const
  {
    return debug_cast<const tariq_t*>( player );
  }
};

class tariq_t : public fellowship::fs_player_t
{
public:
  struct actions_t
  {
    action_t* auto_attack;
    actions::melee_t* melee_hit;
    action_t* chain_lightning_ace_of_spades_spin;
    action_t* chain_lightning_ace_of_spades_crush;
    action_t* chain_lightning_blood_and_thunder;
    action_t* raging_currents;
  } actions;

  struct buffs_t
  {
    buff_t* thunder_call;
    buff_t* focused_wrath;
    buff_t* raging_currents;
    buff_t* ride_the_lightning;
    buff_t* square_hammer_stacking;
    buff_t* square_hammer_buff;
    buff_t* far_beyond_driven;
    buff_t* kill_em_all;
    buff_t* schism_hammer_storm;
    buff_t* schism_skull_crusher;
    buff_t* executioners_grin;
    buff_t* thundering_vortex;
  } buffs;

  struct cooldowns_t
  {
    cooldown_t* heavy_strike;
    cooldown_t* leap_smash;
    cooldown_t* thunder_call;
  } cooldowns;

  struct gains_t
  {
    gain_t* spirit_procs;
    gain_t* ride_the_lightning;
  } gains;

  struct procs_t
  {
    proc_t* blood_and_thunder;
    proc_t* ace_of_spades;
    proc_t* schism_skull_crusher;
    proc_t* schism_hammer_storm;
    proc_t* them_bones;
    proc_t* kill_em_all;
    proc_t* executioners_grin;
  } procs;

  struct rng_objects_t
  {
    accumulated_rng_t* them_bones;
    accumulated_rng_t* schism_skull_crusher;
    accumulated_rng_t* schism_hammer_storm;
    accumulated_rng_t* kill_em_all;
    accumulated_rng_t* blood_and_thunder;
    accumulated_rng_t* executioners_grin;
    real_ppm_t* ace_of_spades;
  } rng_objects;

#define TARIQ_TALENT_LIST( X )                                        \
  X( LEFT_HAND_PATH, "left_hand_path", "Left Hand Path" )             \
  X( RIDE_THE_LIGHTNING, "ride_the_lightning", "Ride the Lightning" ) \
  X( SQUARE_HAMMER, "square_hammer", "Square Hammer" )                \
  X( HIGH_ROAD, "high_road", "High Road" )                            \
  X( BLOOD_AND_THUNDER, "blood_and_thunder", "Blood and Thunder" )    \
  X( BLOODLINE, "bloodline", "Bloodline" )                            \
  X( THE_MOTHERLOAD, "the_motherload", "The Motherload" )             \
  X( MOUTH_FOR_WAR, "mouth_for_war", "Mouth for War" )                \
  X( THUNDERSTRUCK, "thunderstruck", "Thunderstruck" )                \
  X( PNEUMA, "pneuma", "Pneuma" )                                     \
  X( SLEDGEHAMMER, "sledgehammer", "Sledgehammer" )                   \
  X( FAR_BEYOND_DRIVEN, "far_beyond_driven", "Far Beyond Driven" )    \
  X( KILL_EM_ALL, "kill_em_all", "Kill Em All" )                      \
  X( ACE_OF_SPADES, "ace_of_spades", "Ace of Spades" )                \
  X( SCHISM, "schism", "Schism" )                                     \
  X( THEM_BONES, "them_bones", "Them Bones" )                         \
  X( CRACK_THE_SKY, "crack_the_sky", "Crack the Sky" )                \
  X( KILLING_IN_THE_NAME, "killing_in_the_name", "Killing in the Name" )

  enum tariq_talent_index_t
  {
#define X( name, id, pretty ) name##_INDEX,
    TARIQ_TALENT_LIST( X )
#undef X
        TARIQ_TALENT_MAX
  };

  enum tariq_talents_t : unsigned long long
  {
    NONE = 0,
#define X( name, id, pretty ) name = 1ULL << name##_INDEX,
    TARIQ_TALENT_LIST( X )
#undef X
        MAX = 1ULL << TARIQ_TALENT_MAX
  };

  static constexpr talent_info tariq_TALENTS[] = {
#define X( name, id, pretty ) { tariq_talents_t::name, id, pretty },
      TARIQ_TALENT_LIST( X )
#undef X
  };

  constexpr std::string_view talent_name( long long t ) override
  {
    for ( const auto& talent : tariq_TALENTS )
      if ( talent.flag == t )
        return talent.id;

    return "unknown_talent";
  }

  constexpr std::string_view talent_name_formatted( long long t ) override
  {
    for ( const auto& talent : tariq_TALENTS )
      if ( talent.flag == t )
        return talent.pretty;

    return "Unknown Talent";
  }

  struct spell_const_t
  {
    // Ink.MeleeAutoAttack.StrengthCoefficient, 1.24
    // Ink.MeleeAutoAttack.ResourceGain, 0.01	; In percentage of max
    // Ink.MeleeAutoAttack.Spread, 0.1
    // Ink.MeleeAutoAttack.VisualHitDelay, 0.6	; AutoAttackBuff uses the same
    timespan_t auto_attack_time = 30_s;
    double auto_attack_coeff = 1.24;
    timespan_t melee_hit_visual_delay = 0.6_s;
    double auto_attack_fury           = 1.0;

    // Ink.SpiritProc.ResourceRefund.FlatSpiritPointGain, 1.0
    double spirit_refund_mul = 1.0;

    // Ink.Rage.MaxRageMultiplier, 200.0					; This is multiplied with his strength and will result in his total amount of max rage
    // Ink.Rage.RageIncreaseBaseline, 0.5				;was 7.0        The rage will increase by RageIncreaseBaseline * (Damage Done / 100) and same for Damage Taken
    
    //Ink.DodgeChanceIncreaseForMeleeAttacks, 0.03	; This is an additive operation
    double wild_swing_extra_dodge = 0.03;
    //Ink.Expertise.ExpertiseDivider, 3.0	; (Expertise % / ExpertiseDivider)% * Stacks

    //Ink.Expertise.LightningDamageIncrease.ExpertiseMultiplier, 1.0
    
    //Ink.Rage.MaxRageMultiplier, 200.0					; This is multiplied with his strength and will result in his total amount of max rage
    //Ink.Rage.RageIncreaseBaseline, 0.5				;was 7.0        The rage will increase by RageIncreaseBaseline * (Damage Done / 100) and same for Damage Taken
    //Ink.Rage.OutOfCombat.TickInterval, 1.0
    //Ink.Rage.OutOfCombat.AmountToSpendPerTick, 0.02		; Percentage of Max Rage

    double max_fury = 100.0; // Only care about "percents"

    
    //Ink.AutoAttackBuff.StrengthCoefficient, 6.2560	;was 4.065 then 5.0813 then 4.3902
    double heavy_strike_coeff = 5.66;
    //Ink.AutoAttackBuff.Weak.DamageMultiplier, 0.2
    //Ink.AutoAttackBuff.Weak.ResourceGainMultiplier, 0.2
    //Ink.AutoAttackBuff.Cleave.DamageMultiplier, 0.20	;was 0.50
    double heavy_strike_cleave_multiplier = 0.3;
    //Ink.AutoAttackBuff.Cleave.PieMaxRadius, 800.0	; Total length
    //Ink.AutoAttackBuff.Cleave.PieMinRadius, 10.0	; Offset from character position. VFX will also start from here
    //Ink.AutoAttackBuff.Cleave.PieAngleWidth, 270.0
    //Ink.AutoAttackBuff.Cleave.TargetThresholdForDamageScale, 5.0	;was 8.0
    double heavy_strike_target_threshold = 8.0;
    //Ink.AutoAttackBuff.ResourceGain, 0.12	; was 0.05 In percentage of max
    double heavy_strike_fury = 12;
    //Ink.AutoAttackBuff.Cost, 0.0
    //Ink.AutoAttackBuff.VisualDelay, 0.6
    //Ink.AutoAttackBuff.Cooldown, 8.0
    timespan_t heavy_strike_cooldown = 8_s;
    //Ink.AutoAttackBuff.Lightning.StrengthCoefficient, 2.1896 	;was 1.4227 and then x and then 1.7784
    double heavy_strike_lightning_coeff = 1.98;
    //Ink.AutoAttackBuff.Lightning.TargetThresholdForDamageScale, 5.0	;was 8.0
    double heavy_strike_lightning_threshold = 5.0;
    //Ink.AutoAttackBuff.Lightning.VisualDelay, 0.25	; Added upon the original visual delay

    //Ink.MoveToLocationAoeDamage.MaxRangeHorizontal, 3000.0
    //Ink.MoveToLocationAoeDamage.MaxRangeUp, 1000.0
    //Ink.MoveToLocationAoeDamage.MaxRangeDown, 5000.0
    //Ink.MoveToLocationAoeDamage.StrengthCoefficient, 0.69
    double leap_smash_coeff = 0.69;
    //Ink.MoveToLocationAoeDamage.Spread, 0.1
    //Ink.MoveToLocationAoeDamage.DamageRadius, 500.0
    //Ink.MoveToLocationAoeDamage.TargetThresholdForDamageScale, 8.0
    double leap_smash_threshold = 8;
    //Ink.MoveToLocationAoeDamage.MoveDuration, 0.25
    //Ink.MoveToLocationAoeDamage.Cooldown, 20.0
    timespan_t leap_smash_cooldown = 20_s;
    

    //Ink.HeavySingleTargetAttack.StrengthCoefficient, 12.2071				;was 9.162 then 10.078 then 13.4374 then 17.4686 then 11.4218
    double skull_crusher_coeff = 11.6;
    //Ink.HeavySingleTargetAttack.Spread, 0.1
    //Ink.HeavySingleTargetAttack.VisualHitDelay, 0.52
    //Ink.HeavySingleTargetAttack.Cost, 0.25	; Percentage of Max Rage
    double skull_crusher_cost = 25;
    //Ink.HeavySingleTargetAttack.Cooldown, 1.0
    timespan_t skull_crusher_gcd = 1_s;
    //Ink.HeavySingleTargetAttack.Lightning.StrengthCoefficient, 4.2725 	;was 2.863 then 3.5273 then 4.7031 then 6.1140 then 3.9976
    double skull_crusher_lightning_coeff = 4.06;
    //Ink.HeavySingleTargetAttack.Lightning.Spread, 0.20
    //Ink.HeavySingleTargetAttack.Lightning.VisualHitDelay, 0.35	; Added upon the original visual delay
    
    //Ink.AoeAttack.StrengthCoefficient, 1.2514	;was 1.1047 then 1.3256 then 1.0970
    double hammer_storm_coeff = 2.23;
    //Ink.AoeAttack.Spread, 0.1
    //Ink.AoeAttack.Radius, 700.0
    //Ink.AoeAttack.TargetThresholdForDamageScale, 8.0
    double hammer_storm_threshold = 12.0;
    //Ink.AoeAttack.VisualHitDelay, 0.00	; Leaving this as 0 means that the hit will be registret immediately on the first tick
    //Ink.AoeAttack.VisualEndDelay, 0.3 Ink.AoeAttack.Cost, 0.50				;
    // Percentage of Max Rage
    double hammer_storm_cost = 50;
    // Ink.AoeAttack.CostPerTick, 0.04		; DISABLED Percentage of Max Rage
    //Ink.AoeAttack.TickInterval, 0.50
    timespan_t hammer_storm_period = 0.5_s;
    int hammer_storm_spins         = 3;
    //Ink.AoeAttack.MaxAttacks, 4.0			; (Activation + ticks)
    //Ink.AoeAttack.MaxRageToSpend, 0.25	; Percentage of Max Rage
    //Ink.AoeAttack.Cooldown, 1.5
    //Ink.AoeAttack.Lightning.StrengthCoefficient, 0.4380	;was 0.3866 then 0.425 then 0.3840
    double hammer_storm_lightning_coeff = 0.78;
    //Ink.AoeAttack.Lightning.Spread, 0.20
    //Ink.AoeAttack.Lightning.VisualDelay, 0.08	; Added upon the original visual delay
    //Ink.AoeAttack.Lightning.TargetThresholdForDamageScale, 10.0	;was 12.0
    double hammer_storm_lightning_threshold = 15.0;
    //Ink.AoeAttack.Talent.ReducedIncomingDamage.IncomingDamageMultiplier, 0.80	;talent disabled
    //Ink.AoeAttack.Talent.ReducedIncomingDamage.IncreasedStatBonus, 0.20			;talent disabled	
    //Ink.AoeAttack.Talent.ReducedIncomingDamage.PassiveReduceIncomingAoeDamage, 0.10
    //Ink.AoeAttack.Talent.ReducedDurationIncreasedDamageCost.CostMultiplier, 2.0	;not used
    //Ink.AoeAttack.Talent.ReducedDurationIncreasedDamageCost.DamageMultiplier, 1.00	;not used
    //Ink.AoeAttack.Talent.ReducedDurationIncreasedDamageCost.DurationMultiplier, 0.5	;not used
    //Ink.AoeAttack.Talent.ReducedDurationIncreasedDamageCost.DamageExponential, 1.35
    double hammer_storm_amp_per_spin = 1.35;
    //Ink.AoeAttack.Talent.ReducedDurationIncreasedDamageCost.MaxNumberOfTicks, 3.15
    

    ////Ink.SecondChanceSingleTargetAttack.StrengthCoefficient, 5.915	;was 5.20
    //double face_breaker_coeff = 5.915;
    ////Ink.SecondChanceSingleTargetAttack.Spread, 0.1
    ////Ink.SecondChanceSingleTargetAttack.VisualHitDelay, 0.23
    ////Ink.SecondChanceSingleTargetAttack.Cost, 0.0
    ////Ink.SecondChanceSingleTargetAttack.Cooldown, 9.0
    //timespan_t face_breaker_cooldown = 9_s;
    ////Ink.SecondChanceSingleTargetAttack.Proc.NotActive.DamageMultiplier, 0.2
    ////Ink.SecondChanceSingleTargetAttack.ResourceGain, 0.05	; In percentage of max
    //double face_breaker_fury = 5;
    ////Ink.SecondChanceSingleTargetAttack.Talent.IncreasedCritChance.Increase, 0.40	; Additive
    ////Ink.SecondChanceSingleTargetAttack.Talent.PassiveIncreasedDamageMultiplier.Increase, 1.00	; 
    ////Ink.SecondChanceSingleTargetAttack.Talent.CritBleed.DamageToDotScaler, 3.00
    ////Ink.SecondChanceSingleTargetAttack.Talent.CritBleed.DotDuration, 24.0
    ////Ink.SecondChanceSingleTargetAttack.Talent.CritBleed.DotTickPeriod, 1.5
    //Ink.SecondChanceSingleTargetAttack.Talent.CritAoe.DirectDamageToDamageScaler, 0.50		;was 0.30
    double face_breaker_cleave_fraction = 0.5;
    //Ink.SecondChanceSingleTargetAttack.Talent.CritAoe.TargetThresholdForDamageScale, 12.0
    double face_breaker_cleave_falloff = 12.0;
    

    //Ink.SingleTargetHeavyNonSpender.StrengthCoefficient, 5.915	;was 4.732
    double face_breaker_coeff = 6.18426;
    //Ink.SingleTargetHeavyNonSpender.VisualHitDelay, 0.23
    //Ink.SingleTargetHeavyNonSpender.Cost, 0.0
    //Ink.SingleTargetHeavyNonSpender.Cooldown, 6.0
    timespan_t face_breaker_cd = 6_s;
    //Ink.SingleTargetHeavyNonSpender.ResourceGain, 0.07	;was 0.05 In percentage of max
    double face_breaker_fury = 7;

    //Ink.CleaveAttack.StrengthCoefficient, 1.5625	;was 1.25
    double wild_swing_coeff = 1.2914;
    //Ink.CleaveAttack.Spread, 0.1
    //Ink.CleaveAttack.Radius, 500.0
    //Ink.CleaveAttack.TargetThresholdForDamageScale, 5.0
    double wild_swing_threshold = 5;
    //Ink.CleaveAttack.VisualHitDelay, 0.15
    //Ink.CleaveAttack.Cooldown, 1.5
    //Ink.CleaveAttack.Cost, 0.0
    //Ink.CleaveAttack.ResourceGain, 0.03		;was 2.5 In percentage of max
    double wild_swing_fury = 3;
    //Ink.CleaveAttack.DodgeChanceIncrease.MinNumOfTargets, 1.0	; The dodge chance increase is at max at and before this value
    //Ink.CleaveAttack.DodgeChanceIncrease.DodgeChanceDecreasePerTarget, 0.08	; Dodge chance decrease of MaxDodgeChanceIncrease per additional target after MinNumOfTargets
    //Ink.CleaveAttack.DodgeChanceIncrease.MaxDodgeChanceIncrease, 0.16	; The dodge chance at and below MinNumOfTargets
    double wild_swing_max_dodge = 0.16;
    double wild_swing_dodge_decrease_per_target = 0.08;
    
    //Ink.ChargedBuff.VisualDelay, 0.27
    //Ink.ChargedBuff.Duration, 21.0
    timespan_t thunder_call_duration = 21_s;
    //Ink.ChargedBuff.Cooldown, 45.0
    timespan_t thunder_call_cooldown = 45_s;
    //Ink.ChargedBuff.ResourceGain, 0.0
    
    //Ink.BouncyProjectile.StrengthCoefficient, 2.0163	;was 1.47 then 2.205 then 1.617 then 1.6979 then 1.568 then 1.1290
    double chain_lightning_coeff = 1.4998;
    //Ink.BouncyProjectile.Spread, 0.1
    //Ink.BouncyProjectile.MaxRange, 3000.0
    //Ink.BouncyProjectile.MaxNumOfTargets, 6.0
    int chain_lightning_bounces = 6;
    //Ink.BouncyProjectile.BounceSearchMaxRadius, 700.0
    //Ink.BouncyProjectile.Cooldown, 8.0
    timespan_t chain_lightning_cooldown = 8_s;
    //Ink.BouncyProjectile.TimeBetweenTargets, 0.05
    //Ink.BouncyProjectile.SpawnDelay, 0.30
    timespan_t chain_lightning_delay = 0.3_s;
    //Ink.BouncyProjectile.ResourceGainPerTarget, 0.012	; was 0.015In percentage of max
    double chain_lightning_fury_per_hit = 1.2;
    
    //Ink.MeleeInterrupt.Range, 550.0
    //Ink.MeleeInterrupt.Duration, 4.0
    //Ink.MeleeInterrupt.Cooldown, 16.0
    
    //Ink.EnhancedChargedBuff.SpiritPointCost, 100.0
    //Ink.EnhancedChargedBuff.ResourceGain, 0.0
    //Ink.EnhancedChargedBuff.BuffDuration, 21.0				;Should be equal to and increase with Heroism Duration gem bonuses
    //timespan_t raging_tempest_empowered_thundercall_duration = 21_s;
    //Ink.EnhancedChargedBuff.MaxRangeHorizontal, 3000.0
    //Ink.EnhancedChargedBuff.MaxRangeUp, 1000.0
    //Ink.EnhancedChargedBuff.MaxRangeDown, 5000.0
    //Ink.EnhancedChargedBuff.MoveDuration, 0.3
    //Ink.EnhancedChargedBuff.VisualHitDelay, 0.2
    //Ink.EnhancedChargedBuff.AoeImpact.StrengthCoefficient, 10.06
    double raging_tempest_impact_coeff = 10.06;
    //Ink.EnhancedChargedBuff.AoeImpact.DamageRadius, 500.0
    //Ink.EnhancedChargedBuff.AoeImpact.TargetThresholdForDamageScale, 5.0
    double raging_tempest_impact_threshold = 5;
    //Ink.EnhancedChargedBuff.PulsatingSingleDamage.PulsePeriod, 1.0
    timespan_t raging_tempest_pulse_period = 1_s;
    //Ink.EnhancedChargedBuff.PulsatingSingleDamage.TargetingRange, 800.0
    //Ink.EnhancedChargedBuff.PulsatingSingleDamage.VisualDelay, 0.05
    //Ink.EnhancedChargedBuff.PulsatingSingleDamage.Damage.StrengthCoefficient, 1.54
    double raging_tempest_pulse_coeff = 1.54;
    //Ink.EnhancedChargedBuff.PulsatingSingleDamage.Damage.Spread, 0.1
    //Ink.EnhancedChargedBuff.PulsatingSingleDamage.SelfBuff.CritChanceIncreasePerStack, 0.01
    double raging_tempest_currents_expertise_per_stack = 0.01;
    //Ink.EnhancedChargedBuff.PulsatingSingleDamage.SelfBuff.StackCap, 20 ; NOTE; SET IN GE!!! THIS IS NOT USED
    int raging_tempest_currents_max_stack = 20;
    //Ink.EnhancedChargedBuff.PulsatingSingleDamage.SelfBuff.Duration, 20.0			;Should be equal to and increase with Heroism Duration gem bonuses
    timespan_t raging_tempest_currents_duration = 20_s;
    
    //Ink.LowHealthSingleTargetResourceDamage.VisualHitDelay, 0.23
    //Ink.LowHealthSingleTargetResourceDamage.StrengthCoefficient, 3.6575	;was 3.08
    double culling_strike_coeff = 3.6575 * 0.87;
    //Ink.LowHealthSingleTargetResourceDamage.Spread, 0.1
    //Ink.LowHealthSingleTargetResourceDamage.MaxResourceToSpend, 0.10		    ; Percentage of Max fury
    double culling_strike_max_fury = 10;
    //Ink.LowHealthSingleTargetResourceDamage.DamageIncreasePerResource, 0.40		; Per Resource means per every percentage of fury
    double culling_strike_increase_per_fury = 0.4;
    //Ink.LowHealthSingleTargetResourceDamage.Cooldown, 6.0
    timespan_t culling_strike_cooldown = 6_s;
    //Ink.LowHealthSingleTargetResourceDamage.Lightning.Buff.Duration, 8.0
    //Ink.LowHealthSingleTargetResourceDamage.Lightning.Buff.StrengthIncreasePerStack, 1.02
    
    //Ink.AoeFear.Radius, 1200.0
    //Ink.AoeFear.Duration, 12.0
    //Ink.AoeFear.MaxNumTargets, 40.0
    //Ink.AoeFear.EffectDamageThresholdScaler, 30.0
    //Ink.AoeFear.VisualDelay, 0.3
    //Ink.AoeFear.Cooldown, 90.0
   
    //Ink.CostReductionBuff.Duration, 15
    timespan_t focused_wrath_duration = 15_s;
    //Ink.CostReductionBuff.Cooldown, 90
    timespan_t focused_wrath_cooldown = 90_s;
    //Ink.CostReductionBuff.CostReduction, 0.48
    double focused_wrath_cost_multiplier = 0.48;
    //Ink.CostReductionBuff.DamageIncrease, 1.4
    double focused_wrath_damage_multiplier = 1.1;
    //Ink.CostReductionBuff.NumOfStacks, 2.0
    int focused_wrath_stacks = 2;
    int focused_wrath_max_stacks = 3;
    //Ink.CostReductionBuff.VisualDelay, 0.3
    //Ink.CostReductionBuff.Talent.CooldownReductionInSeconds, 45.0
    
    //Ink.SelfDefenceBuff.Cooldown, 30.0
    //Ink.SelfDefenceBuff.FirstDuration, 4.0
    //Ink.SelfDefenceBuff.FirstIncomingDamageReduction, 0.6
    //Ink.SelfDefenceBuff.SecondIncomingDamageReduction, 0.8
    //Ink.SelfDefenceBuff.VisualDelay, 0.3
    //Ink.SelfDefenceBuff.Talent.IncreasedDurationAndTaunt.AdditionalDuration, 4.0
    //Ink.SelfDefenceBuff.Talent.IncreasedDurationAndTaunt.Taunt.MaxRange, 4000.0
    //Ink.SelfDefenceBuff.Talent.IncreasedDurationAndTaunt.Taunt.Cooldown, 15.0
    //Ink.SelfDefenceBuff.Talent.IncreasedDurationAndTaunt.Taunt.Duration, 8.0
    //Ink.SelfDefenceBuff.Talent.IncreasedDurationAndTaunt.Taunt.VisualDelay, 0.18
  } spell_const;

  struct talents_t
  {
    double pneuma_start_spirit = 50;
    double pneuma_generated_spirit = 1.0;

    int mouth_for_war_focused_wrath_stacks = 1;

    // Heavy Strike
    // Ink.Talent.AutoAttackBuffDurationalStatBonus.StatIncrease, 0.015		;This is the active effect on attackspeedincrease talent
    double far_beyond_driven_spirit_per_stack = 0.015;
    // Ink.Talent.AutoAttackBuffDurationalStatBonus.Duration, 20.0
    timespan_t far_beyond_driven_duration = 20_s;
    // Ink.Talent.AutoAttackBuffDurationalStatBonus.MaxStacks, 5.0
    int far_beyond_driven_max_stacks = 5;
    // Ink.AutoAttackBuff.Talent.GainToIncreasedDamage.DamageIncreasePerStack, 0.08	;was 0.05
    // Ink.AutoAttackBuff.Talent.GainToIncreasedDamage.PassiveCritStrikePowerScaler, 1.10
    // Ink.AutoAttackBuff.Talent.GainToIncreasedDamage.MaxStacks, 3.0
    // Ink.AutoAttackBuff.Talent.GainToIncreasedDamage.ConsumeStacks.CooldownReduction, 0.4
    
    // Ink.AutoAttackBuff.Talent.StackingBuffConsumedByNextSpender.Stacker.MaxStacks, 5.0		;new Square
    int square_hammer_max_stacks = 5;
    // Hammer Ink.AutoAttackBuff.Talent.StackingBuffConsumedByNextSpender.Stacker.Duration, 20.0
    timespan_t square_hammer_duration = 20_s;
    // Ink.AutoAttackBuff.Talent.StackingBuffConsumedByNextSpender.Buff.Duration, 4.0
    timespan_t square_hammer_expertise_duration = 4_s;
    // Ink.AutoAttackBuff.Talent.StackingBuffConsumedByNextSpender.Buff.AddedExpertisePerStack, 0.05
    double square_hammer_expertise_per_stack = 0.05;
    timespan_t square_hammer_cdr_per_stack   = 1_s;
    
    // Ink.AutoAttackBuff.Talent.AlwaysHitWindowBuff.ProcChance, 0.08
    double kill_em_all_chance = 0.08;
    // Ink.AutoAttackBuff.Talent.AlwaysHitWindowBuff.Buff.Duration, 8.0
    timespan_t kill_em_all_duration = 8_s;
    // Ink.AutoAttackBuff.Talent.AlwaysHitWindowBuff.Buff.MaxStacks, 2.0
    int kill_em_all_stacks = 2;
    // Ink.AutoAttackBuff.Talent.AlwaysHitWindowBuff.Buff.AutoAttackBuffDamageScaler, 3.00	;was 4.00
    double kill_em_all_dmg_multiplier = 2.5; // Multiplier, not an increase.
    double kill_em_all_cda            = 0.6;

    // Leap Smash
    // Ink.MoveToLocationAoeDamage.ResourceGain, 0.20
    double high_road_leap_smash_fury = 20;
    // Ink.MoveToLocationAoeDamage.VisualHitDelay, 0.03
    // Ink.MoveToLocationAoeDamage.Talent.TargetExtraInkDamage.IncomingDamageScaler, 1.15
    // Ink.MoveToLocationAoeDamage.Talent.TargetExtraInkDamage.Duration, 4.0
    // Ink.MoveToLocationAoeDamage.Talent.ProcCostReductionBuff.Chance, 0.50
    // Ink.MoveToLocationAoeDamage.Talent.ReducedCooldownIfNoHit.CooldownReductionInSeconds, 10.0
    // Ink.MoveToLocationAoeDamage.Talent.ReducedCooldownIfNoHit.MovementSpeedBuff.MovementSpeedScaler, 1.50
    // Ink.MoveToLocationAoeDamage.Talent.ReducedCooldownIfNoHit.MovementSpeedBuff.Duration, 4.0
    // Ink.MoveToLocationAoeDamage.Talent.IncreasedCriticalChance.AddedCrit, 0.40

    // Skull Crusher
    // Ink.HeavySingleTargetAttack.Talent.IncreasedDamageScaler, 1.20
    // Ink.HeavySingleTargetAttack.Talent.ProcAddedCritChance.Chance, 0.30
    double them_bones_chance = 0.3;
    // Ink.HeavySingleTargetAttack.Talent.ProcAddedCritChance.AddedCritChance, 1.0
    double them_bones_cc = 1.0;
    // Ink.HeavySingleTargetAttack.SwingTimerPauseDuration, 1.00	;was 0.75
    // Ink.HeavySingleTargetAttack.Talent.HeavySingleTargetAttackAoe.DamageScaler, 0.10
    double sledgehammer_multiplier = 0.1;
    // Ink.HeavySingleTargetAttack.Talent.HeavySingleTargetAttackAoe.Radius, 500.0
    // Ink.HeavySingleTargetAttack.Talent.HeavySingleTargetAttackAoe.TargetThresholdForDamageScale, 8.0
    double sledgehammer_threshold = 8;

    // Thunder Call
    //Ink.ChargedBuff.Talent.RageIncrease.Duration, 21.0
    timespan_t ride_the_lightning_duration = 21_s;
    // Ink.ChargedBuff.Talent.RageIncrease.TickInterval, 0.1	;was 0.2
    timespan_t ride_the_lightning_period = 0.1_s;
    // Ink.ChargedBuff.Talent.RageIncrease.RagePerTick, 0.00143	; was 0.003 Percentage of Max Rage
    double ride_the_lightning_fury_per_tick = 0.143;
    // Ink.ChargedBuff.Talent.AddedHaste, 0.05
    double ride_the_lightning_haste = 0.05;
    // Ink.ChargedBuff.Talent.LifestealAndStatboost.IncomingDamageMultiplier, 0.90
    // Ink.ChargedBuff.Talent.LifestealAndStatboost.DamageMultiplier, 1.05
    // Ink.ChargedBuff.Talent.LifestealAndStatboost.LifestealIncrease, 0.05
    // Ink.ChargedBuff.Talent.LifestealAndStatboost.Duration, 20.0
    // Ink.ChargedBuff.Talent.LifestealAndStatboost.TimeBetweenApplication, 0.1

    // Chain Lightning
    
    // Ink.BouncyProjectile.Talent.IncreasedMaxNumOfTargets.TotalTargets, 8.0
    int thunderstruck_max_bounces = 9;
    
    
    //Ink.BouncyProjectile.Talent.FreeCastFromAttack.Chance, 0.20			;Blood and Thunder
    double blood_and_thunder_chance = 0.2;
    //Ink.BouncyProjectile.Talent.FreeCastFromAttack.VFXDuration, 1.5		; The time until the vfx disappears
    //Ink.BouncyProjectile.Talent.FreeCastFromAttack.ActivationToLightningDelay, 0.2	; The time it takes between the first vfx and the spawn of the projectile
    
    //Ink.BouncyProjectile.Talent.FreeCastFromAoeAttack.PPMChance, 3.0	;Legendary Ring
    double ace_of_spades_ppm = 3;
    //Ink.BouncyProjectile.Talent.FreeCastFromAoeAttack.VFXDuration, 1.5		; The time until the vfx disappears
    //Ink.BouncyProjectile.Talent.FreeCastFromAoeAttack.ActivationToLightningDelay, 0.2	; The time it takes between the first vfx and the spawn of the projectile
    //Ink.BouncyProjectile.Talent.Charged.MaxCharges, 2.0
    int ace_of_spades_charges = 2;
    
    //Ink.BouncyProjectile.Talent.DamagePerUniqueTarget.Scaler, 0.40	;new talent
    double the_motherload_increase_per_enemy = 0.65;
    // Ink.BouncyProjectile.Talent.IncreasedDamage.DamageMultiplier, 1.10	;now as part of Blood and Thunder
    double the_motherload_multiplier = 1.0;
        
    // Ink.LowHealthSingleTargetResourceDamage.Talent.AddedCritChance, 0.35
    double killing_in_the_name_cc = 0.5;

    // Face Breaker
    // Ink.SecondChanceSingleTargetAttack.Talent.IncreasedCritChance.Increase, 0.40	; Additive
    double left_hand_path_cc = 0.4;
    
    // Ink.Talent.PassiveLightningCritChanceIncrease, 0.20
    double crack_the_sky_lightning_crit_chance = 0.2;

    // Ink.Talent.ChanceIncreasedSpenderDamage.PPM, 3.2	;old
    // Ink.Talent.ChanceIncreasedSpenderDamage.ProcChance, 0.25
    double schism_proc_chance = 0.35;
    // Ink.Talent.ChanceIncreasedSpenderDamage.HeavySingleTargetAttack.DamageMultiplier, 3.50
    double schism_damage_skull_crusher_multiplier = 2.5;
    // Ink.Talent.ChanceIncreasedSpenderDamage.AoeAttack.DamageMultiplier, 3.50
    double schism_damage_hammer_storm_multiplier = 2.5;
    
    // Ink.Talent.DecreasedIncomingMagicDamage.DamageMultiplier, 0.90
    // Ink.Talent.UnchargedResourceIncrease.ResourceMultiplier, 1.10

    // Ink.Talent.IncreasedMovementSpeed.SpeedMultiplier, 1.2
    double bloodline_movement_speed_mul = 1.2;
    // Ink.Talent.PassiveResourceGeneration.FuryPerTick, 0.01
    double bloodline_fury_amount = 1.0;
    // Ink.Talent.PassiveResourceGeneration.Period, 1.0
    timespan_t bloodline_fury_period = 1_s;

  } talents;

  struct legendary_t
  {
    bool slayers_mosh      = false;
    timespan_t slayers_mosh_duration = 6_s;
    double slayers_mosh_multiplier    = 1.2;

    bool thundering_vortex = false;
    int thundering_vortex_needed = 20;
    double thundering_vortex_multiplier = 1.5;
    double thundering_vortex_cl_fury_multiplier = 1.5;

    // Ink.LowHealthSingleTargetResourceDamage.Talent.Chance, 0.06
    // Ink.LowHealthSingleTargetResourceDamage.Talent.ResourceAmountToDamage, 0.20
    // Ink.LowHealthSingleTargetResourceDamage.Talent.BuffDuration, 8.0
    bool executioners_grin = false;
    double executioners_grin_chance = 0.06;
    int executioners_grin_counted_fury = 30;
    timespan_t executioners_grin_duration = 8_s;
  } legendary;

  struct options_t
  {
  } options;

  target_specific_t<tariq_td_t> target_data;

  const tariq_td_t* find_target_data( const player_t* target ) const override
  {
    return target_data[ target ];
  }

  tariq_td_t* get_target_data( player_t* target ) const override
  {
    tariq_td_t*& td = target_data[ target ];
    if ( !td )
    {
      td = new tariq_td_t( target, const_cast<tariq_t*>( this ) );
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
    return RESOURCE_FURY;
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

  tariq_t( sim_t* sim, util::string_view name, race_e r = RACE_NONE )
    : fs_player_t( sim, name, r, TARIQ ), target_data()
  {
    resource_regeneration = regen_type::DYNAMIC;


    create_cooldowns();
    spirit_refund_mul = spell_const.spirit_refund_mul;
  }
};

namespace actions
{  // namespace actions



struct tariq_action_state_t : public action_state_t
{
private:

public:
  tariq_action_state_t( action_t* action, player_t* target )
    : action_state_t( action, target )
  {
  }

  void initialize() override
  {
    action_state_t::initialize();
  }

  std::ostringstream& debug_str( std::ostringstream& s ) override
  {
    action_state_t::debug_str( s );
    return s;
  }

  void copy_state( const action_state_t* s )
  {
    action_state_t::copy_state( s );
    const tariq_action_state_t* rs = debug_cast<const tariq_action_state_t*>( s );
  }
};

template <typename Base>
class tariq_action_t : public fellowship::actions::fs_player_action_t<Base>
{
protected:
  /// typedef for tariq_action_t<action_base_t>
  using base_t = tariq_action_t<fellowship::actions::fs_player_action_t<Base>>;

private:
  /// typedef for the templated action type, eg. spell_t, attack_t, heal_t
  using ab = fellowship::actions::fs_player_action_t<Base>;

public:

  // Init =====================================================================

  tariq_action_t( util::string_view n, tariq_t* p, util::string_view options = {} )
    : ab( n, p, options )
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

  static const tariq_action_state_t* cast_state( const action_state_t* st )
  {
    return debug_cast<const tariq_action_state_t*>( st );
  }

  static tariq_action_state_t* cast_state( action_state_t* st )
  {
    return debug_cast<tariq_action_state_t*>( st );
  }

  action_state_t* new_state() override
  {
    return new tariq_action_state_t( this, ab::target );
  }

  void update_state( action_state_t* state, unsigned flags, result_amount_type rt ) override
  {
    ab::update_state( state, flags, rt );
  }

  void snapshot_state( action_state_t* state, result_amount_type rt ) override
  {
    auto rs = cast_state( state );

    ab::snapshot_state( state, rt );
  }


  tariq_t* p()
  {
    return debug_cast<tariq_t*>( ab::player );
  }

  const tariq_t* p() const
  {
    return debug_cast<const tariq_t*>( ab::player );
  }

  tariq_td_t* td( player_t* t ) const
  {
    return p()->get_target_data( t );
  }

  double composite_da_multiplier(const action_state_t* s) const
  {
    auto mul = ab::composite_da_multiplier( s );

    return mul;
  }

public:
  // Ability triggers
  void trigger_auto_attack( const action_state_t* );
  void trigger_spirit_refund( const action_state_t* );

  void roll_executioners_grin()
  {
    if ( !p()->legendary.executioners_grin )
      return;

    if ( p()->rng_objects.executioners_grin->trigger() )
    {
      p()->buffs.executioners_grin->trigger();
      p()->procs.executioners_grin->occur();
    }
  }

  void consume_resource() override
  {
    ab::consume_resource();

    if ( ab::current_resource() == RESOURCE_FURY && ab::last_resource_cost > 0 )
    {
      if ( p()->rng().roll( p()->cache.mastery_value() ) )
        trigger_spirit_refund( ab::execute_state );
    }
  }

  void roll_kill_em_all()
  {
    if ( !p()->talents_enabled( tariq_t::KILL_EM_ALL ) )
      return;

    if ( p()->rng_objects.kill_em_all->trigger() )
    {
      p()->buffs.kill_em_all->trigger( p()->talents.kill_em_all_stacks );
      p()->procs.kill_em_all->occur();
    }
  }

  bool in_thunder_call()
  {
    return p()->buffs.thunder_call->check() || p()->buffs.raging_currents->check();
  }

  void consume_square_hammer()
  {
    auto stacks = p()->buffs.square_hammer_stacking->check();
    if ( stacks )
    {
      if ( p()->buffs.square_hammer_buff->check() )
        p()->buffs.square_hammer_buff->refresh();
      else
        p()->buffs.square_hammer_buff->trigger( stacks );
      p()->cooldowns.thunder_call->adjust( -stacks * p()->talents.square_hammer_cdr_per_stack, false );
      p()->buffs.square_hammer_stacking->expire();
    }
  }
};

// ==========================================================================
// Rogue Attack Classes
// ==========================================================================

struct tariq_heal_t : public tariq_action_t<heal_t>
{
protected:
  using base_t = tariq_heal_t;

private:
  using ab = tariq_action_t<heal_t>;

public:
  tariq_heal_t( util::string_view n, tariq_t* p, util::string_view o = {} ) : ab( n, p, o )
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

struct tariq_spell_t : public tariq_action_t<spell_t>
{
protected:
  using base_t = tariq_spell_t;

private:
  using ab = tariq_action_t<spell_t>;

public:
  tariq_spell_t( util::string_view n, tariq_t* p, util::string_view o = {} ) : ab( n, p, o )
  {
    school = SCHOOL_MAGIC;
  }
};

struct tariq_attack_t : public tariq_action_t<melee_attack_t>
{
protected:
  using base_t = tariq_attack_t;

private:
  using ab = tariq_action_t<melee_attack_t>;

public:
  tariq_attack_t( util::string_view n, tariq_t* p, util::string_view o = {} ) : ab( n, p, o )
  {
    special = true;
    school  = SCHOOL_PHYSICAL;
  }

  void execute() override
  {
    ab::execute();

    if ( !background && trigger_gcd > timespan_t::zero() )
    {
      // not cull
      if ( id != 7 )
        roll_executioners_grin();

      // not heavy strike
      if ( id != 4 )
        roll_kill_em_all();
    }
  }
};

struct tariq_lightning_attack_t : public tariq_attack_t
{
protected:
  using base_t = tariq_lightning_attack_t;

private:
  using ab = tariq_attack_t;

public:
  tariq_lightning_attack_t( util::string_view n, tariq_t* p, util::string_view o = {} ) : ab( n, p, o )
  {
    special = true;
    school  = SCHOOL_MAGIC;
  }
  double composite_crit_chance() const override
  {
    auto cc = ab::composite_crit_chance();

    if ( p()->talents_enabled( tariq_t::CRACK_THE_SKY ) )
      cc += p()->talents.crack_the_sky_lightning_crit_chance;

    return cc;
  }
};

struct tariq_absorb_t : public tariq_action_t<absorb_t>
{
protected:
  using base_t = tariq_absorb_t;

private:
    using ab = tariq_action_t<absorb_t>;

public:
    tariq_absorb_t( util::string_view n, tariq_t* p, util::string_view o = {} ) : ab( n, p, o )
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

  melee_t( const char* name, const char* reporting_name, tariq_t* p )
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
  auto_melee_attack_t( tariq_t* p, util::string_view options_str ) : action_t( ACTION_OTHER, "auto_attack_hit", p )
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

struct wild_swing_t : public tariq_attack_t
{
  wild_swing_t( util::string_view name, tariq_t* p, util::string_view options_str = {} )
    : tariq_attack_t( name, p, options_str )
  {
    id                 = 2;
    name_str_reporting = "Wild Swing";

    ability_flags |= ability_type_e::ABILITY_BASIC;

    // cooldown->duration = p->spell_const.blood_arc_cooldown;
    // cooldown->charges  = 1;
    // cooldown->hasted   = true;

    attack_power_mod.direct = p->spell_const.wild_swing_coeff;

    aoe                 = -1;
    reduced_aoe_targets = p->spell_const.wild_swing_threshold;

    energize_resource = RESOURCE_FURY;
    energize_amount   = p->spell_const.wild_swing_fury;
    energize_type     = action_energize::ON_CAST;

    may_miss = true;
  }

  double miss_chance( action_state_t* s ) const override
  {
    auto m = base_t::miss_chance( s );

    if ( s->n_targets <= 2 )
      m += p()->spell_const.wild_swing_extra_dodge -
           s->n_targets * p()->spell_const.wild_swing_dodge_decrease_per_target;

    return m;
  }
};

struct face_breaker_t : public tariq_attack_t
{
  face_breaker_t( util::string_view name, tariq_t* p, util::string_view options_str = {} )
    : tariq_attack_t( name, p, options_str )
  {
    id                 = 3;
    name_str_reporting = "Face Breaker";

    ability_flags |= ability_type_e::ABILITY_BASIC;

    cooldown->duration = p->spell_const.face_breaker_cd;
    cooldown->charges  = 1;
    cooldown->hasted   = true;

    attack_power_mod.direct = p->spell_const.face_breaker_coeff;

    aoe                 = -1;
    full_amount_targets = 1;
    base_aoe_multiplier = p->spell_const.face_breaker_cleave_fraction;
    reduced_aoe_targets = p->spell_const.face_breaker_cleave_falloff;

    energize_resource = RESOURCE_FURY;
    energize_amount   = p->spell_const.face_breaker_fury;
    energize_type     = action_energize::ON_CAST;

    if ( p->talents_enabled( tariq_t::LEFT_HAND_PATH ) )
      base_crit += p->talents.left_hand_path_cc;
  }
};

struct heavy_strike_t : public tariq_attack_t
{
  struct heavy_strike_lightning_t : public tariq_lightning_attack_t
  {
    heavy_strike_lightning_t( util::string_view name, tariq_t* p )
      : tariq_lightning_attack_t( std::format( "{}_lightning", name ), p )
    {
      id                 = 4;
      name_str_reporting = "Heavy Strike (Lightning)";

      background = true;

      ability_flags |= ability_type_e::ABILITY_CORE;

      attack_power_mod.direct = p->spell_const.heavy_strike_lightning_coeff;

      aoe                 = -1;
      reduced_aoe_targets = p->spell_const.heavy_strike_lightning_threshold;
    }

    double composite_da_multiplier( const action_state_t* s ) const override
    {
      auto dam = base_t::composite_da_multiplier( s );

      dam *= 1.0 + p()->buffs.kill_em_all->check_value();

      return dam;
    }
  };

  heavy_strike_lightning_t* lightning_attack;
  heavy_strike_t( util::string_view name, tariq_t* p, util::string_view options_str = {} )
    : tariq_attack_t( name, p, options_str ), lightning_attack( new heavy_strike_lightning_t( name, p ) )
  {
    id                 = 4;
    name_str_reporting = "Heavy Strike";

    ability_flags |= ability_type_e::ABILITY_CORE;

    cooldown->duration = p->spell_const.heavy_strike_cooldown;
    cooldown->charges  = 1;
    cooldown->hasted   = true;

    attack_power_mod.direct = p->spell_const.heavy_strike_coeff;

    aoe                 = -1;
    full_amount_targets = 1;
    base_aoe_multiplier = p->spell_const.heavy_strike_cleave_multiplier;
    reduced_aoe_targets = p->spell_const.heavy_strike_target_threshold;

    energize_resource = RESOURCE_FURY;
    energize_amount   = p->spell_const.heavy_strike_fury;
    energize_type     = action_energize::ON_CAST;

    add_child( lightning_attack );
  }

  void init_finished() override
  {
    base_t::init_finished();

    if ( p()->actions.chain_lightning_blood_and_thunder )
      add_child( p()->actions.chain_lightning_blood_and_thunder );
  }

  double recharge_rate_multiplier( const cooldown_t& cd ) const override
  {
    auto rrm = base_t::recharge_rate_multiplier( cd );

    rrm = 1.0 / rrm;

    rrm += p()->buffs.kill_em_all->check() * p()->talents.kill_em_all_cda;

    rrm = 1.0 / rrm;

    return rrm;
  }

  double composite_da_multiplier( const action_state_t* s ) const override
  {
    auto dam = base_t::composite_da_multiplier( s );

    dam *= 1.0 + p()->buffs.kill_em_all->check_value();

    return dam;
  }

  void execute() override
  {
    base_t::execute();

    if ( in_thunder_call() )
    {
      lightning_attack->schedule_execute_child_attack( execute_state );
    }

    p()->buffs.kill_em_all->decrement();

    if ( p()->talents_enabled( tariq_t::SQUARE_HAMMER ) )
    {
      p()->buffs.square_hammer_stacking->trigger();
    }

    if ( p()->talents_enabled( tariq_t::FAR_BEYOND_DRIVEN ) )
    {
      p()->buffs.far_beyond_driven->trigger();
    }

    if ( p()->talents_enabled( tariq_t::BLOOD_AND_THUNDER ) )
    {
      if ( p()->rng_objects.blood_and_thunder->trigger() )
      {
        p()->actions.chain_lightning_blood_and_thunder->set_target( target );
        p()->actions.chain_lightning_blood_and_thunder->execute();
      }
    }
  }
};

struct skull_crusher_t : public tariq_attack_t
{
  struct skull_crusher_lightning_t : public tariq_lightning_attack_t
  {
    skull_crusher_lightning_t( util::string_view name, tariq_t* p )
      : tariq_lightning_attack_t( std::format( "{}_lightning", name ), p )
    {
      id                 = 5;
      name_str_reporting = "Skull Crusher (Lightning)";

      background = true;

      ability_flags |= ability_type_e::ABILITY_POWER;

      attack_power_mod.direct = p->spell_const.skull_crusher_lightning_coeff;
    }

    double composite_da_multiplier( const action_state_t* s ) const override
    {
      auto dam = base_t::composite_da_multiplier( s );

      dam *= 1.0 + p()->buffs.schism_skull_crusher->check_value();

      if ( p()->buffs.focused_wrath->check() )
        dam *= p()->spell_const.focused_wrath_damage_multiplier;

      return dam;
    }
  };

  struct sledgehammer_t : public tariq_attack_t
  {
    sledgehammer_t( util::string_view name, tariq_t* p ) : tariq_attack_t( std::format( "{}_sledgehammer", name ), p )
    {
      aoe                 = -1;
      base_multiplier     = p->spell_const.heavy_strike_cleave_multiplier;
      reduced_aoe_targets = p->spell_const.heavy_strike_target_threshold;
    }

    size_t available_targets( std::vector<player_t*>& tl ) const override
    {
      tl.clear();

      for ( auto* t : sim->target_non_sleeping_list )
      {
        if ( t->is_enemy() && ( t != target ) )
        {
          tl.push_back( t );
        }
      }

      if ( sim->debug && !sim->distance_targeting_enabled )
      {
        sim->print_debug( "{} regenerated target cache for {} ({})", *player, signature_str, *this );
        for ( size_t i = 0; i < tl.size(); i++ )
        {
          sim->print_debug( "[{}, {} (id={})]", i, *tl[ i ], tl[ i ]->actor_index );
        }
      }

      return tl.size();
    }

    void execute() override
    {
      if ( target_list().size() <= 0 )
        return;

      base_t::execute();
    }

    double composite_da_multiplier( const action_state_t* s ) const override
    {
      return base_multiplier;
    }

    void init_finished() override
    {
      base_t::init_finished();
      snapshot_flags = STATE_MUL_SPELL_DA;
      update_flags   = snapshot_flags;
    }
  };

  skull_crusher_lightning_t* lightning_attack;
  sledgehammer_t* sledgehammer;
  skull_crusher_t( util::string_view name, tariq_t* p, util::string_view options_str = {} )
    : tariq_attack_t( name, p, options_str ),
      lightning_attack( new skull_crusher_lightning_t( name, p ) ),
      sledgehammer( nullptr )
  {
    id                 = 5;
    name_str_reporting = "Skull Crusher";

    ability_flags |= ability_type_e::ABILITY_POWER;

    attack_power_mod.direct = p->spell_const.skull_crusher_coeff;

    resource_current            = RESOURCE_FURY;
    base_costs[ RESOURCE_FURY ] = p->spell_const.skull_crusher_cost;

    trigger_gcd = p->spell_const.skull_crusher_gcd;

    add_child( lightning_attack );

    if ( p->talents_enabled( tariq_t::SLEDGEHAMMER ) )
    {
      sledgehammer = new sledgehammer_t( name, p );
      add_child( sledgehammer );
    }
  }

  void init_finished() override
  {
    base_t::init_finished();

    if ( p()->actions.chain_lightning_ace_of_spades_crush )
      add_child( p()->actions.chain_lightning_ace_of_spades_crush );
  }

  double composite_crit_chance() const override
  {
    auto cc = base_t::composite_crit_chance();

    if ( p()->talents_enabled( tariq_t::THEM_BONES ) )
    {
      if ( p()->rng_objects.them_bones->trigger() )
      {
        cc += p()->talents.them_bones_cc;
        p()->procs.them_bones->occur();
      }
    }

    return cc;
  }

  double cost() const override
  {
    auto amount = base_t::cost();

    if ( p()->buffs.focused_wrath->up() )
      amount *= p()->spell_const.focused_wrath_cost_multiplier;

    return amount;
  }

  double composite_da_multiplier( const action_state_t* s ) const override
  {
    auto dam = base_t::composite_da_multiplier( s );

    dam *= 1.0 + p()->buffs.schism_skull_crusher->check_value();

    if ( p()->buffs.focused_wrath->check() )
      dam *= p()->spell_const.focused_wrath_damage_multiplier;

    return dam;
  }

  void impact( action_state_t* s ) override
  {
    base_t::impact( s );

    if ( sledgehammer )
    {
      sledgehammer->execute_on_target( s->target, s->result_amount );
    }
  }

  void execute() override
  {
    consume_square_hammer();

    base_t::execute();

    if ( in_thunder_call() )
    {
      lightning_attack->schedule_execute_child_attack( execute_state );
    }

    p()->buffs.focused_wrath->decrement();
    p()->buffs.schism_skull_crusher->decrement();

    if ( p()->talents_enabled( tariq_t::ACE_OF_SPADES ) )
    {
      if ( p()->rng_objects.ace_of_spades->trigger() )
      {
        p()->procs.ace_of_spades->occur();
        p()->actions.chain_lightning_ace_of_spades_crush->set_target( target );
        p()->actions.chain_lightning_ace_of_spades_crush->execute();
      }
    }

    if ( p()->talents_enabled( tariq_t::SCHISM ) )
    {
      if ( p()->rng_objects.schism_skull_crusher->trigger() )
      {
        p()->buffs.schism_hammer_storm->trigger();
        p()->procs.schism_skull_crusher->occur();
      }
    }
  }
};

struct hammer_storm_t : public tariq_attack_t
{
  struct hammer_storm_lightning_t : public tariq_lightning_attack_t
  {
    hammer_storm_lightning_t( util::string_view name, tariq_t* p )
      : tariq_lightning_attack_t( std::format( "{}_lightning", name ), p )
    {
      id                 = 6;
      name_str_reporting = "Hammer Storm (Lightning)";

      background = true;

      ability_flags |= ability_type_e::ABILITY_POWER;

      attack_power_mod.direct = p->spell_const.hammer_storm_lightning_coeff;
      aoe                     = -1;
      reduced_aoe_targets     = p->spell_const.hammer_storm_lightning_threshold;
    }

    double composite_da_multiplier( const action_state_t* s ) const override
    {
      auto dam = base_t::composite_da_multiplier( s );

      if ( parent_dot && parent_dot->current_tick > 1 )
      {
        dam *= pow( 1.35, ( parent_dot->current_tick - 1 ) );
      }

      return dam;
    }
  };

  struct hammer_storm_hit_t : public tariq_attack_t
  {
    hammer_storm_lightning_t* lightning_attack;
    hammer_storm_hit_t( util::string_view name, tariq_t* p )
      : tariq_attack_t( std::format( "{}_hit", name ), p ), lightning_attack( new hammer_storm_lightning_t( name, p ) )
    {
      id                 = 6;
      name_str_reporting = "Hammer Storm";

      background = true;

      ability_flags |= ability_type_e::ABILITY_POWER;

      attack_power_mod.direct = p->spell_const.hammer_storm_coeff;
      aoe                     = -1;
      reduced_aoe_targets     = p->spell_const.hammer_storm_threshold;

      add_child( lightning_attack );
    }

    double composite_da_multiplier( const action_state_t* s ) const override
    {
      auto dam = base_t::composite_da_multiplier( s );

      if ( parent_dot && parent_dot->current_tick > 1 )
      {
        dam *= pow( 1.35, ( parent_dot->current_tick - 1 ) );
      }

      return dam;
    }

    void execute()
    {
      base_t::execute();

      if ( in_thunder_call() )
      {
        lightning_attack->parent_dot = parent_dot;
        lightning_attack->schedule_execute_child_attack( execute_state );
      }

      if ( p()->talents_enabled( tariq_t::ACE_OF_SPADES ) )
      {
        if ( p()->rng_objects.ace_of_spades->trigger() )
        {
          p()->procs.ace_of_spades->occur();
          p()->actions.chain_lightning_ace_of_spades_spin->set_target( target );
          p()->actions.chain_lightning_ace_of_spades_spin->execute();
        }
      }
    }
  };

  hammer_storm_hit_t* custom_tick_action;
  hammer_storm_t( util::string_view name, tariq_t* p, util::string_view options_str = {} )
    : tariq_attack_t( name, p, options_str )
  {
    id                 = 6;
    name_str_reporting = "Hammer Storm";

    ability_flags |= ability_type_e::ABILITY_POWER;

    resource_current            = RESOURCE_FURY;
    base_costs[ RESOURCE_FURY ] = p->spell_const.hammer_storm_cost;

    channeled           = true;
    hasted_ticks        = true;
    hasted_dot_duration = true;
    dot_duration        = p->spell_const.hammer_storm_period * p->spell_const.hammer_storm_spins;
    base_tick_time      = p->spell_const.hammer_storm_period;

    custom_tick_action = new hammer_storm_hit_t( name, p );

    add_child( custom_tick_action );
  }

  double composite_persistent_multiplier( const action_state_t* s ) const override
  {
    auto dam = base_t::composite_persistent_multiplier( s );

    dam *= 1.0 + p()->buffs.schism_hammer_storm->check_value();

    if ( p()->buffs.focused_wrath->up() )
      dam *= p()->spell_const.focused_wrath_damage_multiplier;

    return dam;
  }

  double cost() const override
  {
    auto amount = base_t::cost();

    if ( p()->buffs.focused_wrath->check() )
      amount *= p()->spell_const.focused_wrath_cost_multiplier;

    return amount;
  }

  void init_finished() override
  {
    base_t::init_finished();

    if ( p()->actions.chain_lightning_ace_of_spades_spin )
      add_child( p()->actions.chain_lightning_ace_of_spades_spin );
  }

  void tick( dot_t* d ) override
  {
    base_t::tick( d );

    custom_tick_action->parent_dot = d;
    custom_tick_action->schedule_execute_child_attack( d->state );
  }

  void execute() override
  {
    consume_square_hammer();

    base_t::execute();

    p()->buffs.focused_wrath->decrement();
    p()->buffs.schism_hammer_storm->decrement();

    if ( p()->talents_enabled( tariq_t::SCHISM ) )
    {
      if ( p()->rng_objects.schism_hammer_storm->trigger() )
      {
        p()->buffs.schism_skull_crusher->trigger();
        p()->procs.schism_hammer_storm->occur();
      }
    }
  }
};

struct culling_strike_t : public tariq_attack_t
{
  double max_cost;
  culling_strike_t( util::string_view name, tariq_t* p, util::string_view options_str = {} )
    : tariq_attack_t( name, p, options_str ), max_cost( p->spell_const.culling_strike_max_fury )
  {
    id                 = 7;
    name_str_reporting = "Culling Strike";

    ability_flags |= ability_type_e::ABILITY_POWER;

    
    cooldown->duration = p->spell_const.culling_strike_cooldown;
    cooldown->charges  = 1;
    cooldown->hasted   = true;

    attack_power_mod.direct = p->spell_const.culling_strike_coeff;

    resource_current            = RESOURCE_FURY;
    base_costs[ RESOURCE_FURY ] = 1.0;

    trigger_gcd = 0_s;

    if ( p->talents_enabled( tariq_t::KILLING_IN_THE_NAME ) )
      base_crit += p->talents.killing_in_the_name_cc;

    if ( p->talents_enabled( tariq_t::PNEUMA ) )
    {
      energize_amount   = p->talents.pneuma_generated_spirit;
      energize_resource = RESOURCE_SPIRIT;
      energize_type     = action_energize::ON_HIT;
    }
  }

  bool target_ready( player_t* candidate_target )
  {
    if ( candidate_target->health_percentage() > low_health_threshold && !p()->buffs.executioners_grin->check() )
      return false;

    return base_t::target_ready( candidate_target );
  }

  double cost() const override
  {
    if ( p()->buffs.executioners_grin->check() )
      return 0.0;

    return std::max( base_t::cost(), std::min( max_cost, p()->resources.current[ RESOURCE_FURY ] ) );
  }

  double composite_da_multiplier( const action_state_t* s ) const override
  {
    auto dam = base_t::composite_da_multiplier( s );

    auto resource_cost =
        p()->buffs.executioners_grin->check() ? p()->legendary.executioners_grin_counted_fury : last_resource_cost;

    dam *= 1.0 + resource_cost * p()->spell_const.culling_strike_increase_per_fury;

    return dam;
  }

  void execute()
  {
    base_t::execute();
    p()->buffs.executioners_grin->decrement();
  }
};

struct thunder_call_t : public tariq_spell_t
{
  thunder_call_t( util::string_view name, tariq_t* p, util::string_view options_str = {} )
    : tariq_spell_t( name, p, options_str )
  {
    id = 8;

    name_str_reporting = "Thunder Call";

    trigger_gcd = timespan_t::zero();

    cooldown->duration = p->spell_const.thunder_call_cooldown;
    cooldown->hasted   = false;
    cooldown->charges  = 1;

    ability_flags |= ability_type_e::ABILITY_MAJOR;
  }

  bool ready() override
  {
    if ( in_thunder_call() )
      return false;

    return base_t::ready();
  }

  void execute() override
  {
    base_t::execute();
    p()->buffs.thunder_call->trigger();

    if ( p()->talent_enabled( tariq_t::RIDE_THE_LIGHTNING ) )
    {
      p()->buffs.ride_the_lightning->trigger();
    }
  }
};

struct chain_lightning_t : public tariq_lightning_attack_t
{
  secondary_trigger trigger_type;
  mutable std::unordered_set<size_t> hit_players;
  mutable std::vector<player_t*>::iterator target_list_end;

  chain_lightning_t( util::string_view name, tariq_t* p, util::string_view options_str = {},
                     secondary_trigger _trigger_type = secondary_trigger::NONE )
    : tariq_lightning_attack_t( name, p, options_str ),
      trigger_type( _trigger_type ),
      hit_players()
  {
    id = 9;

    name_str_reporting = "Chain Lightning";

    cooldown->duration = p->spell_const.chain_lightning_cooldown;
    cooldown->hasted   = true;
    cooldown->charges  = 1;

    ability_flags |= ability_type_e::ABILITY_CORE;

    energize_resource = RESOURCE_FURY;
    energize_amount   = p->spell_const.chain_lightning_fury_per_hit;
    energize_type     = action_energize::PER_HIT;

    attack_power_mod.direct = p->spell_const.chain_lightning_coeff;

    aoe = p->spell_const.chain_lightning_bounces;

    if ( p->legendary.thundering_vortex )
    {
      energize_amount *= p->legendary.thundering_vortex_cl_fury_multiplier;
    }

    if ( p->talents_enabled( tariq_t::THUNDERSTRUCK ) )
      aoe = p->talents.thunderstruck_max_bounces;

    if ( p->talents_enabled( tariq_t::ACE_OF_SPADES ) )
      cooldown->charges = p->talents.ace_of_spades_charges;

    if ( p->talents_enabled( tariq_t::THE_MOTHERLOAD ) )
      attack_power_mod.direct *= p->talents.the_motherload_multiplier;

    if ( _trigger_type != secondary_trigger::NONE )
    {
      background = true;
      cooldown->duration = 0_s;
      ability_flags = ability_type_e::ABILITY_NONE;
    }
  }

  void clear_masks()
  {
    hit_players.clear();
  }

  void set_bit_mask( player_t* p ) const
  {
    hit_players.insert( p->actor_index );
  }

  bool get_bit_mask( player_t* p )
  {
    return hit_players.contains( p->actor_index );
  }

  bool ready() override
  {
    if ( !in_thunder_call() )
      return false;

    return base_t::ready();
  }

  double composite_da_multiplier( const action_state_t* s ) const override
  {
    auto dam = base_t::composite_da_multiplier( s );

    if ( p()->talents_enabled( tariq_t::THE_MOTHERLOAD ) && hit_players.size() > 1 )
    {
      dam *= 1.0 + p()->talents.the_motherload_increase_per_enemy * ( hit_players.size() - 1 );
    }

    return dam;
  }

  double composite_persistent_multiplier( const action_state_t* s ) const override
  {
    auto pm = base_t::composite_persistent_multiplier( s );

    if ( p()->buffs.thundering_vortex->check() >= p()->legendary.thundering_vortex_needed )
    {
      pm *= p()->legendary.thundering_vortex_multiplier;
    }

    return pm;
  }

  double calculate_direct_amount( action_state_t* s ) const override
  {
    if ( p()->talents_enabled( tariq_t::THE_MOTHERLOAD ) )
      set_bit_mask( s->target );

    return base_t::calculate_direct_amount( s );
  }

  void impact( action_state_t* s ) override
  {
    base_t::impact( s );

    if ( p()->legendary.thundering_vortex )
      p()->buffs.thundering_vortex->trigger();
  }

  size_t available_targets( std::vector<player_t*>& tl ) const override
  {
    tl.clear();
    tl.reserve( std::max<size_t>( n_targets(), sim->target_non_sleeping_list.size() ) );

    tl.push_back( target );

    target_list_end = tl.end();

    auto start = tl.begin();

    for ( auto* t : sim->target_non_sleeping_list )
    {
      if ( t->is_enemy() && ( t != target ) )
      {
        tl.push_back( t );
      }
    }

    while ( tl.size() < n_targets() )
    {
      target_list_end = tl.end();

      for ( auto* t : sim->target_non_sleeping_list )
      {
        if ( t->is_enemy() )
        {
          tl.push_back( t );
        }
      }
    }

    if ( target_list_end < tl.end() )
    {
      rng().shuffle( target_list_end, tl.end() );
    }

    return tl.size();
  }

  void execute() override
  {
    if ( p()->talents_enabled( tariq_t::THE_MOTHERLOAD ) )
      clear_masks();

    auto thunder_vortex_threshold = p()->buffs.thundering_vortex->check() >= p()->legendary.thundering_vortex_needed;

    if ( target_cache.is_valid )
      rng().shuffle( target_list_end, target_cache.list.end() );

    base_t::execute();

    if ( thunder_vortex_threshold )
      p()->buffs.thundering_vortex->decrement( p()->legendary.thundering_vortex_needed );
  }
};

struct focused_wrath_t : public tariq_spell_t
{
  focused_wrath_t( util::string_view name, tariq_t* p, util::string_view options_str = {} )
    : tariq_spell_t( name, p, options_str )
  {
    id = 10;

    name_str_reporting = "Focused Wrath";

    trigger_gcd = timespan_t::zero();

    cooldown->duration = p->spell_const.focused_wrath_cooldown;
    cooldown->hasted   = false;
    cooldown->charges  = 1;

    ability_flags |= ability_type_e::ABILITY_MAJOR;
  }

  void execute() override
  {
    base_t::execute();
    p()->buffs.focused_wrath->trigger( p()->spell_const.focused_wrath_stacks );
  }
};

struct leap_smash_t : public tariq_attack_t
{
  leap_smash_t( util::string_view name, tariq_t* p, util::string_view options_str = {} )
    : tariq_attack_t( name, p, options_str )
  {
    id = 11;

    name_str_reporting = "Leap Smash";

    trigger_gcd = timespan_t::zero();

    cooldown->duration = p->spell_const.leap_smash_cooldown;
    cooldown->hasted   = true;
    cooldown->charges  = 1;

    ability_flags |= ability_type_e::ABILITY_MOVEMENT;

    attack_power_mod.direct = p->spell_const.leap_smash_coeff;
    aoe                     = -1;
    reduced_aoe_targets     = p->spell_const.leap_smash_threshold;

    if ( p->talents_enabled( tariq_t::HIGH_ROAD ) )
    {
      energize_resource = RESOURCE_FURY;
      energize_amount   = p->talents.high_road_leap_smash_fury;
      energize_type     = action_energize::ON_CAST;
    }
  }

  void impact( action_state_t* s ) override
  {
    base_t::impact( s );

    if ( p()->legendary.slayers_mosh )
    {
      p()->get_target_data( s->target )->debuffs.slayers_mosh->trigger();
    }
  }

  void execute() override
  {
    base_t::execute();

    if ( hit_any_target && p()->talents_enabled( tariq_t::MOUTH_FOR_WAR ) )
    {
      p()->buffs.focused_wrath->trigger( p()->talents.mouth_for_war_focused_wrath_stacks );
    }
  }
};

struct raging_currents_t : public tariq_lightning_attack_t
{
  raging_currents_t( util::string_view name, tariq_t* p ) : tariq_lightning_attack_t( name, p )
  {
    id = 12;

    background = true;

    ability_flags |= ability_type_e::ABILITY_SPIRIT;
    attack_power_mod.direct = p->spell_const.raging_tempest_pulse_coeff;

    name_str_reporting = "Raging Currents";
  }

  void execute()
  {
    auto& tl = target_list();
    rng().shuffle( tl.begin(), tl.end() );
    base_t::execute();    
  }
};

struct raging_tempest_t : public tariq_lightning_attack_t
{
  raging_tempest_t( util::string_view name, tariq_t* p, util::string_view options_str = {} )
    : tariq_lightning_attack_t( name, p, options_str )
  {
    id = 12;

    name_str_reporting = "Raging Tempest";

    trigger_gcd = timespan_t::zero();

    ability_flags |= ability_type_e::ABILITY_SPIRIT;

    attack_power_mod.direct = p->spell_const.raging_tempest_impact_coeff;
    aoe                     = -1;
    reduced_aoe_targets     = p->spell_const.raging_tempest_impact_threshold;

    resource_current              = RESOURCE_SPIRIT;
    base_costs[ RESOURCE_SPIRIT ] = 100;
  }

  void init_finished() override
  {
    base_t::init_finished();
    add_child( p()->actions.raging_currents );
  }

  void execute() override
  {
    base_t::execute();

    p()->fs_buffs.spirit_of_heroism->trigger();
    p()->buffs.raging_currents->trigger();
    p()->used_ultimate();
  }
};

}  // namespace actions

tariq_td_t::tariq_td_t( player_t* target, tariq_t* source ) : fellowship::fs_player_td_t( target, source ), debuffs()
{
  debuffs.slayers_mosh = make_buff( *this, "slayers_mosh" )
                             ->set_duration( source->legendary.slayers_mosh_duration )
                             ->set_default_value( source->legendary.slayers_mosh_multiplier - 1 );
}

// tariq_t::composite_attribute_multiplier ==================================

double tariq_t::composite_attribute_multiplier( attribute_e a ) const
{
  double am = fs_player_t::composite_attribute_multiplier( a );

  return am;
}

// tariq_t::composite_melee_auto_attack_speed ===============================

double tariq_t::composite_melee_auto_attack_speed() const
{
  double h = fs_player_t::composite_melee_auto_attack_speed();

  return h;
}

// tariq_t::composite_melee_haste ==========================================

double tariq_t::composite_melee_haste() const
{
  double h = fs_player_t::composite_melee_haste();

  return h;
}

// tariq_t::composite_spell_haste ==========================================

double tariq_t::composite_spell_haste() const
{
  double h = fs_player_t::composite_spell_haste();

  return h;
}

// tariq_t::composite_melee_crit_chance =========================================

double tariq_t::composite_melee_crit_chance() const
{
  double crit = fs_player_t::composite_melee_crit_chance();

  return crit;
}

// tariq_t::composite_spell_crit_chance =========================================

double tariq_t::composite_spell_crit_chance() const
{
  double crit = fs_player_t::composite_spell_crit_chance();

  return crit;
}

// tariq_t::composite_damage_versatility ===================================

double tariq_t::composite_damage_versatility() const
{
  double cdv = fs_player_t::composite_damage_versatility();

  return cdv;
}

// tariq_t::composite_heal_versatility =====================================

double tariq_t::composite_heal_versatility() const
{
  double chv = fs_player_t::composite_heal_versatility();

  return chv;
}

// tariq_t::composite_leech ===============================================

double tariq_t::composite_leech() const
{
  double l = fs_player_t::composite_leech();

  return l;
}

// tariq_t::matching_gear_multiplier ========================================

double tariq_t::matching_gear_multiplier( attribute_e attr ) const
{
  return 0.0;
}

// tariq_t::composite_player_multiplier =====================================

double tariq_t::composite_player_multiplier( school_e school ) const
{
  double m = fs_player_t::composite_player_multiplier( school );

  //m *= 1.0 + buffs.harvesters_toll->check_stack_value();

  return m;
}

// tariq_t::composite_player_pet_damage_multiplier ==========================

double tariq_t::composite_player_pet_damage_multiplier( const action_state_t* s, bool guardian ) const
{
  double m = fs_player_t::composite_player_pet_damage_multiplier( s, guardian );

  return m;
}

// tariq_t::composite_player_target_multiplier ==============================

double tariq_t::composite_player_target_multiplier( player_t* target, school_e school ) const
{
  double m = fs_player_t::composite_player_target_multiplier( target, school );

  m *= 1.0 + get_target_data( target )->debuffs.slayers_mosh->check_value();

  return m;
}

// tariq_t::composite_player_target_crit_chance =============================

double tariq_t::composite_player_target_crit_chance( player_t* target ) const
{
  double c = fs_player_t::composite_player_target_crit_chance( target );

  return c;
}

// tariq_t::composite_player_target_armor ===================================

double tariq_t::composite_player_target_armor( player_t* target ) const
{
  return 0.0;

  double a = fs_player_t::composite_player_target_armor( target );

  return a;
}
// tariq_t::init_actions ====================================================

void tariq_t::init_action_list()
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

// tariq_t::create_action  ==================================================

action_t* tariq_t::create_action( util::string_view name, util::string_view options_str )
{
  using namespace actions;

  if ( name == "wild_swing" )
    return new wild_swing_t( name, this, options_str );
  if ( name == "face_breaker" )
    return new face_breaker_t( name, this, options_str );
  if ( name == "heavy_strike" )
    return new heavy_strike_t( name, this, options_str );
  if ( name == "skull_crusher" )
    return new skull_crusher_t( name, this, options_str );
  if ( name == "hammer_storm" )
    return new hammer_storm_t( name, this, options_str );
  if ( name == "culling_strike" )
    return new culling_strike_t( name, this, options_str );
  if ( name == "thunder_call" )
    return new thunder_call_t( name, this, options_str );
  if ( name == "chain_lightning" )
    return new chain_lightning_t( name, this, options_str );
  if ( name == "focused_wrath" )
    return new focused_wrath_t( name, this, options_str );
  if ( name == "leap_smash" )
    return new leap_smash_t( name, this, options_str );
  if ( name == "raging_tempest" )
    return new raging_tempest_t( name, this, options_str );

  return fs_player_t::create_action( name, options_str );
}

// tariq_t::create_expression ===============================================

std::unique_ptr<expr_t> tariq_t::create_action_expression( action_t& action, std::string_view name_str )
{
  auto split = util::string_split<util::string_view>( name_str, "." );

  return fs_player_t::create_action_expression( action, name_str );
}

std::unique_ptr<expr_t> tariq_t::create_expression( util::string_view name_str )
{
  auto split = util::string_split<util::string_view>( name_str, "." );

  if ( util::str_compare_ci( split[ 0 ], "legendary" ) )
  {
    if ( split.size() == 2 )
    {
      if ( util::str_compare_ci( split[ 1 ], "thundering_vortex" ) )
      {
        return make_ref_expr( name_str, legendary.thundering_vortex );
      }
      else if ( util::str_compare_ci( split[ 1 ], "executioners_grin" ) )
      {
        return make_ref_expr( name_str, legendary.executioners_grin );
      }
      else if ( util::str_compare_ci( split[ 1 ], "slayers_mosh" ) )
      {
        return make_ref_expr( name_str, legendary.slayers_mosh );
      }
    }
  }
  else if ( util::str_compare_ci( split[ 0 ], "talent" ) )
  {
    if ( split.size() == 2 )
    {
      for ( tariq_talents_t t = static_cast<tariq_talents_t>( 1U ); t < tariq_talents_t::MAX; t++ )
      {
        if ( util::str_compare_ci( split[ 1 ], talent_name( t ) ) )
        {
          return make_fn_expr( name_str, std::bind( std::mem_fn( &tariq_t::talents_enabled ), this, t ) );
        }
      }
    }
  }
  // Split expressions

  return fs_player_t::create_expression( name_str );
}

std::unique_ptr<expr_t> tariq_t::create_resource_expression( util::string_view name_str )
{
  return fs_player_t::create_resource_expression( name_str );
}

// tariq_t::init_base =======================================================

void tariq_t::init_base_stats()
{
  if ( base.distance < 1 )
    base.distance = 5;

  fs_player_t::init_base_stats();

  base.stats.attribute[ STAT_STRENGTH ] = 100;
  resources.base[ RESOURCE_HEALTH ]     = 1811;

  base.health_per_stamina = 54.459;

  base_gcd = timespan_t::from_seconds( 1.5 );
  min_gcd  = timespan_t::from_seconds( 0 );
  //min_gcd  = timespan_t::from_seconds( 0.75 );

  base.parry = 0.05;
  base.dodge = 0.05;

  resources.base[ RESOURCE_FURY ] = 100;

  if ( talents_enabled( tariq_t::BLOODLINE ) )
  {
    resources.base_regen_per_second[ RESOURCE_FURY ] =
        talents.bloodline_fury_amount / talents.bloodline_fury_period.total_seconds();
  }

  if ( talents_enabled( tariq_t::PNEUMA ) )
  {
    resources.start_at[ RESOURCE_SPIRIT ] += talents.pneuma_start_spirit;
  }
}

// tariq_t::init_spells =====================================================

void tariq_t::init_spells()
{
  fs_player_t::init_spells();

  //actions.auto_attack = new actions::auto_melee_attack_t( this, "" );
}

// tariq_t::init_gains ======================================================

void tariq_t::init_gains()
{
  fs_player_t::init_gains();

  gains.spirit_procs = get_gain( "Spirit Procs" );
}

// tariq_t::init_procs ======================================================

void tariq_t::init_procs()
{
  fs_player_t::init_procs();
  procs.ace_of_spades        = get_proc( "Ace of Spades" );
  procs.blood_and_thunder    = get_proc( "Blood and Thunder" );
  procs.kill_em_all          = get_proc( "Kill em All" );
  procs.schism_hammer_storm  = get_proc( "Schism Hammer Storm" );
  procs.schism_skull_crusher = get_proc( "Schism Skull Crusher" );
  procs.them_bones           = get_proc( "Them Bones" );
  procs.executioners_grin    = get_proc( "Executioners Grin" );
}

// tariq_t::init_scaling ====================================================

void tariq_t::init_scaling()
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

// tariq_t::init_resources =================================================

void tariq_t::init_resources( bool force )
{
  fs_player_t::init_resources( force );
}

// tariq_t::init_buffs ======================================================

void tariq_t::create_buffs()
{
  fs_player_t::create_buffs();

  buffs.executioners_grin =
      make_buff<tariq_buff_t>( this, "executioners_grin" )->set_duration( legendary.executioners_grin_duration );

  buffs.thunder_call =
      make_buff<tariq_buff_t>( this, "thunder_call" )->set_duration( spell_const.thunder_call_duration );

  buffs.raging_currents = make_buff<tariq_buff_t>( this, "raging_currents" )
                              ->set_duration( spell_const.raging_tempest_currents_duration )
                              ->set_default_value( spell_const.raging_tempest_currents_expertise_per_stack )
                              ->set_pct_buff_type( STAT_PCT_BUFF_VERSATILITY )
                              ->set_period( spell_const.raging_tempest_pulse_period )
                              ->set_tick_time_behavior( buff_tick_time_behavior::HASTED )
                              ->set_max_stack( spell_const.raging_tempest_currents_max_stack )
                              ->set_tick_callback( [ this ]( buff_t* buff, int current_tick, timespan_t tick_time ) {
                                actions.raging_currents->execute();
                              } );

  buffs.ride_the_lightning =
      make_buff<tariq_buff_t>( this, "raging_currents" )
          ->set_duration( talents.ride_the_lightning_duration )
          ->set_default_value( talents.ride_the_lightning_haste )
          ->set_pct_buff_type( STAT_PCT_BUFF_HASTE )
          ->set_period( talents.ride_the_lightning_period )
          ->set_freeze_stacks( true )
          ->set_tick_callback( [ this ]( buff_t* buff, int current_tick, timespan_t tick_time ) {
            resource_gain( RESOURCE_FURY, talents.ride_the_lightning_fury_per_tick, gains.ride_the_lightning );
          } );

  buffs.kill_em_all = make_buff<tariq_buff_t>( this, "kill_em_all" )
                          ->set_duration( talents.kill_em_all_duration )
                          ->set_default_value( talents.kill_em_all_dmg_multiplier - 1 )
                          ->set_max_stack( talents.kill_em_all_stacks )
                          ->add_stack_change_callback(
                              [ this ]( auto, auto, auto ) { cooldowns.heavy_strike->adjust_recharge_multiplier(); } );

  buffs.focused_wrath = make_buff<tariq_buff_t>( this, "focused_wrath" )
                            ->set_duration( spell_const.focused_wrath_duration )
                            ->set_max_stack( spell_const.focused_wrath_max_stacks );

  buffs.square_hammer_stacking = make_buff<tariq_buff_t>( this, "square_hammer_stacking" )
                                     ->set_duration( talents.square_hammer_duration )
                                     ->set_max_stack( talents.square_hammer_max_stacks )
                                     ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

  buffs.square_hammer_buff = make_buff<tariq_buff_t>( this, "square_hammer_buff" )
                                 ->set_duration( talents.square_hammer_expertise_duration )
                                 //->set_max_stack( talents.square_hammer_max_stacks )
                                 ->set_max_stack( 1 )
                                 ->set_default_value( talents.square_hammer_expertise_per_stack )
                                 ->set_pct_buff_type( STAT_PCT_BUFF_VERSATILITY )
                                 ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

  buffs.far_beyond_driven = make_buff<tariq_buff_t>( this, "far_beyond_driven" )
                                ->set_duration( talents.far_beyond_driven_duration )
                                ->set_max_stack( talents.far_beyond_driven_max_stacks )
                                ->set_default_value( talents.far_beyond_driven_spirit_per_stack )
                                ->set_pct_buff_type( STAT_PCT_BUFF_MASTERY )
                                ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

  buffs.schism_hammer_storm = make_buff<tariq_buff_t>( this, "schism_hammer_storm" )
                                  ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT )
                                  ->set_default_value( talents.schism_damage_hammer_storm_multiplier - 1 );

  buffs.schism_skull_crusher = make_buff<tariq_buff_t>( this, "schism_skull_crusher" )
                                   ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT )
                                   ->set_default_value( talents.schism_damage_skull_crusher_multiplier - 1 );
  buffs.thundering_vortex = make_buff<tariq_buff_t>( this, "thundering_vortex" )
                                ->set_max_stack( legendary.thundering_vortex_needed * 2 )
                                ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT )
                                ->set_default_value( legendary.thundering_vortex_multiplier - 1 );
}

// tariq_t::invalidate_cache =========================================

void tariq_t::invalidate_cache( cache_e c )
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

void tariq_t::create_options()
{
  fs_player_t::create_options();

  add_option( opt_bool( "legendary.slayers_mosh", legendary.slayers_mosh ) );
  add_option( opt_bool( "legendary.thundering_vortex", legendary.thundering_vortex ) );
  add_option( opt_bool( "legendary.executioners_grin", legendary.executioners_grin ) );
}

// tariq_t::copy_from =======================================================

void tariq_t::copy_from( player_t* source )
{
  tariq_t* tariq = static_cast<tariq_t*>( source );
  fs_player_t::copy_from( source );

  talents     = tariq->talents;
  legendary   = tariq->legendary;
  options     = tariq->options;
  spell_const = tariq->spell_const;
}

// tariq_t::create_profile  =================================================

std::string tariq_t::create_profile( save_e stype )
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

// tariq_t::init_items ======================================================

void tariq_t::init_items()
{
  fs_player_t::init_items();
}

// tariq_t::init_special_effects ============================================

void tariq_t::init_special_effects()
{
  fs_player_t::init_special_effects();
}

// tariq_t::init_finished ===================================================

void tariq_t::init_finished()
{
  fs_player_t::init_finished();
}

void tariq_t::init_talents()
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

    for ( tariq_talents_t t = static_cast<tariq_talents_t>( 1U ); t < tariq_talents_t::MAX; t++ )
    {
      if ( util::str_compare_ci( talent_split[ 0 ], talent_name( t ) ) )
      {
        set_talent_points( t, ranks >= 1 );
        break;
      }
    }
  }
}

void tariq_t::init_background_actions()
{
  fs_player_t::init_background_actions();

  actions.chain_lightning_ace_of_spades_spin  = new actions::chain_lightning_t( "chain_lightning_ace_of_spades_spin", this, {}, secondary_trigger::ACE_OF_SPADES );
  actions.chain_lightning_ace_of_spades_crush = new actions::chain_lightning_t( "chain_lightning_ace_of_spades_crush", this, {}, secondary_trigger::ACE_OF_SPADES );
  
  actions.chain_lightning_ace_of_spades_spin->name_str_reporting  = "Chain Lightning (Ace of Spades)";
  actions.chain_lightning_ace_of_spades_crush->name_str_reporting = "Chain Lightning (Ace of Spades)";


  actions.chain_lightning_blood_and_thunder = new actions::chain_lightning_t( "chain_lightning_blood_and_thunder", this, {}, secondary_trigger::BLOOD_AND_THUNDER );
  actions.chain_lightning_blood_and_thunder->name_str_reporting = "Chain Lightning (Blood and Thunder)";
  actions.raging_currents = new actions::raging_currents_t( "raging_currents", this );

  //actions.rend                               = new actions::rend_t( this );
  //actions.slaughter                          = new actions::slaughter_dot_t( this );
  //actions.bloodcraze                         = new actions::bloodcraze_t( this );
  //actions.ravens_precision                   = new actions::ravens_precision_t( this );
  //actions.heart_splitter                     = new actions::heart_splitter_t( "heart_splitter_repeat", this );
  //actions.heart_splitter->name_str_reporting = "Heart Splitter (Legendary)";
  //actions.heart_splitter->background         = true;
  //actions.heart_splitter->cooldown->duration = 0_s;
}

void tariq_t::init_rng()
{
  fs_player_t::init_rng();

  rng_objects.them_bones           = get_accumulated_rng( "them_bones",             rng::CfromP( talents.them_bones_chance ) );
  rng_objects.schism_skull_crusher = get_accumulated_rng( "schism_skull_crusher",   rng::CfromP( talents.schism_proc_chance ) );
  rng_objects.schism_hammer_storm  = get_accumulated_rng( "schism_hammer_storm",    rng::CfromP( talents.schism_proc_chance ) );
  rng_objects.kill_em_all          = get_accumulated_rng( "kill_em_all",            rng::CfromP( talents.kill_em_all_chance ) );
  rng_objects.blood_and_thunder    = get_accumulated_rng( "blood_and_thunder",      rng::CfromP( talents.blood_and_thunder_chance ) );
  rng_objects.executioners_grin    = get_accumulated_rng( "executioners_grin",      rng::CfromP( legendary.executioners_grin_chance ) );
  
  rng_objects.ace_of_spades = get_rppm( "ace_of_spades", talents.ace_of_spades_ppm, 1.0, RPPM_HASTE );
}

// tariq_t::reset ===========================================================

void tariq_t::reset()
{
  fs_player_t::reset();
}

// tariq_t::activate ========================================================

void tariq_t::activate()
{
  fs_player_t::activate();
}

// tariq_t::cancel_auto_attack ==============================================

void tariq_t::cancel_auto_attacks()
{
  if ( actions.melee_hit && actions.melee_hit->execute_event )
  {
    actions.melee_hit->canceled            = true;
    actions.melee_hit->prev_scheduled_time = actions.melee_hit->execute_event->occurs();
  }

  fs_player_t::cancel_auto_attacks();
}

// tariq_t::arise ===========================================================

void tariq_t::arise()
{
  fs_player_t::arise();
  sim->print_debug( "{} arises. Current max hp is: {}, current is: {}, base: {}, base_mul: {}, init_mul: {}", *this,
                    resources.max[ RESOURCE_HEALTH ], resources.current[ RESOURCE_HEALTH ],
                    resources.base[ RESOURCE_HEALTH ], resources.base_multiplier[ RESOURCE_HEALTH ],
                    resources.initial_multiplier[ RESOURCE_HEALTH ] );
}

// tariq_t::combat_begin ====================================================

void tariq_t::combat_begin()
{
  fs_player_t::combat_begin();
}

// tariq_t::energy_regen_per_second =========================================

double tariq_t::resource_regen_per_second( resource_e r ) const
{
  double reg = fs_player_t::resource_regen_per_second( r );

  return reg;
}

double tariq_t::resource_gain( resource_e resource_type, double amount, gain_t* source, action_t* action )
{
  double actual_amount = fs_player_t::resource_gain( resource_type, amount, source, action );

  return actual_amount;
}

// tariq_t::non_stacking_movement_modifier ==================================

double tariq_t::non_stacking_movement_modifier() const
{
  double ms = fs_player_t::non_stacking_movement_modifier();

  return ms;
}

// tariq_t::stacking_movement_modifier===================================

double tariq_t::stacking_movement_modifier() const
{
  double ms = fs_player_t::stacking_movement_modifier();

  return ms;
}

// tariq_t::regen ===========================================================

void tariq_t::regen( timespan_t periodicity )
{
  fs_player_t::regen( periodicity );
}

template <typename Base>
void actions::tariq_action_t<Base>::trigger_auto_attack( const action_state_t* /* state */ )
{
  //if ( !p()->main_hand_attack || p()->main_hand_attack->execute_event )
  //  return;

  //p()->actions.auto_attack->schedule_execute();
}

template <typename Base>
void actions::tariq_action_t<Base>::trigger_spirit_refund( const action_state_t* state )
{
  double fury_restored = ab::last_resource_cost;

  make_event( ab::sim, 200_ms, [ fury_restored, this ] {
    p()->resource_gain( RESOURCE_FURY, fury_restored, p()->gains.spirit_procs, this );
  } );

  p()->spirit_refund();
}

// tariq_t::convert_hybrid_stat ==============================================

stat_e tariq_t::convert_hybrid_stat( stat_e s ) const
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

void tariq_t::create_cooldowns()
{
  cooldowns.heavy_strike = get_cooldown( "heavy_strike" );
  cooldowns.leap_smash   = get_cooldown( "leap_smash" );
  cooldowns.thunder_call = get_cooldown( "thunder_call" );
}

class tariq_module_t : public module_t
{
public:
  tariq_module_t() : module_t( TARIQ )
  {
  }

  player_t* create_player( sim_t* sim, util::string_view name, race_e r = RACE_NONE ) const override
  {
    return new tariq_t( sim, name, r );
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

}  // namespace tariq
}  // namespace fellowship

const module_t* module_t::tariq()
{
  static fellowship::tariq::tariq_module_t m;
  return &m;
}
