#include "fs_player.hpp"
#include "util/util.hpp"

#include "simulationcraft.hpp"

namespace fellowship
{
namespace lisa
{

// Forward Declarations
class lisa_t;

enum class secondary_trigger
{
  NONE = 0U,
};

namespace actions
{
struct lisa_heal_t;
struct lisa_spell_t;

struct melee_t;
}  // namespace actions

class lisa_td_t : public fs_player_td_t
{
public:
  struct dots_t
  {
    dot_t* splinter_of_time;
    dot_t* time_burn;
  } dots;

  struct
  {
    buff_t* chrono_bind;
    buff_t* shifting_sands;
  } debuffs;

  lisa_td_t( player_t* target, lisa_t* source );
};

struct lisa_buff_t : public fs_player_buff_t
{
  lisa_buff_t( player_t* p, util::string_view name ) : fs_player_buff_t( p, name )
  {
  }

  lisa_t* p()
  {
    return debug_cast<lisa_t*>( player );
  }

  const lisa_t* p() const
  {
    return debug_cast<const lisa_t*>( player );
  }
};

class lisa_t : public fellowship::fs_player_t
{
public:
  struct actions_t
  {
  } actions;

  struct buffs_t
  {
    buff_t* time_rift;
    buff_t* absolute_stasis;
    buff_t* clocks_ticking;
    buff_t* temporal_paradox;
    buff_t* uchronia;
    buff_t* practical_overcharge;
  } buffs;

  struct cooldowns_t
  {
  } cooldowns;

  struct gains_t
  {
    gain_t* spirit_procs;
  } gains;

  struct procs_t
  {
  } procs;

  struct rppms_t
  {
  } rppm;

  struct spell_const_t
  {
    timespan_t heal_look_back_duration = 5_s;
    double spirit_proc_resource        = 25;

    timespan_t time_shard_cast_time = 1.5_s;
    double time_shard_sp_coeff        = 1.2;
    double time_shard_resource        = 10;
    double time_shard_resource_crit  = 15;
    double time_shard_mana_cost       = 1;

    double splinter_of_time_sp_coeff = 0.6;
    timespan_t splinter_of_time_dot_duration = 15_s;
    timespan_t splinter_of_time_period       = 2_s;
    double splinter_of_time_tick_sp_coeff       = 0.2;
    double splinter_of_time_tick_resource    = 3.5;
    double splinter_of_time_tick_resource_crit    = 4.5;
    double splinter_of_time_mana_cost          = 3;

    timespan_t shifting_sands_cast_time = 2_s;
    double shifting_sands_sp_coeff      = 7.2;
    double shifting_sands_resource      = 50;
    double shifting_sands_resource_crit = 70;
    timespan_t shifting_sands_cd        = 45_s;
    bool shifting_sands_cd_hasted       = false; // TODO: Confirm
    timespan_t shifting_sands_debuff_duration = 15_s;
    double shifting_sands_debuff_amp          = 0.2;

    timespan_t rewind_cast_time = 1.5_s;
    double rewind_sp_coeff      = 10.5;
    double rewind_past_taken_heal = 0.3;
    double rewind_mana_cost       = 150;
    double rewind_resource        = 10;
    double rewind_resource_crit   = 15;

    timespan_t temporal_barrier_duration = 25_s;
    double temporal_barrier_sp_coeff_minimum   = 4.5;
    double temporal_barrier_sp_coeff_maximum   = 20;
    double temporal_barrier_mana_cost    = 80;

    timespan_t chrono_barrage_channel_duration = 1.7_s;
    timespan_t chrono_barrage_channel_period   = 0.6_s;
    double chrono_barrage_dmg_sp_coeff_minimum = 1;
    double chrono_barrage_dmg_sp_coeff_maximum = 5;
    double chrono_barrage_heal_sp_coeff_minimum = 1.7;
    double chrono_barrage_heal_sp_coeff_maximum = 11.5;
    double chrono_barrage_mana_cost             = 50;

    timespan_t time_slip_cd = 15_s;
    bool time_slip_cd_hasted = false;

    timespan_t absolute_stasis_duration = 6_s;
    double absolute_stasis_cd_recovery  = 6;
    double absolute_stasis_cd_reduction = 6;

    timespan_t chrono_veil_duration = 15_s;
    timespan_t chrono_veil_cd       = 180_s;
    double chrono_veil_mana_cost    = 20;

    timespan_t chrono_bind_cd = 300_s;
    timespan_t chrono_bind_duration = 7_s;
    double chrono_bind_mana         = 80;
    double chrono_bind_max_targets  = 8;

    timespan_t time_burn_cast_time = 2_s;
    timespan_t time_burn_dot_duration = 21_s;
    timespan_t time_burn_dot_period   = 3_s;
    double time_burn_tick_sp_coeff    = 4.5;
    timespan_t time_burn_cd           = 30_s;
    bool time_burn_cd_hasted          = false; // TODO: Confirm
    double time_burn_tick_resource    = 5;
    double time_burn_tick_resource_crit = 10;
    double time_burn_mana               = 40;

    double time_rift_haste = 0.15;
    double time_rift_resource = 2;
    timespan_t time_rift_duration = 20_s;
    double time_rift_mana         = 50;
    timespan_t time_rift_cd       = 90_s;
    timespan_t time_rift_period   = 0.15_s;

    timespan_t time_mending_hot_duration = 12_s;
    timespan_t time_mending_hot_period = 2_s;
    double time_mending_tick_sp_coeff_minimum = 0.3;
    double time_mending_tick_sp_coeff_maximum = 2.5;
    double time_mending_past_taken_heal       = 0.15;
    double time_mending_mana                  = 80;
  } spell_const;

  enum lisa_talents_t : unsigned long long
  {
    NONE      = 0ULL,
    TALENT_1  = 1ULL << 0,
    TALENT_2  = 1ULL << 1,
    TALENT_3  = 1ULL << 2,
    TALENT_4  = 1ULL << 3,
    TALENT_5  = 1ULL << 4,
    TALENT_6  = 1ULL << 5,
    TALENT_7  = 1ULL << 6,
    TALENT_8  = 1ULL << 7,
    TALENT_9  = 1ULL << 8,
    TALENT_10 = 1ULL << 9,
    TALENT_11 = 1ULL << 10,
    TALENT_12 = 1ULL << 11,
    TALENT_13 = 1ULL << 12,
    TALENT_14 = 1ULL << 13,
    TALENT_15 = 1ULL << 14,
    TALENT_16 = 1ULL << 15,
    TALENT_17 = 1ULL << 16,
    TALENT_18 = 1ULL << 17,
    MAX       = 1ULL << 18
  };

  static constexpr std::string_view talent_name_formatted( lisa_talents_t t )
  {
    switch ( t )
    {
      case lisa_talents_t::TALENT_1:
        return "Fractions of Time";
      case lisa_talents_t::TALENT_2:
        return "Overcharge Manipulation";
      case lisa_talents_t::TALENT_3:
        return "Good Timing";
      case lisa_talents_t::TALENT_4:
        return "Practical Overcharge";
      case lisa_talents_t::TALENT_5:
        return "Clocks Ticking";
      case lisa_talents_t::TALENT_6:
        return "Wears of Time";
      case lisa_talents_t::TALENT_7:
        return "Temporal Protection";
      case lisa_talents_t::TALENT_8:
        return "Echoes of Time";
      case lisa_talents_t::TALENT_9:
        return "A Tear in Time";
      case lisa_talents_t::TALENT_10:
        return "Temporal Bulwark";
      case lisa_talents_t::TALENT_11:
        return "Magic Ward";
      case lisa_talents_t::TALENT_12:
        return "Chrono Dynamic";
      case lisa_talents_t::TALENT_13:
        return "Temporal Paradox";
      case lisa_talents_t::TALENT_16:
        return "Uchronia";
      case lisa_talents_t::TALENT_18:
        return "Spirited Fortitude";
      case lisa_talents_t::TALENT_14:
      case lisa_talents_t::TALENT_15:
      case lisa_talents_t::TALENT_17:
        return "NYI Talent";
      default:
        return "Unknown Talent";
    }
  }
  static constexpr std::string_view talent_name( lisa_talents_t t )
  {
    switch ( t )
    {
      case lisa_talents_t::TALENT_1:
        return "fractions_of_time";
      case lisa_talents_t::TALENT_2:
        return "overcharge_manipulation";
      case lisa_talents_t::TALENT_3:
        return "good_timing";
      case lisa_talents_t::TALENT_4:
        return "practical_overcharge";
      case lisa_talents_t::TALENT_5:
        return "clocks_ticking";
      case lisa_talents_t::TALENT_6:
        return "wears_of_time";
      case lisa_talents_t::TALENT_7:
        return "temporal_protection";
      case lisa_talents_t::TALENT_8:
        return "echoes_of_time";
      case lisa_talents_t::TALENT_9:
        return "a_tear_in_time";
      case lisa_talents_t::TALENT_10:
        return "temporal_bulwark";
      case lisa_talents_t::TALENT_11:
        return "magic_ward";
      case lisa_talents_t::TALENT_12:
        return "chrono_dynamic";
      case lisa_talents_t::TALENT_13:
        return "temporal_paradox";
      case lisa_talents_t::TALENT_16:
        return "uchronia";
      case lisa_talents_t::TALENT_18:
        return "spirited_fortitude";
      case lisa_talents_t::TALENT_14:
      case lisa_talents_t::TALENT_15:
      case lisa_talents_t::TALENT_17:
        return "nyi_talent";
      default:
        return "unknown_talent";
    }
  }

  struct talents_t
  {
    double practical_overcharge_mana_regen = 0.25;
    timespan_t practical_overcharge_duration = 8_s;

    double overcharge_manipulation_threshold = 0.5;
    double overcharge_manipulation_amp       = 0.15;


  } talents;

  struct legendary_t
  {
  } legendary;

  struct options_t
  {
  } options;

  target_specific_t<lisa_td_t> target_data;

  const lisa_td_t* find_target_data( const player_t* target ) const override
  {
    return target_data[ target ];
  }

  lisa_td_t* get_target_data( player_t* target ) const override
  {
    lisa_td_t*& td = target_data[ target ];
    if ( !td )
    {
      td = new lisa_td_t( target, const_cast<lisa_t*>( this ) );
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

  role_e primary_role() const override
  {
    return ROLE_SPELL;
  }
  stat_e convert_hybrid_stat( stat_e s ) const override;

  double composite_attribute_multiplier( attribute_e attr ) const override;
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
  double non_stacking_movement_modifier() const override;
  double stacking_movement_modifier() const override;
  void invalidate_cache( cache_e ) override;

  void analyze( sim_t& sim ) override;

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

  double current_temporal_overcharge( bool /* react */ = false ) const
  {
    return resources.current[ RESOURCE_TEMPORAL_OVERCHARGE ];
  }

  resource_e primary_resource() const override
  {
    return RESOURCE_SPIRIT;
  }

  // rogue_t::default_temporary_enchant =======================================

  std::string default_temporary_enchant() const override
  {
    return "disabled";
  }

  lisa_t( sim_t* sim, util::string_view name, race_e r = RACE_NONE )
    : fs_player_t( sim, name, r, LISA ), target_data()
  {
    create_cooldowns();
  }
};

namespace actions
{  // namespace actions

template <typename Base>
struct secondary_action_trigger_t : public event_t
{
  Base* action;
  action_state_t* state;
  player_t* target;

  secondary_action_trigger_t( action_state_t* s, timespan_t delay = timespan_t::zero() )
    : event_t( *s->action->sim, delay ), action( dynamic_cast<Base*>( s->action ) ), state( s ), target( nullptr )
  {
  }

  secondary_action_trigger_t( player_t* target, Base* action, timespan_t delay = timespan_t::zero() )
    : event_t( *action->sim, delay ), action( action ), state( nullptr ), target( target )
  {
  }

  const char* name() const override
  {
    return "secondary_action_trigger";
  }

  void execute() override
  {
    assert( action->is_secondary_action() );

    player_t* action_target = state ? state->target : target;

    // Ensure target is still available and did not demise during delay.
    if ( !action_target || action_target->is_sleeping() )
      return;

    action->set_target( action_target );

    // No state, construct one and grab combo points from the event instead of current CP amount.
    if ( !state )
    {
      state         = action->get_state();
      state->target = action_target;
      // Calling snapshot_internal, snapshot_state would overwrite CP.
      action->snapshot_internal( state, STATE_CRIT, action->amount_type( state ) );
    }

    assert( !action->pre_execute_state );

    action->pre_execute_state = state;
    action->snapshot_internal( state, action->snapshot_flags & ~STATE_CRIT, action->amount_type( state ) );
    action->execute();
    state = nullptr;
  }

  ~secondary_action_trigger_t() override
  {
    if ( state )
      action_state_t::release( state );
  }
};

template <typename T_ACTION>
struct lisa_action_state_t : public action_state_t
{
private:
  T_ACTION* action;

public:
  lisa_action_state_t( action_t* action, player_t* target )
    : action_state_t( action, target ), action( dynamic_cast<T_ACTION*>( action ) )
  {
  }

  lisa_t* p() const
  {
    return debug_cast<lisa_t*>( action->player );
  }

  lisa_t* p()
  {
    return debug_cast<lisa_t*>( action->player );
  }

  void initialize() override
  {
    action_state_t::initialize();
  }

  std::ostringstream& debug_str( std::ostringstream& s ) override
  {
    // action_state_t::debug_str( s ) << " base_cp=" << base_cp << " total_cp=" << total_cp;
    return action_state_t::debug_str( s );
  }

  void copy_state( const action_state_t* s )
  {
    action_state_t::copy_state( s );
    const lisa_action_state_t* rs = debug_cast<const lisa_action_state_t*>( s );
  }

  T_ACTION* get_action() const
  {
    return action;
  }
};

template <typename Base>
class lisa_action_t : public Base
{
protected:
  /// typedef for lisa_action_t<action_base_t>
  using base_t = lisa_action_t<Base>;

private:
  /// typedef for the templated action type, eg. spell_t, attack_t, heal_t
  using ab = Base;

public:
  secondary_trigger secondary_trigger_type;

  // Init =====================================================================

  lisa_action_t( util::string_view n, lisa_t* p, util::string_view options = {} )
    : ab( n, p, options ), secondary_trigger_type( secondary_trigger::NONE )
  {
    ab::parse_options( options );
    ab::may_crit = ab::tick_may_crit = true;
    ab::school                       = SCHOOL_ARCANE;

    // lisa_t sets base and min GCD to 1.5_s hasted
    ab::gcd_type = gcd_haste_type::SPELL_HASTE;
  }

  void init() override
  {
    ab::init();
  }

  // Type Wrappers ============================================================

  static const lisa_action_state_t<base_t>* cast_state( const action_state_t* st )
  {
    return debug_cast<const lisa_action_state_t<base_t>*>( st );
  }

  static lisa_action_state_t<base_t>* cast_state( action_state_t* st )
  {
    return debug_cast<lisa_action_state_t<base_t>*>( st );
  }

  lisa_t* p()
  {
    return debug_cast<lisa_t*>( ab::player );
  }

  const lisa_t* p() const
  {
    return debug_cast<const lisa_t*>( ab::player );
  }

  lisa_td_t* td( player_t* t ) const
  {
    return p()->get_target_data( t );
  }

  // Action State =============================================================

  action_state_t* new_state() override
  {
    return new lisa_action_state_t<base_t>( this, ab::target );
  }

  void update_state( action_state_t* state, unsigned flags, result_amount_type rt ) override
  {
    ab::update_state( state, flags, rt );
  }

  void snapshot_state( action_state_t* state, result_amount_type rt ) override
  {
    ab::snapshot_state( state, rt );
  }

  // Secondary Trigger Functions ==============================================

  bool is_secondary_action() const
  {
    return secondary_trigger_type != secondary_trigger::NONE;
  }

  virtual void trigger_secondary_action( player_t* target, timespan_t delay = timespan_t::zero() )
  {
    assert( is_secondary_action() );
    make_event<secondary_action_trigger_t<base_t>>( *ab::sim, target, this, delay );
  }

  virtual void trigger_secondary_action( action_state_t* s, timespan_t delay = timespan_t::zero() )
  {
    assert( is_secondary_action() && s->action == this );
    make_event<secondary_action_trigger_t<base_t>>( *ab::sim, s, delay );
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

    p()->sim->print_debug( "{} triggers residual {} for {:.2f} damage ({:.2f} * {} * {:.3f}) on {}", *p(), *this,
                           amount, base_damage, multiplier, target_da_multiplier, *primary_target );

    if ( !ab::callbacks || !trigger_event )
    {
      ab::execute_on_target( primary_target, amount );
    }
    else
    {
      // Trigger as an event so that this happens after the impact for proc/RPPM targeting purposes
      make_event( *p()->sim, 0_ms,
                  [ this, amount, primary_target ]() { ab::execute_on_target( primary_target, amount ); } );
    }
  }

  virtual void trigger_residual_action( player_t* primary_target, double amount, bool trigger_event = true )
  {
    if ( amount <= 0 )
      return;

    p()->sim->print_debug( "{} triggers residual {} for {:.2f} damage on {}", *p(), *this, amount, *primary_target );

    if ( !ab::callbacks || !trigger_event )
    {
      ab::execute_on_target( primary_target, amount );
    }
    else
    {
      // Trigger as an event so that this happens after the impact for proc/RPPM targeting purposes
      make_event( *p()->sim, 0_ms,
                  [ this, amount, primary_target ]() { ab::execute_on_target( primary_target, amount ); } );
    }
  }

  // Helper Functions =========================================================

  // Helper function for expressions. Returns the number of guaranteed generated combo points for
  // this ability, taking into account any potential buffs.
  virtual double generate_temporal_overcharge() const
  {
    double cp = 0;

    if ( ab::energize_type != action_energize::NONE && ab::energize_resource == RESOURCE_TEMPORAL_OVERCHARGE )
    {
      cp += ab::energize_amount;
    }

    return cp;
  }

  double parry_chance( double exp, player_t* target ) const override
  {
    return 0.0;
  }

public:
  // Ability triggers
  void spend_resource_costs( const action_state_t* );
  void trigger_spirit_refund( const action_state_t*, double );

  // General Methods ==========================================================

  void update_ready( timespan_t cd_duration = timespan_t::min() ) override
  {
    if ( secondary_trigger_type != secondary_trigger::NONE )
    {
      cd_duration = timespan_t::zero();
    }

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

    return crit_bonus;
  }

  timespan_t tick_time( const action_state_t* s ) const override
  {
    timespan_t tt = ab::tick_time( s );

    return tt;
  }

  double cost_pct_multiplier() const override
  {
    double c = ab::cost_pct_multiplier();

    return c;
  }

  void consume_resource() override
  {
    // Abilities triggered as part of another ability (secondary triggers) do not consume resources
    if ( is_secondary_action() )
    {
      return;
    }

    ab::consume_resource();

    spend_resource_costs( ab::execute_state );
  }

  void execute() override
  {
    ab::execute();

    // if ( p()->talents.soulfrost_torrent && !is_secondary_action() && !ab::background && !ab::tick_action )
    //{
    //   if ( !p()->buffs.soulfrost_torrent->check() && p()->rppm.soulfrost_torrent->trigger() )
    //   {
    //     p()->buffs.soulfrost_torrent->trigger();
    //   }
    // }

    // if ( p()->legendary.undulating_spirit && !is_secondary_action() && !ab::tick_action && !ab::background )
    // {
    //   if ( p()->rng().roll( p()->legendary.undulating_spirit_chance ) )
    //   {
    //     p()->buffs.undulating_spirit->trigger();
    //   }
    //}
  }

  void schedule_travel( action_state_t* state ) override
  {
    ab::schedule_travel( state );
  }

  bool ready() override
  {
    if ( !ab::ready() )
      return false;

    return true;
  }

  std::unique_ptr<expr_t> create_expression( std::string_view name ) override
  {
    if ( util::str_compare_ci( name, "overcharge_gain" ) )
    {
      return make_mem_fn_expr( "overcharge_gain", *this, &base_t::generate_temporal_overcharge );
    }

    return ab::create_expression( name );
  }
};

struct lisa_heal_t : public lisa_action_t<fellowship::actions::fs_player_action_t<heal_t>>
{
  lisa_heal_t( util::string_view n, lisa_t* p, util::string_view o = {} ) : base_t( n, p, o )
  {
    harmful = false;
    set_target( p );
  }
};

struct lisa_spell_t : public lisa_action_t<fellowship::actions::fs_player_action_t<spell_t>>
{
  double overcharge_on_hit;
  double overcharge_on_crit;
  double overcharge_on_tick;
  double overcharge_on_tick_crit;

  lisa_spell_t( util::string_view n, lisa_t* p, util::string_view o = {} )
    : base_t( n, p, o ),
      overcharge_on_hit( 0 ),
      overcharge_on_crit( 0 ),
      overcharge_on_tick( 0 ),
      overcharge_on_tick_crit( 0 )
  {
    resource_current = RESOURCE_MANA;
  }

  double composite_da_multiplier( const action_state_t* s ) const override
  {
    auto da = base_t::composite_da_multiplier( s );

    if ( p()->talents_enabled( lisa_t::TALENT_2 ) && secondary_costs[ RESOURCE_TEMPORAL_OVERCHARGE ] < 1 &&
         p()->resources.pct( RESOURCE_TEMPORAL_OVERCHARGE ) <= p()->talents.overcharge_manipulation_threshold )
    {
      da *= 1 + p()->talents.overcharge_manipulation_amp;
    }

    return da;
  }

  double composite_ta_multiplier( const action_state_t* s ) const override
  {
    auto ta = base_t::composite_ta_multiplier( s );

    if ( p()->talents_enabled( lisa_t::TALENT_2 ) && secondary_costs[ RESOURCE_TEMPORAL_OVERCHARGE ] < 1 &&
         p()->resources.pct( RESOURCE_TEMPORAL_OVERCHARGE ) <= p()->talents.overcharge_manipulation_threshold )
    {
      ta *= 1 + p()->talents.overcharge_manipulation_amp;
    }

    return ta;
  }

  void impact( action_state_t* s ) override
  {
    base_t::impact( s );

    if ( result_is_hit( s->result ) )
    {
      if ( s->result == RESULT_HIT && overcharge_on_hit > 0 )
      {
        p()->resource_gain( RESOURCE_TEMPORAL_OVERCHARGE, overcharge_on_hit, gain, this );
      }
      if ( s->result == RESULT_CRIT && overcharge_on_crit > 0 )
      {
        p()->resource_gain( RESOURCE_TEMPORAL_OVERCHARGE, overcharge_on_crit, gain, this );
      }
    }
  }

  void tick( dot_t* d ) override
  {
    base_t::tick( d );

    if ( result_is_hit( d->state->result ) )
    {
      if ( d->state->result == RESULT_HIT && overcharge_on_tick > 0 )
      {
        p()->resource_gain( RESOURCE_TEMPORAL_OVERCHARGE, overcharge_on_tick, gain, this );
      }
      if ( d->state->result == RESULT_CRIT && overcharge_on_tick_crit > 0 )
      {
        p()->resource_gain( RESOURCE_TEMPORAL_OVERCHARGE, overcharge_on_tick_crit, gain, this );
      }
    }
  }
};

struct time_shard_t : public lisa_spell_t
{
  time_shard_t( util::string_view name, lisa_t* p, util::string_view options_str = {} )
    : lisa_spell_t( name, p, options_str )
  {
    id = 2;

    spell_power_mod.direct = p->spell_const.time_shard_sp_coeff;

    name_str_reporting = "Time Shard";

    base_execute_time = p->spell_const.time_shard_cast_time;

    overcharge_on_hit  = p->spell_const.time_shard_resource;
    overcharge_on_crit = p->spell_const.time_shard_resource_crit;
  }
};

//struct detonate_t : public lisa_spell_t
//{
//  struct detonate_damage_t : public lisa_spell_t
//  {
//    timespan_t sample_duration;
//    detonate_damage_t( util::string_view name, lisa_t* p, util::string_view options_str = {} )
//      : lisa_spell_t( fmt::format( "{}_dmg", name ), p, options_str ),
//        sample_duration( p->spell_const.detonate_sample_duration )
//    {
//      id                 = 3;
//      name_str_reporting = "Detonate";
//
//      aoe = -1;
//      // tested 08/12/2025 - No reduced AOE currently.
//      // reduced_aoe_targets = 1;
//
//      dual = background = true;
//
//      if ( p->talents.double_detonate_cost_efficiency )
//        sample_duration *= 2;
//    }
//
//    double tick_damage_over_time( const dot_t* dot ) const
//    {
//      if ( !dot->is_ticking() )
//      {
//        return 0.0;
//      }
//
//      action_state_t* state = dot->current_action->get_state( dot->state );
//      dot->current_action->calculate_tick_amount( state, 1.0 );
//      double tick_base_damage  = state->result_raw;
//      timespan_t dot_tick_time = dot->current_action->tick_time( state );
//      // We don't care how much is remaining on the target, this will always deal
//      // Xs worth of DoT ticks even if the amount is currently less
//      double ticks_left   = sample_duration / dot_tick_time;
//      double total_damage = ticks_left * tick_base_damage;
//      total_damage /= state->target_ta_multiplier;
//      action_state_t::release( state );
//
//      sim->print_debug( "{} Detonate tick damage from {}: {:.2f} ({:.2f} per tick for {:.2f} ticks)", *p(),
//                        *dot->current_action, total_damage, tick_base_damage, ticks_left );
//      return total_damage;
//    }
//
//    double get_detonate_damage( player_t* target ) const
//    {
//      double da = 0.0;
//
//      auto td = p()->get_target_data( target );
//      da += tick_damage_over_time( td->dots.crackling_inferno );
//      da += tick_damage_over_time( td->dots.engulfing_flames );
//      da += tick_damage_over_time( td->dots.fire_ball );
//      da += tick_damage_over_time( td->dots.fire_frog );
//      da += tick_damage_over_time( td->dots.incinerate );
//      da += tick_damage_over_time( td->dots.searing_blaze );
//
//      return da / p()->spell_const.detonate_hits;
//    }
//
//    void init() override
//    {
//      lisa_spell_t::init();
//
//      snapshot_flags &= ~( STATE_VERSATILITY | STATE_MUL_PLAYER_DAM );
//      update_flags &= ~( STATE_VERSATILITY | STATE_MUL_PLAYER_DAM );
//
//      snapshot_flags |= STATE_TGT_CRIT | STATE_TGT_MUL_DA;
//      update_flags |= STATE_TGT_CRIT | STATE_TGT_MUL_DA;
//    }
//
//    void snapshot_internal( action_state_t* s, unsigned flags, result_amount_type rt ) override
//    {
//      lisa_spell_t::snapshot_internal( s, flags, rt );
//
//      base_dd_min = base_dd_max = get_detonate_damage( s->target );
//    }
//
//    double composite_target_multiplier( player_t* target ) const override
//    {
//      double m = lisa_spell_t::composite_target_multiplier( target );
//      if ( p()->legendary.explosive_potency )
//      {
//        if ( target->health_percentage() <= 35.0 )
//        {
//          m *= 1.0 + p()->legendary.explosive_potency_detonate_amp;
//        }
//      }
//      return m;
//    }
//  };
//
//  detonate_damage_t* damage_action;
//  detonate_t( util::string_view name, lisa_t* p, util::string_view options_str = {} )
//    : lisa_spell_t( name, p, options_str )
//  {
//    id                 = 3;
//    name_str_reporting = "Detonate";
//
//    resource_current               = RESOURCE_TEMPORAL_OVERCHARGE;
//    base_costs[ RESOURCE_TEMPORAL_OVERCHARGE ] = p->spell_const.detonate_embers_cost;
//
//    if ( p->talents.double_detonate_cost_efficiency )
//      base_costs[ RESOURCE_TEMPORAL_OVERCHARGE ] *= 2;
//
//    base_execute_time = 0_s;
//    trigger_gcd       = 1_s;
//    gcd_type          = gcd_haste_type::NONE;
//
//    damage_action        = new detonate_damage_t( name, p, options_str );
//    damage_action->stats = stats;
//  }
//
//  void execute() override
//  {
//    lisa_spell_t::execute();
//
//    for ( int i = 0; i < p()->spell_const.detonate_hits; ++i )
//    {
//      damage_action->execute();
//    }
//
//    if ( p()->talents_enabled( lisa_t::REIGN_OF_FIRE ) )
//    {
//      if ( p()->rppm.reign_of_fire->trigger() )
//      {
//        p()->buffs.reign_of_fire->trigger();
//        p()->cooldowns.fire_ball->reset( true, 1 );
//      }
//    }
//  }
//};
//
//struct wildfire_t : public lisa_spell_t
//{
//  wildfire_t( util::string_view name, lisa_t* p, util::string_view options_str = {} )
//    : lisa_spell_t( name, p, options_str )
//  {
//    id = 7;
//
//    name_str_reporting = "Wildfire";
//
//    trigger_gcd = timespan_t::zero();
//
//    cooldown->duration = p->spell_const.wildfire_cooldown;
//    cooldown->hasted   = false;
//    cooldown->charges  = 1;
//  }
//
//  void execute() override
//  {
//    p()->buffs.wildfire->trigger();
//    lisa_spell_t::execute();
//  }
//};
//
//struct incinerate_t : public lisa_spell_t
//{
//  struct incinerate_dot_t : public lisa_spell_t
//  {
//    incinerate_dot_t( util::string_view name, lisa_t* p, util::string_view options_str = {} )
//      : lisa_spell_t( fmt::format( "{}_dot", name ), p, options_str )
//    {
//      id = 9;
//
//      name_str_reporting   = "Incinerate (DoT)";
//      spell_power_mod.tick = p->spell_const.incinerate_dot_coeff;
//      dot_duration         = p->spell_const.incinerate_dot_duration;
//      base_tick_time       = p->spell_const.incinerate_dot_period;
//      hasted_ticks         = true;
//      dot_behavior         = DOT_REFRESH_DURATION;
//
//      base_crit += p->talents_enabled( lisa_t::FIRESTARTER ) ? p->talents.firestarter_crit_chance : 0.0;
//    }
//
//    double composite_ta_multiplier( const action_state_t* s ) const override
//    {
//      double m = lisa_spell_t::composite_ta_multiplier( s );
//
//      auto td = p()->get_target_data( s->target );
//      m *= 1.0 + td->debuffs.incinerate_stacks->check_stack_value();
//
//      return m;
//    }
//
//    void trigger_dot( action_state_t* s ) override
//    {
//      lisa_spell_t::trigger_dot( s );
//      p()->get_target_data( s->target )->debuffs.incinerate_stacks->increment();
//    }
//
//    void last_tick( dot_t* d ) override
//    {
//      lisa_spell_t::last_tick( d );
//      p()->get_target_data( d->target )->debuffs.incinerate_stacks->expire();
//    }
//  };
//
//  struct incinerate_aoe_t : public lisa_spell_t
//  {
//    action_t* dot;
//    timespan_t engulf_duration;
//    incinerate_aoe_t( util::string_view name, lisa_t* p, util::string_view options_str = {} )
//      : lisa_spell_t( fmt::format( "{}_aoe", name ), p, options_str ),
//        engulf_duration( p->talents_enabled( lisa_t::UNDYING_FLAME )
//                             ? p->spell_const.engulfing_flames_duration + p->talents.undying_flame_extension
//                             : p->spell_const.engulfing_flames_duration )
//    {
//      id = 9;
//
//      aoe                    = -1;
//      name_str_reporting     = "Incinerate";
//      spell_power_mod.direct = p->spell_const.incinerate_coeff;
//      reduced_aoe_targets    = p->spell_const.incinerate_falloff;
//
//      dot = new incinerate_dot_t( name, p, options_str );
//      add_child( dot );
//    }
//
//    double composite_da_multiplier( const action_state_t* s ) const override
//    {
//      double m = base_t::composite_da_multiplier( s );
//
//      if ( parent_dot )
//      {
//        m *= parent_dot->get_tick_factor();
//      }
//
//      return m;
//    }
//
//    void impact( action_state_t* s ) override
//    {
//      lisa_spell_t::impact( s );
//      // Apply dot
//      dot->set_target( s->target );
//      dot->execute();
//
//      auto td = p()->get_target_data( s->target );
//
//      td->dots.crackling_inferno->adjust_duration( p()->spell_const.incinerate_dot_extend, p()->talents.crackling_inferno_dot_duration );
//
//      if ( td->dots.engulfing_flames->max_stack == 1 )
//        td->dots.engulfing_flames->adjust_duration( p()->spell_const.incinerate_dot_extend, engulf_duration );
//
//      auto& vec = td->dots.engulfing_decrement_events;
//      for ( size_t i = 0; i < vec.size(); )
//      {
//        if ( vec[ i ]->remains() < 0_s || vec[ i ]->canceled )
//        {
//          erase_unordered( vec, vec.begin() + i );
//        }
//        else
//        {
//          timespan_t new_length = std::min(
//              engulf_duration, vec[ i ]->occurs() - sim->current_time() + p()->spell_const.incinerate_dot_extend );
//
//          vec[ i ]->reschedule( new_length );
//          i++;
//        }
//      }
//
//      td->dots.fire_ball->adjust_duration( p()->spell_const.incinerate_dot_extend,
//                                           p()->spell_const.fire_ball_dot_duration );
//      td->dots.fire_frog->adjust_duration( p()->spell_const.incinerate_dot_extend,
//                                           p()->spell_const.fire_frog_dot_duration );
//      td->dots.searing_blaze->adjust_duration( p()->spell_const.incinerate_dot_extend,
//                                               p()->spell_const.searing_blaze_duration );
//    }
//  };
//
//  struct incinerate_channel_t : public lisa_spell_t
//  {
//    action_t* custom_tick_action;
//    action_t* parent;
//
//    incinerate_channel_t( util::string_view name, lisa_t* p, action_t* parent, util::string_view options_str = {} )
//      : lisa_spell_t( fmt::format( "{}_channel", name ), p, options_str ), parent( parent )
//    {
//      id = 9;
//
//      name_str_reporting = "Incinerate";
//
//      dual = true;
//
//      channeled              = true;
//      dot_allow_partial_tick = true;
//      dot_duration           = p->spell_const.incinerate_duration;
//      hasted_ticks           = true;
//      base_tick_time         = p->spell_const.incinerate_period;
//
//      target = p;
//
//      custom_tick_action = new incinerate_aoe_t( name, p, options_str );
//    }
//
//    void execute() override
//    {
//      target = player;
//      lisa_spell_t::execute();
//    }
//
//    void tick( dot_t* d ) override
//    {
//      lisa_spell_t::tick( d );
//      custom_tick_action->set_target( parent->target );
//      custom_tick_action->execute();
//    }
//  };
//
//  incinerate_channel_t* channel_action;
//  incinerate_t( util::string_view name, lisa_t* p, util::string_view options_str = {} )
//    : lisa_spell_t( name, p, options_str )
//  {
//    id = 9;
//
//    name_str_reporting = "Incinerate";
//
//    base_execute_time = 1_s;
//
//    resource_current              = RESOURCE_SPIRIT;
//    base_costs[ RESOURCE_SPIRIT ] = 100;
//
//    channel_action        = new incinerate_channel_t( name, p, this, options_str );
//    channel_action->stats = stats;
//    add_child( channel_action->custom_tick_action );
//  }
//
//  void execute() override
//  {
//    lisa_spell_t::execute();
//    p()->fs_buffs.spirit_of_heroism->trigger();
//    p()->used_ultimate();
//    channel_action->execute();
//  }
//};
//
//struct searing_blaze_t : public lisa_spell_t
//{
//  searing_blaze_t( util::string_view name, lisa_t* p, util::string_view options_str = {} )
//    : lisa_spell_t( name, p, options_str )
//  {
//    id                     = 11;
//    name_str_reporting     = "Searing Blaze";
//    spell_power_mod.tick   = p->spell_const.searing_blaze_tick_coeff;
//    dot_duration           = p->spell_const.searing_blaze_duration;
//    base_tick_time         = p->spell_const.searing_blaze_period;
//    dot_allow_partial_tick = true;
//    hasted_ticks           = true;
//
//    dot_behavior = DOT_REFRESH_PANDEMIC;
//
//    base_execute_time = 0_s;
//
//    energize_type     = action_energize::NONE;
//    energize_amount   = p->spell_const.searing_blaze_embers_per_tick;
//    energize_resource = RESOURCE_TEMPORAL_OVERCHARGE;
//
//    if ( p->talents_enabled( lisa_t::FLICKERING_CINDERS ) )
//    {
//      energize_amount *= 1 + p->talents.flickering_cinders_cinder_multiplier;
//    }
//
//    base_crit += p->talents_enabled( lisa_t::FIRESTARTER ) ? p->talents.firestarter_crit_chance : 0.0;
//  }
//
//  void execute() override
//  {
//    lisa_spell_t::execute();
//  }
//
//  double composite_ta_multiplier( const action_state_t* s ) const override
//  {
//    double m = lisa_spell_t::composite_ta_multiplier( s );
//
//    auto td = p()->get_target_data( s->target );
//    m *= 1.0 + td->debuffs.agonizing_blaze_stacks->check_stack_value();
//
//    return m;
//  }
//
//  void impact( action_state_t* s ) override
//  {
//    lisa_spell_t::impact( s );
//  }
//
//  double spontaneous_chance() const
//  {
//    return p()->talents.spontaneous_combustion_chance +
//           0.01 * p()->cache.spell_crit_chance() / p()->talents.spontaneous_combustion_extra_chance;
//  }
//
//  double composite_crit_chance() const override
//  {
//    auto cc = lisa_spell_t::composite_crit_chance();
//
//    if ( p()->talents_enabled( lisa_t::SPONTANEOUS_COMBUSTION ) && rng().roll( spontaneous_chance() ) )
//      cc += 1.0;
//
//    return cc;
//  }
//
//  void tick( dot_t* d ) override
//  {
//    lisa_spell_t::tick( d );
//
//    if ( p()->talents_enabled( lisa_t::AGONIZING_BLAZE ) )
//    {
//      // Currently a dot ticking increases all stacks. This is similar but not identical, but significantly more
//      // performant.
//      p()->get_target_data( d->target )
//          ->debuffs.agonizing_blaze_stacks->increment(
//              p()->get_active_dots( p()->get_target_data( d->target )->dots.searing_blaze ) );
//    }
//
//    p()->resource_gain( RESOURCE_TEMPORAL_OVERCHARGE, energize_amount * p()->cache.spell_haste(), energize_gain( d->state ), this );
//
//    if ( p()->talents_enabled( lisa_t::ROLLING_FLAMES ) )
//    {
//      p()->cooldowns.engulfing_flames->adjust( -p()->talents.rolling_flames_cdr, true );
//    }
//
//    if ( p()->talents_enabled( lisa_t::OUROBOROS ) )
//    {
//      p()->cooldowns.pyromania->adjust(
//          -( d->state->result == RESULT_CRIT ? p()->talents.ouroboros_cdr_crit : p()->talents.ouroboros_cdr ), true );
//    }
//  }
//
//  void last_tick( dot_t* d ) override
//  {
//    lisa_spell_t::last_tick( d );
//    if ( p()->talents_enabled( lisa_t::AGONIZING_BLAZE ) )
//    {
//      p()->get_target_data( d->target )->debuffs.agonizing_blaze_stacks->expire();
//    }
//  }
//};
//
//struct engulfing_flames_t : public lisa_spell_t
//{
//  timespan_t actual_duration;
//  engulfing_flames_t( lisa_t* p, util::string_view options_str = {} )
//    : lisa_spell_t( "engulfing_flames", p, options_str )
//  {
//    id                     = 12;
//    name_str_reporting     = "Engulfing Flames";
//    spell_power_mod.tick   = p->spell_const.engulfing_flames_tick_coeff;
//    dot_duration           = p->spell_const.engulfing_flames_duration;
//    base_tick_time         = p->spell_const.engulfing_flames_period;
//    dot_allow_partial_tick = true;
//    hasted_ticks           = true;
//
//    dot_behavior = DOT_REFRESH_PANDEMIC;
//
//    if ( p->talents_enabled( lisa_t::UNDYING_FLAME ) )
//      dot_duration += p->talents.undying_flame_extension;
//
//    base_execute_time = 1.5_s;
//
//    if ( p->talents_enabled( lisa_t::ROLLING_FLAMES ) && p->talents.rolling_flames_instant )
//    {
//      base_execute_time = 0_s;
//    }
//
//    cooldown->duration = p->spell_const.engufling_flames_cooldown;
//    cooldown->hasted   = false;
//    cooldown->charges  = 1;
//
//    energize_type     = action_energize::NONE;
//    energize_amount   = p->spell_const.engulfing_flames_embers_per_tick;
//    energize_resource = RESOURCE_TEMPORAL_OVERCHARGE;
//
//    if ( p->talents_enabled( lisa_t::FLICKERING_CINDERS ) )
//    {
//      energize_amount *= 1 + p->talents.flickering_cinders_cinder_multiplier;
//    }
//
//    base_crit += p->talents_enabled( lisa_t::FIRESTARTER ) ? p->talents.firestarter_crit_chance : 0.0;
//
//    dot_max_stack = p->options.engulfing_flames_max_stacks;
//
//    actual_duration = dot_duration;
//    if ( dot_max_stack > 1 )
//    {
//      dot_duration = 9999_s;
//    }
//  }
//
//  void tick( dot_t* d ) override
//  {
//    lisa_spell_t::tick( d );
//
//    p()->resource_gain( RESOURCE_TEMPORAL_OVERCHARGE, energize_amount * p()->cache.spell_haste() * d->current_stack(),
//                        energize_gain( d->state ), this );
//  }
//
//  double spontaneous_chance() const
//  {
//    return p()->talents.spontaneous_combustion_chance +
//           0.01 * p()->cache.spell_crit_chance() / p()->talents.spontaneous_combustion_extra_chance;
//  }
//
//  double composite_crit_chance() const override
//  {
//    auto cc = lisa_spell_t::composite_crit_chance();
//
//    if ( p()->talents_enabled( lisa_t::SPONTANEOUS_COMBUSTION ) && rng().roll( spontaneous_chance() ) )
//      cc += 1.0;
//
//    return cc;
//  }
//
//  void schedule_decrement_event( timespan_t time, dot_t* dot )
//  {
//    p()->get_target_data( dot->target )->dots.engulfing_decrement_events.push_back( make_event( sim, time, [ dot ] {
//      dot->decrement( 1 );
//    } ) );
//  }
//
//  void trigger_dot( action_state_t* s ) override
//  {
//    lisa_spell_t::trigger_dot( s );
//
//    if ( dot_max_stack > 1 )
//    {
//      dot_t* dot = get_dot( s->target );
//      schedule_decrement_event( actual_duration, dot );
//    }
//  }
//
//  void execute() override
//  {
//    lisa_spell_t::execute();
//  }
//
//  void impact( action_state_t* s ) override
//  {
//    lisa_spell_t::impact( s );
//  }
//};
//
//struct apocalypse_t : public lisa_spell_t
//{
//  apocalypse_t( util::string_view name, lisa_t* p, util::string_view options_str = {} )
//    : lisa_spell_t( name, p, options_str )
//  {
//    id                 = 13;
//    name_str_reporting = "Apocalypse";
//
//    spell_power_mod.direct = p->spell_const.apocalypse_coeff;
//
//    base_execute_time = p->spell_const.apocalypse_cast_time;
//
//    aoe                 = -1;
//    reduced_aoe_targets = p->spell_const.apocalypse_falloff;
//
//    cooldown->duration = p->spell_const.apocalypse_cooldown;
//    cooldown->hasted   = false;
//    cooldown->charges  = 1;
//  }
//
//  void impact( action_state_t* s ) override
//  {
//    lisa_spell_t::impact( s );
//    p()->actions.searing_blaze->execute_on_target( s->target );
//  }
//
//  void execute() override
//  {
//    lisa_spell_t::execute();
//
//    if ( p()->legendary.brimstone_cataclysm )
//    {
//      timespan_t cdr = std::min( p()->legendary.brimstone_cataclysm_cdr_cap,
//                                 p()->legendary.brimstone_cataclysm_cdr_per_hit * execute_state->n_targets );
//
//      p()->cooldowns.apocalypse->adjust( -cdr, false );
//    }
//  }
//};
//
//struct fire_ball_t : public lisa_spell_t
//{
//  struct fire_ball_dot_t : public residual_action::residual_periodic_action_t<lisa_spell_t>
//  {
//    fire_ball_dot_t( util::string_view name, lisa_t* p ) : residual_action_t( fmt::format( "{}_dot", name ), p )
//    {
//      id = 14;
//
//      background = true;
//
//      name_str_reporting = "Fire Ball (DoT)";
//
//      tick_may_crit = p->talents_enabled( lisa_t::FIRESTARTER );
//
//      dot_duration           = p->spell_const.fire_ball_dot_duration;
//      dot_behavior           = DOT_REFRESH_DURATION;
//      base_tick_time         = p->spell_const.fire_ball_dot_period;
//      hasted_ticks           = true;
//      dot_allow_partial_tick = true;
//
//      base_multiplier *= p->spell_const.fire_ball_damage_to_dot;
//
//      energize_type     = action_energize::PER_TICK;
//      energize_amount   = p->spell_const.fire_ball_embers_per_tick;
//      energize_resource = RESOURCE_TEMPORAL_OVERCHARGE;
//
//      base_crit = p->talents_enabled( lisa_t::FIRESTARTER ) ? p->talents.firestarter_crit_chance : 0.0;
//    }
//
//    void snapshot_state( action_state_t* state, result_amount_type rt ) override
//    {
//      spell_t::snapshot_state( state, rt );
//    }
//
//    double composite_ta_multiplier( const action_state_t* s ) const override
//    {
//      return fs_player_action_t::action_multiplier();
//    }
//
//    void init() override
//    {
//      base_t::init();
//      snapshot_flags &= STATE_NO_MULTIPLIER;
//      update_flags &= STATE_NO_MULTIPLIER;
//      snapshot_flags |= STATE_HASTE | STATE_MUL_TA;
//      update_flags &= ~STATE_HASTE;
//      snapshot_flags &= ~STATE_TGT_CRIT;
//      update_flags &= ~STATE_TGT_CRIT;
//
//      if ( p()->talents_enabled( lisa_t::FIRESTARTER ) )
//      {
//        snapshot_flags |= STATE_CRIT;
//      }
//    }
//
//    double composite_crit_chance() const override
//    {
//      return base_crit;
//    }
//
//    void tick( dot_t* d ) override
//    {
//      residual_action_t::tick( d );
//
//      if ( p()->talents_enabled( lisa_t::SLOW_BURN ) )
//      {
//        auto td = p()->get_target_data( d->target );
//        td->dots.engulfing_flames->adjust_duration( p()->talents.slow_burn_extend );
//        auto& vec = td->dots.engulfing_decrement_events;
//        for ( size_t i = 0; i < vec.size(); )
//        {
//          if ( vec[ i ]->remains() < 0_s || vec[ i ]->canceled )
//          {
//            erase_unordered( vec, vec.begin() + i );
//          }
//          else
//          {
//            //vec[ i ]->reschedule()
//            vec[ i ]->reschedule_time = vec[ i ]->occurs() + p()->talents.slow_burn_extend;
//            i++;
//          }
//        }
//
//        td->dots.searing_blaze->adjust_duration( p()->talents.slow_burn_extend );
//      }
//    }
//  };
//
//  fire_ball_dot_t* dot_action;
//  fire_ball_t( util::string_view name, lisa_t* p, util::string_view options_str = {} )
//    : lisa_spell_t( name, p, options_str )
//  {
//    id                 = 14;
//    name_str_reporting = "Fire Ball";
//
//    spell_power_mod.direct = p->spell_const.fire_ball_coeff;
//
//    base_execute_time = 0_s;
//
//    aoe                 = -1;
//    reduced_aoe_targets = p->spell_const.fire_ball_falloff;
//
//    cooldown->duration = p->spell_const.fire_ball_cooldown;
//    cooldown->hasted   = true;
//    cooldown->charges  = 2;
//
//    dot_action = new fire_ball_dot_t( name, p );
//    add_child( dot_action );
//
//    if ( p->talents_enabled( lisa_t::GREAT_BALLS_OF_FIRE ) )
//    {
//      base_multiplier *= 1.0 + p->talents.great_balls_of_fire_amp;
//    }
//  }
//
//  double composite_crit_chance() const override
//  {
//    double c = lisa_spell_t::composite_crit_chance();
//
//    if ( p()->buffs.reign_of_fire->check() )
//    {
//      c += p()->buffs.reign_of_fire->check_value();
//    }
//
//    return c;
//  }
//
//  void impact( action_state_t* s ) override
//  {
//    lisa_spell_t::impact( s );
//    residual_action::trigger( dot_action, s->target, s->result_amount );
//  }
//
//  void execute() override
//  {
//    lisa_spell_t::execute();
//    p()->buffs.reign_of_fire->decrement();
//  }
//};
//
//struct pyromania_t : public lisa_spell_t
//{
//  pyromania_t( util::string_view name, lisa_t* p, util::string_view options_str = {} )
//    : lisa_spell_t( name, p, options_str )
//  {
//    id = 22;
//
//    name_str_reporting = "Pyromania";
//
//    trigger_gcd = timespan_t::zero();
//
//    aoe = p->spell_const.pyromania_targets;
//
//    cooldown->duration = p->spell_const.pyromania_cooldown;
//    cooldown->hasted   = false;
//    cooldown->charges  = 1;
//  }
//
//  void impact( action_state_t* s ) override
//  {
//    lisa_spell_t::impact( s );
//
//    p()->actions.engulfing_flames->execute_on_target( s->target );
//  }
//
//  void execute() override
//  {
//    if ( target_list().size() > 1 )
//    {
//      auto partition = std::partition( target_list().begin() + 1, target_list().end(), [ this ]( player_t* a ) {
//        return !p()->get_target_data( a )->dots.engulfing_flames->is_ticking();
//      } );
//
//      std::sort( target_list().begin() + 1, partition,
//                 []( player_t* a, player_t* b ) { return a->current_health() > b->current_health(); } );
//
//      std::sort( partition, target_list().end(),
//                 []( player_t* a, player_t* b ) { return a->current_health() > b->current_health(); } );
//    }
//
//    lisa_spell_t::execute();
//  }
//};
//
//struct fire_frog_hit_t : public lisa_spell_t
//{
//  struct fire_frog_dot_t : public residual_action::residual_periodic_action_t<lisa_spell_t>
//  {
//    fire_frog_dot_t( lisa_t* p ) : residual_action_t( "fire_frog_dot", p )
//    {
//      id = 18;
//
//      background = true;
//
//      name_str_reporting = "Fire Frog (DoT)";
//
//      tick_may_crit = p->talents_enabled( lisa_t::FIRESTARTER );
//
//      dot_duration           = p->spell_const.fire_frog_dot_duration;
//      dot_behavior           = DOT_REFRESH_DURATION;
//      base_tick_time         = p->spell_const.fire_frog_dot_period;
//      hasted_ticks           = true;
//      dot_allow_partial_tick = true;
//
//      base_multiplier *= p->spell_const.fire_frog_coeff;
//
//      base_crit = p->talents_enabled( lisa_t::FIRESTARTER ) ? p->talents.firestarter_crit_chance : 0.0;
//    }
//
//    void snapshot_state( action_state_t* state, result_amount_type rt ) override
//    {
//      spell_t::snapshot_state( state, rt );
//    }
//
//    double composite_ta_multiplier( const action_state_t* s ) const override
//    {
//      return fs_player_action_t::action_multiplier();
//    }
//
//    void init() override
//    {
//      base_t::init();
//      snapshot_flags &= STATE_NO_MULTIPLIER;
//      update_flags &= STATE_NO_MULTIPLIER;
//      snapshot_flags |= STATE_HASTE | STATE_MUL_TA;
//      update_flags &= ~STATE_HASTE;
//      snapshot_flags &= ~STATE_TGT_CRIT;
//      update_flags &= ~STATE_TGT_CRIT;
//
//      if ( p()->talents_enabled( lisa_t::FIRESTARTER ) )
//      {
//        snapshot_flags |= STATE_CRIT;
//      }
//    }
//
//    double composite_crit_chance() const override
//    {
//      return base_crit;
//    }
//  };
//
//  fire_frog_dot_t* dot_action;
//  fire_frog_hit_t( lisa_t* p ) : lisa_spell_t( "fire_frog_hit", p )
//  {
//    id                 = 17;
//    name_str_reporting = "Fire Frog";
//
//    spell_power_mod.direct = p->spell_const.fire_frog_coeff;
//    background             = true;
//
//    dot_action = new fire_frog_dot_t( p );
//  }
//
//  void impact( action_state_t* s ) override
//  {
//    lisa_spell_t::impact( s );
//    residual_action::trigger( dot_action, s->target, s->result_amount );
//  }
//  void execute() override
//  {
//    // Always target a random target
//    target = rng().range( target_list() );
//    lisa_spell_t::execute();
//  }
//};
//
//struct fire_frog_t : public lisa_spell_t
//{
//  int frog_hits;
//  fire_frog_t( lisa_t* p ) : lisa_spell_t( "fire_frog", p ), frog_hits( p->spell_const.fire_frog_max_jumps )
//  {
//    id                 = 16;
//    name_str_reporting = "Fire Frog";
//
//    background = true;
//
//    if ( p->talents_enabled( lisa_t::FROG_SQUAD ) )
//    {
//      frog_hits += p->talents.frog_squad_extra_hits;
//    }
//  }
//
//  void execute() override
//  {
//    lisa_spell_t::execute();
//
//    for ( int i = 0; i < frog_hits; i++ )
//    {
//      timespan_t frog_delay                     = ( 0.5_s + p()->spell_const.fire_frog_jump_duration * i );
//      p()->actions.fire_frogs_hit->travel_delay = frog_delay.total_seconds();
//      p()->actions.fire_frogs_hit->execute();
//    }
//  }
//};
//
//struct fire_frogs_t : public lisa_spell_t
//{
//  int frogs;
//  fire_frogs_t( util::string_view name, lisa_t* p, util::string_view options_str = {} )
//    : lisa_spell_t( name, p, options_str ), frogs( p->spell_const.fire_frog_frogs )
//  {
//    id                 = 15;
//    name_str_reporting = "Fire Frogs";
//
//    base_execute_time = 0_s;
//
//    cooldown->duration = p->spell_const.fire_frogs_cooldown;
//    cooldown->hasted   = false;
//    cooldown->charges  = 1;
//
//    if ( p->talents_enabled( lisa_t::FROG_SQUAD ) )
//    {
//      frogs += p->talents.frog_squad_extra_frogs;
//    }
//
//    if ( !p->actions.fire_frog->stats->parent )
//      add_child( p->actions.fire_frog );
//  }
//
//  void execute() override
//  {
//    lisa_spell_t::execute();
//
//    for ( int i = 0; i < frogs; i++ )
//    {
//      p()->actions.fire_frog->execute();
//    }
//  }
//};

}  // namespace actions

// ==========================================================================
// Rogue Targetdata Definitions
// ==========================================================================

lisa_td_t::lisa_td_t( player_t* target, lisa_t* source )
  : fellowship::fs_player_td_t( target, source ), dots(), debuffs()
{
  dots.splinter_of_time = target->get_dot( "splinter_of_time", source );
  dots.time_burn        = target->get_dot( "time_burn", source );

  debuffs.chrono_bind = make_buff( *this, "chrono_bind" )->set_duration( source->spell_const.chrono_bind_duration );

  debuffs.shifting_sands = make_buff( *this, "shifting_sands" )
                               ->set_duration( source->spell_const.shifting_sands_debuff_duration )
                               ->set_default_value( source->spell_const.shifting_sands_debuff_amp );

}

// ==========================================================================
// Rogue Character Definition
// ==========================================================================

// lisa_t::composite_attribute_multiplier ==================================

double lisa_t::composite_attribute_multiplier( attribute_e a ) const
{
  double am = fs_player_t::composite_attribute_multiplier( a );

  return am;
}
// lisa_t::composite_spell_haste ==========================================

double lisa_t::composite_spell_haste() const
{
  double h = fs_player_t::composite_spell_haste();

  return h;
}

// lisa_t::composite_spell_crit_chance =========================================

double lisa_t::composite_spell_crit_chance() const
{
  double crit = fs_player_t::composite_spell_crit_chance();

  return crit;
}

// lisa_t::composite_damage_versatility ===================================

double lisa_t::composite_damage_versatility() const
{
  double cdv = fs_player_t::composite_damage_versatility();

  return cdv;
}

// lisa_t::composite_heal_versatility =====================================

double lisa_t::composite_heal_versatility() const
{
  double chv = fs_player_t::composite_heal_versatility();

  return chv;
}

// lisa_t::composite_leech ===============================================

double lisa_t::composite_leech() const
{
  double l = fs_player_t::composite_leech();

  return l;
}

// lisa_t::matching_gear_multiplier ========================================

double lisa_t::matching_gear_multiplier( attribute_e attr ) const
{
  return 0.0;
}

// lisa_t::composite_player_multiplier =====================================

double lisa_t::composite_player_multiplier( school_e school ) const
{
  double m = fs_player_t::composite_player_multiplier( school );

  return m;
}

// lisa_t::composite_player_pet_damage_multiplier ==========================

double lisa_t::composite_player_pet_damage_multiplier( const action_state_t* s, bool guardian ) const
{
  double m = fs_player_t::composite_player_pet_damage_multiplier( s, guardian );

  return m;
}

// lisa_t::composite_player_target_multiplier ==============================

double lisa_t::composite_player_target_multiplier( player_t* target, school_e school ) const
{
  double m = fs_player_t::composite_player_target_multiplier( target, school );

  lisa_td_t* tdata = get_target_data( target );

  m += 1.0 + tdata->debuffs.shifting_sands->check_value();

  return m;
}

// lisa_t::composite_player_target_crit_chance =============================

double lisa_t::composite_player_target_crit_chance( player_t* target ) const
{
  double c = fs_player_t::composite_player_target_crit_chance( target );

  return c;
}

// lisa_t::composite_player_target_armor ===================================

double lisa_t::composite_player_target_armor( player_t* target ) const
{
  return 0.0;

  double a = fs_player_t::composite_player_target_armor( target );

  return a;
}
// lisa_t::init_actions ====================================================

void lisa_t::init_action_list()
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

// lisa_t::create_action  ==================================================

action_t* lisa_t::create_action( util::string_view name, util::string_view options_str )
{
  using namespace actions;

  if ( name == "time_shard" )
    return new time_shard_t( name, this, options_str );
  //if ( name == "detonate" )
  //  return new detonate_t( name, this, options_str );
  //if ( name == "wildfire" )
  //  return new wildfire_t( name, this, options_str );
  //if ( name == "incinerate" )
  //  return new incinerate_t( name, this, options_str );
  //if ( name == "searing_blaze" || name == "sb" )
  //  return new searing_blaze_t( "searing_blaze", this, options_str );
  //if ( name == "engulfing_flames" || name == "ef" )
  //  return new engulfing_flames_t( this, options_str );
  //if ( name == "apocalypse" )
  //  return new apocalypse_t( name, this, options_str );
  //if ( name == "fire_ball" || name == "fireball" )
  //  return new fire_ball_t( "fire_ball", this, options_str );
  //if ( name == "pyromania" || name == "pyro" )
  //  return new pyromania_t( "pyromania", this, options_str );
  //if ( name == "frogs" || name == "fire_frog" || name == "fire_frogs" )
  //  return new fire_frogs_t( "fire_frogs", this, options_str );

  return fs_player_t::create_action( name, options_str );
}

// lisa_t::create_expression ===============================================

std::unique_ptr<expr_t> lisa_t::create_action_expression( action_t& action, std::string_view name_str )
{
  // auto split = util::string_split<util::string_view>( name_str, "." );

  return fs_player_t::create_action_expression( action, name_str );
}

std::unique_ptr<expr_t> lisa_t::create_expression( util::string_view name_str )
{
  auto split = util::string_split<util::string_view>( name_str, "." );

  if ( split[ 0 ] == "temporal_overcharge" || split[ 0 ] == "to" || split[ 0 ] == "t_oc" || split[ 0 ] == "tmp_oc" ||
       split[ 0 ] == "oc" || split[ 0 ] == "overcharge" )
  {
    if ( split.size() == 1 )
    {
      return make_fn_expr( name_str, [ this ] { return this->current_temporal_overcharge( true ); } );
    }

    if ( split.size() == 2 && split[ 1 ] == "deficit" )
    {
      return make_fn_expr( name_str,
                           [ this ] { return resources.max[ RESOURCE_TEMPORAL_OVERCHARGE ] - this->current_temporal_overcharge( true ); } );
    }
  }
  else if ( util::str_compare_ci( split[ 0 ], "talent" ) )
  {
    if ( split.size() == 2 )
    {
      for ( lisa_talents_t t = static_cast<lisa_talents_t>( 1U ); t < lisa_talents_t::MAX; t++ )
      {
        if ( util::str_compare_ci( split[ 1 ], talent_name( t ) ) )
        {
          return make_fn_expr( name_str, std::bind( std::mem_fn( &lisa_t::talents_enabled ), this, t ) );
        }
      }
    }
  }
  else if ( util::str_compare_ci( split[ 0 ], "legendary" ) )
  {
    if ( split.size() == 2 )
    {
      //if ( util::str_compare_ci( split[ 1 ], "brimstone_cataclysm" ) )
      //  return make_ref_expr( name_str, legendary.brimstone_cataclysm );
      //if ( util::str_compare_ci( split[ 1 ], "explosive_potency" ) )
      //  return make_ref_expr( name_str, legendary.explosive_potency );
      //if ( util::str_compare_ci( split[ 1 ], "untamed_flame" ) )
      //  return make_ref_expr( name_str, legendary.untamed_flame );
    }
  }

  return fs_player_t::create_expression( name_str );
}

std::unique_ptr<expr_t> lisa_t::create_resource_expression( util::string_view name_str )
{
  return fs_player_t::create_resource_expression( name_str );
}

// lisa_t::init_base =======================================================

void lisa_t::init_base_stats()
{
  if ( base.distance < 1 )
    base.distance = 25;

  fs_player_t::init_base_stats();

  base.stats.attribute[ STAT_INTELLECT ] = 1000;
  base.stats.attribute[ STAT_STAMINA ]   = 1000;
  resources.base[ RESOURCE_HEALTH ]      = 16180;

  base.health_per_stamina = 38.005;

  resources.base[ RESOURCE_TEMPORAL_OVERCHARGE ]   = 100;
  resources.base[ RESOURCE_MANA ]                  = 1444;
  resources.base_regen_per_second[ RESOURCE_MANA ] = 0.005 * resources.base[ RESOURCE_MANA ];

  base_gcd = timespan_t::from_seconds( 1.5 );
  min_gcd  = timespan_t::from_seconds( 0.75 );
}

// lisa_t::init_spells =====================================================

void lisa_t::init_spells()
{
  fs_player_t::init_spells();

  // actions.auto_attack = new actions::auto_melee_attack_t( this, "" );
}

// lisa_t::init_talents ====================================================

void lisa_t::init_talents()
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

    for ( lisa_talents_t t = static_cast<lisa_talents_t>( 1U ); t < lisa_talents_t::MAX; t++ )
    {
      if ( util::str_compare_ci( talent_split[ 0 ], talent_name( t ) ) )
      {
        set_talent_points( t, ranks >= 1 );
        break;
      }
    }
  }
}

// lisa_t::init_gains ======================================================

void lisa_t::init_gains()
{
  fs_player_t::init_gains();

  gains.spirit_procs = get_gain( "Spirit Procs" );
}

// lisa_t::init_procs ======================================================

void lisa_t::init_procs()
{
  fs_player_t::init_procs();
}

// lisa_t::init_rng ========================================================
void lisa_t::init_rng()
{
  fs_player_t::init_rng();
}

// lisa_t::init_scaling ====================================================

void lisa_t::init_scaling()
{
  fs_player_t::init_scaling();

  scaling->disable( STAT_STRENGTH );
  scaling->disable( STAT_AGILITY );

  // Break out early if scaling is disabled on this player, or there's no
  // scaling stat
  if ( !scale_player || sim->scaling->scale_stat == STAT_NONE )
  {
    return;
  }
}

// lisa_t::init_resources =================================================

void lisa_t::init_resources( bool force )
{
  fs_player_t::init_resources( force );
}

// lisa_t::init_buffs ======================================================

void lisa_t::create_buffs()
{
  fs_player_t::create_buffs();

  //buffs.reign_of_fire = make_buff<lisa_buff_t>( this, "reign_of_fire" )
  //                          ->set_duration( talents.reign_of_fire_duration )
  //                          ->set_default_value( talents.reign_of_fire_crit_chance )
  //                          ->set_refresh_behavior( buff_refresh_behavior::DURATION );

  //buffs.wildfire = make_buff<lisa_buff_t>( this, "wildfire" )
  //                     ->set_duration( spell_const.wildfire_duration )
  //                     ->set_default_value( spell_const.wildfire_tickrate )
  //                     ->set_refresh_behavior( buff_refresh_behavior::DURATION )
  //                     ->add_invalidate( CACHE_HASTE );

  //buffs.untamed_flame_crit = make_buff<lisa_buff_t>( this, "untamed_flame_crit" )
  //                               ->set_default_value( legendary.untamed_flame_crit_chance )
  //                               ->set_pct_buff_type( STAT_PCT_BUFF_CRIT );

  //buffs.untamed_flame_spirit = make_buff<lisa_buff_t>( this, "untamed_flame_spirit" )
  //                                 ->set_default_value( legendary.untamed_flame_crit_chance )
  //                                 ->set_pct_buff_type( STAT_PCT_BUFF_MASTERY );

  //if ( legendary.untamed_flame )
  //{
  //  buffs.wildfire->add_stack_change_callback( [ this ]( buff_t*, int, int _new ) {
  //    if ( _new )
  //    {
  //      buffs.untamed_flame_crit->trigger();
  //      buffs.untamed_flame_spirit->trigger();
  //    }
  //    else
  //    {
  //      buffs.untamed_flame_crit->expire();
  //      buffs.untamed_flame_spirit->expire();
  //    }
  //  } );
  //}
}

// lisa_t::invalidate_cache =========================================

void lisa_t::invalidate_cache( cache_e c )
{
  fs_player_t::invalidate_cache( c );
}

void lisa_t::create_options()
{
  fs_player_t::create_options();

  /*add_option( opt_bool( "talent.rolling_flames_instant", talents.rolling_flames_instant ) );
  add_option( opt_bool( "talent.double_detonate_cost_efficiency", talents.double_detonate_cost_efficiency ) );
  add_option( opt_uint( "engulfing_flames_max_stacks", options.engulfing_flames_max_stacks ) );

  add_option( opt_float( "talent.reign_of_fire_ppm", talents.reign_of_fire_ppm ) );*/

  /*add_option( opt_bool( "talent.chilling_finesse", talents.chilling_finesse ) );
  add_option( opt_bool( "talent.winters_embrace", talents.winters_embrace ) );
  add_option( opt_bool( "talent.glacial_assault", talents.glacial_assault ) );

  add_option( opt_bool( "talent.burstbolter", talents.burstbolter ) );
  add_option( opt_bool( "talent.supreme_torrent", talents.supreme_torrent ) );
  add_option( opt_bool( "talent.navirs_keeper", talents.navirs_keeper ) );

  add_option( opt_bool( "talent.icy_flow", talents.icy_flow ) );
  add_option( opt_bool( "talent.avalanche", talents.avalanche ) );
  add_option( opt_bool( "talent.coalescing_frost", talents.coalescing_frost ) );

  add_option( opt_bool( "talent.greater_glacial_blast", talents.greater_glacial_blast ) );

  add_option( opt_bool( "talent.cascading_blitz", talents.cascading_blitz ) );
  add_option( opt_bool( "talent.frostweavers_wrath", talents.frostweavers_wrath ) );
  add_option( opt_bool( "talent.soulfrost_torrent", talents.soulfrost_torrent ) );

  add_option( opt_bool( "talent.biting_cold", talents.biting_cold ) );
  add_option( opt_bool( "talent.wisdom_of_the_north", talents.wisdom_of_the_north ) );*/

  /*add_option( opt_bool( "legendary.brimstone_cataclysm", legendary.brimstone_cataclysm ) );
  add_option( opt_bool( "legendary.explosive_potency", legendary.explosive_potency ) );
  add_option( opt_bool( "legendary.untamed_flame", legendary.untamed_flame ) );*/
}

// lisa_t::copy_from =======================================================

void lisa_t::copy_from( player_t* source )
{
  lisa_t* lisa = static_cast<lisa_t*>( source );
  fs_player_t::copy_from( source );

  talents   = lisa->talents;
  legendary = lisa->legendary;
  options   = lisa->options;
}

// lisa_t::create_profile  =================================================

std::string lisa_t::create_profile( save_e stype )
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

// lisa_t::init_items ======================================================

void lisa_t::init_items()
{
  fs_player_t::init_items();
}

// lisa_t::init_special_effects ============================================

void lisa_t::init_special_effects()
{
  fs_player_t::init_special_effects();

  /*if ( talents_enabled( PYROPHIBIAN_FRENZY ) )
  {
    auto effect          = new special_effect_t( this );
    effect->spell_id     = 9120102;
    effect->name_str     = "pyrophibian_frenzy";
    effect->proc_flags_  = PF_PERIODIC;
    effect->proc_flags2_ = PF2_CRIT;
    effect->proc_chance_ = talents.pyrophibian_frenzy_chance;

    special_effects.push_back( effect );

    effect->execute_action = actions.fire_frog;

    auto dbc = new dbc_proc_callback_t( this, *effect );

    dbc->initialize();
    dbc->activate();
  }*/
}

// lisa_t::init_finished ===================================================

void lisa_t::init_finished()
{
  fs_player_t::init_finished();
}

void lisa_t::init_background_actions()
{
  fs_player_t::init_background_actions();

  //actions.searing_blaze    = new actions::searing_blaze_t( "searing_blaze", this );
  //actions.engulfing_flames = new actions::engulfing_flames_t( this );

  //actions.fire_frogs_hit    = new actions::fire_frog_hit_t( this );
  //actions.fire_frog         = new actions::fire_frog_t( this );
  //actions.crackling_inferno = new actions::crackling_inferno_t( this );
  //actions.flare_up          = new actions::flare_up_t( this );
}

// lisa_t::reset ===========================================================

void lisa_t::reset()
{
  fs_player_t::reset();

  //for ( auto enemy : sim->target_list )
  //{
  //  get_target_data( enemy )->dots.engulfing_decrement_events.clear();
  //}
}

// lisa_t::activate ========================================================

void lisa_t::activate()
{
  fs_player_t::activate();
}

// lisa_t::arise ===========================================================

void lisa_t::arise()
{
  fs_player_t::arise();
}

// lisa_t::combat_begin ====================================================

void lisa_t::combat_begin()
{
  fs_player_t::combat_begin();
}

double lisa_t::resource_gain( resource_e resource_type, double amount, gain_t* source, action_t* action )
{
  double actual_amount = fs_player_t::resource_gain( resource_type, amount, source, action );

  return actual_amount;
}

// lisa_t::non_stacking_movement_modifier ==================================

double lisa_t::non_stacking_movement_modifier() const
{
  double ms = fs_player_t::non_stacking_movement_modifier();

  return ms;
}

// lisa_t::stacking_movement_modifier===================================

double lisa_t::stacking_movement_modifier() const
{
  double ms = fs_player_t::stacking_movement_modifier();

  return ms;
}

template <typename Base>
void actions::lisa_action_t<Base>::trigger_spirit_refund( const action_state_t* state, double resource_refund )
{
  make_event( ab::sim, 200_ms, [ resource_refund, this ] {
    p()->resource_gain( RESOURCE_MANA, resource_refund, p()->gains.spirit_procs, this );
    p()->resource_gain( RESOURCE_TEMPORAL_OVERCHARGE, p()->spell_const.spirit_proc_resource, p()->gains.spirit_procs,
                        this );
    p()->sim->print_debug( "{} actually refunded {:.0f} Mana ", *p(), resource_refund );
  } );

  p()->spirit_refund();
}

template <typename Base>
void actions::lisa_action_t<Base>::spend_resource_costs( const action_state_t* s )
{
  double mana_spent = s->action->base_costs[ RESOURCE_MANA ];
  if ( mana_spent <= 0 )
    return;

  if ( p()->rng().roll( p()->cache.mastery_value() ) )
  {
    p()->sim->print_debug( "{} proc'd Spirit Refund (Chance: {:.2f}%, Sprit: {:.2f}%)", *p(),
                           p()->cache.mastery_value() * 100.0, p()->cache.mastery() * 100.0 );

    trigger_spirit_refund( s, mana_spent );
  }
}

// lisa_t::convert_hybrid_stat ==============================================

stat_e lisa_t::convert_hybrid_stat( stat_e s ) const
{
  // this converts hybrid stats that either morph based on spec or only work
  // for certain specs into the appropriate "basic" stats
  switch ( s )
  {
    case STAT_STR_AGI_INT:
    case STAT_AGI_INT:
    case STAT_STR_INT:
      return STAT_INTELLECT;
    case STAT_STR_AGI:
      return STAT_NONE;
    case STAT_BONUS_ARMOR:
      return STAT_NONE;
    default:
      return s;
  }
}

void lisa_t::analyze( sim_t& sim )
{
  auto incin = find_action( "incinerate" );

  if ( incin )
  {
    // Set to channeled to add the time spent of the channeled dual action.
    incin->channeled = true;
  }

  player_t::analyze( sim );

  if ( incin )
  {
    incin->channeled = false;
  }
}

void lisa_t::create_cooldowns()
{
}

class lisa_module_t : public module_t
{
public:
  lisa_module_t() : module_t( LISA )
  {
  }

  player_t* create_player( sim_t* sim, util::string_view name, race_e r = RACE_NONE ) const override
  {
    return new lisa_t( sim, name, r );
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

}  // namespace lisa
}  // namespace fellowship

const module_t* module_t::lisa()
{
  static fellowship::lisa::lisa_module_t m;
  return &m;
}