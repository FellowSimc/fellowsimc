#include "fs_player.hpp"
#include "util/util.hpp"

#include "simulationcraft.hpp"

namespace fellowship
{
namespace elarion
{

// Forward Declarations
class elarion_t;

namespace actions
{
struct elarion_heal_t;
struct elarion_spell_t;

struct melee_t;
}  // namespace actions

class elarion_td_t : public fs_player_td_t
{
public:
  struct dots_t
  {
  } dots;

  struct
  {
    buff_t* lunarlight_mark;
    buff_t* shimmer;
  } debuffs;

  elarion_td_t( player_t* target, elarion_t* source );
};

struct elarion_buff_t : public fs_player_buff_t
{
  elarion_buff_t( player_t* p, util::string_view name ) : fs_player_buff_t( p, name )
  {
  }

  elarion_t* p()
  {
    return debug_cast<elarion_t*>( player );
  }

  const elarion_t* p() const
  {
    return debug_cast<const elarion_t*>( player );
  }
};

class elarion_t : public fellowship::fs_player_t
{
public:
  struct actions_t
  {
    action_t* lunarlight_salvo;
    action_t* starfall_volley;
    action_t* lunarlight_marks_spirit;
  } actions;

  struct buffs_t
  {
    buff_t* starfall_volleys;
    buff_t* event_horizon;
    buff_t* skystriders_grace;
    buff_t* skystriders_supremacy;
    buff_t* focused_expanse;
    buff_t* celestial_impetus;
    buff_t* final_crescendo;
    buff_t* multishot;
    buff_t* resurgent_winds;
    buff_t* impending_heartseeker;
  } buffs;

  struct cooldowns_t
  {
    cooldown_t* skystriders_grace;
    cooldown_t* starfall_volley;
    cooldown_t* heartseeker_barrage;
    cooldown_t* highwind_arrow;
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
    real_ppm_t* celestial_impetus;
  } rppm;

  struct spell_const_t
  {
    timespan_t auto_attack_time = 2.4_s;
    double auto_attack_ap_coeff = 0.6;

    double focused_shot_ap_coeff          = 1.347;
    timespan_t focused_shot_cast_time     = 1.5_s;
    double focused_shot_resource_gain     = 20;
    int celestial_impetus_max_stacks      = 2;
    double celestial_impetus_ppm          = 2.0;
    int celestial_impetus_marks_applied   = 3;
    timespan_t celestial_impetus_duration = 15_s;

    double celestial_shot_ap_coeff   = 2.879;
    double celestial_shot_focus_cost = 15;

    double multishot_ap_coeff       = 2.683;
    double multishot_target_falloff = 12;
    double multishot_focus_cost     = 20;
    int multishot_max_stacks        = 5;

    timespan_t skystriders_grace_cooldown = 120_s;
    timespan_t skystriders_grace_duration = 20_s;
    double skystriders_grace_haste_bonus  = 0.3;

    timespan_t heartseeker_barrage_period   = 0.2_s;
    timespan_t heartseeker_barrage_duration = 2.0_s;
    double heartseeker_barrage_ap_coeff     = 1.249;
    timespan_t heartseeker_barrage_cooldown = 20_s;
    double heartseeker_barrage_focus_cost   = 30;

    double highwind_arrow_ap_coeff                    = 9.1;
    timespan_t highwind_arrow_cast_time               = 2.0_s;
    double highwind_arrow_focus_cost                  = 30;
    int highwind_arrow_charges                        = 3;
    timespan_t highwind_arrow_cooldown                = 15_s;
    int highwind_arrow_targets                        = 3;
    double highwind_arrow_cleave_mul                  = 0.7;
    unsigned int highwind_arrow_targets_for_multishot = 3;

    timespan_t lunarlight_mark_cooldown = 30_s;
    int lunarlight_mark_max_targets     = 12;
    int lunarlight_mark_stacks_applied  = 3;
    timespan_t lunarlight_mark_duration = 15_s;
    double lunarlight_mark_ap_coeff     = 2.2597;
    double lunarlight_mark_chance_hit   = 0.25;
    double lunarlight_mark_chance_crit  = 0.5;

    timespan_t skystriders_supremacy_duration = 4_s;
    double skystriders_supremacy_focus_mul    = 0.5;
    int skystriders_supremacy_minimum_arrows  = 3;
    timespan_t skystriders_supremacy_cooldown = 45_s;

    timespan_t starfall_volley_duration   = 8_s;
    double starfall_volley_target_falloff = 10;
    timespan_t starfall_volley_cooldown   = 30_s;
    timespan_t starfall_volley_period     = 1_s;
    double starfall_volley_ap_coeff       = 1.0864;
    double starfall_volley_focus_cost     = 30;

    timespan_t event_horizon_duration             = 20_s;
    timespan_t event_horizon_cast_time            = 0.7_s;
    double event_horizon_haste_to_cdr             = 1.0;
    double event_horizon_dmg_mul                  = 0.2;
    double event_horizon_resource_mul             = 0.5;
    timespan_t event_horizon_barrage_cdr_highwind = 0.5_s;
    timespan_t event_horizon_volley_cdr_barrage   = 1_s;

    int spirit_refund_marks_applied       = 3;
    int spirit_refund_marks_cleave        = 2;
    int spirit_refund_marks_extra_targets = 2;
  } spell_const;

  enum elarion_talents_t : unsigned long long
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

  static constexpr std::string_view talent_name_formatted( elarion_talents_t t )
  {
    switch ( t )
    {
      case elarion_talents_t::TALENT_1:
        return "Focused Expanse";
      case elarion_talents_t::TALENT_2:
        return "Fusillade";
      case elarion_talents_t::TALENT_3:
        return "Final Crescendo";
      case elarion_talents_t::TALENT_4:
        return "Skylit Grace";
      case elarion_talents_t::TALENT_5:
        return "Piercing Seekers";
      case elarion_talents_t::TALENT_6:
        return "Skyward Munitions";
      case elarion_talents_t::TALENT_7:
        return "Repeating Stars";
      case elarion_talents_t::TALENT_8:
        return "Lunarlight Affinity";
      case elarion_talents_t::TALENT_9:
        return "Lethal Shots";
      case elarion_talents_t::TALENT_10:
        return "Path of Twilight";
      case elarion_talents_t::TALENT_11:
        return "Lunar Fury";
      case elarion_talents_t::TALENT_12:
        return "Magic Ward";
      case elarion_talents_t::TALENT_13:
        return "Fervent Supremacy";
      case elarion_talents_t::TALENT_14:
        return "Impending Heartseeker";
      case elarion_talents_t::TALENT_15:
        return "Resurgent Winds";
      case elarion_talents_t::TALENT_16:
        return "Last Lights";
      case elarion_talents_t::TALENT_17:
        return "Spirited Fortitude";
      case elarion_talents_t::TALENT_18:
        return "The Weight of Gravity";
      default:
        return "Unknown Talent";
    }
  }
  static constexpr std::string_view talent_name( elarion_talents_t t )
  {
    switch ( t )
    {
      case elarion_talents_t::TALENT_1:
        return "focused_expanse";
      case elarion_talents_t::TALENT_2:
        return "fusillade";
      case elarion_talents_t::TALENT_3:
        return "final_crescendo";
      case elarion_talents_t::TALENT_4:
        return "skylit_grace";
      case elarion_talents_t::TALENT_5:
        return "piercing_seekers";
      case elarion_talents_t::TALENT_6:
        return "skyward_munitions";
      case elarion_talents_t::TALENT_7:
        return "repeating_stars";
      case elarion_talents_t::TALENT_8:
        return "lunarlight_affinity";
      case elarion_talents_t::TALENT_9:
        return "lethal_shots";
      case elarion_talents_t::TALENT_10:
        return "path_of_twilight";
      case elarion_talents_t::TALENT_11:
        return "lunar_fury";
      case elarion_talents_t::TALENT_12:
        return "magic_ward";
      case elarion_talents_t::TALENT_13:
        return "fervent_supremacy";
      case elarion_talents_t::TALENT_14:
        return "impending_heartseeker";
      case elarion_talents_t::TALENT_15:
        return "resurgent_winds";
      case elarion_talents_t::TALENT_16:
        return "last_lights";
      case elarion_talents_t::TALENT_17:
        return "spirited_fortitude";
      case elarion_talents_t::TALENT_18:
        return "the_weight_of_gravity";
      default:
        return "unknown_talent";
    }
  }

  struct talents_t
  {
    double focused_expanse_chance       = 0.2;
    double focused_expanse_amp          = 0.25;
    double focused_expanse_focus_mul    = 0.5;
    timespan_t focused_expanse_duration = 15_s;
    int focused_expanse_max_stacks      = 2;

    timespan_t fusillade_duration = 0.5_s;
    double fusillade_crit         = 0.2;

    double final_crescendo_dmg_mul = 1.0;
    int final_crescendo_max_stacks = 3;
    int final_crescendo_ricochets  = 8;

    double skylit_grace_cdr = 1.0;

    int piercing_seekers_ricochet_targets = 1;
    double piercing_seekers_ricochet_mul  = 0.7;

    timespan_t skyward_munitions_cdr = 1_s;

    timespan_t repeating_stars_cdr = 0.3_s;

    double lunarlight_affinity_volley_chance_mul = 1.0;
    double lunarlight_affinity_salvo_cc          = 0.4;

    double lethal_shots_proc_chance = 0.3;
    double lethal_shots_added_cc    = 1.0;

    double lunar_fury_mul = 0.3;

    timespan_t fervent_supremacy_duration         = 15_s;
    timespan_t fervent_supremacy_reduced_cooldown = 20_s;
    int fervent_supremacy_stacks                  = 4;
    double fervent_supremacy_mul                  = 0.25;

    double impending_heartseeker_mul_per_arrow = 0.1;
    timespan_t impending_heartseeker_duration  = 15_s;

    timespan_t resurgent_winds_duration  = 15_s;
    double resurgent_winds_mul           = 0.5;
    int resurgent_winds_maximum_stacks   = 2;
    double resurgent_winds_cast_time_mul = 0.0;

    double last_lights_cc    = 0.2;
    double last_light_hp_pct = 30;
  } talents;

  struct legendary_t
  {
    bool shimmer                 = false;
    timespan_t shimmer_duration  = 9_s;
    double shimmer_mul_per_stack = 0.1;
    int shimmer_max_stacks       = 2;

    bool starstrikers_ascent                                = false;
    int starstrikers_ascent_spirit_refunds_resurgent_stacks = 1;

    bool astronomers_hail                        = false;
    timespan_t astronomers_hail_volley_duration  = 2_s;
    timespan_t astronomers_hail_multishot_extend = 0.5_s;


    bool new_spirit_legendary = false;
    double new_spirit_legendary_chance_to_consume_mark = 1.0;
  } legendary;

  struct options_t
  {
  } options;

  target_specific_t<elarion_td_t> target_data;

  const elarion_td_t* find_target_data( const player_t* target ) const override
  {
    return target_data[ target ];
  }

  elarion_td_t* get_target_data( player_t* target ) const override
  {
    elarion_td_t*& td = target_data[ target ];
    if ( !td )
    {
      td = new elarion_td_t( target, const_cast<elarion_t*>( this ) );
    }
    return td;
  }

  // Character Definition
  void init_spells() override;
  void init_base_stats() override;
  void init_talents() override;
  void init_gains() override;
  void init_scaling() override;
  void init_finished() override;
  void init_background_actions() override;
  void init_rng() override;

  void create_cooldowns();
  void create_buffs() override;
  void create_options() override;

  void copy_from( player_t* source ) override;
  std::string create_profile( save_e stype ) override;
  void init_action_list() override;

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
  double composite_player_target_multiplier( player_t* target, school_e school ) const override;
  double composite_player_target_crit_chance( player_t* target ) const override;
  double composite_player_target_armor( player_t* target ) const override;
  void invalidate_cache( cache_e ) override;

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

  double current_focus( bool /* react */ = false ) const
  {
    return resources.current[ RESOURCE_FOCUS ];
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

  std::set<ground_aoe_event_t*> starfall_volleys;

  elarion_t( sim_t* sim, util::string_view name, race_e r = RACE_NONE )
    : fs_player_t( sim, name, r, ELARION ), target_data(), starfall_volleys()
  {
    resource_regeneration              = regen_type::DYNAMIC;
    regen_caches[ CACHE_HASTE ]        = true;
    regen_caches[ CACHE_ATTACK_HASTE ] = true;

    create_cooldowns();
  }
};

namespace actions
{  // namespace actions

template <typename T_ACTION>
struct elarion_action_state_t : public action_state_t
{
private:
  T_ACTION* action;

public:
  elarion_action_state_t( action_t* action, player_t* target )
    : action_state_t( action, target ), action( dynamic_cast<T_ACTION*>( action ) )
  {
  }

  elarion_t* p() const
  {
    return debug_cast<elarion_t*>( action->player );
  }

  elarion_t* p()
  {
    return debug_cast<elarion_t*>( action->player );
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
    const elarion_action_state_t* rs = debug_cast<const elarion_action_state_t*>( s );
  }

  T_ACTION* get_action() const
  {
    return action;
  }
};

template <typename Base>
class elarion_action_t : public Base
{
protected:
  /// typedef for elarion_action_t<action_base_t>
  using base_t = elarion_action_t<Base>;

private:
  /// typedef for the templated action type, eg. spell_t, attack_t, heal_t
  using ab = Base;

public:
  double lunarlight_salvo_chance_hit;
  double lunarlight_salvo_chance_crit;

  // Init =====================================================================
  elarion_action_t( util::string_view n, elarion_t* p, util::string_view options = {} )
    : ab( n, p, options ),
      lunarlight_salvo_chance_hit( p->spell_const.lunarlight_mark_chance_hit ),
      lunarlight_salvo_chance_crit( p->spell_const.lunarlight_mark_chance_crit )
  {
    ab::may_crit = ab::tick_may_crit = true;
    ab::school                       = SCHOOL_PHYSICAL;

    ab::resource_current = RESOURCE_FOCUS;
    // elarion_t sets base and min GCD to 1.5_s hasted
    ab::gcd_type = gcd_haste_type::ATTACK_HASTE;

    if ( p->legendary.new_spirit_legendary )
    {
      lunarlight_salvo_chance_hit += p->legendary.new_spirit_legendary_chance_to_consume_mark;
      lunarlight_salvo_chance_crit += p->legendary.new_spirit_legendary_chance_to_consume_mark;
    }
  }

  // Type Wrappers ============================================================

  static const elarion_action_state_t<base_t>* cast_state( const action_state_t* st )
  {
    return debug_cast<const elarion_action_state_t<base_t>*>( st );
  }

  static elarion_action_state_t<base_t>* cast_state( action_state_t* st )
  {
    return debug_cast<elarion_action_state_t<base_t>*>( st );
  }

  elarion_t* p()
  {
    return debug_cast<elarion_t*>( ab::player );
  }

  const elarion_t* p() const
  {
    return debug_cast<const elarion_t*>( ab::player );
  }

  elarion_td_t* td( player_t* t ) const
  {
    return p()->get_target_data( t );
  }

  // Action State =============================================================

  action_state_t* new_state() override
  {
    return new elarion_action_state_t<base_t>( this, ab::target );
  }

  // Helper Functions =========================================================

  // Helper function for expressions. Returns the number of guaranteed generated combo points for
  // this ability, taking into account any potential buffs.
  virtual double generate_focus() const
  {
    double cp = 0;

    if ( ab::energize_type != action_energize::NONE && ab::energize_resource == RESOURCE_FOCUS )
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

  double cost_pct_multiplier() const override
  {
    auto mul = ab::cost_pct_multiplier();

    // Note 04/01/2026 - Focused Expanse does not reduce cost by 50%
    if ( p()->buffs.event_horizon->check() )
      mul *= p()->spell_const.event_horizon_resource_mul;

    return mul;
  }

  double recharge_multiplier( const cooldown_t& cd ) const
  {
    double m = ab::recharge_multiplier( cd );

    if ( p()->buffs.event_horizon->check() )
    {
      m *= ab::player->cache.attack_haste();
    }

    return m;
  }


  void consume_resource() override
  {
    ab::consume_resource();

    spend_resource_costs( ab::execute_state );
  }

  

  void impact( action_state_t* s ) override
  {
    ab::impact( s );

    if ( ab::result_is_hit( s->result ) && s->result_amount > 0 )
    {
      auto td = p()->get_target_data( s->target );
      if ( td->debuffs.lunarlight_mark->check() )
      {
        if ( s->result == RESULT_HIT && ab::rng().roll( lunarlight_salvo_chance_hit ) )
        {
          p()->actions.lunarlight_salvo->execute_on_target( s->target );
          td->debuffs.lunarlight_mark->decrement();
        }

        if ( s->result == RESULT_CRIT && ab::rng().roll( lunarlight_salvo_chance_crit ) )
        {
          p()->actions.lunarlight_salvo->execute_on_target( s->target );
          td->debuffs.lunarlight_mark->decrement();
        }
      }
    }
  }

  std::unique_ptr<expr_t> create_expression( std::string_view name ) override
  {
    if ( util::str_compare_ci( name, "focus_gain" ) )
    {
      return make_mem_fn_expr( "focus_gain", *this, &base_t::generate_focus );
    }

    return ab::create_expression( name );
  }
};

struct elarion_heal_t : public elarion_action_t<fellowship::actions::fs_player_action_t<heal_t>>
{
  elarion_heal_t( util::string_view n, elarion_t* p, util::string_view o = {} ) : base_t( n, p, o )
  {
    harmful = false;
    set_target( p );
  }
};

struct elarion_spell_t : public elarion_action_t<fellowship::actions::fs_player_action_t<spell_t>>
{
  elarion_spell_t( util::string_view n, elarion_t* p, util::string_view o = {} ) : base_t( n, p, o )
  {
    school = SCHOOL_MAGIC;
  }
};

struct elarion_attack_t : public elarion_action_t<fellowship::actions::fs_player_action_t<ranged_attack_t>>
{
  elarion_attack_t( util::string_view n, elarion_t* p, util::string_view o = {} ) : base_t( n, p, o )
  {
  }
};

struct focused_shot_t : public elarion_attack_t
{
  focused_shot_t( elarion_t* p, util::string_view options_str = {} )
    : elarion_attack_t( "focused_shot", p, options_str )
  {
    id = 2;

    attack_power_mod.direct = p->spell_const.focused_shot_ap_coeff;

    name_str_reporting = "Focused Shot";

    base_execute_time = p->spell_const.focused_shot_cast_time;

    energize_amount   = p->spell_const.focused_shot_resource_gain;
    energize_type     = action_energize::ON_CAST;
    energize_resource = RESOURCE_FOCUS;

    parse_options( options_str );
  }

  void execute() override
  {
    base_t::execute();

    if ( p()->rppm.celestial_impetus->trigger() )
    {
      p()->buffs.celestial_impetus->trigger();
    }

    if ( p()->talents_enabled( elarion_t::TALENT_1 ) && rng().roll( p()->talents.focused_expanse_chance ) )
    {
      p()->buffs.focused_expanse->trigger();
    }
  }
};

struct celestial_shot_t : public elarion_attack_t
{
  celestial_shot_t( elarion_t* p, util::string_view options_str = {} )
    : elarion_attack_t( "celestial_shot", p, options_str )
  {
    id = 3;

    school = SCHOOL_MAGIC;

    attack_power_mod.direct = p->spell_const.celestial_shot_ap_coeff;

    name_str_reporting = "Celestial Shot";

    base_execute_time = 0_s;

    base_costs[ RESOURCE_FOCUS ] = p->spell_const.celestial_shot_focus_cost;

    parse_options( options_str );
  }

  void impact( action_state_t* s ) override
  {
    base_t::impact( s );

    if ( p()->buffs.celestial_impetus->check() )
    {
      p()->get_target_data( s->target )
          ->debuffs.lunarlight_mark->trigger( p()->spell_const.celestial_impetus_marks_applied );
    }
  }

  void execute() override
  {
    base_t::execute();

    if ( p()->buffs.celestial_impetus->check() )
    {
      if ( p()->talents_enabled( elarion_t::TALENT_14 ) )
      {
        p()->cooldowns.heartseeker_barrage->reset( false, 1 );
        p()->buffs.impending_heartseeker->trigger();
      }

      p()->buffs.celestial_impetus->decrement();
    }

    if ( p()->talents_enabled( elarion_t::TALENT_6 ) )
    {
      p()->cooldowns.heartseeker_barrage->adjust( -p()->talents.skyward_munitions_cdr );
      p()->cooldowns.highwind_arrow->adjust( -p()->talents.skyward_munitions_cdr );
    }
  }
};

struct multishot_t : public elarion_attack_t
{
  multishot_t( elarion_t* p, util::string_view options_str = {} ) : elarion_attack_t( "multishot", p, options_str )
  {
    id = 4;

    attack_power_mod.direct = p->spell_const.multishot_ap_coeff;
    aoe                     = -1;
    reduced_aoe_targets     = p->spell_const.multishot_target_falloff;

    name_str_reporting = "Multishot";

    base_execute_time = 0_s;

    base_costs[ RESOURCE_FOCUS ] = p->spell_const.multishot_focus_cost;

    parse_options( options_str );
  }

  void impact( action_state_t* s ) override
  {
    base_t::impact( s );

    if ( result_is_hit( s->result ) )
    {
      if ( p()->talent_enabled( elarion_t::TALENT_7 ) )
      {
        p()->cooldowns.starfall_volley->adjust( -p()->talents.repeating_stars_cdr );
      }
    }
  }

  bool is_empowered() const
  {
    return p()->buffs.skystriders_supremacy->check() || p()->buffs.focused_expanse->check();
  }

  double cost_pct_multiplier() const override
  {
    auto mul = base_t::cost_pct_multiplier();

    // Note 04/01/2026 - Focused Expanse does not reduce cost by 50%
    if ( is_empowered() )
      mul *= p()->spell_const.skystriders_supremacy_focus_mul;

    return mul;
  }

  size_t available_targets( std::vector<player_t*>& tl ) const override
  {
    base_t::available_targets( tl );

    if ( is_empowered() )
    {
      while ( tl.size() < p()->spell_const.skystriders_supremacy_minimum_arrows )
      {
        tl.push_back( target );
      }
    }

    return tl.size();
  }

  double composite_da_multiplier( const action_state_t* state ) const override
  {
    double m = base_t::composite_da_multiplier( state );

    if ( is_empowered() )
    {
      auto empowered_amp = 1.0 + p()->buffs.skystriders_supremacy->check_value();

      if ( p()->talents_enabled( elarion_t::TALENT_1 ) )
        empowered_amp += p()->talents.focused_expanse_amp;

      m *= empowered_amp;
    }

    return m;
  }

  void activate() override
  {
    base_t::activate();

    p()->buffs.focused_expanse->add_stack_change_callback( [ this ]( buff_t*, int old, int _new ) {
      if ( old && !_new || !old && _new )
      {
        target_cache.is_valid = false;
      }
    } );

    p()->buffs.skystriders_supremacy->add_stack_change_callback( [ this ]( buff_t*, int old, int _new ) {
      if ( old && !_new || !old && _new )
      {
        target_cache.is_valid = false;
      }
    } );
  }

  bool action_ready() override
  {
    if ( !( p()->buffs.focused_expanse->check() || p()->buffs.multishot->check() ||
            p()->buffs.skystriders_supremacy->check() ) )
      return false;

    return base_t::action_ready();
  }

  std::unique_ptr<expr_t> create_expression( std::string_view name ) override
  {
    if ( util::str_compare_ci( name, "is_empowered" ) )
    {
      return make_mem_fn_expr( "is_empowered", *this, &multishot_t::is_empowered );
    }

    return base_t::create_expression( name );
  }

  void execute() override
  {
    base_t::execute();

    if ( p()->talents_enabled( elarion_t::TALENT_6 ) )
    {
      p()->cooldowns.heartseeker_barrage->adjust( -p()->talents.skyward_munitions_cdr );
      p()->cooldowns.highwind_arrow->adjust( -p()->talents.skyward_munitions_cdr );
    }

    if ( p()->buffs.skystriders_supremacy->check() )
    {
      if ( p()->talents_enabled( elarion_t::TALENT_13 ) )
        p()->buffs.skystriders_supremacy->decrement();
    }
    else if ( p()->buffs.focused_expanse->check() )
    {
      p()->buffs.focused_expanse->decrement();
    }
    else
    {
      p()->buffs.multishot->decrement();
    }

    if ( p()->legendary.astronomers_hail )
    {
      for ( auto volley : p()->starfall_volleys )
      {
        /* volley->params->duration_ += p()->legendary.astronomers_hail_multishot_extend;*/

        // volley->_time_left
        // volley->extend_duration(p()->legendary.astronomers_hail_multishot_extend);
      }
    }
  }
};

struct highwind_arrow_t : public elarion_attack_t
{
  highwind_arrow_t( elarion_t* p, util::string_view options_str = {} )
    : elarion_attack_t( "highwind_arrow", p, options_str )
  {
    id = 5;

    attack_power_mod.direct = p->spell_const.highwind_arrow_ap_coeff;
    aoe                     = p->spell_const.highwind_arrow_targets;

    name_str_reporting = "Highwind Arrow";

    base_execute_time = p->spell_const.highwind_arrow_cast_time;

    base_costs[ RESOURCE_FOCUS ] = p->spell_const.highwind_arrow_focus_cost;

    cooldown->duration = p->spell_const.highwind_arrow_cooldown;
    cooldown->charges  = p->spell_const.highwind_arrow_charges;
    cooldown->hasted   = true;

    parse_options( options_str );
  }

  timespan_t execute_time() const override
  {
    if ( p()->buffs.resurgent_winds->check() )
    {
      return timespan_t::zero();
    }

    return base_t::execute_time();
  }

  size_t available_targets( std::vector<player_t*>& tl ) const override
  {
    base_t::available_targets( tl );

    if ( tl.size() > 2 )
    {
      std::sort( tl.begin() + 1, tl.end(), [ this ]( player_t* a, player_t* b ) {
        return p()->get_target_data( a )->debuffs.lunarlight_mark->check() >
               p()->get_target_data( b )->debuffs.lunarlight_mark->check();
      } );
    }

    return tl.size();
  }

  int n_targets() const override
  {
    return p()->buffs.final_crescendo->at_max_stacks() ? p()->talents.final_crescendo_ricochets : base_t::n_targets();
  }

  void impact( action_state_t* s ) override
  {
    base_t::impact( s );

    if ( result_is_hit( s->result ) )
    {
      if ( p()->legendary.shimmer )
      {
        p()->get_target_data( s->target )->debuffs.shimmer->trigger();
      }

      if ( p()->buffs.event_horizon->check() )
      {
        p()->cooldowns.heartseeker_barrage->adjust( -p()->spell_const.event_horizon_barrage_cdr_highwind );
      }
    }
  }

  double cost_pct_multiplier() const override
  {
    auto mul = base_t::cost_pct_multiplier();

    if ( p()->buffs.resurgent_winds->check() )
      mul = 0;

    return mul;
  }

  double composite_da_multiplier( const action_state_t* s ) const override
  {
    double m = base_t::composite_da_multiplier( s );

    if ( s->chain_target > 0 )
    {
      m *= p()->spell_const.highwind_arrow_cleave_mul;
    }

    if ( p()->buffs.final_crescendo->at_max_stacks() )
    {
      m *= 1.0 + p()->talents.final_crescendo_dmg_mul;
    }

    if ( p()->buffs.resurgent_winds->check() && p()->get_target_data( s->target )->debuffs.lunarlight_mark->check() )
    {
      m *= 1.0 + p()->talents.resurgent_winds_mul;
    }

    return m;
  }

  double composite_crit_chance() const override
  {
    double cc = base_t::composite_crit_chance();

    if ( p()->talents_enabled( elarion_t::TALENT_9 ) && rng().roll( p()->talents.lethal_shots_proc_chance ) )
    {
      cc += p()->talents.lethal_shots_added_cc;
    }

    return cc;
  }

  void update_ready( timespan_t cd_duration ) override
  {
    // Decrementing a stack of shadowy insight will consume a max charge. Consuming a max charge loses you a current
    // charge. Therefore update_ready needs to not be called in that case.
    if ( p()->buffs.resurgent_winds->up() )
    {
      p()->buffs.resurgent_winds->decrement();
    }
    else
    {
      base_t::update_ready( cd_duration );
    }
  }

  void reset() override
  {
    cooldown->charges = p()->spell_const.highwind_arrow_charges;

    base_t::reset();
  }

  void execute() override
  {
    base_t::execute();

    if ( p()->buffs.final_crescendo->at_max_stacks() )
    {
      p()->buffs.final_crescendo->expire();
    }
    else if ( p()->talents_enabled( elarion_t::TALENT_3 ) )
    {
      p()->buffs.final_crescendo->trigger();
    }

    if ( hit_any_target && execute_state->n_targets >= p()->spell_const.highwind_arrow_targets_for_multishot )
    {
      p()->buffs.multishot->trigger();
    }
  }
};

struct heartseeker_barrage_t : public elarion_attack_t
{
  struct heartseeker_barrage_arrow_t : public elarion_attack_t
  {
    heartseeker_barrage_arrow_t( elarion_t* p, util::string_view options_str = {} )
      : elarion_attack_t( "heartseeker_barrage_arrow", p, options_str )
    {
      id = 6;

      background = true;

      name_str_reporting = "Heartseeker Barrage";

      attack_power_mod.direct = p->spell_const.heartseeker_barrage_ap_coeff;
      aoe = p->talents_enabled( elarion_t::TALENT_5 ) ? 1 + p->talents.piercing_seekers_ricochet_targets : 0;

      if ( p->talents_enabled( elarion_t::TALENT_2 ) )
      {
        base_crit += p->talents.fusillade_crit;
      }
    }

    void impact( action_state_t* s ) override
    {
      base_t::impact( s );

      if ( result_is_hit( s->result ) )
      {
        if ( p()->buffs.event_horizon->check() && s->chain_target == 0 )
        {
          p()->cooldowns.starfall_volley->adjust( -p()->spell_const.event_horizon_volley_cdr_barrage );
        }
      }
    }

    double composite_da_multiplier( const action_state_t* s ) const override
    {
      double m = base_t::composite_da_multiplier( s );

      if ( s->chain_target > 0 )
      {
        m *= p()->talents.piercing_seekers_ricochet_mul;
      }

      return m;
    }
  };

  heartseeker_barrage_arrow_t* arrow;
  heartseeker_barrage_t( elarion_t* p, util::string_view options_str = {} )
    : elarion_attack_t( "heartseeker_barrage", p, options_str ), arrow( nullptr )
  {
    id = 6;

    name_str_reporting = "Heartseeker Barrage";

    dot_duration           = p->spell_const.heartseeker_barrage_duration;
    base_tick_time         = p->spell_const.heartseeker_barrage_period;
    hasted_ticks           = true;
    dot_allow_partial_tick = false;
    tick_on_application    = false;
    channeled              = true;

    if ( p->talents_enabled( elarion_t::TALENT_2 ) )
    {
      dot_duration += p->talents.fusillade_duration;
    }

    base_costs[ RESOURCE_FOCUS ] = p->spell_const.highwind_arrow_focus_cost;

    cooldown->duration = p->spell_const.heartseeker_barrage_cooldown;
    cooldown->hasted   = false;

    arrow = new heartseeker_barrage_arrow_t( p, options_str );

    add_child( arrow );

    parse_options( options_str );
  }


  /*double cost_pct_multiplier() const override
  {
    auto mul = base_t::cost_pct_multiplier();

    if ( p()->buffs.resurgent_winds->check() )
      mul = 0;

    return mul;
  }*/

  void tick( dot_t* d ) override
  {
    base_t::tick( d );

    arrow->set_target( d->target );
    action_state_t* damage_state = arrow->get_state();
    damage_state->target         = arrow->target;

    arrow->snapshot_state( damage_state, result_amount_type::DMG_DIRECT );

    if ( p()->buffs.impending_heartseeker->check() )
    {
      damage_state->da_multiplier *= 1.0 + d->current_tick * p()->talents.impending_heartseeker_mul_per_arrow;
    }

    arrow->schedule_execute( damage_state );
  }

  void last_tick( dot_t* d ) override
  {
    base_t::last_tick( d );

    p()->buffs.impending_heartseeker->expire();
  }

  void execute() override
  {
    base_t::execute();
  }
};

struct skystriders_supremacy_t : public elarion_spell_t
{
  skystriders_supremacy_t( elarion_t* p, util::string_view options_str = {} )
    : elarion_spell_t( "skystriders_supremacy", p, options_str )
  {
    id = 6;

    name_str_reporting = "Skystrider's Supremacy";

    trigger_gcd = 0_s;

    cooldown->duration = p->spell_const.skystriders_supremacy_cooldown;

    if ( p->talents_enabled( elarion_t::TALENT_13 ) )
      cooldown->duration -= p->talents.fervent_supremacy_reduced_cooldown;
    parse_options( options_str );
  }

  void execute() override
  {
    elarion_spell_t::execute();
    p()->buffs.skystriders_supremacy->trigger();
  }
};

struct skystriders_grace_t : public elarion_spell_t
{
  skystriders_grace_t( elarion_t* p, util::string_view options_str = {} )
    : elarion_spell_t( "skystriders_grace", p, options_str )
  {
    id = 7;

    name_str_reporting = "Skystrider's Grace";

    usable_while_casting = true;

    trigger_gcd = 0_s;

    cooldown->duration = p->spell_const.skystriders_grace_cooldown;
    parse_options( options_str );
  }

  void execute() override
  {
    elarion_spell_t::execute();
    p()->buffs.skystriders_grace->trigger();
  }
};

struct event_horizon_t : public elarion_spell_t
{
  event_horizon_t( elarion_t* p, util::string_view options_str = {} )
    : elarion_spell_t( "event_horizon", p, options_str )
  {
    id = 8;

    name_str_reporting = "Event Horizon";

    trigger_gcd = base_execute_time = p->spell_const.event_horizon_cast_time;

    resource_current              = RESOURCE_SPIRIT;
    base_costs[ RESOURCE_SPIRIT ] = 100;

    parse_options( options_str );
  }

  void execute() override
  {
    elarion_spell_t::execute();
    p()->fs_buffs.spirit_of_heroism->trigger();
    p()->buffs.event_horizon->trigger();
    p()->used_ultimate();
  }
};

struct lunarlight_salvo_t : public elarion_spell_t
{
  lunarlight_salvo_t( elarion_t* p )
    : elarion_spell_t( "lunarlight_salvo", p, {} )
  {
    id = 9;

    background = true;

    name_str_reporting = "Lunarlight Salvo";

    attack_power_mod.direct = p->spell_const.lunarlight_mark_ap_coeff;

    if ( p->talents_enabled( elarion_t::TALENT_11 ) )
    {
      base_multiplier *= 1.0 + p->talents.lunar_fury_mul;
    }
    if ( p->talents_enabled( elarion_t::TALENT_8 ) )
    {
      base_crit += p->talents.lunarlight_affinity_salvo_cc;
    }

    lunarlight_salvo_chance_hit = lunarlight_salvo_chance_crit = 0;
  }
};

struct lunarlight_mark_t : public elarion_spell_t
{
  lunarlight_mark_t( elarion_t* p, util::string_view options_str = {} )
    : elarion_spell_t( "lunarlight_mark", p, options_str )
  {
    id = 10;

    name_str_reporting = "Lunarlight Mark";

    trigger_gcd = 0_s;

    aoe = p->spell_const.lunarlight_mark_max_targets;

    cooldown->duration = p->spell_const.lunarlight_mark_cooldown;
    parse_options( options_str );
  }

  void impact( action_state_t* s ) override
  {
    elarion_spell_t::impact( s );
    if ( result_is_hit( s->result ) )
    {
      p()->get_target_data( s->target )
          ->debuffs.lunarlight_mark->trigger( p()->spell_const.lunarlight_mark_stacks_applied );
    }
  }

  void execute() override
  {
    elarion_spell_t::execute();

    if ( p()->talents_enabled( elarion_t::TALENT_15 ) )
    {
      p()->buffs.resurgent_winds->trigger();
    }
  }
};

struct lunarlight_mark_spirit_t : public elarion_spell_t
{
  lunarlight_mark_spirit_t( elarion_t* p, util::string_view options_str = {} )
    : elarion_spell_t( "lunarlight_mark_spirit_proc", p, options_str )
  {
    id = 10;

    name_str_reporting = "Lunarlight Mark Spirit";

    background = false;

    aoe = 1 + p->spell_const.spirit_refund_marks_extra_targets;

    parse_options( options_str );
  }

  void impact( action_state_t* s ) override
  {
    elarion_spell_t::impact( s );
    if ( result_is_hit( s->result ) )
    {
      p()->get_target_data( s->target )
          ->debuffs.lunarlight_mark->trigger( s->chain_target == 0 ? p()->spell_const.spirit_refund_marks_applied
                                                                   : p()->spell_const.spirit_refund_marks_cleave );
    }
  }
};


struct starfall_volley_damage_t : public elarion_attack_t
{
  starfall_volley_damage_t( elarion_t* p ) : elarion_attack_t( "starfall_volley_dmg", p, {} )
  {
    id = 11;

    school = SCHOOL_MAGIC;

    background = true;

    name_str_reporting = "Starfall Volley";

    aoe                     = -1;
    attack_power_mod.direct = p->spell_const.starfall_volley_ap_coeff;

    reduced_aoe_targets = p->spell_const.starfall_volley_target_falloff;

    if ( p->talents_enabled( elarion_t::TALENT_8 ) )
    {
      lunarlight_salvo_chance_hit *= 1.0 + p->talents.lunarlight_affinity_volley_chance_mul;
      lunarlight_salvo_chance_crit *= 1.0 + p->talents.lunarlight_affinity_volley_chance_mul;
    }
  }
};

struct starfall_volley_t : public elarion_spell_t
{
  ground_aoe_params_t aoe_params;

  starfall_volley_t( elarion_t* p, util::string_view options_str = {} )
    : elarion_spell_t( "starfall_volley", p, options_str )
  {
    id = 11;

    name_str_reporting = "Starfall Volley";

    cooldown->duration = p->spell_const.starfall_volley_cooldown;
    parse_options( options_str );

    if ( !p->actions.starfall_volley->stats->parent )
      add_child( p->actions.starfall_volley );

    aoe_params = ground_aoe_params_t()
                     .duration( p->legendary.astronomers_hail ? p->spell_const.starfall_volley_duration +
                                                                    p->legendary.astronomers_hail_volley_duration
                                                              : p->spell_const.starfall_volley_duration )
                     .pulse_time( p->spell_const.starfall_volley_period )
                     .hasted( ground_aoe_params_t::SPELL_HASTE )
                     .action( p->actions.starfall_volley )
                     .state_callback( [ this, p ]( ground_aoe_params_t::state_type type, ground_aoe_event_t* event ) {
                       switch ( type )
                       {
                         case ground_aoe_params_t::EVENT_STARTED:
                           p->buffs.starfall_volleys->increment();
                           break;
                         case ground_aoe_params_t::EVENT_STOPPED:
                           p->buffs.starfall_volleys->decrement();
                           break;
                         case ground_aoe_params_t::EVENT_CREATED:
                           p->starfall_volleys.insert( event );
                           break;
                         case ground_aoe_params_t::EVENT_DESTRUCTED:
                           p->starfall_volleys.erase( event );
                           break;
                         default:
                           break;
                       }
                     } );

    base_costs[ RESOURCE_FOCUS ] = p->spell_const.starfall_volley_focus_cost;
  }

  void execute() override
  {
    elarion_spell_t::execute();

    aoe_params.target( execute_state->target ).start_time( sim->current_time() );

    if ( sim->distance_targeting_enabled )
      aoe_params.x( execute_state->target->x_position ).y( execute_state->target->y_position );

    make_event<ground_aoe_event_t>( *sim, p(), aoe_params, true );
  }
};

}  // namespace actions

// ==========================================================================
// Rogue Targetdata Definitions
// ==========================================================================

elarion_td_t::elarion_td_t( player_t* target, elarion_t* source )
  : fellowship::fs_player_td_t( target, source ), dots(), debuffs()
{
  debuffs.lunarlight_mark = make_buff( *this, "lunarlight_mark" )
                                ->set_duration( source->spell_const.lunarlight_mark_duration )
                                ->set_max_stack( 30 );

  debuffs.shimmer = make_buff( *this, "shimmer" )
                        ->set_duration( source->legendary.shimmer_duration )
                        ->set_max_stack( source->legendary.shimmer_max_stacks )
                        ->set_default_value( source->legendary.shimmer_mul_per_stack );
}

// ==========================================================================
// Rogue Character Definition
// ==========================================================================

// elarion_t::composite_attribute_multiplier ==================================

double elarion_t::composite_attribute_multiplier( attribute_e a ) const
{
  double am = fs_player_t::composite_attribute_multiplier( a );

  return am;
}
// elarion_t::composite_spell_haste ==========================================

double elarion_t::composite_spell_haste() const
{
  double h = fs_player_t::composite_spell_haste();

  return h;
}

// elarion_t::composite_spell_crit_chance =========================================

double elarion_t::composite_spell_crit_chance() const
{
  double crit = fs_player_t::composite_spell_crit_chance();

  return crit;
}

// elarion_t::composite_damage_versatility ===================================

double elarion_t::composite_damage_versatility() const
{
  double cdv = fs_player_t::composite_damage_versatility();

  return cdv;
}

// elarion_t::composite_heal_versatility =====================================

double elarion_t::composite_heal_versatility() const
{
  double chv = fs_player_t::composite_heal_versatility();

  return chv;
}

// elarion_t::composite_leech ===============================================

double elarion_t::composite_leech() const
{
  double l = fs_player_t::composite_leech();

  return l;
}

// elarion_t::matching_gear_multiplier ========================================

double elarion_t::matching_gear_multiplier( attribute_e attr ) const
{
  return 0.0;
}

// elarion_t::composite_player_multiplier =====================================

double elarion_t::composite_player_multiplier( school_e school ) const
{
  double m = fs_player_t::composite_player_multiplier( school );

  if ( buffs.event_horizon->check() )
  {
    m *= 1.0 + spell_const.event_horizon_dmg_mul;
  }

  return m;
}

// elarion_t::composite_player_target_multiplier ==============================

double elarion_t::composite_player_target_multiplier( player_t* target, school_e school ) const
{
  double m = fs_player_t::composite_player_target_multiplier( target, school );

  elarion_td_t* tdata = get_target_data( target );

  m *= 1.0 + tdata->debuffs.shimmer->check_stack_value();

  return m;
}

// elarion_t::composite_player_target_crit_chance =============================

double elarion_t::composite_player_target_crit_chance( player_t* target ) const
{
  double c = fs_player_t::composite_player_target_crit_chance( target );

  if ( talents_enabled( TALENT_16 ) )
  {
    if ( target->health_percentage() <= talents.last_light_hp_pct )
      c += talents.last_lights_cc;
  }

  return c;
}

// elarion_t::composite_player_target_armor ===================================

double elarion_t::composite_player_target_armor( player_t* target ) const
{
  return 0.0;

  double a = fs_player_t::composite_player_target_armor( target );

  return a;
}
// elarion_t::init_actions ====================================================

void elarion_t::init_action_list()
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

// elarion_t::create_action  ==================================================

action_t* elarion_t::create_action( util::string_view name, util::string_view options_str )
{
  using namespace actions;

  if ( name == "focused_shot" )
    return new focused_shot_t( this, options_str );
  if ( name == "celestial_shot" )
    return new celestial_shot_t( this, options_str );
  if ( name == "multishot" )
    return new multishot_t( this, options_str );
  if ( name == "highwind_arrow" )
    return new highwind_arrow_t( this, options_str );
  if ( name == "heartseeker_barrage" )
    return new heartseeker_barrage_t( this, options_str );
  if ( name == "skystriders_grace" )
    return new skystriders_grace_t( this, options_str );
  if ( name == "skystriders_supremacy" )
    return new skystriders_supremacy_t( this, options_str );
  if ( name == "event_horizon" )
    return new event_horizon_t( this, options_str );
  if ( name == "lunarlight_mark" )
    return new lunarlight_mark_t( this, options_str );
  if ( name == "starfall_volley" )
    return new starfall_volley_t( this, options_str );

  return fs_player_t::create_action( name, options_str );
}

// elarion_t::create_expression ===============================================

std::unique_ptr<expr_t> elarion_t::create_action_expression( action_t& action, std::string_view name_str )
{
  // auto split = util::string_split<util::string_view>( name_str, "." );

  return fs_player_t::create_action_expression( action, name_str );
}

std::unique_ptr<expr_t> elarion_t::create_expression( util::string_view name_str )
{
  auto split = util::string_split<util::string_view>( name_str, "." );

  /*if ( split[ 0 ] == "temporal_overcharge" || split[ 0 ] == "to" || split[ 0 ] == "t_oc" || split[ 0 ] == "tmp_oc" ||
       split[ 0 ] == "oc" || split[ 0 ] == "overcharge" )
  {
    if ( split.size() == 1 )
    {
      return make_fn_expr( name_str, [ this ] { return this->current_focus( true ); } );
    }

    if ( split.size() == 2 && split[ 1 ] == "deficit" )
    {
      return make_fn_expr( name_str,
                           [ this ] { return resources.max[ RESOURCE_FOCUS ] - this->current_focus( true ); } );
    }
  }
  else*/
  if ( util::str_compare_ci( split[ 0 ], "talent" ) )
  {
    if ( split.size() == 2 )
    {
      for ( elarion_talents_t t = static_cast<elarion_talents_t>( 1U ); t < elarion_talents_t::MAX; t++ )
      {
        if ( util::str_compare_ci( split[ 1 ], talent_name( t ) ) )
        {
          return make_fn_expr( name_str, std::bind( std::mem_fn( &elarion_t::talents_enabled ), this, t ) );
        }
      }
    }
  }
  else if ( util::str_compare_ci( split[ 0 ], "legendary" ) )
  {
    if ( split.size() == 2 )
    {
      if ( util::str_compare_ci( split[ 1 ], "astronomers_hail" ) )
        return make_ref_expr( name_str, legendary.astronomers_hail );
      if ( util::str_compare_ci( split[ 1 ], "shimmer" ) )
        return make_ref_expr( name_str, legendary.shimmer );
      if ( util::str_compare_ci( split[ 1 ], "starstrikers_ascent" ) )
        return make_ref_expr( name_str, legendary.starstrikers_ascent );
    }
  }

  return fs_player_t::create_expression( name_str );
}

std::unique_ptr<expr_t> elarion_t::create_resource_expression( util::string_view name_str )
{
  return fs_player_t::create_resource_expression( name_str );
}

// elarion_t::init_base =======================================================

void elarion_t::init_base_stats()
{
  if ( base.distance < 1 )
    base.distance = 25;

  fs_player_t::init_base_stats();

  base.stats.attribute[ STAT_AGILITY ] = 1000;
  base.stats.attribute[ STAT_STAMINA ] = 1000;
  resources.base[ RESOURCE_HEALTH ]    = 17135;

  base.health_per_stamina = 39.081;

  resources.base[ RESOURCE_FOCUS ]                  = 100;
  resources.base_regen_per_second[ RESOURCE_FOCUS ] = 0.05 / 0.01;

  base_gcd = timespan_t::from_seconds( 1.5 );
  min_gcd  = timespan_t::from_seconds( 0.75 );
}

// elarion_t::init_spells =====================================================

void elarion_t::init_spells()
{
  fs_player_t::init_spells();

  // actions.auto_attack = new actions::auto_melee_attack_t( this, "" );
}

// elarion_t::init_talents ====================================================

void elarion_t::init_talents()
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

    for ( elarion_talents_t t = static_cast<elarion_talents_t>( 1U ); t < elarion_talents_t::MAX; t++ )
    {
      if ( util::str_compare_ci( talent_split[ 0 ], talent_name( t ) ) )
      {
        set_talent_points( t, ranks >= 1 );
        break;
      }
    }
  }
}

// elarion_t::init_gains ======================================================

void elarion_t::init_gains()
{
  fs_player_t::init_gains();

  gains.spirit_procs = get_gain( "Spirit Procs" );
  // gains.time_rift    = get_gain( "Time Rift" );
}

// elarion_t::init_rng ========================================================
void elarion_t::init_rng()
{
  fs_player_t::init_rng();

  rppm.celestial_impetus = get_rppm( "celestial_impetus", spell_const.celestial_impetus_ppm, 1.0, RPPM_HASTE );
}

// elarion_t::init_scaling ====================================================

void elarion_t::init_scaling()
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

// elarion_t::init_buffs ======================================================

void elarion_t::create_buffs()
{
  fs_player_t::create_buffs();

  buffs.celestial_impetus = make_buff<elarion_buff_t>( this, "celestial_impetus" )
                                ->set_max_stack( spell_const.celestial_impetus_max_stacks )
                                ->set_duration( spell_const.celestial_impetus_duration );

  buffs.final_crescendo = make_buff<elarion_buff_t>( this, "final_crescendo" )
                              ->set_max_stack( talents.final_crescendo_max_stacks )
                              ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

  buffs.focused_expanse = make_buff<elarion_buff_t>( this, "focused_expanse" )
                              ->set_max_stack( talents.focused_expanse_max_stacks )
                              ->set_duration( talents.focused_expanse_duration );

  buffs.impending_heartseeker = make_buff<elarion_buff_t>( this, "impending_heartseeker" )
                                    ->set_duration( talents.impending_heartseeker_duration );

  buffs.multishot = make_buff<elarion_buff_t>( this, "multishot" )
                        ->set_max_stack( spell_const.multishot_max_stacks )
                        ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

  buffs.resurgent_winds = make_buff<elarion_buff_t>( this, "resurgent_winds" )
                              ->set_duration( talents.resurgent_winds_duration )
                              ->set_max_stack( talents.resurgent_winds_maximum_stacks )
                              ->add_stack_change_callback( [ this ]( buff_t*, int old, int cur ) {
                                cooldowns.highwind_arrow->adjust_max_charges( cur - old );
                              } );

  buffs.skystriders_supremacy = make_buff<elarion_buff_t>( this, "skystriders_supremacy" )
                                    ->set_default_value( 0 )
                                    ->set_max_stack( 1 )
                                    ->set_duration( spell_const.skystriders_supremacy_duration );

  if ( talents_enabled( TALENT_13 ) )
  {
    buffs.skystriders_supremacy->set_max_stack( talents.fervent_supremacy_stacks )
        ->set_duration( talents.fervent_supremacy_duration )
        ->set_default_value( talents.fervent_supremacy_mul )
        ->set_max_stack( talents.fervent_supremacy_stacks )
        ->set_initial_stack( talents.fervent_supremacy_stacks );
  }

  struct skylit_grace_buff_t : elarion_buff_t
  {
    double cdr_mod;
    skylit_grace_buff_t( elarion_t* pl )
      : elarion_buff_t( pl, "skylit_grace" ), cdr_mod( 1 + pl->talents.skylit_grace_cdr )
    {
      set_max_stack( 99 );
      set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

      add_stack_change_callback( [ this ]( buff_t*, int old, int _new ) {
        if ( _new != old )
        {
          auto change         = _new - old;
          auto mod_difference = std::pow( cdr_mod, -change );

          if ( p()->cooldowns.skystriders_grace->action )
          {
            p()->cooldowns.skystriders_grace->action->base_recharge_rate_multiplier *= mod_difference;
            p()->cooldowns.skystriders_grace->adjust_recharge_multiplier();
          }
          else
          {
            for ( auto& action : p()->action_list )
            {
              if ( action->cooldown == p()->cooldowns.skystriders_grace )
              {
                action->base_recharge_rate_multiplier *= mod_difference;
                action->cooldown->adjust_recharge_multiplier();
              }
            }
          }
        }
      } );
    }
  };

  if ( talents_enabled( TALENT_4 ) )
  {
    buffs.starfall_volleys = make_buff<skylit_grace_buff_t>( this );
  }
  else
  {
    buffs.starfall_volleys = make_buff<elarion_buff_t>( this, "starfall_volleys" )
                                 ->set_max_stack( 99 )
                                 ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );
  }

  buffs.skystriders_grace = make_buff<elarion_buff_t>( this, "skystriders_grace" )
                                ->set_duration( spell_const.skystriders_grace_duration )
                                ->set_default_value( spell_const.skystriders_grace_haste_bonus )
                                ->set_pct_buff_type( STAT_PCT_BUFF_HASTE );

  buffs.event_horizon =
      make_buff<elarion_buff_t>( this, "event_horizon" )
          ->set_duration( spell_const.event_horizon_duration )
          ->add_stack_change_callback( [ this ]( buff_t*, int, int ) { adjust_dynamic_cooldowns(); } );
}

// elarion_t::invalidate_cache =========================================

void elarion_t::invalidate_cache( cache_e c )
{
  fs_player_t::invalidate_cache( c );
}

void elarion_t::create_options()
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

  add_option( opt_bool( "legendary.shimmer", legendary.shimmer ) );
  add_option( opt_bool( "legendary.astronomers_hail", legendary.astronomers_hail ) );
  add_option( opt_bool( "legendary.starstrikers_ascent", legendary.starstrikers_ascent ) );

  add_option( opt_int( "elarion.spirit_refund_marks_applied", spell_const.spirit_refund_marks_applied ) );

  add_option( opt_bool( "legendary.new_spirit_legendary", legendary.new_spirit_legendary ) );
}

// elarion_t::copy_from =======================================================

void elarion_t::copy_from( player_t* source )
{
  elarion_t* elarion = static_cast<elarion_t*>( source );
  fs_player_t::copy_from( source );

  talents     = elarion->talents;
  legendary   = elarion->legendary;
  options     = elarion->options;
  spell_const = elarion->spell_const;
}

// elarion_t::create_profile  =================================================

std::string elarion_t::create_profile( save_e stype )
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

// elarion_t::init_finished ===================================================

void elarion_t::init_finished()
{
  fs_player_t::init_finished();

  range::for_each( cooldown_list, [ this ]( cooldown_t* c ) {
    if ( !c->hasted )
    {
      dynamic_cooldown_list.push_back( c );
    }
  } );

  
}

void elarion_t::init_background_actions()
{
  fs_player_t::init_background_actions();

  actions.lunarlight_salvo        = new actions::lunarlight_salvo_t( this );
  actions.starfall_volley         = new actions::starfall_volley_damage_t( this );
  actions.lunarlight_marks_spirit = new actions::lunarlight_mark_spirit_t( this );
}

template <typename Base>
void actions::elarion_action_t<Base>::trigger_spirit_refund( const action_state_t* state, double resource_refund )
{
  make_event( ab::sim, 200_ms, [ resource_refund, this ] {
    p()->resource_gain( RESOURCE_FOCUS, resource_refund, p()->gains.spirit_procs, this );
    p()->sim->print_debug( "{} actually refunded {:.0f} Focus ", *p(), resource_refund );
  } );

  p()->spirit_refund();

  p()->actions.lunarlight_marks_spirit->execute_on_target( state->target );

  if ( p()->legendary.starstrikers_ascent )
  {
    p()->buffs.resurgent_winds->trigger();
  }
}

template <typename Base>
void actions::elarion_action_t<Base>::spend_resource_costs( const action_state_t* s )
{
  double focus_spent = s->action->base_costs[ RESOURCE_FOCUS ];
  if ( focus_spent <= 0 )
    return;

  if ( p()->rng().roll( p()->cache.mastery_value() ) )
  {
    p()->sim->print_debug( "{} proc'd Spirit Refund (Chance: {:.2f}%, Sprit: {:.2f}%)", *p(),
                           p()->cache.mastery_value() * 100.0, p()->cache.mastery() * 100.0 );

    trigger_spirit_refund( s, focus_spent );
  }
}

// elarion_t::convert_hybrid_stat ==============================================

stat_e elarion_t::convert_hybrid_stat( stat_e s ) const
{
  // this converts hybrid stats that either morph based on spec or only work
  // for certain specs into the appropriate "basic" stats
  switch ( s )
  {
    case STAT_STR_AGI_INT:
    case STAT_AGI_INT:
    case STAT_STR_AGI:
      return STAT_AGILITY;
    case STAT_STR_INT:
      return STAT_NONE;
    case STAT_BONUS_ARMOR:
      return STAT_NONE;
    default:
      return s;
  }
}

void elarion_t::create_cooldowns()
{
  cooldowns.heartseeker_barrage = get_cooldown( "heartseeker_barrage" );
  cooldowns.highwind_arrow      = get_cooldown( "highwind_arrow" );
  cooldowns.skystriders_grace   = get_cooldown( "skystriders_grace" );
  cooldowns.starfall_volley     = get_cooldown( "starfall_volley" );
}

class elarion_module_t : public module_t
{
public:
  elarion_module_t() : module_t( ELARION )
  {
  }

  player_t* create_player( sim_t* sim, util::string_view name, race_e r = RACE_NONE ) const override
  {
    return new elarion_t( sim, name, r );
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

}  // namespace elarion
}  // namespace fellowship

const module_t* module_t::elarion()
{
  static fellowship::elarion::elarion_module_t m;
  return &m;
}