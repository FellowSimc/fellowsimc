#include "util/util.hpp"

#include "fs_player.hpp"
#include "simulationcraft.hpp"

namespace fellowship
{
namespace mara
{

// Forward Declarations
class mara_t;

constexpr int COMBO_POINT_MAX = 6;

enum class secondary_trigger
{
  NONE = 0U,
  ULTIMATE_CLONE,
  TALENT_CLONE,
  LEGENDARY_CLONE,
};

namespace actions
{
struct mara_attack_t;
struct mara_heal_t;
struct mara_spell_t;

struct mara_poison_t;

struct melee_t;
}  // namespace actions

class mara_td_t : public fs_player_td_t
{
  std::vector<dot_t*> bleeds;
  std::vector<dot_t*> poison_dots;

public:
  struct dots_t
  {
    dot_t* hemorrhaging_strike;
    dot_t* seething_poison;
    dot_t* volatile_poison;
    dot_t* hemotoxin;
  } dots;

  struct
  {
    buff_t* hemotoxin;
    buff_t* nightstalker;
    buff_t* puncture;
  } debuffs;

  mara_td_t( player_t* target, mara_t* source );

  int total_bleeds() const
  {
    // TOCHECK -- Confirm all these things count as intended
    return as<int>( range::count_if( bleeds, []( dot_t* dot ) { return dot->is_ticking(); } ) );
  }

  int total_poisons() const
  {
    // TOCHECK -- Confirm all these things count as intended
    return as<int>( range::count_if( poison_dots, []( dot_t* d ) { return d->is_ticking(); } ) );
  }
};

struct mara_buff_t : public fs_player_buff_t
{
  mara_buff_t( player_t* p, util::string_view name )
    : fs_player_buff_t( p, name )
  {
  }

  mara_t* p()
  {
    return debug_cast<mara_t*>( player );
  }

  const mara_t* p() const
  {
    return debug_cast<const mara_t*>( player );
  }
};

class mara_t : public fellowship::fs_player_t
{
public:
  // Ready trigger energy threshold
  double mara_ready_trigger_threshold;
  double deadly_energy_tracker;
  player_t* seething_poison_target;

  struct actions_t
  {
    action_t* auto_attack;
    actions::melee_t* melee_hit;
    actions::mara_attack_t* backstab;
    actions::mara_poison_t* caustic_poison;
    actions::mara_attack_t* queens_fang;
    actions::mara_attack_t* queens_fang_ult_clone;
    actions::mara_attack_t* queens_fang_fts_clone;
    actions::mara_attack_t* queens_fang_lego_clone;
    actions::mara_attack_t* skittering_blades;
    actions::mara_attack_t* arachnid_assault;
    actions::mara_attack_t* arachnid_assault_clone;
    actions::mara_attack_t* arachnid_assault_lego_clone;
    actions::mara_poison_t* volatile_poison_dot;
    actions::mara_poison_t* volatile_poison_aoe;
    actions::mara_spell_t* brooding_shadows;
    actions::mara_attack_t* widows_bite;
    actions::mara_poison_t* seething_poison;
    actions::mara_attack_t* hemorrhaging_strike;
    actions::mara_spell_t* maiden_of_death;
    actions::mara_spell_t* final_stratagem;         // 3m reset cd
    actions::mara_spell_t* matriach_macabre;  // ult
    actions::mara_poison_t* hemotoxin_dot;
    actions::mara_poison_t* hemotoxin;
    actions::mara_poison_t* corrosive_spill;
    actions::mara_spell_t* stalker_step;
    actions::mara_poison_t* vexiras_venom;
  } actions;

  struct buffs_t
  {
    mara_buff_t* ultimate_buff_window;
    mara_buff_t* brooding_shadows;
    mara_buff_t* predators_rush;
    mara_buff_t* red_ledger;
    mara_buff_t* red_ledger_additional;
    mara_buff_t* deadly_scheme;
    mara_buff_t* feed_the_queen;
    mara_buff_t* maiden_of_death;
    mara_buff_t* assassins_guile;
    mara_buff_t* drenched_in_blood;
    mara_buff_t* malevolence_qf_buffs_aa;
    mara_buff_t* malevolence_aa_buffs_qf;
    mara_buff_t* spirit_proc_clones;

  } buffs;

  struct cooldowns_t
  {
    cooldown_t* brooding_shadows;
    cooldown_t* widows_bite;
    cooldown_t* stalker_step;
    cooldown_t* maiden_of_death;
    cooldown_t* final_stratagem;
  } cooldowns;

  struct gains_t
  {
    gain_t* spirit_procs;
    gain_t* venomous_delight;
    gain_t* efficient_killer;
  } gains;

  struct spell_const_t
  {
    double hemorrhaging_strike_energy_gen = 5.0;
    double hemorrhaging_strike_damage     = 2.73;
    double hemorrhaging_stike_tick_dmg    = 1.076;
    timespan_t hemorrhaging_strike_period = 3_s;
  } spell_const;

#define MARA_TALENT_LIST( X )                                         \
  X( RED_LEDGER, "red_ledger", "Red Ledger" )                         \
  X( CORROSIVE_SPILL, "corrosive_spill", "Corrosive Spill" )          \
  X( ASSASSINS_GUILE, "assassins_guile", "Assassins Guile" )          \
  X( BLOODRUSH, "bloodrush", "Bloodrush" )                            \
  X( VENOMOUS_DELIGHT, "venomous_delight", "Venomous Delight" )       \
  X( EFFICIENT_KILLER, "efficient_killer", "Efficient Killer" )       \
  X( GUSHING_BLOOD, "gushing_blood", "Gushing Blood" )                \
  X( FEED_THE_QUEEN, "feed_the_queen", "Feed the Queen" )             \
  X( DEADLY_SCHEME, "deadly_scheme", "Deadly Scheme" )                \
  X( VEIL_OF_SHADOWS, "veil_of_shadows", "Veil of Shadows" )          \
  X( MAIDENS_DOOM, "maidens_doom", "Maidens Doom" )                   \
  X( MAGIC_WARD, "magic_ward", "Magic Ward" )                         \
  X( HEMOTOXIN, "hemotoxin", "Hemotoxin" )                            \
  X( SINNERS_PRIDE, "sinners_pride", "Sinners Pride" )                \
  X( MALEVOLENCE, "malevolence", "Malevolence" )                      \
  X( ARACHNID_ONSLAUGHT, "arachnid_onslaught", "Arachnid Onslaught" ) \
  X( SPIRITED_FORTITUDE, "spirited_fortitude", "Spirited Fortitude" ) \
  X( PUNCTURE, "puncture", "Puncture" )

  enum mara_talents_t : unsigned long long
  {
    NONE = 0,
#define X( name, id, pretty ) name = 1ULL << __COUNTER__,
    MARA_TALENT_LIST( X )
#undef X
        MAX = 1ULL << __COUNTER__
  };

  struct talent_info
  {
    mara_talents_t flag;
    std::string_view id;
    std::string_view pretty;
  };

  static constexpr talent_info MARA_TALENTS[] = {
#define X( name, id, pretty ) { mara_talents_t::name, id, pretty },
      MARA_TALENT_LIST( X )
#undef X
  };

  static constexpr std::string_view talent_name( mara_talents_t t )
  {
    for ( const auto& talent : MARA_TALENTS )
      if ( talent.flag == t )
        return talent.id;

    return "unknown_talent";
  }

  static constexpr std::string_view talent_name_formatted( mara_talents_t t )
  {
    for ( const auto& talent : MARA_TALENTS )
      if ( talent.flag == t )
        return talent.pretty;

    return "Unknown Talent";
  }

  struct talents_t
  {
    double red_ledger_base      = 0.1;
    double red_ledger_per_stack = 0.02;
    int red_ledger_max          = 5;

    double malevolence_amplifier    = 1;
    int malevolence_max_stacks      = 2;
    timespan_t malevolence_duration = 20_s;

    double deadly_scheme_added_crit      = 1.0;
    double deadly_scheme_required_energy = 200;

    double bloodrush_tickrate = 1.3;
    double bloodrush_damage   = 1.5;

    double venomous_delight_chance = 0.1;
    double venomous_delight_energy = 10;

    double vile_venoms_poison_multiplier = 1.15;

    double efficient_killer_energy_cost_modifier = 0.9;
    double efficient_killer_max_energy_boost     = 1.1;
    double efficient_killer_energy_per_cp        = 1.0;

    int gushing_blood_hemorrhaging_additional_targets = 4;
    bool gushing_blood_always_works = true;

    double corrosive_spill_chance_per_cp                      = 0.03;
    timespan_t corrosive_spill_duration                       = 3_s;
    double corrosive_spill_damage                             = 0.9;
    timespan_t corrosive_spill_ticktime                       = 1.5_s;
    double corrosive_spill_cumulative_chance_per_tick_of_miss = 0.25;

    int feed_the_queen_max_stacks         = 6;
    double feed_the_queen_bonus_per_stack = 0.1;

    double nightstalker_damage_taken = 0.1;
    timespan_t nightstalker_duration = 10_s;

    bool magic_ward = false;

    timespan_t sinners_pride_cdr_reduction_per_cp = 0.6_s;

    double hemotoxin_chance               = 0.12;
    int hemotoxin_applied_stacks          = 1;
    int hemotoxin_max_stacks              = 3;
    double hemotoxin_damage_coefficient   = 0.2;
    timespan_t hemotoxin_duration         = 9_s;
    timespan_t hemotoxin_period           = 1.5_s;
    double hemotoxin_conversion_rate      = 0.50;
    double hemotoxin_aoe_damage_reduction = 0.8;
    double hemotoxin_aoe_falloff          = 1.0;
    bool hemotoxin_double_dip_stats       = false;
    bool hemotoxin_double_dip_multipliers = false;

    double maidens_doom_execute_threshold = 30.0;
    double maidens_doom_execute_amp       = 0.2;

    double puncture_cc                = 1.0;
    bool puncture_buff                = false;
    double puncture_buff_tickrate     = 0.3;
    timespan_t puncture_buff_duration = 9_s;


    double arachnid_onslaught_multiplier = 1.2;

    double assassins_guile_finisher_boost     = 0.4;
    timespan_t assassins_guile_buff_duration = 4.99_s;
  } talents;

  struct legendary_t
  {
    bool vexiras_venom = false;
    // Consider penalising it by 25%
    double vexiras_venom_accumulate   = 0.25;
    timespan_t vexiras_venom_period   = 2_s;
    timespan_t vexiras_venom_duration = 6_s;

    bool drenched_in_blood = false;
    double drenched_in_blood_exp         = 0.24;
    timespan_t drenched_in_blood_duration = 8_s;

    bool from_the_shadows              = false;
    double from_the_shadows_chance     = 0.15;
    int from_the_shadows_combo_points  = 6;
    double from_the_shadows_bleed_rate = 0.15;

    bool spirit_procs_clones                = false;
    bool spirit_procs_clones_proc_on_next   = true;
    int spirit_procs_clones_clones          = 2;
    timespan_t spirit_procs_clones_duration = 15_s;
  } legendary;

  struct options_t
  {
    double widows_bite_extra_energy = 0;
  } options;

  double current_cp( bool /* react */ = false ) const
  {
    return resources.current[ RESOURCE_COMBO_POINT ];
  }

  target_specific_t<mara_td_t> target_data;

  const mara_td_t* find_target_data( const player_t* target ) const override
  {
    return target_data[ target ];
  }

  mara_td_t* get_target_data( player_t* target ) const override
  {
    mara_td_t*& td = target_data[ target ];
    if ( !td )
    {
      td = new mara_td_t( target, const_cast<mara_t*>( this ) );
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
  timespan_t available() const override;
  action_t* create_action( util::string_view name, util::string_view options ) override;

  std::unique_ptr<expr_t> create_action_expression( action_t& action, std::string_view name_str ) override;
  std::unique_ptr<expr_t> create_expression( util::string_view name_str ) override;
  std::unique_ptr<expr_t> create_resource_expression( util::string_view name ) override;

  void regen( timespan_t periodicity ) override;
  resource_e primary_resource() const override
  {
    return RESOURCE_ENERGY;
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


  void break_stealth();
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

  double consume_cp_max() const
  {
    return COMBO_POINT_MAX;
  }

  mara_t( sim_t* sim, util::string_view name, race_e r = RACE_NONE )
    : fs_player_t( sim, name, r, MARA ),
      deadly_energy_tracker( 0.0 ),
      seething_poison_target( nullptr ),
      target_data(),
      mara_ready_trigger_threshold( 20 )
  {
    resource_regeneration              = regen_type::DYNAMIC;
    regen_caches[ CACHE_HASTE ]        = true;
    regen_caches[ CACHE_ATTACK_HASTE ] = true;

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
  int cp;

  secondary_action_trigger_t( action_state_t* s, timespan_t delay = timespan_t::zero() )
    : event_t( *s->action->sim, delay ),
      action( dynamic_cast<Base*>( s->action ) ),
      state( s ),
      target( nullptr ),
      cp( 0 )
  {
  }

  secondary_action_trigger_t( player_t* target, Base* action, int cp, timespan_t delay = timespan_t::zero() )
    : event_t( *action->sim, delay ), action( action ), state( nullptr ), target( target ), cp( cp )
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
      action->cast_state( state )->set_combo_points( cp, cp );
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

struct mara_action_state_t : public action_state_t
{
private:
  int base_cp;
  int total_cp;

public:
  mara_action_state_t( action_t* action, player_t* target )
    : action_state_t( action, target ), base_cp( 0 ), total_cp( 0 )
  {
  }

  void initialize() override
  {
    action_state_t::initialize();
    base_cp  = 0;
    total_cp = 0;
  }

  std::ostringstream& debug_str( std::ostringstream& s ) override
  {
    action_state_t::debug_str( s ) << " base_cp=" << base_cp << " total_cp=" << total_cp;
    return s;
  }

  void copy_state( const action_state_t* s )
  {
    action_state_t::copy_state( s );
    const mara_action_state_t* rs = debug_cast<const mara_action_state_t*>( s );
    base_cp                       = rs->base_cp;
    total_cp                      = rs->total_cp;
  }

  void set_combo_points( int base_cp, int total_cp )
  {
    this->base_cp  = base_cp;
    this->total_cp = total_cp;
  }

  int get_combo_points( bool base_only = false ) const
  {
    if ( base_only )
      return base_cp;

    return total_cp;
  }
};

template <typename Base>
class mara_action_t : public Base
{
protected:
  /// typedef for mara_action_t<action_base_t>
  using base_t = mara_action_t<Base>;

private:
  /// typedef for the templated action type, eg. spell_t, attack_t, heal_t
  using ab = Base;

public:
  bool _requires_stealth;
  bool _breaks_stealth;
  // Secondary triggered ability, due to Weaponmaster talent or Death from Above. Secondary
  // triggered abilities cost no resources or incur cooldowns.
  secondary_trigger secondary_trigger_type;

  double combo_point_multiplier = 0.2;

  // Init =====================================================================

  mara_action_t( util::string_view n, mara_t* p, util::string_view options = {} )
    : ab( n, p, options ),
      _requires_stealth( false ),
      _breaks_stealth( true ),
      secondary_trigger_type( secondary_trigger::NONE )
  {
    ab::parse_options( options );
    ab::may_crit = ab::tick_may_crit = true;
    ab::school                       = SCHOOL_PHYSICAL;

    // mara_t sets base and min GCD to 1s by default but let's also enforce non-hasted GCDs.
    // Even for rogue abilities that can be considered spells, hasted GCDs seem to be an exception rather than rule.
    // Those should be set explicitly. (see Vendetta, Shadow Blades, Detection)
    ab::gcd_type = gcd_haste_type::NONE;
  }

  void init() override
  {
    ab::init();
  }

  // Type Wrappers ============================================================

  static const mara_action_state_t* cast_state( const action_state_t* st )
  {
    return debug_cast<const mara_action_state_t*>( st );
  }

  static mara_action_state_t* cast_state( action_state_t* st )
  {
    return debug_cast<mara_action_state_t*>( st );
  }

  mara_t* p()
  {
    return debug_cast<mara_t*>( ab::player );
  }

  const mara_t* p() const
  {
    return debug_cast<const mara_t*>( ab::player );
  }

  mara_td_t* td( player_t* t ) const
  {
    return p()->get_target_data( t );
  }

  // Action State =============================================================

  action_state_t* new_state() override
  {
    return new mara_action_state_t( this, ab::target );
  }

  void update_state( action_state_t* state, unsigned flags, result_amount_type rt ) override
  {
    ab::update_state( state, flags, rt );
  }

  void snapshot_state( action_state_t* state, result_amount_type rt ) override
  {
    int consume_cp   = as<int>( std::min( p()->current_cp(), p()->consume_cp_max() ) );
    int effective_cp = consume_cp;

    auto rs = cast_state( state );
    rs->set_combo_points( consume_cp, effective_cp );

    ab::snapshot_state( state, rt );
  }

  // Secondary Trigger Functions ==============================================

  bool is_secondary_action() const
  {
    return secondary_trigger_type != secondary_trigger::NONE;
  }

  virtual void trigger_secondary_action( player_t* target, int cp = 0, timespan_t delay = timespan_t::zero() )
  {
    assert( is_secondary_action() );
    make_event<secondary_action_trigger_t<base_t>>( *ab::sim, target, this, cp, delay );
  }

  virtual void trigger_secondary_action( player_t* target, timespan_t delay )
  {
    trigger_secondary_action( target, 0, delay );
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
  virtual double generate_cp() const
  {
    double cp = 0;

    if ( ab::energize_type != action_energize::NONE && ab::energize_resource == RESOURCE_COMBO_POINT )
    {
      cp += ab::energize_amount;
    }

    return cp;
  }

  // Overridable wrapper for checking stealth requirement
  virtual bool requires_stealth() const
  {
    return _requires_stealth;
  }

  // Overridable wrapper for checking stealth breaking
  virtual bool breaks_stealth() const
  {
    return _breaks_stealth;
  }

  double parry_chance( double exp, player_t* target ) const override
  {
    return 0.0;
  }

public:
  // Ability triggers
  void spend_combo_points( const action_state_t* );
  void trigger_auto_attack( const action_state_t* );
  void trigger_spirit_refund( const action_state_t*, double );
  void trigger_poison_bomb( const action_state_t*, double );
  void roll_for_hemotoxin( const action_state_t* );
  void handle_vexiras_venom( const action_state_t* );
  void trigger_combo_point_gain( int, gain_t* gain = nullptr );

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

  virtual double combo_point_da_multiplier( const action_state_t* s ) const
  {
    if ( ab::base_costs[ RESOURCE_COMBO_POINT ] )
      return 1.0 + cast_state( s )->get_combo_points() * combo_point_multiplier;

    return 1.0;
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

    m *= 1 + tdata->debuffs.nightstalker->check_stack_value();

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

    spend_combo_points( ab::execute_state );

    if ( ab::current_resource() == RESOURCE_ENERGY && ab::last_resource_cost > 0 )
    {
      if ( !ab::hit_any_target )
      {
        // trigger_spirit_refund( ab::execute_state );
      }
      else
      {
        // Energy Spend Mechanics
      }
    }
  }

  void execute() override
  {
    ab::execute();

    if ( ab::hit_any_target && !ab::background )
    {
      trigger_auto_attack( ab::execute_state );
    }

    // Trigger the 1ms delayed breaking of all stealth buffs
    if ( p()->buffs.brooding_shadows->check() && breaks_stealth() )
    {
      p()->break_stealth();
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

    if ( ab::base_costs[ RESOURCE_COMBO_POINT ] > 0 && p()->current_cp() < ab::base_costs[ RESOURCE_COMBO_POINT ] )
      return false;

    return true;
  }

  std::unique_ptr<expr_t> create_expression( std::string_view name ) override
  {
    if ( util::str_compare_ci( name, "cp_gain" ) )
    {
      return make_mem_fn_expr( "cp_gain", *this, &base_t::generate_cp );
    }

    return ab::create_expression( name );
  }
};

// ==========================================================================
// Rogue Attack Classes
// ==========================================================================

struct mara_heal_t : public mara_action_t<fellowship::actions::fs_player_action_t<heal_t>>
{
  mara_heal_t( util::string_view n, mara_t* p, util::string_view o = {} ) : base_t( n, p, o )
  {
    harmful = false;
    set_target( p );
  }

  bool breaks_stealth() const override
  {
    return false;
  }
};

struct mara_spell_t : public mara_action_t<fellowship::actions::fs_player_action_t<spell_t>>
{
  mara_spell_t( util::string_view n, mara_t* p, util::string_view o = {} ) : base_t( n, p, o )
  {
  }
};

struct mara_attack_t : public mara_action_t<fellowship::actions::fs_player_action_t<melee_attack_t>>
{
  mara_attack_t( util::string_view n, mara_t* p, util::string_view o = {} ) : base_t( n, p, o )
  {
    special = true;
  }

  void impact( action_state_t* state ) override
  {
    base_t::impact( state );
  }
};

// ==========================================================================
// Poisons
// ==========================================================================

struct mara_poison_t : public mara_attack_t
{
  mara_poison_t( util::string_view name, mara_t* p, bool triggers_procs = false ) : mara_attack_t( name, p )
  {
    background            = true;
    channeled             = false;
    interrupt_auto_attack = false;
    reset_auto_attack     = false;
    proc                  = !triggers_procs;
    callbacks             = triggers_procs;

    school = SCHOOL_MAGIC;

    trigger_gcd = timespan_t::zero();
  }

  timespan_t execute_time() const override
  {
    return timespan_t::zero();
  }

  void assess_damage( result_amount_type rt, action_state_t* state ) override
  {
    mara_attack_t::assess_damage( rt, state );

    if ( p()->talents_enabled( mara_t::VENOMOUS_DELIGHT ) && rt != result_amount_type::NONE &&
         state->result_amount > 0 )
    {
      if ( rng().roll( p()->talents.venomous_delight_chance ) )
      {
        p()->resource_gain( RESOURCE_ENERGY, p()->talents.venomous_delight_energy, p()->gains.venomous_delight, this );
      }
    }
  }
};

struct melee_t : public mara_attack_t
{
  bool first;
  bool canceled;
  timespan_t prev_scheduled_time;

  melee_t( const char* name, const char* reporting_name, mara_t* p )
    : mara_attack_t( name, p ), first( true ), canceled( false ), prev_scheduled_time( timespan_t::zero() )
  {
    background = repeating = may_glance = may_crit = true;
    may_miss                                       = true;
    allow_class_ability_procs = not_a_proc = true;
    special                                = false;

    school             = SCHOOL_PHYSICAL;
    trigger_gcd        = timespan_t::zero();
    name_str_reporting = reporting_name;

    attack_power_mod.direct = 0.3;
    _breaks_stealth         = false;
  }

  double miss_chance( double /* hit */, player_t* /* target */ ) const
  {
    return 1-0.95*0.95;
  }

  void reset() override
  {
    mara_attack_t::reset();
    first               = true;
    canceled            = false;
    prev_scheduled_time = timespan_t::zero();
  }

  timespan_t execute_time() const override
  {
    timespan_t t = mara_attack_t::execute_time();

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

  void impact( action_state_t* s ) override
  {
    base_t::impact( s );

    roll_for_hemotoxin( s );
  }

  void schedule_execute( action_state_t* state ) override
  {
    mara_attack_t::schedule_execute( state );

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
  auto_melee_attack_t( mara_t* p, util::string_view options_str ) : action_t( ACTION_OTHER, "auto_attack", p )
  {
    trigger_gcd        = timespan_t::zero();
    name_str_reporting = "Auto Attack";

    background = true;

    p->actions.melee_hit = debug_cast<melee_t*>( p->find_action( "auto_attack_damage" ) );
    if ( !p->actions.melee_hit )
      p->actions.melee_hit = new melee_t( "auto_attack_damage", "Auto Attack", p );

    p->main_hand_attack                    = p->actions.melee_hit;
    p->main_hand_attack->base_execute_time = 1.5_s;
    p->main_hand_attack->id                = 1;

    id = 1;

    school = SCHOOL_PHYSICAL;

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

struct backstab_t : public mara_attack_t
{
  backstab_t( util::string_view name, mara_t* p, util::string_view options_str = {} )
    : mara_attack_t( name, p, options_str )
  {
    id = 2;

    school                        = SCHOOL_PHYSICAL;
    attack_power_mod.direct       = 0.996;
    resource_current              = RESOURCE_ENERGY;
    base_costs[ RESOURCE_ENERGY ] = 20;

    name_str_reporting = "Backstab";
  }

  void execute() override
  {
    mara_attack_t::execute();

    if ( p()->buffs.brooding_shadows->check() )
    {
      p()->actions.caustic_poison->set_target( target );
      p()->actions.caustic_poison->execute();
    }

    if ( p()->buffs.maiden_of_death->check() )
    {
      trigger_combo_point_gain( COMBO_POINT_MAX, p()->actions.maiden_of_death->gain );
    }
  }

  double composite_da_multiplier( const action_state_t* state ) const override
  {
    double m = mara_attack_t::composite_da_multiplier( state );

    if ( p()->position() == POSITION_BACK )
    {
      m *= 1.4;
    }

    return m;
  }

  void impact( action_state_t* state ) override
  {
    mara_attack_t::impact( state );

    trigger_combo_point_gain( state->result == RESULT_CRIT ? 3 : 2, gain );

    roll_for_hemotoxin( state );
  }
};

struct queens_fang_t : public mara_attack_t
{
  queens_fang_t( util::string_view name, mara_t* p, util::string_view options_str = {},
                 secondary_trigger st = secondary_trigger::NONE )
    : mara_attack_t( name, p, options_str )
  {
    id = 3;

    secondary_trigger_type = st;

    name_str_reporting = "Queens Fang";

    school                             = SCHOOL_PHYSICAL;
    attack_power_mod.direct            = 2.292 * 1.1;
    resource_current                   = RESOURCE_ENERGY;
    base_costs[ RESOURCE_COMBO_POINT ] = 1;
    base_costs[ RESOURCE_ENERGY ]      = 40;

    /*if ( p->talents.efficient_killer )
      base_costs[ RESOURCE_ENERGY ] *= p->talents.efficient_killer_energy_cost_modifier;*/
        
    if ( st == secondary_trigger::NONE && p->legendary.spirit_procs_clones )
    {
      add_child( p->actions.queens_fang_lego_clone );
    }

    if ( st == secondary_trigger::ULTIMATE_CLONE )
    {
      base_dd_multiplier *= 0.5;
    }
  }

  double composite_da_multiplier( const action_state_t* state ) const override
  {
    double m = mara_attack_t::composite_da_multiplier( state );

    m *= combo_point_da_multiplier( state );

    m *= 1 + p()->buffs.assassins_guile->check_value();

    if ( secondary_trigger_type != secondary_trigger::TALENT_CLONE )
    {
      m *= 1 + p()->buffs.malevolence_aa_buffs_qf->check_value();
      m *= 1 + p()->buffs.feed_the_queen->check_stack_value();
    }

    return m;
  }

  double composite_crit_chance() const override
  {
    return mara_attack_t::composite_crit_chance() + p()->buffs.deadly_scheme->check_value();
  }

  void execute() override
  {
    bool had_clones = false;
    if ( !is_secondary_action() )
    {
      had_clones = p()->buffs.spirit_proc_clones->check();
      p()->buffs.spirit_proc_clones->decrement();
    }
    mara_attack_t::execute();

    if ( secondary_trigger_type == secondary_trigger::NONE && had_clones )
    {
      p()->buffs.spirit_proc_clones->expire();
      for ( int i = 0; i <= p()->legendary.spirit_procs_clones_clones; i++ )
      {
        p()->actions.queens_fang_lego_clone->trigger_secondary_action(
            p()->actions.queens_fang_lego_clone->get_state( execute_state ),
            0.6_s * as<double>( i ) / p()->legendary.spirit_procs_clones_clones );
      }
    }

    if ( !is_secondary_action() && p()->talents_enabled( mara_t::MALEVOLENCE ) )
    {
      p()->buffs.malevolence_qf_buffs_aa->trigger();
      p()->buffs.malevolence_aa_buffs_qf->decrement();
    }
  }

  void impact( action_state_t* state ) override
  {
    mara_attack_t::impact( state );

    if ( secondary_trigger_type == secondary_trigger::NONE && p()->buffs.ultimate_buff_window->check() )
    {
      p()->actions.queens_fang_ult_clone->trigger_secondary_action(
          p()->actions.queens_fang_ult_clone->get_state( state ), 0.3_s );
      p()->actions.queens_fang_ult_clone->trigger_secondary_action(
          p()->actions.queens_fang_ult_clone->get_state( state ), 0.6_s );
    }

    if ( !is_secondary_action() )
    {
      p()->buffs.deadly_scheme->expire();
      p()->buffs.feed_the_queen->expire();
    }

    handle_vexiras_venom( state );
  }
};

struct hemorrhaging_strike_t : public mara_attack_t
{
  double energy_gain_per_tick;
  hemorrhaging_strike_t( util::string_view name, mara_t* p, util::string_view options_str = {} )
    : mara_attack_t( name, p, options_str ), energy_gain_per_tick( p->spell_const.hemorrhaging_strike_energy_gen )
  {
    dot_duration   = 15_s;
    dot_behavior   = DOT_REFRESH_PANDEMIC;
    base_tick_time = 3_s;
    hasted_ticks   = true;

    name_str_reporting = "Hemorrhaging Strike";

    attack_power_mod.tick              = p->spell_const.hemorrhaging_stike_tick_dmg;
    attack_power_mod.direct            = p->spell_const.hemorrhaging_strike_damage;
    resource_current                   = RESOURCE_ENERGY;
    base_costs[ RESOURCE_COMBO_POINT ] = 1;
    base_costs[ RESOURCE_ENERGY ]      = 20;

    /*if ( p->talents_enabled( mara_t::EFFICIENT_KILLER ) )
      base_costs[ RESOURCE_ENERGY ] *= p->talents.efficient_killer_energy_cost_modifier;*/

    if ( p->legendary.from_the_shadows )
      add_child( p->actions.queens_fang_fts_clone );

    if ( p->talents_enabled( mara_t::HEMOTOXIN ) )
      add_child( p->actions.hemotoxin );
  }

  int n_targets() const override
  {
    int n = mara_attack_t::n_targets();

    if ( !is_secondary_action() &&
         ( p()->talents_enabled( mara_t::GUSHING_BLOOD ) &&
           ( p()->buffs.maiden_of_death->check() || p()->talents.gushing_blood_always_works ) ) )
    {
      n = 1 + p()->talents.gushing_blood_hemorrhaging_additional_targets;
    }

    return n;
  }

  size_t available_targets( std::vector<player_t*>& tl ) const override
  {
    mara_attack_t::available_targets( tl );

    if ( is_aoe() && tl.size() > 1 && !is_secondary_action() )
    {
      if ( p()->talents_enabled( mara_t::GUSHING_BLOOD ) )
      {
        // Find Target and move to front
        auto it     = std::find( tl.begin(), tl.end(), this->target );
        auto tmp    = *it;
        *it         = *tl.begin();
        *tl.begin() = tmp;

        // Sort remaining by dot remains
        std::sort( tl.begin() + 1, tl.end(), [ this ]( player_t* a, player_t* b ) {
          return td( a )->dots.hemorrhaging_strike->remains() <= td( b )->dots.hemorrhaging_strike->remains();
        } );
      }
    }

    return tl.size();
  }

  timespan_t composite_dot_duration( const action_state_t* s ) const override
  {
    const auto rs       = cast_state( s );
    timespan_t duration = 15_s * ( 1 + rs->get_combo_points() * combo_point_multiplier );

    return duration;
  }

  timespan_t tick_time( const action_state_t* s ) const override
  {
    timespan_t tt = mara_attack_t::tick_time( s );

    if ( p()->talents_enabled( mara_t::BLOODRUSH ) )
    {
      tt /= p()->talents.bloodrush_tickrate;
    }

    if ( p()->talents_enabled( mara_t::PUNCTURE ) && p()->talents.puncture_buff &&
         td( s->target )->debuffs.puncture->check() )
    {
      tt /= 1 + td( s->target )->debuffs.puncture->check_value();
    }

    if ( p()->legendary.from_the_shadows )
    {
      tt /= 1.0 + p()->legendary.from_the_shadows_bleed_rate;
    }

    return tt;
  }

  double composite_ta_multiplier( const action_state_t* s ) const override
  {
    double tam = mara_attack_t::composite_ta_multiplier( s );

    return tam;
  }

  double composite_da_multiplier( const action_state_t* s ) const override
  {
    if ( s->chain_target > 0 )
      return 0.0;

    double m = mara_attack_t::composite_da_multiplier( s );
    return m;
  }

  void execute() override
  {
    mara_attack_t::execute();
  }

  double hemo_tick_damage_over_time( timespan_t duration, const dot_t* dot ) const
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
    double ticks_left   = duration / dot_tick_time;
    double total_damage = ticks_left * tick_base_damage;
    total_damage /= state->target_ta_multiplier;
    action_state_t::release( state );
    return total_damage;
  }

  void impact( action_state_t* s ) override
  {
    mara_attack_t::impact( s );

    if ( p()->talents_enabled( mara_t::RED_LEDGER ) )
      make_event( *p()->sim, [ this ] { trigger_red_ledger(); } );

    if ( p()->talents_enabled( mara_t::HEMOTOXIN ) && p()->get_target_data( s->target )->debuffs.hemotoxin->check() )
    {
      p()->get_target_data( s->target )->debuffs.hemotoxin->decrement();
      
      double amount = hemo_tick_damage_over_time( composite_dot_duration( s ), get_dot( s->target ) ) * p()->talents.hemotoxin_conversion_rate;
      p()->actions.hemotoxin->execute_on_target( s->target, amount );
    }
  }

  void tick( dot_t* d ) override
  {
    mara_attack_t::tick( d );

    p()->resource_gain( RESOURCE_ENERGY, energy_gain_per_tick * d->get_tick_factor(), gain, this );
    if ( p()->legendary.from_the_shadows && rng().roll( p()->legendary.from_the_shadows_chance ) )
    {
      p()->actions.queens_fang_fts_clone->trigger_secondary_action(
          d->state->target, p()->legendary.from_the_shadows_combo_points, 0.3_s );
    }
  }

  void last_tick( dot_t* d ) override
  {
    mara_attack_t::last_tick( d );

    // Delay to allow the demise to reset() and update get_active_dots() count
    if ( p()->talents_enabled( mara_t::RED_LEDGER ) )
      make_event( *p()->sim, [ this ] { trigger_red_ledger(); } );
  }

  void trigger_red_ledger()
  {
    if ( !p()->talents_enabled( mara_t::RED_LEDGER ) )
      return;

    const int current_stacks = p()->buffs.red_ledger_additional->check();
    int desired_stacks       = 0;
    int dots_up = desired_stacks = p()->get_active_dots( td( this->target )->dots.hemorrhaging_strike );
    desired_stacks               = std::min( p()->talents.red_ledger_max, dots_up - 1 );

    if ( dots_up == 0 )
    {
      p()->buffs.red_ledger_additional->expire();
      p()->buffs.red_ledger->expire();
    }
    else
    {
      p()->buffs.red_ledger->trigger();
    }

    if ( current_stacks != desired_stacks )
    {
      p()->sim->print_log( "{} adjusting Ledger stacks from {} to {}", *p(), current_stacks, desired_stacks );
    }

    if ( desired_stacks < current_stacks )
    {
      p()->buffs.red_ledger_additional->decrement( current_stacks - desired_stacks );
    }
    else if ( desired_stacks > current_stacks )
    {
      p()->buffs.red_ledger_additional->increment( desired_stacks - current_stacks );
    }
  }
};

struct seething_poison_t : public mara_poison_t
{
  seething_poison_t( util::string_view name, mara_t* p ) : mara_poison_t( name, p )
  {
    dot_duration   = 60_s;
    dot_behavior   = DOT_REFRESH_PANDEMIC;
    base_tick_time = 2_s;
    hasted_ticks   = true;

    id = 7;

    name_str_reporting = "Seething Poison";

    attack_power_mod.tick = 1.116;
  }
  void trigger_dot( action_state_t* s ) override
  {
    if ( p()->seething_poison_target && p()->seething_poison_target != s->target )
    {
      p()->get_target_data( p()->seething_poison_target )->dots.seething_poison->reset();
    }
    p()->seething_poison_target = s->target;
    mara_poison_t::trigger_dot( s );
    p()->buffs.predators_rush->trigger();
  }

  void last_tick( dot_t* d ) override
  {
    mara_poison_t::last_tick( d );

    p()->buffs.predators_rush->expire();
  }
};

struct caustic_poison_t : public mara_poison_t
{
  caustic_poison_t( util::string_view name, mara_t* p ) : mara_poison_t( name, p )
  {
    id = 8;

    name_str_reporting = "Caustic Poison";

    attack_power_mod.direct = 4.476;

    base_crit += 1.0;
  }

  void execute() override
  {
    mara_poison_t::execute();

    trigger_combo_point_gain( 4, gain );
  }
};

struct widows_bite_t : public mara_attack_t
{
  struct widows_bite_hit_t : public mara_attack_t
  {
    widows_bite_hit_t( util::string_view name, mara_t* p, double coeff ) : mara_attack_t( name, p )
    {
      attack_power_mod.direct = coeff;

      dual = true;

      id = 4;

      name_str_reporting = "Widows Bite";

      if ( p->talents_enabled( mara_t::PUNCTURE ) )
      {
        base_crit += p->talents.puncture_cc;
      }
    }

    void impact( action_state_t* state ) override
    {
      mara_attack_t::impact( state );

      trigger_combo_point_gain( state->result == RESULT_CRIT ? 3 : 2, this->gain );
      roll_for_hemotoxin( state );

      if ( p()->talents_enabled( mara_t::PUNCTURE ) && p()->talents.puncture_buff && state->result == RESULT_CRIT )
      {
        td( state->target )->debuffs.puncture->trigger();
      }
    }
  };

  action_t* widows_bite_hit_1;
  action_t* widows_bite_hit_2;
  widows_bite_t( util::string_view name, mara_t* p, util::string_view options_str = {} )
    : mara_attack_t( name, p, options_str ), widows_bite_hit_1( nullptr ), widows_bite_hit_2( nullptr )
  {
    id = 4;

    name_str_reporting = "Widows Bite";

    energize_type     = action_energize::ON_HIT;
    energize_amount   = 30 + p->options.widows_bite_extra_energy;
    energize_resource = RESOURCE_ENERGY;

    widows_bite_hit_1       = new widows_bite_hit_t( "widows_bite_hit_1", p, 1.452 );
    widows_bite_hit_1->gain = this->gain;
    widows_bite_hit_2       = new widows_bite_hit_t( "widows_bite_hit_2", p, 1.08 );
    widows_bite_hit_2->gain = this->gain;

    add_child( widows_bite_hit_1 );
    add_child( widows_bite_hit_2 );

    cooldown->duration = 9_s;
    cooldown->hasted   = true;
    cooldown->charges  = 3;
  }

  void execute() override
  {
    mara_attack_t::execute();

    widows_bite_hit_1->set_target( target );
    widows_bite_hit_1->execute();

    widows_bite_hit_2->set_target( target );
    widows_bite_hit_2->execute();

    if ( p()->buffs.brooding_shadows->check() )
    {
      p()->actions.seething_poison->set_target( target );
      p()->actions.seething_poison->execute();
    }

    if ( p()->buffs.maiden_of_death->check() )
    {
      trigger_combo_point_gain( COMBO_POINT_MAX, p()->actions.maiden_of_death->gain );
    }
  }
};

struct brooding_shadows_t : public mara_spell_t
{
  brooding_shadows_t( util::string_view name, mara_t* p, util::string_view options_str = {} )
    : mara_spell_t( name, p, options_str )
  {
    id = 6;

    _breaks_stealth = false;

    name_str_reporting = "Brooding Shadows";

    add_child( p->actions.seething_poison );
    add_child( p->actions.caustic_poison );
    add_child( p->actions.volatile_poison_aoe );
    add_child( p->actions.volatile_poison_dot );

    trigger_gcd = timespan_t::zero();

    cooldown->duration = 15_s;
    cooldown->hasted   = true;
    cooldown->charges  = 2;
  }

  void execute() override
  {
    p()->buffs.brooding_shadows->trigger();
    mara_spell_t::execute();
  }
};

struct maiden_of_death_t : public mara_spell_t
{
  maiden_of_death_t( util::string_view name, mara_t* p, util::string_view options_str = {} )
    : mara_spell_t( name, p, options_str )
  {
    id = 9;

    _breaks_stealth = false;

    name_str_reporting = "Maiden of Death";

    trigger_gcd = timespan_t::zero();

    cooldown->duration = 60_s;
    cooldown->hasted   = false;
    cooldown->charges  = 1;
  }

  void execute() override
  {
    p()->buffs.maiden_of_death->trigger();
    mara_spell_t::execute();
  }
};

struct final_stratagem_t : public mara_spell_t
{
  final_stratagem_t( util::string_view name, mara_t* p, util::string_view options_str = {} )
    : mara_spell_t( name, p, options_str )
  {
    id = 10;
        
    energize_type     = action_energize::ON_CAST;
    energize_amount   = p->resources.base[ RESOURCE_ENERGY ];
    energize_resource = RESOURCE_ENERGY;

    _breaks_stealth = false;

    name_str_reporting = "Final Stratagem";

    trigger_gcd = timespan_t::zero();

    cooldown->duration = 180_s;
    cooldown->hasted   = false;
    cooldown->charges  = 1;
  }

  void execute() override
  {
    mara_spell_t::execute();
    p()->cooldowns.brooding_shadows->reset( false, -1 );
    p()->cooldowns.maiden_of_death->reset( false, -1 );
    p()->cooldowns.stalker_step->reset( false, -1 );
    p()->cooldowns.widows_bite->reset( false, -1 );
    trigger_combo_point_gain( COMBO_POINT_MAX, gain );

  }
};

struct matriach_macabre_t : public mara_spell_t
{
  matriach_macabre_t( util::string_view name, mara_t* p, util::string_view options_str = {} )
    : mara_spell_t( name, p, options_str )
  {
    id = 11;

    _breaks_stealth = false;

    name_str_reporting = "Matriach Macabre";

    trigger_gcd                   = timespan_t::zero();
    resource_current              = RESOURCE_SPIRIT;
    base_costs[ RESOURCE_SPIRIT ] = 100;

    add_child( p->actions.queens_fang_ult_clone );
    add_child( p->actions.arachnid_assault_clone );
    
  }

  void execute() override
  {
    mara_spell_t::execute();
    p()->fs_buffs.spirit_of_heroism->trigger();
    p()->buffs.ultimate_buff_window->trigger();
    p()->used_ultimate();
  }
};

struct corrosive_spill_dot_t : public mara_poison_t
{
  corrosive_spill_dot_t( util::string_view name, mara_t* p ) : mara_poison_t( name, p )
  {
    background             = true;
    dot_duration           = p->talents.corrosive_spill_duration;
    dot_behavior           = DOT_REFRESH_DURATION;
    base_tick_time         = p->talents.corrosive_spill_ticktime;
    hasted_ticks           = true;
    tick_zero              = true;
    dot_allow_partial_tick = false;

    id = 12;

    name_str_reporting = "Corrosive Spill";

    attack_power_mod.tick = p->talents.corrosive_spill_damage;
  }

  void tick( dot_t* d ) override
  {
    bool misses = rng().roll( p()->talents.corrosive_spill_cumulative_chance_per_tick_of_miss * d->current_tick );

    if ( !misses )
      mara_poison_t::tick( d );

    if ( misses )
      d->cancel();
  }
};

struct hemotoxin_dot_t : public mara_poison_t
{
  hemotoxin_dot_t( util::string_view name, mara_t* p ) : mara_poison_t( name, p )
  {
    background     = true;

    dot_duration   = p->talents.hemotoxin_duration;
    dot_behavior   = DOT_REFRESH_DURATION;
    base_tick_time = p->talents.hemotoxin_period;
    hasted_ticks   = true;

    id = 13;

    name_str_reporting = "Hemotoxin (DoT)";

    attack_power_mod.tick = p->talents.hemotoxin_damage_coefficient;
  }

  void last_tick( dot_t* d ) override
  {
    mara_poison_t::last_tick( d );

    p()->get_target_data( d->state->target )->debuffs.hemotoxin->expire();
  }
};

struct hemotoxin_explosion_t : public mara_poison_t
{
  hemotoxin_explosion_t( util::string_view name, mara_t* p ) : mara_poison_t( name, p )
  {
    background = true;
    may_crit   = true;
    id         = 14;

    name_str_reporting = "Hemotoxin";

    aoe                 = -1;

    reduced_aoe_targets = p->talents.hemotoxin_aoe_falloff;
    full_amount_targets = 1;
  }

  double composite_aoe_multiplier( const action_state_t* s ) const override
  {
    auto aoe = mara_poison_t::composite_aoe_multiplier( s );

    if ( s->chain_target > 0 )
      aoe *= p()->talents.hemotoxin_aoe_damage_reduction;

    return aoe;
  }

  void init() override
  {
    mara_poison_t::init();

    snapshot_flags &= STATE_NO_MULTIPLIER;
    snapshot_flags |= STATE_TARGET | STATE_CRIT | STATE_TGT_CRIT;
    snapshot_flags &= ~STATE_TGT_MUL_PET;
  }
};

struct stalker_step_t : public mara_spell_t
{
  stalker_step_t( util::string_view name, mara_t* p, util::string_view options_str = {} )
    : mara_spell_t( name, p, options_str )
  {
    id = 15;

    _breaks_stealth = false;

    name_str_reporting = "Stalker Step";

    trigger_gcd = timespan_t::zero();

    cooldown->duration = 30_s;
    cooldown->hasted   = true;
    cooldown->charges  = 1;
  }

  void impact( action_state_t* s ) override
  {
    mara_spell_t::impact( s );
  }
};

struct vexiras_venom_t : public residual_action::residual_periodic_action_t<mara_poison_t>
{
  vexiras_venom_t( util::string_view name, mara_t* p ) : residual_action_t( name, p )
  {
    id = 16;

    name_str_reporting = "Vexiras Venom";

    tick_may_crit = false;
        
    dot_duration           = p->legendary.vexiras_venom_duration;
    dot_behavior           = DOT_REFRESH_DURATION;
    base_tick_time         = p->legendary.vexiras_venom_period;
    hasted_ticks           = true;
    dot_allow_partial_tick = true;
  }

  void snapshot_state( action_state_t* state, result_amount_type rt ) override
  {
    attack_t::snapshot_state( state, rt );
  }

  void init() override
  {
    base_t::init();
    snapshot_flags |= STATE_HASTE;
    update_flags &= ~STATE_HASTE;
  }
};

struct skittering_blades_t : public mara_attack_t
{
  skittering_blades_t( util::string_view name, mara_t* p, util::string_view options_str = {} )
    : mara_attack_t( name, p, options_str )
  {
    id = 17;

    school                        = SCHOOL_PHYSICAL;
    attack_power_mod.direct       = 0.806;
    resource_current              = RESOURCE_ENERGY;
    base_costs[ RESOURCE_ENERGY ] = 35;

    aoe                 = -1;
    reduced_aoe_targets = 8;

    name_str_reporting = "Skittering Blades";
  }

  void impact( action_state_t* s ) override
  {
    mara_attack_t::impact( s );

    trigger_combo_point_gain( s->result == RESULT_CRIT ? 2 : 1, gain );

    if ( p()->talents_enabled( mara_t::FEED_THE_QUEEN ) )
    {
      p()->buffs.feed_the_queen->trigger();
    }

    if ( p()->buffs.brooding_shadows->check() )
    {
      p()->actions.volatile_poison_dot->set_target( s->target );
      p()->actions.volatile_poison_dot->execute();
    }
  }

  void execute() override
  {
    mara_attack_t::execute();

    if ( p()->buffs.maiden_of_death->check() )
    {
      trigger_combo_point_gain( COMBO_POINT_MAX, p()->actions.maiden_of_death->gain );
    }
  }
};

struct arachnid_assault_t : public mara_attack_t
{
  arachnid_assault_t( util::string_view name, mara_t* p, util::string_view options_str = {}, secondary_trigger st = secondary_trigger::NONE )
    : mara_attack_t( name, p, options_str )
  {
    id = 18;

    secondary_trigger_type = st;

    name_str_reporting = "Arachnid Assault";
        
    aoe                 = -1;
    reduced_aoe_targets = 8;

    school                             = SCHOOL_PHYSICAL;
    attack_power_mod.direct            = 0.858;
    resource_current                   = RESOURCE_ENERGY;
    base_costs[ RESOURCE_COMBO_POINT ] = 1;
    base_costs[ RESOURCE_ENERGY ]      = 45;

    /*if ( p->talents_enabled( mara_t::EFFICIENT_KILLER ) )
      base_costs[ RESOURCE_ENERGY ] *= p->talents.efficient_killer_energy_cost_modifier;*/

    if ( st == secondary_trigger::NONE && p->legendary.spirit_procs_clones )
    {
      add_child( p->actions.arachnid_assault_lego_clone );
    }

  }

  double composite_da_multiplier( const action_state_t* state ) const override
  {
    double m = mara_attack_t::composite_da_multiplier( state );

    m *= combo_point_da_multiplier( state );

    m *= 1 + p()->buffs.assassins_guile->check_value();

    if ( secondary_trigger_type != secondary_trigger::TALENT_CLONE )
      m *= 1 + p()->buffs.malevolence_qf_buffs_aa->check_value();

    return m;
  }

  double composite_target_da_multiplier( player_t* t ) const override
  {
    auto m = mara_attack_t::composite_target_multiplier( t );

    if ( p()->talents_enabled( mara_t::ARACHNID_ONSLAUGHT ) &&
         p()->get_target_data( t )->dots.hemorrhaging_strike->is_ticking() )
      m *= p()->talents.arachnid_onslaught_multiplier;

    return m;
  }

  double composite_crit_chance() const override
  {
    return mara_attack_t::composite_crit_chance() + p()->buffs.deadly_scheme->check_value();
  }

  void execute() override
  {
    bool had_clones = false;
    if ( !is_secondary_action() )
    {
      had_clones = p()->buffs.spirit_proc_clones->check();
      p()->buffs.spirit_proc_clones->decrement();
    }

    mara_attack_t::execute();


    if ( secondary_trigger_type == secondary_trigger::NONE && p()->buffs.ultimate_buff_window->check() )
    {
      p()->actions.arachnid_assault_clone->trigger_secondary_action(
          p()->actions.arachnid_assault_clone->get_state( execute_state ), 0.3_s );

      p()->actions.arachnid_assault_clone->trigger_secondary_action(
          p()->actions.arachnid_assault_clone->get_state( execute_state ), 0.6_s );
    }

    if ( secondary_trigger_type == secondary_trigger::NONE && had_clones )
    {
      for ( int i = 0; i <= p()->legendary.spirit_procs_clones_clones; i++ )
      {
        p()->actions.arachnid_assault_lego_clone->trigger_secondary_action(
            p()->actions.arachnid_assault_lego_clone->get_state( execute_state ),
            0.6_s * as<double>( i ) / p()->legendary.spirit_procs_clones_clones );
      }
    }

    if ( !is_secondary_action() )
      p()->buffs.deadly_scheme->expire();

    if ( !is_secondary_action() && p()->talents_enabled( mara_t::MALEVOLENCE ) )
    {
      p()->buffs.malevolence_aa_buffs_qf->trigger();
      p()->buffs.malevolence_qf_buffs_aa->decrement();
    }
  }

  void impact( action_state_t* state ) override
  {
    mara_attack_t::impact( state );

    handle_vexiras_venom( state );
  }
};

struct volatile_poison_state_t : public mara_action_state_t
{

  timespan_t dot_duration;
  timespan_t tick_time;
  volatile_poison_state_t( action_t* action, player_t* target )
    : mara_action_state_t( action, target ), dot_duration( 0_s ), tick_time( 0_s )
  {
  }

  void initialize() override
  {
    mara_action_state_t::initialize();

    dot_duration = 0_s;
    tick_time    = 0_s;
  }

  void copy_state( const action_state_t* s )
  {
    mara_action_state_t::copy_state( s );
    const volatile_poison_state_t* rs = debug_cast<const volatile_poison_state_t*>( s );

    dot_duration = rs->dot_duration;
    tick_time    = rs->tick_time;
  }

};

struct volatile_poison_dot_t : public mara_poison_t
{
  volatile_poison_dot_t( util::string_view name, mara_t* p ) : mara_poison_t( name, p )
  {
    background = true;

    dot_duration   = 6_s;
    dot_behavior   = DOT_REFRESH_DURATION;
    base_tick_time = 1_s;

    id = 18;

    name_str_reporting = "Volatile Poison";

    attack_power_mod.tick = 0.312;
  }

  mara_t* p()
  {
    return debug_cast<mara_t*>( player );
  }

  const mara_t* p() const
  {
    return debug_cast<const mara_t*>( player );
  }

  mara_td_t* td( player_t* t ) const
  {
    return p()->get_target_data( t );
  }

  // Action State =============================================================
  // Type Wrappers ============================================================

  static const volatile_poison_state_t* cast_state( const action_state_t* st )
  {
    return debug_cast<const volatile_poison_state_t*>( st );
  }

  static volatile_poison_state_t* cast_state( action_state_t* st )
  {
    return debug_cast<volatile_poison_state_t*>( st );
  }

  action_state_t* new_state() override
  {
    return new volatile_poison_state_t( this, target );
  }

  void update_state( action_state_t* state, unsigned flags, result_amount_type rt ) override
  {
    mara_poison_t::update_state( state, flags, rt );
  }

  void snapshot_state( action_state_t* state, result_amount_type rt ) override
  {
    auto rs = cast_state( state );
    // rs->

    rs->dot_duration = composite_dot_duration( state );
    rs->tick_time    = tick_time( state );

    mara_poison_t::snapshot_state( state, rt );
  }

  /*timespan_t composite_dot_duration( const action_state_t* s ) const override
  {
    if ( cast_state( s )->dot_duration > 0_s )
      return cast_state( s )->dot_duration;

    return rng().range( 6_s, 8_s );
  }

  timespan_t tick_time( const action_state_t* s ) const override
  {
    if ( cast_state( s )->tick_time > 0_s )
      return cast_state( s )->tick_time;

    if ( cast_state( s )->dot_duration > 0_s )
    {
      return cast_state( s )->dot_duration / 6;
    }

    return 1_s;
  }*/

  void last_tick( dot_t* d ) override
  {
    mara_poison_t::last_tick( d );

    p()->actions.volatile_poison_aoe->execute_on_target( d->target );
  }
};

struct volatile_poison_aoe_t : public mara_poison_t
{
  volatile_poison_aoe_t( util::string_view name, mara_t* p ) : mara_poison_t( name, p )
  {
    background = true;
    id         = 19;

    attack_power_mod.direct = 0.624;

    name_str_reporting = "Volatile Poison (AoE)";

    aoe                 = -1;
    reduced_aoe_targets = 3;
  }
};

}  // namespace actions

// ==========================================================================
// Rogue Targetdata Definitions
// ==========================================================================

mara_td_t::mara_td_t( player_t* target, mara_t* source ) : fellowship::fs_player_td_t( target, source ), dots(), debuffs()
{
  dots.seething_poison    = target->get_dot( "seething_poison", source );
  dots.hemorrhaging_strike = target->get_dot( "hemorrhaging_strike", source );
  dots.hemotoxin          = target->get_dot( "hemotoxin_dot", source );
  dots.volatile_poison    = target->get_dot( "volatile_poison", source );

  debuffs.nightstalker = make_buff( *this, "nightstalker" )
                             ->set_duration( source->talents.nightstalker_duration )
                             ->set_max_stack( 1 )
                             ->set_refresh_behavior( buff_refresh_behavior::DURATION )
                             ->set_default_value( source->talents.nightstalker_damage_taken );

  debuffs.hemotoxin = make_buff( *this, "hemotoxin" )
                          ->set_duration( source->talents.hemotoxin_duration )
                          ->set_max_stack( source->talents.hemotoxin_max_stacks )
                          ->set_refresh_behavior( buff_refresh_behavior::DURATION )
                          ->set_stack_change_callback( [ this ]( buff_t* b, int old, int _new ) {
                            if ( !_new )
                            {
                              dots.hemotoxin->reset();
                            }
                          } );

  debuffs.puncture = make_buff( *this, "puncture" )
                         ->set_duration( source->talents.puncture_buff_duration )
                         ->set_max_stack( 1 )
                         ->set_refresh_behavior( buff_refresh_behavior::DURATION )
                         ->set_default_value( source->talents.puncture_buff_tickrate );

}

// ==========================================================================
// Rogue Character Definition
// ==========================================================================

// mara_t::composite_attribute_multiplier ==================================

double mara_t::composite_attribute_multiplier( attribute_e a ) const
{
  double am = fs_player_t::composite_attribute_multiplier( a );

  return am;
}

// mara_t::composite_melee_auto_attack_speed ===============================

double mara_t::composite_melee_auto_attack_speed() const
{
  double h = fs_player_t::composite_melee_auto_attack_speed();

  return h;
}

// mara_t::composite_melee_haste ==========================================

double mara_t::composite_melee_haste() const
{
  double h = fs_player_t::composite_melee_haste();

  return h;
}

// mara_t::composite_spell_haste ==========================================

double mara_t::composite_spell_haste() const
{
  double h = fs_player_t::composite_spell_haste();

  return h;
}

// mara_t::composite_melee_crit_chance =========================================

double mara_t::composite_melee_crit_chance() const
{
  double crit = fs_player_t::composite_melee_crit_chance();

  return crit;
}

// mara_t::composite_spell_crit_chance =========================================

double mara_t::composite_spell_crit_chance() const
{
  double crit = fs_player_t::composite_spell_crit_chance();

  return crit;
}

// mara_t::composite_damage_versatility ===================================

double mara_t::composite_damage_versatility() const
{
  double cdv = fs_player_t::composite_damage_versatility();

  return cdv;
}

// mara_t::composite_heal_versatility =====================================

double mara_t::composite_heal_versatility() const
{
  double chv = fs_player_t::composite_heal_versatility();

  return chv;
}

// mara_t::composite_leech ===============================================

double mara_t::composite_leech() const
{
  double l = fs_player_t::composite_leech();

  return l;
}

// mara_t::matching_gear_multiplier ========================================

double mara_t::matching_gear_multiplier( attribute_e attr ) const
{
  return 0.0;
}

// mara_t::composite_player_multiplier =====================================

double mara_t::composite_player_multiplier( school_e school ) const
{
  double m = fs_player_t::composite_player_multiplier( school );

  return m;
}

// mara_t::composite_player_pet_damage_multiplier ==========================

double mara_t::composite_player_pet_damage_multiplier( const action_state_t* s, bool guardian ) const
{
  double m = fs_player_t::composite_player_pet_damage_multiplier( s, guardian );

  return m;
}

// mara_t::composite_player_target_multiplier ==============================

double mara_t::composite_player_target_multiplier( player_t* target, school_e school ) const
{
  double m = fs_player_t::composite_player_target_multiplier( target, school );

  // mara_td_t* tdata = get_target_data( target );

  if ( buffs.maiden_of_death->check() )
  {
    if ( talents_enabled( MAIDENS_DOOM ) && target->health_percentage() <= talents.maidens_doom_execute_threshold )
    {
      m *= 1.2 + talents.maidens_doom_execute_amp;
    }
    else
    {
      m *= 1.2;
    }
  }

  if ( buffs.ultimate_buff_window->check() )
  {
    m *= 1 + buffs.ultimate_buff_window->value();
  }

  return m;
}

// mara_t::composite_player_target_crit_chance =============================

double mara_t::composite_player_target_crit_chance( player_t* target ) const
{
  double c = fs_player_t::composite_player_target_crit_chance( target );

  return c;
}

// mara_t::composite_player_target_armor ===================================

double mara_t::composite_player_target_armor( player_t* target ) const
{
  return 0.0;

  double a = fs_player_t::composite_player_target_armor( target );

  return a;
}
// mara_t::init_actions ====================================================

void mara_t::init_action_list()
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

// mara_t::create_action  ==================================================

action_t* mara_t::create_action( util::string_view name, util::string_view options_str )
{
  using namespace actions;

  if ( name == "backstab" )
    return new backstab_t( name, this, options_str );
  if ( name == "queens_fang" )
    return new queens_fang_t( name, this, options_str );
  if ( name == "widows_bite" )
    return new widows_bite_t( name, this, options_str );
  if ( name == "brooding_shadows" )
    return new brooding_shadows_t( name, this, options_str );
  if ( name == "hemorrhaging_strike" )
    return new hemorrhaging_strike_t( name, this, options_str );
  if ( name == "maiden_of_death" )
    return new maiden_of_death_t( name, this, options_str );
  if ( name == "final_stratagem" )
    return new final_stratagem_t( name, this, options_str );
  if ( name == "matriach_macabre" )
    return new matriach_macabre_t( name, this, options_str );
  if ( name == "stalker_step" )
    return new stalker_step_t( name, this, options_str );
  if ( name == "skittering_blades" )
    return new skittering_blades_t( name, this, options_str );
  if ( name == "arachnid_assault" )
    return new arachnid_assault_t( name, this, options_str );

  return fs_player_t::create_action( name, options_str );
}

// mara_t::create_expression ===============================================

std::unique_ptr<expr_t> mara_t::create_action_expression( action_t& action, std::string_view name_str )
{
  auto split = util::string_split<util::string_view>( name_str, "." );

  if ( util::str_compare_ci( name_str, "bleeds" ) )
  {
    return make_fn_expr( name_str, [ this, &action ]() {
      mara_td_t* tdata = get_target_data( action.get_expression_target() );
      return tdata->total_bleeds();
    } );
  }
  else if ( util::str_compare_ci( name_str, "poisons" ) )
  {
    return make_fn_expr( name_str, [ this, &action ]() {
      mara_td_t* tdata = get_target_data( action.get_expression_target() );
      return tdata->total_poisons();
    } );
  }

  return fs_player_t::create_action_expression( action, name_str );
}

std::unique_ptr<expr_t> mara_t::create_expression( util::string_view name_str )
{
  auto split = util::string_split<util::string_view>( name_str, "." );

  if ( split[ 0 ] == "combo_points" || split[ 1 ] == "cp" )
  {
    if ( split.size() == 1 )
    {
      return make_fn_expr( name_str, [ this ] { return this->current_cp( true ); } );
    }

    if ( split.size() == 2 && split[ 1 ] == "deficit" )
    {
      return make_fn_expr( name_str,
                           [ this ] { return resources.max[ RESOURCE_COMBO_POINT ] - this->current_cp( true ); } );
    }
  }
  else if ( util::str_compare_ci( name_str, "cp_max_spend" ) )
  {
    return make_mem_fn_expr( name_str, *this, &mara_t::consume_cp_max );
  }
  else if ( util::str_compare_ci( split[ 0 ], "talent" ) )
  {
    if ( split.size() == 2 )
    {
      for ( mara_talents_t t = static_cast<mara_talents_t>( 1U ); t < mara_talents_t::MAX; t++ )
      {
        if ( util::str_compare_ci( split[ 1 ], talent_name( t ) ) )
        {
          return make_fn_expr( name_str, std::bind( std::mem_fn( &mara_t::talents_enabled ), this, t ) );
        }
      }
    }
  }
  else if ( util::str_compare_ci( split[ 0 ], "legendary" ) )
  {
    if ( split.size() == 2 )
    {
      if ( util::str_compare_ci( split[ 1 ], "vexiras_venom" ) )
      {
        return make_ref_expr( name_str, legendary.vexiras_venom );
      }
      else if ( util::str_compare_ci( split[ 1 ], "drenched_in_blood" ) )
      {
        return make_ref_expr( name_str, legendary.drenched_in_blood );
      }
      else if ( util::str_compare_ci( split[ 1 ], "from_the_shadows" ) )
      {
        return make_ref_expr( name_str, legendary.from_the_shadows );
      }
      else if ( util::str_compare_ci( split[ 1 ], "spirit_procs_clones" ) )
      {
        return make_ref_expr( name_str, legendary.spirit_procs_clones );
      }
    }
  }
  else if ( util::str_compare_ci( split[ 0 ], "deadly_scheme_energy" ) )
  {
    return make_ref_expr( name_str, deadly_energy_tracker );
  }
  // Split expressions

  return fs_player_t::create_expression( name_str );
}

std::unique_ptr<expr_t> mara_t::create_resource_expression( util::string_view name_str )
{
  auto splits = util::string_split<util::string_view>( name_str, "." );
  if ( splits.empty() )
    return nullptr;

  resource_e r = util::parse_resource_type( splits[ 0 ] );
  if ( r == RESOURCE_ENERGY )
  {
    // Custom Rogue Energy Deficit
    // Ignores temporary energy max when calculating the current deficit
    if ( splits.size() == 2 && ( splits[ 1 ] == "base_deficit" ) )
    {
      return make_fn_expr( name_str, [ this ] {
        return std::max( resources.max[ RESOURCE_ENERGY ] - resources.current[ RESOURCE_ENERGY ] -
                             resources.temporary[ RESOURCE_ENERGY ],
                         0.0 );
      } );
    }
    // Custom Rogue Energy Regen Functions
    // Optionally handles things that are outside of the normal resource_regen_per_second flow
    if ( splits.size() == 2 && ( splits[ 1 ] == "regen" || splits[ 1 ] == "regen_combined" ||
                                 splits[ 1 ] == "time_to_max" || splits[ 1 ] == "time_to_max_combined" ||
                                 splits[ 1 ] == "base_time_to_max" || splits[ 1 ] == "base_time_to_max_combined" ) )
    {
      const bool regen    = util::str_prefix_ci( splits[ 1 ], "regen" );
      const bool combined = util::str_in_str_ci( splits[ 1 ], "_combined" );
      const bool base_max = util::str_prefix_ci( splits[ 1 ], "base" );

      return make_fn_expr( name_str, [ this, regen, combined, base_max ] {
        double energy_deficit          = resources.max[ RESOURCE_ENERGY ] - resources.current[ RESOURCE_ENERGY ];
        double energy_regen_per_second = resource_regen_per_second( RESOURCE_ENERGY );

        if ( base_max )
        {
          energy_deficit = std::max( energy_deficit - resources.temporary[ RESOURCE_ENERGY ], 0.0 );
        }

        // Combined non-traditional or inconsistent regen sources
        if ( combined )
        {
          double bleed_regen = 0;

          for ( auto p : sim->target_non_sleeping_list )
          {
            mara_td_t* tdata = get_target_data( p );
            if ( tdata->dots.hemorrhaging_strike->is_ticking() )

            {
              bleed_regen += spell_const.hemorrhaging_strike_energy_gen *
                             tdata->dots.hemorrhaging_strike->ticks_left_fractional() /
                             tdata->dots.hemorrhaging_strike->remains().total_seconds();
            }
          }

          energy_regen_per_second += bleed_regen;
        }

        // TODO - Add support for Master of Shadows, and potentially also estimated Combat Potency, ShT etc.
        //        Also consider if buffs such as Adrenaline Rush should be prorated based on duration for time_to_max

        return regen ? energy_regen_per_second : energy_deficit / energy_regen_per_second;
      } );
    }
  }

  return fs_player_t::create_resource_expression( name_str );
}

// mara_t::init_base =======================================================

void mara_t::init_base_stats()
{
  if ( base.distance < 1 )
    base.distance = 5;

  fs_player_t::init_base_stats();

  base.stats.attribute[ STAT_AGILITY ] = 1000;
  resources.base[ RESOURCE_HEALTH ]    = 17239;

  base.health_per_stamina = 53.099;

  base.mastery = 0.0;

  resources.base[ RESOURCE_COMBO_POINT ] = COMBO_POINT_MAX;

  resources.base[ RESOURCE_ENERGY ] = 200;

  /*if ( talents.efficient_killer )
    resources.base[ RESOURCE_ENERGY ] *= talents.efficient_killer_max_energy_boost;*/

  resources.base_regen_per_second[ RESOURCE_ENERGY ] = 10;

  base_gcd = timespan_t::from_seconds( 1.0 );
  min_gcd  = timespan_t::from_seconds( 1.0 );
}

// mara_t::init_spells =====================================================

void mara_t::init_spells()
{
  fs_player_t::init_spells();

  actions.auto_attack = new actions::auto_melee_attack_t( this, "" );
}

// mara_t::init_talents ====================================================

void mara_t::init_talents()
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

    for ( mara_talents_t t = static_cast<mara_talents_t>( 1U ); t < mara_talents_t::MAX; t++ )
    {
      if ( util::str_compare_ci( talent_split[ 0 ], talent_name( t ) ) )
      {
        set_talent_points( t, ranks >= 1 );
        break;
      }
    }
  }
}

// mara_t::init_gains ======================================================

void mara_t::init_gains()
{
  fs_player_t::init_gains();

  gains.spirit_procs     = get_gain( "Spirit Procs" );
  gains.venomous_delight = get_gain( "Venomous Delight" );
  gains.efficient_killer = get_gain( "Efficient Killer" );

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

// mara_t::init_procs ======================================================

void mara_t::init_procs()
{
  fs_player_t::init_procs();
}

// mara_t::init_scaling ====================================================

void mara_t::init_scaling()
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

// mara_t::init_resources =================================================

void mara_t::init_resources( bool force )
{
  fs_player_t::init_resources( force );

  resources.current[ RESOURCE_COMBO_POINT ] = 0;
}

// mara_t::init_buffs ======================================================

void mara_t::create_buffs()
{
  fs_player_t::create_buffs();

  
  buffs.feed_the_queen = make_buff<mara_buff_t>( this, "feed_the_queen" );
  buffs.feed_the_queen->set_duration( 0_s )
      ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT )
      ->set_default_value( talents.feed_the_queen_bonus_per_stack )
      ->set_max_stack( talents.feed_the_queen_max_stacks );

  buffs.brooding_shadows = make_buff<mara_buff_t>( this, "brooding_shadows" );
  buffs.brooding_shadows->set_duration( 0_s )->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

  if ( talents_enabled( ASSASSINS_GUILE ) )
  {
    buffs.brooding_shadows->add_stack_change_callback( [ this ]( buff_t*, int, int _new ) {
      if ( !_new )
      {
        buffs.assassins_guile->trigger();
      }
    } );
  }

  buffs.predators_rush = make_buff<mara_buff_t>( this, "predators_rush" );
  buffs.predators_rush->set_duration( 0_s )
      ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT )
      ->set_default_value( 0.4 );

  buffs.maiden_of_death = make_buff<mara_buff_t>( this, "maiden_of_death" );
  buffs.maiden_of_death->set_duration( 10_s )
      //->set_refresh_behavior( buff_refresh_behavior::EXTEND )
      ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT )
      ->set_default_value( 0.2 );

  buffs.ultimate_buff_window = make_buff<mara_buff_t>( this, "matriach_macabre" );
  buffs.ultimate_buff_window->set_duration( 20_s )->set_default_value( 0.2 );

  buffs.red_ledger = make_buff<mara_buff_t>( this, "red_ledger" );
  buffs.red_ledger->set_pct_buff_type( STAT_PCT_BUFF_HASTE )->set_default_value( talents.red_ledger_base );

  buffs.red_ledger_additional = make_buff<mara_buff_t>( this, "red_ledger_additional" );
  buffs.red_ledger_additional->set_max_stack( talents.red_ledger_max )
      ->set_pct_buff_type( STAT_PCT_BUFF_HASTE )
      ->set_default_value( talents.red_ledger_per_stack );

  buffs.deadly_scheme = make_buff<mara_buff_t>( this, "deadly_scheme" );
  buffs.deadly_scheme->set_default_value( talents.deadly_scheme_added_crit )
      ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

  buffs.assassins_guile = make_buff<mara_buff_t>( this, "assassins_guile" );
  buffs.assassins_guile->set_default_value( talents.assassins_guile_finisher_boost )
      ->set_duration( talents.assassins_guile_buff_duration )
      ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

  buffs.drenched_in_blood = make_buff<mara_buff_t>( this, "drenched_in_blood" );
  buffs.drenched_in_blood->set_pct_buff_type( STAT_PCT_BUFF_VERSATILITY )
      ->set_default_value( legendary.drenched_in_blood_exp )
      ->set_duration( legendary.drenched_in_blood_duration );

  buffs.malevolence_aa_buffs_qf = make_buff<mara_buff_t>( this, "malevolence_aa_buffs_qf" );
  buffs.malevolence_aa_buffs_qf->set_duration( talents.malevolence_duration )
      ->set_max_stack( talents.malevolence_max_stacks )
      ->set_default_value( talents.malevolence_amplifier );

  buffs.malevolence_qf_buffs_aa = make_buff<mara_buff_t>( this, "malevolence_qf_buffs_aa" );
  buffs.malevolence_qf_buffs_aa->set_duration( talents.malevolence_duration )
      ->set_max_stack( talents.malevolence_max_stacks )
      ->set_default_value( talents.malevolence_amplifier );

  buffs.spirit_proc_clones = make_buff<mara_buff_t>( this, "spirit_procs_clones" );
  buffs.spirit_proc_clones->set_duration( legendary.spirit_procs_clones_duration );
}

// mara_t::invalidate_cache =========================================

void mara_t::invalidate_cache( cache_e c )
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

void mara_t::create_options()
{
  fs_player_t::create_options();

  add_option( opt_bool( "talent.gushing_blood_always_works", talents.gushing_blood_always_works ) );
  add_option( opt_int( "talent.gushing_blood_hemorrhaging_additional_targets",
                       talents.gushing_blood_hemorrhaging_additional_targets, 1, 20 ) );

  
  add_option( opt_timespan( "talent.sinners_pride_cdr_reduction_per_cp",
                            talents.sinners_pride_cdr_reduction_per_cp, 0_s, 180_s ) );

  add_option( opt_bool( "talent.puncture_buff", talents.puncture_buff ) );
  add_option( opt_float( "talent.puncture_buff_tickrate", talents.puncture_buff_tickrate, 0.0, 10.0 ) );
  add_option( opt_float( "talent.puncture_cc", talents.puncture_cc, 0.0, 2.0 ) );
  add_option( opt_timespan( "talent.puncture_buff_duration", talents.puncture_buff_duration, 1_s, 30_s ) );
  add_option( opt_float( "options.widows_bite_extra_energy", options.widows_bite_extra_energy, 0.0, 100.0 ) );


  add_option( opt_bool( "legendary.vexiras_venom", legendary.vexiras_venom ) );
  add_option( opt_bool( "legendary.drenched_in_blood", legendary.drenched_in_blood ) );

  add_option( opt_bool( "legendary.from_the_shadows", legendary.from_the_shadows ) );
  add_option( opt_float( "legendary.from_the_shadows_chance", legendary.from_the_shadows_chance, 0.0, 1.0 ) );
  add_option( opt_float( "legendary.from_the_shadows_bleed_rate", legendary.from_the_shadows_bleed_rate, 0.0, 100.0 ) );

  add_option( opt_bool( "legendary.spirit_procs_clones", legendary.spirit_procs_clones ) );
  add_option( opt_bool( "legendary.spirit_procs_clones_proc_on_next", legendary.spirit_procs_clones_proc_on_next ) );
  add_option( opt_int( "legendary.spirit_procs_clones_clones", legendary.spirit_procs_clones_clones, 1, 200 ) );


  /*add_option( opt_bool( "ready_trigger", options.mara_ready_trigger ) );

  add_option( opt_func( "off_hand_secondary", parse_offhand_secondary ) );
  add_option( opt_func( "main_hand_secondary", parse_mainhand_secondary ) );
  add_option( opt_int( "initial_combo_points", options.initial_combo_points ) );
  add_option( opt_int( "initial_shadow_techniques", options.initial_shadow_techniques, -1, 4 ) );
  add_option( opt_int( "initial_supercharged_cp", options.initial_supercharged_cp, 0, 2 ) );
  add_option( opt_func( "fixed_rtb", parse_fixed_rtb ) );
  add_option( opt_func( "fixed_rtb_odds", parse_fixed_rtb_odds ) );
  add_option( opt_bool( "priority_rotation", options.priority_rotation ) );*/
}

// mara_t::copy_from =======================================================

void mara_t::copy_from( player_t* source )
{
  mara_t* mara = static_cast<mara_t*>( source );
  fs_player_t::copy_from( source );

  talents   = mara->talents;
  legendary = mara->legendary;
  options   = mara->options;
}

// mara_t::create_profile  =================================================

std::string mara_t::create_profile( save_e stype )
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

// mara_t::init_items ======================================================

void mara_t::init_items()
{
  fs_player_t::init_items();
}

// mara_t::init_special_effects ============================================

void mara_t::init_special_effects()
{
  fs_player_t::init_special_effects();
}

// mara_t::init_finished ===================================================

void mara_t::init_finished()
{
  fs_player_t::init_finished();
}

void mara_t::init_background_actions()
{
  fs_player_t::init_background_actions();

  actions.seething_poison = new actions::seething_poison_t( "seething_poison", this );
  actions.caustic_poison  = new actions::caustic_poison_t( "caustic_poison", this );

  actions.maiden_of_death = new actions::maiden_of_death_t( "maiden_of_death", this );

  actions.queens_fang_ult_clone =
      new actions::queens_fang_t( "queens_fang_ult", this, {} , secondary_trigger::ULTIMATE_CLONE );
  actions.queens_fang_ult_clone->background = true;

  actions.queens_fang_fts_clone =
      new actions::queens_fang_t( "queens_fang_fts", this, {}, secondary_trigger::TALENT_CLONE );
  actions.queens_fang_fts_clone->background = true;

  actions.queens_fang_lego_clone =
      new actions::queens_fang_t( "queens_fang_lego", this, {}, secondary_trigger::LEGENDARY_CLONE );
  actions.queens_fang_lego_clone->background = true;

  actions.arachnid_assault_lego_clone =
      new actions::arachnid_assault_t( "arachnid_assault_lego", this, {}, secondary_trigger::LEGENDARY_CLONE );
  actions.arachnid_assault_lego_clone->background = true;


  actions.corrosive_spill = new actions::corrosive_spill_dot_t( "corrosive_spill", this );

  actions.hemotoxin_dot = new actions::hemotoxin_dot_t( "hemotoxin_dot", this );
  actions.hemotoxin     = new actions::hemotoxin_explosion_t( "hemotoxin", this );

  actions.vexiras_venom = new actions::vexiras_venom_t( "vexiras_venom", this );

  actions.arachnid_assault_clone =
      new actions::arachnid_assault_t( "arachnid_assault_ult", this, {}, secondary_trigger::ULTIMATE_CLONE );
  actions.arachnid_assault_clone->background = true;


  actions.volatile_poison_dot = new actions::volatile_poison_dot_t( "volatile_poison", this );
  actions.volatile_poison_aoe = new actions::volatile_poison_aoe_t( "volatile_poison_aoe", this );
}

// mara_t::reset ===========================================================

void mara_t::reset()
{
  fs_player_t::reset();
}

// mara_t::activate ========================================================

void mara_t::activate()
{
  fs_player_t::activate();
}

// mara_t::break_stealth ===================================================

void mara_t::break_stealth()
{
  if ( buffs.brooding_shadows->check() )
  {
    buffs.brooding_shadows->expire( 1_ms );
  }
}

// mara_t::cancel_auto_attack ==============================================

void mara_t::cancel_auto_attacks()
{
  if ( actions.melee_hit && actions.melee_hit->execute_event )
  {
    actions.melee_hit->canceled            = true;
    actions.melee_hit->prev_scheduled_time = actions.melee_hit->execute_event->occurs();
  }

  fs_player_t::cancel_auto_attacks();
}

// mara_t::arise ===========================================================

void mara_t::arise()
{
  fs_player_t::arise();

  resources.current[ RESOURCE_COMBO_POINT ] = 0;
}

// mara_t::combat_begin ====================================================

void mara_t::combat_begin()
{
  fs_player_t::combat_begin();
}

// mara_t::energy_regen_per_second =========================================

double mara_t::resource_regen_per_second( resource_e r ) const
{
  double reg = fs_player_t::resource_regen_per_second( r );

  if ( r == RESOURCE_ENERGY )
  {
    reg *= 1.0 + buffs.predators_rush->check_value();
    reg *= 1.0 + buffs.maiden_of_death->check_value();
  }

  // We handle some energy gain increases in mara_t::regen() instead of the resource_regen_per_second method in order
  // to better track their benefits. For simple implementation without separate tracking, can put stuff here.

  return reg;
}

double mara_t::resource_gain( resource_e resource_type, double amount, gain_t* source, action_t* action )
{
  double actual_amount = fs_player_t::resource_gain( resource_type, amount, source, action );
  if ( talent_enabled( DEADLY_SCHEME ) && resource_type == RESOURCE_ENERGY && actual_amount > 0 )
  {
    deadly_energy_tracker += actual_amount;
    if ( deadly_energy_tracker >= talents.deadly_scheme_required_energy )
    {
      deadly_energy_tracker -= talents.deadly_scheme_required_energy;
      buffs.deadly_scheme->trigger();
    }
  }
  return actual_amount;
}

// mara_t::non_stacking_movement_modifier ==================================

double mara_t::non_stacking_movement_modifier() const
{
  double ms = fs_player_t::non_stacking_movement_modifier();

  return ms;
}

// mara_t::stacking_movement_modifier===================================

double mara_t::stacking_movement_modifier() const
{
  double ms = fs_player_t::stacking_movement_modifier();

  return ms;
}

// mara_t::regen ===========================================================

void mara_t::regen( timespan_t periodicity )
{
  fs_player_t::regen( periodicity );

  // We handle some energy gain increases here instead of the resource_regen_per_second method in order to better track
  // their benefits. IMPORTANT NOTE: If anything is updated/added here, mara_t::create_resource_expression() needs to
  // be updated as well to reflect this
  // if ( !resources.is_infinite( RESOURCE_ENERGY ) )
  //{
  //  // Multiplicative energy gains
  //  double mult_regen_base = periodicity.total_seconds() * resource_regen_per_second( RESOURCE_ENERGY );

  //  if ( buffs.adrenaline_rush->up() )
  //  {
  //    double energy_regen = mult_regen_base * buffs.adrenaline_rush->data().effectN( 1 ).percent();
  //    resource_gain( RESOURCE_ENERGY, energy_regen, gains.adrenaline_rush );
  //    mult_regen_base += energy_regen;
  //  }

  //  // Additive energy gains
  //  if ( buffs.buried_treasure->up() )
  //    resource_gain( RESOURCE_ENERGY, buffs.buried_treasure->check_value() * periodicity.total_seconds(),
  //                   gains.buried_treasure );
  //}
}

// mara_t::available =======================================================

timespan_t mara_t::available() const
{
  if ( ready_type == READY_POLL )
    return fs_player_t::available();

  const double energy = resources.current[ RESOURCE_ENERGY ];
  if ( energy >= mara_ready_trigger_threshold )
    return 100_ms;

  // TODO -- See if this is improved by considering more regen sources
  return std::max( 100_ms, timespan_t::from_seconds( ( mara_ready_trigger_threshold - energy ) /
                                                     resource_regen_per_second( RESOURCE_ENERGY ) ) );
}

template <typename Base>
void actions::mara_action_t<Base>::trigger_combo_point_gain( int cp, gain_t* gain )
{
  p()->resource_gain( RESOURCE_COMBO_POINT, cp, gain, this );
}

template <typename Base>
void actions::mara_action_t<Base>::trigger_auto_attack( const action_state_t* /* state */ )
{
  if ( !p()->main_hand_attack || p()->main_hand_attack->execute_event )
    return;

  p()->actions.auto_attack->schedule_execute();
}

template <typename Base>
void actions::mara_action_t<Base>::spend_combo_points( const action_state_t* state )
{
  if ( ab::base_costs[ RESOURCE_COMBO_POINT ] == 0 )
    return;

  if ( !ab::hit_any_target )
    return;

  const auto rs    = cast_state( state );
  double max_spend = std::min( p()->current_cp(), p()->consume_cp_max() );
  ab::stats->consume_resource( RESOURCE_COMBO_POINT, max_spend );
  p()->resource_loss( RESOURCE_COMBO_POINT, max_spend );

  p()->sim->print_log( "{} consumes {} {} for {} ({})", *p(), max_spend,
                       util::resource_type_string( RESOURCE_COMBO_POINT ), *this, p()->current_cp() );

  trigger_poison_bomb( state, max_spend );

  if ( p()->talents_enabled( mara_t::SINNERS_PRIDE ) )
  {
    p()->cooldowns.maiden_of_death->adjust( -max_spend * p()->talents.sinners_pride_cdr_reduction_per_cp, false, true );
  }

  if ( p()->talents_enabled( mara_t::EFFICIENT_KILLER ) ) 
  {
    p()->resource_gain( RESOURCE_ENERGY, p()->talents.efficient_killer_energy_per_cp * max_spend,
                        p()->gains.efficient_killer, this );
  }

  if ( p()->rng().roll( p()->cache.mastery_value() ) )
  {
    trigger_spirit_refund( state, max_spend );
  }
}

template <typename Base>
void actions::mara_action_t<Base>::trigger_spirit_refund( const action_state_t* state, double max_spend )
{
  double energy_restored = ab::last_resource_cost;

  if ( p()->legendary.drenched_in_blood )
  {
    p()->buffs.drenched_in_blood->trigger();
  }

  if ( p()->legendary.spirit_procs_clones )
  {
    p()->buffs.spirit_proc_clones->trigger();
  }

  make_event( ab::sim, 200_ms, [ energy_restored, this, max_spend ] {
    p()->resource_gain( RESOURCE_ENERGY, energy_restored, p()->gains.spirit_procs, this);
    p()->resource_gain( RESOURCE_COMBO_POINT, max_spend, p()->gains.spirit_procs, this );
  } );

  p()->spirit_refund();
}

template <typename Base>
void actions::mara_action_t<Base>::trigger_poison_bomb( const action_state_t* state, double max_spend )
{
  if ( !p()->talents_enabled( mara_t::CORROSIVE_SPILL ) && !is_secondary_action() )
    return;

  const auto rs = cast_state( state );
  if ( p()->rng().roll( p()->talents.corrosive_spill_chance_per_cp * rs->get_combo_points() ) )
  {

    for ( player_t* t : ab::sim->target_non_sleeping_list )
    {
      p()->actions.corrosive_spill->execute_on_target( t );
    }
  }
}

template <typename Base>
void actions::mara_action_t<Base>::roll_for_hemotoxin( const action_state_t* state )
{
  if ( !p()->talents_enabled( mara_t::HEMOTOXIN ) || ab::result_is_miss( state->result ) )
    return;

  if ( p()->rng().roll( p()->talents.hemotoxin_chance ) )
  {
    mara_td_t* tdata = p()->get_target_data( state->target );
    tdata->debuffs.hemotoxin->trigger( p()->talents.hemotoxin_applied_stacks );
    p()->actions.hemotoxin_dot->set_target( state->target );
    p()->actions.hemotoxin_dot->execute();
  }
}

template <typename Base>
void actions::mara_action_t<Base>::handle_vexiras_venom( const action_state_t* state )
{
  if ( !p()->legendary.vexiras_venom || state->result != RESULT_CRIT )
    return;

  residual_action::trigger( p()->actions.vexiras_venom, state->target,
                            state->result_amount * p()->legendary.vexiras_venom_accumulate );
}


// mara_t::convert_hybrid_stat ==============================================

stat_e mara_t::convert_hybrid_stat( stat_e s ) const
{
  // this converts hybrid stats that either morph based on spec or only work
  // for certain specs into the appropriate "basic" stats
  switch ( s )
  {
    case STAT_STR_AGI_INT:
    case STAT_AGI_INT:
    case STAT_STR_AGI:
      return STAT_AGILITY;
      // This is a guess at how STR/INT gear will work for Rogues, TODO: confirm
      // This should probably never come up since rogues can't equip plate, but....
    case STAT_STR_INT:
      return STAT_NONE;
    case STAT_BONUS_ARMOR:
      return STAT_NONE;
    default:
      return s;
  }
}

void mara_t::create_cooldowns()
{
  cooldowns.brooding_shadows = get_cooldown( "brooding_shadows" );
  cooldowns.maiden_of_death  = get_cooldown( "maiden_of_death" );
  cooldowns.stalker_step     = get_cooldown( "stalker_step" );
  cooldowns.widows_bite      = get_cooldown( "widows_bite" );
  cooldowns.final_stratagem  = get_cooldown( "final_stratagem" );
}

class mara_module_t : public module_t
{
public:
  mara_module_t() : module_t( MARA )
  {
  }

  player_t* create_player( sim_t* sim, util::string_view name, race_e r = RACE_NONE ) const override
  {
    return new mara_t( sim, name, r );
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

}  // namespace mara
}

const module_t* module_t::mara()
{
  static fellowship::mara::mara_module_t m;
  return &m;
}
