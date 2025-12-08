#include "fs_player.hpp"
#include "util/util.hpp"

#include "simulationcraft.hpp"

namespace fellowship
{
namespace ardeos
{

// Forward Declarations
class ardeos_t;

enum class secondary_trigger
{
  NONE = 0U,
};

namespace actions
{
struct ardeos_heal_t;
struct ardeos_spell_t;

struct melee_t;
}  // namespace actions

class ardeos_td_t : public fs_player_td_t
{
public:
  struct dots_t
  {
    dot_t* incinerate;
    dot_t* searing_blaze;
    dot_t* engulfing_flames;
    dot_t* fire_ball;
    dot_t* fire_frogs;
    dot_t* crackling_inferno;
  } dots;

  struct
  {
    buff_t* incinerate_stacks;
    buff_t* agonizing_blaze_stacks;
  } debuffs;

  ardeos_td_t( player_t* target, ardeos_t* source );

  int dot_count() const
  {
    return dots.incinerate->is_ticking() + dots.searing_blaze->is_ticking() + dots.engulfing_flames->is_ticking() +
           dots.fire_ball->is_ticking() + dots.fire_frogs->is_ticking() + dots.crackling_inferno->is_ticking();
  }
};

struct ardeos_buff_t : public fs_player_buff_t
{
  ardeos_buff_t( player_t* p, util::string_view name ) : fs_player_buff_t( p, name )
  {
  }

  ardeos_t* p()
  {
    return debug_cast<ardeos_t*>( player );
  }

  const ardeos_t* p() const
  {
    return debug_cast<const ardeos_t*>( player );
  }
};

class ardeos_t : public fellowship::fs_player_t
{
public:
  struct actions_t
  {
    actions::ardeos_spell_t* incinerate;
    actions::ardeos_spell_t* fire_ball;
    actions::ardeos_spell_t* fire_frogs;
    actions::ardeos_spell_t* fire_frogs_hit;
    actions::ardeos_spell_t* infernal_wave;
    actions::ardeos_spell_t* searing_blaze;
    actions::ardeos_spell_t* engulfing_flames;
    actions::ardeos_spell_t* detonate;
    actions::ardeos_spell_t* wildfire;
    actions::ardeos_spell_t* pyromania;
    actions::ardeos_spell_t* crackling_inferno;
    actions::ardeos_spell_t* apocalypse;
  } actions;

  struct buffs_t
  {
    buff_t* reign_of_fire;
    buff_t* wildfire;
  } buffs;

  struct cooldowns_t
  {
    cooldown_t* apocalypse;
    cooldown_t* engulfing_flames;
    cooldown_t* pyromania;
    cooldown_t* fireball;
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
    real_ppm_t* reign_of_fire;
  } rppm;

  enum ardeos_talents_t
  {
    NONE                   = 0,
    SLOW_BURN              = 1 << 0,
    GREAT_BALLS_OF_FIRE    = 1 << 1,
    FLICKERING_CINDERS     = 1 << 2,
    FLARE_UP               = 1 << 3,
    UNDYING_FLAME          = 1 << 4,
    AGONIZING_BLAZE        = 1 << 5,
    FIRESTARTER            = 1 << 6,
    OUROBOROS              = 1 << 7,
    FIERY_RESILIENCE       = 1 << 8,
    CRACKLING_INFERNO      = 1 << 9,
    MAGIC_WARD             = 1 << 10,
    ROLLING_FLAMES         = 1 << 11,
    PYROPHIBIAN_FRENZY     = 1 << 12,
    REIGN_OF_FIRE          = 1 << 13,
    INTENSIFYING_INFERNO   = 1 << 14,
    SPIRITED_FORTITUDE     = 1 << 15,
    SPONTANEOUS_COMBUSTION = 1 << 16,
    MAX                    = 1 << 17
  };

  static constexpr std::string_view talent_name_formatted( ardeos_talents_t t )
  {
    switch ( t )
    {
      case ardeos_talents_t::SLOW_BURN:
        return "Slow Burn";
      case ardeos_talents_t::GREAT_BALLS_OF_FIRE:
        return "Great Balls of Fire";
      case ardeos_talents_t::FLICKERING_CINDERS:
        return "Flickering Cinders";
      case ardeos_talents_t::FLARE_UP:
        return "Flare Up";
      case ardeos_talents_t::UNDYING_FLAME:
        return "Undying Flame";
      case ardeos_talents_t::AGONIZING_BLAZE:
        return "Agonizing Blaze";
      case ardeos_talents_t::FIRESTARTER:
        return "Firestarter";
      case ardeos_talents_t::OUROBOROS:
        return "Ouroboros";
      case ardeos_talents_t::FIERY_RESILIENCE:
        return "Fiery Resilience";
      case ardeos_talents_t::CRACKLING_INFERNO:
        return "Crackling Inferno";
      case ardeos_talents_t::MAGIC_WARD:
        return "Magic Ward";
      case ardeos_talents_t::ROLLING_FLAMES:
        return "Rolling Flames";
      case ardeos_talents_t::PYROPHIBIAN_FRENZY:
        return "Pyrophibian Frenzy";
      case ardeos_talents_t::REIGN_OF_FIRE:
        return "Reign of Fire";
      case ardeos_talents_t::INTENSIFYING_INFERNO:
        return "Intensifying Inferno";
      case ardeos_talents_t::SPIRITED_FORTITUDE:
        return "Spirited Fortitude";
      case ardeos_talents_t::SPONTANEOUS_COMBUSTION:
        return "Spontaneous Combustion";
      default:
        return "Unknown Talent";
    }
  }
  static constexpr std::string_view talent_name( ardeos_talents_t t )
  {
    switch ( t )
    {
      case ardeos_talents_t::SLOW_BURN:
        return "slow_burn";
      case ardeos_talents_t::GREAT_BALLS_OF_FIRE:
        return "great_balls_of_fire";
      case ardeos_talents_t::FLICKERING_CINDERS:
        return "flickering_cinders";
      case ardeos_talents_t::FLARE_UP:
        return "flare_up";
      case ardeos_talents_t::UNDYING_FLAME:
        return "undying_flame";
      case ardeos_talents_t::AGONIZING_BLAZE:
        return "agonizing_blaze";
      case ardeos_talents_t::FIRESTARTER:
        return "firestarter";
      case ardeos_talents_t::OUROBOROS:
        return "ouroboros";
      case ardeos_talents_t::FIERY_RESILIENCE:
        return "fiery_resilience";
      case ardeos_talents_t::CRACKLING_INFERNO:
        return "crackling_inferno";
      case ardeos_talents_t::MAGIC_WARD:
        return "magic_ward";
      case ardeos_talents_t::ROLLING_FLAMES:
        return "rolling_flames";
      case ardeos_talents_t::PYROPHIBIAN_FRENZY:
        return "pyrophibian_frenzy";
      case ardeos_talents_t::REIGN_OF_FIRE:
        return "reign_of_fire";
      case ardeos_talents_t::INTENSIFYING_INFERNO:
        return "intensifying_inferno";
      case ardeos_talents_t::SPIRITED_FORTITUDE:
        return "spirited_fortitude";
      case ardeos_talents_t::SPONTANEOUS_COMBUSTION:
        return "spontaneous_combustion";
      default:
        return "unknown_talent";
    }
  }

  struct talents_t
  {
    timespan_t slow_burn_extend = 0.5_s;

    unsigned frog_squad_extra_hits  = 1;
    unsigned frog_squad_extra_frogs = 1;

    double great_balls_of_fire_amp = 0.6;

    double flickering_cinders_cinder_multiplier = 0.25;

    double flare_up_multiplier = 0.25;

    timespan_t undying_flame_extension = 3_s;

    double agonizing_blaze_amp_per_stack    = 0.03;
    unsigned agonizing_blaze_maximum_stacks = 10;

    double firestarter_crit_chance = 0.2;

    timespan_t ouroboros_cdr      = 0.15_s;
    timespan_t ouroboros_cdr_crit = 0.3_s;

    double crackling_inferno_crit_chance      = 0.2;
    double crackling_inferno_dot_fraction     = 0.6;
    timespan_t crackling_inferno_dot_duration = 24_s;
    timespan_t crackling_inferno_dot_period   = 3_s;

    timespan_t rolling_flames_cdr = 0.5_s;

    double pyrophibian_frenzy_chance = 0.08;

    double reign_of_fire_ppm         = 1.0;
    double reign_of_fire_crit_chance = 1.0;

    double intensifying_inferno_amp = 0.15;

    double spontaneous_combustion_chance       = 0.04;
    double spontaneous_combustion_extra_chance = 0.05;
  } talents;

  struct legendary_t
  {
    bool untamed_flame               = false;
    double untamed_flame_spirit      = 0.3;
    double untamed_flame_crit_chance = 0.3;

    bool explosive_potency                = false;
    double explosive_potency_detonate_amp = 0.5;

    bool brimstone_cataclysm                   = false;
    timespan_t brimstone_cataclysm_cdr_per_hit = 2_s;
    timespan_t brimstone_cataclysm_cdr_cap     = 30_s;
  } legendary;

  struct options_t
  {
  } options;

  target_specific_t<ardeos_td_t> target_data;

  const ardeos_td_t* find_target_data( const player_t* target ) const override
  {
    return target_data[ target ];
  }

  ardeos_td_t* get_target_data( player_t* target ) const override
  {
    ardeos_td_t*& td = target_data[ target ];
    if ( !td )
    {
      td = new ardeos_td_t( target, const_cast<ardeos_t*>( this ) );
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

  ardeos_t( sim_t* sim, util::string_view name, race_e r = RACE_NONE ) : fs_player_t( sim, name, r, ARDEOS ), target_data()
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
struct ardeos_action_state_t : public action_state_t
{
private:
  T_ACTION* action;

public:
  ardeos_action_state_t( action_t* action, player_t* target )
    : action_state_t( action, target ), action( dynamic_cast<T_ACTION*>( action ) )
  {
  }
  
  ardeos_t* p() const
  {
    return debug_cast<ardeos_t*>( action->player );
  }

  ardeos_t* p()
  {
    return debug_cast<ardeos_t*>( action->player );
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
    const ardeos_action_state_t* rs = debug_cast<const ardeos_action_state_t*>( s );
  }

  T_ACTION* get_action() const
  {
    return action;
  }
};

template <typename Base>
class ardeos_action_t : public Base
{
protected:
  /// typedef for ardeos_action_t<action_base_t>
  using base_t = ardeos_action_t<Base>;

private:
  /// typedef for the templated action type, eg. spell_t, attack_t, heal_t
  using ab = Base;

public:
  secondary_trigger secondary_trigger_type;

  // Init =====================================================================

  ardeos_action_t( util::string_view n, ardeos_t* p, util::string_view options = {} )
    : ab( n, p, options ), secondary_trigger_type( secondary_trigger::NONE )
  {
    ab::parse_options( options );
    ab::may_crit = ab::tick_may_crit = true;
    ab::school                       = SCHOOL_FROST;

    // ardeos_t sets base and min GCD to 1.5_s hasted
    ab::gcd_type = gcd_haste_type::SPELL_HASTE;
  }

  void init() override
  {
    ab::init();
  }

  // Type Wrappers ============================================================

  static const ardeos_action_state_t<base_t>* cast_state( const action_state_t* st )
  {
    return debug_cast<const ardeos_action_state_t<base_t>*>( st );
  }

  static ardeos_action_state_t<base_t>* cast_state( action_state_t* st )
  {
    return debug_cast<ardeos_action_state_t<base_t>*>( st );
  }

  ardeos_t* p()
  {
    return debug_cast<ardeos_t*>( ab::player );
  }

  const ardeos_t* p() const
  {
    return debug_cast<const ardeos_t*>( ab::player );
  }

  ardeos_td_t* td( player_t* t ) const
  {
    return p()->get_target_data( t );
  }

  // Action State =============================================================

  action_state_t* new_state() override
  {
    return new ardeos_action_state_t<base_t>( this, ab::target );
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

struct ardeos_heal_t : public ardeos_action_t<fellowship::actions::fs_player_action_t<heal_t>>
{
  ardeos_heal_t( util::string_view n, ardeos_t* p, util::string_view o = {} ) : base_t( n, p, o )
  {
    harmful = false;
    set_target( p );
  }
};

struct ardeos_spell_t : public ardeos_action_t<fellowship::actions::fs_player_action_t<spell_t>>
{
  ardeos_spell_t( util::string_view n, ardeos_t* p, util::string_view o = {} ) : base_t( n, p, o )
  {
  }
};

struct frost_bolt_t : public ardeos_spell_t
{
  frost_bolt_t( util::string_view name, ardeos_t* p, util::string_view options_str = {} )
    : ardeos_spell_t( name, p, options_str )
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
    ardeos_spell_t::execute();
  }

  void impact( action_state_t* s ) override
  {
    ardeos_spell_t::impact( s );

    if ( p()->talents.burstbolter )
    {
      p()->actions.bursting_ice_tick_burstbolter->execute_on_target( s->target );
    }
  }
};

struct glacial_blast_t : public ardeos_spell_t
{
  glacial_blast_t( util::string_view name, ardeos_t* p, util::string_view options_str = {} )
    : ardeos_spell_t( name, p, options_str )
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
    double m = ardeos_spell_t::composite_da_multiplier( state );

    if ( p()->buffs.glacial_assault->at_max_stacks() )
      m *= 1 + p()->buffs.glacial_assault->check_value();

    return m;
  }

  double composite_crit_chance() const override
  {
    return ardeos_spell_t::composite_crit_chance() + p()->buffs.icy_flow->check_value() +
           p()->buffs.frostweavers_wrath->check_value();
  }

  void execute() override
  {
    ardeos_spell_t::execute();

    if ( p()->buffs.glacial_assault->at_max_stacks() )
    {
      p()->buffs.glacial_assault->expire();
    }

    p()->buffs.frostweavers_wrath->decrement();
    p()->buffs.icy_flow->decrement();
  }
};

struct ice_comet_t : public ardeos_spell_t
{
  ice_comet_t( util::string_view name, ardeos_t* p, util::string_view options_str = {},
               secondary_trigger st = secondary_trigger::NONE )
    : ardeos_spell_t( name, p, options_str )
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
    double m = ardeos_spell_t::composite_da_multiplier( state );

    return m;
  }

  double composite_crit_chance() const override
  {
    return ardeos_spell_t::composite_crit_chance() + p()->buffs.icy_flow->check_value() +
           p()->buffs.frostweavers_wrath->check_value();
  }

  void execute() override
  {
    ardeos_spell_t::execute();

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
    ardeos_spell_t::impact( s );
  }
};

struct ice_blitz_t : public ardeos_spell_t
{
  ice_blitz_t( util::string_view name, ardeos_t* p, util::string_view options_str = {} )
    : ardeos_spell_t( name, p, options_str )
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
    ardeos_spell_t::execute();
  }
};

struct winters_blessing_t : public ardeos_spell_t
{
  winters_blessing_t( util::string_view name, ardeos_t* p, util::string_view options_str = {} )
    : ardeos_spell_t( name, p, options_str )
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
    ardeos_spell_t::execute();
  }
};

struct flight_of_the_navir_t : public ardeos_spell_t
{
  flight_of_the_navir_t( util::string_view name, ardeos_t* p, util::string_view options_str = {} )
    : ardeos_spell_t( name, p, options_str )
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
    ardeos_spell_t::execute();

    if ( p()->talents.navirs_keeper )
    {
      p()->cooldowns.cold_snap->reset( false, p()->talents.navirs_keeper_cold_snaps );
    }
  }
};

struct wrath_of_winter_t : public ardeos_spell_t
{
  wrath_of_winter_t( util::string_view name, ardeos_t* p, util::string_view options_str = {} )
    : ardeos_spell_t( name, p, options_str )
  {
    id = 9;

    name_str_reporting = "Wrath of Winter";

    base_execute_time = 1.5_s;

    resource_current              = RESOURCE_SPIRIT;
    base_costs[ RESOURCE_SPIRIT ] = 100;
  }

  void execute() override
  {
    ardeos_spell_t::execute();
    p()->fs_buffs.spirit_of_heroism->trigger();
    p()->buffs.ultimate_buff_window->trigger();
    p()->used_ultimate();
  }
};

struct cold_snap_t : public ardeos_spell_t
{
  cold_snap_t( util::string_view name, ardeos_t* p, util::string_view options_str = {} )
    : ardeos_spell_t( name, p, options_str )
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
    ardeos_spell_t::execute();

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
    int n = ardeos_spell_t::n_targets();

    if ( p()->legendary.frostwyrms_spite )
    {
      n += p()->buffs.frostwyrms_spite->check();
    }

    return n;
  }

  double composite_da_multiplier( const action_state_t* state ) const override
  {
    double m = ardeos_spell_t::composite_da_multiplier( state );

    if ( p()->legendary.frostwyrms_spite )
    {
      m *= 1.0 + p()->buffs.frostwyrms_spite->check_stack_value();
    }

    return m;
  }
};

struct bursting_ice_tick_t : public ardeos_spell_t
{
  bursting_ice_tick_t( util::string_view name, ardeos_t* p ) : ardeos_spell_t( name, p )
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

struct bursting_ice_t : public ardeos_spell_t
{
  bursting_ice_t( util::string_view name, ardeos_t* p, util::string_view options_str = {} )
    : ardeos_spell_t( name, p, options_str )
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
struct freezing_torrent_state_t : public ardeos_action_state_t<T_ACTION>

{
  using base_t = ardeos_action_state_t<T_ACTION>;

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

struct freezing_torrent_t : public ardeos_spell_t
{
  freezing_torrent_t( util::string_view name, ardeos_t* p, util::string_view options_str = {} )
    : ardeos_spell_t( name, p, options_str )
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

struct coalescing_frost_t : public ardeos_spell_t
{
  coalescing_frost_t( util::string_view name, ardeos_t* p ) : ardeos_spell_t( name, p )
  {
    id = 14;

    name_str_reporting = "Coalescing Frost";

    aoe                 = -1;
    reduced_aoe_targets = 3;

    background = true;

    // Was 1.42, old beam value
    spell_power_mod.direct = 0.333 * 1.562;
  }
};

struct frost_swallow_t : public ardeos_spell_t
{
  frost_swallow_t( util::string_view name, ardeos_t* p ) : ardeos_spell_t( name, p )
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

// struct vexiras_venom_t : public residual_action::residual_periodic_action_t<ardeos_spell_t>
//{
//   vexiras_venom_t( util::string_view name, ardeos_t* p ) : residual_action_t( name, p )
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

ardeos_td_t::ardeos_td_t( player_t* target, ardeos_t* source )
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

// ardeos_t::composite_attribute_multiplier ==================================

double ardeos_t::composite_attribute_multiplier( attribute_e a ) const
{
  double am = fs_player_t::composite_attribute_multiplier( a );

  return am;
}
// ardeos_t::composite_spell_haste ==========================================

double ardeos_t::composite_spell_haste() const
{
  double h = fs_player_t::composite_spell_haste();

  return h;
}

// ardeos_t::composite_spell_crit_chance =========================================

double ardeos_t::composite_spell_crit_chance() const
{
  double crit = fs_player_t::composite_spell_crit_chance();

  return crit;
}

// ardeos_t::composite_damage_versatility ===================================

double ardeos_t::composite_damage_versatility() const
{
  double cdv = fs_player_t::composite_damage_versatility();

  return cdv;
}

// ardeos_t::composite_heal_versatility =====================================

double ardeos_t::composite_heal_versatility() const
{
  double chv = fs_player_t::composite_heal_versatility();

  return chv;
}

// ardeos_t::composite_leech ===============================================

double ardeos_t::composite_leech() const
{
  double l = fs_player_t::composite_leech();

  return l;
}

// ardeos_t::matching_gear_multiplier ========================================

double ardeos_t::matching_gear_multiplier( attribute_e attr ) const
{
  return 0.0;
}

// ardeos_t::composite_player_multiplier =====================================

double ardeos_t::composite_player_multiplier( school_e school ) const
{
  double m = fs_player_t::composite_player_multiplier( school );
  
  return m;
}

// ardeos_t::composite_player_pet_damage_multiplier ==========================

double ardeos_t::composite_player_pet_damage_multiplier( const action_state_t* s, bool guardian ) const
{
  double m = fs_player_t::composite_player_pet_damage_multiplier( s, guardian );

  return m;
}

// ardeos_t::composite_player_target_multiplier ==============================

double ardeos_t::composite_player_target_multiplier( player_t* target, school_e school ) const
{
  double m = fs_player_t::composite_player_target_multiplier( target, school );

  // ardeos_td_t* tdata = get_target_data( target );

  return m;
}

// ardeos_t::composite_player_target_crit_chance =============================

double ardeos_t::composite_player_target_crit_chance( player_t* target ) const
{
  double c = fs_player_t::composite_player_target_crit_chance( target );

  return c;
}

// ardeos_t::composite_player_target_armor ===================================

double ardeos_t::composite_player_target_armor( player_t* target ) const
{
  return 0.0;

  double a = fs_player_t::composite_player_target_armor( target );

  return a;
}
// ardeos_t::init_actions ====================================================

void ardeos_t::init_action_list()
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

// ardeos_t::create_action  ==================================================

action_t* ardeos_t::create_action( util::string_view name, util::string_view options_str )
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

// ardeos_t::create_expression ===============================================

std::unique_ptr<expr_t> ardeos_t::create_action_expression( action_t& action, std::string_view name_str )
{
  // auto split = util::string_split<util::string_view>( name_str, "." );

  return fs_player_t::create_action_expression( action, name_str );
}

std::unique_ptr<expr_t> ardeos_t::create_expression( util::string_view name_str )
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
      for ( ardeos_talents_t t = static_cast<ardeos_talents_t>( 1U ); t < ardeos_talents_t::MAX; t << 1 )
      {
        if ( util::str_compare_ci( split[ 1 ], talent_name( t ) ) )
        {
          return make_fn_expr( name_str, std::bind( std::mem_fn( talent_enabled ), *this, t ) );
        }
      }
    }
  }
  else if ( util::str_compare_ci( split[ 0 ], "legendary" ) )
  {
    if ( split.size() == 2 )
    {
      if ( util::str_compare_ci( split[ 1 ], "brimstone_cataclysm" ) )
        return make_ref_expr( name_str, legendary.brimstone_cataclysm );
      if ( util::str_compare_ci( split[ 1 ], "explosive_potency" ) )
        return make_ref_expr( name_str, legendary.explosive_potency );
      if ( util::str_compare_ci( split[ 1 ], "untamed_flame" ) )
        return make_ref_expr( name_str, legendary.untamed_flame );
    }
  }

  return fs_player_t::create_expression( name_str );
}

std::unique_ptr<expr_t> ardeos_t::create_resource_expression( util::string_view name_str )
{
  return fs_player_t::create_resource_expression( name_str );
}

// ardeos_t::init_base =======================================================

void ardeos_t::init_base_stats()
{
  if ( base.distance < 1 )
    base.distance = 5;

  fs_player_t::init_base_stats();

  base.stats.attribute[ STAT_INTELLECT ] = 1000;
  base.stats.attribute[ STAT_STAMINA ]   = 1000;
  resources.base[ RESOURCE_HEALTH ]      = 15927;

  base.health_per_stamina = 38.496;

  resources.base[ RESOURCE_CINDERS ] = 400;

  base_gcd = timespan_t::from_seconds( 1.5 );
  min_gcd  = timespan_t::from_seconds( 0.75 );
}

// ardeos_t::init_spells =====================================================

void ardeos_t::init_spells()
{
  fs_player_t::init_spells();

  // actions.auto_attack = new actions::auto_melee_attack_t( this, "" );
}

// ardeos_t::init_talents ====================================================

void ardeos_t::init_talents()
{
  fs_player_t::init_talents();
}

// ardeos_t::init_gains ======================================================

void ardeos_t::init_gains()
{
  fs_player_t::init_gains();

  gains.spirit_procs = get_gain( "Spirit Procs" );
}

// ardeos_t::init_procs ======================================================

void ardeos_t::init_procs()
{
  fs_player_t::init_procs();
}

// ardeos_t::init_rng ========================================================
void ardeos_t::init_rng()
{
  fs_player_t::init_rng();



 /* if ( talents.soulfrost_torrent )
    rppm.soulfrost_torrent = get_rppm( "soulfrost_torrent", talents.soulfrost_torrent_rppm, 1.0, RPPM_HASTE );*/
}

// ardeos_t::init_scaling ====================================================

void ardeos_t::init_scaling()
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

// ardeos_t::init_resources =================================================

void ardeos_t::init_resources( bool force )
{
  fs_player_t::init_resources( force );

  resources.current[ RESOURCE_WINTER_ORB ] = 0;
}

// ardeos_t::init_buffs ======================================================

void ardeos_t::create_buffs()
{
  fs_player_t::create_buffs();

  buffs.icy_flow = make_buff<ardeos_buff_t>( this, "icy_flow" )
                       ->set_duration( talents.icy_flow_buff_duration )
                       ->set_default_value( talents.icy_flow_cc )
                       ->set_max_stack( talents.icy_flow_max_stacks );

  buffs.flight_of_the_navir = make_buff<ardeos_buff_t>( this, "flight_of_the_navir" )->set_duration( 20_s );

  buffs.frostweavers_wrath = make_buff<ardeos_buff_t>( this, "frostweaver_wrath" )
                                 ->set_duration( talents.frostweavers_wrath_buff_duration )
                                 ->set_default_value( talents.frostweavers_wrath_added_cc );

  buffs.glacial_assault = make_buff<ardeos_buff_t>( this, "glacial_assault" )
                              ->set_default_value( talents.glacial_assault_amp )
                              ->set_max_stack( talents.glacial_assault_stacks );

  buffs.ice_blitz = make_buff<ardeos_buff_t>( this, "ice_blitz" )
                        ->set_duration( 20_s )
                        ->set_default_value( 0.2 )
                        ->add_invalidate( CACHE_PLAYER_DAMAGE_MULTIPLIER );

  buffs.soulfrost_torrent = make_buff<ardeos_buff_t>( this, "soulfrost_torrent" )
                                ->set_duration( talents.soulfrost_torrent_buff_duration )
                                ->set_default_value( talents.soulfrost_torrent_crit_chance );

  buffs.ultimate_buff_window =
      make_buff<ardeos_buff_t>( this, "wrath_of_winter" )
          ->set_default_value( 0.2 )
          ->set_duration( 20_s )
          ->set_period( 4_s )
          ->set_tick_on_application( true )
          ->set_tick_callback( [ this ]( buff_t*, int, timespan_t ) {
            resource_gain( RESOURCE_WINTER_ORB, 1.0, gains.ult_worbs, actions.wrath_of_winter );
          } )
          ->add_invalidate( CACHE_PLAYER_DAMAGE_MULTIPLIER );

  buffs.winters_blessing = make_buff<ardeos_buff_t>( this, "winters_blessing" )
                               ->set_duration( 20_s )
                               ->set_default_value( 0.20 )
                               ->set_pct_buff_type( STAT_PCT_BUFF_MASTERY );

  buffs.winters_embrace = make_buff<ardeos_buff_t>( this, "winters_embrace" )->set_default_value( 0.2 );

  buffs.frostwyrms_spite = make_buff<ardeos_buff_t>( this, "frostwyrms_spite" )
                               ->set_duration( legendary.frostwyrms_spite_duration )
                               ->set_max_stack( legendary.frostwyrms_spite_max_stacks )
                               ->set_default_value( legendary.frostwyrms_spite_dmg_per_stack )
                               ->set_refresh_behavior( buff_refresh_behavior::DURATION );

  buffs.undulating_spirit = make_buff<ardeos_buff_t>( this, "undulating_spirit" )->set_max_stack( 1 );
}

// ardeos_t::invalidate_cache =========================================

void ardeos_t::invalidate_cache( cache_e c )
{
  fs_player_t::invalidate_cache( c );
}

void ardeos_t::create_options()
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

// ardeos_t::copy_from =======================================================

void ardeos_t::copy_from( player_t* source )
{
  ardeos_t* ardeos = static_cast<ardeos_t*>( source );
  fs_player_t::copy_from( source );

  talents   = ardeos->talents;
  legendary = ardeos->legendary;
  options   = ardeos->options;
}

// ardeos_t::create_profile  =================================================

std::string ardeos_t::create_profile( save_e stype )
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

// ardeos_t::init_items ======================================================

void ardeos_t::init_items()
{
  fs_player_t::init_items();
}

// ardeos_t::init_special_effects ============================================

void ardeos_t::init_special_effects()
{
  fs_player_t::init_special_effects();
}

// ardeos_t::init_finished ===================================================

void ardeos_t::init_finished()
{
  fs_player_t::init_finished();
}

void ardeos_t::init_background_actions()
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

// ardeos_t::reset ===========================================================

void ardeos_t::reset()
{
  fs_player_t::reset();
}

// ardeos_t::activate ========================================================

void ardeos_t::activate()
{
  fs_player_t::activate();
}

// ardeos_t::arise ===========================================================

void ardeos_t::arise()
{
  fs_player_t::arise();

  resources.current[ RESOURCE_ANIMA ]      = 0;
  resources.current[ RESOURCE_WINTER_ORB ] = 0;
}

// ardeos_t::combat_begin ====================================================

void ardeos_t::combat_begin()
{
  fs_player_t::combat_begin();
}

double ardeos_t::resource_gain( resource_e resource_type, double amount, gain_t* source, action_t* action )
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

// ardeos_t::non_stacking_movement_modifier ==================================

double ardeos_t::non_stacking_movement_modifier() const
{
  double ms = fs_player_t::non_stacking_movement_modifier();

  return ms;
}

// ardeos_t::stacking_movement_modifier===================================

double ardeos_t::stacking_movement_modifier() const
{
  double ms = fs_player_t::stacking_movement_modifier();

  return ms;
}

template <typename Base>
void actions::ardeos_action_t<Base>::trigger_spirit_refund( const action_state_t* state, double orbs_refunded )
{
  make_event( ab::sim, 200_ms, [ orbs_refunded, this ] {
    p()->resource_gain( RESOURCE_WINTER_ORB, orbs_refunded, p()->gains.spirit_procs, this );
    p()->sim->print_debug( "{} actually refunded {:.0f} Winter Orbs", *p(), orbs_refunded );
  } );
  
  p()->spirit_refund();
}

template <typename Base>
void actions::ardeos_action_t<Base>::spend_winter_orbs( const action_state_t* s )
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
void actions::ardeos_action_t<Base>::gain_winter_orb( int gain )
{
}

template <typename Base>
void actions::ardeos_action_t<Base>::gain_anima( int gain )
{
}

// ardeos_t::convert_hybrid_stat ==============================================

stat_e ardeos_t::convert_hybrid_stat( stat_e s ) const
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

void ardeos_t::create_cooldowns()
{
  cooldowns.bursting_ice        = get_cooldown( "bursting_ice" );
  cooldowns.cold_snap           = get_cooldown( "cold_snap" );
  cooldowns.flight_of_the_navir = get_cooldown( "flight_of_the_navir" );
  cooldowns.freezing_torrent    = get_cooldown( "freezing_torrent" );
  cooldowns.ice_blitz           = get_cooldown( "ice_blitz" );
  cooldowns.winters_blessing    = get_cooldown( "winters_blessing" );
}

class ardeos_module_t : public module_t
{
public:
  ardeos_module_t() : module_t( ARDEOS )
  {
  }

  player_t* create_player( sim_t* sim, util::string_view name, race_e r = RACE_NONE ) const override
  {
    return new ardeos_t( sim, name, r );
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

}  // namespace ardeos
}  // namespace fellowship

const module_t* module_t::ardeos()
{
  static fellowship::ardeos::ardeos_module_t m;
  return &m;
}