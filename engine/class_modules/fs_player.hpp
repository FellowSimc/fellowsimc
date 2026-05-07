#pragma once
#include "util/util.hpp"

#include "simulationcraft.hpp"

namespace fellowship
{  // UNNAMED NAMESPACE

const double GEM_TIER_1 = 120.0;
const double GEM_TIER_2 = 240.0;
const double GEM_TIER_3 = 480.0;
const double GEM_TIER_4 = 720.0;
const double GEM_TIER_5 = 960.0;
const double GEM_TIER_6 = 1200.0;
const double GEM_TIER_7 = 1560.0;
const double GEM_TIER_8 = 1920.0;
const double GEM_TIER_9 = 2280.0;
const double GEM_TIER_10 = 2640.0;

// Forward Declarations
class fs_player_t;

namespace actions
{
struct fs_player_attack_t;
struct fs_player_heal_t;
struct fs_player_spell_t;

struct melee_t;
}  // namespace actions

class fs_player_td_t : public actor_target_data_t
{

public:
  struct dots_t
  {
    dot_t* curse_of_anzhyr;
    dot_t* kindling;
    dot_t* amethyst_splinters;
  } fs_dots;

  struct debuffs_t
  {
    buff_t* triggered_first_strike;
    buff_t* diamond_strike_amp;
    buff_t* voidbringer_debuff;
    buff_t* aurastone_accumulator;
  } debuffs;

  struct buffs_t
  {
    buff_t* inspired_allegiance;
  } buffs;

  fs_player_td_t( player_t* target, fs_player_t* source );
};

struct fs_player_buff_t : public buff_t
{
  fs_player_buff_t( player_t* p, util::string_view name ) : buff_t( p->sim, p, p, name, spell_data_t::nil(), nullptr )
  {
  }

  fs_player_buff_t( player_t* t, player_t* p, util::string_view name )
    : buff_t( p->sim, t, p, name, spell_data_t::nil(), nullptr )
  {
  }

  fs_player_t* p()
  {
    return debug_cast<fs_player_t*>( source );
  }

  const fs_player_t* p() const
  {
    return debug_cast<const fs_player_t*>( source );
  }

  // Used by underhanded upper hand, it's not a *real* pause, but rather an application of a 100x slowdown via time mod
  fs_player_buff_t* pause()
  {
    set_dynamic_time_duration_multiplier( 100.0 );
    return this;
  }

  fs_player_buff_t* unpause()
  {
    set_dynamic_time_duration_multiplier( 1.0 );
    return this;
  }
};

class fs_player_t : public player_t
{
public:

  struct fs_actions_t
  {
    action_t* amethyst_splinters;
    action_t* voidbringer_dmg;
    action_t* aurastone_dmg;
    action_t* finesse_f;
    action_t* finesse_l;
  } fs_actions;

  struct fs_buffs_t
  {
    buff_t* spirit_of_heroism;
    buff_t* ancestral_surge;
    buff_t* first_strike;
    buff_t* virtuoso;
    buff_t* adrenaline_rush;
    buff_t* might_of_the_minotaur;
    buff_t* fated_strike;
    buff_t* chronoshift;
    buff_t* drakheims_absolution;
    buff_t* dark_prophecy;
    buff_t* draconic_might;
    buff_t* willful_momentum;
    buff_t* vengeful_soul;
    buff_t* seized_opportunity_stacking;
    buff_t* seized_opportunity;
    buff_t* martial_initiative;
    buff_t* hidden_power_stacking;
    buff_t* hidden_power;
    buff_t* sundering_wrath;
    buff_t* harmonious_soul;
    buff_t* finesse_a;
    buff_t* finesse_b;
    buff_t* finesse_g;
    buff_t* finesse_i;
    buff_t* finesse_l;
    buff_t* finesse_k;
  } fs_buffs;

  struct rng_objects_t
  {
    accumulated_rng_t* finesse_d;
    accumulated_rng_t* finesse_f;
  } fs_rng_objects;

  struct fs_cooldowns_t
  {
  } fs_cooldowns;

  struct fs_gains_t
  {
    gain_t* grandeur;
    gain_t* spirit_procs;
    gain_t* finesse_d;
  } fs_gains;

  struct options_t
  {
  } fs_options;

  struct fs_sets_t
  {
    // Tuzari
    bool dark_prophecy                = false;
    double dark_prophecy_ppm          = 0.8;
    double dark_prophecy_haste        = 0.25;
    timespan_t dark_prophecy_duration = 20_s;
    timespan_t dark_prophecy_cooldown = 5_s;

    bool deaths_grasp              = false;
    double deaths_grasp_spirit     = 0.03;
    double death_grasp_execute_amp = 0.15;

    bool draconic_might                = false;
    double draconic_might_ppm          = 1.0;
    double draconic_might_amp          = 0.18;
    timespan_t draconic_might_duration = 14_s;
    timespan_t draconic_might_cooldown  = 5_s;

    bool drakheims_absolution                = false;
    double drakheims_absolution_amp          = 0.20;
    timespan_t drakheims_absolution_duration = 20_s;

    bool eldrin_deceit                       = false;
    double eldrin_deceit_crit                = 0.03;
    // Threat Reduction

    bool eldrin_fury        = false;
    double eldrin_fury_crit = 0.03;
    // Threat Generation

    bool haunting_lament                 = false;
    double haunting_lament_spirit        = 0.03;
    double haunting_lament_max_mana      = 0.15;

    bool sin_warding             = false;
    double sin_warding_expertise = 0.03;
    double sin_warding_max_hp    = 0.05;

    bool sintharas_veil                  = false;
    double sintharas_veil_spirit         = 0.03;
    double sintharas_veil_magic_dr       = 0.1;

    bool torment_of_baelaurum            = false;
    double torment_of_baelaurum_amp      = 0.04;
    double torment_of_baelaurum_heal_pct = 0.35;

    bool tuzari_grace                    = false;
    double tuzari_grace_haste            = 0.03;
    double tuzari_grace_movement_speed   = 0.2;
  } fs_sets;

  struct fs_gems_t
  {
    std::array<double, GEM_MAX> gem_powers;

    timespan_t harmonious_duration = 5_s;
    int harmonious_max_stacks      = 10;
    double harmonious_diamond_amp  = 0.35;
    double harmonious_buff_minor   = 0.003;
    double harmonious_buff_major   = 0.009;

    double sapphire_additional_max_spirit_minor  = 10.0;
    double sapphire_additional_max_spirit_major  = 30.0;
    double sapphire_spirit_cost_multiplier_minor = 0.95;
    double sapphire_spirit_cost_multiplier_major = 0.85;
  } fs_gems;

  std::array<unsigned short, FINESSE_MAX + 1> finesse_traits = {};

  struct finesse_values_t
  {
    const double finesse_a_per_stack[ 5 ] = { 0, 0.012, 0.02, 0.03, 0.05 };
    const int finesse_a_max_stacks        = 5;

    const timespan_t finesse_b_duration = 4_s;
    const double finesse_b_crit[ 5 ]    = { 0, 0.02, 0.03, 0.05, 0.08 };

    const timespan_t finesse_c_duration[ 5 ] = { 0_s, 0.5_s, 1_s, 2_s, 3_s };
    const timespan_t finesse_c_divisor       = 60_s;

    const double finesse_d_chance[ 5 ]   = { 0, 0.05, 0.08, 0.13, 0.2 };
    const double finesse_d_spirit_points = 1.0;

    const double finesse_e_cdmg[ 5 ] = { 0, 0.01, 0.02, 0.03, 0.04 };

    const double finesse_f_drain[ 5 ]   = { 0, 2.6, 4.16, 6.66, 10.65 };
    const double finesse_f_drain_chance = 0.1;

    const double finesse_g_spirit_to_stats[ 5 ] = { 0.0, 0.25, 0.25, 0.25, 0.25 };
    const timespan_t finesse_g_duration[ 5 ]    = { 0_s, 3_s, 5_s, 8_s, 12_s };

    const double finesse_h_added[ 5 ] = { 0, 0.12, 0.19, 0.31, 0.49 };

    const double finesse_i_haste[ 5 ]   = { 0, 0.03, 0.048, 0.077, 0.123 };
    const timespan_t finesse_i_interval = 30_s;
    const timespan_t finesse_i_duration = 7_s;
    const timespan_t finesse_i_cdr      = 2_s;

    const double finesse_j_amp[ 5 ] = { 0, 0.005, 0.008, 0.013, 0.02 };
    const double finesse_j_divisor  = 0.03;

    const double finesse_k_cdr[ 5 ]         = { 0, 0.02, 0.03, 0.05, 0.08 };
    const double finesse_k_amp_multiplier   = 1.0;
    const timespan_t finesse_k_amp_duration = 5_s;

    const double finesse_l_dmg[ 5 ]     = { 0, 0.68, 1.09, 1.74, 2.79 };
    const double finesse_l_heal[ 5 ]    = { 0, 0.87, 1.39, 2.23, 3.56 };
    const int finesse_l_targets         = 4;
    const int finesse_l_max_stacks      = 1;
    const timespan_t finesse_l_duration = 15_s;
  } finesse_trait_values;

  struct fs_weapons_t
  {
    fsweapon_e equipped_weapon = fsweapon_e::FSWEAPON_NONE;

    std::array<unsigned short, WEAPON_TRAIT_MAX> weapon_traits = {};

    // You know I should have made these into an enum and an array shouldn't I...
    unsigned amethyst_splinters               = 0;
    unsigned brave_machinations               = 0;
    unsigned diamond_strike                   = 0;
    unsigned divine_mediation                 = 0;
    unsigned emerald_judgement                = 0;
    unsigned first_man_standing               = 0;
    unsigned grounded_spirit                  = 0;
    unsigned heart_of_stone                   = 0;
    unsigned heroic_brand                     = 0;
    unsigned hidden_power                     = 0;
    unsigned hunters_focus                    = 0;
    unsigned inspired_allegiance              = 0;
    unsigned iron_spikes                      = 0;
    unsigned kindling                         = 0;
    unsigned king_of_the_hill                 = 0;
    unsigned latent_resurgence                = 0;
    unsigned martial_initiative               = 0;
    unsigned navigators_intuition             = 0;
    unsigned patient_soul                     = 0;
    unsigned ruby_storm                       = 0;
    unsigned sapphire_aurastone               = 0;
    unsigned seized_opportunity               = 0;
    unsigned stalwart_readiness               = 0;
    unsigned treasure_hunters_delight         = 0;
    unsigned vengeful_soul                    = 0;
    unsigned visions_of_grandeur              = 0;
    unsigned willful_momentum                 = 0;
  } fs_weapons;

  
  struct fs_relic_t
  {
    fsrelic_e relic1 = fsrelic_e::FSRELIC_NONE;
    fsrelic_e relic2 = fsrelic_e::FSRELIC_NONE;
  } fs_relics;

  struct fs_relic_values_t
  {
    const double alzeracs_mana_pct = 0.4;
    timespan_t alzeracs_cd         = 120_s;
  } fs_relic_values;

  struct fs_weapon_values_t
  {
    const double voidbringer_cap          = 42.5;
    const double voidbringer_acc          = 0.1;
    const timespan_t voidbringer_duration = 15_s;

    const timespan_t earthbreaker_cooldown        = 180_s;
    const double earthbreaker_initial_hit_coeff   = 8.767;
    const double earthbreaker_repeating_hit_coeff = 1.332;
    const double earthbreaker_final_hit_coeff     = 12.66;
    const double earthbreaker_target_falloff      = 3;
    const timespan_t earthbreaker_duration        = 10_s;
    const timespan_t earthbreaker_period          = 1.5_s;
  } fs_weapon_values;

  struct fs_weapon_trait_values_t
  {
    const double willful_momentum_spirit[ 5 ]           = { 0, 4, 7, 10, 13 };
    const double willful_momentum_amp[ 5 ]              = { 0, 0.015, 0.026, 0.037, 0.048 };
    const double vengeful_soul_amp[ 5 ]                 = { 0, 0.02, 0.034, 0.048, 0.062 };
    const double seized_opportunity_crit[ 5 ]           = { 0, 7, 13, 19, 25 };
    const double martial_initiative_duration[ 5 ]       = { 0, 0.1, 0.18, 0.25, 0.32 };
    const double kindling_tick_damage[ 5 ]              = { 0, 0.46, 0.84, 1.22, 1.6 };
    const timespan_t inspired_allegiance_cdr[ 5 ]       = { 0_s, 1_s, 2.5_s, 4_s, 5.5_s };
    const int inspired_allegiance_allies[ 5 ]           = { 0, 1, 2, 3, 3 };
    const double inspired_allegiance_haste[ 5 ]         = { 0, 5.0, 8.0, 11.0, 14.0 };
    const double hidden_power_amp[ 5 ]                  = { 0, 0.075, 0.09, 0.105, 0.12 };
    const double emerald_judgement_dmg[ 5 ]             = { 0, 3.0, 5.0, 7.0, 9.0 };
    const double diamond_strike_dmg[ 5 ]                = { 0, 0.72, 1.27, 1.82, 2.37 };
    const double diamond_strike_ppm[ 5 ]                = { 0, 2.5, 3.9, 5.3, 6.7 };
    const double amethyst_splinters_fraction[ 5 ]       = { 0, 0.0355, 0.057, 0.0785, 0.1 };
    const double brave_machinations_crit[ 5 ]           = { 0, 0.095, 0.17, 0.245, 0.32 };
    const double brave_machinations_cdr[ 5 ]            = { 0, 0.15, 0.2, 0.25, 0.3 };
    const double heroic_brand_amp[ 5 ]                  = { 1.0, 1.24, 1.42, 1.6, 1.78 };
    const double ruby_storm_ppm[ 5 ]                    = { 1.3, 1.3, 1.3, 1.3, 1.3 };
    const double ruby_storm_damage[ 5 ]                 = { 0.0, 0.0235, 0.039, 0.0545, 0.07 };
    const double sapphire_aurastone_dmg_acc[ 5 ]        = { 0.0, 0.0345, 0.063, 0.0915, 0.12 };
    const double sapphire_aurastone_heal_acc[ 5 ]       = { 0.0, 0.046, 0.084, 0.122, 0.16 };
    const double sapphire_aurastone_cap[ 5 ]            = { 0.0, 50.0, 50.0, 50.0, 50.0 };
    const timespan_t sapphire_aura_period               = 3_s;
    const double navigators_intuition_stats[ 5 ]        = { 0.0, 14.0, 33.0, 52.0, 71.0 };
    const timespan_t navigators_intuition_duration[ 5 ] = { 0_s, 30_s, 30_s, 30_s, 30_s };
    const timespan_t navigators_intuition_cd[ 5 ]       = { 0_s, 90_s, 90_s, 90_s, 90_s };
    const double navigators_intuition_chance[ 5 ]       = { 0.0, 0.2, 0.2, 0.2, 0.2 };
    const double hunters_focus_haste[ 5 ]               = { 0.0, 2.0, 4.0, 6.0, 8.0 };
    const timespan_t hunters_focus_duration[ 5 ]        = { 0_s, 8_s, 8_s, 8_s, 8_s };
    const int hunters_focus_max_stacks[ 5 ]             = { 5, 5, 5, 5, 5 };
  } fs_weapon_trait_values;

  double spirit_refund_mul = 1;

  cooldown_t* weapon_cd;
  bool brave_machinations_available;

  target_specific_t<fs_player_td_t> target_data;
  std::vector<buff_t*> active_voidbringer_buffs;
  std::vector<buff_t*> active_aurastone_buffs;

  event_t* finesse_i_event;

  virtual const fs_player_td_t* find_target_data( const player_t* target ) const override
  {
    return target_data[ target ];
  }

  virtual fs_player_td_t* get_target_data( player_t* target ) const override
  {
    fs_player_td_t*& td = target_data[ target ];
    if ( !td )
    {
      td = new fs_player_td_t( target, const_cast<fs_player_t*>( this ) );
    }
    return td;
  }

  double composite_mastery_value() const override
  {
    return composite_mastery() / ( 1 + composite_mastery() );
  }

  struct talent_info
  {
    long long flag;
    std::string_view id;
    std::string_view pretty;
  };

  virtual constexpr std::string_view talent_name( long long t )
  {
    return "";
  }

  virtual constexpr std::string_view talent_name_formatted( long long t )
  {
    return "";
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

  void create_buffs() override;
  void create_options() override;

  void copy_from( player_t* source ) override;
  std::string create_profile( save_e stype ) override;
  void init_action_list() override;
  void init_assessors() override;

  void reset() override;
  void activate() override;
  void arise() override;
  void combat_begin() override;
  void finesse_i_event_fn();
  void finesse_i_cdr( timespan_t cdr );

  action_t* create_action( util::string_view name, util::string_view options ) override;

  std::unique_ptr<expr_t> create_action_expression( action_t& action, std::string_view name_str ) override;
  std::unique_ptr<expr_t> create_expression( util::string_view name_str ) override;

  void regen( timespan_t periodicity ) override;

  double composite_attribute_multiplier( attribute_e attr ) const override;
  double composite_player_critical_damage_multiplier( const action_state_t* /* s */ ) const override;
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
  void spirit_refund();
  void used_ultimate();
  void voidbringer_accumulate( double damage );
  void sapphire_aurastone_accumulate( double damage );

  double resource_gain( resource_e r, double amount, gain_t* source = nullptr, action_t* a = nullptr ) override;

  virtual bool talents_enabled( unsigned mask ) const
  {
    return ( talent_points_fs & mask ) == mask;
  }

  virtual bool talent_enabled( unsigned mask ) const
  {
    return ( talent_points_fs & mask );
  }

  void enable_talent_points( unsigned long long mask )
  {
    talent_points_fs |= mask;
    talent_points_fs_count = as<unsigned long>( __popcnt64( talent_points_fs ) );
  }

  void set_talent_points( unsigned long long mask, bool enable )
  {
    if ( enable )
      talent_points_fs |= mask;
    else
      talent_points_fs &= ~mask;
    talent_points_fs_count = as<unsigned long>( __popcnt64( talent_points_fs ) );
  }

  static bool parse_fsweapon( sim_t* sim, std::string_view, std::string_view value )
  {
    fs_player_t* player = static_cast<fs_player_t*>( sim->active_player );
    for ( fsweapon_e weapon = fsweapon_e::FSWEAPON_NONE; weapon < fsweapon_e::FSWEAPON_MAX; weapon++ )
    {
      if ( util::str_compare_ci( value, util::fsweapon_string( weapon ) ) )
      {
        player->fs_weapons.equipped_weapon = weapon;
        return true;
      }
    }

    sim->error( "{} weapon string '{}' not valid.", sim->active_player->name(), value );
    return false;
  }

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

  template <typename CLASS, typename... ARGS>
  action_t* create_fs_proc_action( util::string_view name, ARGS&&... args )
  {
    auto a = find_action( name );

    if ( a == nullptr )
    {
      if constexpr ( std::is_constructible_v<CLASS, fs_player_t*, ARGS...> )
      {
        a = new CLASS( this, std::forward<ARGS>( args )... );
      }
      else if constexpr ( std::is_constructible_v<CLASS, std::string_view, fs_player_t*, ARGS...> )
      {
        a = new CLASS( name, this, std::forward<ARGS>( args )... );
      }
      else
      {
        static_assert( static_false<CLASS>, "Invalid constructor arguments for create_proc_action" );
      }
    }

    return a;
  }

  fs_player_t( sim_t* sim, util::string_view name, race_e r = RACE_NONE, player_e p = PLAYER_NONE );

  // Secondary Action Tracking
private:
  std::vector<action_t*> background_actions;

public:
  template <typename T, typename... Ts>
  T* find_background_action( util::string_view n = {} )
  {
    T* found_action = nullptr;
    for ( auto action : background_actions )
    {
      found_action = dynamic_cast<T*>( action );
      if ( found_action )
      {
        if ( n.empty() || found_action->name_str == n )
          break;
        else
          found_action = nullptr;
      }
    }
    return found_action;
  }

  template <typename T, typename... Ts>
  T* get_background_action( util::string_view n, Ts&&... args )
  {
    auto it = range::find( background_actions, n, &action_t::name_str );
    if ( it != background_actions.cend() )
    {
      return dynamic_cast<T*>( *it );
    }

    auto action        = new T( n, this, std::forward<Ts>( args )... );
    action->background = true;
    background_actions.push_back( action );
    return action;
  }
};

namespace actions
{  // namespace actions

template <typename Base>
class fs_player_action_t : public Base
{
protected:
  /// typedef for fs_player_action_t<action_base_t>
  using base_t = fs_player_action_t<Base>;

private:
  /// typedef for the templated action type, eg. spell_t, attack_t, heal_t
  using ab = Base;

public:
  // Init =====================================================================

    double finesse_j_mul;

  fs_player_action_t( util::string_view n, fs_player_t* p, util::string_view options = {} )
      : ab( n, p, spell_data_t::nil() ), finesse_j_mul(0.0)
  {
    ab::may_crit = ab::tick_may_crit = true;
    ab::school                       = SCHOOL_PHYSICAL; 

    if ( fs_p()->finesse_traits[ FINESSE_J ] )
    {
      finesse_j_mul = fs_p()->finesse_trait_values.finesse_j_amp[ fs_p()->finesse_traits[ FINESSE_J ] ] /
                      fs_p()->finesse_trait_values.finesse_j_divisor;
    }
  }


  fs_player_t* fs_p()
  {
    return debug_cast<fs_player_t*>( ab::player );
  }

  const fs_player_t* fs_p() const
  {
    return debug_cast<const fs_player_t*>( ab::player );
  }

  double composite_persistent_multiplier( const action_state_t* s ) const
  {
    auto m = ab::composite_persistent_multiplier( s );

    if ( fs_p()->fs_buffs.finesse_a && ab::ability_flags & ability_type_e::ABILITY_POWER )
    {
      m *= 1.0 + fs_p()->fs_buffs.finesse_a->check_stack_value();
    }

    return m;
  }

  double action_multiplier() const
  {
    auto m = ab::action_multiplier();

    if ( fs_p()->finesse_traits[ FINESSE_J ] && ab::ability_flags & ability_type_e::ABILITY_POWER )
    {
      m *= 1.0 + finesse_j_mul * fs_p()->cache.mastery();
    }

    return m;
  }


  double recharge_rate_multiplier( const cooldown_t& c ) const override
  {
    auto m = ab::recharge_rate_multiplier( c );

    if ( fs_p()->finesse_traits[ FINESSE_K ] > 0 && ab::ability_flags & ability_type_e::ABILITY_MAJOR )
    {
      auto cdr = fs_p()->finesse_trait_values.finesse_k_cdr[ fs_p()->finesse_traits[ FINESSE_K ] ];

      cdr *= 1.0 + fs_p()->fs_buffs.finesse_k->check_value();

      m /= 1.0 + cdr;
    }

    return m;
  }

  void init_finished() override
  {
    ab::init_finished();

    if ( fs_p()->finesse_traits[ FINESSE_H ] > 0 && ab::ability_flags & ability_type_e::ABILITY_BASIC )
    {
      auto added = fs_p()->finesse_trait_values.finesse_h_added[ fs_p()->finesse_traits[ FINESSE_H ] ];
      if ( ab::spell_power_mod.direct )
      {
        ab::spell_power_mod.direct += added;
      }
      else if ( ab::attack_power_mod.direct )
      {
        ab::attack_power_mod.direct += added;
      }
    }
  }

  void execute() override
  {
    ab::execute();

    if ( !ab::background )
    {
      if ( fs_p()->finesse_traits[ FINESSE_A ] > 0 )
      {
        if ( ab::ability_flags & ability_type_e::ABILITY_BASIC )
        {
          fs_p()->fs_buffs.finesse_a->trigger();
        }

        if ( ab::ability_flags & ability_type_e::ABILITY_POWER )
        {
          fs_p()->fs_buffs.finesse_a->expire();
        }
      }

      if ( fs_p()->finesse_traits[ FINESSE_B ] > 0 && ab::ability_flags & ability_type_e::ABILITY_POWER )
      {
        fs_p()->fs_buffs.finesse_b->trigger();
      }

      if ( fs_p()->finesse_traits[ FINESSE_D ] > 0 && ab::ability_flags & ability_type_e::ABILITY_POWER &&
           fs_p()->fs_rng_objects.finesse_d->trigger() )
      {
        fs_p()->resource_gain( RESOURCE_SPIRIT, fs_p()->finesse_trait_values.finesse_d_spirit_points,
                               fs_p()->fs_gains.finesse_d );
      }

      if ( fs_p()->finesse_traits[ FINESSE_G ] > 0 && ab::ability_flags & ability_type_e::ABILITY_MAJOR )
      {
        fs_p()->fs_buffs.finesse_g->trigger(
            1, fs_p()->finesse_trait_values.finesse_g_spirit_to_stats[ fs_p()->finesse_traits[ FINESSE_G ] ] *
                   fs_p()->cache.mastery() );
      }

      if ( fs_p()->finesse_traits[ FINESSE_F ] > 0 && ab::ability_flags & ability_type_e::ABILITY_CORE &&
           fs_p()->fs_rng_objects.finesse_f->trigger() )
      {
        fs_p()->fs_actions.finesse_f->execute_on_target( ab::target );
      }

      if ( fs_p()->finesse_traits[ FINESSE_I ] > 0 && ab::ability_flags & ability_type_e::ABILITY_MOVEMENT )
      {
        fs_p()->finesse_i_cdr( fs_p()->finesse_trait_values.finesse_i_cdr );
      }

      if ( fs_p()->fs_buffs.finesse_l && fs_p()->fs_buffs.finesse_l->check() &&
           ab::ability_flags & ability_type_e::ABILITY_CORE )
      {
        fs_p()->fs_actions.finesse_l->execute();
        fs_p()->fs_buffs.finesse_l->decrement();
      }
    }
  }

  double composite_player_critical_multiplier( const action_state_t* s ) const override
  {
    double cm = ab::composite_player_critical_multiplier( s );

    if ( fs_p()->finesse_traits[ FINESSE_E ] > 0 && ab::ability_flags & ability_type_e::ABILITY_CORE )
    {
      cm *= 1.0 + fs_p()->finesse_trait_values.finesse_e_cdmg[ fs_p()->finesse_traits[ FINESSE_E ] ];
    }

    return cm;
  }

  double cost_pct_multiplier() const override
  {
    auto mul = ab::cost_pct_multiplier();

    if ( ab::current_resource() == RESOURCE_SPIRIT && fs_p()->fs_gems.gem_powers[ GEM_SAPPHIRE ] >= 960.0 )
    {
      mul *= fs_p()->fs_gems.gem_powers[ GEM_SAPPHIRE ] >= 2640 ? fs_p()->fs_gems.sapphire_spirit_cost_multiplier_major
                                                                : fs_p()->fs_gems.sapphire_spirit_cost_multiplier_minor;
    }

    return mul;
  }

  // Residual Trigger Functions ===============================================

  virtual void trigger_residual_action( const action_state_t* s, double multiplier = 1.0, bool unmitigated = true,
                                        bool reverse_target_da_multiplier = true, player_t* override_target = nullptr,
                                        bool trigger_event = true )
  {
    // Depending on the ability, may use unmitigated or mitigated results
    const double base_damage = unmitigated ? s->result_total : s->result_amount;
    // Target multipliers may not replicate to secondary targets, which requires reversing them out
    const double target_da_multiplier =
        ( unmitigated && reverse_target_da_multiplier ) ? ( 1.0 / s->target_da_multiplier ) : 1.0;
    const double amount = base_damage * multiplier * target_da_multiplier;

    if ( amount <= 0 )
      return;

    player_t* primary_target = override_target ? override_target : s->target;

    fs_p()->sim->print_debug( "{} triggers residual {} for {:.2f} damage ({:.2f} * {} * {:.3f}) on {}", *fs_p(), *this,
                           amount, base_damage, multiplier, target_da_multiplier, *primary_target );

    if ( !ab::callbacks || !trigger_event )
    {
      ab::execute_on_target( primary_target, amount );
    }
    else
    {
      // Trigger as an event so that this happens after the impact for proc/RPPM targeting purposes
      make_event( *fs_p()->sim, 0_ms,
                  [ this, amount, primary_target ]() { ab::execute_on_target( primary_target, amount ); } );
    }
  }

  virtual void trigger_residual_action( player_t* primary_target, double amount, bool trigger_event = true )
  {
    if ( amount <= 0 )
      return;

    fs_p()->sim->print_debug( "{} triggers residual {} for {:.2f} damage on {}", *fs_p(), *this, amount,
                              *primary_target );

    if ( !ab::callbacks || !trigger_event )
    {
      ab::execute_on_target( primary_target, amount );
    }
    else
    {
      // Trigger as an event so that this happens after the impact for proc/RPPM targeting purposes
      make_event( *fs_p()->sim, 0_ms,
                  [ this, amount, primary_target ]() { ab::execute_on_target( primary_target, amount ); } );
    }
  }

  double parry_chance( double exp, player_t* target ) const override
  {
    return 0.0;
  }

public:
  double total_crit_bonus( const action_state_t* state ) const override
  {
    double crit_bonus = ab::total_crit_bonus( state );

    return crit_bonus + std::clamp( state->composite_crit_chance() - 1.0, 0.0, 99.0 );
  }
};

template <typename Base>
class fs_weapon_action_t : public fs_player_action_t<Base>
{
protected:
  /// typedef for fs_weapon_action_t<action_base_t>
  using base_t = fs_weapon_action_t<Base>;

  /// typedef for the templated action type, eg. spell_t, attack_t, heal_t
  using ab = fs_player_action_t<Base>;

public:
  double grandeur_gain = 0.0;
  bool active_weapon;

  fs_weapon_action_t( util::string_view n, fs_player_t* p, util::string_view options = {} )
    : ab( n, p, options ), active_weapon( false )
  {
    if ( p->fs_weapons.brave_machinations )
    {
      ab::base_crit += p->fs_weapon_trait_values.brave_machinations_crit[ p->fs_weapons.brave_machinations ];
    }

    if ( p->fs_weapons.heroic_brand )
    {
      ab::base_multiplier *= p->fs_weapon_trait_values.heroic_brand_amp[ p->fs_weapons.heroic_brand ];
    }
  }

  void init_finished() override
  {
    ab::init_finished();

    unsigned grandeur = ab::fs_p()->fs_weapons.visions_of_grandeur;
    double mul_from_cd = ab::cooldown->duration / 30_s;
    double max_spirit  = 100;

    switch ( grandeur )
    {
      case 1:
        grandeur_gain = 0.02 * max_spirit * mul_from_cd;
        break;
      case 2:
        grandeur_gain = 0.024 * max_spirit * mul_from_cd;
        break;
      case 3:
        grandeur_gain = 0.028 * max_spirit * mul_from_cd;
        break;
      case 4:
        grandeur_gain = 0.032 * max_spirit * mul_from_cd;
        break;
    }

    if ( active_weapon && !ab::background && !ab::channeled )
    {
      ab::fs_p()->weapon_cd = ab::cooldown;
    }
  }
   
  bool ready() override
  {
    if ( !active_weapon && !ab::background )
      return false;

    return ab::ready();
  }


  void execute() override
  {
    if ( !ab::background && ab::fs_p()->fs_weapons.brave_machinations )
    {
      ab::fs_p()->brave_machinations_available = true;
    }

    if ( !ab::background && ab::fs_p()->fs_weapons.martial_initiative )
    {
      ab::fs_p()->fs_buffs.martial_initiative->trigger(
          1,
          ab::fs_p()->weapon_cd->base_duration *
              ab::fs_p()
                  ->fs_weapon_trait_values.martial_initiative_duration[ ab::fs_p()->fs_weapons.martial_initiative ] );
    }

    ab::execute();

    if ( !ab::background && ab::fs_p()->fs_weapons.visions_of_grandeur )
    {
      ab::fs_p()->resource_gain( RESOURCE_SPIRIT, grandeur_gain, ab::fs_p()->fs_gains.grandeur, this );
    }
  }

  void impact( action_state_t* s ) override
  {
    ab::impact( s );

    if ( ab::fs_p()->fs_weapons.brave_machinations && ab::fs_p()->brave_machinations_available &&
         s->result == RESULT_CRIT && s->result_amount > 0 )
    {
      ab::fs_p()->brave_machinations_available = false;

      // Event to make sure the CDR is reduced after the weapon goes on CD.
      make_event( ab::sim, 0_ms, [ this ]() {
        ab::fs_p()->weapon_cd->adjust(
            -ab::fs_p()->weapon_cd->base_duration *
                ab::fs_p()->fs_weapon_trait_values.brave_machinations_cdr[ ab::fs_p()->fs_weapons.brave_machinations ],
            false );
      } );
    }
  }

  double recharge_rate_multiplier( const cooldown_t& ) const override
  {
    return ab::base_recharge_multiplier;
  }
    
  double composite_total_spell_power() const override
  {
    return std::max( ab::composite_total_spell_power(), ab::composite_total_attack_power() );
  }

  double composite_total_attack_power() const override
  {
    return std::max( ab::composite_total_spell_power(), ab::composite_total_attack_power() );
  }

  /*double composite_da_multiplier( const action_state_t* s ) const override
  {
    double m = base_t::composite_da_multiplier( s );

    if ( s->chain_target != 0 )
    {
      m *= cleave_ratio;
    }

    return m;
  }*/
};

class fs_proc_spell_t : public fs_player_action_t<spell_t>
{
protected:
  using base_t = fs_proc_spell_t;
  using ab = fs_player_action_t<spell_t>;

public:
  fs_proc_spell_t( util::string_view n, fs_player_t* p, util::string_view options = {} )
    : ab( n, p, options )
  {
    school = SCHOOL_MAGIC;
    background = true;
  }

  double composite_total_spell_power() const override
  {
    return std::max( ab::composite_total_spell_power(), ab::composite_total_attack_power() );
  }

  double composite_total_attack_power() const override
  {
    return std::max( ab::composite_total_spell_power(), ab::composite_total_attack_power() );
  }
};
template <typename Base>
class fs_relic_action_t : public fs_player_action_t<Base>
{
protected:
  /// typedef for fs_weapon_action_t<action_base_t>
  using base_t = fs_relic_action_t<Base>;

  /// typedef for the templated action type, eg. spell_t, attack_t, heal_t
  using ab = fs_player_action_t<Base>;

public:
  bool usable_relic;

  fs_relic_action_t( util::string_view n, fs_player_t* p, util::string_view options = {} )
    : ab( n, p, options ), usable_relic( false )
  {
  }

  void init_finished() override
  {
    ab::init_finished();
  }

  bool ready() override
  {
    if ( !usable_relic && !ab::background )
      return false;

    return ab::ready();
  }

  void execute() override
  {
    ab::execute();

  }

  void impact( action_state_t* s ) override
  {
    ab::impact( s );
  }

  double composite_total_spell_power() const override
  {
    return std::max( ab::composite_total_spell_power(), ab::composite_total_attack_power() );
  }

  double composite_total_attack_power() const override
  {
    return std::max( ab::composite_total_spell_power(), ab::composite_total_attack_power() );
  }
};


}  // namespace actions

}  // namespace fellowship