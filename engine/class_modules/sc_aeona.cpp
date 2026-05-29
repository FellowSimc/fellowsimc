#include "fs_player.hpp"
#include "util/util.hpp"
#include "simulationcraft.hpp"

namespace fellowship
{
namespace aeona
{

// Forward Declarations
class aeona_t;

enum class secondary_trigger
{
  NONE = 0U,
};

namespace actions
{
struct aeona_heal_t;
struct aeona_spell_t;

struct melee_t;
}  // namespace actions

class aeona_td_t : public fs_player_td_t
{
public:
  struct dots_t
  {
    dot_t* echoes_of_ruin;
    dot_t* entropys_claim;
    dot_t* entropic_burst;
    dot_t* erasure;
  } dots;

  struct
  {
    buff_t* chrono_bind;
    buff_t* unfolding_doom;
  } debuffs;

  aeona_td_t( player_t* target, aeona_t* source );
};

struct aeona_buff_t : public fs_player_buff_t
{
  aeona_buff_t( player_t* p, util::string_view name ) : fs_player_buff_t( p, name )
  {
  }

  aeona_t* p()
  {
    return debug_cast<aeona_t*>( player );
  }

  const aeona_t* p() const
  {
    return debug_cast<const aeona_t*>( player );
  }
};

class aeona_t : public fellowship::fs_player_t
{
public:
  struct actions_t
  {
    action_t* entropic_burst;
    action_t* erasure;
  } actions;

  struct buffs_t
  {
    buff_t* fleeting_hour;
    buff_t* hastening_doom;
    buff_t* hastening_dooms;
    buff_t* chrona_tap;
    buff_t* epoch_break;
    buff_t* clocks_ticking;
    buff_t* uchronia;
    buff_t* quickening;
    buff_t* continuum_shift;
    buff_t* lonesome_song;
  } buffs;

  struct cooldowns_t
  {
    cooldown_t* fleeting_hour;
    cooldown_t* temporal_barrage;
  } cooldowns;

  struct gains_t
  {
    gain_t* spirit_procs;
    gain_t* fleeting_hour;
    gain_t* chrona_tap;
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
    double spirit_proc_resource        = 30;

    timespan_t time_shard_cast_time = 1.5_s;
    double time_shard_sp_coeff      = 1.06;
    double time_shard_resource      = 4;
    double time_shard_resource_crit = 6;
    double time_shard_mana_cost     = 12;

    double echoes_of_ruin_sp_coeff           = 0.3;
    timespan_t echoes_of_ruin_dot_duration   = 18_s;
    timespan_t echoes_of_ruin_period         = 2_s;
    double echoes_of_ruin_tick_sp_coeff      = 0.3;
    double echoes_of_ruin_tick_resource      = 0;
    double echoes_of_ruin_tick_resource_crit = 0;
    double echoes_of_ruin_mana_cost          = 33;
    int echoes_of_ruin_aoe                   = 3;

    timespan_t unfolding_doom_cast_time       = 2_s;
    double unfolding_doom_sp_coeff            = 3.6;
    double unfolding_doom_resource            = 20;
    double unfolding_doom_resource_crit       = 30;
    timespan_t unfolding_doom_cd              = 45_s;
    bool unfolding_doom_cd_hasted             = false;
    timespan_t unfolding_doom_debuff_duration = 20_s;
    double unfolding_doom_debuff_amp          = 0.2;
    double unfolding_doom_mana_cost           = 132;

    timespan_t flash_revision_cast_time   = 1.5_s;
    double flash_revision_sp_coeff        = 16.16;
    double flash_revision_past_taken_heal = 0.0;
    double flash_revision_mana_cost       = 88;
    double flash_revision_resource        = 6;
    double flash_revision_resource_crit   = 9;
    double flash_revision_heal_to_stagger = 0.5;

    timespan_t temporal_barrage_channel_duration = 2_s;
    timespan_t temporal_barrage_channel_period   = 0.5_s;
    double temporal_barrage_dmg_sp_coeff         = 2.28;
    double temporal_barrage_heal_sp_coeff        = 8.7;
    double temporal_barrage_mana_cost            = 92;
    timespan_t temporal_barrage_cd               = 12_s;
    double temporal_barrage_tick_resource        = 4;
    double temporal_barrage_tick_resource_crit   = 6;
    bool temporal_barrage_cd_hasted              = true;

    timespan_t time_slip_cd  = 15_s;
    bool time_slip_cd_hasted = false;

    timespan_t epoch_break_duration = 6_s;
    double epoch_break_cd_recovery  = 0.001;
    
    timespan_t chrono_bind_cd       = 300_s;
    timespan_t chrono_bind_duration = 7_s;
    double chrono_bind_mana         = 80;
    double chrono_bind_max_targets  = 8;

    timespan_t entropys_calm_cast_time      = 1.5_s;
    timespan_t entropys_calm_dot_duration   = 6_s;
    timespan_t entropys_calm_dot_period     = 1.5_s;
    double entropys_calm_tick_sp_coeff      = 2.225;
    timespan_t entropys_calm_cd             = 20_s;
    bool entropys_calm_cd_hasted            = false;
    double entropys_calm_tick_resource      = 4;
    double entropys_calm_tick_resource_crit = 6;
    double entropys_calm_mana_cost          = 94;

    timespan_t fleeting_hour_duration = 15_s;
    double fleeting_hour_mana         = 104;
    timespan_t fleeting_hour_cd       = 20_s;
    bool fleeting_hour_cd_hasted      = false;
    double fleeting_hour_cdr          = 1.5;

    double oblivion_resource_cost = 30;
    double oblivion_sp_coeff      = 7.896;
  } spell_const;

  enum aeona_talents_t : unsigned long long
  {
    NONE               = 0ULL,
    ENTROPIC_BURST     = 1ULL << 0,
    ERASURE            = 1ULL << 1,
    KIND_REWIND        = 1ULL << 2,
    CHRONA_TAP         = 1ULL << 3,
    QUICKENING         = 1ULL << 4,
    UCHRONIA           = 1ULL << 5,
    HASTENING_DOOM     = 1ULL << 6,
    SYNCHRONICITY      = 1ULL << 7,
    TEMPORAL_SHIFT     = 1ULL << 8,
    SURGING_CHRONA     = 1ULL << 9,
    MAGIC_WARD         = 1ULL << 10,
    RESONANT_FATE      = 1ULL << 11,
    PARADOXICAL_TWIST  = 1ULL << 12,
    OBLIVIONS_EMBRACE  = 1ULL << 13,
    CONTINUUM_SHIFT    = 1ULL << 14,
    ECHOES_OF_DIVINITY = 1ULL << 15,
    SPIRITED_FORTITUDE = 1ULL << 16,
    VESTIGE_OF_TRUTH   = 1ULL << 17,
    MAX                = 1ULL << 18
  };

static constexpr std::string_view talent_name_formatted( aeona_talents_t t )
  {
    switch ( t )
    {
      case aeona_talents_t::ENTROPIC_BURST:
        return "Entropic Burst";
      case aeona_talents_t::ERASURE:
        return "Erasure";
      case aeona_talents_t::KIND_REWIND:
        return "Kind Rewind";
      case aeona_talents_t::CHRONA_TAP:
        return "Chrona Tap";
      case aeona_talents_t::QUICKENING:
        return "Quickening";
      case aeona_talents_t::UCHRONIA:
        return "Uchronia";
      case aeona_talents_t::HASTENING_DOOM:
        return "Hastening Doom";
      case aeona_talents_t::SYNCHRONICITY:
        return "Synchronicity";
      case aeona_talents_t::TEMPORAL_SHIFT:
        return "Temporal Shift";
      case aeona_talents_t::SURGING_CHRONA:
        return "Surging Chrona";
      case aeona_talents_t::MAGIC_WARD:
        return "Magic Ward";
      case aeona_talents_t::RESONANT_FATE:
        return "Resonant Fate";
      case aeona_talents_t::PARADOXICAL_TWIST:
        return "Paradoxical Twist";
      case aeona_talents_t::OBLIVIONS_EMBRACE:
        return "Oblivion's Embrace";
      case aeona_talents_t::CONTINUUM_SHIFT:
        return "Continuum Shift";
      case aeona_talents_t::ECHOES_OF_DIVINITY:
        return "Echoes of Divinity";
      case aeona_talents_t::SPIRITED_FORTITUDE:
        return "Spirited Fortitude";
      case aeona_talents_t::VESTIGE_OF_TRUTH:
        return "Vestige of Truth";
      default:
        return "Unknown Talent";
    }
  }

  static constexpr std::string_view talent_name( aeona_talents_t t )
  {
    switch ( t )
    {
      case aeona_talents_t::ENTROPIC_BURST:
        return "entropic_burst";
      case aeona_talents_t::ERASURE:
        return "erasure";
      case aeona_talents_t::KIND_REWIND:
        return "kind_rewind";
      case aeona_talents_t::CHRONA_TAP:
        return "chrona_tap";
      case aeona_talents_t::QUICKENING:
        return "quickening";
      case aeona_talents_t::UCHRONIA:
        return "uchronia";
      case aeona_talents_t::HASTENING_DOOM:
        return "hastening_doom";
      case aeona_talents_t::SYNCHRONICITY:
        return "synchronicity";
      case aeona_talents_t::TEMPORAL_SHIFT:
        return "temporal_shift";
      case aeona_talents_t::SURGING_CHRONA:
        return "surging_chrona";
      case aeona_talents_t::MAGIC_WARD:
        return "magic_ward";
      case aeona_talents_t::RESONANT_FATE:
        return "resonant_fate";
      case aeona_talents_t::PARADOXICAL_TWIST:
        return "paradoxical_twist";
      case aeona_talents_t::OBLIVIONS_EMBRACE:
        return "oblivions_embrace";
      case aeona_talents_t::CONTINUUM_SHIFT:
        return "continuum_shift";
      case aeona_talents_t::ECHOES_OF_DIVINITY:
        return "echoes_of_divinity";
      case aeona_talents_t::SPIRITED_FORTITUDE:
        return "spirited_fortitude";
      case aeona_talents_t::VESTIGE_OF_TRUTH:
        return "vestige_of_truth";
      default:
        return "unknown_talent";
    }
  }

  struct talents_t
  {
    double entropic_burst_tick_coeff         = 0.35;
    timespan_t entropic_burstc_duration      = 9_s;
    timespan_t entropic_burst_tick_period    = 1.5_s;
    int entropic_burst_target_falloff        = 5;


    double erasure_multiplier = 0.2;
    timespan_t erasure_dot_duration = 8_s;
    timespan_t erasure_tick_interval = 2_s;

    timespan_t time_rewind_cdr = 1_s;

    timespan_t chrona_tap_duration = 9_s;
    int chrona_tap_max_stacks      = 5;
    double chrona_tap_mana_recovery_pct = 0.025;

    double quickening_chance = 0.2;
    int quickening_max_stacks = 2;

    int uchronia_required_spenders = 3;

    double hastening_doom_haste = 0.2;



    double synchronicity_threshold    = 0.5;
    double synchronicity_amp          = 0.15;
    double synchronicity_resource_amp = 0.15;

    timespan_t temporal_shift_extension = 0.3_s;
    timespan_t temporal_shift_cdr       = 0.3_s;

    double surging_chrona_resource = 60.0;

    // Resonant Fate

    double paradoxical_twist_amp = 0.2;
    int paradoxical_twist_cleave_targets = 2;
    double paradoxical_twist_cleave = 0.3;
    double paradoxical_twist_stagger_cleanse = 0.05;

    double oblivions_embrace_crit_dmg = 0.2;

    double continuum_shift_time_shard_cast_time_mul = 2;
    double continuum_shift_time_shard_dmg_mul       = 10;
    double continuum_shift_time_shard_resource_mul  = 10;
    int continuum_shift_echoes_of_ruin_targets      = 20;

    // Echoes of Divinity
    // Vestige of Truth
  } talents;

  struct legendary_t
  {
    bool mass_entropy                = false;
    int mass_entropy_charges         = 2;
    timespan_t mass_entropy_duration = 2_s;

    bool chrono_trigger                            = false;
    double chrono_trigger_cd_multiplier            = 0.66667;
    double chrono_trigger_mana_multiplier          = 0.66667;
    double chrono_trigger_duration_multiplier      = 0.75;
    double chrono_trigger_tick_interval_multiplier = 0.8;
    double chrono_trigger_time_shard_crit_chance   = 1.0;

    bool lonesome_song                = false;
    double lonesome_song_cdr_mul      = 2.0;
    timespan_t lonesome_song_duration = 1_s;
  } legendary;

  struct options_t
  {
    double max_mana_multiplier = 1.0;
  } options;

  target_specific_t<aeona_td_t> target_data;

  const aeona_td_t* find_target_data( const player_t* target ) const override
  {
    return target_data[ target ];
  }

  aeona_td_t* get_target_data( player_t* target ) const override
  {
    aeona_td_t*& td = target_data[ target ];
    if ( !td )
    {
      td = new aeona_td_t( target, const_cast<aeona_t*>( this ) );
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

  double composite_action_ta_multiplier( const action_state_t* s ) const override
  {
    auto m = fs_player_t::composite_action_ta_multiplier( s );

    if ( talents_enabled( aeona_t::SYNCHRONICITY ) && s->action->secondary_costs[ RESOURCE_CHRONA ] < 1 &&
         resources.pct( RESOURCE_CHRONA ) >= talents.synchronicity_threshold )
    {
      m *= 1 + talents.synchronicity_amp;
    }

    return m;
  }

  double composite_action_da_multiplier( const action_state_t* s ) const override
  {
    auto m = fs_player_t::composite_action_da_multiplier( s );

    if ( talents_enabled( aeona_t::SYNCHRONICITY ) && s->action->secondary_costs[ RESOURCE_CHRONA ] < 1 &&
         resources.pct( RESOURCE_CHRONA ) >= talents.synchronicity_threshold )
    {
      m *= 1 + talents.synchronicity_amp;
    }

    return m;
  }

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
    return resources.current[ RESOURCE_CHRONA ];
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

  aeona_t( sim_t* sim, util::string_view name, race_e r = RACE_NONE )
    : fs_player_t( sim, name, r, AEONA ), target_data()
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
struct aeona_action_state_t : public action_state_t
{
private:
  T_ACTION* action;

public:
  double temporal_pct;
  aeona_action_state_t( action_t* action, player_t* target )
    : action_state_t( action, target ), action( dynamic_cast<T_ACTION*>( action ) )
  {
  }

  aeona_t* p() const
  {
    return debug_cast<aeona_t*>( action->player );
  }

  aeona_t* p()
  {
    return debug_cast<aeona_t*>( action->player );
  }

  void initialize() override
  {
    action_state_t::initialize();
    temporal_pct     = 0.0;
  }

  std::ostringstream& debug_str( std::ostringstream& s ) override
  {
    // action_state_t::debug_str( s ) << " base_cp=" << base_cp << " total_cp=" << total_cp;
    return action_state_t::debug_str( s );
  }

  void copy_state( const action_state_t* s )
  {
    action_state_t::copy_state( s );
    const aeona_action_state_t* rs = debug_cast<const aeona_action_state_t*>( s );
    temporal_pct                  = rs->temporal_pct;
  }

  T_ACTION* get_action() const
  {
    return action;
  }
};

template <typename Base>
class aeona_action_t : public Base
{
protected:
  /// typedef for aeona_action_t<action_base_t>
  using base_t = aeona_action_t<Base>;

private:
  /// typedef for the templated action type, eg. spell_t, attack_t, heal_t
  using ab = Base;

public:
  secondary_trigger secondary_trigger_type;

  // Init =====================================================================
  aeona_action_t( util::string_view n, aeona_t* p, util::string_view options = {} )
    : ab( n, p, options ), secondary_trigger_type( secondary_trigger::NONE )
  {
    ab::may_crit = ab::tick_may_crit = true;
    ab::school                       = SCHOOL_ARCANE;

    // aeona_t sets base and min GCD to 1.5_s hasted
    ab::gcd_type = gcd_haste_type::SPELL_HASTE;
  }

  void init() override
  {
    ab::init();
  }

  // Type Wrappers ============================================================

  static const aeona_action_state_t<base_t>* cast_state( const action_state_t* st )
  {
    return debug_cast<const aeona_action_state_t<base_t>*>( st );
  }

  static aeona_action_state_t<base_t>* cast_state( action_state_t* st )
  {
    return debug_cast<aeona_action_state_t<base_t>*>( st );
  }

  aeona_t* p()
  {
    return debug_cast<aeona_t*>( ab::player );
  }

  const aeona_t* p() const
  {
    return debug_cast<const aeona_t*>( ab::player );
  }

  aeona_td_t* td( player_t* t ) const
  {
    return p()->get_target_data( t );
  }

  // Action State =============================================================

  action_state_t* new_state() override
  {
    return new aeona_action_state_t<base_t>( this, ab::target );
  }

  void update_state( action_state_t* state, unsigned flags, result_amount_type rt ) override
  {
    ab::update_state( state, flags, rt );
  }

  void snapshot_state( action_state_t* state, result_amount_type rt ) override
  {
    ab::snapshot_state( state, rt );
    cast_state( state )->temporal_pct =
        p()->buffs.epoch_break->check() || p()->buffs.uchronia->at_max_stacks() ? 1.0 : p()->resources.pct( RESOURCE_CHRONA );
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
  virtual double generate_chrona() const
  {
    double cp = 0;

    if ( ab::energize_type != action_energize::NONE && ab::energize_resource == RESOURCE_CHRONA )
    {
      cp += ab::energize_amount;
    }

    return cp;
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

    if ( ab::current_resource() == RESOURCE_CHRONA )
    {
      if ( p()->talents_enabled( aeona_t::CHRONA_TAP ) )
      {
        p()->buffs.chrona_tap->trigger();
      }

      if ( p()->buffs.uchronia->at_max_stacks() )
      {
        p()->buffs.uchronia->expire();
        return;
      }

      if ( p()->talents_enabled( aeona_t::UCHRONIA ) )
      {
        p()->buffs.uchronia->trigger();
      }
    }

    ab::consume_resource();

    spend_resource_costs( ab::execute_state );
  }

  void execute() override
  {
    ab::execute();
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
    if ( util::str_compare_ci( name, "chrona_gain" ) )
    {
      return make_mem_fn_expr( "chrona_gain", *this, &base_t::generate_chrona );
    }

    return ab::create_expression( name );
  }
};

struct aeona_heal_t : public aeona_action_t<fellowship::actions::fs_player_action_t<heal_t>>
{
  double chrona_on_hit;
  double chrona_on_crit;
  double chrona_on_tick;
  double chrona_on_tick_crit;

  aeona_heal_t( util::string_view n, aeona_t* p, util::string_view o = {} )
    : base_t( n, p, o ),
      chrona_on_hit( 0 ),
      chrona_on_crit( 0 ),
      chrona_on_tick( 0 ),
      chrona_on_tick_crit( 0 )
  {
    harmful = false;
    set_target( p );
    resource_current = RESOURCE_MANA;
  }

  virtual double chrona_multiplier( const action_state_t* s ) const
  {
    if ( p()->talent_enabled( aeona_t::SYNCHRONICITY ) &&
         p()->resources.pct( RESOURCE_CHRONA ) < p()->talents.synchronicity_threshold )
    {
      return 1.0 + p()->talents.synchronicity_resource_amp;
    }

    return 1.0;
  }

  void impact( action_state_t* s ) override
  {
    base_t::impact( s );

    if ( result_is_hit( s->result ) )
    {
      if ( p()->rng().roll( p()->cache.spell_crit_chance() ) )
      {
        p()->resource_gain( RESOURCE_CHRONA, chrona_on_crit * chrona_multiplier( s ), gain, this );
      }
      else
      {
        p()->resource_gain( RESOURCE_CHRONA, chrona_on_hit * chrona_multiplier( s ), gain, this );
      }
    }
  }

  void tick( dot_t* d ) override
  {
    base_t::tick( d );

    if ( result_is_hit( d->state->result ) )
    {
      if ( p()->rng().roll( p()->cache.spell_crit_chance() ) )
      {
        p()->resource_gain( RESOURCE_CHRONA, chrona_on_tick_crit * chrona_multiplier( d->state ), gain, this );
      }
      else
      {
        p()->resource_gain( RESOURCE_CHRONA, chrona_on_tick * chrona_multiplier( d->state ), gain, this );
      }
    }
  }
};

struct aeona_spell_t : public aeona_action_t<fellowship::actions::fs_player_action_t<spell_t>>
{
  double chrona_on_hit;
  double chrona_on_crit;
  double chrona_on_tick;
  double chrona_on_tick_crit;

  aeona_spell_t( util::string_view n, aeona_t* p, util::string_view o = {} )
    : base_t( n, p, o ), chrona_on_hit( 0 ), chrona_on_crit( 0 ), chrona_on_tick( 0 ), chrona_on_tick_crit( 0 )
  {
    resource_current = RESOURCE_MANA;
  }

  virtual double chrona_multiplier( const action_state_t* s ) const
  {
    if ( p()->talent_enabled( aeona_t::SYNCHRONICITY ) &&
         p()->resources.pct( RESOURCE_CHRONA ) < p()->talents.synchronicity_threshold )
    {
      return 1.0 + p()->talents.synchronicity_resource_amp;
    }

    return 1.0;
  }

  void impact( action_state_t* s ) override
  {
    base_t::impact( s );

    if ( result_is_hit( s->result ) )
    {
      if ( s->result == RESULT_HIT && chrona_on_hit > 0 )
      {
        p()->resource_gain( RESOURCE_CHRONA, chrona_on_hit * chrona_multiplier( s ), gain, this );
      }
      if ( s->result == RESULT_CRIT && chrona_on_crit > 0 )
      {
        p()->resource_gain( RESOURCE_CHRONA, chrona_on_crit * chrona_multiplier( s ), gain, this );
      }
    }
  }

  void tick( dot_t* d ) override
  {
    base_t::tick( d );

    if ( result_is_hit( d->state->result ) )
    {
      if ( d->state->result == RESULT_HIT && chrona_on_tick > 0 )
      {
        p()->resource_gain( RESOURCE_CHRONA, chrona_on_tick * chrona_multiplier( d->state ), gain, this );
      }
      if ( d->state->result == RESULT_CRIT && chrona_on_tick_crit > 0 )
      {
        p()->resource_gain( RESOURCE_CHRONA, chrona_on_tick_crit * chrona_multiplier( d->state ), gain, this );
      }
    }
  }
};

struct time_shard_t : public aeona_spell_t
{
  time_shard_t( util::string_view name, aeona_t* p, util::string_view options_str = {} )
    : aeona_spell_t( name, p, options_str )
  {
    id = 2;

    spell_power_mod.direct = p->spell_const.time_shard_sp_coeff;

    name_str_reporting = "Time Shard";

    base_execute_time = p->spell_const.time_shard_cast_time;

    chrona_on_hit  = p->spell_const.time_shard_resource;
    chrona_on_crit = p->spell_const.time_shard_resource_crit;

    base_costs[ RESOURCE_MANA ] = p->spell_const.time_shard_mana_cost;

    parse_options( options_str );
  }

  double chrona_multiplier( const action_state_t* s ) const override
  {
    double m = aeona_spell_t::chrona_multiplier( s );

    if ( p()->buffs.continuum_shift->check() )
    {
      m *= p()->talents.continuum_shift_time_shard_resource_mul;
    }
    return m;
  }

  double composite_target_crit_chance( player_t* target ) const override
  {
    double c = aeona_spell_t::composite_target_crit_chance( target );

    if ( p()->legendary.chrono_trigger && p()->get_target_data( target )->debuffs.unfolding_doom->check() )
    {
      c += p()->legendary.chrono_trigger_time_shard_crit_chance;
    }

    return c;
  }

  double execute_time_pct_multiplier() const override
  {
    auto m = aeona_spell_t::execute_time_pct_multiplier();

    if ( p()->buffs.continuum_shift->check() )
    {
      m *= p()->talents.continuum_shift_time_shard_cast_time_mul;
    }

    return m;
  }

  double composite_da_multiplier( const action_state_t* s ) const override
  {
    double m = aeona_spell_t::composite_da_multiplier( s );
    if ( p()->buffs.continuum_shift->check() )
    {
      m *= p()->talents.continuum_shift_time_shard_dmg_mul;
    }
    return m;
  }

  void impact( action_state_t* s ) override
  {
    aeona_spell_t::impact( s );
    if ( p()->talents_enabled( aeona_t::KIND_REWIND ) &&
         p()->get_target_data( s->target )->dots.echoes_of_ruin->is_ticking() )
    {
      p()->cooldowns.temporal_barrage->adjust( -p()->talents.time_rewind_cdr, false );
    }
  }

  void execute() override
  {
    aeona_spell_t::execute();

    if ( p()->talents_enabled( aeona_t::QUICKENING ) && rng().roll( p()->talents.quickening_chance ) )
    {
      p()->buffs.quickening->trigger();
    }

    p()->buffs.continuum_shift->expire();
  }
};

struct echoes_of_ruin_t : public aeona_spell_t
{
  echoes_of_ruin_t( aeona_t* p, util::string_view options_str = {} )
    : aeona_spell_t( "echoes_of_ruin", p, options_str )
  {
    id                 = 3;
    name_str_reporting = "Echoes of Ruin";

    spell_power_mod.direct = p->spell_const.echoes_of_ruin_sp_coeff;
    spell_power_mod.tick   = p->spell_const.echoes_of_ruin_tick_sp_coeff;
    dot_duration           = p->spell_const.echoes_of_ruin_dot_duration;
    base_tick_time         = p->spell_const.echoes_of_ruin_period;
    dot_allow_partial_tick = true;
    hasted_ticks           = true;

    dot_behavior = DOT_REFRESH_PANDEMIC;

    base_execute_time = 0_s;

    chrona_on_tick      = p->spell_const.echoes_of_ruin_tick_resource;
    chrona_on_tick_crit = p->spell_const.echoes_of_ruin_tick_resource_crit;

    base_costs[ RESOURCE_MANA ] = p->spell_const.echoes_of_ruin_mana_cost;

    parse_options( options_str );

    aoe = p->spell_const.echoes_of_ruin_aoe;
  }

  int n_targets() const override
  {
    return p()->buffs.continuum_shift->check() ? p()->talents.continuum_shift_echoes_of_ruin_targets
                                               : aeona_spell_t::n_targets();
  }

  double tick_time_pct_multiplier( const action_state_t* s ) const override
  {
    double m = aeona_spell_t::tick_time_pct_multiplier( s );

    if ( p()->legendary.chrono_trigger && p()->get_target_data( target )->debuffs.unfolding_doom->check() )
    {
      m *= p()->legendary.chrono_trigger_tick_interval_multiplier;
    }

    return m;
  }

  void execute() override
  {
    if ( aoe > 0 && target_list().size() > 1 )
    {
      auto partition = std::partition( target_list().begin() + 1, target_list().end(), [ this ]( player_t* a ) {
        return !p()->get_target_data( a )->dots.echoes_of_ruin->is_ticking();
      } );

      std::sort( target_list().begin() + 1, partition,
                 []( player_t* a, player_t* b ) { return a->current_health() > b->current_health(); } );

      std::sort( partition, target_list().end(),
                 []( player_t* a, player_t* b ) { return a->current_health() > b->current_health(); } );
    }
    base_t::execute();

    p()->buffs.continuum_shift->expire();
  }
};

struct entropys_claim_t : public aeona_spell_t
{
  entropys_claim_t( aeona_t* p, util::string_view options_str = {} ) : aeona_spell_t( "entropys_claim", p, options_str )
  {
    id                     = 4;
    name_str_reporting     = "Entropys Claim";
    spell_power_mod.tick   = p->spell_const.entropys_calm_tick_sp_coeff;
    dot_duration           = p->spell_const.entropys_calm_dot_duration;
    base_tick_time         = p->spell_const.entropys_calm_dot_period;
    dot_allow_partial_tick = true;
    hasted_ticks           = true;

    dot_behavior = DOT_REFRESH_PANDEMIC;

    base_execute_time = p->spell_const.entropys_calm_cast_time;

    cooldown->duration = p->spell_const.entropys_calm_cd;
    cooldown->hasted   = p->spell_const.entropys_calm_cd_hasted;
    cooldown->charges  = 1;

    chrona_on_tick      = p->spell_const.entropys_calm_tick_resource;
    chrona_on_tick_crit = p->spell_const.entropys_calm_tick_resource_crit;

    base_costs[ RESOURCE_MANA ] = p->spell_const.entropys_calm_mana_cost;

    parse_options( options_str );

    if ( p->legendary.mass_entropy )
    {
      cooldown->charges = p->legendary.mass_entropy_charges;
      dot_duration += p->legendary.mass_entropy_duration;
    }
  }

  double tick_time_pct_multiplier( const action_state_t* s ) const override
  {
    double m = aeona_spell_t::tick_time_pct_multiplier( s );

    if ( p()->legendary.chrono_trigger && p()->get_target_data( target )->debuffs.unfolding_doom->check() )
    {
      m *= p()->legendary.chrono_trigger_tick_interval_multiplier;
    }

    return m;
  }

  void last_tick( dot_t* d ) override
  {
    aeona_spell_t::last_tick( d );

    if ( p()->talents_enabled( aeona_t::ENTROPIC_BURST ) )
    {
      player_t* target = d->target;

      if ( target->is_sleeping() )
      {
        for ( auto player : sim->target_non_sleeping_list )
        {
          if ( player->is_sleeping() )
            continue;

          target = player;
          break;
        }
      }

      if ( target->is_sleeping() )
        return;

      p()->actions.entropic_burst->execute_on_target( target );
    }
  }

  void execute() override
  {
    aeona_spell_t::execute();
    
    p()->buffs.continuum_shift->expire();
  }
};

struct unfolding_doom_t : public aeona_spell_t
{
  unfolding_doom_t( aeona_t* p, util::string_view options_str = {} ) : aeona_spell_t( "unfolding_doom", p, options_str )
  {
    id                     = 5;
    name_str_reporting     = "Unfolding Doom";
    spell_power_mod.direct = p->spell_const.unfolding_doom_sp_coeff;
    base_execute_time      = p->spell_const.unfolding_doom_cast_time;

    cooldown->duration = p->spell_const.unfolding_doom_cd;
    cooldown->hasted   = p->spell_const.unfolding_doom_cd_hasted;
    cooldown->charges  = 1;

    chrona_on_hit  = p->spell_const.unfolding_doom_resource;
    chrona_on_crit = p->spell_const.unfolding_doom_resource_crit;

    base_costs[ RESOURCE_MANA ] = p->spell_const.unfolding_doom_mana_cost;

    if ( p->legendary.chrono_trigger )
    {
      cooldown->duration *= p->legendary.chrono_trigger_cd_multiplier;
      base_costs[ RESOURCE_MANA ] *= p->legendary.chrono_trigger_mana_multiplier;
    }

    parse_options( options_str );
  }

  void impact( action_state_t* s ) override
  {
    base_t::impact( s );

    p()->get_target_data( s->target )->debuffs.unfolding_doom->trigger();
  }

  void execute() override
  {
    base_t::execute();

    if ( p()->talents_enabled( aeona_t::CONTINUUM_SHIFT ) )
    {
      p()->buffs.continuum_shift->trigger();
    }
  }
};

struct temporal_barrage_t : public aeona_spell_t
{
    struct temporal_barrage_projectile_t : public aeona_spell_t
  {
    temporal_barrage_projectile_t( aeona_t* p, util::string_view options_str = {} )
      : aeona_spell_t( "temporal_barrage_projectile", p, options_str )
    {
      id = 6;

      background = true;

      spell_power_mod.direct = p->spell_const.temporal_barrage_dmg_sp_coeff;
      name_str_reporting     = "Temporal Barrage";

      chrona_on_hit  = p->spell_const.temporal_barrage_tick_resource;
      chrona_on_crit = p->spell_const.temporal_barrage_tick_resource_crit;

      parse_options( options_str );
      
      aoe = 1;
    }

    int n_targets() const override
    {
      if ( p()->talents_enabled( aeona_t::PARADOXICAL_TWIST ) && p()->buffs.fleeting_hour->check() )
      {
        return 1 + p()->talents.paradoxical_twist_cleave_targets;
      }
      return 1;
    }

    double chrona_multiplier( const action_state_t* s ) const override
    {
      double m = aeona_spell_t::chrona_multiplier( s );

      if ( s->chain_target > 0 )
      {
        m *= p()->talents.paradoxical_twist_cleave;
      }

      return m;
    }

    double composite_da_multiplier( const action_state_t* s ) const override
    {
      double m = aeona_spell_t::composite_da_multiplier( s );

      if ( p()->talents_enabled( aeona_t::PARADOXICAL_TWIST ) && s->chain_target == 0 &&
           p()->buffs.fleeting_hour->check() )
      {
        m *= 1.0 + p()->talents.paradoxical_twist_amp;
      }

      if ( s->chain_target > 0 )
      {
        m *= p()->talents.paradoxical_twist_cleave;
      }

      return m;
    }

    void execute() override
    {
      aeona_spell_t::execute();

      if ( p()->talents_enabled( aeona_t::TEMPORAL_SHIFT ) )
      {
        if ( p()->buffs.fleeting_hour->check() )
        {
          p()->buffs.fleeting_hour->extend_duration( p(), p()->talents.temporal_shift_extension );
        }
        else
        {
          p()->cooldowns.fleeting_hour->adjust( -p()->talents.temporal_shift_cdr, false );
        }
      }
    }
  };

  temporal_barrage_t( aeona_t* p, util::string_view options_str = {} ) : aeona_spell_t( "temporal_barrage", p, options_str )
  {
    id = 6;

    name_str_reporting = "Temporal Barrage";

    dot_duration        = p->spell_const.temporal_barrage_channel_duration;
    base_tick_time      = p->spell_const.temporal_barrage_channel_period;
    hasted_ticks        = true;
    hasted_dot_duration = true;
    tick_on_application = true;
    channeled           = true;
    
    cooldown->duration = p->spell_const.temporal_barrage_cd;
    cooldown->hasted   = p->spell_const.temporal_barrage_cd_hasted;
    cooldown->charges  = 1;

    base_costs[ RESOURCE_MANA ] = p->spell_const.unfolding_doom_mana_cost;


    tick_action = new temporal_barrage_projectile_t( p, options_str );

    add_child( tick_action );

    parse_options( options_str );
  }

  void init_finished() override
  {
    base_t::init_finished();

    update_flags &= ~STATE_HASTE;
  }
};

struct fleeting_hour_t : public aeona_spell_t
{
  fleeting_hour_t( aeona_t* p, util::string_view options_str = {} ) : aeona_spell_t( "fleeting_hour", p, options_str )
  {
    id = 7;

    trigger_gcd = timespan_t::zero();

    name_str_reporting = "Fleeting Hour";

    cooldown = p->cooldowns.fleeting_hour;

    cooldown->duration = p->spell_const.fleeting_hour_cd;
    cooldown->hasted   = p->spell_const.fleeting_hour_cd_hasted;
    cooldown->charges  = 1;

    base_costs[ RESOURCE_MANA ] = p->spell_const.fleeting_hour_mana;

    parse_options( options_str );
  }

  void execute() override
  {
    aeona_spell_t::execute();
    p()->buffs.fleeting_hour->trigger();

    if ( p()->talents_enabled( aeona_t::SURGING_CHRONA ) )
    {
      p()->resource_gain( RESOURCE_CHRONA, p()->talents.surging_chrona_resource, gain, this );
    }
  }
  
  bool action_ready() override
  {
    if ( p()->buffs.fleeting_hour->check() )
      return false;

    return base_t::action_ready();
  }
};

struct epoch_break_t : public aeona_spell_t
{
  epoch_break_t( aeona_t* p, util::string_view options_str = {} ) : aeona_spell_t( "epoch_break", p, options_str )
  {
    id = 8;

    name_str_reporting = "Epoch Break";

    base_execute_time = 1.5_s;

    resource_current              = RESOURCE_SPIRIT;
    base_costs[ RESOURCE_SPIRIT ] = 100;

    parse_options( options_str );
  }

  void execute() override
  {
    aeona_spell_t::execute();
    p()->fs_buffs.spirit_of_heroism->trigger();
    p()->buffs.epoch_break->trigger();
    p()->used_ultimate();
  }
};

struct oblivion_t : public aeona_spell_t
{
  oblivion_t( aeona_t* p, util::string_view options_str = {} ) : aeona_spell_t( "oblivion", p, options_str )
  {
    id = 9;

    spell_power_mod.direct = p->spell_const.oblivion_sp_coeff;

    name_str_reporting = "Oblivion";

    base_execute_time = 0_s;

    resource_current              = RESOURCE_CHRONA;
    base_costs[ RESOURCE_CHRONA ] = p->spell_const.oblivion_resource_cost;

    parse_options( options_str );
  }

  double cost() const override
  {
    if ( p()->buffs.epoch_break->check() )
      return 0;

    return aeona_spell_t::cost();
  }

  double composite_player_critical_multiplier( const action_state_t* s ) const override
  {
    double cm = aeona_spell_t::composite_player_critical_multiplier( s );

    if ( p()->talents_enabled( aeona_t::OBLIVIONS_EMBRACE ) )
    {
      cm *= 1.0 + p()->talents.oblivions_embrace_crit_dmg;
    }

    return cm;
  }

  void impact( action_state_t* s ) override
  {
    aeona_spell_t::impact( s );
    if ( p()->talents_enabled( aeona_t::ERASURE ) )
    {
      residual_action::trigger( p()->actions.erasure, s->target, s->result_amount * p()->talents.erasure_multiplier );
    }
  }

  void execute() override
  {
    aeona_spell_t::execute();

    if ( p()->legendary.lonesome_song )
      p()->buffs.lonesome_song->trigger();
  }
};

struct erasure_t : public residual_action::residual_periodic_action_t<aeona_spell_t>
{
  erasure_t( util::string_view name, aeona_t* p ) : residual_action_t( name, p )
  {
    id = 10;

    name_str_reporting = "Erasure";

    tick_may_crit = false;

    dot_duration           = p->talents.erasure_dot_duration;
    dot_behavior           = DOT_REFRESH_DURATION;
    base_tick_time         = p->talents.erasure_tick_interval;
    hasted_ticks           = true;
    dot_allow_partial_tick = true;
  }

  void snapshot_state( action_state_t* state, result_amount_type rt ) override
  {
    spell_t::snapshot_state( state, rt );
  }

  void init() override
  {
    base_t::init();
    snapshot_flags |= STATE_HASTE;
    update_flags &= ~STATE_HASTE;
  }
};

struct entropic_burst_t : public aeona_spell_t
{
  entropic_burst_t( util::string_view name, aeona_t* p ) : aeona_spell_t( name, p )
  {
    id = 11;

    name_str_reporting = "Entropic Burst";

    aoe = -1;

    dot_max_stack = 10;

    reduced_aoe_targets = p->talents.entropic_burst_target_falloff;

    
    spell_power_mod.tick = p->talents.entropic_burst_tick_coeff;
    dot_duration           = p->talents.entropic_burstc_duration;
    dot_behavior           = DOT_REFRESH_DURATION;
    base_tick_time         = p->talents.entropic_burst_tick_period;
    hasted_ticks           = true;
    dot_allow_partial_tick = true;
  }
};

struct restore_continuity_t : public aeona_heal_t
{
  restore_continuity_t( aeona_t* p, util::string_view options_str = {} ) : aeona_heal_t( "restore_continuity", p, options_str )
  {
    id = 12;

    name_str_reporting = "Restore Continuity";

    aoe               = -1;
    base_execute_time = 0_s;

    resource_current              = RESOURCE_CHRONA;
    base_costs[ RESOURCE_CHRONA ] = 50;

    parse_options( options_str );
  }

  double cost() const override
  {
    if ( p()->buffs.epoch_break->check() )
      return 0;

    return aeona_heal_t::cost();
  }
};

struct amend_fate_t : public aeona_heal_t
{
  amend_fate_t( aeona_t* p, util::string_view options_str = {} )
    : aeona_heal_t( "amend_fate", p, options_str )
  {
    id = 13;

    name_str_reporting = "Amend Fate";

    aoe               = -1;
    base_execute_time = 0_s;

    resource_current              = RESOURCE_CHRONA;
    base_costs[ RESOURCE_CHRONA ] = 30;

    parse_options( options_str );
  }

  double cost() const override
  {
    if ( p()->buffs.epoch_break->check() )
      return 0;

    return aeona_heal_t::cost();
  }
};

}  // namespace actions

// ==========================================================================
// Rogue Targetdata Definitions
// ==========================================================================

aeona_td_t::aeona_td_t( player_t* target, aeona_t* source )
  : fellowship::fs_player_td_t( target, source ), dots(), debuffs()
{
  dots.echoes_of_ruin = target->get_dot( "echoes_of_ruin", source );
  dots.entropys_claim  = target->get_dot( "entropys_claim", source );

  dots.erasure        = target->get_dot( "erasure", source );
  dots.entropic_burst = target->get_dot( "entropic_burst", source );

  debuffs.chrono_bind = make_buff( *this, "chrono_bind" )->set_duration( source->spell_const.chrono_bind_duration );

  debuffs.unfolding_doom = make_buff( *this, "unfolding_doom" )
                               ->set_duration( source->spell_const.unfolding_doom_debuff_duration )
                               ->set_default_value( source->spell_const.unfolding_doom_debuff_amp );

  if ( source->talents_enabled( aeona_t::HASTENING_DOOM ) )
  {
    debuffs.unfolding_doom->add_stack_change_callback( [ this, source ]( buff_t* b, int old, int _new ) {
      if ( _new > old )
        source->buffs.hastening_dooms->increment( _new - old );
      else
        source->buffs.hastening_dooms->decrement( old - _new );
    } );
  }

  if ( source->legendary.chrono_trigger )
  {
    debuffs.unfolding_doom->set_duration( debuffs.unfolding_doom->base_buff_duration *
                                          source->legendary.chrono_trigger_duration_multiplier );
  }
}

// ==========================================================================
// Rogue Character Definition
// ==========================================================================

// aeona_t::composite_attribute_multiplier ==================================

double aeona_t::composite_attribute_multiplier( attribute_e a ) const
{
  double am = fs_player_t::composite_attribute_multiplier( a );

  return am;
}
// aeona_t::composite_spell_haste ==========================================

double aeona_t::composite_spell_haste() const
{
  double h = fs_player_t::composite_spell_haste();

  return h;
}

// aeona_t::composite_spell_crit_chance =========================================

double aeona_t::composite_spell_crit_chance() const
{
  double crit = fs_player_t::composite_spell_crit_chance();

  return crit;
}

// aeona_t::composite_damage_versatility ===================================

double aeona_t::composite_damage_versatility() const
{
  double cdv = fs_player_t::composite_damage_versatility();

  return cdv;
}

// aeona_t::composite_heal_versatility =====================================

double aeona_t::composite_heal_versatility() const
{
  double chv = fs_player_t::composite_heal_versatility();

  return chv;
}

// aeona_t::composite_leech ===============================================

double aeona_t::composite_leech() const
{
  double l = fs_player_t::composite_leech();

  return l;
}

// aeona_t::matching_gear_multiplier ========================================

double aeona_t::matching_gear_multiplier( attribute_e attr ) const
{
  return 0.0;
}

// aeona_t::composite_player_multiplier =====================================

double aeona_t::composite_player_multiplier( school_e school ) const
{
  double m = fs_player_t::composite_player_multiplier( school );

  return m;
}

// aeona_t::composite_player_pet_damage_multiplier ==========================

double aeona_t::composite_player_pet_damage_multiplier( const action_state_t* s, bool guardian ) const
{
  double m = fs_player_t::composite_player_pet_damage_multiplier( s, guardian );

  return m;
}

// aeona_t::composite_player_target_multiplier ==============================

double aeona_t::composite_player_target_multiplier( player_t* target, school_e school ) const
{
  double m = fs_player_t::composite_player_target_multiplier( target, school );

  const aeona_td_t* tdata = find_target_data( target );

  if ( tdata )
    m *= 1.0 + tdata->debuffs.unfolding_doom->check_value();

  return m;
}

// aeona_t::composite_player_target_crit_chance =============================

double aeona_t::composite_player_target_crit_chance( player_t* target ) const
{
  double c = fs_player_t::composite_player_target_crit_chance( target );

  return c;
}

// aeona_t::composite_player_target_armor ===================================

double aeona_t::composite_player_target_armor( player_t* target ) const
{
  return 0.0;

  double a = fs_player_t::composite_player_target_armor( target );

  return a;
}
// aeona_t::init_actions ====================================================

void aeona_t::init_action_list()
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

// aeona_t::create_action  ==================================================

action_t* aeona_t::create_action( util::string_view name, util::string_view options_str )
{
  using namespace actions;

  if ( name == "time_shard" )
    return new time_shard_t( name, this, options_str );
  if ( name == "echoes_of_ruin" )
    return new echoes_of_ruin_t( this, options_str );
  if ( name == "entropys_claim" )
    return new entropys_claim_t( this, options_str );
  if ( name == "unfolding_doom" )
    return new unfolding_doom_t( this, options_str );
  if ( name == "temporal_barrage" )
    return new temporal_barrage_t( this, options_str );
  if ( name == "fleeting_hour" )
    return new fleeting_hour_t( this, options_str );
  if ( name == "epoch_break" )
    return new epoch_break_t( this, options_str );
  if ( name == "oblivion" )
    return new oblivion_t( this, options_str );
  if ( name == "restore_continuity" )
    return new restore_continuity_t( this, options_str );
  if ( name == "amend_fate" )
    return new amend_fate_t( this, options_str );

  return fs_player_t::create_action( name, options_str );
}

// aeona_t::create_expression ===============================================

std::unique_ptr<expr_t> aeona_t::create_action_expression( action_t& action, std::string_view name_str )
{
  // auto split = util::string_split<util::string_view>( name_str, "." );

  return fs_player_t::create_action_expression( action, name_str );
}

std::unique_ptr<expr_t> aeona_t::create_expression( util::string_view name_str )
{
  auto split = util::string_split<util::string_view>( name_str, "." );

  if ( split[ 0 ] == "chrona" )
  {
    if ( split.size() == 1 )
    {
      return make_fn_expr( name_str, [ this ] { return this->current_temporal_overcharge( true ); } );
    }

    if ( split.size() == 2 && split[ 1 ] == "deficit" )
    {
      return make_fn_expr( name_str, [ this ] {
        return resources.max[ RESOURCE_CHRONA ] - this->current_temporal_overcharge( true );
      } );
    }
  }
  else if ( util::str_compare_ci( split[ 0 ], "talent" ) )
  {
    if ( split.size() == 2 )
    {
      for ( aeona_talents_t t = static_cast<aeona_talents_t>( 1U ); t < aeona_talents_t::MAX; t++ )
      {
        if ( util::str_compare_ci( split[ 1 ], talent_name( t ) ) )
        {
          return make_fn_expr( name_str, std::bind( std::mem_fn( &aeona_t::talents_enabled ), this, t ) );
        }
      }
    }
  }
  else if ( util::str_compare_ci( split[ 0 ], "legendary" ) )
  {
    if ( split.size() == 2 )
    {
      if ( util::str_compare_ci( split[ 1 ], "chrono_trigger" ) )
        return make_ref_expr( name_str, legendary.chrono_trigger );
      if ( util::str_compare_ci( split[ 1 ], "lonesome_song" ) )
        return make_ref_expr( name_str, legendary.lonesome_song );
      if ( util::str_compare_ci( split[ 1 ], "mass_entropy" ) )
        return make_ref_expr( name_str, legendary.mass_entropy );
    }
  }

  return fs_player_t::create_expression( name_str );
}

std::unique_ptr<expr_t> aeona_t::create_resource_expression( util::string_view name_str )
{
  return fs_player_t::create_resource_expression( name_str );
}

double aeona_t::resource_regen_per_second( resource_e r ) const
{
  double reg = fs_player_t::resource_regen_per_second( r );

  return reg;
}

// aeona_t::init_base =======================================================

void aeona_t::init_base_stats()
{
  if ( base.distance < 1 )
    base.distance = 25;

  fs_player_t::init_base_stats();

  base.stats.attribute[ STAT_INTELLECT ] = 100;
  resources.base[ RESOURCE_HEALTH ]      = 1618;

  base.health_per_stamina = 47.506;

  resources.base[ RESOURCE_CHRONA ]                = 100;
  resources.base[ RESOURCE_MANA ]                  = 1440;
  resources.base_regen_per_second[ RESOURCE_MANA ] = 0.005 * resources.base[ RESOURCE_MANA ];

  resources.base_multiplier[ RESOURCE_MANA ] *= options.max_mana_multiplier;

  base_gcd = timespan_t::from_seconds( 1.5 );
  min_gcd  = timespan_t::from_seconds( 0.75 );
}

// aeona_t::init_spells =====================================================

void aeona_t::init_spells()
{
  fs_player_t::init_spells();

  // actions.auto_attack = new actions::auto_melee_attack_t( this, "" );
}

// aeona_t::init_talents ====================================================

void aeona_t::init_talents()
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

    for ( aeona_talents_t t = static_cast<aeona_talents_t>( 1U ); t < aeona_talents_t::MAX; t++ )
    {
      if ( util::str_compare_ci( talent_split[ 0 ], talent_name( t ) ) )
      {
        set_talent_points( t, ranks >= 1 );
        break;
      }
    }
  }
}

// aeona_t::init_gains ======================================================

void aeona_t::init_gains()
{
  fs_player_t::init_gains();

  gains.spirit_procs  = get_gain( "Spirit Procs" );
  gains.fleeting_hour = get_gain( "Fleeting Hour" );
  gains.chrona_tap    = get_gain( "Chrona Tap" );

}

// aeona_t::init_procs ======================================================

void aeona_t::init_procs()
{
  fs_player_t::init_procs();
}

// aeona_t::init_rng ========================================================
void aeona_t::init_rng()
{
  fs_player_t::init_rng();
}

// aeona_t::init_scaling ====================================================

void aeona_t::init_scaling()
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

// aeona_t::init_resources =================================================

void aeona_t::init_resources( bool force )
{
  fs_player_t::init_resources( force );
}

// aeona_t::init_buffs ======================================================

void aeona_t::create_buffs()
{
  fs_player_t::create_buffs();

  buffs.continuum_shift = make_buff<aeona_buff_t>( this, "continuum_shift" )
                              ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

  buffs.chrona_tap = make_buff<aeona_buff_t>( this, "chrona_tap" )
                         ->set_duration( talents.chrona_tap_duration )
                         ->set_default_value( talents.chrona_tap_mana_recovery_pct )
                         ->set_max_stack( talents.chrona_tap_max_stacks )
                         ->set_refresh_behavior( buff_refresh_behavior::DISABLED )->add_stack_change_callback( [ this ]( buff_t* b, int old, int _new ) {
                           if ( !_new && old )
                           {
                             resource_gain( RESOURCE_MANA, resources.max[RESOURCE_MANA] * old * b->default_value, gains.chrona_tap );
                           }
                         } );

  buffs.uchronia = make_buff<aeona_buff_t>( this, "uchronia" )
                       ->set_max_stack( talents.uchronia_required_spenders )
                       ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

  buffs.quickening = make_buff<aeona_buff_t>( this, "quickening" )
                         ->set_max_stack( talents.quickening_max_stacks )
                         ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

  buffs.hastening_doom = make_buff<aeona_buff_t>( this, "hastening_doom" )
                             ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT )
                             ->set_pct_buff_type( STAT_PCT_BUFF_HASTE )
                             ->set_default_value( talents.hastening_doom_haste );

  buffs.hastening_dooms = make_buff<aeona_buff_t>( this, "hastening_dooms" )
                              ->set_max_stack( 99 )
                              ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT )
                              ->add_stack_change_callback( [ this ]( buff_t* b, int old, int _new ) {
                                if ( _new && !old )
                                {
                                  buffs.hastening_doom->trigger();
                                }
                                else if ( old && !_new )
                                {
                                  buffs.hastening_doom->expire();
                                }
                              } );


  struct fleeting_hour_buff_t : aeona_buff_t
  {
    double cdr_mod;
    fleeting_hour_buff_t( aeona_t* pl )
      : aeona_buff_t( pl, "fleeting_hour" ), cdr_mod( pl->spell_const.fleeting_hour_cdr )
    {
      set_duration( pl->spell_const.fleeting_hour_duration );
      add_stack_change_callback( [ this ]( buff_t*, int, int _new ) {
        for ( auto& action : p()->action_list )
        {
          if ( _new )
          {
            action->dynamic_recharge_rate_multiplier /= cdr_mod;
            action->cooldown->adjust_recharge_multiplier();
            sim->print_debug( "Applying Fleeting Hour CDR to {}", *action );
          }
          else
          {
            action->dynamic_recharge_rate_multiplier *= cdr_mod;
            sim->print_debug( "Removing Fleeting Hour CDR to {}", *action );
          }
          if ( action->cooldown->action == action )
            action->cooldown->adjust_recharge_multiplier();
          if ( action->internal_cooldown->action == action )
            action->internal_cooldown->adjust_recharge_multiplier();
        }
        if ( !_new )
          p()->cooldowns.fleeting_hour->start( p()->cooldowns.fleeting_hour->action );
      } );
    }
  };

  buffs.fleeting_hour = make_buff<fleeting_hour_buff_t>( this );

  struct lonesome_song_t : aeona_buff_t
  {
    double cdr_mod;
    lonesome_song_t( aeona_t* pl )
      : aeona_buff_t( pl, "lonesome_song" ), cdr_mod( pl->legendary.lonesome_song_cdr_mul )
    {
      set_duration( pl->legendary.lonesome_song_duration);
      set_refresh_behavior( buff_refresh_behavior::DURATION );
      add_stack_change_callback( [ this ]( buff_t*, int, int _new ) {
        for ( auto& action : p()->action_list )
        {
          if ( _new )
          {
            action->dynamic_recharge_rate_multiplier /= cdr_mod;
            action->cooldown->adjust_recharge_multiplier();
          }
          else
          {
            action->dynamic_recharge_rate_multiplier *= cdr_mod;
          }
          if ( action->cooldown->action == action )
            action->cooldown->adjust_recharge_multiplier();
          if ( action->internal_cooldown->action == action )
            action->internal_cooldown->adjust_recharge_multiplier();
        }
      } );
    }
  };

  buffs.lonesome_song = make_buff<lonesome_song_t>( this );

  struct epoch_break_buff_t : aeona_buff_t
  {
    double cdr_mod;
    epoch_break_buff_t( aeona_t* pl )
      : aeona_buff_t( pl, "epoch_break" ), cdr_mod( pl->spell_const.epoch_break_cd_recovery )
    {
      set_duration( pl->spell_const.epoch_break_duration );
      add_stack_change_callback( [ this ]( buff_t*, int, int _new ) {
        for ( auto& action : p()->action_list )
        {
          if ( _new )
          {
            action->dynamic_recharge_rate_multiplier /= cdr_mod;
            action->cooldown->adjust_recharge_multiplier();
          }
          else
          {
            action->dynamic_recharge_rate_multiplier *= cdr_mod;
          }
          if ( action->cooldown->action == action )
            action->cooldown->adjust_recharge_multiplier();
          if ( action->internal_cooldown->action == action )
            action->internal_cooldown->adjust_recharge_multiplier();
        }
      } );
    }
  };

  buffs.epoch_break = make_buff<epoch_break_buff_t>( this );
}

// aeona_t::invalidate_cache =========================================

void aeona_t::invalidate_cache( cache_e c )
{
  fs_player_t::invalidate_cache( c );
}

void aeona_t::create_options()
{
  fs_player_t::create_options();

  add_option( opt_bool( "legendary.chrono_trigger", legendary.chrono_trigger ) );
  add_option( opt_bool( "legendary.lonesome_song", legendary.lonesome_song ) );
  add_option( opt_bool( "legendary.mass_entropy", legendary.mass_entropy ) );
  add_option( opt_float( "aeona.max_mana_multiplier", options.max_mana_multiplier ) );
}

// aeona_t::copy_from =======================================================

void aeona_t::copy_from( player_t* source )
{
  aeona_t* aeona = static_cast<aeona_t*>( source );
  fs_player_t::copy_from( source );

  talents     = aeona->talents;
  legendary   = aeona->legendary;
  options     = aeona->options;
  spell_const = aeona->spell_const;
}

// aeona_t::create_profile  =================================================

std::string aeona_t::create_profile( save_e stype )
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

// aeona_t::init_items ======================================================

void aeona_t::init_items()
{
  fs_player_t::init_items();
}

// aeona_t::init_special_effects ============================================

void aeona_t::init_special_effects()
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

// aeona_t::init_finished ===================================================

void aeona_t::init_finished()
{
  fs_player_t::init_finished();
}

void aeona_t::init_background_actions()
{
  fs_player_t::init_background_actions();

  actions.entropic_burst = new actions::entropic_burst_t( "entropic_burst", this );
  actions.erasure        = new actions::erasure_t( "erasure", this );
}

// aeona_t::reset ===========================================================

void aeona_t::reset()
{
  fs_player_t::reset();

  // for ( auto enemy : sim->target_list )
  //{
  //   get_target_data( enemy )->dots.engulfing_decrement_events.clear();
  // }
}

// aeona_t::activate ========================================================

void aeona_t::activate()
{
  fs_player_t::activate();
}

// aeona_t::arise ===========================================================

void aeona_t::arise()
{
  fs_player_t::arise();
}

// aeona_t::combat_begin ====================================================

void aeona_t::combat_begin()
{
  fs_player_t::combat_begin();
}

double aeona_t::resource_gain( resource_e resource_type, double amount, gain_t* source, action_t* action )
{
  double actual_amount = fs_player_t::resource_gain( resource_type, amount, source, action );


  return actual_amount;
}

double aeona_t::resource_loss( resource_e resource_type, double amount, gain_t* source, action_t* action )
{
  double actual_amount = fs_player_t::resource_loss( resource_type, amount, source, action );

  return actual_amount;
}

// aeona_t::non_stacking_movement_modifier ==================================

double aeona_t::non_stacking_movement_modifier() const
{
  double ms = fs_player_t::non_stacking_movement_modifier();

  return ms;
}

// aeona_t::stacking_movement_modifier===================================

double aeona_t::stacking_movement_modifier() const
{
  double ms = fs_player_t::stacking_movement_modifier();

  return ms;
}

template <typename Base>
void actions::aeona_action_t<Base>::trigger_spirit_refund( const action_state_t* state, double resource_refund )
{
  make_event( ab::sim, 200_ms, [ resource_refund, this ] {
    p()->resource_gain( RESOURCE_MANA, resource_refund, p()->gains.spirit_procs, this );
    p()->resource_gain( RESOURCE_CHRONA, p()->spell_const.spirit_proc_resource, p()->gains.spirit_procs,
                        this );
    p()->sim->print_debug( "{} actually refunded {:.0f} Mana ", *p(), resource_refund );
  } );

  p()->spirit_refund();
}

template <typename Base>
void actions::aeona_action_t<Base>::spend_resource_costs( const action_state_t* s )
{
  if ( ab::last_resource_cost <= 0 || ab::current_resource() != RESOURCE_MANA )
    return;

  if ( p()->rng().roll( p()->cache.mastery_value() ) )
  {
    p()->sim->print_debug( "{} proc'd Spirit Refund (Chance: {:.2f}%, Sprit: {:.2f}%)", *p(),
                           p()->cache.mastery_value() * 100.0, p()->cache.mastery() * 100.0 );

    trigger_spirit_refund( s, ab::last_resource_cost );
  }
}

// aeona_t::convert_hybrid_stat ==============================================

stat_e aeona_t::convert_hybrid_stat( stat_e s ) const
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

void aeona_t::analyze( sim_t& sim )
{
  fs_player_t::analyze( sim );
}

void aeona_t::create_cooldowns()
{
  cooldowns.fleeting_hour    = get_cooldown( "fleeting_hour" );
  cooldowns.temporal_barrage = get_cooldown( "temporal_barrage" );
}

class aeona_module_t : public module_t
{
public:
  aeona_module_t() : module_t( AEONA )
  {
  }

  player_t* create_player( sim_t* sim, util::string_view name, race_e r = RACE_NONE ) const override
  {
    return new aeona_t( sim, name, r );
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

}  // namespace aeona
}  // namespace fellowship

const module_t* module_t::aeona()
{
  static fellowship::aeona::aeona_module_t m;
  return &m;
}