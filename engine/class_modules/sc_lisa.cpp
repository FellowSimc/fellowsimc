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
    gain_t* time_rift;
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
    double time_shard_sp_coeff      = 1.2;
    double time_shard_resource      = 10;
    double time_shard_resource_crit = 15;
    double time_shard_mana_cost     = 1;

    double splinter_of_time_sp_coeff = 0.6;
    timespan_t splinter_of_time_dot_duration = 15_s;
    timespan_t splinter_of_time_period       = 2_s;
    double splinter_of_time_tick_sp_coeff       = 0.2;
    double splinter_of_time_tick_resource    = 3.5;
    double splinter_of_time_tick_resource_crit    = 4.5;
    double splinter_of_time_mana_cost          = 3;

    timespan_t shifting_sands_cast_time       = 2_s;
    double shifting_sands_sp_coeff            = 7.2;
    double shifting_sands_resource            = 50;
    double shifting_sands_resource_crit       = 70;
    timespan_t shifting_sands_cd              = 45_s;
    bool shifting_sands_cd_hasted             = false;  // TODO: Confirm
    timespan_t shifting_sands_debuff_duration = 15_s;
    double shifting_sands_debuff_amp          = 0.2;
    double shifting_sands_mana_cost           = 70;

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

    timespan_t time_burn_cast_time      = 2_s;
    timespan_t time_burn_dot_duration   = 21_s;
    timespan_t time_burn_dot_period     = 3_s;
    double time_burn_tick_sp_coeff      = 4.5;
    timespan_t time_burn_cd             = 30_s;
    bool time_burn_cd_hasted            = false;  // TODO: Confirm
    double time_burn_tick_resource      = 5;
    double time_burn_tick_resource_crit = 10;
    double time_burn_mana_cost          = 40;

    double time_rift_haste = 0.15;
    double time_rift_resource = 2;
    timespan_t time_rift_duration = 20_s;
    double time_rift_mana         = 50;
    timespan_t time_rift_cd       = 90_s;
    bool time_rift_cd_hasted      = false;
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
    int fractions_of_time_targets          = 4;
    timespan_t fractions_of_time_cast_time = 2_s;
    double fractions_of_time_mana_cost     = 10;

    double practical_overcharge_mana_regen = 0.25;
    timespan_t practical_overcharge_duration = 8_s;

    double overcharge_manipulation_threshold = 0.5;
    double overcharge_manipulation_amp       = 0.15;

    double uchronia_threshold = 500;

    timespan_t temporal_paradox_duration = 15_s;
    double temporal_paradox_extra_ticks  = 3;
    int temporal_paradox_burn_aoe        = 2;

    double a_tear_in_time_cdr   = 0.1;
    double a_tear_in_time_haste = 0.1;

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
  double resource_regen_per_second( resource_e ) const override;
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
  double resource_loss( resource_e r, double amount, gain_t* source = nullptr, action_t* a = nullptr ) override;

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

  
  double uchronia_tracker;

  lisa_t( sim_t* sim, util::string_view name, race_e r = RACE_NONE )
    : fs_player_t( sim, name, r, LISA ), target_data(), uchronia_tracker( 0.0 )
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
  double temporal_pct;
  bool temporal_paradox;
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
    temporal_pct = 0.0;
    temporal_paradox = false;
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
    temporal_pct                  = rs->temporal_pct;
    temporal_paradox              = rs->temporal_paradox;
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
    cast_state( state )->temporal_pct     = p()->buffs.absolute_stasis->check() || p()->buffs.uchronia->check()
                                                ? 1.0
                                                : p()->resources.pct( RESOURCE_TEMPORAL_OVERCHARGE );
    cast_state( state )->temporal_paradox = p()->buffs.temporal_paradox->check();
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
    
    base_costs[ RESOURCE_MANA ] = p->spell_const.time_shard_mana_cost;

    parse_options( options_str );
  }
};

struct splinter_of_time_t : public lisa_spell_t
{
  bool hold_action;
  splinter_of_time_t( lisa_t* p, util::string_view options_str = {} )
    : lisa_spell_t( "splinter_of_time", p, options_str ), hold_action( false )
  {
    id                 = 3;
    name_str_reporting = "Splinter of Time";

    spell_power_mod.direct = p->spell_const.splinter_of_time_sp_coeff;
    spell_power_mod.tick   = p->spell_const.splinter_of_time_tick_sp_coeff;
    dot_duration           = p->spell_const.splinter_of_time_dot_duration;
    base_tick_time         = p->spell_const.splinter_of_time_period;
    dot_allow_partial_tick = true;
    hasted_ticks           = true;

    dot_behavior = DOT_REFRESH_PANDEMIC;

    base_execute_time = 0_s;

    overcharge_on_tick      = p->spell_const.splinter_of_time_tick_resource;
    overcharge_on_tick_crit = p->spell_const.splinter_of_time_tick_resource_crit;

    base_costs[ RESOURCE_MANA ] = p->spell_const.splinter_of_time_mana_cost;

    add_option( opt_bool( "hold_action", hold_action ) );
    parse_options( options_str );

    if ( hold_action && p->talents_enabled( lisa_t::TALENT_1 ) )
    {
      base_execute_time = p->talents.fractions_of_time_cast_time;
      aoe                         = p->talents.fractions_of_time_targets;
      base_costs[ RESOURCE_MANA ] = p->talents.fractions_of_time_mana_cost;

    }
  }

  void execute() override
  {
    if ( aoe > 0 && target_list().size() > 1 )
    {
      auto partition = std::partition( target_list().begin() + 1, target_list().end(), [ this ]( player_t* a ) {
        return !p()->get_target_data( a )->dots.splinter_of_time->is_ticking();
      } );

      std::sort( target_list().begin() + 1, partition,
                 []( player_t* a, player_t* b ) { return a->current_health() > b->current_health(); } );

      std::sort( partition, target_list().end(),
                 []( player_t* a, player_t* b ) { return a->current_health() > b->current_health(); } );
    }
    base_t::execute();
  }

  bool action_ready() override
  {
    if ( hold_action && !p()->talents_enabled( lisa_t::TALENT_1 ) )
      return false;

    return base_t::action_ready();
  }

 };

struct time_burn_t : public lisa_spell_t
{
  time_burn_t( lisa_t* p, util::string_view options_str = {} ) : lisa_spell_t( "time_burn", p, options_str )
  {
    id                     = 4;
    name_str_reporting     = "Time Burn";
    spell_power_mod.tick   = p->spell_const.time_burn_tick_sp_coeff;
    dot_duration           = p->spell_const.time_burn_dot_duration;
    base_tick_time         = p->spell_const.time_burn_dot_period;
    dot_allow_partial_tick = true;
    hasted_ticks           = true;

    dot_behavior = DOT_REFRESH_PANDEMIC;

    base_execute_time = p->spell_const.time_burn_cast_time;

    cooldown->duration = p->spell_const.time_burn_cd;
    cooldown->hasted   = p->spell_const.time_burn_cd_hasted;
    cooldown->charges  = 1;

    overcharge_on_tick      = p->spell_const.time_burn_tick_resource;
    overcharge_on_tick_crit = p->spell_const.time_burn_tick_resource_crit;

    base_costs[ RESOURCE_MANA ] = p->spell_const.time_burn_mana_cost;
    
    parse_options( options_str );
  }

  int n_targets() const override
  {
    return p()->buffs.temporal_paradox->check() ? p()->talents.temporal_paradox_burn_aoe : 0;
  }
  void execute() override
  {
    base_t::execute();
    p()->buffs.temporal_paradox->decrement();
  }
};

struct shifting_sands_t : public lisa_spell_t
{
  shifting_sands_t( lisa_t* p, util::string_view options_str = {} ) : lisa_spell_t( "shifting_sands", p, options_str )
  {
    id                     = 5;
    name_str_reporting     = "Shfiting Sands";
    spell_power_mod.direct = p->spell_const.shifting_sands_sp_coeff;
    base_execute_time      = p->spell_const.shifting_sands_cast_time;

    cooldown->duration = p->spell_const.shifting_sands_cd;
    cooldown->hasted   = p->spell_const.shifting_sands_cd_hasted;
    cooldown->charges  = 1;

    overcharge_on_hit  = p->spell_const.shifting_sands_resource;
    overcharge_on_crit = p->spell_const.shifting_sands_resource_crit;

    base_costs[ RESOURCE_MANA ] = p->spell_const.shifting_sands_mana_cost;

    parse_options( options_str );
  }

  void impact( action_state_t* s ) override
  {
    base_t::impact( s );

    p()->get_target_data( s->target )->debuffs.shifting_sands->trigger();
  }

  void execute() override
  {
    base_t::execute();

    if ( p()->talents_enabled( lisa_t::TALENT_13 ) )
    {
      p()->buffs.temporal_paradox->trigger();
    }
  }
};

struct chrono_barrage_t : public lisa_spell_t
{
  double sp_coeff_extra;
  chrono_barrage_t( lisa_t* p, util::string_view options_str = {} )
    : lisa_spell_t( "chrono_barrage", p, options_str ), sp_coeff_extra( 0 )
  {
    id = 6;

    name_str_reporting = "Chrono Barrage";

    spell_power_mod.tick = p->spell_const.chrono_barrage_dmg_sp_coeff_minimum;
    sp_coeff_extra       = p->spell_const.chrono_barrage_dmg_sp_coeff_maximum - spell_power_mod.tick;

    dot_duration           = p->spell_const.chrono_barrage_channel_duration;
    base_tick_time         = p->spell_const.chrono_barrage_channel_period;
    hasted_ticks           = true;
    hasted_dot_duration    = false; // Confirmed :aware:
    dot_allow_partial_tick = true;
    tick_on_application    = false;
    channeled              = true;
    
    base_costs[ RESOURCE_MANA ] = p->spell_const.shifting_sands_mana_cost;
    secondary_costs[ RESOURCE_TEMPORAL_OVERCHARGE ] = 1;

    parse_options( options_str );
  }

  double spell_tick_power_coefficient( const action_state_t* s ) const override
  {
    return spell_power_mod.tick + cast_state( s )->temporal_pct * sp_coeff_extra;
  }

  timespan_t dot_duration_flat_modifier( const action_state_t* s ) const override
  {
    auto t = base_t::dot_duration_flat_modifier( s );

    // This is 3 bolts... I suppose. Not really.
    if ( cast_state( s )->temporal_paradox )
    {
      t *= 2;
    }

    return t;
  }

  double tick_time_pct_multiplier( const action_state_t* s ) const override
  {
    auto base = base_t::tick_time_pct_multiplier( s );

  /*  if ( cast_state( s )->temporal_paradox )
    {
      auto ticks = dot_duration / base_tick_time.base * s->haste;
      base *= ticks / ( p()->talents.temporal_paradox_extra_ticks + 1 );
    }*/

    return base;
  }

  void execute() override
  {
    base_t::execute();

    if ( p()->buffs.uchronia->check() )
    {
      p()->buffs.uchronia->decrement();
    }
    else
    {
      auto resource_available = p()->resources.current[ RESOURCE_TEMPORAL_OVERCHARGE ];
      p()->resource_loss( RESOURCE_TEMPORAL_OVERCHARGE, resource_available, nullptr, this );
      stats->consume_resource( RESOURCE_TEMPORAL_OVERCHARGE, resource_available );
    }

    if ( p()->talents_enabled( lisa_t::TALENT_4 ) )
    {
      p()->buffs.practical_overcharge->trigger();
    }

    p()->buffs.temporal_paradox->decrement();
  }

  void init_finished() override
  {
    base_t::init_finished();

    update_flags &= ~STATE_HASTE;
  }
};

struct time_rift_t : public lisa_spell_t
{
  time_rift_t( lisa_t* p, util::string_view options_str = {} ) : lisa_spell_t( "time_rift", p, options_str )
   {
     id = 7;

     name_str_reporting = "Time Rift";

     cooldown->duration = p->spell_const.time_rift_cd;
     cooldown->hasted   = p->spell_const.time_rift_cd_hasted;
     cooldown->charges  = 1;

     parse_options( options_str );
   }

   void execute() override
   {
     p()->buffs.time_rift->trigger();
     lisa_spell_t::execute();
   }
 };

struct absolute_stasis_t : public lisa_spell_t
 {
  absolute_stasis_t( lisa_t* p, util::string_view options_str = {} ) : lisa_spell_t( "absolute_stasis", p, options_str )
   {
     id = 8;

     name_str_reporting = "Absolute Stasis";

     base_execute_time = 0_s;

     resource_current              = RESOURCE_SPIRIT;
     base_costs[ RESOURCE_SPIRIT ] = 100;

     parse_options( options_str );
   }

   void execute() override
   {
     lisa_spell_t::execute();
     p()->fs_buffs.spirit_of_heroism->trigger();
     p()->buffs.absolute_stasis->trigger();
     p()->used_ultimate();
   }
 };

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

  m *= 1.0 + tdata->debuffs.shifting_sands->check_value();

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
  if ( name == "splinter_of_time" )
    return new splinter_of_time_t( this, options_str );
  if ( name == "time_burn" )
    return new time_burn_t( this, options_str );
  if ( name == "shifting_sands" )
    return new shifting_sands_t( this, options_str );
  if ( name == "chrono_barrage" )
    return new chrono_barrage_t( this, options_str );
  if ( name == "time_rift" )
    return new time_rift_t( this, options_str );
  if ( name == "absolute_stasis" )
    return new absolute_stasis_t( this, options_str );

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
  else if ( util::str_compare_ci( split[ 0 ], "uchronia_energy" ) )
  {
    return make_ref_expr( name_str, uchronia_tracker );
  }

  return fs_player_t::create_expression( name_str );
}

std::unique_ptr<expr_t> lisa_t::create_resource_expression( util::string_view name_str )
{
  return fs_player_t::create_resource_expression( name_str );
}


double lisa_t::resource_regen_per_second( resource_e r ) const
{
  double reg = fs_player_t::resource_regen_per_second( r );

  if ( r == RESOURCE_MANA )
  {
    reg *= 1.0 + buffs.practical_overcharge->check_value();
  }

  return reg;
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
  resources.base[ RESOURCE_MANA ]                  = 1440;
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
  gains.time_rift    = get_gain( "Time Rift" );
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

  buffs.practical_overcharge = make_buff<lisa_buff_t>( this, "practical_overcharge" )
                                   ->set_duration( talents.practical_overcharge_duration )
                                   ->set_default_value( talents.practical_overcharge_mana_regen );

  buffs.uchronia = make_buff<lisa_buff_t>( this, "uchronia" );

  buffs.temporal_paradox =
      make_buff<lisa_buff_t>( this, "temporal_paradox" )->set_duration( talents.temporal_paradox_duration );

  buffs.time_rift =
      make_buff<lisa_buff_t>( this, "time_rift" )
          ->set_duration( spell_const.time_rift_duration )
          ->set_period( spell_const.time_rift_period )
          ->set_default_value( spell_const.time_rift_haste )
          ->set_freeze_stacks( true )
          ->set_pct_buff_type( STAT_PCT_BUFF_HASTE )
          ->set_tick_callback( [ this ]( buff_t*, int, timespan_t ) {
            resource_gain( RESOURCE_TEMPORAL_OVERCHARGE, spell_const.time_rift_resource, gains.time_rift, nullptr );
          } );

  if ( talents_enabled( TALENT_10 ) )
  {
    buffs.time_rift->add_stack_change_callback( [ this ]( buff_t*, int, int _new ) {
      if ( _new )
      {
        for ( auto& action : action_list )
        {
          action->base_recharge_multiplier -= talents.a_tear_in_time_cdr;
          action->cooldown->adjust_recharge_multiplier();
        }
      }
      else
      {
        for ( auto& action : action_list )
        {
          action->base_recharge_multiplier += talents.a_tear_in_time_cdr;
          action->cooldown->adjust_recharge_multiplier();
        }
      }
    } );
  }

  struct absolute_stasis_buff_t : lisa_buff_t
  {
    double cdr_mod;
    double cd_mod;
    absolute_stasis_buff_t( lisa_t* pl )
      : lisa_buff_t( pl, "absolute_stasis" ),
        cdr_mod( pl->spell_const.absolute_stasis_cd_recovery ),
        cd_mod( pl->spell_const.absolute_stasis_cd_reduction )
    {
      set_duration( pl->spell_const.absolute_stasis_duration );
      add_stack_change_callback( [ this ]( buff_t*, int, int _new ) {
        if ( _new )
        {
          for ( auto& action : p()->action_list )
          {
            action->base_recharge_rate_multiplier /= cdr_mod;
            action->base_recharge_multiplier = 1.0 - ( 1.0 - action->base_recharge_multiplier ) * cd_mod;
            action->cooldown->adjust_recharge_multiplier();
          }
        }
        else
        {
          for ( auto& action : p()->action_list )
          {
            action->base_recharge_rate_multiplier *= cdr_mod;
            action->base_recharge_multiplier = 1.0 - ( 1.0 - action->base_recharge_multiplier ) / cd_mod;
            action->cooldown->adjust_recharge_multiplier();
          }
        }
      } );
    }
  };

  buffs.absolute_stasis = make_buff<absolute_stasis_buff_t>( this );


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
  
  uchronia_tracker = 0;

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

double lisa_t::resource_loss( resource_e resource_type, double amount, gain_t* source, action_t* action )
{
  double actual_amount = fs_player_t::resource_loss( resource_type, amount, source, action );

  if ( talents_enabled( TALENT_16 ) && resource_type == RESOURCE_TEMPORAL_OVERCHARGE && actual_amount > 0 )
  {
    uchronia_tracker += actual_amount;
    if ( uchronia_tracker >= talents.uchronia_threshold )
    {
      uchronia_tracker -= talents.uchronia_threshold;
      buffs.uchronia->trigger();
    }
  }

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
  fs_player_t::analyze( sim );
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