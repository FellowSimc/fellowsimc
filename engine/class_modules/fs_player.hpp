#pragma once
#include "util/util.hpp"

#include "simulationcraft.hpp"

namespace fellowship
{  // UNNAMED NAMESPACE

// Forward Declarations
class fs_player_t;

namespace actions
{
struct fs_player_attack_t;
struct fs_player_heal_t;
struct fs_player_spell_t;

struct fs_player_poison_t;

struct melee_t;
}  // namespace actions

class fs_player_td_t : public actor_target_data_t
{

public:
  struct dots_t
  {
  } dots;

  struct
  {
  } debuffs;

  fs_player_td_t( player_t* target, fs_player_t* source );
};

struct fs_player_buff_t : public buff_t
{
  fs_player_buff_t( player_t* p, util::string_view name ) : buff_t( p->sim, p, p, name, spell_data_t::nil(), nullptr )
  {
  }

  fs_player_t* p()
  {
    return debug_cast<fs_player_t*>( player );
  }

  const fs_player_t* p() const
  {
    return debug_cast<const fs_player_t*>( player );
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
  } fs_actions;

  struct fs_buffs_t
  {
    fs_player_buff_t* spirit_of_heroism;
    fs_player_buff_t* ancestral_surge;
    fs_player_buff_t* first_strike;
  } fs_buffs;

  struct fs_cooldowns_t
  {
  } fs_cooldowns;

  struct fs_gains_t
  {
  } fs_gains;

  struct options_t
  {
  } fs_options;

  struct fs_sets_t
  {
    // Tuzari
    bool haste_buff_on_ability_use                = false;
    double haste_buff_on_ability_use_ppm          = 0.8;
    double haste_buff_on_ability_use_haste        = 0.25;
    timespan_t haste_buff_on_ability_use_duration = 20_s;
    timespan_t haste_buff_on_ability_use_cooldown = 0_s;

  } fs_sets;

  struct fs_gems_t
  {
    std::array<double, GEM_MAX> gem_powers;
  } fs_gems;

  target_specific_t<fs_player_td_t> target_data;

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

  double resource_gain( resource_e r, double amount, gain_t* source = nullptr, action_t* a = nullptr ) override;


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

  fs_player_action_t( util::string_view n, fs_player_t* p, util::string_view options = {} )
    : ab( n, p, spell_data_t::nil() )
  {
    ab::may_crit = ab::tick_may_crit = true;
    ab::school                       = SCHOOL_PHYSICAL;
  }

  fs_player_t* fs_p()
  {
    return debug_cast<fs_player_t*>( ab::player );
  }

  const fs_player_t* fs_p() const
  {
    return debug_cast<const fs_player_t*>( ab::player );
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

  // General Methods ==========================================================

  void update_ready( timespan_t cd_duration = timespan_t::min() ) override
  {
    ab::update_ready( cd_duration );
  }

  timespan_t gcd() const override
  {
    timespan_t t = ab::gcd();

    return t;
  }

  double composite_da_multiplier( const action_state_t* state ) const override
  {
    double m = ab::composite_da_multiplier( state );

    return m;
  }

  double composite_ta_multiplier( const action_state_t* state ) const override
  {
    double m = ab::composite_ta_multiplier( state );

    return m;
  }

  double composite_target_multiplier( player_t* target ) const override
  {
    double m = ab::composite_target_multiplier( target );

    return m;
  }

  double composite_target_crit_chance( player_t* target ) const override
  {
    double c = ab::composite_target_crit_chance( target );
    return c;
  }

  double composite_crit_chance() const override
  {
    double c = ab::composite_crit_chance();

    return c;
  }

  double composite_crit_damage_bonus_multiplier() const override
  {
    double cm = ab::composite_crit_damage_bonus_multiplier();

    return cm;
  }

  double composite_target_crit_damage_bonus_multiplier( player_t* target ) const override
  {
    double cm = ab::composite_target_crit_damage_bonus_multiplier( target );

    return cm;
  }

  double total_crit_bonus( const action_state_t* state ) const override
  {
    double crit_bonus = ab::total_crit_bonus( state );

    return crit_bonus + std::clamp( state->crit_chance + state->target_crit_chance - 1.0, 0.0, 99.0 );
  }


  std::unique_ptr<expr_t> create_expression( std::string_view name ) override
  {

    return ab::create_expression( name );
  }
};
}  // namespace actions

}  // namespace fellowship