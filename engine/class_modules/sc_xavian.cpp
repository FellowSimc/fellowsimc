#include "fs_player.hpp"
#include "util/util.hpp"

#include "simulationcraft.hpp"

namespace fellowship
{
namespace xavian
{

// Forward Declarations
class xavian_t;

enum class secondary_trigger
{
  NONE = 0U
};

namespace actions
{
struct xavian_attack_t;
struct xavian_heal_t;
struct xavian_spell_t;

struct melee_t;
}  // namespace actions

class xavian_td_t : public fs_player_td_t
{
public:
  struct dots_t
  {
    dot_t* sunburn;
    dot_t* suns_touch;
  } dots;

  struct
  {
    buff_t* solar_glare;
    buff_t* sunburn;
    buff_t* sunstruck;
    buff_t* blind;
  } debuffs;

  xavian_td_t( player_t* target, xavian_t* source );
};

struct xavian_buff_t : public fs_player_buff_t
{
  xavian_buff_t( player_t* p, util::string_view name ) : fs_player_buff_t( p, name )
  {
  }

  xavian_t* p()
  {
    return debug_cast<xavian_t*>( player );
  }

  const xavian_t* p() const
  {
    return debug_cast<const xavian_t*>( player );
  }
};

class xavian_t : public fellowship::fs_player_t
{
public:
  struct actions_t
  {
    action_t* auto_attack;
    actions::melee_t* melee_hit;
    action_t* suns_verdict;
    action_t* brilliant_flash_heal;
    action_t* vanguard_of_vengeance;
    action_t* solaris;
    action_t* decree_of_the_sun_dmg;
    action_t* decree_of_the_sun_heal;
  } actions;

  struct buffs_t
  {
    buff_t* decree_of_the_sun;
    buff_t* decree_of_the_sun_invuln;
    buff_t* decree_of_the_sun_dr;
    buff_t* omega_reprieval;
    buff_t* aura_of_solace;
    buff_t* solar_shield;
    buff_t* golden_hour;
    buff_t* vanguard_of_vengeance;
    buff_t* invictus_expertise;
    buff_t* invictus_haste;
    buff_t* lights_oath;
    buff_t* shining_halo;
    buff_t* suns_verdict;
    buff_t* hallowed_shield;
    buff_t* grossly_incandescent;
    buff_t* swift_reprieval;
  } buffs;

  struct cooldowns_t
  {
    cooldown_t* omnistrike;
    cooldown_t* solar_blades;
    cooldown_t* omega_reprieval;
    cooldown_t* blinding_slash;
    cooldown_t* solar_shield;
  } cooldowns;

  struct gains_t
  {
    gain_t* spirit_procs;
    gain_t* omega_reprieval;
  } gains;

  struct rng_objects_t
  {
    accumulated_rng_t* golden_hour;
    accumulated_rng_t* suns_verdict;
    accumulated_rng_t* rising_sun;
  } rng_objects;

  #define XAVIAN_TALENT_LIST( X )                                                \
  X( SUNS_TOUCH, "suns_touch", "Suns Touch" )                                  \
  X( GRAND_HALO, "grand_halo", "Grand Halo" )                                  \
  X( SOLAR_BURN, "solar_burn", "Solar Burn" )                                  \
  X( TEMPERED_DESCENT, "tempered_descent", "Tempered Descent" )                \
  X( LIGHTS_OATH, "lights_oath", "Lights Oath" )                               \
  X( RISING_SUN, "rising_sun", "Rising Sun" )                                  \
  X( IMPOSING_PRESENCE, "imposing_presence", "Imposing Presence" )             \
  X( CELESTIAL_FAVOR, "celestial_favor", "Celestial Favor" )                   \
  X( GLEAMING_STRIKES, "gleaming_strikes", "Gleaming Strikes" )                \
  X( SUNS_VERDICT, "suns_verdict", "Suns Verdict" )                            \
  X( MAGIC_WARD, "magic_ward", "Magic Ward" )                                  \
  X( HALLOWED_SHIELD, "hallowed_shield", "Hallowed Shield" )                   \
  X( VANGUARD_OF_VENGEANCE, "vanguard_of_vengeance", "Vanguard of Vengeance" ) \
  X( INVICTUS, "invictus", "Invictus" )                                        \
  X( GOLDEN_HOUR, "golden_hour", "Golden Hour" )                               \
  X( ZENITH_TECHNIQUE, "zenith_technique", "Zenith Technique" )                \
  X( HEALING_HALO, "healing_halo", "Healing Halo" )                            \
  X( SHIFTING_SPECTRUM, "shifting_spectrum", "Shifting Spectrum" )

  enum xavian_talent_index_t
  {
#define X( name, id, pretty ) name##_INDEX,
    XAVIAN_TALENT_LIST( X )
#undef X
        XAVIAN_TALENT_MAX
  };

  enum xavian_talents_t : unsigned long long
  {
    NONE = 0,
#define X( name, id, pretty ) name = 1ULL << name##_INDEX,
    XAVIAN_TALENT_LIST( X )
#undef X
        MAX = 1ULL << XAVIAN_TALENT_MAX
  };

  static constexpr talent_info XAVIAN_TALENTS[] = {
#define X( name, id, pretty ) { xavian_talents_t::name, id, pretty },
      XAVIAN_TALENT_LIST( X )
#undef X
  };

  constexpr std::string_view talent_name( long long t ) override
  {
    for ( const auto& talent : XAVIAN_TALENTS )
      if ( talent.flag == t )
        return talent.id;

    return "unknown_talent";
  }

  constexpr std::string_view talent_name_formatted( long long t ) override
  {
    for ( const auto& talent : XAVIAN_TALENTS )
      if ( talent.flag == t )
        return talent.pretty;

    return "Unknown Talent";
  }

  struct spell_const_t
  {
    double sunstruck_mana_refund_base      = 100;
    double sunstruck_mana_refund_per_stack = 50;
    timespan_t sunstruck_duration          = 20_s;
    int suntruck_max_stacks                = 10;

    double solaris_overheal_scaler       = 0.05;
    int solaris_target_scaling_threshold = 5;

    timespan_t auto_attack_time = 3_s;
    double auto_attack_coeff    = 0.85;

    double sun_strike_coeff                = 1.28;
    double sun_strike_resource_proc_chance = 0.15;
    double sun_strike_cleave_mod           = 0.15;
    int sun_strike_scaling_thresold        = 3;

    double blinding_slash_coeff                = 1.14;
    int blinding_slash_scaling_threshold       = 8;
    timespan_t blinding_slash_cd               = 6_s;
    double blinding_slash_resource_proc_chance = 0.15;
    int blinding_slash_max_stacks              = 2;
    double blinding_slash_parry_chance         = 0.2;
    timespan_t blinding_slash_blind_duration   = 8_s;

    double omnistrike_coeff                = 2.26;
    int omnistrike_scaling_threshold       = 8;
    timespan_t omnistrike_cooldown         = 18_s;
    double omnistrike_resource_proc_chance = 0.3;
    double omnistrike_spirit_gain          = 4;

    double solar_blades_coeff           = 1.49;
    int solar_blades_scaling_threshold  = 5;
    double solar_blades_main_target_mul = 2;
    double solar_blades_resource_chance = 1.0;
    double solar_blades_mana_cost       = 55;
    timespan_t solar_blades_cd          = 9_s;

    double shining_halo_coeff                     = 0.25;
    int shining_halo_scaling_thresold             = 12;
    timespan_t shining_halo_duration              = 12_s;
    timespan_t shining_halo_period                = 1.5_s;
    double shining_halo_mana_cost                 = 218;
    double shining_halo_dr_mul                    = 0.8;
    timespan_t shining_halo_extension_per_spender = 1.5_s;

    double brilliant_flare_coeff     = 6.95;  // Brilliant Flare
    double brilliant_flare_mana_cost = 132;

    double brilliant_flash_dmg_coeff     = 2.22;  // Brilliant Flash
    double brilliant_flash_heal_coeff    = 6.95;
    double brilliant_flash_self_heal_mul = 1.2;
    int swift_reprieval_max_stacks       = 3;

    double ruptured_dawn_coeff  = 0.78;
    int ruptured_dawn_falloff   = 8;
    timespan_t ruptured_dawn_cd = 60_s;

    double sky_crash_coeff   = 0.62;
    double sky_crash_falloff = 8;
    int sky_crash_charges    = 2;
    timespan_t sky_crash_cd  = 60_s;

    timespan_t interrupt_cd = 12_s;
    timespan_t taunt_cd     = 8_s;

    double solar_shield_coeff        = 19.165;
    timespan_t solar_shield_duration = 8_s;
    timespan_t solar_shield_cd       = 15_s;
    double solar_shield_mana_cost    = 182;
    double solar_shield_dr           = 0.8;

    int omega_reprieval_stacks             = 2;
    int omega_reprieval_max_stacks         = 3;
    timespan_t omega_reprieval_cd          = 60_s;
    double omega_reprieval_main_target_mul = 1.5;
    int omega_reprieval_falloff            = 3;
    double omega_reprieval_mana_pct        = 0.12;

    double aura_of_solace_coeff         = 0.31;
    timespan_t aura_of_solace_period    = 1.5_s;
    int aura_of_solace_falloff          = 5;
    double aura_of_solace_parry         = 0.08;
    double aura_of_solace_mana_per_tick = 10;

    double decree_of_the_sun_dmg_coeff           = 5.12;
    double decree_of_the_sun_heal_coeff          = 1.42;
    int decree_of_the_sun_falloff                = 8;
    timespan_t decree_of_the_sun_pulse_period    = 1_s;
    timespan_t decree_of_the_sun_invuln_duration = 3_s;
    timespan_t decree_of_the_sun_dr_duration     = 5_s;
    double decree_of_the_sun_dr_mul              = 0.5;

  } spell_const;

  struct talents_t
  {
    timespan_t suns_touch_duration  = 12_s;
    double suns_touch_dmg_modifier  = 0.4;
    double suns_touch_heal_modifier = 0.4;

    timespan_t solar_burn_duration            = 9_s;
    timespan_t solar_burn_period              = 1.5_s;
    double solar_burn_dmg_received_multiplier = 0.9;
    double solar_burn_dmg_mul                 = 1;

    int gleaming_strikes_extra_sunstruck_on_crit     = 1;
    double gleaming_strikes_omnistrike_sunstruck_amp = 0.25;

    double vanguard_of_vengeance_coeff             = 1.16;
    double vanguard_of_vengeance_mana_gain_pct     = 0.005;
    timespan_t vanguard_of_vengeance_buff_duration = 20_s;
    int vanguard_of_vengeance_max_stacks           = 20;
    double vanguard_of_vengeance_spirit_per_stack  = 0.005;
    double vanguard_of_vengeance_parry             = 0.1;

    double shifting_spectrum_threshold     = 0.4;
    double shifting_spectrum_dmg_taken_mul = 0.8;

    timespan_t suns_verdict_duration      = 12_s;
    int suns_verdict_max_stacks           = 2;
    double suns_verdict_chance            = 0.2;
    double suns_verdict_coeff             = 1.15;
    double suns_verdict_scaling_threshold = 8;

    double golden_hour_brilliant_flash_amp = 1.6;
    double golden_hour_proc_chance         = 0.06;
    timespan_t golden_hour_buff_duration   = 12_s;

    double rising_sun_chance = 0.5;

    double hallowed_shield_mul_per_stack = 0.01;
    int hallowed_shield_max_stacks       = 30;
    timespan_t hallowed_shield_duration  = 30_s;

    double invictus_cc_dmg               = 0.5;
    double invictus_haste_cdr            = 0.5;
    double invictus_expertise_armour_mul = 1.0;
    timespan_t invictus_cdr_spirit_proc  = 3_s;

    double grand_halo_mul          = 1.25;
    timespan_t grand_halo_duration = 6_s;

    timespan_t lights_oath_duration = 30_s;
    int lights_oath_stacks          = 8;
    double lights_oath_mul_at_max   = 2;

    timespan_t celestial_favor_cdr = 3_s;
  } talents;

  struct legendary_t
  {
    bool grossly_incandescent       = false;
    int grossly_incandescent_stacks = 5;

    bool solar_glare                 = false;
    timespan_t solar_glare_duration  = 5_s;
    double solar_glare_dmg_taken_mul = 0.8;
    double solar_glare_dmg_chance    = 0.3;
    double solar_glare_proc_added_cc = 1.0;

    bool fortress_in_the_sands                      = false;
    double fortress_in_the_sands_shield_dr_mul      = 0.6;
    timespan_t fortress_in_the_sands_cheat_death_cd = 300_s;
  } legendary;

  struct options_t
  {
  } options;

  target_specific_t<xavian_td_t> target_data;

  const xavian_td_t* find_target_data( const player_t* target ) const override
  {
    return target_data[ target ];
  }

  xavian_td_t* get_target_data( player_t* target ) const override
  {
    xavian_td_t*& td = target_data[ target ];
    if ( !td )
    {
      td = new xavian_td_t( target, const_cast<xavian_t*>( this ) );
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
  void parry_effects( action_state_t* s );

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
    return RESOURCE_MANA;
  }
  role_e primary_role() const override
  {
    return ROLE_TANK;
  }
  stat_e convert_hybrid_stat( stat_e s ) const override;

  double composite_parry( action_state_t* s ) const override;
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

  xavian_t( sim_t* sim, util::string_view name, race_e r = RACE_NONE )
    : fs_player_t( sim, name, r, XAVIAN ), target_data()
  {
    resource_regeneration = regen_type::DYNAMIC;

    create_cooldowns();
  }
};

namespace actions
{  // namespace actions

template <typename Base>
class xavian_action_t : public fellowship::actions::fs_player_action_t<Base>
{
protected:
  /// typedef for xavian_action_t<action_base_t>
  using base_t = xavian_action_t<fellowship::actions::fs_player_action_t<Base>>;

private:
  /// typedef for the templated action type, eg. spell_t, attack_t, heal_t
  using ab = fellowship::actions::fs_player_action_t<Base>;

public:
  bool _procs_golden_hour;
  double swift_reprieval_chance;

  // Init =====================================================================

  xavian_action_t( util::string_view n, xavian_t* p, util::string_view options = {} )
    : ab( n, p, options ),
      _procs_golden_hour( false ),
      swift_reprieval_chance( 0.0 )
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

  xavian_t* p()
  {
    return debug_cast<xavian_t*>( ab::player );
  }

  const xavian_t* p() const
  {
    return debug_cast<const xavian_t*>( ab::player );
  }

  xavian_td_t* td( player_t* t ) const
  {
    return p()->get_target_data( t );
  }

public:
  // Ability triggers
  void trigger_auto_attack( const action_state_t* );
  void trigger_spirit_refund( const action_state_t* );

  void consume_resource() override
  {
    ab::consume_resource();

    if ( ab::background )
      return;

    if ( ab::current_resource() == RESOURCE_MANA && ab::last_resource_cost > 0 )
    {
      if ( p()->rng().roll( p()->cache.mastery_value() ) )
      {
        trigger_spirit_refund( ab::execute_state );
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

    if ( _procs_golden_hour && !ab::background )
    {
      if ( p()->talents_enabled( xavian_t::GOLDEN_HOUR ) && p()->rng_objects.golden_hour->trigger() )
      {
        p()->cooldowns.omnistrike->reset( false, 1 );
        p()->buffs.golden_hour->trigger();
      }
    }

    if ( p()->rng().roll( swift_reprieval_chance ) )
    {
      p()->buffs.swift_reprieval->trigger();
    }
  }
};

// ==========================================================================
// Rogue Attack Classes
// ==========================================================================

struct xavian_heal_t : public xavian_action_t<heal_t>
{
protected:
  using base_t = xavian_action_t<heal_t>;

public:
  xavian_heal_t( util::string_view n, xavian_t* p, util::string_view o = {} ) : base_t( n, p, o )
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

struct xavian_spell_t : public xavian_action_t<spell_t>
{
protected:
  using base_t = xavian_action_t<spell_t>;

public:
  xavian_spell_t( util::string_view n, xavian_t* p, util::string_view o = {} ) : base_t( n, p, o )
  {
    school = SCHOOL_MAGIC;
  }
};

struct xavian_attack_t : public xavian_action_t<melee_attack_t>
{
protected:
  using base_t = xavian_action_t<melee_attack_t>;

public:
  xavian_attack_t( util::string_view n, xavian_t* p, util::string_view o = {} ) : base_t( n, p, o )
  {
    special = true;
    school  = SCHOOL_PHYSICAL;
  }
};

struct xavian_absorb_t : public xavian_action_t<absorb_t>
{
protected:
  using base_t = xavian_action_t<absorb_t>;

public:
  xavian_absorb_t( util::string_view n, xavian_t* p, util::string_view o = {} ) : base_t( n, p, o )
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

struct melee_t : public xavian_attack_t
{
  bool first;
  bool canceled;
  timespan_t prev_scheduled_time;

  melee_t( const char* name, const char* reporting_name, xavian_t* p )
    : xavian_attack_t( name, p ), first( true ), canceled( false ), prev_scheduled_time( timespan_t::zero() )
  {
    background = repeating = may_glance = may_crit = true;
    may_miss                                       = true;
    allow_class_ability_procs = not_a_proc = true;
    special                                = false;

    school             = SCHOOL_PHYSICAL;
    trigger_gcd        = timespan_t::zero();
    name_str_reporting = reporting_name;

    attack_power_mod.direct = p->spell_const.auto_attack_coeff;
  }

  double miss_chance( double /* hit */, player_t* /* target */ ) const
  {
    return 1 - 0.95 * 0.95;
  }

  void reset() override
  {
    xavian_attack_t::reset();
    first               = true;
    canceled            = false;
    prev_scheduled_time = timespan_t::zero();
  }

  timespan_t execute_time() const override
  {
    timespan_t t = xavian_attack_t::execute_time();

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

  void schedule_execute( action_state_t* state ) override
  {
    xavian_attack_t::schedule_execute( state );

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
  auto_melee_attack_t( xavian_t* p, util::string_view options_str ) : action_t( ACTION_OTHER, "auto_attack", p )
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

struct sun_strike_t : public xavian_attack_t
{
  sun_strike_t( util::string_view name, xavian_t* p, util::string_view options_str = {} )
    : xavian_attack_t( name, p, options_str )
  {
    id = 2;
    _procs_golden_hour      = true;

    school                  = SCHOOL_PHYSICAL;
    attack_power_mod.direct = p->spell_const.sun_strike_coeff;

    name_str_reporting     = "Sun Strike";
    swift_reprieval_chance = p->spell_const.sun_strike_resource_proc_chance;

    aoe                 = -1;
    reduced_aoe_targets = p->spell_const.sun_strike_scaling_thresold;
    full_amount_targets = 1;

    ability_flags |= ability_type_e::ABILITY_BASIC;
  }

  double composite_da_multiplier( const action_state_t* s ) const override
  {
    double m = xavian_attack_t::composite_da_multiplier( s );

    if ( s->chain_target > 0 )
      m *= p()->spell_const.sun_strike_cleave_mod;

    return m;
  }

  void impact( action_state_t* s ) override
  {
    xavian_attack_t::impact( s );

    if ( s->chain_target == 0 )
    {
      int debuffs = 1;

      if ( p()->talent_enabled( xavian_t::GLEAMING_STRIKES )  )
        debuffs += p()->talents.gleaming_strikes_extra_sunstruck_on_crit;

      p()->get_target_data( s->target )->debuffs.sunstruck->trigger( debuffs );
    }
  }

  void execute() override
  {
    xavian_attack_t::execute();

    if ( p()->buffs.suns_verdict->check() )
    {
      p()->actions.suns_verdict->execute();
      p()->cooldowns.blinding_slash->reset( false, -1 );
    }
  }
};

struct blinding_slash_t : public xavian_attack_t
{
  blinding_slash_t( util::string_view name, xavian_t* p, util::string_view options_str = {} )
    : xavian_attack_t( name, p, options_str )
  {
    id = 3;
    _procs_golden_hour      = true;

    school                  = SCHOOL_PHYSICAL;
    attack_power_mod.direct = p->spell_const.blinding_slash_coeff;

    name_str_reporting     = "Blinding Slash";
    swift_reprieval_chance = p->spell_const.blinding_slash_resource_proc_chance;

    aoe                 = -1;
    reduced_aoe_targets = p->spell_const.blinding_slash_scaling_threshold;

    cooldown->hasted   = true;
    cooldown->duration = p->spell_const.blinding_slash_cd;
    cooldown->charges  = 1;

    ability_flags |= ability_type_e::ABILITY_BASIC;
  }

  void impact( action_state_t* s ) override
  {
    xavian_attack_t::impact( s );

    p()->get_target_data( s->target )->debuffs.blind->trigger();
  }
};

struct omnistrike_t : public xavian_attack_t
{
  struct omnistrike_action_state_t : public action_state_t
  {
  public:
    int sunstruck_stacks;
    omnistrike_action_state_t( action_t* action, player_t* target )
      : action_state_t( action, target ), sunstruck_stacks( 0 )
    {
    }

    void initialize() override
    {
      action_state_t::initialize();
      sunstruck_stacks = 0;
    }

    std::ostringstream& debug_str( std::ostringstream& s ) override
    {
      action_state_t::debug_str( s ) << " sunstruck_stacks=" << sunstruck_stacks;
      return s;
    }

    void copy_state( const action_state_t* s )
    {
      action_state_t::copy_state( s );
      const omnistrike_action_state_t* rs = debug_cast<const omnistrike_action_state_t*>( s );
      sunstruck_stacks                    = rs->sunstruck_stacks;
    }
  };

  omnistrike_t( util::string_view name, xavian_t* p, util::string_view options_str = {} )
    : xavian_attack_t( name, p, options_str )
  {
    id = 4;
    _procs_golden_hour      = true;

    school                  = SCHOOL_PHYSICAL;
    attack_power_mod.direct = p->spell_const.omnistrike_coeff;

    name_str_reporting     = "Omnistrike";
    swift_reprieval_chance = p->spell_const.omnistrike_resource_proc_chance;

    aoe                 = -1;
    reduced_aoe_targets = p->spell_const.omnistrike_scaling_threshold;

    cooldown->hasted   = true;
    cooldown->duration = p->spell_const.omnistrike_cooldown;
    cooldown->charges  = 1;

    energize_type     = action_energize::ON_CAST;
    energize_amount   = p->spell_const.omnistrike_spirit_gain;
    energize_resource = RESOURCE_SPIRIT;
    
    ability_flags |= ability_type_e::ABILITY_CORE;
  }

  static const omnistrike_action_state_t* cast_state( const action_state_t* st )
  {
    return debug_cast<const omnistrike_action_state_t*>( st );
  }

  static omnistrike_action_state_t* cast_state( action_state_t* st )
  {
    return debug_cast<omnistrike_action_state_t*>( st );
  }

  action_state_t* new_state() override
  {
    return new omnistrike_action_state_t( this, target );
  }

  void impact( action_state_t* s ) override
  {
    xavian_attack_t::impact( s );

    p()->get_target_data( s->target )->debuffs.blind->trigger();
  }

  double composite_da_multiplier( const action_state_t* s ) const override
  {
    double m = xavian_attack_t::composite_da_multiplier( s );
    if ( p()->talents_enabled( xavian_t::GLEAMING_STRIKES ) && cast_state( s )->sunstruck_stacks > 0 )
      m *= 1 + p()->talents.gleaming_strikes_omnistrike_sunstruck_amp * cast_state( s )->sunstruck_stacks;
    return m;
  }

  void execute() override
  {
    int debuffs = 0;
    for ( auto t : target_list() )
    {
      debuffs += p()->get_target_data( t )->debuffs.sunstruck->check();

      p()->get_target_data( t )->debuffs.sunstruck->expire();
    }

    pre_execute_state                                 = get_state( pre_execute_state );
    cast_state( pre_execute_state )->sunstruck_stacks = debuffs;
    snapshot_state( pre_execute_state, amount_type( pre_execute_state ) );

    xavian_attack_t::execute();

    if ( debuffs > 0 )
    {
      auto mana_gained = p()->spell_const.sunstruck_mana_refund_base +
                         ( debuffs - 1 ) * p()->spell_const.sunstruck_mana_refund_per_stack;
      p()->resource_gain( RESOURCE_MANA, mana_gained, gain, this );
    }

    if ( p()->talents_enabled( xavian_t::RISING_SUN ) && p()->rng_objects.rising_sun->trigger() )
    {
      p()->cooldowns.solar_blades->reset( false, 1 );
    }

    if ( p()->buffs.golden_hour->check() )
    {
      p()->buffs.omega_reprieval->trigger();
      p()->buffs.golden_hour->expire();
    }
  }
};

struct suns_verdict_t : public xavian_attack_t
{
  suns_verdict_t( xavian_t* p ) : xavian_attack_t( "suns_verdict", p )
  {
    id = 5;

    background = true;

    school                  = SCHOOL_MAGIC;
    attack_power_mod.direct = p->talents.suns_verdict_coeff;

    name_str_reporting = "Suns Verdict";

    aoe                 = -1;
    reduced_aoe_targets = p->talents.suns_verdict_scaling_threshold;
    full_amount_targets = 1;
  }
};

struct solar_blades_t : public xavian_attack_t
{
  solar_blades_t( util::string_view name, xavian_t* p, util::string_view options_str = {} )
    : xavian_attack_t( name, p, options_str )
  {
    id = 6;
    _procs_golden_hour = true;

    use_off_gcd = true;

    gcd_type    = gcd_haste_type::NONE;
    trigger_gcd = 0_s;

    attack_power_mod.direct = p->spell_const.solar_blades_coeff;

    name_str_reporting     = "Solar Blades";
    swift_reprieval_chance = p->spell_const.solar_blades_resource_chance;

    aoe                 = -1;
    reduced_aoe_targets = p->spell_const.solar_blades_scaling_threshold;
    full_amount_targets = 1;

    cooldown->hasted   = true;
    cooldown->duration = p->spell_const.solar_blades_cd;
    cooldown->charges  = 1;

    resource_current            = RESOURCE_MANA;
    base_costs[ RESOURCE_MANA ] = p->spell_const.solar_blades_mana_cost;
    ability_flags |= ability_type_e::ABILITY_CORE;
  }

  void impact( action_state_t* s ) override
  {
    xavian_attack_t::impact( s );

    if ( p()->legendary.solar_glare )
    {
      p()->get_target_data( s->target )->debuffs.solar_glare->trigger();
    }

    if ( s->chain_target == 0 && p()->talents_enabled( xavian_t::SOLAR_BURN ) )
    {
      p()->get_target_data( s->target )->debuffs.sunburn->trigger();
      // Also do the residual dot.
    }
  }

  double composite_crit_chance() const override
  {
    double crit = xavian_attack_t::composite_crit_chance();

    if ( p()->legendary.solar_glare && p()->rng().roll( p()->legendary.solar_glare_dmg_chance ) )
      crit += p()->legendary.solar_glare_proc_added_cc;

    return crit;
  }

  double composite_da_multiplier( const action_state_t* s ) const override
  {
    double m = xavian_attack_t::composite_da_multiplier( s );

    if ( s->chain_target == 0 )
      m *= p()->spell_const.solar_blades_main_target_mul;

    return m;
  }

  void execute() override
  {
    xavian_attack_t::execute();
  }
};

struct brilliant_flash_action_state_t : public action_state_t
{
public:
  int omega_reprieval;

  brilliant_flash_action_state_t( action_t* action, player_t* target )
    : action_state_t( action, target ), omega_reprieval( 0 )
  {
  }

  void initialize() override
  {
    action_state_t::initialize();
    omega_reprieval = 0;
  }

  std::ostringstream& debug_str( std::ostringstream& s ) override
  {
    action_state_t::debug_str( s ) << " omega_reprieval=" << omega_reprieval;
    return s;
  }

  void copy_state( const action_state_t* s )
  {
    action_state_t::copy_state( s );
    const brilliant_flash_action_state_t* rs = debug_cast<const brilliant_flash_action_state_t*>( s );
  }
};

template <typename Base>
struct brilliant_flash_base_t : public xavian_action_t<Base>
{
  using ab = xavian_action_t<Base>;

  brilliant_flash_base_t( util::string_view name, xavian_t* p, util::string_view options_str = {} )
    : ab( name, p, options_str )
  {
    ab::id = 7;

    ab::_procs_golden_hour = true;

    ab::name_str_reporting = "Brilliant Flash";

    // aoe                 = -1;
    ab::reduced_aoe_targets = p->spell_const.omega_reprieval_falloff;
    ab::full_amount_targets = 1;

    if ( p->talents_enabled( xavian_t::GOLDEN_HOUR ) )
    {
      ab::base_dd_multiplier *= p->talents.golden_hour_brilliant_flash_amp;
    }

    ab::ability_flags |= ability_type_e::ABILITY_POWER;
  }

  static const brilliant_flash_action_state_t* cast_state( const action_state_t* st )
  {
    return debug_cast<const brilliant_flash_action_state_t*>( st );
  }

  static brilliant_flash_action_state_t* cast_state( action_state_t* st )
  {
    return debug_cast<brilliant_flash_action_state_t*>( st );
  }

  action_state_t* new_state() override
  {
    return new brilliant_flash_action_state_t( this, ab::target );
  }

  void snapshot_state( action_state_t* state, result_amount_type rt ) override
  {
    auto rs = cast_state( state );

    rs->omega_reprieval = ab::p()->buffs.omega_reprieval->check();

    ab::snapshot_state( state, rt );
  }

  void impact( action_state_t* s ) override
  {
    ab::impact( s );
  }

  int n_targets() const override
  {
    if ( ab::background )
      return 1;

    if ( ab::p()->buffs.omega_reprieval->check() )
      return -1;

    return 1;
  }

  double composite_da_multiplier( const action_state_t* s ) const override
  {
    double m = ab::composite_da_multiplier( s );

    if ( !ab::background && cast_state( s )->omega_reprieval && s->chain_target == 0 && !ab::background )
      m *= ab::p()->spell_const.omega_reprieval_main_target_mul;

    return m;
  }

  bool ready() override
  { 
    if ( !ab::p()->buffs.omega_reprieval->check() && !ab::p()->buffs.swift_reprieval->check() )
      return false;

    return ab::ready();
  }

  void execute() override
  {
    ab::execute();
    
    if ( ab::background )
      return;

    if ( ab::p()->buffs.omega_reprieval->check() )
    {
      ab::p()->buffs.omega_reprieval->decrement();
      ab::p()->resource_gain( RESOURCE_MANA,
                              ab::p()->resources.max[ RESOURCE_MANA ] * ab::p()->spell_const.omega_reprieval_mana_pct,
                              ab::p()->gains.omega_reprieval, this );
    }
    else
    {
      ab::p()->buffs.swift_reprieval->decrement();
    }

    if ( ab::p()->talents_enabled( xavian_t::SUNS_VERDICT ) && ab::p()->rng_objects.suns_verdict->trigger() )
    {
      ab::p()->buffs.suns_verdict->trigger();
    }

    if ( ab::p()->talents_enabled( xavian_t::CELESTIAL_FAVOR ) )
    {
      ab::p()->cooldowns.omega_reprieval->adjust( -ab::p()->talents.celestial_favor_cdr, false );
    }
  }
};

struct brilliant_flash_t : public brilliant_flash_base_t<spell_t>
{
private:
  using ab = brilliant_flash_base_t<spell_t>;

public:
  brilliant_flash_t( util::string_view name, xavian_t* p, util::string_view options_str = {} )
    : brilliant_flash_base_t<spell_t>( name, p, options_str )
  {
    attack_power_mod.direct = p->spell_const.brilliant_flash_dmg_coeff;
  }

  void execute() override
  {
    ab::execute();

    p()->actions.brilliant_flash_heal->execute_on_target( p() );
  }

  double composite_da_multiplier( const action_state_t* s ) const override
  {
    double m = ab::composite_da_multiplier( s );

    if ( s->target == p() )
      m *= p()->spell_const.brilliant_flash_self_heal_mul;

    return m;
  }
};

struct brilliant_flash_heal_t : public brilliant_flash_base_t<heal_t>
{
  brilliant_flash_heal_t( util::string_view name, xavian_t* p, util::string_view options_str = {} )
    : brilliant_flash_base_t<heal_t>( name, p, options_str )
  {
    attack_power_mod.direct = p->spell_const.brilliant_flash_heal_coeff;
  }
};

struct omega_repreival_t : public xavian_spell_t
{
  omega_repreival_t( util::string_view name, xavian_t* p, util::string_view options_str = {} )
    : xavian_spell_t( name, p, options_str )
  {
    id = 9;

    use_off_gcd = true;

    gcd_type    = gcd_haste_type::NONE;
    trigger_gcd = 0_s;

    name_str_reporting = "Omega Reprieval";

    cooldown->hasted   = false;
    cooldown->duration = p->spell_const.omega_reprieval_cd;
    cooldown->charges  = 1;

    ability_flags |= ability_type_e::ABILITY_MAJOR;
  }

  void execute() override
  {
    xavian_spell_t::execute();

    p()->buffs.omega_reprieval->trigger( 2 );
  }
};

struct vanguard_of_vengeance_t : public xavian_attack_t
{
  vanguard_of_vengeance_t( xavian_t* p ) : xavian_attack_t( "vanguard_of_vengeance", p )
  {
    id = 10;

    background = true;
    name_str_reporting = "Vanguard of Vengeance";

    energize_resource = RESOURCE_MANA;
    energize_amount   = p->resources.max[ RESOURCE_MANA ] * p->talents.vanguard_of_vengeance_mana_gain_pct;

    attack_power_mod.direct = p->talents.vanguard_of_vengeance_coeff;
  }

  void execute() override
  {
    p()->buffs.vanguard_of_vengeance->trigger();
    base_t::execute();
  }
};

struct solaris_t : public xavian_spell_t
{
  solaris_t( xavian_t* p ) : xavian_spell_t( "solaris", p )
  {
    id                 = 11;
    background         = true;
    name_str_reporting = "Solaris";

    may_crit = false;

    aoe = -1;
    reduced_aoe_targets = p->spell_const.solaris_target_scaling_threshold;
  }

  void init_finished()
  {
    snapshot_flags &= STATE_NO_MULTIPLIER;
    snapshot_flags |= STATE_TARGET_NO_PET & ~STATE_TGT_CRIT;
  }
};

struct decree_of_the_sun_t : public xavian_spell_t
{
  decree_of_the_sun_t( std::string_view name, xavian_t* p, std::string_view opt ) : xavian_spell_t( "decree_of_the_sun", p, opt )
  {
    id                 = 12;
    name_str_reporting = "Decree of the Sun";

    resource_current              = RESOURCE_SPIRIT;
    base_costs[ RESOURCE_SPIRIT ] = 100;
    ability_flags |= ability_type_e::ABILITY_SPIRIT;
    
    use_off_gcd = true;

    gcd_type    = gcd_haste_type::NONE;
    trigger_gcd = 0_s;
  }

  void execute() override
  {
    xavian_spell_t::execute();
    p()->fs_buffs.spirit_of_heroism->trigger();
    p()->buffs.decree_of_the_sun->trigger();
    p()->buffs.decree_of_the_sun_invuln->trigger();
    p()->used_ultimate();
  }
};

struct decree_of_the_sun_dmg_t : public xavian_spell_t
{
  decree_of_the_sun_dmg_t( xavian_t* p ) : xavian_spell_t( "decree_of_the_sun_dmg", p )
  {
    id                      = 12;
    background              = true;
    name_str_reporting      = "Decree of the Sun Damage";
    attack_power_mod.direct = p->spell_const.decree_of_the_sun_dmg_coeff;
    aoe                     = -1;
    reduced_aoe_targets     = p->spell_const.decree_of_the_sun_falloff;
    ability_flags |= ability_type_e::ABILITY_SPIRIT;
  }
};

struct decree_of_the_sun_heal_t : public xavian_heal_t
{
  decree_of_the_sun_heal_t( xavian_t* p ) : xavian_heal_t( "decree_of_the_sun_heal", p )
  {
    id                      = 12;
    background              = true;
    name_str_reporting      = "Decree of the Sun Heal";
    attack_power_mod.direct = p->spell_const.decree_of_the_sun_heal_coeff;
    aoe                     = -1;
    ability_flags |= ability_type_e::ABILITY_SPIRIT;
  }
};

}  // namespace actions

xavian_td_t::xavian_td_t( player_t* target, xavian_t* source )
  : fellowship::fs_player_td_t( target, source ), dots(), debuffs()
{
  dots.sunburn    = target->get_dot( "sunburn", source );
  dots.suns_touch = target->get_dot( "suns_touch", source );

  debuffs.blind = make_buff( *this, "blind" )
                      ->set_duration( source->spell_const.blinding_slash_blind_duration )
                      ->set_max_stack( source->spell_const.blinding_slash_max_stacks )
                      ->set_default_value( source->spell_const.blinding_slash_parry_chance )
                      ->set_refresh_behavior( buff_refresh_behavior::DURATION );

  debuffs.solar_glare = make_buff( *this, "solar_glare" )
                            ->set_duration( source->legendary.solar_glare_duration )
                            ->set_max_stack( 1 )
                            ->set_default_value( source->legendary.solar_glare_dmg_taken_mul )
                            ->set_refresh_behavior( buff_refresh_behavior::DURATION );

  debuffs.sunburn = make_buff( *this, "sunburn" )
                        ->set_duration( source->talents.solar_burn_duration )
                        ->set_period( source->talents.solar_burn_period )
                        ->set_default_value( source->talents.solar_burn_dmg_received_multiplier )
                        ->set_refresh_behavior( buff_refresh_behavior::DURATION );

  debuffs.sunstruck = make_buff( *this, "sunstruck" )
                          ->set_duration( source->spell_const.sunstruck_duration )
                          ->set_max_stack( source->spell_const.suntruck_max_stacks )
                          ->set_refresh_behavior( buff_refresh_behavior::DURATION );
}

double xavian_t::composite_parry( action_state_t* s ) const
{
  auto parry = fs_player_t::composite_parry( s );

  if ( s )
    parry += get_target_data( s->action->player )->debuffs.blind->check_value();

  return parry;
}

// xavian_t::composite_attribute_multiplier ==================================

double xavian_t::composite_attribute_multiplier( attribute_e a ) const
{
  double am = fs_player_t::composite_attribute_multiplier( a );

  return am;
}

// xavian_t::composite_melee_auto_attack_speed ===============================

double xavian_t::composite_melee_auto_attack_speed() const
{
  double h = fs_player_t::composite_melee_auto_attack_speed();

  return h;
}

// xavian_t::composite_melee_haste ==========================================

double xavian_t::composite_melee_haste() const
{
  double h = fs_player_t::composite_melee_haste();

  return h;
}

// xavian_t::composite_spell_haste ==========================================

double xavian_t::composite_spell_haste() const
{
  double h = fs_player_t::composite_spell_haste();

  return h;
}

// xavian_t::composite_melee_crit_chance =========================================

double xavian_t::composite_melee_crit_chance() const
{
  double crit = fs_player_t::composite_melee_crit_chance();

  return crit;
}

// xavian_t::composite_spell_crit_chance =========================================

double xavian_t::composite_spell_crit_chance() const
{
  double crit = fs_player_t::composite_spell_crit_chance();

  return crit;
}

// xavian_t::composite_damage_versatility ===================================

double xavian_t::composite_damage_versatility() const
{
  double cdv = fs_player_t::composite_damage_versatility();

  return cdv;
}

// xavian_t::composite_heal_versatility =====================================

double xavian_t::composite_heal_versatility() const
{
  double chv = fs_player_t::composite_heal_versatility();

  return chv;
}

// xavian_t::composite_leech ===============================================

double xavian_t::composite_leech() const
{
  double l = fs_player_t::composite_leech();

  return l;
}

// xavian_t::matching_gear_multiplier ========================================

double xavian_t::matching_gear_multiplier( attribute_e attr ) const
{
  return 0.0;
}

// xavian_t::composite_player_multiplier =====================================

double xavian_t::composite_player_multiplier( school_e school ) const
{
  double m = fs_player_t::composite_player_multiplier( school );

  return m;
}

// xavian_t::composite_player_pet_damage_multiplier ==========================

double xavian_t::composite_player_pet_damage_multiplier( const action_state_t* s, bool guardian ) const
{
  double m = fs_player_t::composite_player_pet_damage_multiplier( s, guardian );

  return m;
}

// xavian_t::composite_player_target_multiplier ==============================

double xavian_t::composite_player_target_multiplier( player_t* target, school_e school ) const
{
  double m = fs_player_t::composite_player_target_multiplier( target, school );

  return m;
}

// xavian_t::composite_player_target_crit_chance =============================

double xavian_t::composite_player_target_crit_chance( player_t* target ) const
{
  double c = fs_player_t::composite_player_target_crit_chance( target );

  return c;
}

// xavian_t::composite_player_target_armor ===================================

double xavian_t::composite_player_target_armor( player_t* target ) const
{
  return 0.0;

  double a = fs_player_t::composite_player_target_armor( target );

  return a;
}
// xavian_t::init_actions ====================================================

void xavian_t::init_action_list()
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

// xavian_t::create_action  ==================================================

action_t* xavian_t::create_action( util::string_view name, util::string_view options_str )
{
  using namespace actions;

  if ( name == "sun_strike" )
    return new sun_strike_t( name, this, options_str );
  else if ( name == "blinding_slash" )
    return new blinding_slash_t( name, this, options_str );
  else if ( name == "omnistrike" )
    return new omnistrike_t( name, this, options_str );
  else if ( name == "solar_blades" )
    return new solar_blades_t( name, this, options_str );
  else if ( name == "brilliant_flash" )
    return new brilliant_flash_t( name, this, options_str );
  else if ( name == "brilliant_flash_heal" )
    return new brilliant_flash_heal_t( name, this, options_str );
  else if ( name == "omega_reprieval" )
    return new omega_repreival_t( name, this, options_str );
  else if ( name == "decree_of_the_sun" )
    return new decree_of_the_sun_t( name, this, options_str );

  return fs_player_t::create_action( name, options_str );
}

// xavian_t::create_expression ===============================================

std::unique_ptr<expr_t> xavian_t::create_action_expression( action_t& action, std::string_view name_str )
{
  auto split = util::string_split<util::string_view>( name_str, "." );

  return fs_player_t::create_action_expression( action, name_str );
}

std::unique_ptr<expr_t> xavian_t::create_expression( util::string_view name_str )
{
  auto split = util::string_split<util::string_view>( name_str, "." );

  if ( util::str_compare_ci( split[ 0 ], "legendary" ) )
  {
    if ( split.size() == 2 )
    {
      if ( util::str_compare_ci( split[ 1 ], "solar_glare" ) )
      {
        return make_ref_expr( name_str, legendary.solar_glare );
      }
      else if ( util::str_compare_ci( split[ 1 ], "fortress_in_the_sands" ) )
      {
        return make_ref_expr( name_str, legendary.fortress_in_the_sands );
      }
      else if ( util::str_compare_ci( split[ 1 ], "grossly_incandescent" ) )
      {
        return make_ref_expr( name_str, legendary.grossly_incandescent );
      }
    }
  }
  else if ( util::str_compare_ci( split[ 0 ], "talent" ) )
  {
    if ( split.size() == 2 )
    {
      for ( xavian_talents_t t = static_cast<xavian_talents_t>( 1U ); t < xavian_talents_t::MAX; t++ )
      {
        if ( util::str_compare_ci( split[ 1 ], talent_name( t ) ) )
        {
          return make_fn_expr( name_str, std::bind( std::mem_fn( &xavian_t::talents_enabled ), this, t ) );
        }
      }
    }
  }
  // Split expressions

  return fs_player_t::create_expression( name_str );
}

std::unique_ptr<expr_t> xavian_t::create_resource_expression( util::string_view name_str )
{
  return fs_player_t::create_resource_expression( name_str );
}

// xavian_t::init_base =======================================================

void xavian_t::init_base_stats()
{
  if ( base.distance < 1 )
    base.distance = 5;

  fs_player_t::init_base_stats();

  base.stats.attribute[ STAT_STRENGTH ] = 100;
  resources.base[ RESOURCE_HEALTH ]     = 2556;

  base.health_per_stamina = 90.303;

  resources.base[ RESOURCE_MANA ] = 1000;

  resources.base_regen_per_second[ RESOURCE_MANA ] = 0.005 * resources.base[ RESOURCE_MANA ];

  base_gcd = timespan_t::from_seconds( 1.5 );
  min_gcd  = timespan_t::from_seconds( 0 );
  //min_gcd  = timespan_t::from_seconds( 0.75 );

  base.parry = 0.05;
  base.dodge = 0.05;

  if ( talents_enabled( VANGUARD_OF_VENGEANCE ) )
    base.parry += talents.vanguard_of_vengeance_parry;
}

// xavian_t::init_spells =====================================================

void xavian_t::init_spells()
{
  fs_player_t::init_spells();

  actions.auto_attack = new actions::auto_melee_attack_t( this, "" );
}

// xavian_t::init_gains ======================================================

void xavian_t::init_gains()
{
  fs_player_t::init_gains();

  gains.spirit_procs    = get_gain( "Spirit Procs" );
  gains.omega_reprieval = get_gain( "Omega Reprieval" );
}

// xavian_t::init_procs ======================================================

void xavian_t::init_procs()
{
  fs_player_t::init_procs();
}

// xavian_t::init_scaling ====================================================

void xavian_t::init_scaling()
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

// xavian_t::init_resources =================================================

void xavian_t::init_resources( bool force )
{
  fs_player_t::init_resources( force );
}

// xavian_t::init_buffs ======================================================

void xavian_t::create_buffs()
{
  fs_player_t::create_buffs();

  buffs.decree_of_the_sun =
      make_buff<xavian_buff_t>( this, "decree_of_the_sun" )
          ->set_duration( spell_const.decree_of_the_sun_dr_duration + spell_const.decree_of_the_sun_invuln_duration )
          ->set_max_stack( 1 )
          ->set_refresh_behavior( buff_refresh_behavior::DURATION )
          ->set_period( spell_const.decree_of_the_sun_pulse_period )
          ->set_tick_callback( [ this ]( buff_t*, int, timespan_t ) { actions.decree_of_the_sun_heal->execute(); } );

  buffs.decree_of_the_sun_invuln = make_buff<xavian_buff_t>( this, "decree_of_the_sun_invuln" )
                                       ->set_duration( spell_const.decree_of_the_sun_invuln_duration )
                                       ->set_max_stack( 1 )
                                       ->set_refresh_behavior( buff_refresh_behavior::DURATION )
                                       ->add_stack_change_callback( [ this ]( buff_t* b, int old, int _new ) {
                                         if ( !_new )
                                         {
                                           actions.decree_of_the_sun_dmg->execute();
                                           buffs.decree_of_the_sun_dr->trigger();
                                         }
                                       } );

  buffs.decree_of_the_sun_dr = make_buff<xavian_buff_t>( this, "decree_of_the_sun_dr" )
                                   ->set_duration( spell_const.decree_of_the_sun_dr_duration )
                                   ->set_max_stack( 1 )
                                   ->set_refresh_behavior( buff_refresh_behavior::DURATION );

  buffs.vanguard_of_vengeance = make_buff<xavian_buff_t>( this, "vanguard_of_vengeance" )
                                    ->set_duration( talents.vanguard_of_vengeance_buff_duration )
                                    ->set_max_stack( talents.vanguard_of_vengeance_max_stacks )
                                    ->set_default_value( talents.vanguard_of_vengeance_spirit_per_stack )
                                    ->set_pct_buff_type( STAT_PCT_BUFF_MASTERY );

  buffs.swift_reprieval = make_buff<xavian_buff_t>( this, "swift_reprieval" )
                              ->set_max_stack( spell_const.swift_reprieval_max_stacks )
                              ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

  buffs.aura_of_solace = make_buff<xavian_buff_t>( this, "aura_of_solace" )
                             ->set_default_value( spell_const.aura_of_solace_parry )
                             ->add_stack_change_callback( [ this ]( buff_t* b, int old, int _new ) {
                               if ( _new > old )
                               {
                                 b->player->current.parry += b->current_value;
                               }
                               else
                               {
                                 b->player->current.parry -= b->current_value;
                               }
                             } )
                             ->add_invalidate( CACHE_PARRY );

  buffs.suns_verdict = make_buff<xavian_buff_t>( this, "suns_verdict" )
                           ->set_max_stack( talents.suns_verdict_max_stacks )
                           ->set_duration( talents.suns_verdict_duration );

  buffs.golden_hour = make_buff<xavian_buff_t>( this, "golden_hour" )
                          ->set_duration( talents.golden_hour_buff_duration )
                          ->set_max_stack( 1 )
                          ->set_refresh_behavior( buff_refresh_behavior::DURATION );

  buffs.omega_reprieval = make_buff<xavian_buff_t>( this, "omega_reprieval" )
                              ->set_max_stack( spell_const.omega_reprieval_max_stacks )
                              ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );
}

// xavian_t::invalidate_cache =========================================

void xavian_t::invalidate_cache( cache_e c )
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

void xavian_t::create_options()
{
  fs_player_t::create_options();
}

// xavian_t::copy_from =======================================================

void xavian_t::copy_from( player_t* source )
{
  xavian_t* xavian = static_cast<xavian_t*>( source );
  fs_player_t::copy_from( source );

  talents     = xavian->talents;
  legendary   = xavian->legendary;
  options     = xavian->options;
  spell_const = xavian->spell_const;
}

// xavian_t::create_profile  =================================================

std::string xavian_t::create_profile( save_e stype )
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

// xavian_t::init_items ======================================================

void xavian_t::init_items()
{
  fs_player_t::init_items();
}

// xavian_t::init_special_effects ============================================

void xavian_t::init_special_effects()
{
  fs_player_t::init_special_effects();

  {
    auto effect                   = new special_effect_t( this );
    effect->spell_id              = 107;
    effect->name_str              = "xavian_parry";
    effect->proc_flags_           = PF_DAMAGE_TAKEN;
    effect->proc_flags2_          = PF2_PARRY;
    effect->rppm_scale_           = rppm_scale_e::RPPM_NONE;
    effect->proc_chance_          = 1.0;
    effect->type                  = special_effect_e::SPECIAL_EFFECT_EQUIP;

    special_effects.push_back( effect );

    struct parry_cb_t : dbc_proc_callback_t
    {
      parry_cb_t( xavian_t* p, const special_effect_t& e ) : dbc_proc_callback_t( p, e )
      {
      }

      xavian_t* p() const
      {
        return static_cast<xavian_t*>( listener );
      }

      void execute( action_t*, action_state_t* s ) override
      {
        p()->parry_effects( s );
      }
    };

    auto cb = new parry_cb_t( this, *effect );
    cb->initialize();
    cb->activate();
  }

  {
    auto effect                   = new special_effect_t( this );
    effect->spell_id              = 108;
    effect->name_str              = "solaris";
    effect->proc_flags_           = PF_ALL_HEAL_TAKEN;
    effect->proc_flags2_          = PF2_ALL_HIT | PF2_PERIODIC_HEAL;
    effect->set_can_proc_from_procs( true );
    effect->rppm_scale_           = rppm_scale_e::RPPM_NONE;
    effect->proc_chance_          = 1.0;
    effect->type                  = special_effect_e::SPECIAL_EFFECT_EQUIP;

    special_effects.push_back( effect );

    struct solaris_cb_t : dbc_proc_callback_t
    {
      solaris_cb_t( xavian_t* p, const special_effect_t& e ) : dbc_proc_callback_t( p, e )
      {
      }

      xavian_t* p() const
      {
        return static_cast<xavian_t*>( listener );
      }

      void execute( action_t*, action_state_t* s ) override
      {
        auto overheal = s->result_total - s->result_amount;

        if ( overheal > 0 )
        {
          p()->actions.solaris->execute_on_target( p()->actions.solaris->target,
                                                   overheal * p()->spell_const.solaris_overheal_scaler );
        }
      }
    };

    auto cb = new solaris_cb_t( this, *effect );
    cb->initialize();
    cb->activate();
  }
}

// xavian_t::parry_effects ===================================================
void xavian_t::parry_effects( action_state_t* s )
{
  if ( !s )
    return;

  player_t* source = s->action->player;
  auto td          = get_target_data( source );

  td->debuffs.blind->decrement();

  if ( talents_enabled( VANGUARD_OF_VENGEANCE ) )
  {
    actions.vanguard_of_vengeance->execute_on_target( source );
  }

  if ( talents_enabled( HALLOWED_SHIELD ) )
  {
    buffs.hallowed_shield->trigger();
  }
}

// xavian_t::init_finished ===================================================

void xavian_t::init_finished()
{
  fs_player_t::init_finished();
}

void xavian_t::init_talents()
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

    for ( xavian_talents_t t = static_cast<xavian_talents_t>( 1U ); t < xavian_talents_t::MAX; t++ )
    {
      if ( util::str_compare_ci( talent_split[ 0 ], talent_name( t ) ) )
      {
        set_talent_points( t, ranks >= 1 );
        break;
      }
    }
  }
}

void xavian_t::init_background_actions()
{
  fs_player_t::init_background_actions();

  actions.suns_verdict                     = new actions::suns_verdict_t( this );

  actions.brilliant_flash_heal             = new actions::brilliant_flash_heal_t( "brilliant_flash_heal", this, {} );
  actions.brilliant_flash_heal->background = true;

  actions.vanguard_of_vengeance  = new actions::vanguard_of_vengeance_t( this );
  actions.solaris                = new actions::solaris_t( this );
  actions.decree_of_the_sun_dmg  = new actions::decree_of_the_sun_dmg_t( this );
  actions.decree_of_the_sun_heal = new actions::decree_of_the_sun_heal_t( this );
}

void xavian_t::init_rng()
{
  fs_player_t::init_rng();

  rng_objects.golden_hour = get_accumulated_rng( "golden_hour", rng::CfromP( talents.golden_hour_proc_chance ) );

  rng_objects.rising_sun = get_accumulated_rng( "rising_sun", rng::CfromP( talents.rising_sun_chance ) );

  rng_objects.suns_verdict = get_accumulated_rng( "suns_verdict", rng::CfromP( talents.suns_verdict_chance ) );
}

// xavian_t::reset ===========================================================

void xavian_t::reset()
{
  fs_player_t::reset();
}

// xavian_t::activate ========================================================

void xavian_t::activate()
{
  fs_player_t::activate();
}

// xavian_t::cancel_auto_attack ==============================================

void xavian_t::cancel_auto_attacks()
{
  if ( actions.melee_hit && actions.melee_hit->execute_event )
  {
    actions.melee_hit->canceled            = true;
    actions.melee_hit->prev_scheduled_time = actions.melee_hit->execute_event->occurs();
  }

  fs_player_t::cancel_auto_attacks();
}

// xavian_t::arise ===========================================================

void xavian_t::arise()
{
  fs_player_t::arise();
}

// xavian_t::combat_begin ====================================================

void xavian_t::combat_begin()
{
  fs_player_t::combat_begin();
}

// xavian_t::energy_regen_per_second =========================================

double xavian_t::resource_regen_per_second( resource_e r ) const
{
  double reg = fs_player_t::resource_regen_per_second( r );

  return reg;
}

double xavian_t::resource_gain( resource_e resource_type, double amount, gain_t* source, action_t* action )
{
  double actual_amount = fs_player_t::resource_gain( resource_type, amount, source, action );

  return actual_amount;
}

// xavian_t::non_stacking_movement_modifier ==================================

double xavian_t::non_stacking_movement_modifier() const
{
  double ms = fs_player_t::non_stacking_movement_modifier();

  return ms;
}

// xavian_t::stacking_movement_modifier===================================

double xavian_t::stacking_movement_modifier() const
{
  double ms = fs_player_t::stacking_movement_modifier();

  return ms;
}

// xavian_t::regen ===========================================================

void xavian_t::regen( timespan_t periodicity )
{
  fs_player_t::regen( periodicity );
}

template <typename Base>
void actions::xavian_action_t<Base>::trigger_auto_attack( const action_state_t* /* state */ )
{
  if ( !p()->main_hand_attack || p()->main_hand_attack->execute_event )
    return;

  p()->actions.auto_attack->schedule_execute();
}

template <typename Base>
void actions::xavian_action_t<Base>::trigger_spirit_refund( const action_state_t* state )
{
  double mana_spent = ab::last_resource_cost;

  make_event( ab::sim, 200_ms, [ mana_spent, this ] {
    p()->resource_gain( RESOURCE_MANA, mana_spent, p()->gains.spirit_procs, this );
  } );

  p()->spirit_refund();
}

// xavian_t::convert_hybrid_stat ==============================================

stat_e xavian_t::convert_hybrid_stat( stat_e s ) const
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

void xavian_t::create_cooldowns()
{
  cooldowns.blinding_slash  = get_cooldown( "blinding_slash" );
  cooldowns.omega_reprieval = get_cooldown( "omega_reprieval" );
  cooldowns.omnistrike      = get_cooldown( "omnistrike" );
  cooldowns.solar_blades    = get_cooldown( "solar_blades" );
  cooldowns.solar_shield    = get_cooldown( "solar_shield" );
}

class xavian_module_t : public module_t
{
public:
  xavian_module_t() : module_t( XAVIAN )
  {
  }

  player_t* create_player( sim_t* sim, util::string_view name, race_e r = RACE_NONE ) const override
  {
    return new xavian_t( sim, name, r );
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

}  // namespace xavian
}  // namespace fellowship

const module_t* module_t::xavian()
{
  static fellowship::xavian::xavian_module_t m;
  return &m;
}
