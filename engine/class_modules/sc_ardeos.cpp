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
    std::vector<dot_t*> engulfing_flames_individual;
    dot_t* fire_ball;
    dot_t* fire_frog;
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
    return dots.incinerate->is_ticking() + dots.searing_blaze->is_ticking() + dots.engulfing_flames->current_stack() +
           dots.fire_ball->is_ticking() + dots.fire_frog->is_ticking() + dots.crackling_inferno->is_ticking();
    // fs_dots.amethyst_splinters->is_ticking() + fs_dots.curse_of_anzhyr->is_ticking() +
    // fs_dots.kindling->is_ticking();
  }

  int unique_dot_count() const
  {
    return dots.incinerate->is_ticking() + dots.searing_blaze->is_ticking() + dots.engulfing_flames->is_ticking() +
           dots.fire_ball->is_ticking() + dots.fire_frog->is_ticking() + dots.crackling_inferno->is_ticking();
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
    actions::ardeos_spell_t* fire_frog;
    actions::ardeos_spell_t* fire_frogs_hit;
    actions::ardeos_spell_t* fire_toads_hit;
    actions::ardeos_spell_t* infernal_wave;
    actions::ardeos_spell_t* searing_blaze;
    actions::ardeos_spell_t* engulfing_flames;
    actions::ardeos_spell_t* detonate;
    actions::ardeos_spell_t* wildfire;
    actions::ardeos_spell_t* pyromania;
    actions::ardeos_spell_t* crackling_inferno;
    actions::ardeos_spell_t* apocalypse;
    actions::ardeos_spell_t* flare_up;
  } actions;

  struct buffs_t
  {
    buff_t* reign_of_fire;
    buff_t* wildfire;
    buff_t* untamed_flame_crit;
    buff_t* untamed_flame_spirit;
  } buffs;

  struct cooldowns_t
  {
    cooldown_t* apocalypse;
    cooldown_t* engulfing_flames;
    cooldown_t* pyromania;
    cooldown_t* fire_ball;
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

  struct spell_const_t
  {
    timespan_t wildfire_duration = 9_s;
    double wildfire_tickrate     = 0.2;
    timespan_t wildfire_cooldown = 45_s;

    double infernal_wave_cinders = 40;
    double infernal_wave_coeff   = 1.55;

    double fire_frog_coeff             = 0.77;
    double fire_frog_dot_conversion    = 1.0;
    timespan_t fire_frog_dot_duration  = 12_s;
    timespan_t fire_frog_dot_period    = 3_s;
    timespan_t fire_frog_jump_duration = 0.3_s;
    unsigned fire_frog_max_jumps       = 3;
    unsigned fire_frog_frogs           = 5;
    timespan_t fire_frogs_cooldown     = 45_s;

    timespan_t pyromania_cooldown = 90_s;
    unsigned pyromania_targets    = 3;

    double searing_blaze_tick_coeff      = 0.717;
    timespan_t searing_blaze_duration    = 24_s;
    timespan_t searing_blaze_period      = 2_s;
    double searing_blaze_embers_per_tick = 1.0;  // This is currently reduced by haste.

    double engulfing_flames_tick_coeff      = 1.82;
    timespan_t engulfing_flames_duration    = 9_s;
    timespan_t engulfing_flames_period      = 1.5_s;
    double engulfing_flames_embers_per_tick = 5.0;  // This is currently reduced by haste.
    timespan_t engufling_flames_cooldown    = 20_s;
    int engulfing_flames_charges            = 1;

    timespan_t detonate_cast_time         = 1.0_s;  // Data has 1.5s but it is clearly faster than GCD still.
    double detonate_embers_cost           = 100;
    double detonate_hits                  = 3;      // Divides sample duration equally into these.
    timespan_t detonate_delay             = 0.5_s;  // Damage is sampled at cast time and snapshots.
    timespan_t detonate_between_hit_delay = 0.3_s;
    timespan_t detonate_sample_duration   = 2_s;

    double fire_ball_coeff            = 3.0*1.2;
    timespan_t fire_ball_cooldown     = 30_s;
    double fire_ball_falloff          = 5;
    double fire_ball_damage_to_dot    = 0.7;
    timespan_t fire_ball_dot_duration = 12_s;
    timespan_t fire_ball_dot_period   = 2_s;
    double fire_ball_embers_per_tick  = 2.0;  // This is **NOT** currently reduced by haste.
    double fire_ball_embers_chance    = 1.0;

    double incinerate_coeff             = 5.914;
    double incinerate_falloff           = 8;
    timespan_t incinerate_dot_extend    = 1.5_s;
    timespan_t incinerate_duration      = 2.5_s;
    timespan_t incinerate_period        = 0.5_s;
    timespan_t incinerate_dot_period    = 3_s;
    timespan_t incinerate_dot_duration  = 12_s;
    double incinerate_dot_coeff         = 0.39;
    double incinerate_dot_amp_per_stack = 0.3;

    timespan_t apocalypse_cast_time = 3_s;
    timespan_t apocalypse_cooldown  = 60_s;
    double apocalypse_coeff         = 23.19;
    double apocalypse_falloff       = 1;

  } spell_const;

  enum ardeos_talents_t : unsigned long long
  {
    NONE                   = 0ULL,
    SLOW_BURN              = 1ULL << 0,
    FROG_SQUAD             = 1ULL << 1,
    GREAT_BALLS_OF_FIRE    = 1ULL << 2,
    FLICKERING_CINDERS     = 1ULL << 3,
    FLARE_UP               = 1ULL << 4,
    UNDYING_FLAME          = 1ULL << 5,
    AGONIZING_BLAZE        = 1ULL << 6,
    FIRESTARTER            = 1ULL << 7,
    OUROBOROS              = 1ULL << 8,
    FIERY_RESILIENCE       = 1ULL << 9,
    CRACKLING_INFERNO      = 1ULL << 10,
    MAGIC_WARD             = 1ULL << 11,
    ROLLING_FLAMES         = 1ULL << 12,
    PYROPHIBIAN_FRENZY     = 1ULL << 13,
    REIGN_OF_FIRE          = 1ULL << 14,
    INTENSIFYING_INFERNO   = 1ULL << 15,
    SPIRITED_FORTITUDE     = 1ULL << 16,
    SPONTANEOUS_COMBUSTION = 1ULL << 17,
    MAX                    = 1ULL << 18
  };

  static constexpr std::string_view talent_name_formatted( ardeos_talents_t t )
  {
    switch ( t )
    {
      case ardeos_talents_t::SLOW_BURN:
        return "Slow Burn";
      case ardeos_talents_t::GREAT_BALLS_OF_FIRE:
        return "Great Balls of Fire";
      case ardeos_talents_t::FROG_SQUAD:
        return "Frog Squad";
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
      case ardeos_talents_t::FROG_SQUAD:
        return "frog_squad";
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

    double reign_of_fire_ppm          = 1.0;
    double reign_of_fire_crit_chance  = 1.0;
    timespan_t reign_of_fire_duration = 12_s;

    double intensifying_inferno_amp = 0.15;

    double spontaneous_combustion_chance       = 0.04;
    double spontaneous_combustion_extra_chance = 0.05;

    bool rolling_flames_instant = false;

    bool double_detonate_cost_efficiency        = false;
    timespan_t rolling_flames_infernal_wave_cdr = 0_ms;
    timespan_t detonate_extends_searing         = 0_s;
  } talents;

  struct legendary_t
  {
    bool untamed_flame               = false;
    double untamed_flame_spirit      = 0.3;
    double untamed_flame_crit_chance = 0.3;

    bool fire_toad              = false;
    double fire_toad_chance     = 0.12;
    double fire_toad_mul        = 8;
    double fire_toad_falloff    = 1;
    bool fire_toad_full_primary = false;
    bool fire_toad_on_cast      = false;

    bool brimstone_cataclysm                   = false;
    timespan_t brimstone_cataclysm_cdr_per_hit = 2_s;
    timespan_t brimstone_cataclysm_cdr_cap     = 30_s;

    bool engulf_stacker       = false;
    double engulf_stacker_amp = 0.1;
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

  void analyze( sim_t& sim ) override;

  double resource_gain( resource_e r, double amount, gain_t* source = nullptr, action_t* a = nullptr ) override;

  void extend_engulfing_flames( player_t* target, timespan_t duration );
  void extend_dot( dot_t* d, timespan_t duration );
  void extend_dots( player_t* target, timespan_t duration );

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

  double current_cinders( bool /* react */ = false ) const
  {
    return resources.current[ RESOURCE_CINDERS ];
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

  ardeos_t( sim_t* sim, util::string_view name, race_e r = RACE_NONE )
    : fs_player_t( sim, name, r, ARDEOS ), target_data()
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
    ab::school                       = SCHOOL_FIRE;

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
  virtual double generate_cinders() const
  {
    double cp = 0;

    if ( ab::energize_type != action_energize::NONE && ab::energize_resource == RESOURCE_CINDERS )
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
  void spend_cinders( const action_state_t* );
  void gain_cinders( int );
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

    if ( !ab::channeled && p()->buffs.wildfire->check() )
    {
      tt /= 1 + p()->buffs.wildfire->check_value();
    }

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

    spend_cinders( ab::execute_state );
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

    if ( ab::base_costs[ RESOURCE_CINDERS ] > 0 && p()->current_cinders() < ab::base_costs[ RESOURCE_CINDERS ] )
      return false;

    return true;
  }

  std::unique_ptr<expr_t> create_expression( std::string_view name ) override
  {
    if ( util::str_compare_ci( name, "cinders_gain" ) )
    {
      return make_mem_fn_expr( "cinders_gain", *this, &base_t::generate_cinders );
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

struct crackling_inferno_t : public residual_action::residual_periodic_action_t<ardeos_spell_t>
{
  crackling_inferno_t( ardeos_t* p ) : residual_action_t( "crackling_inferno", p )
  {
    id = 20;

    background = true;

    name_str_reporting = "Crackling Inferno";

    dot_duration           = p->talents.crackling_inferno_dot_duration;
    dot_behavior           = DOT_REFRESH_DURATION;
    base_tick_time         = p->talents.crackling_inferno_dot_period;
    hasted_ticks           = true;
    dot_allow_partial_tick = true;

    base_multiplier *= p->talents.crackling_inferno_dot_fraction;

    base_crit += p->talents_enabled( ardeos_t::FIRESTARTER ) ? p->talents.firestarter_crit_chance : 0.0;
  }

  void snapshot_state( action_state_t* state, result_amount_type rt ) override
  {
    spell_t::snapshot_state( state, rt );
  }

  double composite_ta_multiplier( const action_state_t* s ) const override
  {
    return fs_player_action_t::action_multiplier();
  }

  void init() override
  {
    base_t::init();
    snapshot_flags &= STATE_NO_MULTIPLIER;
    update_flags &= STATE_NO_MULTIPLIER;
    snapshot_flags |= STATE_HASTE | STATE_MUL_TA;
    update_flags &= ~STATE_HASTE;

    snapshot_flags |= STATE_CRIT | STATE_TGT_CRIT;
    update_flags |= STATE_CRIT | STATE_TGT_CRIT;
  }
};

struct flare_up_t : public ardeos_spell_t
{
  flare_up_t( ardeos_t* p ) : ardeos_spell_t( "flare_up", p )
  {
    id = 23;

    name_str_reporting = "Flare Up";

    may_crit = false;

    base_multiplier *= p->talents.flare_up_multiplier;
    aoe = -1;
  }

  double composite_ta_multiplier( const action_state_t* s ) const override
  {
    return fs_player_action_t::action_multiplier();
  }

  void init() override
  {
    base_t::init();

    snapshot_flags &= STATE_NO_MULTIPLIER;
    update_flags &= STATE_NO_MULTIPLIER;

    snapshot_flags |= STATE_MUL_DA;
  }

  size_t available_targets( std::vector<player_t*>& tl ) const override
  {
    tl.clear();

    for ( auto* t : sim->target_non_sleeping_list )
    {
      if ( t->is_enemy() && p()->get_target_data( t )->dots.searing_blaze->is_ticking() )
      {
        tl.push_back( t );
      }
    }

    return tl.size();
  }

  void execute() override
  {
    target_cache.is_valid = false;

    if ( target_list().size() > 0 )
      ardeos_spell_t::execute();
  }

  void impact( action_state_t* s ) override
  {
    ardeos_spell_t::impact( s );
  }
};

struct infernal_wave_t : public ardeos_spell_t
{
  infernal_wave_t( util::string_view name, ardeos_t* p, util::string_view options_str = {} )
    : ardeos_spell_t( name, p, options_str )
  {
    id = 2;

    spell_power_mod.direct = p->spell_const.infernal_wave_coeff;

    name_str_reporting = "Infernal Wave";

    energize_type     = action_energize::ON_CAST;
    energize_resource = RESOURCE_CINDERS;
    energize_amount   = p->spell_const.infernal_wave_cinders;

    base_execute_time = 1.5_s;

    if ( !p->actions.flare_up->stats->parent )
      add_child( p->actions.flare_up );
  }

  double composite_da_multiplier( const action_state_t* s ) const override
  {
    auto da = ardeos_spell_t::composite_da_multiplier( s );

    if ( p()->talents_enabled( ardeos_t::INTENSIFYING_INFERNO ) )
    {
      da *= 1 + p()->get_target_data( s->target )->unique_dot_count() * p()->talents.intensifying_inferno_amp;
    }

    return da;
  }

  void execute() override
  {
    ardeos_spell_t::execute();

    if ( p()->talents_enabled( ardeos_t::ROLLING_FLAMES ) )
    {
      p()->cooldowns.engulfing_flames->adjust( -p()->talents.rolling_flames_infernal_wave_cdr, false );
    }
  }

  void impact( action_state_t* s ) override
  {
    ardeos_spell_t::impact( s );

    if ( s->result_amount > 0 )
    {
      if ( s->result == RESULT_CRIT && p()->talents_enabled( ardeos_t::CRACKLING_INFERNO ) )

      {
        residual_action::trigger( p()->actions.crackling_inferno, s->target, s->result_amount );
      }

      if ( p()->talents_enabled( ardeos_t::FLARE_UP ) && result_is_hit( s->result ) )
      {
        p()->actions.flare_up->execute_on_target( s->target, s->result_amount );
      }
    }
  }
};

struct detonate_t : public ardeos_spell_t
{
  struct detonate_damage_t : public ardeos_spell_t
  {
    timespan_t sample_duration;
    detonate_damage_t( util::string_view name, ardeos_t* p, util::string_view options_str = {} )
      : ardeos_spell_t( fmt::format( "{}_dmg", name ), p, options_str ),
        sample_duration( p->spell_const.detonate_sample_duration )
    {
      id                 = 3;
      name_str_reporting = "Detonate";

      aoe = -1;
      // tested 08/12/2025 - No reduced AOE currently.
      // reduced_aoe_targets = 1;

      dual = background = true;

      if ( p->talents.double_detonate_cost_efficiency )
        sample_duration *= 2;
    }

    double tick_damage_over_time( const dot_t* dot ) const
    {
      if ( !dot->is_ticking() )
      {
        return 0.0;
      }

      action_state_t* state = dot->current_action->get_state( dot->state );
      dot->current_action->calculate_tick_amount( state, 1.0 );
      double tick_base_damage  = state->result_raw;
      timespan_t dot_tick_time = dot->current_action->tick_time( state );
      // We don't care how much is remaining on the target, this will always deal
      // Xs worth of DoT ticks even if the amount is currently less
      double ticks_left   = sample_duration / dot_tick_time;
      double total_damage = ticks_left * tick_base_damage;
      total_damage /= state->target_ta_multiplier;
      action_state_t::release( state );

      sim->print_debug( "{} Detonate tick damage from {}: {:.2f} ({:.2f} per tick for {:.2f} ticks)", *p(),
                        *dot->current_action, total_damage, tick_base_damage, ticks_left );
      return total_damage;
    }

    double get_detonate_damage( player_t* target ) const
    {
      double da = 0.0;

      auto td = p()->get_target_data( target );
      da += tick_damage_over_time( td->dots.crackling_inferno );
      for ( auto& engulf : td->dots.engulfing_flames_individual )
      {
        da += tick_damage_over_time( engulf );
      }
      da += tick_damage_over_time( td->dots.fire_ball );
      da += tick_damage_over_time( td->dots.fire_frog );
      da += tick_damage_over_time( td->dots.incinerate );
      da += tick_damage_over_time( td->dots.searing_blaze );

      return da / p()->spell_const.detonate_hits;
    }

    void init() override
    {
      ardeos_spell_t::init();

      snapshot_flags &= ~( STATE_VERSATILITY | STATE_MUL_PLAYER_DAM );
      update_flags &= ~( STATE_VERSATILITY | STATE_MUL_PLAYER_DAM );

      snapshot_flags |= STATE_TGT_CRIT | STATE_TGT_MUL_DA;
      update_flags |= STATE_TGT_CRIT | STATE_TGT_MUL_DA;
    }

    void snapshot_internal( action_state_t* s, unsigned flags, result_amount_type rt ) override
    {
      ardeos_spell_t::snapshot_internal( s, flags, rt );

      base_dd_min = base_dd_max = get_detonate_damage( s->target );
    }

    double composite_target_multiplier( player_t* target ) const override
    {
      double m = ardeos_spell_t::composite_target_multiplier( target );
      //if ( p()->legendary.explosive_potency )
      //{
      //  if ( target->health_percentage() <= 35.0 )
      //  {
      //    m *= 1.0 + p()->legendary.explosive_potency_detonate_amp;
      //  }
      //}
      return m;
    }

    void impact( action_state_t* s )
    {
      ardeos_spell_t::impact( s );
      if ( p()->talents.detonate_extends_searing > 0_s )
      {
        const auto& td = p()->get_target_data( s->target );
        p()->extend_dot( td->dots.searing_blaze, p()->talents.detonate_extends_searing );
      }
    }
  };

  detonate_damage_t* damage_action;
  detonate_t( util::string_view name, ardeos_t* p, util::string_view options_str = {} )
    : ardeos_spell_t( name, p, options_str )
  {
    id                 = 3;
    name_str_reporting = "Detonate";

    resource_current               = RESOURCE_CINDERS;
    base_costs[ RESOURCE_CINDERS ] = p->spell_const.detonate_embers_cost;

    if ( p->talents.double_detonate_cost_efficiency )
      base_costs[ RESOURCE_CINDERS ] *= 2;

    base_execute_time = 0_s;
    trigger_gcd       = 1_s;
    gcd_type          = gcd_haste_type::NONE;

    damage_action        = new detonate_damage_t( name, p, options_str );
    damage_action->stats = stats;
  }

  void execute() override
  {
    ardeos_spell_t::execute();

    for ( int i = 0; i < p()->spell_const.detonate_hits; ++i )
    {
      damage_action->execute();
    }

    if ( p()->talents_enabled( ardeos_t::REIGN_OF_FIRE ) )
    {
      if ( p()->rppm.reign_of_fire->trigger() )
      {
        p()->buffs.reign_of_fire->trigger();
        p()->cooldowns.fire_ball->reset( true, 1 );
      }
    }
  }
};

struct wildfire_t : public ardeos_spell_t
{
  wildfire_t( util::string_view name, ardeos_t* p, util::string_view options_str = {} )
    : ardeos_spell_t( name, p, options_str )
  {
    id = 7;

    name_str_reporting = "Wildfire";

    trigger_gcd = timespan_t::zero();

    cooldown->duration = p->spell_const.wildfire_cooldown;
    cooldown->hasted   = false;
    cooldown->charges  = 1;
  }

  void execute() override
  {
    p()->buffs.wildfire->trigger();
    ardeos_spell_t::execute();
  }
};

struct incinerate_t : public ardeos_spell_t
{
  struct incinerate_dot_t : public ardeos_spell_t
  {
    incinerate_dot_t( util::string_view name, ardeos_t* p, util::string_view options_str = {} )
      : ardeos_spell_t( fmt::format( "{}_dot", name ), p, options_str )
    {
      id = 9;

      name_str_reporting   = "Incinerate (DoT)";
      spell_power_mod.tick = p->spell_const.incinerate_dot_coeff;
      dot_duration         = p->spell_const.incinerate_dot_duration;
      base_tick_time       = p->spell_const.incinerate_dot_period;
      hasted_ticks         = true;
      dot_behavior         = DOT_REFRESH_DURATION;

      base_crit += p->talents_enabled( ardeos_t::FIRESTARTER ) ? p->talents.firestarter_crit_chance : 0.0;
    }

    double composite_ta_multiplier( const action_state_t* s ) const override
    {
      double m = ardeos_spell_t::composite_ta_multiplier( s );

      auto td = p()->get_target_data( s->target );
      m *= 1.0 + td->debuffs.incinerate_stacks->check_stack_value();

      return m;
    }

    void trigger_dot( action_state_t* s ) override
    {
      ardeos_spell_t::trigger_dot( s );
      p()->get_target_data( s->target )->debuffs.incinerate_stacks->increment();
    }

    void last_tick( dot_t* d ) override
    {
      ardeos_spell_t::last_tick( d );
      p()->get_target_data( d->target )->debuffs.incinerate_stacks->expire();
    }
  };

  struct incinerate_aoe_t : public ardeos_spell_t
  {
    action_t* dot;
    timespan_t engulf_duration;
    incinerate_aoe_t( util::string_view name, ardeos_t* p, util::string_view options_str = {} )
      : ardeos_spell_t( fmt::format( "{}_aoe", name ), p, options_str ),
        engulf_duration( p->talents_enabled( ardeos_t::UNDYING_FLAME )
                             ? p->spell_const.engulfing_flames_duration + p->talents.undying_flame_extension
                             : p->spell_const.engulfing_flames_duration )
    {
      id = 9;

      aoe                    = -1;
      name_str_reporting     = "Incinerate";
      spell_power_mod.direct = p->spell_const.incinerate_coeff;
      reduced_aoe_targets    = p->spell_const.incinerate_falloff;

      dot = new incinerate_dot_t( name, p, options_str );
      add_child( dot );
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

    void impact( action_state_t* s ) override
    {
      ardeos_spell_t::impact( s );
      // Apply dot
      dot->set_target( s->target );
      dot->execute();

      p()->extend_dots( s->target, p()->spell_const.incinerate_dot_extend );
    }
  };

  struct incinerate_channel_t : public ardeos_spell_t
  {
    action_t* custom_tick_action;
    action_t* parent;

    incinerate_channel_t( util::string_view name, ardeos_t* p, action_t* parent, util::string_view options_str = {} )
      : ardeos_spell_t( fmt::format( "{}_channel", name ), p, options_str ), parent( parent )
    {
      id = 9;

      name_str_reporting = "Incinerate";

      dual = true;

      channeled              = true;
      dot_allow_partial_tick = true;
      dot_duration           = p->spell_const.incinerate_duration;
      hasted_ticks           = true;
      base_tick_time         = p->spell_const.incinerate_period;

      target = p;

      custom_tick_action = new incinerate_aoe_t( name, p, options_str );
    }

    void execute() override
    {
      target = player;
      ardeos_spell_t::execute();
    }

    void tick( dot_t* d ) override
    {
      ardeos_spell_t::tick( d );
      custom_tick_action->set_target( parent->target );
      custom_tick_action->execute();
    }
  };

  incinerate_channel_t* channel_action;
  incinerate_t( util::string_view name, ardeos_t* p, util::string_view options_str = {} )
    : ardeos_spell_t( name, p, options_str )
  {
    id = 9;

    name_str_reporting = "Incinerate";

    base_execute_time = 1_s;

    resource_current              = RESOURCE_SPIRIT;
    base_costs[ RESOURCE_SPIRIT ] = 100;

    channel_action        = new incinerate_channel_t( name, p, this, options_str );
    channel_action->stats = stats;
    add_child( channel_action->custom_tick_action );
  }

  void execute() override
  {
    ardeos_spell_t::execute();
    p()->fs_buffs.spirit_of_heroism->trigger();
    p()->used_ultimate();
    channel_action->execute();
  }
};

struct searing_blaze_t : public ardeos_spell_t
{
  searing_blaze_t( util::string_view name, ardeos_t* p, util::string_view options_str = {} )
    : ardeos_spell_t( name, p, options_str )
  {
    id                     = 11;
    name_str_reporting     = "Searing Blaze";
    spell_power_mod.tick   = p->spell_const.searing_blaze_tick_coeff;
    dot_duration           = p->spell_const.searing_blaze_duration;
    base_tick_time         = p->spell_const.searing_blaze_period;
    dot_allow_partial_tick = true;
    hasted_ticks           = true;

    dot_behavior = DOT_REFRESH_PANDEMIC;

    base_execute_time = 0_s;

    energize_type     = action_energize::NONE;
    energize_amount   = p->spell_const.searing_blaze_embers_per_tick;
    energize_resource = RESOURCE_CINDERS;

    if ( p->talents_enabled( ardeos_t::FLICKERING_CINDERS ) )
    {
      energize_amount *= 1 + p->talents.flickering_cinders_cinder_multiplier;
    }

    base_crit += p->talents_enabled( ardeos_t::FIRESTARTER ) ? p->talents.firestarter_crit_chance : 0.0;
  }

  void execute() override
  {
    ardeos_spell_t::execute();
  }

  double composite_ta_multiplier( const action_state_t* s ) const override
  {
    double m = ardeos_spell_t::composite_ta_multiplier( s );

    auto td = p()->get_target_data( s->target );
    m *= 1.0 + td->debuffs.agonizing_blaze_stacks->check_stack_value();

    return m;
  }

  void impact( action_state_t* s ) override
  {
    ardeos_spell_t::impact( s );
  }

  double spontaneous_chance() const
  {
    return p()->talents.spontaneous_combustion_chance +
           0.01 * p()->cache.spell_crit_chance() / p()->talents.spontaneous_combustion_extra_chance;
  }

  double composite_crit_chance() const override
  {
    auto cc = ardeos_spell_t::composite_crit_chance();

    if ( p()->talents_enabled( ardeos_t::SPONTANEOUS_COMBUSTION ) && rng().roll( spontaneous_chance() ) )
      cc += 1.0;

    return cc;
  }

  void tick( dot_t* d ) override
  {
    ardeos_spell_t::tick( d );

    if ( p()->talents_enabled( ardeos_t::AGONIZING_BLAZE ) )
    {
      // Currently a dot ticking increases all stacks. This is similar but not identical, but significantly more
      // performant.
      p()->get_target_data( d->target )
          ->debuffs.agonizing_blaze_stacks->increment(
              p()->get_active_dots( p()->get_target_data( d->target )->dots.searing_blaze ) );
    }

    p()->resource_gain( RESOURCE_CINDERS, energize_amount * p()->cache.spell_haste(), energize_gain( d->state ), this );

    if ( p()->talents_enabled( ardeos_t::ROLLING_FLAMES ) )
    {
      p()->cooldowns.engulfing_flames->adjust( -p()->talents.rolling_flames_cdr, true );
    }

    if ( p()->talents_enabled( ardeos_t::OUROBOROS ) )
    {
      p()->cooldowns.pyromania->adjust(
          -( d->state->result == RESULT_CRIT ? p()->talents.ouroboros_cdr_crit : p()->talents.ouroboros_cdr ), true );
    }
  }

  void last_tick( dot_t* d ) override
  {
    ardeos_spell_t::last_tick( d );
    if ( p()->talents_enabled( ardeos_t::AGONIZING_BLAZE ) )
    {
      p()->get_target_data( d->target )->debuffs.agonizing_blaze_stacks->expire();
    }
  }
};

struct engulfing_flames_t : public ardeos_spell_t
{
  engulfing_flames_t( ardeos_t* p, util::string_view options_str = {} )
    : ardeos_spell_t( "engulfing_flames", p, options_str )
  {
    id                     = 12;
    name_str_reporting     = "Engulfing Flames";
    spell_power_mod.tick   = p->spell_const.engulfing_flames_tick_coeff;
    dot_duration           = p->spell_const.engulfing_flames_duration;
    base_tick_time         = p->spell_const.engulfing_flames_period;
    dot_allow_partial_tick = true;
    hasted_ticks           = true;

    dot_behavior = DOT_REFRESH_PANDEMIC;

    if ( p->talents_enabled( ardeos_t::UNDYING_FLAME ) )
      dot_duration += p->talents.undying_flame_extension;

    base_execute_time = 1.5_s;

    if ( p->talents_enabled( ardeos_t::ROLLING_FLAMES ) && p->talents.rolling_flames_instant )
    {
      base_execute_time = 0_s;
    }

    cooldown->duration = p->spell_const.engufling_flames_cooldown;
    cooldown->hasted   = false;
    cooldown->charges  = p->spell_const.engulfing_flames_charges;

    energize_type     = action_energize::NONE;
    energize_amount   = p->spell_const.engulfing_flames_embers_per_tick;
    energize_resource = RESOURCE_CINDERS;

    if ( p->talents_enabled( ardeos_t::FLICKERING_CINDERS ) )
    {
      energize_amount *= 1 + p->talents.flickering_cinders_cinder_multiplier;
    }

    base_crit += p->talents_enabled( ardeos_t::FIRESTARTER ) ? p->talents.firestarter_crit_chance : 0.0;
  }

  dot_t* get_dot( player_t* t, size_t i )
  {
    if ( !t )
      t = target;
    if ( !t )
      return nullptr;

    std::vector<dot_t*>& target_dots = p()->get_target_data( t )->dots.engulfing_flames_individual;

    if ( i >= target_dots.size()  )
    {
      for ( size_t j = target_dots.size(); j <= i; ++j )
      {
        target_dots.push_back( t->get_dot( fmt::format( "{}_{}", name_str, j + 1 ), player ) );
      }
    }

    return target_dots.at( i );
  }

  dot_t* get_first_ticking( player_t* t )
  {
    if ( !t )
      t = target;
    if ( !t )
      return nullptr;

    std::vector<dot_t*>& target_dots = p()->get_target_data( t )->dots.engulfing_flames_individual;

    for ( dot_t* d : target_dots )
    {
      if ( d->is_ticking() )
        return d;
    }

    return nullptr;
  }

  dot_t* get_first_missing( player_t* t )
  {
    if ( !t )
      t = target;
    if ( !t )
      return nullptr;

    std::vector<dot_t*>& target_dots = p()->get_target_data( t )->dots.engulfing_flames_individual;

    for ( dot_t* d : target_dots )
    {
      if ( !d->is_ticking() )
        return d;
    }

    return get_dot( t, target_dots.size() );
  }

  dot_t* get_dot( player_t* t ) override
  {
    ardeos_spell_t::get_dot( t );
    return get_first_missing( t );
  }

  void tick( dot_t* d ) override
  {
    ardeos_spell_t::tick( d );

    p()->resource_gain( RESOURCE_CINDERS, energize_amount * p()->cache.spell_haste(), energize_gain( d->state ), this );
  }

  double spontaneous_chance() const
  {
    return p()->talents.spontaneous_combustion_chance +
           0.01 * p()->cache.spell_crit_chance() / p()->talents.spontaneous_combustion_extra_chance;
  }

  double composite_crit_chance() const override
  {
    auto cc = ardeos_spell_t::composite_crit_chance();

    if ( p()->talents_enabled( ardeos_t::SPONTANEOUS_COMBUSTION ) && rng().roll( spontaneous_chance() ) )
      cc += 1.0;

    return cc;
  }

  void trigger_dot( action_state_t* s ) override
  {
    ardeos_spell_t::trigger_dot( s );

    dot_t* base_dot = ardeos_spell_t::get_dot( s->target );

    base_dot->current_action = this;
    base_dot->max_stack = 9999;

    if ( !base_dot->state )
      base_dot->state = get_state();
    base_dot->state->copy_state( s );

    base_dot->false_start( composite_dot_duration( s ) );
  }

  void last_tick( dot_t* d ) override
  {
    ardeos_spell_t::last_tick( d );
    dot_t* base_dot = ardeos_spell_t::get_dot( d->target );
    if ( base_dot->current_stack() > 1 )
    {
      base_dot->decrement( 1 );
      p()->remove_active_dot( base_dot );
    }
    else
    {
      base_dot->reset();
    }
  }

  bool dot_refreshable( const dot_t*, timespan_t ) const override
  {
    return true;
  }

  void execute() override
  {
    ardeos_spell_t::execute();
  }

  void impact( action_state_t* s ) override
  {
    ardeos_spell_t::impact( s );
  }
};

struct apocalypse_t : public ardeos_spell_t
{
  apocalypse_t( util::string_view name, ardeos_t* p, util::string_view options_str = {} )
    : ardeos_spell_t( name, p, options_str )
  {
    id                 = 13;
    name_str_reporting = "Apocalypse";

    spell_power_mod.direct = p->spell_const.apocalypse_coeff;

    base_execute_time = p->spell_const.apocalypse_cast_time;

    aoe                 = -1;
    reduced_aoe_targets = p->spell_const.apocalypse_falloff;

    cooldown->duration = p->spell_const.apocalypse_cooldown;
    cooldown->hasted   = false;
    cooldown->charges  = 1;
  }

  void impact( action_state_t* s ) override
  {
    ardeos_spell_t::impact( s );
    p()->actions.searing_blaze->execute_on_target( s->target );
  }

  void execute() override
  {
    ardeos_spell_t::execute();

    if ( p()->legendary.brimstone_cataclysm )
    {
      timespan_t cdr = std::min( p()->legendary.brimstone_cataclysm_cdr_cap,
                                 p()->legendary.brimstone_cataclysm_cdr_per_hit * execute_state->n_targets );

      p()->cooldowns.apocalypse->adjust( -cdr, false );
    }
  }
};

struct fire_ball_t : public ardeos_spell_t
{
  struct fire_ball_dot_t : public residual_action::residual_periodic_action_t<ardeos_spell_t>
  {
    fire_ball_dot_t( util::string_view name, ardeos_t* p ) : residual_action_t( fmt::format( "{}_dot", name ), p )
    {
      id = 14;

      background = true;

      name_str_reporting = "Fire Ball (DoT)";

      tick_may_crit = p->talents_enabled( ardeos_t::FIRESTARTER );

      dot_duration           = p->spell_const.fire_ball_dot_duration;
      dot_behavior           = DOT_REFRESH_DURATION;
      base_tick_time         = p->spell_const.fire_ball_dot_period;
      hasted_ticks           = true;
      dot_allow_partial_tick = true;

      base_multiplier *= p->spell_const.fire_ball_damage_to_dot;

      //energize_type     = action_energize::PER_TICK;
      // Manually handle it.
      energize_type     = action_energize::NONE;
      energize_amount   = p->spell_const.fire_ball_embers_per_tick;
      energize_resource = RESOURCE_CINDERS;

      base_crit = p->talents_enabled( ardeos_t::FIRESTARTER ) ? p->talents.firestarter_crit_chance : 0.0;
    }

    void snapshot_state( action_state_t* state, result_amount_type rt ) override
    {
      spell_t::snapshot_state( state, rt );
    }

    double composite_ta_multiplier( const action_state_t* s ) const override
    {
      return fs_player_action_t::action_multiplier();
    }

    void init() override
    {
      base_t::init();
      snapshot_flags &= STATE_NO_MULTIPLIER;
      update_flags &= STATE_NO_MULTIPLIER;
      snapshot_flags |= STATE_HASTE | STATE_MUL_TA;
      update_flags &= ~STATE_HASTE;
      snapshot_flags &= ~STATE_TGT_CRIT;
      update_flags &= ~STATE_TGT_CRIT;

      if ( p()->talents_enabled( ardeos_t::FIRESTARTER ) )
      {
        snapshot_flags |= STATE_CRIT;
      }
    }

    double composite_crit_chance() const override
    {
      return base_crit;
    }

    void tick( dot_t* d ) override
    {
      residual_action_t::tick( d );

      if ( rng().roll( p()->spell_const.fire_ball_embers_chance ) )
      {
        gain_energize_resource( energize_resource_(), composite_energize_amount( d->state ), gain );
      }

      if ( p()->talents_enabled( ardeos_t::SLOW_BURN ) )
      {
        p()->extend_engulfing_flames( d->target, p()->talents.slow_burn_extend );
        auto td = p()->get_target_data( d->target );
        p()->extend_dot( td->dots.searing_blaze, p()->talents.slow_burn_extend );
      }
    }
  };

  fire_ball_dot_t* dot_action;
  fire_ball_t( util::string_view name, ardeos_t* p, util::string_view options_str = {} )
    : ardeos_spell_t( name, p, options_str )
  {
    id                 = 14;
    name_str_reporting = "Fire Ball";

    spell_power_mod.direct = p->spell_const.fire_ball_coeff;

    base_execute_time = 0_s;

    aoe                 = -1;
    reduced_aoe_targets = p->spell_const.fire_ball_falloff;

    cooldown->duration = p->spell_const.fire_ball_cooldown;
    cooldown->hasted   = true;
    cooldown->charges  = 2;

    dot_action = new fire_ball_dot_t( name, p );
    add_child( dot_action );

    if ( p->talents_enabled( ardeos_t::GREAT_BALLS_OF_FIRE ) )
    {
      base_multiplier *= 1.0 + p->talents.great_balls_of_fire_amp;
    }
  }

  double composite_crit_chance() const override
  {
    double c = ardeos_spell_t::composite_crit_chance();

    if ( p()->buffs.reign_of_fire->check() )
    {
      c += p()->buffs.reign_of_fire->check_value();
    }

    return c;
  }

  void impact( action_state_t* s ) override
  {
    ardeos_spell_t::impact( s );
    residual_action::trigger( dot_action, s->target, s->result_amount );
  }

  void execute() override
  {
    ardeos_spell_t::execute();
    p()->buffs.reign_of_fire->decrement();
  }
};

struct pyromania_t : public ardeos_spell_t
{
  pyromania_t( util::string_view name, ardeos_t* p, util::string_view options_str = {} )
    : ardeos_spell_t( name, p, options_str )
  {
    id = 22;

    name_str_reporting = "Pyromania";

    trigger_gcd = timespan_t::zero();

    aoe = p->spell_const.pyromania_targets;

    cooldown->duration = p->spell_const.pyromania_cooldown;
    cooldown->hasted   = false;
    cooldown->charges  = 1;
  }

  void impact( action_state_t* s ) override
  {
    ardeos_spell_t::impact( s );

    p()->actions.engulfing_flames->execute_on_target( s->target );
  }

  void execute() override
  {
    if ( target_list().size() > 1 )
    {
      auto partition = std::partition( target_list().begin() + 1, target_list().end(), [ this ]( player_t* a ) {
        return !p()->get_target_data( a )->dots.engulfing_flames->is_ticking();
      } );

      std::sort( target_list().begin() + 1, partition,
                 []( player_t* a, player_t* b ) { return a->current_health() > b->current_health(); } );

      std::sort( partition, target_list().end(),
                 []( player_t* a, player_t* b ) { return a->current_health() > b->current_health(); } );
    }

    ardeos_spell_t::execute();
  }
};

struct fire_frog_dot_t : public residual_action::residual_periodic_action_t<ardeos_spell_t>
{
  fire_frog_dot_t( std::string_view n, fs_player_t* p ) : residual_action_t( n, static_cast<ardeos_t*>( p ) )
  {
    id = 18;

    ardeos_t* player = static_cast<ardeos_t*>( p );

    background = true;

    name_str_reporting = "Fire Frog (DoT)";

    tick_may_crit = player->talents_enabled( ardeos_t::FIRESTARTER );

    dot_duration           = player->spell_const.fire_frog_dot_duration;
    dot_behavior           = DOT_REFRESH_DURATION;
    base_tick_time         = player->spell_const.fire_frog_dot_period;
    hasted_ticks           = true;
    dot_allow_partial_tick = true;

    base_multiplier *= player->spell_const.fire_frog_coeff;

    base_crit = player->talents_enabled( ardeos_t::FIRESTARTER ) ? player->talents.firestarter_crit_chance : 0.0;
  }

  void snapshot_state( action_state_t* state, result_amount_type rt ) override
  {
    spell_t::snapshot_state( state, rt );
  }

  double composite_ta_multiplier( const action_state_t* s ) const override
  {
    return fs_player_action_t::action_multiplier();
  }

  void init() override
  {
    base_t::init();
    snapshot_flags &= STATE_NO_MULTIPLIER;
    update_flags &= STATE_NO_MULTIPLIER;
    snapshot_flags |= STATE_HASTE | STATE_MUL_TA;
    update_flags &= ~STATE_HASTE;
    snapshot_flags &= ~STATE_TGT_CRIT;
    update_flags &= ~STATE_TGT_CRIT;

    if ( p()->talents_enabled( ardeos_t::FIRESTARTER ) )
    {
      snapshot_flags |= STATE_CRIT;
    }
  }

  double composite_crit_chance() const override
  {
    return base_crit;
  }
};

struct fire_frog_hit_t : public ardeos_spell_t
{

  fire_frog_dot_t* dot_action;
  fire_frog_hit_t( ardeos_t* p, bool toad ) : ardeos_spell_t( toad ? "fire_toad_hit" : "fire_frog_hit", p )
  {
    id                 = 17;
    name_str_reporting = toad  ? "Large Lad" : "Fire Frog";

    spell_power_mod.direct = p->spell_const.fire_frog_coeff;
    background             = true;

    dot_action = p->get_background_action<fire_frog_dot_t>( "fire_frog_dot" );

    if ( toad )
    {
      base_dd_multiplier *= p->legendary.fire_toad_mul;
      aoe                 = -1;
      full_amount_targets = p->legendary.fire_toad_full_primary ? 1 : 0;
      reduced_aoe_targets = p->legendary.fire_toad_falloff;
      travel_delay        = 0.5;
    }
  }

  void impact( action_state_t* s ) override
  {
    ardeos_spell_t::impact( s );
    residual_action::trigger( dot_action, s->target, s->result_amount );
  }

  void execute() override
  {
    // Always target a random target
    target = rng().range( target_list() );
    ardeos_spell_t::execute();
  }
};

struct fire_frog_t : public ardeos_spell_t
{
  int frog_hits;
  fire_frog_t( ardeos_t* p ) : ardeos_spell_t( "fire_frog", p ), frog_hits( p->spell_const.fire_frog_max_jumps )
  {
    id                 = 16;
    name_str_reporting = "Fire Frog";

    background = true;

    if ( p->talents_enabled( ardeos_t::FROG_SQUAD ) )
    {
      frog_hits += p->talents.frog_squad_extra_hits;
    }
  }

  void execute() override
  {
    ardeos_spell_t::execute();

    if ( p()->legendary.fire_toad && rng().roll( p()->legendary.fire_toad_chance ) )
    {
      p()->actions.fire_toads_hit->execute();
    }
    else
    {
      for ( int i = 0; i < frog_hits; i++ )
      {
        timespan_t frog_delay                     = ( 0.5_s + p()->spell_const.fire_frog_jump_duration * i );
        p()->actions.fire_frogs_hit->travel_delay = frog_delay.total_seconds();
        p()->actions.fire_frogs_hit->execute();
      }
    }
  }
};

struct fire_frogs_t : public ardeos_spell_t
{
  int frogs;
  fire_frogs_t( util::string_view name, ardeos_t* p, util::string_view options_str = {} )
    : ardeos_spell_t( name, p, options_str ), frogs( p->spell_const.fire_frog_frogs )
  {
    id                 = 15;
    name_str_reporting = "Fire Frogs";

    base_execute_time = 0_s;

    cooldown->duration = p->spell_const.fire_frogs_cooldown;
    cooldown->hasted   = false;
    cooldown->charges  = 1;

    if ( p->talents_enabled( ardeos_t::FROG_SQUAD ) )
    {
      frogs += p->talents.frog_squad_extra_frogs;
    }

    if ( !p->actions.fire_frog->stats->parent )
      add_child( p->actions.fire_frog );
  }

  void execute() override
  {
    ardeos_spell_t::execute();

    for ( int i = 0; i < frogs; i++ )
    {
      p()->actions.fire_frog->execute();
    }

    if ( p()->legendary.fire_toad && p()->legendary.fire_toad_on_cast )
    {
      p()->actions.fire_toads_hit->execute();
    }
  }
};

}  // namespace actions

// ==========================================================================
// Rogue Targetdata Definitions
// ==========================================================================

ardeos_td_t::ardeos_td_t( player_t* target, ardeos_t* source )
  : fellowship::fs_player_td_t( target, source ), dots(), debuffs()
{
  dots.engulfing_flames_individual = {};
  dots.crackling_inferno = target->get_dot( "crackling_inferno", source );
  dots.engulfing_flames  = target->get_dot( "engulfing_flames", source );
  dots.fire_ball         = target->get_dot( "fire_ball_dot", source );
  dots.fire_frog         = target->get_dot( "fire_frog_dot", source );
  dots.incinerate        = target->get_dot( "incinerate_dot", source );
  dots.searing_blaze     = target->get_dot( "searing_blaze", source );

  debuffs.agonizing_blaze_stacks = make_buff( *this, "agonizing_blaze_stacks" )
                                       ->set_max_stack( source->talents.agonizing_blaze_maximum_stacks )
                                       ->set_default_value( source->talents.agonizing_blaze_amp_per_stack );
  debuffs.incinerate_stacks = make_buff( *this, "incinerate_stacks" )
                                  ->set_max_stack( 99 )
                                  ->set_default_value( source->spell_const.incinerate_dot_amp_per_stack );
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

  if ( legendary.engulf_stacker )
  {
    ardeos_td_t* tdata = get_target_data( target );
    //int dot_count      = 0;
    //for ( dot_t* d : tdata->dots.engulfing_flames_individual )
    //{
    //  if ( d->is_ticking() )
    //    dot_count++;
    //}
    //sim->print_debug( "{} Engulf Stacker stacks: {}. Count dots {}.", *this,
    //                  tdata->dots.engulfing_flames->current_stack(), dot_count );

    //if ( dot_count != tdata->dots.engulfing_flames->current_stack() )
    //  sim->print_debug( "{} ENGULF COUNT NOT EQUAL.", *this );
    m *= 1.0 + legendary.engulf_stacker_amp * tdata->dots.engulfing_flames->current_stack();
  }

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

// ardeos_t::extend_engulfing_flames ========================================
void ardeos_t::extend_engulfing_flames( player_t* t, timespan_t extension )
{
  if ( !t )
    return;

  dot_t* base_dot = get_target_data( t )->dots.engulfing_flames;

  if ( !base_dot->current_action )
    return;

  timespan_t dot_max = base_dot->current_action->dot_duration;
  base_dot->adjust_duration( extension, dot_max );

  std::vector<dot_t*> target_dots = get_target_data( t )->dots.engulfing_flames_individual;

  for ( dot_t* d : target_dots )
  {
    if ( d && d->is_ticking() && d->current_action )
      d->adjust_duration( extension, d->current_action->composite_dot_duration( d->state ) );
  }
}

// ardeos_t::extend_dot ====================================================
void ardeos_t::extend_dot( dot_t* d, timespan_t extension )
{
  if ( d->is_ticking() && d->current_action )
  {
    d->adjust_duration( extension, d->current_action->composite_dot_duration( d->state ) );
  }
}

// ardeos_t::extend_dots ====================================================
void ardeos_t::extend_dots( player_t* target, timespan_t extension )
{
  ardeos_td_t* tdata = get_target_data( target );
  extend_engulfing_flames( target, extension );

  auto dots = { tdata->dots.crackling_inferno, tdata->dots.fire_ball, tdata->dots.fire_frog, tdata->dots.incinerate,
                tdata->dots.searing_blaze };

  for ( dot_t* d : dots )
  {
    extend_dot( d, extension );
  }
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

  if ( name == "infernal_wave" )
    return new infernal_wave_t( name, this, options_str );
  if ( name == "detonate" )
    return new detonate_t( name, this, options_str );
  if ( name == "wildfire" )
    return new wildfire_t( name, this, options_str );
  if ( name == "incinerate" )
    return new incinerate_t( name, this, options_str );
  if ( name == "searing_blaze" || name == "sb" )
    return new searing_blaze_t( "searing_blaze", this, options_str );
  if ( name == "engulfing_flames" || name == "ef" )
    return new engulfing_flames_t( this, options_str );
  if ( name == "apocalypse" )
    return new apocalypse_t( name, this, options_str );
  if ( name == "fire_ball" || name == "fireball" )
    return new fire_ball_t( "fire_ball", this, options_str );
  if ( name == "pyromania" || name == "pyro" )
    return new pyromania_t( "pyromania", this, options_str );
  if ( name == "frogs" || name == "fire_frog" || name == "fire_frogs" )
    return new fire_frogs_t( "fire_frogs", this, options_str );

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

  if ( split.size() == 2 && split[ 0 ] == "active_dot" && split[ 1 ] == "engulfing_flames" )
  {
    return make_fn_expr( name_str,
                         [ this ] { return get_active_dots( get_target_data( target )->dots.engulfing_flames ); } );
  }

  if ( split[ 0 ] == "cinder" || split[ 0 ] == "cinders" )
  {
    if ( split.size() == 1 )
    {
      return make_fn_expr( name_str, [ this ] { return this->current_cinders( true ); } );
    }

    if ( split.size() == 2 && split[ 1 ] == "deficit" )
    {
      return make_fn_expr( name_str,
                           [ this ] { return resources.max[ RESOURCE_CINDERS ] - this->current_cinders( true ); } );
    }
  }
  else if ( util::str_compare_ci( split[ 0 ], "talent" ) )
  {
    if ( split.size() == 2 )
    {
      for ( ardeos_talents_t t = static_cast<ardeos_talents_t>( 1U ); t < ardeos_talents_t::MAX; t++ )
      {
        if ( util::str_compare_ci( split[ 1 ], talent_name( t ) ) )
        {
          return make_fn_expr( name_str, std::bind( std::mem_fn( &ardeos_t::talents_enabled ), this, t ) );
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
      if ( util::str_compare_ci( split[ 1 ], "fire_toad" ) )
        return make_ref_expr( name_str, legendary.fire_toad );
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

  base.health_per_stamina = 48.12;

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

    for ( ardeos_talents_t t = static_cast<ardeos_talents_t>( 1U ); t < ardeos_talents_t::MAX; t++ )
    {
      if ( util::str_compare_ci( talent_split[ 0 ], talent_name( t ) ) )
      {
        set_talent_points( t, ranks >= 1 );
        break;
      }
    }
  }
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

  rppm.reign_of_fire = get_rppm( "reign_of_fire", talents.reign_of_fire_ppm, 1.0, RPPM_HASTE );
}

// ardeos_t::init_scaling ====================================================

void ardeos_t::init_scaling()
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

// ardeos_t::init_resources =================================================

void ardeos_t::init_resources( bool force )
{
  fs_player_t::init_resources( force );
}

// ardeos_t::init_buffs ======================================================

void ardeos_t::create_buffs()
{
  fs_player_t::create_buffs();

  buffs.reign_of_fire = make_buff<ardeos_buff_t>( this, "reign_of_fire" )
                            ->set_duration( talents.reign_of_fire_duration )
                            ->set_default_value( talents.reign_of_fire_crit_chance )
                            ->set_refresh_behavior( buff_refresh_behavior::DURATION );

  buffs.wildfire = make_buff<ardeos_buff_t>( this, "wildfire" )
                       ->set_duration( spell_const.wildfire_duration )
                       ->set_default_value( spell_const.wildfire_tickrate )
                       ->set_refresh_behavior( buff_refresh_behavior::DURATION )
                       ->add_invalidate( CACHE_HASTE );

  buffs.untamed_flame_crit = make_buff<ardeos_buff_t>( this, "untamed_flame_crit" )
                                 ->set_default_value( legendary.untamed_flame_crit_chance )
                                 ->set_pct_buff_type( STAT_PCT_BUFF_CRIT );

  buffs.untamed_flame_spirit = make_buff<ardeos_buff_t>( this, "untamed_flame_spirit" )
                                   ->set_default_value( legendary.untamed_flame_crit_chance )
                                   ->set_pct_buff_type( STAT_PCT_BUFF_MASTERY );

  if ( legendary.untamed_flame )
  {
    buffs.wildfire->add_stack_change_callback( [ this ]( buff_t*, int, int _new ) {
      if ( _new )
      {
        buffs.untamed_flame_crit->trigger();
        buffs.untamed_flame_spirit->trigger();
      }
      else
      {
        buffs.untamed_flame_crit->expire();
        buffs.untamed_flame_spirit->expire();
      }
    } );
  }
}

// ardeos_t::invalidate_cache =========================================

void ardeos_t::invalidate_cache( cache_e c )
{
  fs_player_t::invalidate_cache( c );
}

void ardeos_t::create_options()
{
  fs_player_t::create_options();

  add_option( opt_bool( "talent.rolling_flames_instant", talents.rolling_flames_instant ) );
  add_option( opt_timespan( "talent.rolling_flames_cdr", talents.rolling_flames_cdr ) );
  add_option( opt_bool( "talent.double_detonate_cost_efficiency", talents.double_detonate_cost_efficiency ) );

  add_option( opt_float( "talent.reign_of_fire_ppm", talents.reign_of_fire_ppm ) );
  add_option( opt_int( "ardeos.engulfing_flames_charges", spell_const.engulfing_flames_charges ) );
  add_option( opt_timespan( "ardeos.engulfing_flames_cooldown", spell_const.engufling_flames_cooldown ) );
  add_option( opt_timespan( "talent.rolling_flames_infernal_wave_cdr", talents.rolling_flames_infernal_wave_cdr ) );

  add_option( opt_timespan( "talent.detonate_extends_searing", talents.detonate_extends_searing ) );

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

  add_option( opt_bool( "legendary.brimstone_cataclysm", legendary.brimstone_cataclysm ) );
  add_option( opt_bool( "legendary.fire_toad", legendary.fire_toad ) );
  add_option( opt_bool( "legendary.untamed_flame", legendary.untamed_flame ) );
  add_option( opt_bool( "legendary.engulf_stacker", legendary.engulf_stacker ) );
  add_option( opt_float( "legendary.engulf_stacker_amp", legendary.engulf_stacker_amp ) );

  add_option( opt_float( "legendary.fire_toad_chance", legendary.fire_toad_chance ) );
  add_option( opt_float( "legendary.fire_toad_mul", legendary.fire_toad_mul ) );
  add_option( opt_bool( "legendary.fire_toad_on_cast", legendary.fire_toad_on_cast ) );
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

  if ( talents_enabled( PYROPHIBIAN_FRENZY ) )
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
  }
}

// ardeos_t::init_finished ===================================================

void ardeos_t::init_finished()
{
  fs_player_t::init_finished();
}

void ardeos_t::init_background_actions()
{
  fs_player_t::init_background_actions();

  actions.searing_blaze    = new actions::searing_blaze_t( "searing_blaze", this );
  actions.engulfing_flames = new actions::engulfing_flames_t( this );
  actions.engulfing_flames->background = true;
  actions.engulfing_flames->cooldown   = get_cooldown( "engulfing_flames_pyro" );

  actions.fire_frogs_hit    = new actions::fire_frog_hit_t( this, false );
  actions.fire_toads_hit    = new actions::fire_frog_hit_t( this, true );
  actions.fire_frog         = new actions::fire_frog_t( this );
  actions.crackling_inferno = new actions::crackling_inferno_t( this );
  actions.flare_up          = new actions::flare_up_t( this );
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

  resources.current[ RESOURCE_CINDERS ] = 0;
}

// ardeos_t::combat_begin ====================================================

void ardeos_t::combat_begin()
{
  fs_player_t::combat_begin();
}

double ardeos_t::resource_gain( resource_e resource_type, double amount, gain_t* source, action_t* action )
{
  double actual_amount = fs_player_t::resource_gain( resource_type, amount, source, action );

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
void actions::ardeos_action_t<Base>::trigger_spirit_refund( const action_state_t* state, double cinders_refunded )
{
  make_event( ab::sim, 200_ms, [ cinders_refunded, this ] {
    p()->resource_gain( RESOURCE_CINDERS, cinders_refunded, p()->gains.spirit_procs, this );
    p()->sim->print_debug( "{} actually refunded {:.0f} Cinders", *p(), cinders_refunded );
  } );

  p()->spirit_refund();
}

template <typename Base>
void actions::ardeos_action_t<Base>::spend_cinders( const action_state_t* s )
{
  double orbs_spent = s->action->base_costs[ RESOURCE_CINDERS ];
  if ( orbs_spent <= 0 )
    return;

  if ( p()->rng().roll( p()->cache.mastery_value() ) )
  {
    p()->sim->print_debug( "{} proc'd Spirit Orb Refund (Chance: {:.2f}%, Sprit: {:.2f}%)", *p(),
                           p()->cache.mastery_value() * 100.0, p()->cache.mastery() * 100.0 );

    trigger_spirit_refund( s, orbs_spent );
  }
}

template <typename Base>
void actions::ardeos_action_t<Base>::gain_cinders( int gain )
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

void ardeos_t::analyze( sim_t& sim )
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

void ardeos_t::create_cooldowns()
{
  cooldowns.apocalypse       = get_cooldown( "apocalypse" );
  cooldowns.engulfing_flames = get_cooldown( "engulfing_flames" );
  cooldowns.fire_ball        = get_cooldown( "fire_ball" );
  cooldowns.pyromania        = get_cooldown( "pyromania" );
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