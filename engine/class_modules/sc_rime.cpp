#include "fs_player.hpp"
#include "util/util.hpp"

#include "simulationcraft.hpp"

namespace fellowship
{
namespace rime
{

// Forward Declarations
class rime_t;

enum class secondary_trigger
{
  NONE = 0U,
  AVALANCHE
};

namespace actions
{
struct rime_heal_t;
struct rime_spell_t;

struct melee_t;
}  // namespace actions

class rime_td_t : public fs_player_td_t
{
public:
  struct dots_t
  {
    dot_t* bursting_ice;
    dot_t* freezing_torrent;
  } dots;

  struct
  {
    buff_t* coalescing_frost;
  } debuffs;

  rime_td_t( player_t* target, rime_t* source );
};

struct rime_buff_t : public fs_player_buff_t
{
  rime_buff_t( player_t* p, util::string_view name ) : fs_player_buff_t( p, name )
  {
  }

  rime_t* p()
  {
    return debug_cast<rime_t*>( player );
  }

  const rime_t* p() const
  {
    return debug_cast<const rime_t*>( player );
  }
};

class rime_t : public fellowship::fs_player_t
{
public:
  struct actions_t
  {
    actions::rime_spell_t* ice_comet;
    actions::rime_spell_t* flight_of_navir;
    actions::rime_spell_t* frost_swallow;
    actions::rime_spell_t* frost_swallow_navir;
    actions::rime_spell_t* frost_swallow_cascading;
    actions::rime_spell_t* cold_snap;
    actions::rime_spell_t* freezing_torrent;
    actions::rime_spell_t* glacial_blast;
    actions::rime_spell_t* frost_bolt;
    actions::rime_spell_t* bursting_ice;
    actions::rime_spell_t* bursting_ice_tick_burstbolter;
    actions::rime_spell_t* ice_blitz;
    actions::rime_spell_t* wrath_of_winter;
    actions::rime_spell_t* winters_blessing;
    actions::rime_spell_t* coalescing_frost;
    actions::rime_spell_t* ice_comet_avalanche;
  } actions;

  struct buffs_t
  {
    buff_t* ultimate_buff_window;
    buff_t* glacial_assault;
    buff_t* flight_of_the_navir;
    buff_t* icy_flow;
    buff_t* ice_blitz;
    buff_t* winters_embrace;
    buff_t* winters_blessing;
    buff_t* soulfrost_torrent;
    buff_t* frostweavers_wrath;
    buff_t* frostwyrms_spite;
    buff_t* undulating_spirit;
  } buffs;

  struct cooldowns_t
  {
    cooldown_t* cold_snap;
    cooldown_t* freezing_torrent;
    cooldown_t* bursting_ice;
    cooldown_t* ice_blitz;
    cooldown_t* flight_of_the_navir;
    cooldown_t* winters_blessing;
  } cooldowns;

  struct gains_t
  {
    gain_t* spirit_procs;
    gain_t* anima_worbs;
    gain_t* ult_worbs;
  } gains;

  struct procs_t
  {
    proc_t* coal_no_targets;
  } procs;

  struct rppms_t
  {
    real_ppm_t* soulfrost_torrent;
  } rppm;

  struct talents_t
  {
    bool chilling_finesse                                 = false;
    timespan_t chilling_finesse_bursting_ice_cdr_per_tick = 0.3_s;
    timespan_t chilling_finesse_torrent_cdr_per_snap      = 1.5_s;

    bool winters_embrace       = false;
    double winters_embrace_amp = 0.2;

    bool glacial_assault       = false;
    int glacial_assault_stacks = 4;
    double glacial_assault_amp = 0.4;

    bool burstbolter                    = false;
    int burstbolter_additional_anima    = 1;
    int burstbolter_bursting_ice_pulses = 1;

    bool supreme_torrent                = false;
    timespan_t supreme_torrent_duration = 0.8_s;

    bool navirs_keeper           = false;
    int navirs_keeper_cold_snaps = 2;

    bool icy_flow                     = false;
    timespan_t icy_flow_gb_reduction  = 0.5_s;
    timespan_t icy_flow_buff_duration = 8_s;
    double icy_flow_cc                = 0.25;
    int icy_flow_max_stacks           = 2;

    bool avalanche          = false;
    double avalanche_double = 0.15;
    double avalanche_triple = 0.07;

    bool coalescing_frost                     = false;
    timespan_t coalescing_frost_duration      = 3_s;
    double coalescing_frost_sp_coeff          = 0.43;
    double coalescing_frost_crit_extra_chance = 0.5;
    int coalescing_frost_crit_stacks          = 2;
    int coalescing_frost_max_stacks           = 30;

    bool greater_glacial_blast                       = false;
    double greater_glacial_blast_amp                 = 0.4;
    timespan_t greater_glacial_blast_added_cast_time = 0.5_s;

    bool cascading_blitz                          = false;
    int cascading_blitz_birds_per_anima           = 1;
    timespan_t cascading_blitz_extension_per_bird = 0.2_s;

    bool frostweavers_wrath                     = false;
    double frostweavers_wrath_chance_per_orb    = 0.17;
    timespan_t frostweavers_wrath_buff_duration = 12_s;
    double frostweavers_wrath_added_cc          = 1.0;

    bool soulfrost_torrent                     = false;
    timespan_t soulfrost_torrent_buff_duration = 18_s;
    double soulfrost_torrent_tickrate_increase = 1.4;
    double soulfrost_torrent_crit_chance       = 1.0;
    double soulfrost_torrent_rppm              = 1.2;

    bool biting_cold              = false;
    double biting_cold_crit_power = 0.1;

    bool wisdom_of_the_north           = false;
    timespan_t wisdom_of_the_north_cdr = 0.3_s;
  } talents;

  struct legendary_t
  {
    bool frostwyrms_spite                 = false;
    double frostwyrms_spite_dmg_per_stack = 0.20;
    int frostwyrms_spite_max_stacks       = 30;
    timespan_t frostwyrms_spite_duration  = 15_s;

    bool skandis_decree                      = false;
    timespan_t skandis_decree_duration_bonus = 2_s;

    bool undulating_spirit          = false;
    double undulating_spirit_chance = 0.10;
  } legendary;

  struct options_t
  {
  } options;

  target_specific_t<rime_td_t> target_data;

  const rime_td_t* find_target_data( const player_t* target ) const override
  {
    return target_data[ target ];
  }

  rime_td_t* get_target_data( player_t* target ) const override
  {
    rime_td_t*& td = target_data[ target ];
    if ( !td )
    {
      td = new rime_td_t( target, const_cast<rime_t*>( this ) );
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

  double current_worbs( bool /* react */ = false ) const
  {
    return resources.current[ RESOURCE_WINTER_ORB ];
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

  rime_t( sim_t* sim, util::string_view name, race_e r = RACE_NONE ) : fs_player_t( sim, name, r, RIME ), target_data()
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
struct rime_action_state_t : public action_state_t
{
private:
  T_ACTION* action;

public:
  rime_action_state_t( action_t* action, player_t* target )
    : action_state_t( action, target ), action( dynamic_cast<T_ACTION*>( action ) )
  {
  }
  
  rime_t* p() const
  {
    return debug_cast<rime_t*>( action->player );
  }

  rime_t* p()
  {
    return debug_cast<rime_t*>( action->player );
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
    const rime_action_state_t* rs = debug_cast<const rime_action_state_t*>( s );
  }

  T_ACTION* get_action() const
  {
    return action;
  }
};

template <typename Base>
class rime_action_t : public Base
{
protected:
  /// typedef for rime_action_t<action_base_t>
  using base_t = rime_action_t<Base>;

private:
  /// typedef for the templated action type, eg. spell_t, attack_t, heal_t
  using ab = Base;

public:
  secondary_trigger secondary_trigger_type;

  // Init =====================================================================

  rime_action_t( util::string_view n, rime_t* p, util::string_view options = {} )
    : ab( n, p, options ), secondary_trigger_type( secondary_trigger::NONE )
  {
    ab::parse_options( options );
    ab::may_crit = ab::tick_may_crit = true;
    ab::school                       = SCHOOL_FROST;

    // rime_t sets base and min GCD to 1.5_s hasted
    ab::gcd_type = gcd_haste_type::SPELL_HASTE;
  }

  void init() override
  {
    ab::init();
  }

  // Type Wrappers ============================================================

  static const rime_action_state_t<base_t>* cast_state( const action_state_t* st )
  {
    return debug_cast<const rime_action_state_t<base_t>*>( st );
  }

  static rime_action_state_t<base_t>* cast_state( action_state_t* st )
  {
    return debug_cast<rime_action_state_t<base_t>*>( st );
  }

  rime_t* p()
  {
    return debug_cast<rime_t*>( ab::player );
  }

  const rime_t* p() const
  {
    return debug_cast<const rime_t*>( ab::player );
  }

  rime_td_t* td( player_t* t ) const
  {
    return p()->get_target_data( t );
  }

  // Action State =============================================================

  action_state_t* new_state() override
  {
    return new rime_action_state_t<base_t>( this, ab::target );
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
  virtual double generate_anima() const
  {
    double cp = 0;

    if ( ab::energize_type != action_energize::NONE && ab::energize_resource == RESOURCE_ANIMA )
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
  void spend_winter_orbs( const action_state_t* );
  void gain_winter_orb( int );
  void gain_anima( int );
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

    if ( p()->buffs.winters_embrace->check() )
    {
      if ( ab::id != 11 && ab::id != 12 )
      {
        m *= ( 1.0 + p()->buffs.winters_embrace->check_value() );
      }
    }

    return m;
  }

  double composite_ta_multiplier( const action_state_t* state ) const override
  {
    double m = ab::composite_ta_multiplier( state );

    if ( p()->buffs.winters_embrace->check() )
    {
      if ( ab::id != 11 && ab::id != 12 )
      {
        m *= ( 1.0 + p()->buffs.winters_embrace->check_value() );
      }
    }

    return m;
  }

  double composite_target_multiplier( player_t* target ) const override
  {
    double m = ab::composite_target_multiplier( target );

    auto tdata = td( target );

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

    if ( p()->talents.biting_cold )
    {
      cm *= 1.0 + p()->talents.biting_cold_crit_power;
    }

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

  timespan_t tick_time( const action_state_t* state ) const override
  {
    timespan_t tt = ab::tick_time( state );

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

    spend_winter_orbs( ab::execute_state );
  }

  void execute() override
  {
    ab::execute();

    if ( p()->talents.soulfrost_torrent && !is_secondary_action() && !ab::background && !ab::tick_action )
    {
      if ( !p()->buffs.soulfrost_torrent->check() && p()->rppm.soulfrost_torrent->trigger() )
      {
        p()->buffs.soulfrost_torrent->trigger();
      }
    }

     if ( p()->legendary.undulating_spirit && !is_secondary_action() && !ab::tick_action && !ab::background )
     {
       if ( p()->rng().roll( p()->legendary.undulating_spirit_chance ) )
       {
         p()->buffs.undulating_spirit->trigger();
       }
    }
  }

  void schedule_travel( action_state_t* state ) override
  {
    ab::schedule_travel( state );
  }

  bool ready() override
  {
    if ( !ab::ready() )
      return false;

    if ( ab::base_costs[ RESOURCE_WINTER_ORB ] > 0 && p()->current_worbs() < ab::base_costs[ RESOURCE_WINTER_ORB ] )
      return false;

    return true;
  }

  std::unique_ptr<expr_t> create_expression( std::string_view name ) override
  {
    if ( util::str_compare_ci( name, "anima_gain" ) )
    {
      return make_mem_fn_expr( "anima_gain", *this, &base_t::generate_anima );
    }

    return ab::create_expression( name );
  }
};

// ==========================================================================
// Rogue Attack Classes
// ==========================================================================

struct rime_heal_t : public rime_action_t<fellowship::actions::fs_player_action_t<heal_t>>
{
  rime_heal_t( util::string_view n, rime_t* p, util::string_view o = {} ) : base_t( n, p, o )
  {
    harmful = false;
    set_target( p );
  }
};

struct rime_spell_t : public rime_action_t<fellowship::actions::fs_player_action_t<spell_t>>
{
  rime_spell_t( util::string_view n, rime_t* p, util::string_view o = {} ) : base_t( n, p, o )
  {
  }
};

struct frost_bolt_t : public rime_spell_t
{
  frost_bolt_t( util::string_view name, rime_t* p, util::string_view options_str = {} )
    : rime_spell_t( name, p, options_str )
  {
    id = 2;

    spell_power_mod.direct = 2.34;

    name_str_reporting = "Frost Bolt";

    energize_type     = action_energize::ON_CAST;
    energize_resource = RESOURCE_ANIMA;
    energize_amount   = 1.0;

    if ( p->talents.burstbolter )
    {
      energize_amount += p->talents.burstbolter_additional_anima;
      add_child( p->actions.bursting_ice_tick_burstbolter );
    }

    base_execute_time = 1.5_s;
  }

  void execute() override
  {
    rime_spell_t::execute();
  }

  void impact( action_state_t* s ) override
  {
    rime_spell_t::impact( s );

    if ( p()->talents.burstbolter )
    {
      p()->actions.bursting_ice_tick_burstbolter->execute_on_target( s->target );
    }
  }
};

struct glacial_blast_t : public rime_spell_t
{
  glacial_blast_t( util::string_view name, rime_t* p, util::string_view options_str = {} )
    : rime_spell_t( name, p, options_str )
  {
    id                 = 3;
    name_str_reporting = "Glacial Blast";

    resource_current                  = RESOURCE_WINTER_ORB;
    base_costs[ RESOURCE_WINTER_ORB ] = 2;

    base_execute_time = 2_s;

    spell_power_mod.direct = 11.88;

    if ( p->talents.greater_glacial_blast )
    {
      base_execute_time += p->talents.greater_glacial_blast_added_cast_time;
      base_dd_multiplier *= 1 + p->talents.greater_glacial_blast_amp;
    }
  }

  timespan_t execute_time() const override
  {
    if ( p()->buffs.ultimate_buff_window->check() || p()->buffs.glacial_assault->at_max_stacks() )
      return 0_s;

    timespan_t base = base_execute_time.base;

    if ( p()->buffs.icy_flow->check() )
      base -= p()->talents.icy_flow_gb_reduction;

    auto mul = base_execute_time.pct_mul * execute_time_pct_multiplier();
    if ( mul <= 0 )
      return 0_ms;

    base += base_execute_time.flat_add + execute_time_flat_modifier();
    if ( base <= 0_ms )
      return 0_ms;

    // TODO: assumed to be rounded to ms like tick_time(), confirm if possible.
    return timespan_t::from_millis( std::round( static_cast<double>( base.total_millis() ) * mul ) );
  }

  double cost() const override
  {
    if ( p()->buffs.glacial_assault->at_max_stacks() )
    {
      return 0.0;
    }

    return base_t::cost();
  }

  double composite_da_multiplier( const action_state_t* state ) const override
  {
    double m = rime_spell_t::composite_da_multiplier( state );

    if ( p()->buffs.glacial_assault->at_max_stacks() )
      m *= 1 + p()->buffs.glacial_assault->check_value();

    return m;
  }

  double composite_crit_chance() const override
  {
    return rime_spell_t::composite_crit_chance() + p()->buffs.icy_flow->check_value() +
           p()->buffs.frostweavers_wrath->check_value();
  }

  void execute() override
  {
    rime_spell_t::execute();

    if ( p()->buffs.glacial_assault->at_max_stacks() )
    {
      p()->buffs.glacial_assault->expire();
    }

    p()->buffs.frostweavers_wrath->decrement();
    p()->buffs.icy_flow->decrement();
  }
};

struct ice_comet_t : public rime_spell_t
{
  ice_comet_t( util::string_view name, rime_t* p, util::string_view options_str = {},
               secondary_trigger st = secondary_trigger::NONE )
    : rime_spell_t( name, p, options_str )
  {
    id                     = 3;
    name_str_reporting     = "Ice Comet";
    secondary_trigger_type = st;

    resource_current                  = RESOURCE_WINTER_ORB;
    base_costs[ RESOURCE_WINTER_ORB ] = 2;

    spell_power_mod.direct = 4.51;
    aoe                    = -1;
    reduced_aoe_targets    = 8;

    if ( st == secondary_trigger::NONE )
    {
      add_child( p->actions.ice_comet_avalanche );
    }
  }

  double composite_da_multiplier( const action_state_t* state ) const override
  {
    double m = rime_spell_t::composite_da_multiplier( state );

    return m;
  }

  double composite_crit_chance() const override
  {
    return rime_spell_t::composite_crit_chance() + p()->buffs.icy_flow->check_value() +
           p()->buffs.frostweavers_wrath->check_value();
  }

  void execute() override
  {
    rime_spell_t::execute();

    if ( !is_secondary_action() )
    {
      p()->buffs.frostweavers_wrath->decrement();
      p()->buffs.icy_flow->decrement();

      if ( p()->talents.avalanche )
      {
        if ( p()->rng().roll( p()->talents.avalanche_double ) )
        {
          p()->actions.ice_comet_avalanche->trigger_secondary_action( target, 0.3_s );
        }

        if ( p()->rng().roll( p()->talents.avalanche_triple ) )
        {
          p()->actions.ice_comet_avalanche->trigger_secondary_action( target, 0.6_s );
        }
      }
    }
  }

  void impact( action_state_t* s ) override
  {
    rime_spell_t::impact( s );
  }
};

struct ice_blitz_t : public rime_spell_t
{
  ice_blitz_t( util::string_view name, rime_t* p, util::string_view options_str = {} )
    : rime_spell_t( name, p, options_str )
  {
    id = 6;

    name_str_reporting = "Ice Blitz";

    add_child( p->actions.frost_swallow_cascading );

    trigger_gcd = timespan_t::zero();

    cooldown->duration = 120_s;
    cooldown->hasted   = false;
    cooldown->charges  = 1;
  }

  void execute() override
  {
    p()->buffs.ice_blitz->trigger();
    rime_spell_t::execute();
  }
};

struct winters_blessing_t : public rime_spell_t
{
  winters_blessing_t( util::string_view name, rime_t* p, util::string_view options_str = {} )
    : rime_spell_t( name, p, options_str )
  {
    id = 7;

    name_str_reporting = "Winters Blessing";

    trigger_gcd = timespan_t::zero();

    cooldown->duration = 60_s;
    cooldown->hasted   = false;
    cooldown->charges  = 1;
  }

  void execute() override
  {
    p()->buffs.winters_blessing->trigger();
    rime_spell_t::execute();
  }
};

struct flight_of_the_navir_t : public rime_spell_t
{
  flight_of_the_navir_t( util::string_view name, rime_t* p, util::string_view options_str = {} )
    : rime_spell_t( name, p, options_str )
  {
    id = 8;

    name_str_reporting = "Flight of the Navir";

    add_child( p->actions.frost_swallow_navir );

    cooldown->duration = 60_s;
    cooldown->hasted   = false;
    cooldown->charges  = 1;
  }

  void execute() override
  {
    p()->buffs.flight_of_the_navir->trigger();
    rime_spell_t::execute();

    if ( p()->talents.navirs_keeper )
    {
      p()->cooldowns.cold_snap->reset( false, p()->talents.navirs_keeper_cold_snaps );
    }
  }
};

struct wrath_of_winter_t : public rime_spell_t
{
  wrath_of_winter_t( util::string_view name, rime_t* p, util::string_view options_str = {} )
    : rime_spell_t( name, p, options_str )
  {
    id = 9;

    name_str_reporting = "Wrath of Winter";

    base_execute_time = 1.5_s;

    resource_current              = RESOURCE_SPIRIT;
    base_costs[ RESOURCE_SPIRIT ] = 100;
  }

  void execute() override
  {
    rime_spell_t::execute();
    p()->fs_buffs.spirit_of_heroism->trigger();
    p()->buffs.ultimate_buff_window->trigger();
    p()->used_ultimate();
  }
};

struct cold_snap_t : public rime_spell_t
{
  cold_snap_t( util::string_view name, rime_t* p, util::string_view options_str = {} )
    : rime_spell_t( name, p, options_str )
  {
    id = 10;

    name_str_reporting = "Cold Snap";

    cooldown->duration = 12_s;
    cooldown->hasted   = true;
    cooldown->charges  = 2;

    spell_power_mod.direct = 3.648;

    energize_type     = action_energize::ON_CAST;
    energize_resource = RESOURCE_WINTER_ORB;
    energize_amount   = 1;

    aoe = 1;
    if ( p->legendary.frostwyrms_spite )
    {
      reduced_aoe_targets = 3;
    }
  }

  void execute() override
  {
    rime_spell_t::execute();

    if ( p()->legendary.frostwyrms_spite )
    {
      p()->buffs.frostwyrms_spite->expire();
    }

    if ( p()->buffs.flight_of_the_navir->check() )
    {
      for ( auto i = 0; i < 5; i++ )
        p()->actions.frost_swallow_navir->execute();
    }

    if ( p()->talents.glacial_assault )
      p()->buffs.glacial_assault->trigger();

    if ( p()->talents.icy_flow )
      p()->buffs.icy_flow->trigger();

    if ( p()->talents.chilling_finesse )
      p()->cooldowns.freezing_torrent->adjust( -p()->talents.chilling_finesse_torrent_cdr_per_snap, true, false );
  }

  int n_targets() const override
  {
    int n = rime_spell_t::n_targets();

    if ( p()->legendary.frostwyrms_spite )
    {
      n += p()->buffs.frostwyrms_spite->check();
    }

    return n;
  }

  double composite_da_multiplier( const action_state_t* state ) const override
  {
    double m = rime_spell_t::composite_da_multiplier( state );

    if ( p()->legendary.frostwyrms_spite )
    {
      m *= 1.0 + p()->buffs.frostwyrms_spite->check_stack_value();
    }

    return m;
  }
};

struct bursting_ice_tick_t : public rime_spell_t
{
  bursting_ice_tick_t( util::string_view name, rime_t* p ) : rime_spell_t( name, p )
  {
    id = 11;

    name_str_reporting = "Bursting Ice (Tick)";

    aoe                 = -1;
    reduced_aoe_targets = 5;

    background = true;

    spell_power_mod.direct = 0.55;

    energize_type     = action_energize::ON_CAST;
    energize_resource = RESOURCE_ANIMA;
    energize_amount   = 1;
  }

  double composite_da_multiplier( const action_state_t* s ) const override
  {
    double m = base_t::composite_da_multiplier( s );

    if ( parent_dot )
    {
      m *= parent_dot->get_tick_factor();
    }

    return m;
  }
};

struct bursting_ice_t : public rime_spell_t
{
  bursting_ice_t( util::string_view name, rime_t* p, util::string_view options_str = {} )
    : rime_spell_t( name, p, options_str )
  {
    id = 12;

    name_str_reporting = "Bursting Ice";
    tick_action        = new bursting_ice_tick_t( "bursting_ice_tick", p );
    add_child( tick_action );

    base_execute_time      = 2_s;
    dot_duration           = 3_s;

    if ( p->legendary.skandis_decree )
    {
      dot_duration += p->legendary.skandis_decree_duration_bonus;
    }

    base_tick_time         = 0.5_s;
    hasted_ticks           = true;
    dot_allow_partial_tick = true;

    cooldown->duration = 10_s;
    cooldown->hasted   = false;
    cooldown->charges  = 1;
  }

  void execute() override
  {
    base_t::execute();

    if ( p()->talents.winters_embrace )
      p()->buffs.winters_embrace->trigger();
  }

  void last_tick( dot_t* d ) override
  {
    base_t::last_tick( d );

    p()->buffs.winters_embrace->expire();
  }
};

template <typename T_ACTION>
struct freezing_torrent_state_t : public rime_action_state_t<T_ACTION>

{
  using base_t = rime_action_state_t<T_ACTION>;

  bool soulfrost_torrent;
  freezing_torrent_state_t( action_t* action, player_t* target ) : base_t( action, target ), soulfrost_torrent( false )
  {
  }

  void initialize() override
  {
    base_t::initialize();

    soulfrost_torrent = false;
  }

  void copy_state( const action_state_t* s )
  {
    base_t::copy_state( s );
    const freezing_torrent_state_t* rs = debug_cast<const freezing_torrent_state_t*>( s );

    soulfrost_torrent = rs->soulfrost_torrent;
  }

  double composite_crit_chance() const override
  {
    auto cc = base_t::composite_crit_chance();

    if ( soulfrost_torrent )
      cc += base_t::p()->talents.soulfrost_torrent_crit_chance;

    return cc;
  }
};

struct freezing_torrent_t : public rime_spell_t
{
  freezing_torrent_t( util::string_view name, rime_t* p, util::string_view options_str = {} )
    : rime_spell_t( name, p, options_str )
  {
    id = 13;

    name_str_reporting = "Freezing Torrent";

    spell_power_mod.tick = 1.562;

    dot_duration           = 2_s;
    base_tick_time         = 0.4_s;
    hasted_ticks           = true;
    dot_allow_partial_tick = true;
    tick_on_application    = true;
    channeled              = true;

    if ( p->talents.supreme_torrent )
    {
      dot_duration += p->talents.supreme_torrent_duration;
    }

    if ( p->talents.coalescing_frost )
    {
      add_child( p->actions.coalescing_frost );
    }

    energize_type     = action_energize::PER_TICK;
    energize_resource = RESOURCE_ANIMA;
    energize_amount   = 1;

    cooldown->duration = 15_s;
    cooldown->hasted   = false;
    cooldown->charges  = 1;
  }

  static const freezing_torrent_state_t<base_t>* cast_state( const action_state_t* st )
  {
    return debug_cast<const freezing_torrent_state_t<base_t>*>( st );
  }

  static freezing_torrent_state_t<base_t>* cast_state( action_state_t* st )
  {
    return debug_cast<freezing_torrent_state_t<base_t>*>( st );
  }

  action_state_t* new_state() override
  {
    return new freezing_torrent_state_t<base_t>( this, target );
  }

  void update_state( action_state_t* state, unsigned flags, result_amount_type rt ) override
  {
    base_t::update_state( state, flags, rt );
  }

  void snapshot_state( action_state_t* state, result_amount_type rt ) override
  {
    auto rs = cast_state( state );

    rs->soulfrost_torrent = p()->buffs.soulfrost_torrent->check() > 0;

    base_t::snapshot_state( state, rt );
  }

  void execute() override
  {
    base_t::execute();

    sim->print_debug( "{}'s should be executing a torrent", *p() );

    p()->buffs.soulfrost_torrent->decrement();
  }

  void init_finished() override
  {
    base_t::init_finished();

    update_flags &= ~STATE_HASTE;
  }

  double tick_time_pct_multiplier( const action_state_t* s ) const override
  {
    auto base = base_t::tick_time_pct_multiplier( s );

    if ( cast_state( s )->soulfrost_torrent )
      base /= p()->talents.soulfrost_torrent_tickrate_increase;

    return base;
  }

  void tick( dot_t* d ) override
  {
    base_t::tick( d );

    if ( p()->talents.coalescing_frost )
    {
      if ( !p()->get_target_data( d->target )->debuffs.coalescing_frost->check() )
      {
        sim->print_debug( "{}'s Freezing Torrent tick triggers Coalescing Frost on {}. Target is sleeping: {}", *p(),
                          *d->target, d->target->is_sleeping() );
      }
      if ( result_is_hit( d->state->result ) )
      {
        if ( d->state->result == RESULT_CRIT && rng().roll( p()->talents.coalescing_frost_crit_extra_chance ) )
        {
          p()->get_target_data( d->target )
              ->debuffs.coalescing_frost->trigger( p()->talents.coalescing_frost_crit_stacks );
        }
        else
        {
          p()->get_target_data( d->target )->debuffs.coalescing_frost->trigger();
        }
      }
    }

    if ( p()->talents.chilling_finesse )
    {
      p()->cooldowns.bursting_ice->adjust( -p()->talents.chilling_finesse_bursting_ice_cdr_per_tick, false, false );
    }

    if ( p()->buffs.flight_of_the_navir->check() )
    {
      p()->actions.frost_swallow_navir->execute();
    }

    if ( p()->legendary.frostwyrms_spite )
    {
      p()->buffs.frostwyrms_spite->trigger();
    }
  }
};

struct coalescing_frost_t : public rime_spell_t
{
  coalescing_frost_t( util::string_view name, rime_t* p ) : rime_spell_t( name, p )
  {
    id = 14;

    name_str_reporting = "Coalescing Frost";

    aoe                 = -1;
    reduced_aoe_targets = 3;

    background = true;

    // Was 1.42, old beam value
    spell_power_mod.direct = 0.302816 * 1.562;
  }
};

struct frost_swallow_t : public rime_spell_t
{
  frost_swallow_t( util::string_view name, rime_t* p ) : rime_spell_t( name, p )
  {
    id = 15;

    name_str_reporting = "Birb";

    background = true;

    spell_power_mod.direct = 0.68;
  }

  void execute() override
  {
    base_t::execute();
    if ( p()->talents.cascading_blitz && p()->buffs.ice_blitz->check() )
    {
      p()->buffs.ice_blitz->extend_duration( p(), p()->talents.cascading_blitz_extension_per_bird );
    }
  }
};

// struct vexiras_venom_t : public residual_action::residual_periodic_action_t<rime_spell_t>
//{
//   vexiras_venom_t( util::string_view name, rime_t* p ) : residual_action_t( name, p )
//   {
//     id = 16;
//
//     name_str_reporting = "Vexiras Venom";
//
//     tick_may_crit = false;
//
//     dot_duration   = p->legendary.vexiras_venom_duration;
//     dot_behavior   = DOT_REFRESH_DURATION;
//     base_tick_time = p->legendary.vexiras_venom_period;
//     hasted_ticks   = true;
//   }
//
//   void init() override
//   {
//     base_t::init();
//     snapshot_flags |= STATE_HASTE;
//     update_flags &= ~STATE_HASTE;
//   }
// };

}  // namespace actions

// ==========================================================================
// Rogue Targetdata Definitions
// ==========================================================================

rime_td_t::rime_td_t( player_t* target, rime_t* source )
  : fellowship::fs_player_td_t( target, source ), dots(), debuffs()
{
  dots.bursting_ice     = target->get_dot( "bursting_ice", source );
  dots.freezing_torrent = target->get_dot( "freezing_torrent", source );

  debuffs.coalescing_frost = make_buff( *this, "coalescing_frost" )
                                 ->set_duration( source->talents.coalescing_frost_duration )
                                 ->set_refresh_behavior( buff_refresh_behavior::DURATION )
                                 ->set_max_stack( source->talents.coalescing_frost_max_stacks )
                                 ->add_stack_change_callback( [ source ]( buff_t* b, int old, int _new ) {
                                   if ( !_new && old )
                                   {
                                     auto damage = source->actions.coalescing_frost;
                                     if ( !b->player->is_sleeping() )
                                     {
                                       damage->set_target( b->player );
                                     }
                                     else
                                     {
                                       for ( auto& enemy : b->sim->target_non_sleeping_list )
                                       {
                                         if ( !enemy->is_sleeping() )
                                         {
                                           damage->set_target( enemy );
                                           break;
                                         }
                                       }
                                     }
                                     if ( !damage->target->is_sleeping() )
                                     {
                                       action_state_t* damage_state = damage->get_state();
                                       damage_state->target         = damage->target;

                                       damage->snapshot_state( damage_state, result_amount_type::DMG_DIRECT );
                                       damage_state->da_multiplier *= old;
                                       damage->schedule_execute( damage_state );
                                     }
                                     else
                                     {
                                       b->sim->print_debug( "{} tried to execute {} on {} but all targets were dead.",
                                                            *b->source, *damage, *b->player );
                                       source->procs.coal_no_targets->occur();
                                       
                                     }
                                   }
                                 } );
}

// ==========================================================================
// Rogue Character Definition
// ==========================================================================

// rime_t::composite_attribute_multiplier ==================================

double rime_t::composite_attribute_multiplier( attribute_e a ) const
{
  double am = fs_player_t::composite_attribute_multiplier( a );

  return am;
}
// rime_t::composite_spell_haste ==========================================

double rime_t::composite_spell_haste() const
{
  double h = fs_player_t::composite_spell_haste();

  return h;
}

// rime_t::composite_spell_crit_chance =========================================

double rime_t::composite_spell_crit_chance() const
{
  double crit = fs_player_t::composite_spell_crit_chance();

  return crit;
}

// rime_t::composite_damage_versatility ===================================

double rime_t::composite_damage_versatility() const
{
  double cdv = fs_player_t::composite_damage_versatility();

  return cdv;
}

// rime_t::composite_heal_versatility =====================================

double rime_t::composite_heal_versatility() const
{
  double chv = fs_player_t::composite_heal_versatility();

  return chv;
}

// rime_t::composite_leech ===============================================

double rime_t::composite_leech() const
{
  double l = fs_player_t::composite_leech();

  return l;
}

// rime_t::matching_gear_multiplier ========================================

double rime_t::matching_gear_multiplier( attribute_e attr ) const
{
  return 0.0;
}

// rime_t::composite_player_multiplier =====================================

double rime_t::composite_player_multiplier( school_e school ) const
{
  double m = fs_player_t::composite_player_multiplier( school );
  
  m *= 1.0 + buffs.ice_blitz->check_value();
  m *= 1.0 + buffs.ultimate_buff_window->check_value();

  return m;
}

// rime_t::composite_player_pet_damage_multiplier ==========================

double rime_t::composite_player_pet_damage_multiplier( const action_state_t* s, bool guardian ) const
{
  double m = fs_player_t::composite_player_pet_damage_multiplier( s, guardian );

  return m;
}

// rime_t::composite_player_target_multiplier ==============================

double rime_t::composite_player_target_multiplier( player_t* target, school_e school ) const
{
  double m = fs_player_t::composite_player_target_multiplier( target, school );

  // rime_td_t* tdata = get_target_data( target );

  return m;
}

// rime_t::composite_player_target_crit_chance =============================

double rime_t::composite_player_target_crit_chance( player_t* target ) const
{
  double c = fs_player_t::composite_player_target_crit_chance( target );

  return c;
}

// rime_t::composite_player_target_armor ===================================

double rime_t::composite_player_target_armor( player_t* target ) const
{
  return 0.0;

  double a = fs_player_t::composite_player_target_armor( target );

  return a;
}
// rime_t::init_actions ====================================================

void rime_t::init_action_list()
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

// rime_t::create_action  ==================================================

action_t* rime_t::create_action( util::string_view name, util::string_view options_str )
{
  using namespace actions;

  if ( name == "frost_bolt" )
    return new frost_bolt_t( name, this, options_str );
  if ( name == "glacial_blast" )
    return new glacial_blast_t( name, this, options_str );
  if ( name == "cold_snap" )
    return new cold_snap_t( name, this, options_str );
  if ( name == "winters_blessing" )
    return new winters_blessing_t( name, this, options_str );
  if ( name == "bursting_ice" )
    return new bursting_ice_t( name, this, options_str );
  if ( name == "freezing_torrent" )
    return new freezing_torrent_t( name, this, options_str );
  if ( name == "flight_of_the_navir" )
    return new flight_of_the_navir_t( name, this, options_str );
  if ( name == "ice_comet" )
    return new ice_comet_t( name, this, options_str );
  if ( name == "wrath_of_winter" )
    return new wrath_of_winter_t( name, this, options_str );
  if ( name == "ice_blitz" )
    return new ice_blitz_t( name, this, options_str );

  return fs_player_t::create_action( name, options_str );
}

// rime_t::create_expression ===============================================

std::unique_ptr<expr_t> rime_t::create_action_expression( action_t& action, std::string_view name_str )
{
  // auto split = util::string_split<util::string_view>( name_str, "." );

  return fs_player_t::create_action_expression( action, name_str );
}

std::unique_ptr<expr_t> rime_t::create_expression( util::string_view name_str )
{
  auto split = util::string_split<util::string_view>( name_str, "." );

  if ( split[ 0 ] == "winter_orbs" || split[ 0 ] == "worb" || split[ 0 ] == "worbs" )
  {
    if ( split.size() == 1 )
    {
      return make_fn_expr( name_str, [ this ] { return this->current_worbs( true ); } );
    }

    if ( split.size() == 2 && split[ 1 ] == "deficit" )
    {
      return make_fn_expr( name_str,
                           [ this ] { return resources.max[ RESOURCE_WINTER_ORB ] - this->current_worbs( true ); } );
    }
  } 
  else if ( util::str_compare_ci( split[ 0 ], "talent" ) )
  {
    if ( split.size() == 2 )
    {
      if ( util::str_compare_ci( split[ 1 ], "chilling_finesse" ) )
        return make_ref_expr( name_str, talents.chilling_finesse );
      if ( util::str_compare_ci( split[ 1 ], "winters_embrace" ) )
        return make_ref_expr( name_str, talents.winters_embrace );
      if ( util::str_compare_ci( split[ 1 ], "glacial_assault" ) )
        return make_ref_expr( name_str, talents.glacial_assault );
      if ( util::str_compare_ci( split[ 1 ], "burstbolter" ) )
        return make_ref_expr( name_str, talents.burstbolter );
      if ( util::str_compare_ci( split[ 1 ], "supreme_torrent" ) )
        return make_ref_expr( name_str, talents.supreme_torrent );
      if ( util::str_compare_ci( split[ 1 ], "navirs_keeper" ) )
        return make_ref_expr( name_str, talents.navirs_keeper );
      if ( util::str_compare_ci( split[ 1 ], "icy_flow" ) )
        return make_ref_expr( name_str, talents.icy_flow );
      if ( util::str_compare_ci( split[ 1 ], "avalanche" ) )
        return make_ref_expr( name_str, talents.avalanche );
      if ( util::str_compare_ci( split[ 1 ], "coalescing_frost" ) )
        return make_ref_expr( name_str, talents.coalescing_frost );
      if ( util::str_compare_ci( split[ 1 ], "greater_glacial_blast" ) )
        return make_ref_expr( name_str, talents.greater_glacial_blast );
      if ( util::str_compare_ci( split[ 1 ], "cascading_blitz" ) )
        return make_ref_expr( name_str, talents.cascading_blitz );
      if ( util::str_compare_ci( split[ 1 ], "frostweavers_wrath" ) )
        return make_ref_expr( name_str, talents.frostweavers_wrath );
      if ( util::str_compare_ci( split[ 1 ], "soulfrost_torrent" ) )
        return make_ref_expr( name_str, talents.soulfrost_torrent );
      if ( util::str_compare_ci( split[ 1 ], "biting_cold" ) )
        return make_ref_expr( name_str, talents.biting_cold );
      if ( util::str_compare_ci( split[ 1 ], "wisdom_of_the_north" ) )
        return make_ref_expr( name_str, talents.wisdom_of_the_north );
    }
  }
  else if ( util::str_compare_ci( split[ 0 ], "legendary" ) )
  {
    if ( split.size() == 2 )
    {
      if ( util::str_compare_ci( split[ 1 ], "frostwyrms_spite" ) )
        return make_ref_expr( name_str, legendary.frostwyrms_spite );
      if ( util::str_compare_ci( split[ 1 ], "skandis_decree" ) )
        return make_ref_expr( name_str, legendary.skandis_decree );
      if ( util::str_compare_ci( split[ 1 ], "undulating_spirit" ) )
        return make_ref_expr( name_str, legendary.undulating_spirit );
    }
  }

  return fs_player_t::create_expression( name_str );
}

std::unique_ptr<expr_t> rime_t::create_resource_expression( util::string_view name_str )
{
  return fs_player_t::create_resource_expression( name_str );
}

// rime_t::init_base =======================================================

void rime_t::init_base_stats()
{
  if ( base.distance < 1 )
    base.distance = 5;

  fs_player_t::init_base_stats();

  base.stats.attribute[ STAT_INTELLECT ] = 1000;
  base.stats.attribute[ STAT_STAMINA ]   = 1000;
  resources.base[ RESOURCE_HEALTH ]      = 16180;

  base.health_per_stamina = 38.005;

  resources.base[ RESOURCE_ANIMA ] = 18;

  resources.base[ RESOURCE_WINTER_ORB ] = 5;

  base_gcd = timespan_t::from_seconds( 1.5 );
  min_gcd  = timespan_t::from_seconds( 0.75 );
}

// rime_t::init_spells =====================================================

void rime_t::init_spells()
{
  fs_player_t::init_spells();

  // actions.auto_attack = new actions::auto_melee_attack_t( this, "" );
}

// rime_t::init_talents ====================================================

void rime_t::init_talents()
{
  fs_player_t::init_talents();
}

// rime_t::init_gains ======================================================

void rime_t::init_gains()
{
  fs_player_t::init_gains();

  gains.anima_worbs  = get_gain( "Anima Worbs" );
  gains.ult_worbs    = get_gain( "Ultimate Worbs" );
  gains.spirit_procs = get_gain( "Spirit Procs" );
  /*gains.venomous_delight = get_gain( "Venomous Delight" );
  gains.efficient_killer = get_gain( "Efficient Killer" );*/

  // gains.ace_up_your_sleeve              = get_gain( "Ace Up Your Sleeve" );
  // gains.adrenaline_rush                 = get_gain( "Adrenaline Rush" );
  // gains.adrenaline_rush_expiry          = get_gain( "Adrenaline Rush (Expiry)" );
  // gains.blade_rush                      = get_gain( "Blade Rush" );
  // gains.broadside                       = get_gain( "Broadside" );
  // gains.buried_treasure                 = get_gain( "Buried Treasure" );
  // gains.darkest_night                   = get_gain( "Darkest Night" );
  // gains.dashing_scoundrel               = get_gain( "Dashing Scoundrel" );
  // gains.deal_fate                       = get_gain( "Deal Fate" );
  // gains.energy_refund                   = get_gain( "Energy Refund" );
  // gains.fatal_flourish                  = get_gain( "Fatal Flourish" );
  // gains.improved_adrenaline_rush        = get_gain( "Improved Adrenaline Rush" );
  // gains.improved_adrenaline_rush_expiry = get_gain( "Improved Adrenaline Rush (Expiry)" );
  // gains.improved_ambush                 = get_gain( "Improved Ambush" );
  // gains.killing_spree                   = get_gain( "Killing Spree" );
  // gains.master_of_shadows               = get_gain( "Master of Shadows" );
  // gains.premeditation                   = get_gain( "Premeditation" );
  // gains.quick_draw                      = get_gain( "Quick Draw" );
  // gains.relentless_strikes              = get_gain( "Relentless Strikes" );
  // gains.ruthlessness                    = get_gain( "Ruthlessness" );
  // gains.seal_fate                       = get_gain( "Seal Fate" );
  // gains.serrated_bone_spike             = get_gain( "Serrated Bone Spike" );
  // gains.shadow_blades                   = get_gain( "Shadow Blades" );
  // gains.shadow_techniques               = get_gain( "Shadow Techniques" );
  // gains.shadow_techniques_shadowcraft   = get_gain( "Shadow Techniques (Shadowcraft)" );
  // gains.shrouded_suffocation            = get_gain( "Shrouded Suffocation" );
  // gains.slice_and_dice                  = get_gain( "Slice and Dice" );
  // gains.symbols_of_death                = get_gain( "Symbols of Death" );
  // gains.venomous_wounds                 = get_gain( "Venomous Vim" );
  // gains.venomous_wounds_death           = get_gain( "Venomous Vim (Death)" );
}

// rime_t::init_procs ======================================================

void rime_t::init_procs()
{
  fs_player_t::init_procs();

  procs.coal_no_targets = get_proc( "Coalescing Frost No Targets" );
}

// rime_t::init_rng ========================================================
void rime_t::init_rng()
{
  fs_player_t::init_rng();

  if ( talents.soulfrost_torrent )
    rppm.soulfrost_torrent = get_rppm( "soulfrost_torrent", talents.soulfrost_torrent_rppm, 1.0, RPPM_HASTE );
}

// rime_t::init_scaling ====================================================

void rime_t::init_scaling()
{
  fs_player_t::init_scaling();

  scaling->disable( STAT_STRENGTH );
  scaling->disable( STAT_INTELLECT );

  // Break out early if scaling is disabled on this player, or there's no
  // scaling stat
  if ( !scale_player || sim->scaling->scale_stat == STAT_NONE )
  {
    return;
  }
}

// rime_t::init_resources =================================================

void rime_t::init_resources( bool force )
{
  fs_player_t::init_resources( force );

  resources.current[ RESOURCE_WINTER_ORB ] = 0;
}

// rime_t::init_buffs ======================================================

void rime_t::create_buffs()
{
  fs_player_t::create_buffs();

  buffs.icy_flow = make_buff<rime_buff_t>( this, "icy_flow" )
                       ->set_duration( talents.icy_flow_buff_duration )
                       ->set_default_value( talents.icy_flow_cc )
                       ->set_max_stack( talents.icy_flow_max_stacks );

  buffs.flight_of_the_navir = make_buff<rime_buff_t>( this, "flight_of_the_navir" )->set_duration( 20_s );

  buffs.frostweavers_wrath = make_buff<rime_buff_t>( this, "frostweaver_wrath" )
                                 ->set_duration( talents.frostweavers_wrath_buff_duration )
                                 ->set_default_value( talents.frostweavers_wrath_added_cc );

  buffs.glacial_assault = make_buff<rime_buff_t>( this, "glacial_assault" )
                              ->set_default_value( talents.glacial_assault_amp )
                              ->set_max_stack( talents.glacial_assault_stacks );

  buffs.ice_blitz = make_buff<rime_buff_t>( this, "ice_blitz" )
                        ->set_duration( 20_s )
                        ->set_default_value( 0.2 )
                        ->add_invalidate( CACHE_PLAYER_DAMAGE_MULTIPLIER );

  buffs.soulfrost_torrent = make_buff<rime_buff_t>( this, "soulfrost_torrent" )
                                ->set_duration( talents.soulfrost_torrent_buff_duration )
                                ->set_default_value( talents.soulfrost_torrent_crit_chance );

  buffs.ultimate_buff_window =
      make_buff<rime_buff_t>( this, "wrath_of_winter" )
          ->set_default_value( 0.2 )
          ->set_duration( 20_s )
          ->set_period( 4_s )
          ->set_tick_on_application( true )
          ->set_tick_callback( [ this ]( buff_t*, int, timespan_t ) {
            resource_gain( RESOURCE_WINTER_ORB, 1.0, gains.ult_worbs, actions.wrath_of_winter );
          } )
          ->add_invalidate( CACHE_PLAYER_DAMAGE_MULTIPLIER );

  buffs.winters_blessing = make_buff<rime_buff_t>( this, "winters_blessing" )
                               ->set_duration( 20_s )
                               ->set_default_value( 0.20 )
                               ->set_pct_buff_type( STAT_PCT_BUFF_MASTERY );

  buffs.winters_embrace = make_buff<rime_buff_t>( this, "winters_embrace" )->set_default_value( 0.2 );

  buffs.frostwyrms_spite = make_buff<rime_buff_t>( this, "frostwyrms_spite" )
                               ->set_duration( legendary.frostwyrms_spite_duration )
                               ->set_max_stack( legendary.frostwyrms_spite_max_stacks )
                               ->set_default_value( legendary.frostwyrms_spite_dmg_per_stack )
                               ->set_refresh_behavior( buff_refresh_behavior::DURATION );

  buffs.undulating_spirit = make_buff<rime_buff_t>( this, "undulating_spirit" )->set_max_stack( 1 );
}

// rime_t::invalidate_cache =========================================

void rime_t::invalidate_cache( cache_e c )
{
  fs_player_t::invalidate_cache( c );
}

void rime_t::create_options()
{
  fs_player_t::create_options();

  add_option( opt_bool( "talent.chilling_finesse", talents.chilling_finesse ) );
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
  add_option( opt_bool( "talent.wisdom_of_the_north", talents.wisdom_of_the_north ) );

  add_option( opt_bool( "legendary.frostwyrms_spite", legendary.frostwyrms_spite ) );
  add_option( opt_bool( "legendary.skandis_decree", legendary.skandis_decree ) );
  add_option( opt_bool( "legendary.undulating_spirit", legendary.undulating_spirit ) );
}

// rime_t::copy_from =======================================================

void rime_t::copy_from( player_t* source )
{
  rime_t* rime = static_cast<rime_t*>( source );
  fs_player_t::copy_from( source );

  talents   = rime->talents;
  legendary = rime->legendary;
  options   = rime->options;
}

// rime_t::create_profile  =================================================

std::string rime_t::create_profile( save_e stype )
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

// rime_t::init_items ======================================================

void rime_t::init_items()
{
  fs_player_t::init_items();
}

// rime_t::init_special_effects ============================================

void rime_t::init_special_effects()
{
  fs_player_t::init_special_effects();
}

// rime_t::init_finished ===================================================

void rime_t::init_finished()
{
  fs_player_t::init_finished();
}

void rime_t::init_background_actions()
{
  fs_player_t::init_background_actions();

  actions.bursting_ice_tick_burstbolter = new actions::bursting_ice_tick_t( "bursting_ice_burstbolter", this );
  actions.coalescing_frost              = new actions::coalescing_frost_t( "coalescing_frost", this );
  actions.frost_swallow                 = new actions::frost_swallow_t( "frost_swallow", this );
  actions.frost_swallow_cascading       = new actions::frost_swallow_t( "frost_swallow_cascading", this );
  actions.frost_swallow_navir           = new actions::frost_swallow_t( "frost_swallow_navir", this );
  actions.ice_comet_avalanche =
      new actions::ice_comet_t( "ice_comet_avalanche", this, {}, secondary_trigger::AVALANCHE );
  actions.ice_comet_avalanche->background = true;
}

// rime_t::reset ===========================================================

void rime_t::reset()
{
  fs_player_t::reset();
}

// rime_t::activate ========================================================

void rime_t::activate()
{
  fs_player_t::activate();
}

// rime_t::arise ===========================================================

void rime_t::arise()
{
  fs_player_t::arise();

  resources.current[ RESOURCE_ANIMA ]      = 0;
  resources.current[ RESOURCE_WINTER_ORB ] = 0;
}

// rime_t::combat_begin ====================================================

void rime_t::combat_begin()
{
  fs_player_t::combat_begin();
}

double rime_t::resource_gain( resource_e resource_type, double amount, gain_t* source, action_t* action )
{
  if ( resource_type == RESOURCE_ANIMA && talents.cascading_blitz && buffs.ice_blitz->check() )
  {
    for ( int i = 0; i < amount; i++ )
    {
      actions.frost_swallow_cascading->execute();
    }
  }

  double actual_amount = fs_player_t::resource_gain( resource_type, amount, source, action );

  if ( resource_type == RESOURCE_ANIMA )
  {
    if ( resources.current[ RESOURCE_ANIMA ] >= 9 )
    {
      resources.current[ RESOURCE_ANIMA ] -= 9;
      resource_gain( RESOURCE_WINTER_ORB, 1.0, gains.anima_worbs, action );
    }
  }

  if ( resource_type == RESOURCE_WINTER_ORB )
  {
    if ( talents.frostweavers_wrath && rng().roll( talents.frostweavers_wrath_chance_per_orb ) )
    {
      buffs.frostweavers_wrath->trigger();
    }

    for ( int i = 0; i < amount; i++ )
    {
      actions.frost_swallow->execute();
    }
  }

  return actual_amount;
}

// rime_t::non_stacking_movement_modifier ==================================

double rime_t::non_stacking_movement_modifier() const
{
  double ms = fs_player_t::non_stacking_movement_modifier();

  return ms;
}

// rime_t::stacking_movement_modifier===================================

double rime_t::stacking_movement_modifier() const
{
  double ms = fs_player_t::stacking_movement_modifier();

  return ms;
}

template <typename Base>
void actions::rime_action_t<Base>::trigger_spirit_refund( const action_state_t* state, double orbs_refunded )
{
  make_event( ab::sim, 200_ms, [ orbs_refunded, this ] {
    p()->resource_gain( RESOURCE_WINTER_ORB, orbs_refunded, p()->gains.spirit_procs, this );
    p()->sim->print_debug( "{} actually refunded {:.0f} Winter Orbs", *p(), orbs_refunded );
  } );
  
  p()->spirit_refund();
}

template <typename Base>
void actions::rime_action_t<Base>::spend_winter_orbs( const action_state_t* s )
{
  double orbs_spent = s->action->base_costs[ RESOURCE_WINTER_ORB ];
  if ( orbs_spent <= 0 )
    return;

  if ( p()->legendary.undulating_spirit && p()->buffs.undulating_spirit->check() )
  {
    p()->buffs.undulating_spirit->expire();
    p()->sim->print_debug( "{} proc'd Undulating Spirit Refund", *p() );
    trigger_spirit_refund( s, orbs_spent );
  }
  else if ( p()->rng().roll( p()->cache.mastery_value() ) )
  {
    p()->sim->print_debug( "{} proc'd Spirit Orb Refund (Chance: {:.2f}%, Sprit: {:.2f}%)", *p(),
                           p()->cache.mastery_value() * 100.0, p()->cache.mastery() * 100.0 );

    trigger_spirit_refund( s, orbs_spent );
  }

  if ( p()->talents.wisdom_of_the_north )
  {
    timespan_t total_reduction = -orbs_spent * p()->talents.wisdom_of_the_north_cdr;

    p()->cooldowns.ice_blitz->adjust( total_reduction );
    p()->cooldowns.flight_of_the_navir->adjust( total_reduction );
    p()->cooldowns.winters_blessing->adjust( total_reduction );
  }
}

template <typename Base>
void actions::rime_action_t<Base>::gain_winter_orb( int gain )
{
}

template <typename Base>
void actions::rime_action_t<Base>::gain_anima( int gain )
{
}

// rime_t::convert_hybrid_stat ==============================================

stat_e rime_t::convert_hybrid_stat( stat_e s ) const
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

void rime_t::create_cooldowns()
{
  cooldowns.bursting_ice        = get_cooldown( "bursting_ice" );
  cooldowns.cold_snap           = get_cooldown( "cold_snap" );
  cooldowns.flight_of_the_navir = get_cooldown( "flight_of_the_navir" );
  cooldowns.freezing_torrent    = get_cooldown( "freezing_torrent" );
  cooldowns.ice_blitz           = get_cooldown( "ice_blitz" );
  cooldowns.winters_blessing    = get_cooldown( "winters_blessing" );
}

class rime_module_t : public module_t
{
public:
  rime_module_t() : module_t( RIME )
  {
  }

  player_t* create_player( sim_t* sim, util::string_view name, race_e r = RACE_NONE ) const override
  {
    return new rime_t( sim, name, r );
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

}  // namespace rime
}  // namespace fellowship

const module_t* module_t::rime()
{
  static fellowship::rime::rime_module_t m;
  return &m;
}