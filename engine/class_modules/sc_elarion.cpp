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
    action_t* lunarlight_salvo_aoe;
    action_t* starfall_volley;
    action_t* lunarlight_marks_spirit;
    action_t* high_impact;
    action_t* precision_strike;

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
    buff_t* impending_heartseeker_channel;
    buff_t* stars_aligned;
    buff_t* strikers_aim;
  } buffs;

  struct cooldowns_t
  {
    cooldown_t* skystriders_grace;
    cooldown_t* starfall_volley;
    cooldown_t* heartseeker_barrage;
    cooldown_t* highwind_arrow;
    cooldown_t* lunarlight_mark;
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

  dbc_proc_callback_t* lunarlight_mark_external;

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

    double multishot_ap_coeff       = 2.415;
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

    double highwind_arrow_ap_coeff                    = 9.3;
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

    int spirit_refund_marks_applied       = 5;
    int spirit_refund_marks_cleave        = 2;
    int spirit_refund_marks_extra_targets = 2;

    double barrage_mark_aoe_chance = 0.2;
  } spell_const;

#define ELARION_TALENT_LIST(X) \
  X( FOCUSED_EXPANSE,        "focused_expanse",        "Focused Expanse" ) \
  X( PIERCING_SEEKERS,       "piercing_seekers",       "Piercing Seekers" ) \
  X( FINAL_CRESCENDO,        "final_crescendo",        "Final Crescendo" ) \
  X( SKYLIT_GRACE,           "skylit_grace",           "Skylit Grace" ) \
  X( FUSILLADE,              "fusillade",              "Fusillade" ) \
  X( SKYWARD_MUNITIONS,      "skyward_munitions",      "Skyward Munitions" ) \
  X( REPEATING_STARS,        "repeating_stars",        "Repeating Stars" ) \
  X( LUNARLIGHT_AFFINITY,    "lunarlight_affinity",    "Lunarlight Affinity" ) \
  X( LETHAL_SHOTS,           "lethal_shots",           "Lethal Shots" ) \
  X( PRECISION_STRIKE,       "precision_strike",       "Precision Strike" ) \
  X( LUNAR_FURY,             "lunar_fury",             "Lunar Fury" ) \
  X( STARS_ALIGNED,          "stars_aligned",          "Stars Aligned" ) \
  X( FERVENT_SUPREMACY,      "fervent_supremacy",      "Fervent Supremacy" ) \
  X( IMPENDING_HEARTSEEKER,  "impending_heartseeker",  "Impending Heartseeker" ) \
  X( RESURGENT_WINDS,        "resurgent_winds",        "Resurgent Winds" ) \
  X( LAST_LIGHTS,            "last_lights",            "Last Lights" ) \
  X( RISING_MOON,            "rising_moon",            "Rising Moon" ) \
  X( STRIKERS_AIM,           "strikers_aim",           "Striker's Aim" ) \
  X( SWIFT_RELOAD, "swift_reload", "Swift Reload" ) \
  X( DEADLY_FOCUS, "deadly_focus", "Deadly Focus" ) \
  X( HIGH_IMPACT,            "high_impact",            "High Impact" )

  enum elarion_talent_index_t
  {
#define X( name, id, pretty ) name##_INDEX,
    ELARION_TALENT_LIST( X )
#undef X
        ELARION_TALENT_MAX
  };

  enum elarion_talents_t : unsigned long long
  {
    NONE = 0,
#define X( name, id, pretty ) name = 1ULL << name##_INDEX,
    ELARION_TALENT_LIST( X )
#undef X
        MAX = 1ULL << ELARION_TALENT_MAX
  };

  static constexpr talent_info ELARION_TALENTS[] = {
#define X( name, id, pretty ) { elarion_talents_t::name, id, pretty },
      ELARION_TALENT_LIST( X )
#undef X
  };

  constexpr std::string_view talent_name( long long t ) override
  {
    for ( const auto& talent : ELARION_TALENTS )
      if ( talent.flag == t )
        return talent.id;

    return "unknown_talent";
  }

  constexpr std::string_view talent_name_formatted( long long t ) override
  {
    for ( const auto& talent : ELARION_TALENTS )
      if ( talent.flag == t )
        return talent.pretty;

    return "Unknown Talent";
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

    double lunarlight_affinity_volley_chance_mul = 0.0;
    double lunarlight_affinity_salvo_cc          = 0.2;

    double lethal_shots_proc_chance = 0.4;
    double lethal_shots_added_cc    = 1.0;

    double lunar_fury_mul                = 0.3;
    double lunar_fury_barrage_chance_mul = 1.0;

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

    timespan_t rising_moon_cdr = 2_s;

    double high_impact_ratio = 0.3;

    timespan_t stars_aligned_interval = 3_s;
    bool stars_aligned_hasted         = false;
    int stars_aligned_max_stacks      = 2;
    timespan_t stars_aligned_duration = 15_s;

    double precision_strike_effectiveness = 0.5;
    timespan_t precision_strike_delay     = 0.2_s;

    double strikers_aim_expertise    = 0.05;
    timespan_t strikers_aim_duration = 18_s;
    unsigned int strikers_aim_threshold       = 1;
    int strikers_aim_max_stacks      = 3;

    double deadly_focus_dmg_mod   = 1.0;
    double deadly_focus_focus_mod = -0.5;

    timespan_t swift_reload_cdr = 0.5_s;
  } talents;

  struct legendary_t
  {
    bool shimmer                 = false;
    timespan_t shimmer_duration  = 9_s;
    double shimmer_mul_per_stack = 0.1;
    int shimmer_max_stacks       = 2;

    bool starstrikers_ascent                                = false;
    int starstrikers_ascent_spirit_refunds_resurgent_stacks = 1;
    double starstrikers_ascent_chance                       = 0.5;

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


  struct starfall_volley_event_handler_t
  {
  private:
    elarion_t* elarion;

  public:
    player_t* current_target;
    event_t* tick_event;
    event_t* stars_aligned_event;
    event_t* end_event;
    double persistent_multiplier;
    int tick_number;
    bool active;

    starfall_volley_event_handler_t( elarion_t* p )
      : elarion( p ),
        current_target( nullptr ),
        tick_event( nullptr ),
        stars_aligned_event( nullptr ),
        end_event( nullptr ),
        persistent_multiplier( 1 ),
        tick_number( 0 ),
        active( false )
    {
    }

    elarion_t* p()
    {
      return elarion;
    }

    const elarion_t* p() const
    {
      return elarion;
    }

    timespan_t initial_duration()
    {
      timespan_t duration = p()->spell_const.starfall_volley_duration;

      if ( p()->legendary.astronomers_hail )
        duration += p()->legendary.astronomers_hail_volley_duration;

      return duration;
    }

    player_t* get_target()
    {
      if ( !current_target || current_target->is_sleeping() )
      {
        p()->actions.starfall_volley->target_cache.is_valid = false;

        if ( p()->actions.starfall_volley->target_list().size() < 1 )
        {
          return nullptr;
        }

        current_target = p()->rng().range( p()->actions.starfall_volley->target_list() );
      }

      return current_target;
    }

    void tick()
    {
      if ( !is_active() )
        return;

      player_t* t = get_target();

      assert( !t || t && !t->is_sleeping() );

      if ( !t )
      {
        reset();
        return;
      }

      auto dmg_action = p()->actions.starfall_volley;

      dmg_action->set_target( t );
      action_state_t* damage_state = dmg_action->get_state();
      damage_state->target         = dmg_action->target;

      dmg_action->snapshot_state( damage_state, result_amount_type::DMG_DIRECT );
      damage_state->persistent_multiplier = persistent_multiplier;

      dmg_action->schedule_execute( damage_state );

      tick_number++;

      //if ( p()->talents_enabled( elarion_t::STARS_ALIGNED ) )
      //  p()->buffs.stars_aligned->trigger();

      tick_event = make_event( p()->sim, tick_interval(), std::bind( std::mem_fn( &starfall_volley_event_handler_t::tick ), this ) );
    }

    void stars_aligned_tick()
    {
      if ( !is_active() )
        return;

      p()->buffs.stars_aligned->trigger();

      stars_aligned_event = make_event( p()->sim, stars_aligned_interval(),
                      std::bind( std::mem_fn( &starfall_volley_event_handler_t::stars_aligned_tick ), this ) );
    }

    timespan_t tick_interval() const
    {
      return p()->spell_const.starfall_volley_period * p()->cache.spell_haste();
    }

    timespan_t stars_aligned_interval() const
    {
      timespan_t interval = p()->talents.stars_aligned_interval;

      if ( p()->talents.stars_aligned_hasted )
        return interval * p()->cache.spell_haste();

      return interval;
    }

    void extend_duration( timespan_t extension )
    {
      assert( is_active() && "Attempted to extend a Starfall Volley that is not active" );

      timespan_t new_remains = extension + end_event->remains();
      
      p()->sim->print_debug( "{} extends Starfall Volley with extension {}. Previous remains: {}, new remains: {}", *p(), extension, end_event->occurs(), new_remains );

      if ( extension > 0_s )
      {
        end_event->reschedule( new_remains );
      }
      else
      {
        event_t::cancel( end_event );

        if ( new_remains > p()->sim->current_time() )
        {
          end_event = make_event( p()->sim, new_remains,
                                  std::bind( std::mem_fn( &starfall_volley_event_handler_t::expire ), this ) );
        }
        else
        {
          expire();
        }
      }
    }

    void expire()
    {
      p()->buffs.starfall_volleys->decrement();

      event_t::cancel( tick_event );
      event_t::cancel( stars_aligned_event );
      
      //erase_unordered( p()->active_starfall_volley_handlers, range::find( p()->active_starfall_volley_handlers, this ) );
      //p()->cached_starfall_volley_handlers.push_back( this );

      reset();
    }

    void reset()
    {
      stars_aligned_event = nullptr;
      tick_event          = nullptr;
      end_event           = nullptr;
      tick_number         = 0;
      active              = false;
    }

    bool is_active()
    {
      return active;
    }

    void start( const action_state_t* s )
    {
      reset();
      active = true;

      player_t* t = s->target;
      persistent_multiplier = s->persistent_multiplier;

      if ( t )
      {
        current_target = t;
      }

      assert( !tick_event && "Attempted to start an already active Starfall Volley" );

      p()->buffs.starfall_volleys->trigger();

      end_event = make_event( p()->sim, initial_duration(),
                              std::bind( std::mem_fn( &starfall_volley_event_handler_t::expire ), this ) );

      p()->sim->print_debug( "{} starts Starfall Volley with duration {}. Event Occurs at: {} in {}", *p(),
                             initial_duration(), end_event->occurs(), end_event->remains() );

      // ticks on application, so we don't need to schedule the first tick
      tick();

      if ( p()->talents_enabled( elarion_t::STARS_ALIGNED ) )
      {
        stars_aligned_event =
            make_event( p()->sim, stars_aligned_interval(),
                        std::bind( std::mem_fn( &starfall_volley_event_handler_t::stars_aligned_tick ), this ) );
      }
    }
  };

  auto_dispose<std::vector<starfall_volley_event_handler_t*>> starfall_volley_handlers;

  starfall_volley_event_handler_t* start_starfall_volley( action_state_t* s )
  {
    assert( s->target && !s->target->is_sleeping() && "Started a starfall volley on a null or sleeping enemy" );

    starfall_volley_event_handler_t* handler = nullptr;
    for ( auto it : starfall_volley_handlers )
    {
      if ( !it->is_active() )
        handler = it;
    }

    if ( !handler )
    {
      handler = new starfall_volley_event_handler_t( this );
      starfall_volley_handlers.push_back( handler );
    }

    handler->start( s );

    return handler;
  }

  // Character Definition
  void init_spells() override;
  void init_base_stats() override;
  void init_talents() override;
  void init_gains() override;
  void init_scaling() override;
  void init_finished() override;
  void init_background_actions() override;
  void init_special_effects() override;
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
    return ROLE_ATTACK;
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
  void reset() override;

  // ardeos_t::extend_starfall_volleys ========================================
  void extend_starfall_volleys( timespan_t extension );

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

  std::set<player_t*> starfall_volley_players;

  elarion_t( sim_t* sim, util::string_view name, race_e r = RACE_NONE )
    : fs_player_t( sim, name, r, ELARION ), target_data(), starfall_volley_players()
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
  bool trigger_lunarlight_without_consume;

  // Init =====================================================================
  elarion_action_t( util::string_view n, elarion_t* p, util::string_view options = {} )
    : ab( n, p, options ),
      lunarlight_salvo_chance_hit( p->spell_const.lunarlight_mark_chance_hit ),
      lunarlight_salvo_chance_crit( p->spell_const.lunarlight_mark_chance_crit ),
      trigger_lunarlight_without_consume( false )
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
    if ( ab::current_resource() == RESOURCE_FOCUS && p()->buffs.event_horizon->check() )
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

  virtual void handle_lunarlight_salvo( const action_state_t* s )
  {
    if ( ab::result_is_hit( s->result ) && s->result_amount > 0 )
    {
      auto td = p()->get_target_data( s->target );
      if ( td->debuffs.lunarlight_mark->check() )
      {
        if ( s->result == RESULT_HIT && ab::rng().roll( lunarlight_salvo_chance_hit ) )
        {
          p()->actions.lunarlight_salvo->execute_on_target( s->target );
          if ( !trigger_lunarlight_without_consume )
            td->debuffs.lunarlight_mark->decrement();
        }

        if ( s->result == RESULT_CRIT && ab::rng().roll( lunarlight_salvo_chance_crit ) )
        {
          p()->actions.lunarlight_salvo->execute_on_target( s->target );
          if ( !trigger_lunarlight_without_consume )
            td->debuffs.lunarlight_mark->decrement();
        }
      }
    }
  }

  void impact( action_state_t* s ) override
  {
    ab::impact( s );

    handle_lunarlight_salvo( s );
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

    ability_flags |= ability_type_e::ABILITY_BASIC;

    parse_options( options_str );
  }

  void init_finished() override
  {
    base_t::init_finished();

    if ( p()->talents_enabled( elarion_t::DEADLY_FOCUS ) )
    {
      energize_amount *= 1.0 + p()->talents.deadly_focus_focus_mod;
      attack_power_mod.direct *= 1.0 + p()->talents.deadly_focus_dmg_mod;
    }
  }

  void execute() override
  {
    base_t::execute();

    if ( p()->rppm.celestial_impetus->trigger() )
    {
      p()->buffs.celestial_impetus->trigger();
    }

    if ( p()->talents_enabled( elarion_t::FOCUSED_EXPANSE ) && rng().roll( p()->talents.focused_expanse_chance ) )
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

    ability_flags |= ability_type_e::ABILITY_BASIC;

    parse_options( options_str );
  }

  void init_finished() override
  {
    base_t::init_finished();

    if ( p()->talents_enabled( elarion_t::DEADLY_FOCUS ) )
    {
      attack_power_mod.direct *= 1.0 + p()->talents.deadly_focus_dmg_mod;
    }
  }

  double cost_pct_multiplier() const override
  {
    auto mul = base_t::cost_pct_multiplier();

    if ( p()->buffs.celestial_impetus->check() )
      mul = 0;

    return mul;
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
      if ( p()->talents_enabled( elarion_t::IMPENDING_HEARTSEEKER ) )
      {
        p()->cooldowns.heartseeker_barrage->reset( false, 1 );
        p()->buffs.impending_heartseeker->trigger();
      }

      p()->buffs.celestial_impetus->decrement();
    }

    if ( p()->talents_enabled( elarion_t::SKYWARD_MUNITIONS ) )
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

    ability_flags |= ability_type_e::ABILITY_CORE;

    parse_options( options_str );
  }

  void impact( action_state_t* s ) override
  {
    base_t::impact( s );

    if ( result_is_hit( s->result ) )
    {
      if ( p()->talent_enabled( elarion_t::REPEATING_STARS ) )
      {
        p()->cooldowns.starfall_volley->adjust( -p()->talents.repeating_stars_cdr );
      }
    }
  }

  bool is_empowered() const
  {
    return p()->buffs.skystriders_supremacy->check() || p()->buffs.focused_expanse->check() || p()->buffs.stars_aligned->check();
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

      if ( p()->talents_enabled( elarion_t::FOCUSED_EXPANSE ) )
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

    p()->buffs.stars_aligned->add_stack_change_callback( [ this ]( buff_t*, int old, int _new ) {
      if ( old && !_new || !old && _new )
      {
        target_cache.is_valid = false;
      }
    } );
  }

  bool action_ready() override
  {
    if ( !( is_empowered() || p()->buffs.multishot->check() ) )
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

    if ( p()->talents_enabled( elarion_t::SKYWARD_MUNITIONS ) )
    {
      p()->cooldowns.heartseeker_barrage->adjust( -p()->talents.skyward_munitions_cdr );
      p()->cooldowns.highwind_arrow->adjust( -p()->talents.skyward_munitions_cdr );
    }

    if ( p()->buffs.skystriders_supremacy->check() )
    {
      if ( p()->talents_enabled( elarion_t::FERVENT_SUPREMACY ) )
        p()->buffs.skystriders_supremacy->decrement();
    }
    else if ( p()->buffs.stars_aligned->check() )
    {
      p()->buffs.stars_aligned->decrement();
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
      p()->extend_starfall_volleys( p()->legendary.astronomers_hail_multishot_extend );
    }
  }
};

struct highwind_arrow_t : public elarion_attack_t
{
  struct high_impact_t : public elarion_spell_t
  {
    high_impact_t( elarion_t* p ) : elarion_spell_t( "high_impact", p )
    {
      id = 51;

      name_str_reporting = "High Impact";

      aoe = -1;
      may_crit = false;
      background = true;
    }

    void init_finished()
    {
      elarion_spell_t::init_finished();
      snapshot_flags &= ~( STATE_CRIT | STATE_TGT_CRIT | STATE_VERSATILITY | STATE_MUL_PERSISTENT | STATE_MUL_PLAYER_DAM );
    }
  };

  bool is_precision_strike;
  highwind_arrow_t( elarion_t* p, util::string_view options_str = {}, util::string_view name = "highwind_arrow",
                    bool is_precision_strike = false )
    : elarion_attack_t( name, p, options_str ), is_precision_strike( is_precision_strike )
  {
    id = 5;

    if ( is_precision_strike )
    {
      background = true;
      base_multiplier *= p->talents.precision_strike_effectiveness;
    }
    else
    {
      if ( p->talents_enabled( elarion_t::PRECISION_STRIKE ) )
      {
        if ( !p->actions.precision_strike->stats->parent )
          add_child( p->actions.precision_strike );
      }
    }

    attack_power_mod.direct = p->spell_const.highwind_arrow_ap_coeff;
    aoe                     = p->spell_const.highwind_arrow_targets;

    name_str_reporting = is_precision_strike ? "Precision Strike" : "Highwind Arrow";

    base_execute_time = p->spell_const.highwind_arrow_cast_time;

    base_costs[ RESOURCE_FOCUS ] = p->spell_const.highwind_arrow_focus_cost;

    cooldown->duration = p->spell_const.highwind_arrow_cooldown;
    cooldown->charges  = p->spell_const.highwind_arrow_charges;
    cooldown->hasted   = true;

    ability_flags |= ability_type_e::ABILITY_POWER;

    if ( p->talents_enabled( elarion_t::HIGH_IMPACT ) && !is_precision_strike )
    {
      if ( !p->actions.high_impact->stats->parent )
        add_child( p->actions.high_impact );
    }

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

  int n_targets() const override
  {
    return p()->buffs.final_crescendo->at_max_stacks() ? p()->talents.final_crescendo_ricochets : base_t::n_targets();
  }

  void impact( action_state_t* s ) override
  {
    base_t::impact( s );

    if ( result_is_hit( s->result ) )
    {
      if ( p()->talents_enabled( elarion_t::SWIFT_RELOAD ) )
      {
        p()->cooldowns.highwind_arrow->adjust( -p()->talents.swift_reload_cdr );
      }
      if ( s->chain_target == 0 && p()->talents_enabled( elarion_t::HIGH_IMPACT ) )
      {
        p()->actions.high_impact->execute_on_target( s->target, s->result_amount * p()->talents.high_impact_ratio );
      }

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

    if ( p()->talents_enabled( elarion_t::LETHAL_SHOTS ) && rng().roll( p()->talents.lethal_shots_proc_chance ) )
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
    auto& tl = target_list();

    if ( tl.size() > 2 )
    {
      std::sort( tl.begin() + 1, tl.end(), [ this ]( player_t* a, player_t* b ) {
        return p()->get_target_data( a )->debuffs.lunarlight_mark->check() >
               p()->get_target_data( b )->debuffs.lunarlight_mark->check();
      } );
    }

    base_t::execute();

    if ( !is_precision_strike && p()->talents_enabled( elarion_t::PRECISION_STRIKE ) && execute_state &&
         execute_state->n_targets == 1 )
    {
      auto precision_strike = p()->actions.precision_strike;

      action_state_t* damage_state = precision_strike->get_state();
      damage_state->target         = precision_strike->target;

      precision_strike->snapshot_state( damage_state, result_amount_type::DMG_DIRECT );

      make_event( p()->sim, p()->talents.precision_strike_delay, [ this, damage_state ] {
        p()->actions.precision_strike->set_target( execute_state->target );
        p()->actions.precision_strike->schedule_execute( damage_state );
      } );
    }

    if ( p()->talents_enabled( elarion_t::STRIKERS_AIM ) && execute_state && execute_state->n_targets <= p()->talents.strikers_aim_threshold )
    {
      p()->buffs.strikers_aim->trigger();
    }

    if ( !is_precision_strike )
    {
      if ( p()->buffs.final_crescendo->at_max_stacks() )
      {
        p()->buffs.final_crescendo->expire();
      }
      else if ( p()->talents_enabled( elarion_t::FINAL_CRESCENDO ) )
      {
        p()->buffs.final_crescendo->trigger();
      }

      if ( p()->talents_enabled( elarion_t::RISING_MOON ) )
      {
        p()->cooldowns.lunarlight_mark->adjust( -p()->talents.rising_moon_cdr );
      }
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
      aoe = p->talents_enabled( elarion_t::PIERCING_SEEKERS ) ? 1 + p->talents.piercing_seekers_ricochet_targets : 0;

      if ( p->talents_enabled( elarion_t::FUSILLADE ) )
      {
        base_crit += p->talents.fusillade_crit;
      }

      if ( p->talents_enabled( elarion_t::LUNAR_FURY ) )
      {
        lunarlight_salvo_chance_hit *= 1.0 + p->talents.lunar_fury_barrage_chance_mul;
        lunarlight_salvo_chance_crit *= 1.0 + p->talents.lunar_fury_barrage_chance_mul;
      }

      ability_flags |= ability_type_e::ABILITY_POWER;
    }

    void handle_lunarlight_salvo( const action_state_t* s ) override
    {
      if ( result_is_hit( s->result ) && s->result_amount > 0 )
      {
        auto td = p()->get_target_data( s->target );
        if ( td->debuffs.lunarlight_mark->check() )
        {
          action_t* lunar_light_action = rng().roll( p()->spell_const.barrage_mark_aoe_chance )
                                             ? p()->actions.lunarlight_salvo_aoe
                                             : p()->actions.lunarlight_salvo;

          if ( s->result == RESULT_HIT && rng().roll( lunarlight_salvo_chance_hit ) )
          {
            lunar_light_action->execute_on_target( s->target );
            td->debuffs.lunarlight_mark->decrement();
          }

          if ( s->result == RESULT_CRIT && rng().roll( lunarlight_salvo_chance_crit ) )
          {
            lunar_light_action->execute_on_target( s->target );
            td->debuffs.lunarlight_mark->decrement();
          }
        }
      }
    }

    void execute() override
    {
      auto& tl = target_list();

      if ( tl.size() > 1 )
      {
        std::sort( tl.begin() + 1, tl.end(), [ this ]( player_t* a, player_t* b ) {
          return p()->get_target_data( a )->debuffs.lunarlight_mark->check() >
                 p()->get_target_data( b )->debuffs.lunarlight_mark->check();
        } );

        // rng().shuffle( tl.begin() + 1, tl.end() );
      }

      base_t::execute();
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

    if ( p->talents_enabled( elarion_t::FUSILLADE ) )
    {
      dot_duration += p->talents.fusillade_duration;
    }

    base_costs[ RESOURCE_FOCUS ] = p->spell_const.heartseeker_barrage_focus_cost;

    cooldown->duration = p->spell_const.heartseeker_barrage_cooldown;
    cooldown->hasted   = false;

    arrow = new heartseeker_barrage_arrow_t( p, options_str );

    ability_flags |= ability_type_e::ABILITY_POWER;

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
    damage_state->persistent_multiplier = d->state->persistent_multiplier;

    if ( p()->buffs.impending_heartseeker_channel->check() )
    {
      damage_state->da_multiplier *= 1.0 + d->current_tick * p()->talents.impending_heartseeker_mul_per_arrow;
    }

    arrow->schedule_execute( damage_state );
  }

  void last_tick( dot_t* d ) override
  {
    base_t::last_tick( d );

    p()->buffs.impending_heartseeker_channel->expire();
  }

  void execute() override
  {
    if ( p()->buffs.impending_heartseeker->check() )
    {
      p()->buffs.impending_heartseeker_channel->trigger();
      p()->buffs.impending_heartseeker->expire();
    }
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

    if ( p->talents_enabled( elarion_t::FERVENT_SUPREMACY ) )
      cooldown->duration -= p->talents.fervent_supremacy_reduced_cooldown;
    parse_options( options_str );

    ability_flags |= ability_type_e::ABILITY_MAJOR;
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
    ability_flags |= ability_type_e::ABILITY_MAJOR;
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
    ability_flags |= ability_type_e::ABILITY_SPIRIT;
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
    ability_flags |= ability_type_e::ABILITY_MAJOR;

    if ( p->talents_enabled( elarion_t::LUNAR_FURY ) )
    {
      base_multiplier *= 1.0 + p->talents.lunar_fury_mul;
    }
    if ( p->talents_enabled( elarion_t::LUNARLIGHT_AFFINITY ) )
    {
      base_crit += p->talents.lunarlight_affinity_salvo_cc;
    }

    lunarlight_salvo_chance_hit = lunarlight_salvo_chance_crit = 0;
  }
};

struct lunarlight_salvo_aoe_t : public elarion_spell_t
{
  lunarlight_salvo_aoe_t( elarion_t* p ) : elarion_spell_t( "lunarlight_salvo_aoe", p, {} )
  {
    id = 9;

    background = true;

    name_str_reporting = "Lunarlight Salvo (AoE)";

    attack_power_mod.direct = p->spell_const.lunarlight_mark_ap_coeff;

    ability_flags |= ability_type_e::ABILITY_MAJOR;
    aoe = 12;

    if ( p->talents_enabled( elarion_t::LUNAR_FURY ) )
    {
      base_multiplier *= 1.0 + p->talents.lunar_fury_mul;
    }
    if ( p->talents_enabled( elarion_t::LUNARLIGHT_AFFINITY ) )
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
    ability_flags |= ability_type_e::ABILITY_MAJOR;
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

    if ( p()->talents_enabled( elarion_t::RESURGENT_WINDS ) )
    {
      p()->buffs.resurgent_winds->trigger();
    }
  }
};

struct lunarlight_mark_spirit_t : public elarion_spell_t
{
  lunarlight_mark_spirit_t( elarion_t* p )
    : elarion_spell_t( "lunarlight_mark_spirit_proc", p )
  {
    id = 10;

    name_str_reporting = "Lunarlight Mark Spirit";

    background = true;

    aoe = 1 + p->spell_const.spirit_refund_marks_extra_targets;

    ability_flags |= ability_type_e::ABILITY_MAJOR;
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

  void execute() override
  {
    target_cache.is_valid = false;
    elarion_spell_t::execute();
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

    if ( p->talents_enabled( elarion_t::LUNARLIGHT_AFFINITY ) )
    {
      trigger_lunarlight_without_consume = true;
      // lunarlight_salvo_chance_hit *= 1.0 + p->talents.lunarlight_affinity_volley_chance_mul;
      // lunarlight_salvo_chance_crit *= 1.0 + p->talents.lunarlight_affinity_volley_chance_mul;
    }
  }
};

struct starfall_volley_t : public elarion_spell_t
{
  starfall_volley_t( elarion_t* p, util::string_view options_str = {} )
    : elarion_spell_t( "starfall_volley", p, options_str )
  {
    id = 11;

    name_str_reporting = "Starfall Volley";

    cooldown->duration = p->spell_const.starfall_volley_cooldown;
    parse_options( options_str );

    if ( !p->actions.starfall_volley->stats->parent )
      add_child( p->actions.starfall_volley );


    base_costs[ RESOURCE_FOCUS ] = p->spell_const.starfall_volley_focus_cost;
    
    ability_flags |= ability_type_e::ABILITY_POWER;
  }

  void impact( action_state_t* s ) override
  {
    elarion_spell_t::impact( s );
    p()->start_starfall_volley( s );
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
                                ->set_max_stack( 20 );

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

  if ( talents_enabled( LAST_LIGHTS ) )
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

  base.stats.attribute[ STAT_AGILITY ] = 100;
  base.stats.attribute[ STAT_STAMINA ] = 100;
  resources.base[ RESOURCE_HEALTH ]    = 1713;

  base.health_per_stamina = 48.851;

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

  buffs.strikers_aim = make_buff<elarion_buff_t>( this, "strikers_aim" )
                           ->set_default_value( talents.strikers_aim_expertise )
                           ->set_pct_buff_type( STAT_PCT_BUFF_VERSATILITY )
                            ->set_max_stack( talents.strikers_aim_max_stacks )
                            ->set_duration( talents.strikers_aim_duration )
                           ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

  buffs.celestial_impetus = make_buff<elarion_buff_t>( this, "celestial_impetus" )
                                ->set_max_stack( spell_const.celestial_impetus_max_stacks )
                                ->set_duration( spell_const.celestial_impetus_duration );

  buffs.final_crescendo = make_buff<elarion_buff_t>( this, "final_crescendo" )
                              ->set_max_stack( talents.final_crescendo_max_stacks )
                              ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

  buffs.focused_expanse = make_buff<elarion_buff_t>( this, "focused_expanse" )
                              ->set_max_stack( talents.focused_expanse_max_stacks )
                              ->set_duration( talents.focused_expanse_duration );

  
  buffs.stars_aligned = make_buff<elarion_buff_t>( this, "stars_aligned" )
                              ->set_max_stack( talents.stars_aligned_max_stacks )
                              ->set_duration( talents.stars_aligned_duration );

  buffs.impending_heartseeker = make_buff<elarion_buff_t>( this, "impending_heartseeker" )
                                    ->set_duration( talents.impending_heartseeker_duration );

  buffs.impending_heartseeker_channel = make_buff<elarion_buff_t>( this, "impending_heartseeker_channel" )
                                            ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

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

  if ( talents_enabled( FERVENT_SUPREMACY ) )
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
      : elarion_buff_t( pl, "starfall_volleys" ), cdr_mod( 1 + pl->talents.skylit_grace_cdr )
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
            p()->cooldowns.skystriders_grace->action->dynamic_recharge_rate_multiplier *= mod_difference;
            p()->cooldowns.skystriders_grace->adjust_recharge_multiplier();
          }
          else
          {
            for ( auto& action : p()->action_list )
            {
              if ( action->cooldown == p()->cooldowns.skystriders_grace )
              {
                action->dynamic_recharge_rate_multiplier *= mod_difference;
                action->cooldown->adjust_recharge_multiplier();
              }
            }
          }
        }
      } );
    }
  };

  if ( talents_enabled( SKYLIT_GRACE ) )
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
          ->add_invalidate( CACHE_PLAYER_DAMAGE_MULTIPLIER )
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
  add_option( opt_float( "elarion.barrage_mark_aoe_chance", spell_const.barrage_mark_aoe_chance ) );
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
    if ( !c->hasted && c->action )
    {
      dynamic_cooldown_list.push_back( c );
    }
  } );
}

void elarion_t::init_special_effects()
{
  fs_player_t::init_special_effects();
  //``` auto effect      = new special_effect_t( this );
  //effect->spell_id     = 2297;
  //effect->name_str     = "ruby_storm";
  //effect->proc_flags_  = PF_ALL_DAMAGE | PF_PERIODIC;
  //effect->proc_flags2_ = PF2_ALL_HIT | PF2_PERIODIC_DAMAGE;
  //effect->cooldown_    = 0_s;
  //effect->ppm_         = -fs_weapon_trait_values.ruby_storm_ppm[ fs_weapons.ruby_storm ];
  //effect->rppm_scale_  = rppm_scale_e::RPPM_HASTE;
  //effect->rppm_blp_    = real_ppm_t::BLP_ENABLED;
  //effect->type         = special_effect_e::SPECIAL_EFFECT_EQUIP;
  //```


  auto effect = new special_effect_t( this );
  effect->spell_id = 9;
  effect->name_str = "lunarlight_salvo";
  effect->proc_flags_  = PF_ALL_DAMAGE | PF_PERIODIC;
  effect->proc_flags2_ = PF2_ALL_HIT | PF2_PERIODIC_DAMAGE;
  effect->cooldown_    = 0_s;
  effect->type         = special_effect_e::SPECIAL_EFFECT_EQUIP;
  effect->set_can_proc_from_procs( true );

  struct lunarlight_salvo_cb_t : dbc_proc_callback_t
  {
    elarion_t* elarion;

    double lunarlight_salvo_chance_hit = 0.25;
    double lunarlight_salvo_chance_crit = 0.5;
    lunarlight_salvo_cb_t( elarion_t* p, const special_effect_t& e )
      : dbc_proc_callback_t( p, e ),
        elarion( p ),
        lunarlight_salvo_chance_hit( p->spell_const.lunarlight_mark_chance_hit ),
        lunarlight_salvo_chance_crit( p->spell_const.lunarlight_mark_chance_crit )
    {
    } 
    
    void trigger( action_t* a, action_state_t* s ) override
    {
      // Elarion local IDs are currently all sub 100.
      if ( a->id < 100 )
        return;

      dbc_proc_callback_t::trigger( a, s );
    }

    void execute( action_t*, action_state_t* s ) override
    {
      if ( s->result_amount > 0 )
      {
        auto td = elarion->get_target_data( s->target );
        if ( td->debuffs.lunarlight_mark->check() )
        {
          if ( s->result == RESULT_HIT && rng().roll( lunarlight_salvo_chance_hit ) )
          {
            elarion->actions.lunarlight_salvo->execute_on_target( s->target );
            td->debuffs.lunarlight_mark->decrement();
          }

          if ( s->result == RESULT_CRIT && rng().roll( lunarlight_salvo_chance_crit ) )
          {
            elarion->actions.lunarlight_salvo->execute_on_target( s->target );
            td->debuffs.lunarlight_mark->decrement();
          }
        }
      }
    }
  };

  lunarlight_mark_external = new dbc_proc_callback_t( this, *effect );
  lunarlight_mark_external->initialize();
  lunarlight_mark_external->activate();
}

void elarion_t::init_background_actions()
{
  fs_player_t::init_background_actions();

  actions.lunarlight_salvo        = new actions::lunarlight_salvo_t( this );
  actions.lunarlight_salvo_aoe    = new actions::lunarlight_salvo_aoe_t( this );
  actions.starfall_volley         = new actions::starfall_volley_damage_t( this );
  actions.lunarlight_marks_spirit = new actions::lunarlight_mark_spirit_t( this );
  actions.high_impact             = new actions::highwind_arrow_t::high_impact_t( this );
  actions.precision_strike        = new actions::highwind_arrow_t( this, {}, "precision_stirke", true );
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

  if ( p()->legendary.starstrikers_ascent && p()->rng().roll( p()->legendary.starstrikers_ascent_chance ) )
  {
    make_event( ab::sim, 200_ms, [ resource_refund, this ] {
      p()->buffs.impending_heartseeker->trigger();
      p()->cooldowns.heartseeker_barrage->reset( true, 1 );
    } );
  }
}

template <typename Base>
void actions::elarion_action_t<Base>::spend_resource_costs( const action_state_t* s )
{
  //double focus_spent = s->action->base_costs[ RESOURCE_FOCUS ];
  if ( ab::last_resource_cost <= 0 || ab::current_resource() != RESOURCE_FOCUS )
    return;

  if ( p()->rng().roll( p()->cache.mastery_value() ) )
  {
    p()->sim->print_debug( "{} proc'd Spirit Refund (Chance: {:.2f}%, Sprit: {:.2f}%)", *p(),
                           p()->cache.mastery_value() * 100.0, p()->cache.mastery() * 100.0 );

    trigger_spirit_refund( s, ab::last_resource_cost );
  }
}

void elarion_t::reset()
{
  fs_player_t::reset();

  for ( auto*& volley : starfall_volley_handlers )
  {
    volley->reset();
  }
}

// elarion_t::extend_starfall_volleys ========================================
void elarion_t::extend_starfall_volleys( timespan_t extension )
{
  for ( auto*& volley : starfall_volley_handlers )
  {
    if ( volley->is_active() )
      volley->extend_duration( extension );
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
  cooldowns.lunarlight_mark     = get_cooldown( "lunarlight_mark" );
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