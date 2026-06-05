#pragma once
#include "fs_player.hpp"

#include "util/util.hpp"

#include "simulationcraft.hpp"

namespace fellowship
{  // UNNAMED NAMESPACE

// ==========================================================================
// FS Targetdata Definitions
// ==========================================================================

struct voidbringer_debuff_t : fs_player_buff_t
{
  double current_cap;
  voidbringer_debuff_t( player_t* target, fs_player_t* pl ) : fs_player_buff_t( target, pl, "voidbringer_accumulator" )
  {
    default_value = 0;

    set_duration( p()->fs_weapon_values.voidbringer_duration );
    add_stack_change_callback( [ this ]( buff_t*, int, int _new ) {
      if ( _new )
      {
        p()->active_voidbringer_buffs.push_back( this );
      }
    } );
  }

  bool trigger( int stacks, double value, double chance, timespan_t duration ) override
  {
    current_cap = p()->fs_weapon_values.voidbringer_cap *
                  std::max( p()->cache.attack_power(), p()->cache.spell_power( SCHOOL_SHADOW ) );

    return fs_player_buff_t::trigger( stacks, value, chance, duration );
  }

  void expire_override( int expiration_stacks, timespan_t remaining_duration ) override
  {
    fs_player_buff_t::expire_override( expiration_stacks, remaining_duration );

    auto it = range::find( p()->active_voidbringer_buffs, this );
    if ( it != p()->active_voidbringer_buffs.end() )
    {
      erase_unordered( p()->active_voidbringer_buffs, it );
    }

    p()->fs_actions.voidbringer_dmg->execute_on_target( player, current_value );
  }
};

struct aurastone_buff_t : fs_player_buff_t
{
  //double current_cap;
  //double max_pct_cap;
  aurastone_buff_t( player_t* target, fs_player_t* pl )
    : fs_player_buff_t( target, pl, "aurastone_accumulator" )/*,
      max_pct_cap( pl->fs_weapon_trait_values.sapphire_aurastone_cap[ pl->fs_weapons.sapphire_aurastone ] )*/
  {
    default_value = 0;

    buff_period = pl->fs_weapon_trait_values.sapphire_aura_period;

    set_tick_behavior( buff_tick_behavior::REFRESH );

    set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

    add_stack_change_callback( [ this ]( buff_t*, int, int _new ) {
      if ( _new )
      {
        p()->active_aurastone_buffs.push_back( this );
      }    } );

      p()->fs_buffs.spirit_of_heroism->add_stack_change_callback( [ this ]( buff_t*, int old, int _new ) {
      if ( !_new )
      {
        expire();
      }
    } );

    set_tick_callback( [ this ]( buff_t* b, int, timespan_t ) {
      if ( p()->fs_actions.aurastone_dmg->target_list().size() > 0 )
      {
        // current_cap = max_pct_cap * std::max( p()->cache.attack_power(), p()->cache.spell_power( SCHOOL_MAGIC ) );
        p()->fs_actions.aurastone_dmg->execute_on_target( player, b->current_value );
        b->current_value = 0;
      }
    } );
  }

  bool trigger( int stacks, double value, double chance, timespan_t duration ) override
  {
    // current_cap = max_pct_cap * std::max( p()->cache.attack_power(), p()->cache.spell_power( SCHOOL_MAGIC ) );

    return fs_player_buff_t::trigger( stacks, value, chance, duration );
  }

  void expire_override( int expiration_stacks, timespan_t remaining_duration ) override
  {
    fs_player_buff_t::expire_override( expiration_stacks, remaining_duration );

    auto it = range::find( p()->active_aurastone_buffs, this );
    if ( it != p()->active_aurastone_buffs.end() )
    {
      erase_unordered( p()->active_aurastone_buffs, it );
    }
  }
};

fs_player_td_t::fs_player_td_t( player_t* target, fs_player_t* source )
  : actor_target_data_t( target, source ), fs_dots(), debuffs(), buffs()
{
  if ( target->is_enemy() || target == source )
    debuffs.aurastone_accumulator = make_buff<aurastone_buff_t>( target, source );

  if ( target->is_enemy() )
  {
    fs_dots.curse_of_anzhyr    = target->get_dot( "curse_of_anzhyr", source );
    fs_dots.amethyst_splinters = target->get_dot( "amethyst_splinters", source );
    fs_dots.kindling           = target->get_dot( "kindling", source );

    debuffs.triggered_first_strike = make_buff( *this, "first_strike_triggered" );

    debuffs.diamond_strike_amp =
        make_buff( *this, "diamond_strike_amp" )->set_max_stack( 5 )->set_duration( 20_s )->set_default_value( 0.4 );

    debuffs.voidbringer_debuff    = make_buff<voidbringer_debuff_t>( target, source );
  }
  else
  {
    buffs.inspired_allegiance =
        make_buff<stat_buff_t>( *this, "inspired_allegiance" )
            ->add_stat(
                STAT_HASTE_RATING,
                source->fs_weapon_trait_values.inspired_allegiance_haste[ source->fs_weapons.inspired_allegiance ] )
            ->set_duration( 8_s );
  }
}

// ==========================================================================
// FS Character Definition
// ==========================================================================

fs_player_t::fs_player_t( sim_t* sim, util::string_view name, race_e r, player_e p )
  : player_t( sim, p, name, r ),
    target_data(),
    fs_gems(),
    fs_weapons(),
    weapon_cd( nullptr ),
    brave_machinations_available( false )
{
  // resource_regeneration              = regen_type::DYNAMIC;
  // regen_caches[ CACHE_HASTE ]        = true;
  // regen_caches[ CACHE_ATTACK_HASTE ] = true;
}

// fs_player_t::composite_attribute_multiplier ==================================

double fs_player_t::composite_attribute_multiplier( attribute_e a ) const
{
  double am = player_t::composite_attribute_multiplier( a );

  return am;
}

// fs_player_t::composite_melee_auto_attack_speed ===============================

double fs_player_t::composite_melee_auto_attack_speed() const
{
  double h = player_t::composite_melee_auto_attack_speed();

  return h;
}

// fs_player_t::composite_melee_haste ==========================================

double fs_player_t::composite_melee_haste() const
{
  return player_t::composite_melee_haste();
}

// fs_player_t::composite_spell_haste ==========================================

double fs_player_t::composite_spell_haste() const
{
  return player_t::composite_spell_haste();
}

// fs_player_t::composite_melee_crit_chance =========================================

double fs_player_t::composite_melee_crit_chance() const
{
  double crit = player_t::composite_melee_crit_chance();

  return crit;
}

double fs_player_t::composite_player_critical_damage_multiplier( const action_state_t* s ) const
{
  double cdm = player_t::composite_player_critical_damage_multiplier( s );

  if ( fs_gems.gem_powers[ GEM_AMETHYST ] >= GEM_TIER_10 )
  {
    cdm *= fs_gems.amethyst_crit_power_major;
  }
  else if ( fs_gems.gem_powers[ GEM_AMETHYST ] >= GEM_TIER_5 )
  {
    cdm *= fs_gems.amethyst_crit_power_minor;
  }

  return cdm;
}

// fs_player_t::composite_spell_crit_chance =========================================

double fs_player_t::composite_spell_crit_chance() const
{
  double crit = player_t::composite_spell_crit_chance();

  return crit;
}

// fs_player_t::composite_damage_versatility ===================================

double fs_player_t::composite_damage_versatility() const
{
  double cdv = player_t::composite_damage_versatility();

  return cdv;
}

// fs_player_t::composite_heal_versatility =====================================

double fs_player_t::composite_heal_versatility() const
{
  double chv = player_t::composite_heal_versatility();

  return chv;
}

// fs_player_t::composite_leech ===============================================

double fs_player_t::composite_leech() const
{
  double l = player_t::composite_leech();

  return l;
}

// fs_player_t::matching_gear_multiplier ========================================

double fs_player_t::matching_gear_multiplier( attribute_e attr ) const
{
  return 0.0;
}

// fs_player_t::composite_player_multiplier =====================================

double fs_player_t::composite_player_multiplier( school_e school ) const
{
  double m = player_t::composite_player_multiplier( school );

  if ( in_boss_encounter && fs_options.ruby_allow_boss_amp )
  {
    if ( fs_gems.gem_powers[ GEM_RUBY ] >= GEM_TIER_10 )
    {
      m *= fs_gems.ruby_boss_amp_major;
    }
    else if ( fs_gems.gem_powers[ GEM_RUBY ] >= GEM_TIER_5 )
    {
      m *= fs_gems.ruby_boss_amp_minor;
    }
  }

  return m;
}

// fs_player_t::composite_player_pet_damage_multiplier ==========================

double fs_player_t::composite_player_pet_damage_multiplier( const action_state_t* s, bool guardian ) const
{
  double m = player_t::composite_player_pet_damage_multiplier( s, guardian );

  return m;
}

// fs_player_t::composite_player_target_multiplier ==============================

double fs_player_t::composite_player_target_multiplier( player_t* target, school_e school ) const
{
  double m = player_t::composite_player_target_multiplier( target, school );

  if ( fs_sets.deaths_grasp && target->health_percentage() <= 35.0 )
  {
    m *= 1.0 + fs_sets.death_grasp_execute_amp;
  }

  return m;
}

// fs_player_t::composite_player_target_crit_chance =============================

double fs_player_t::composite_player_target_crit_chance( player_t* target ) const
{
  double c = player_t::composite_player_target_crit_chance( target );

  if ( fs_gems.gem_powers[ GEM_AMETHYST ] >= GEM_TIER_1 && target->health_percentage() >= fs_gems.amethyst_crit_threshold )
  {
    if ( fs_gems.gem_powers[ GEM_AMETHYST ] >= GEM_TIER_6 )
    {
      c += fs_gems.amethyst_crit_major;
    }
    else
    {
      c += fs_gems.amethyst_crit_minor;
    }
  }

  return c;
}

// fs_player_t::composite_player_target_armor ===================================

double fs_player_t::composite_player_target_armor( player_t* target ) const
{
  return 0.0;

  double a = player_t::composite_player_target_armor( target );

  return a;
}
// fs_player_t::init_actions ====================================================

void fs_player_t::init_action_list()
{
  if ( !action_list_str.empty() )
  {
    player_t::init_action_list();
    return;
  }

  clear_action_priority_lists();

  use_default_action_list = true;

  player_t::init_action_list();
}

namespace actions
{

struct amethyst_splinters_t : public residual_action::residual_periodic_action_t<fs_player_action_t<spell_t>>
{
  amethyst_splinters_t( util::string_view name, fs_player_t* p ) : residual_action_t( name, p )
  {
    id = 11471;

    background = true;

    name_str_reporting = "Amethyst Splinters";

    tick_may_crit = false;

    dot_duration           = 8_s;
    dot_behavior           = DOT_REFRESH_DURATION;
    base_tick_time         = 2_s;
    hasted_ticks           = true;
    dot_allow_partial_tick = true;

    base_multiplier *= p->fs_weapon_trait_values.amethyst_splinters_fraction[ p->fs_weapons.amethyst_splinters ];
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
  }
};

struct fated_strike_t : fs_weapon_action_t<attack_t>
{
  double st_mod       = 7.39;
  double cleave_mod   = 2.874;
  double cleave_ratio = cleave_mod / st_mod;

  fated_strike_t( util::string_view n, fs_player_t* p, util::string_view options = {} )
    : fs_weapon_action_t( n, p, options )
  {
    id = 20001;

    base_execute_time = trigger_gcd = min_gcd = 0.3_s;

    gcd_type                = gcd_haste_type::NONE;
    attack_power_mod.direct = st_mod;

    name_str_reporting = "Fated Strike";
    cooldown->duration = 60_s;
    cooldown->hasted   = false;
    cooldown->charges  = 1;

    aoe                 = -1;
    full_amount_targets = 1;
    reduced_aoe_targets = 3;

    if ( fs_p()->fs_weapons.equipped_weapon == FSWEAPON_FATED_STRIKE )
      active_weapon = true;

    parse_options( options );
  }

  double composite_da_multiplier( const action_state_t* s ) const override
  {
    double m = base_t::composite_da_multiplier( s );

    if ( s->chain_target != 0 )
    {
      m *= cleave_ratio;
    }

    return m;
  }

  void execute() override
  {
    base_t::execute();
    fs_p()->fs_buffs.fated_strike->trigger();
  }
};

struct chronoshift_t : fs_weapon_action_t<spell_t>
{
  struct chronoshift_pulse_t : fs_weapon_action_t<spell_t>
  {
    chronoshift_pulse_t( util::string_view n, fs_player_t* p ) : fs_weapon_action_t( n, p )
    {
      id                  = 1558;
      name_str_reporting  = "Chronoshift (Pulse)";
      background          = true;
      aoe                 = -1;
      school              = SCHOOL_ARCANE;
      reduced_aoe_targets = 5;

      spell_power_mod.direct = 5.769;

      if ( fs_p()->fs_weapons.equipped_weapon == FSWEAPON_CHRONOSHIFT )
        active_weapon = true;
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

    void execute() override
    {
      if ( target_list().size() > 1 )
      {
        target = target_list().front();
      }

      if ( pre_execute_state )
        pre_execute_state->target = target;

      base_t::execute();
    }
  };

  struct chronoshift_channel_t : fs_weapon_action_t<spell_t>
  {
    chronoshift_channel_t( util::string_view n, fs_player_t* p, util::string_view options = {} )
      : fs_weapon_action_t( n, p, options )
    {
      id = 1926;

      name_str_reporting = "Chronoshift";
      dot_duration       = 3.0_s;
      base_tick_time     = 1.5_s;

      channeled              = true;
      hasted_ticks           = true;
      tick_on_application    = true;
      dot_allow_partial_tick = true;
      may_crit               = false;

      aoe = 0;

      school             = SCHOOL_ARCANE;

      if ( fs_p()->fs_weapons.equipped_weapon == FSWEAPON_CHRONOSHIFT )
        active_weapon = true;

      tick_action = new chronoshift_pulse_t( "chronoshift_pulse", p );
      add_child( tick_action );

      parse_options( options );
    }

    void execute() override
    {
      target = player;
      fs_weapon_action_t::execute();
      fs_p()->fs_buffs.chronoshift->trigger();
    }

    void cancel() override
    {
      fs_weapon_action_t::cancel();
      fs_p()->fs_buffs.chronoshift->expire();
    }

    void last_tick( dot_t* d ) override
    {
      fs_weapon_action_t::last_tick( d );
      fs_p()->fs_buffs.chronoshift->expire();
    }
  };

  action_t* channel_action;
  chronoshift_t( util::string_view n, fs_player_t* p, util::string_view options = {} )
    : fs_weapon_action_t( n, p, options )
  {
    id = 1926;

    name_str_reporting = "Chronoshift";
    base_execute_time  = 1_s;

    cooldown->duration = 180_s;

    if ( fs_p()->fs_weapons.equipped_weapon == FSWEAPON_CHRONOSHIFT )
      active_weapon = true;


    channel_action = new chronoshift_channel_t( fmt::format( "{}_channel", n ), p, options );

    parse_options( options );
  }

  void execute() override
  {
    fs_weapon_action_t::execute();
    
    channel_action->execute();
  }
};

struct natures_fury_t : fs_weapon_action_t<spell_t>
{
  natures_fury_t( util::string_view n, fs_player_t* p, util::string_view options = {} )
    : fs_weapon_action_t( n, p, options )
  {
    id = 161;

    name_str_reporting = "Nature's Fury";
    school             = SCHOOL_NATURE;
    base_execute_time  = 2_s;
    base_crit += 0.30;

    spell_power_mod.direct = 13.977;

    aoe                 = 4;

    cooldown->duration = 60_s;

    if ( fs_p()->fs_weapons.equipped_weapon == FSWEAPON_NATURES_FURY )
      active_weapon = true;

    parse_options( options );
  }

  double composite_da_multiplier(const action_state_t* s) const override
  {
    auto m = base_t::composite_da_multiplier( s );

    if ( s->chain_target == 0 )
      m *= 2;

    return m;
  }
};

struct earthbreaker_t : fs_weapon_action_t<spell_t>
{
  struct earthbreaker_reapeating_t : fs_weapon_action_t<spell_t>
  {
    earthbreaker_reapeating_t( util::string_view n, fs_player_t* p ) : fs_weapon_action_t( n, p )
    {
      id                 = 30203;
      name_str_reporting = "Earthbreaker (Repeating)";
      school             = SCHOOL_NATURE;

      spell_power_mod.direct = p->fs_weapon_values.earthbreaker_repeating_hit_coeff;

      reduced_aoe_targets = p->fs_weapon_values.earthbreaker_target_falloff;
      aoe        = -1;
      background = true;
    }
  };

  struct earthbreaker_final_t : fs_weapon_action_t<spell_t>
  {
    earthbreaker_final_t( util::string_view n, fs_player_t* p ) : fs_weapon_action_t( n, p )
    {
      id                 = 30204;
      name_str_reporting = "Earthbreaker (Final)";
      school             = SCHOOL_NATURE;

      spell_power_mod.direct = p->fs_weapon_values.earthbreaker_final_hit_coeff;

      reduced_aoe_targets = p->fs_weapon_values.earthbreaker_target_falloff;
      aoe                 = -1;
      background          = true;
    }
  };

  earthbreaker_reapeating_t* repeating_action;
  earthbreaker_final_t* final_action;

  earthbreaker_t( util::string_view n, fs_player_t* p, util::string_view options = {} )
    : fs_weapon_action_t( n, p, options ), repeating_action( nullptr ), final_action( nullptr )
  {
    id                 = 30201;
    name_str_reporting = "Earthbreaker";
    cooldown->duration = p->fs_weapon_values.earthbreaker_cooldown;

    spell_power_mod.direct = p->fs_weapon_values.earthbreaker_initial_hit_coeff;

    reduced_aoe_targets = p->fs_weapon_values.earthbreaker_target_falloff;
    aoe                 = -1;

    repeating_action = new earthbreaker_reapeating_t( "earthbreaker_repeating", p );
    final_action     = new earthbreaker_final_t( "earthbreaker_final", p );

    add_child( repeating_action );
    add_child( final_action );

    if ( fs_p()->fs_weapons.equipped_weapon == FSWEAPON_EARTHBREAKER )
      active_weapon = true;

    parse_options( options );
  }

  void execute() override
  {
    fs_weapon_action_t::execute();

    make_event<ground_aoe_event_t>(
        *sim, fs_p(),
        ground_aoe_params_t()
            .target( execute_state->target )
            .action( repeating_action )
            .pulse_time( fs_p()->fs_weapon_values.earthbreaker_period )
            .duration( fs_p()->fs_weapon_values.earthbreaker_duration )
            .hasted( ground_aoe_params_t::hasted_with::SPELL_HASTE )
            .state_callback( [ this ]( ground_aoe_params_t::state_type type, ground_aoe_event_t* e ) {
              if ( type == ground_aoe_params_t::EVENT_STOPPED )
              {
                final_action->execute();
              }
            } ),
        false );
  }
};

struct icicles_of_anzhyr_t : fs_weapon_action_t<spell_t>
{
  struct curse_of_anzhyr_t : fs_weapon_action_t<spell_t>
  {
    curse_of_anzhyr_t( util::string_view n, fs_player_t* p, util::string_view options = {} )
      : fs_weapon_action_t( n, p, options )
    {
      id                 = 1561;
      name_str_reporting = "Curse of An'zhyr";
      school             = SCHOOL_FROST;

      spell_power_mod.tick = 0.384;
      base_tick_time       = 3.0_s;
      dot_duration         = sim->expected_iteration_time > 0_ms ? 2 * sim->expected_iteration_time
                                                                   : 2 * sim->max_time * ( 1.0 + sim->vary_combat_length );
      hasted_ticks           = true;
      tick_may_crit          = true;
      background             = true;
      dot_allow_partial_tick = true;

      aoe = -1;

      parse_options( options );
    }

    void init() override
    {
      fs_weapon_action_t::init();

      update_flags &= ~STATE_CRIT;
    }
  };

  struct icicles_of_anzhyr_wave_t : fs_weapon_action_t<spell_t>
  {
    icicles_of_anzhyr_wave_t( util::string_view n, fs_player_t* p ) : fs_weapon_action_t( n, p )
    {
      id                 = 20003;
      name_str_reporting = "Icicles of An'zhyr (Wave)";
      school             = SCHOOL_FROST;

      spell_power_mod.direct = 1.44;

      reduced_aoe_targets = 12;
      aoe                 = -1;
      background          = true;
    }

    double composite_da_multiplier( const action_state_t* s ) const override
    {
      double m = fs_weapon_action_t::composite_da_multiplier( s );
      const fs_player_td_t* td = fs_p()->find_target_data( s->target );

      if ( td && td->fs_dots.curse_of_anzhyr && td->fs_dots.curse_of_anzhyr->is_ticking() )
      {
        m *= 3.0;
      }

      return m;
    }
  };

  icicles_of_anzhyr_wave_t* wave_action;
  curse_of_anzhyr_t* dot_action;

  icicles_of_anzhyr_t( util::string_view n, fs_player_t* p, util::string_view options = {} )
    : fs_weapon_action_t( n, p, options )
  {
    id                 = 1932;
    name_str_reporting = "Icicles of An'zhyr";
    cooldown->duration = 40_s;

    wave_action  = new icicles_of_anzhyr_wave_t( "icicles_of_anzhyr_wave", p );
    dot_action  = new curse_of_anzhyr_t( "curse_of_anzhyr", p );

    add_child( wave_action );
    add_child( dot_action );

    if ( fs_p()->fs_weapons.equipped_weapon == FSWEAPON_ICICLES_OF_ANZHYR )
      active_weapon = true;

    parse_options( options );
  }

  void execute() override
  {
    fs_weapon_action_t::execute();

    make_event<ground_aoe_event_t>(
        *sim, fs_p(),
        ground_aoe_params_t()
            .target( execute_state->target )
            .action( wave_action )
            .pulse_time( 1.0_s )
            .n_pulses( 3 )
            .state_callback( [ this ]( ground_aoe_params_t::state_type type, ground_aoe_event_t* e ) {
              if ( e && e->params && e->current_pulse == 3 )
              {
                dot_action->execute();
              }
            } ),
        true );
  }
};

struct voidbringers_touch_t : fs_weapon_action_t<spell_t>
{
  voidbringers_touch_t( util::string_view n, fs_player_t* p, util::string_view options = {} )
    : fs_weapon_action_t( "voidbringers_touch", p, options )
  {
    id = 12712;

    base_execute_time = trigger_gcd = min_gcd = 0_s;
    gcd_type = gcd_haste_type::NONE;

    name_str_reporting = "Voidbringer's Touch";
    school             = SCHOOL_SHADOW;

    aoe = 0;

    cooldown->duration = 90_s;

    if ( fs_p()->fs_weapons.equipped_weapon == FSWEAPON_VOIDBRINGERS_TOUCH )
      active_weapon = true;

    parse_options( options );

    if ( p->fs_actions.voidbringer_dmg )
      add_child( p->fs_actions.voidbringer_dmg );
  }

  void impact( action_state_t* state ) override
  {
    fs_weapon_action_t::impact( state );

    auto td = fs_p()->find_target_data( state->target );
    if ( td )
    {
      td->debuffs.voidbringer_debuff->trigger();
    }
  }
};

struct voidbringers_touch_dmg_t : fs_weapon_action_t<spell_t>
{
  voidbringers_touch_dmg_t( util::string_view n, fs_player_t* p )
    : fs_weapon_action_t( n, p )
  {
    id = 12713;

    name_str_reporting = "Voidbringer's Touch";
    school             = SCHOOL_SHADOW;

    aoe = 0;
    background = true;

    base_crit += 1.0;

    if ( fs_p()->fs_weapons.equipped_weapon == FSWEAPON_VOIDBRINGERS_TOUCH )
      active_weapon = true;
  }

  void init_finished() override
  {
    base_t::init_finished();
    snapshot_flags |= STATE_TARGET_NO_PET | STATE_CRIT | STATE_VERSATILITY | STATE_MUL_DA | STATE_MUL_PLAYER_DAM |
                      STATE_MUL_PERSISTENT;
  }
};

struct sapphire_aurastone_dmg_t : fs_player_action_t<spell_t>
{
  sapphire_aurastone_dmg_t( util::string_view n, fs_player_t* p ) : fs_player_action_t( n, p )
  {
    id = 127156;

    name_str_reporting = "Sapphire Aurastone";
    school             = SCHOOL_MAGIC;
    may_miss            = false;
    may_crit            = false;
    aoe                 = -1;
    background          = true;
    split_aoe_damage    = true;
  }

  void init_finished() override
  {
    base_t::init_finished();
    snapshot_flags &= STATE_NO_MULTIPLIER;
  }
};

struct finesse_f_dmg_t : fs_proc_spell_t
{
  finesse_f_dmg_t( util::string_view n, fs_player_t* p ) : fs_proc_spell_t( n, p )
  {
    id = 2589871;

    name_str_reporting = "Finesse F";
    school             = SCHOOL_MAGIC;
    background         = true;

    spell_power_mod.direct = p->finesse_trait_values.finesse_f_drain[ p->finesse_traits[ FINESSE_F ] ];
    ability_flags |= ability_type_e::ABILITY_CORE;
  }
};

struct finesse_l_dmg_t : fs_proc_spell_t
{
  finesse_l_dmg_t( util::string_view n, fs_player_t* p ) : fs_proc_spell_t( n, p )
  {
    id = 2589872;

    name_str_reporting = "Finesse L";
    school             = SCHOOL_MAGIC;
    background         = true;
    aoe                = p->finesse_trait_values.finesse_l_targets;

    spell_power_mod.direct = p->finesse_trait_values.finesse_l_dmg[ p->finesse_traits[ FINESSE_L ] ];

    ability_flags |= ability_type_e::ABILITY_CORE;
  }
};

struct sahrils_wrath_t : fs_weapon_action_t<spell_t>
{
  sahrils_wrath_t( util::string_view n, fs_player_t* p, util::string_view options = {} )
    : fs_weapon_action_t( n, p, options )
  {
    id = 161123;

    name_str_reporting = "Sahril's Wrath";
    school             = SCHOOL_FIRE;

    spell_power_mod.direct = 21.09;

    aoe = -1;

    base_execute_time = trigger_gcd = min_gcd = 0_s;
    gcd_type                                  = gcd_haste_type::NONE;

    reduced_aoe_targets = 1;

    cooldown->duration = 120_s;

    if ( fs_p()->fs_weapons.equipped_weapon == FSWEAPON_SAHRILS_WRATH )
      active_weapon = true;

    parse_options( options );
  }


  void execute() override
  {
    fs_p()->fs_buffs.sundering_wrath->trigger( as<int>( target_list().size() ) );
    base_t::execute();
  }
};

struct alzeracs_essence_t : fs_relic_action_t<attack_t>
{
  alzeracs_essence_t( util::string_view n, fs_player_t* p, util::string_view options = {} )
    : fs_relic_action_t( n, p, options )
  {
    id = 40001;

    base_execute_time = trigger_gcd = min_gcd = 0_s;

    gcd_type                = gcd_haste_type::NONE;

    name_str_reporting = "Alzerac's Essence";
    cooldown->duration = p->fs_relic_values.alzeracs_cd;
    cooldown->hasted   = false;
    cooldown->charges  = 1;

    if ( fs_p()->fs_relics.relic1 == FSRELIC_ALZERACS_ESSENCE || fs_p()->fs_relics.relic2 == FSRELIC_ALZERACS_ESSENCE )
      usable_relic = true;

    parse_options( options );
  }

  void execute() override
  {
    base_t::execute();
    fs_p()->resource_gain(
        RESOURCE_MANA, fs_p()->resources.max[ RESOURCE_MANA ] * fs_p()->fs_relic_values.alzeracs_mana_pct, gain, this );
  }
};


}  // namespace actions

// fs_player_t::create_action  ==================================================

action_t* fs_player_t::create_action( util::string_view name, util::string_view options_str )
{
  using namespace actions;

  if ( name == "fated_strike" )
    return new fated_strike_t( name, this, options_str );
  if ( name == "chronoshift" )
    return new chronoshift_t( name, this, options_str );
  if ( name == "natures_fury" )
    return new natures_fury_t( name, this, options_str );
  if ( name == "icicles_of_anzhyr" || name == "icicles" )
    return new icicles_of_anzhyr_t( name, this, options_str );
  if ( name == "voidbringer" || name == "voidbringers" || name == "voidbringers_touch" )
    return new voidbringers_touch_t( name, this, options_str );
  if ( name == "sahrils_wrath" || name == "sahrils" || name == "sahril" )
    return new sahrils_wrath_t( name, this, options_str );
  if ( name == "alzeracs_essence" )
    return new alzeracs_essence_t( name, this, options_str );
  if ( name == "earthbreaker" )
    return new earthbreaker_t( name, this, options_str );

  return player_t::create_action( name, options_str );
}

// fs_player_t::create_expression ===============================================

std::unique_ptr<expr_t> fs_player_t::create_action_expression( action_t& action, std::string_view name_str )
{
  auto split = util::string_split<util::string_view>( name_str, "." );

  return player_t::create_action_expression( action, name_str );
}

std::unique_ptr<expr_t> fs_player_t::create_expression( util::string_view name_str )
{
  auto split = util::string_split<util::string_view>( name_str, "." );

  if ( split.size() == 2 )
  {
    if ( util::str_compare_ci( split[ 0 ], "sets" ) )
    {
      if ( util::str_compare_ci( split[ 1 ], "dark_prophecy" ) )
        return make_ref_expr( name_str, fs_sets.dark_prophecy );
      else if ( util::str_compare_ci( split[ 1 ], "deaths_grasp" ) )
        return make_ref_expr( name_str, fs_sets.deaths_grasp );
      else if ( util::str_compare_ci( split[ 1 ], "draconic_might" ) )
        return make_ref_expr( name_str, fs_sets.draconic_might );
      else if ( util::str_compare_ci( split[ 1 ], "drakheims_absolution" ) )
        return make_ref_expr( name_str, fs_sets.drakheims_absolution );
      else if ( util::str_compare_ci( split[ 1 ], "eldrin_deceit" ) )
        return make_ref_expr( name_str, fs_sets.eldrin_deceit );
      else if ( util::str_compare_ci( split[ 1 ], "eldrin_fury" ) )
        return make_ref_expr( name_str, fs_sets.eldrin_fury );
      else if ( util::str_compare_ci( split[ 1 ], "haunting_lament" ) )
        return make_ref_expr( name_str, fs_sets.haunting_lament );
      else if ( util::str_compare_ci( split[ 1 ], "sin_warding" ) )
        return make_ref_expr( name_str, fs_sets.sin_warding );
      else if ( util::str_compare_ci( split[ 1 ], "sintharas_veil" ) )
        return make_ref_expr( name_str, fs_sets.sintharas_veil );
      else if ( util::str_compare_ci( split[ 1 ], "torment_of_baelaurum" ) )
        return make_ref_expr( name_str, fs_sets.torment_of_baelaurum );
      else if ( util::str_compare_ci( split[ 1 ], "tuzari_grace" ) )
        return make_ref_expr( name_str, fs_sets.tuzari_grace );
    }
    else if ( util::str_compare_ci( split[ 0 ], "weapon_trait" ) )
    {
      if ( util::str_compare_ci( split[ 1 ], "visions_of_grandeur" ) )
        return make_ref_expr( name_str, fs_weapons.visions_of_grandeur );
      else if ( util::str_compare_ci( split[ 1 ], "brave_machinations" ) )
        return make_ref_expr( name_str, fs_weapons.brave_machinations );
    }
    else if ( util::str_compare_ci( split[ 0 ], "weapon" ) )
    {
      if ( util::str_compare_ci( split[ 1 ], "chronoshift" ) )
        return make_fn_expr( name_str, [ & ] { return fs_weapons.equipped_weapon == FSWEAPON_CHRONOSHIFT; } );
      else if ( util::str_compare_ci( split[ 1 ], "voidbringer" ) ||
                util::str_compare_ci( split[ 1 ], "voidbringers" ) || util::str_compare_ci( split[ 1 ], "voidbringers_touch" ) )
        return make_fn_expr( name_str, [ & ] { return fs_weapons.equipped_weapon == FSWEAPON_VOIDBRINGERS_TOUCH; } );
      else if ( util::str_compare_ci( split[ 1 ], "fated_strike" ) )
        return make_fn_expr( name_str, [ & ] { return fs_weapons.equipped_weapon == FSWEAPON_FATED_STRIKE; } );
    }
    else if ( util::str_compare_ci( split[ 0 ], "blessing" ) || util::str_compare_ci( split[ 0 ], "finesse" ) )
    {
      gear_affix_e affix_stat = gear_affix_from_string( split[ 1 ] );

      if ( affix_stat >= GEAR_AFFIX_FINESSE_START && affix_stat <= GEAR_AFFIX_FINESSE_END )
      {
        return make_ref_expr( name_str, finesse_traits[ affix_stat - GEAR_AFFIX_FINESSE_START ] );
      }
    }
  }
  // Split expressions

  return player_t::create_expression( name_str );
}

// fs_player_t::init_base =======================================================

void fs_player_t::init_base_stats()
{
  if ( base.distance < 1 )
    base.distance = 5;

  player_t::init_base_stats();

  base.rating.attack_crit = base.rating.attack_haste = base.rating.damage_versatility = base.rating.mastery =
      base.rating.heal_versatility = base.rating.spell_crit = base.rating.spell_haste = base.rating.spell_hit = 0.0016;

  /*base.stats.attribute[ STAT_AGILITY ] = 1000;*/
  base.stats.attribute[ STAT_STAMINA ] = 100;
  resources.base[ RESOURCE_HEALTH ]    = 11480;

  base.health_per_stamina = 36.638;

  base.attack_power_per_strength = 1.0;
  base.attack_power_per_agility  = 1.0;
  base.spell_power_per_intellect = 1.0;

  base.mastery = 0.0;

  resources.base[ RESOURCE_SPIRIT ] = resources.max[ RESOURCE_SPIRIT ] = 100;

  resources.start_at[ RESOURCE_SPIRIT ]              = 0;
  resources.base_regen_per_second[ RESOURCE_SPIRIT ] = 100.0 / 300 * 1.20;

  
  if ( finesse_traits[ FINESSE_M ] )
  {
    resources.start_at[ RESOURCE_SPIRIT ] += finesse_trait_values.finesse_m_spirit[ finesse_traits[ FINESSE_M ] ];
  }

  // resources.base_regen_per_second[ RESOURCE_ENERGY ] = 10;

  // base_gcd = timespan_t::from_seconds( 1.0 );
  // min_gcd  = timespan_t::from_seconds( 1.0 );
}

// fs_player_t::init_spells =====================================================

void fs_player_t::init_spells()
{
  player_t::init_spells();
}

// fs_player_t::init_talents ====================================================

void fs_player_t::init_talents()
{
  player_t::init_talents();
}

// fs_player_t::init_gains ======================================================

void fs_player_t::init_gains()
{
  player_t::init_gains();

  fs_gains.grandeur     = get_gain( "Visions of Grandeur" );
  fs_gains.spirit_procs = get_gain( "Spirit Procs" );
  fs_gains.finesse_d = get_gain( "Finesse D" );
}

// fs_player_t::init_procs ======================================================

void fs_player_t::init_procs()
{
  player_t::init_procs();
}

// fs_player_t::init_scaling ====================================================

void fs_player_t::init_scaling()
{
  player_t::init_scaling();

  scaling->disable( STAT_SPIRIT );
  scaling->disable( STAT_WEAPON_OFFHAND_DPS );
  scaling->disable( STAT_WEAPON_DPS );
  scaling->disable( STAT_SPELL_POWER );
  scaling->disable( STAT_ATTACK_POWER );

  // Break out early if scaling is disabled on this player, or there's no
  // scaling stat
  if ( !scale_player || sim->scaling->scale_stat == STAT_NONE )
  {
    return;
  }
}

// fs_player_t::init_resources =================================================

void fs_player_t::init_resources( bool force )
{
  player_t::init_resources( force );
}

// fs_player_t::init_buffs ======================================================

void fs_player_t::create_buffs()
{
  player_t::create_buffs();

  fs_buffs.rising_spirit = make_buff<fs_player_buff_t>( this, "rising_spirit" )
                               ->set_max_stack( 99 )
                               ->set_default_value( 0.05 )
                               ->set_refresh_duration_callback( [ this ]( const buff_t* b, timespan_t ) {
                                 if ( b->check() <= 5 )
                                   return b->remains() + 2_s;
                                 return b->remains();
                               } )
                               ->set_refresh_behavior( buff_refresh_behavior::CUSTOM )
                               ->set_duration( 10_s );

  switch ( convert_hybrid_stat( STAT_STR_AGI_INT ) )
  {
    case STAT_INTELLECT:
      fs_buffs.rising_spirit->set_pct_buff_type( STAT_PCT_BUFF_INTELLECT );
      break;
    case STAT_AGILITY:
      fs_buffs.rising_spirit->set_pct_buff_type( STAT_PCT_BUFF_AGILITY );
      break;
    case STAT_STRENGTH:
      fs_buffs.rising_spirit->set_pct_buff_type( STAT_PCT_BUFF_STRENGTH );
      break;
    default:
      break;
  }


  auto rank          = finesse_traits[ FINESSE_A ];
  fs_buffs.finesse_a = make_buff<fs_player_buff_t>( this, "finesse_a" )
                           ->set_default_value( finesse_trait_values.finesse_a_per_stack[ rank ] )
                           ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT )
                           ->set_max_stack( finesse_trait_values.finesse_a_max_stacks );

  rank               = finesse_traits[ FINESSE_N ];
  fs_buffs.finesse_n =
      make_buff<fs_player_buff_t>( this, "finesse_n" )->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

  if ( rank > 0 )
    fs_buffs.finesse_n->set_max_stack( finesse_trait_values.finesse_n_casts[ rank ] );

  rank               = finesse_traits[ FINESSE_B ];
  fs_buffs.finesse_b = make_buff<fs_player_buff_t>( this, "finesse_b" )
                           ->set_pct_buff_type( STAT_PCT_BUFF_CRIT )
                           //->set_stack_behavior( buff_stack_behavior::ASYNCHRONOUS )
                           ->set_default_value( finesse_trait_values.finesse_b_crit[ rank ] )
                           //->set_max_stack( finesse_trait_values.finesse_a_max_stacks )
                           ->set_duration( finesse_trait_values.finesse_b_duration );

  rank               = finesse_traits[ FINESSE_G ];
  fs_buffs.finesse_g = make_buff<fs_player_buff_t>( this, "finesse_g" )
                           ->set_pct_buff_type( STAT_PCT_BUFF_VERSATILITY )
                           ->set_pct_buff_type( STAT_PCT_BUFF_HASTE )
                           ->set_pct_buff_type( STAT_PCT_BUFF_CRIT )
                           ->set_default_value( 0.0 )
                           ->set_duration( finesse_trait_values.finesse_g_duration[ rank ] );

  rank               = finesse_traits[ FINESSE_I ];
  fs_buffs.finesse_i = make_buff<fs_player_buff_t>( this, "finesse_i" )
                           ->set_pct_buff_type( STAT_PCT_BUFF_HASTE )
                           ->set_default_value( finesse_trait_values.finesse_i_haste[ rank ] )
                           ->set_duration( finesse_trait_values.finesse_i_duration );

  rank               = finesse_traits[ FINESSE_K ];
  fs_buffs.finesse_k = make_buff<fs_player_buff_t>( this, "finesse_k" )
                           ->set_duration( finesse_trait_values.finesse_k_amp_duration )
                           ->set_default_value( finesse_trait_values.finesse_k_amp_multiplier )
                           ->set_stack_change_callback( [ this ]( buff_t*, int, int ) {
                             for ( auto action : action_list )
                             {
                               if ( action->cooldown )
                                 action->cooldown->adjust_recharge_multiplier();
                             }
                           } );

  fs_buffs.spirit_of_heroism = make_buff<fs_player_buff_t>( this, "spirit_of_heroism" )
                                   ->set_pct_buff_type( STAT_PCT_BUFF_HASTE )
                                   ->set_default_value( 0.3 )
                                   ->set_duration( 20_s );


  fs_buffs.ancestral_surge = make_buff<fs_player_buff_t>( this, "ancestral_surge" )
          ->set_default_value( fs_gems.gem_powers[ GEM_SAPPHIRE ] >= GEM_TIER_6 ? fs_gems.sapphire_surge_major
                                                                                : fs_gems.sapphire_surge_minor );

  switch ( convert_hybrid_stat( STAT_STR_AGI_INT ) )
  {
    case STAT_INTELLECT:
      fs_buffs.ancestral_surge->set_pct_buff_type( STAT_PCT_BUFF_INTELLECT );
      break;
    case STAT_AGILITY:
      fs_buffs.ancestral_surge->set_pct_buff_type( STAT_PCT_BUFF_AGILITY );
      break;
    case STAT_STRENGTH:
      fs_buffs.ancestral_surge->set_pct_buff_type( STAT_PCT_BUFF_STRENGTH );
      break;
    default:
      break;
  }

  fs_buffs.drakheims_absolution = make_buff<fs_player_buff_t>( this, "drakheims_absolution" )
                                      ->set_default_value( fs_sets.drakheims_absolution_amp )
                                      ->set_duration( fs_sets.drakheims_absolution_duration );

  switch ( convert_hybrid_stat( STAT_STR_AGI_INT ) )
  {
    case STAT_INTELLECT:
      fs_buffs.drakheims_absolution->set_pct_buff_type( STAT_PCT_BUFF_INTELLECT );
      break;
    case STAT_AGILITY:
      fs_buffs.drakheims_absolution->set_pct_buff_type( STAT_PCT_BUFF_AGILITY );
      break;
    case STAT_STRENGTH:
      fs_buffs.drakheims_absolution->set_pct_buff_type( STAT_PCT_BUFF_STRENGTH );
      break;
    default:
      break;
  }

  fs_buffs.dark_prophecy = make_buff( this, "dark_prophecy" )
                               ->set_pct_buff_type( STAT_PCT_BUFF_HASTE )
                               ->set_duration( fs_sets.dark_prophecy_duration )
                               ->set_default_value( fs_sets.dark_prophecy_haste );

  fs_buffs.draconic_might = make_buff( this, "draconic_might" )
                                ->set_duration( fs_sets.draconic_might_duration )
                                ->set_default_value( fs_sets.draconic_might_amp );

  switch ( convert_hybrid_stat( STAT_STR_AGI_INT ) )
  {
    case STAT_INTELLECT:
      fs_buffs.draconic_might->set_pct_buff_type( STAT_PCT_BUFF_INTELLECT );
      break;
    case STAT_AGILITY:
      fs_buffs.draconic_might->set_pct_buff_type( STAT_PCT_BUFF_AGILITY );
      break;
    case STAT_STRENGTH:
      fs_buffs.draconic_might->set_pct_buff_type( STAT_PCT_BUFF_STRENGTH );
      break;
    default:
      break;
  }

  fs_buffs.adrenaline_rush = make_buff<fs_player_buff_t>( this, "adrenaline_rush" )
                                 ->set_default_value( fs_gems.gem_powers[ GEM_TOPAZ ] >= GEM_TIER_6 ? fs_gems.topaz_adrenaline_major : fs_gems.topaz_adrenaline_minor )
                                 ->set_pct_buff_type( STAT_PCT_BUFF_HASTE )
                                 ->set_duration( 10_s );

  fs_buffs.virtuoso = make_buff<fs_player_buff_t>( this, "virtuoso" )
                          ->set_default_value( fs_gems.gem_powers[ GEM_TOPAZ ] >= GEM_TIER_10 ? fs_gems.topaz_virtuoso_major : fs_gems.topaz_virtuoso_minor )
                          ->set_pct_buff_type( STAT_PCT_BUFF_HASTE )
                          ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

  fs_buffs.first_strike = make_buff<fs_player_buff_t>( this, "first_strike" )
                              ->set_default_value( fs_gems.gem_powers[ GEM_EMERALD ] >= GEM_TIER_6 ? fs_gems.emerald_first_strike_major : fs_gems.emerald_first_strike_minor )
                              ->set_pct_buff_type( STAT_PCT_BUFF_VERSATILITY )
                              ->set_duration( fs_gems.emerald_first_strike_duration );

  fs_buffs.might_of_the_minotaur = make_buff<fs_player_buff_t>( this, "might_of_the_minotaur" )
                                       ->set_default_value( fs_gems.gem_powers[ GEM_RUBY ] >= GEM_TIER_6 ? fs_gems.ruby_minotaur_major : fs_gems.ruby_minotaur_minor );

  switch ( convert_hybrid_stat( STAT_STR_AGI_INT ) )
  {
    case STAT_INTELLECT:
      fs_buffs.might_of_the_minotaur->set_pct_buff_type( STAT_PCT_BUFF_INTELLECT );
      break;
    case STAT_AGILITY:
      fs_buffs.might_of_the_minotaur->set_pct_buff_type( STAT_PCT_BUFF_AGILITY );
      break;
    case STAT_STRENGTH:
      fs_buffs.might_of_the_minotaur->set_pct_buff_type( STAT_PCT_BUFF_STRENGTH );
      break;
    default:
      break;
  }

  fs_buffs.willful_momentum =
      make_buff<fs_player_buff_t>( this, "willful_momentum" )
          ->set_default_value( fs_weapon_trait_values.willful_momentum_amp[ fs_weapons.willful_momentum ] )
          ->set_duration( fs_weapon_trait_values.willful_momentum_duration );

  switch ( convert_hybrid_stat( STAT_STR_AGI_INT ) )
  {
    case STAT_INTELLECT:
      fs_buffs.willful_momentum->set_pct_buff_type( STAT_PCT_BUFF_INTELLECT );
      break;
    case STAT_AGILITY:
      fs_buffs.willful_momentum->set_pct_buff_type( STAT_PCT_BUFF_AGILITY );
      break;
    case STAT_STRENGTH:
      fs_buffs.willful_momentum->set_pct_buff_type( STAT_PCT_BUFF_STRENGTH );
      break;
    default:
      break;
  }

  fs_buffs.vengeful_soul =
      make_buff<fs_player_buff_t>( this, "vengeful_soul" )
          ->set_default_value( fs_weapon_trait_values.vengeful_soul_amp[ fs_weapons.vengeful_soul ] )
          ->set_duration( fs_weapon_trait_values.vengeful_soul_duration )
          ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

  switch ( convert_hybrid_stat( STAT_STR_AGI_INT ) )
  {
    case STAT_INTELLECT:
      fs_buffs.vengeful_soul->set_pct_buff_type( STAT_PCT_BUFF_INTELLECT );
      break;
    case STAT_AGILITY:
      fs_buffs.vengeful_soul->set_pct_buff_type( STAT_PCT_BUFF_AGILITY );
      break;
    case STAT_STRENGTH:
      fs_buffs.vengeful_soul->set_pct_buff_type( STAT_PCT_BUFF_STRENGTH );
      break;
    default:
      break;
  }

  fs_buffs.hunters_focus =
      make_buff<stat_buff_t>( this, "hunters_focus" )
          ->add_stat( STAT_HASTE_RATING, fs_weapon_trait_values.hunters_focus_haste[ fs_weapons.hunters_focus ] )
          ->set_duration( fs_weapon_trait_values.hunters_focus_duration[ fs_weapons.hunters_focus ] )
          ->set_max_stack( fs_weapon_trait_values.hunters_focus_max_stacks[ fs_weapons.hunters_focus ] )
          ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );
  
  fs_buffs.patient_soul =
      make_buff<stat_buff_t>( this, "patient_soul" )
          ->add_stat( STAT_VERSATILITY_RATING,
                      fs_weapon_trait_values.patient_soul_expertise[ fs_weapons.patient_soul ] )
          ->set_default_value( fs_weapon_trait_values.patient_soul_expertise[ fs_weapons.patient_soul ] )
          ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT )
          ->add_stack_change_callback( [ this ]( buff_t* b, int old, int _new ) {
            if ( _new )
            {
              b->player->resources.initial_multiplier[ RESOURCE_HEALTH ] += b->current_value;
              b->player->recalculate_resource_max( RESOURCE_HEALTH );
            }
            else
            {
              b->player->resources.initial_multiplier[ RESOURCE_HEALTH ] -= b->current_value;
              b->player->recalculate_resource_max( RESOURCE_HEALTH );
            }
          } );

  fs_buffs.martial_initiative =
      make_buff<fs_player_buff_t>( this, "martial_initiative" )
          ->set_default_value( 0.1 )
          ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT );

  switch ( convert_hybrid_stat( STAT_STR_AGI_INT ) )
  {
    case STAT_INTELLECT:
      fs_buffs.martial_initiative->set_pct_buff_type( STAT_PCT_BUFF_INTELLECT );
      break;
    case STAT_AGILITY:
      fs_buffs.martial_initiative->set_pct_buff_type( STAT_PCT_BUFF_AGILITY );
      break;
    case STAT_STRENGTH:
      fs_buffs.martial_initiative->set_pct_buff_type( STAT_PCT_BUFF_STRENGTH );
      break;
    default:
      break;
  }
    
  fs_buffs.hidden_power = make_buff<fs_player_buff_t>( this, "hidden_power" )
                              ->set_default_value( fs_weapon_trait_values.hidden_power_amp[ fs_weapons.hidden_power ] )
                              ->set_duration( fs_weapon_trait_values.hidden_power_buff_duration );

  switch ( convert_hybrid_stat( STAT_STR_AGI_INT ) )
  {
    case STAT_INTELLECT:
      fs_buffs.hidden_power->set_pct_buff_type( STAT_PCT_BUFF_INTELLECT );
      break;
    case STAT_AGILITY:
      fs_buffs.hidden_power->set_pct_buff_type( STAT_PCT_BUFF_AGILITY );
      break;
    case STAT_STRENGTH:
      fs_buffs.hidden_power->set_pct_buff_type( STAT_PCT_BUFF_STRENGTH );
      break;
    default:
      break;
  }

  fs_buffs.harmonious_soul =
      make_buff<fs_player_buff_t>( this, "harmonious_soul" )
          ->set_max_stack( fs_gems.gem_powers[ GEM_DIAMOND ] >= GEM_TIER_6 ? fs_gems.harmonious_stacks_major
                                                                           : fs_gems.harmonious_stacks_minor )
          ->set_freeze_stacks( true )
          ->set_period( fs_gems.harmonious_duration )
          ->set_default_value( fs_gems.harmonious_buff )
          ->set_pct_buff_type( STAT_PCT_BUFF_CRIT )
          ->set_pct_buff_type( STAT_PCT_BUFF_HASTE )
          ->set_pct_buff_type( STAT_PCT_BUFF_MASTERY )
          ->set_pct_buff_type( STAT_PCT_BUFF_VERSATILITY )
          ->set_tick_behavior( buff_tick_behavior::CLIP )
          ->set_tick_callback( []( buff_t* b, int, timespan_t ) { b->decrement(); } );


  fs_buffs.hidden_power_stacking = make_buff<fs_player_buff_t>( this, "hidden_power_stacking" )
                                       ->set_max_stack( 5 )
                                       ->set_duration( 60_s )
                                       ->set_expire_at_max_stack( true )
                                       ->add_stack_change_callback( [ this ]( buff_t* b, int old, int _new ) {
                                         if ( b->at_max_stacks() )
                                           fs_buffs.hidden_power->trigger();
                                       } );

  fs_buffs.seized_opportunity_stacking = make_buff<fs_player_buff_t>( this, "seized_opportunity_stacking" )
                                             ->set_max_stack( 20 )
                                             ->set_constant_behavior( buff_constant_behavior::NEVER_CONSTANT )
                                             ->set_expire_at_max_stack( true )
                                             ->add_stack_change_callback( [ this ]( buff_t* b, int old, int _new ) {
                                               if ( b->at_max_stacks() )
                                                 fs_buffs.seized_opportunity->trigger();
                                             } );

  fs_buffs.seized_opportunity =
      make_buff<stat_buff_t>( this, "seized_opportunity" )
          ->add_stat( STAT_CRIT_RATING,
                      fs_weapon_trait_values.seized_opportunity_crit[ fs_weapons.seized_opportunity ] )
          ->set_max_stack( 20 )
          ->set_duration( 12_s );

  fs_buffs.sundering_wrath = make_buff<fs_player_buff_t>( this, "sundering_wrath" )
                               ->set_default_value( 0.06 )
                               ->set_pct_buff_type( STAT_PCT_BUFF_CRIT )
                               ->set_max_stack( 5 )
                               ->set_duration( 20_s )
                               ->set_refresh_behavior( buff_refresh_behavior::DISABLED );

  struct fated_strike_buff_t : fs_player_buff_t
  {
    double cdr_mod = 3.0;
    fated_strike_buff_t( player_t* pl ) : fs_player_buff_t( pl, "fated_strike" )
    {
      set_duration( 6_s );
      add_stack_change_callback( [ this ]( buff_t*, int, int _new ) {
        if ( _new )
        {
          for ( auto& action : p()->action_list )
          {
            action->dynamic_recharge_rate_multiplier /= cdr_mod;
            action->cooldown->adjust_recharge_multiplier();
          }
        }
        else
        {
          for ( auto& action : p()->action_list )
          {
            action->dynamic_recharge_rate_multiplier *= cdr_mod;
            action->cooldown->adjust_recharge_multiplier();
          }
        }
      } );
    }
  };

  struct chronoshift_buff_t : fs_player_buff_t
  {
    double cdr_mod = 9.0;

    chronoshift_buff_t( player_t* pl ) : fs_player_buff_t( pl, "chronoshift_barrier" )
    {
      set_duration( 3_s );

      add_stack_change_callback( [ this ]( buff_t*, int, int _new ) {
        if ( _new )
        {
          for ( auto& action : p()->action_list )
          {
            action->dynamic_recharge_rate_multiplier /= cdr_mod;
            action->cooldown->adjust_recharge_multiplier();
          }
        }
        else
        {
          for ( auto& action : p()->action_list )
          {
            action->dynamic_recharge_rate_multiplier *= cdr_mod;
            action->cooldown->adjust_recharge_multiplier();
          }
        }
      } );
    }
  };

  fs_buffs.fated_strike = make_buff<fated_strike_buff_t>( this );
  fs_buffs.chronoshift  = make_buff<chronoshift_buff_t>( this );
}

// fs_player_t::invalidate_cache =========================================

void fs_player_t::invalidate_cache( cache_e c )
{
  player_t::invalidate_cache( c );
}

bool parse_fsweapon( sim_t* sim, std::string_view, std::string_view value )
{
  auto& player = sim->active_player;
  for ( fsweapon_e weapon = fsweapon_e::FSWEAPON_NONE; weapon < fsweapon_e::FSWEAPON_MAX; weapon++ )
  {
    if ( util::str_compare_ci( value, util::fsweapon_string( weapon ) ) )
    {
      static_cast<fs_player_t*>( player )->fs_weapons.equipped_weapon = weapon;
      return true;
    }
  }

  sim->error( "{} weapon string '{}' not valid.", sim->active_player->name(), value );
  return false;
}

bool parse_fsrelic1( sim_t* sim, std::string_view, std::string_view value )
{
  auto& player = sim->active_player;
  for ( fsrelic_e relic = fsrelic_e::FSRELIC_NONE; relic < fsrelic_e::FSRELIC_MAX; relic++ )
  {
    if ( util::str_compare_ci( value, util::fsrelic_string( relic ) ) )
    {
      static_cast<fs_player_t*>( player )->fs_relics.relic1 = relic;
      return true;
    }
  }

  sim->error( "{} relic string '{}' not valid.", sim->active_player->name(), value );
  return false;
}

bool parse_fsrelic2( sim_t* sim, std::string_view, std::string_view value )
{
  auto& player = sim->active_player;
  for ( fsrelic_e relic = fsrelic_e::FSRELIC_NONE; relic < fsrelic_e::FSRELIC_MAX; relic++ )
  {
    if ( util::str_compare_ci( value, util::fsrelic_string( relic ) ) )
    {
      static_cast<fs_player_t*>( player )->fs_relics.relic2 = relic;
      return true;
    }
  }

  sim->error( "{} relic string '{}' not valid.", sim->active_player->name(), value );
  return false;
}


void fs_player_t::create_options()
{
  player_t::create_options();

  add_option( opt_uint64( "talent_points_fs", talent_points_fs ) );

  add_option( opt_bool( "sets.dark_prophecy", fs_sets.dark_prophecy ) );
  add_option( opt_float( "sets.dark_prophecy_haste", fs_sets.dark_prophecy_haste ) );
  add_option( opt_timespan( "sets.dark_prophecy_duration", fs_sets.dark_prophecy_duration ) );
  add_option( opt_timespan( "sets.dark_prophecy_cooldown", fs_sets.dark_prophecy_cooldown ) );

  add_option( opt_bool( "sets.deaths_grasp", fs_sets.deaths_grasp ) );
  add_option( opt_float( "sets.deaths_grasp_spirit", fs_sets.deaths_grasp_spirit ) );
  add_option( opt_float( "sets.death_grasp_execute_amp", fs_sets.death_grasp_execute_amp ) );

  add_option( opt_bool( "sets.draconic_might", fs_sets.draconic_might ) );
  add_option( opt_float( "sets.draconic_might_ppm", fs_sets.draconic_might_ppm ) );
  add_option( opt_float( "sets.draconic_might_amp", fs_sets.draconic_might_amp ) );
  add_option( opt_timespan( "sets.draconic_might_duration", fs_sets.draconic_might_duration ) );

  add_option( opt_bool( "sets.drakheims_absolution", fs_sets.drakheims_absolution ) );
  add_option( opt_float( "sets.drakheims_absolution_amp", fs_sets.drakheims_absolution_amp ) );
  add_option( opt_timespan( "sets.drakheims_absolution_duration", fs_sets.drakheims_absolution_duration ) );

  add_option( opt_bool( "sets.eldrin_deceit", fs_sets.eldrin_deceit ) );
  add_option( opt_float( "sets.eldrin_deceit_crit", fs_sets.eldrin_deceit_crit ) );

  add_option( opt_bool( "sets.eldrin_fury", fs_sets.eldrin_fury ) );
  add_option( opt_float( "sets.eldrin_fury_crit", fs_sets.eldrin_fury_crit ) );

  add_option( opt_bool( "sets.haunting_lament", fs_sets.haunting_lament ) );
  add_option( opt_float( "sets.haunting_lament_spirit", fs_sets.haunting_lament_spirit ) );
  add_option( opt_float( "sets.haunting_lament_max_mana", fs_sets.haunting_lament_max_mana ) );

  add_option( opt_bool( "sets.sin_warding", fs_sets.sin_warding ) );
  add_option( opt_float( "sets.sin_warding_expertise", fs_sets.sin_warding_expertise ) );

  add_option( opt_bool( "sets.sintharas_veil", fs_sets.sintharas_veil ) );
  add_option( opt_float( "sets.sintharas_veil_spirit", fs_sets.sintharas_veil_spirit ) );
  add_option( opt_float( "sets.sintharas_veil_magic_dr", fs_sets.sintharas_veil_magic_dr ) );

  add_option( opt_bool( "sets.torment_of_baelaurum", fs_sets.torment_of_baelaurum ) );
  add_option( opt_float( "sets.torment_of_baelaurum_amp", fs_sets.torment_of_baelaurum_amp ) );
  add_option( opt_float( "sets.torment_of_baelaurum_heal_pct", fs_sets.torment_of_baelaurum_heal_pct ) );

  add_option( opt_bool( "sets.tuzari_grace", fs_sets.tuzari_grace ) );
  add_option( opt_float( "sets.tuzari_grace_haste", fs_sets.tuzari_grace_haste ) );
  add_option( opt_float( "sets.tuzari_grace_movement_speed", fs_sets.tuzari_grace_movement_speed ) );

  add_option( opt_float( "gems.ruby_power", fs_gems.gem_powers[ GEM_RUBY ] ) );
  add_option( opt_float( "gems.amethyst_power", fs_gems.gem_powers[ GEM_AMETHYST ] ) );
  add_option( opt_float( "gems.diamond_power", fs_gems.gem_powers[ GEM_DIAMOND ] ) );
  add_option( opt_float( "gems.topaz_power", fs_gems.gem_powers[ GEM_TOPAZ ] ) );
  add_option( opt_float( "gems.emerald_power", fs_gems.gem_powers[ GEM_EMERALD ] ) );
  add_option( opt_float( "gems.sapphire_power", fs_gems.gem_powers[ GEM_SAPPHIRE ] ) );

  add_option( opt_func( "weapon", parse_fsweapon ) );
  add_option( opt_func( "relic1", parse_fsrelic1 ) );
  add_option( opt_func( "relic2", parse_fsrelic2 ) );

  add_option( opt_uint( "weapon_trait.amethyst_splinters", fs_weapons.amethyst_splinters, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.brave_machinations", fs_weapons.brave_machinations, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.diamond_strike", fs_weapons.diamond_strike, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.divine_mediation", fs_weapons.divine_mediation, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.emerald_judgement", fs_weapons.emerald_judgement, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.first_man_standing", fs_weapons.first_man_standing, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.grounded_spirit", fs_weapons.grounded_spirit, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.heart_of_stone", fs_weapons.heart_of_stone, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.heroic_brand", fs_weapons.heroic_brand, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.hidden_power", fs_weapons.hidden_power, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.hunters_focus", fs_weapons.hunters_focus, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.inspired_allegiance", fs_weapons.inspired_allegiance, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.iron_spikes", fs_weapons.iron_spikes, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.kindling", fs_weapons.kindling, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.king_of_the_hill", fs_weapons.king_of_the_hill, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.latent_resurgence", fs_weapons.latent_resurgence, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.martial_initiative", fs_weapons.martial_initiative, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.navigators_intuition", fs_weapons.navigators_intuition, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.patient_soul", fs_weapons.patient_soul, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.ruby_storm", fs_weapons.ruby_storm, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.sapphire_aurastone", fs_weapons.sapphire_aurastone, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.seized_opportunity", fs_weapons.seized_opportunity, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.stalwart_readiness", fs_weapons.stalwart_readiness, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.treasure_hunters_delight", fs_weapons.treasure_hunters_delight, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.vengeful_soul", fs_weapons.vengeful_soul, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.visions_of_grandeur", fs_weapons.visions_of_grandeur, 0, 4 ) );
  add_option( opt_uint( "weapon_trait.willful_momentum", fs_weapons.willful_momentum, 0, 4 ) );

  add_option( opt_float( "spirit_refund_mul", spirit_refund_mul ) );

 add_option( opt_bool( "ruby_allow_boss_amp", fs_options.ruby_allow_boss_amp ) );
}

// fs_player_t::copy_from =======================================================

void fs_player_t::copy_from( player_t* source )
{
  fs_player_t* fs_player = static_cast<fs_player_t*>( source );
  player_t::copy_from( source );

  fs_sets            = fs_player->fs_sets;
  fs_options         = fs_player->fs_options;
  fs_gems.gem_powers = fs_player->fs_gems.gem_powers;
  fs_weapons         = fs_player->fs_weapons;
  fs_relics          = fs_player->fs_relics;
}

// fs_player_t::create_profile  =================================================

std::string fs_player_t::create_profile( save_e stype )
{
  std::string profile_str = player_t::create_profile( stype );

  // Break out early if we are not saving everything, or gear
  if ( !( stype & SAVE_PLAYER ) && !( stype & SAVE_GEAR ) )
  {
    return profile_str;
  }

  std::string term = "\n";

  return profile_str;
}

// fs_player_t::init_items ======================================================

void fs_player_t::init_items()
{
  player_t::init_items();

  for ( auto& v : finesse_traits )
  {
    if ( v > 4 )
      v = 4;
  }

  for ( auto& v : fs_weapons.weapon_traits )
  {
    if ( v > 4 )
      v = 4;
  }
}

// fs_player_t::init_special_effects ============================================

void fs_player_t::init_special_effects()
{
  player_t::init_special_effects();

  if ( fs_gems.gem_powers[ GEM_SAPPHIRE ] >= GEM_TIER_1 )
  {
    auto extra_max_spirit = fs_gems.gem_powers[ GEM_SAPPHIRE ] >= GEM_TIER_6 ? fs_gems.sapphire_additional_max_spirit_major
                                                                       : fs_gems.sapphire_additional_max_spirit_minor;
    resources.max[ RESOURCE_SPIRIT ] += extra_max_spirit;
    resources.base[ RESOURCE_SPIRIT ] += extra_max_spirit;
  }

  if ( fs_gems.gem_powers[ GEM_RUBY ] >= GEM_TIER_7 )
  {
    passive.add_stat( convert_hybrid_stat( STAT_STR_AGI_INT ), fs_gems.ruby_stat_main_major );
    passive.add_stat( STAT_STAMINA, fs_gems.ruby_stamina_major );
  }
  else if ( fs_gems.gem_powers[ GEM_RUBY ] >= GEM_TIER_2 )
  {
    passive.add_stat( convert_hybrid_stat( STAT_STR_AGI_INT ), fs_gems.ruby_stat_main_minor );
    passive.add_stat( STAT_STAMINA, fs_gems.ruby_stamina_minor );
  }

  if ( fs_gems.gem_powers[ GEM_AMETHYST ] >= GEM_TIER_7 )
  {
    passive.add_stat( STAT_CRIT_RATING, fs_gems.stat_major );
    passive.add_stat( STAT_STAMINA, fs_gems.stat_major );
  }
  else if ( fs_gems.gem_powers[ GEM_AMETHYST ] >= GEM_TIER_2 )
  {
    passive.add_stat( STAT_CRIT_RATING, fs_gems.stat_minor );
    passive.add_stat( STAT_STAMINA, fs_gems.stat_minor );
  }

  if ( fs_gems.gem_powers[ GEM_TOPAZ ] >= GEM_TIER_7 )
  {
    passive.add_stat( STAT_HASTE_RATING, fs_gems.stat_major );
    passive.add_stat( STAT_STAMINA, fs_gems.stat_major );
  }
  else if ( fs_gems.gem_powers[ GEM_TOPAZ ] >= GEM_TIER_2 )
  {
    passive.add_stat( STAT_HASTE_RATING, fs_gems.stat_minor );
    passive.add_stat( STAT_STAMINA, fs_gems.stat_minor );
  }

  if ( fs_gems.gem_powers[ GEM_EMERALD ] >= GEM_TIER_7 )
  {
    passive.add_stat( STAT_VERSATILITY_RATING, fs_gems.stat_major );
    passive.add_stat( STAT_STAMINA, fs_gems.stat_major );
  }
  else if ( fs_gems.gem_powers[ GEM_EMERALD ] >= GEM_TIER_2 )
  {
    passive.add_stat( STAT_VERSATILITY_RATING, fs_gems.stat_minor );
    passive.add_stat( STAT_STAMINA, fs_gems.stat_minor );
  }

  if ( fs_gems.gem_powers[ GEM_SAPPHIRE ] >= GEM_TIER_7 )
  {
    passive.add_stat( STAT_MASTERY_RATING, fs_gems.stat_major );
    passive.add_stat( STAT_STAMINA, fs_gems.stat_major );
  }
  else if ( fs_gems.gem_powers[ GEM_SAPPHIRE ] >= GEM_TIER_2 )
  {
    passive.add_stat( STAT_MASTERY_RATING, fs_gems.stat_minor );
    passive.add_stat( STAT_STAMINA, fs_gems.stat_minor );
  }

  if ( fs_gems.gem_powers[ GEM_DIAMOND ] >= GEM_TIER_7 )
  {
    passive.add_stat( convert_hybrid_stat( STAT_STR_AGI_INT ), fs_gems.diamond_main_stats_major );
    passive.add_stat( STAT_ARMOR, fs_gems.diamond_armor_major );
  }
  else if ( fs_gems.gem_powers[ GEM_DIAMOND ] >= GEM_TIER_2 )
  {
    passive.add_stat( convert_hybrid_stat( STAT_STR_AGI_INT ), fs_gems.diamond_main_stats_minor );
    passive.add_stat( STAT_ARMOR, fs_gems.diamond_armor_minor );
  }

  if ( fs_gems.gem_powers[ GEM_AMETHYST ] >= GEM_TIER_9 )
  {
    base.spell_crit_chance += fs_gems.stat_percent_major;
    base.attack_crit_chance += fs_gems.stat_percent_major;
  }
  else if ( fs_gems.gem_powers[ GEM_AMETHYST ] >= GEM_TIER_4 )
  {
    base.spell_crit_chance += fs_gems.stat_percent_minor;
    base.attack_crit_chance += fs_gems.stat_percent_minor;
  }

  if ( fs_gems.gem_powers[ GEM_SAPPHIRE ] >= GEM_TIER_9 )
  {
    base.mastery += fs_gems.stat_percent_major;
  }
  else if ( fs_gems.gem_powers[ GEM_SAPPHIRE ] >= GEM_TIER_4 )
  {
    base.mastery += fs_gems.stat_percent_minor;
  }

  if ( fs_gems.gem_powers[ GEM_EMERALD ] >= GEM_TIER_9 )
  {
    base.versatility += fs_gems.stat_percent_major;
  }
  else if ( fs_gems.gem_powers[ GEM_EMERALD ] >= GEM_TIER_4 )
  {
    base.versatility += fs_gems.stat_percent_minor;
  }

  if ( fs_gems.gem_powers[ GEM_TOPAZ ] >= GEM_TIER_9 )
  {
    base.haste += fs_gems.stat_percent_major;
  }
  else if ( fs_gems.gem_powers[ GEM_TOPAZ ] >= GEM_TIER_4 )
  {
    base.haste += fs_gems.stat_percent_minor;
  }

  if ( fs_gems.gem_powers[ GEM_DIAMOND ] >= GEM_TIER_1 )
  {
    register_on_kill_callback( [ this ]( player_t* t ) { fs_buffs.harmonious_soul->trigger( 1 ); } );
  }

  if ( fs_sets.tuzari_grace )
  {
    base.haste += fs_sets.tuzari_grace_haste;
  }

  if ( fs_gems.gem_powers[ GEM_DIAMOND ] >= GEM_TIER_9 )
  {
    base.attribute_multiplier[ STAT_STRENGTH ] += fs_gems.diamond_main_stat_amp_major;
    base.attribute_multiplier[ STAT_INTELLECT ] += fs_gems.diamond_main_stat_amp_major;
    base.attribute_multiplier[ STAT_AGILITY ] += fs_gems.diamond_main_stat_amp_major;
  }
  else if ( fs_gems.gem_powers[ GEM_DIAMOND ] >= GEM_TIER_4 )
  {
    base.attribute_multiplier[ STAT_STRENGTH ] += fs_gems.diamond_main_stat_amp_minor;
    base.attribute_multiplier[ STAT_INTELLECT ] += fs_gems.diamond_main_stat_amp_minor;
    base.attribute_multiplier[ STAT_AGILITY ] += fs_gems.diamond_main_stat_amp_minor;
  }

  // TODO: Implement as a health based check and buff that turns on & off.
  if ( fs_gems.gem_powers[ GEM_RUBY ] >= GEM_TIER_1 )
  {
    register_on_arise_callback( this, [ this ]() { fs_buffs.might_of_the_minotaur->trigger(); } );
  }

  if ( fs_gems.gem_powers[ GEM_SAPPHIRE ] >= GEM_TIER_1 )
  {
    fs_buffs.spirit_of_heroism->add_stack_change_callback( [ this ]( buff_t*, int, int new_stack ) {
      if ( new_stack )
      {
        fs_buffs.ancestral_surge->trigger();
      }
      else
      {
        fs_buffs.ancestral_surge->expire();
      }
    } );
  }

  if ( fs_gems.gem_powers[ GEM_TOPAZ ] >= GEM_TIER_5 )
  {
    register_on_arise_callback( this, [ this ]() {
      if ( !fs_buffs.spirit_of_heroism->check() )
        fs_buffs.virtuoso->trigger();
    } );

    fs_buffs.spirit_of_heroism->add_stack_change_callback( [ this ]( buff_t*, int, int new_stack ) {
      if ( new_stack )
      {
        fs_buffs.virtuoso->expire();
      }
      else
      {
        if ( !sim->is_canceled() )
          fs_buffs.virtuoso->trigger();
      }
    } );
  }

  if ( fs_gems.gem_powers[ GEM_SAPPHIRE ] >= GEM_TIER_10 )
  {
    fs_buffs.spirit_of_heroism->base_buff_duration += fs_gems.sapphire_spirit_duration_major;
  }
  else if ( fs_gems.gem_powers[ GEM_SAPPHIRE ] >= GEM_TIER_5 )
  {
    fs_buffs.spirit_of_heroism->base_buff_duration += fs_gems.sapphire_spirit_duration_minor;
  }

  if ( fs_sets.eldrin_deceit )
  {
    base.spell_crit_chance += fs_sets.eldrin_deceit_crit;
    base.attack_crit_chance += fs_sets.eldrin_deceit_crit;
  }

  if ( fs_sets.eldrin_fury )
  {
    base.spell_crit_chance += fs_sets.eldrin_fury_crit;
    base.attack_crit_chance += fs_sets.eldrin_fury_crit;
  }

  if ( fs_sets.deaths_grasp )
  {
    base.mastery += fs_sets.deaths_grasp_spirit;
  }

  if ( fs_sets.haunting_lament )
  {
    base.mastery += fs_sets.haunting_lament_spirit;
    resources.initial_multiplier[ RESOURCE_MANA ] += fs_sets.haunting_lament_max_mana;
  }

  if ( fs_sets.sintharas_veil )
  {
    base.mastery += fs_sets.sintharas_veil_spirit;
  }

  if ( fs_sets.sin_warding )
  {
    base.versatility += fs_sets.sin_warding_expertise;
    resources.initial_multiplier[ RESOURCE_HEALTH ] += fs_sets.sin_warding_max_hp;
  }

  if ( fs_sets.torment_of_baelaurum )
  {
    base.attribute_multiplier[ convert_hybrid_stat( STAT_STR_AGI_INT ) ] += fs_sets.torment_of_baelaurum_amp;
  }

  if ( fs_sets.dark_prophecy )
  {
    auto effect                   = new special_effect_t( this );
    effect->spell_id              = 1317;
    effect->name_str              = "dark_prophecy";
    effect->proc_flags_           = PF_ALL_DAMAGE;
    effect->proc_flags2_          = PF2_ALL_HIT;
    effect->has_use_buff_override = true;
    effect->cooldown_             = fs_sets.dark_prophecy_cooldown;
    effect->ppm_                  = -fs_sets.dark_prophecy_ppm;
    effect->rppm_scale_           = rppm_scale_e::RPPM_NONE;
    effect->rppm_blp_             = real_ppm_t::BLP_ENABLED;
    effect->type                  = special_effect_e::SPECIAL_EFFECT_EQUIP;

    special_effects.push_back( effect );

    effect->custom_buff = fs_buffs.dark_prophecy;

    auto dbc = new dbc_proc_callback_t( this, *effect );

    dbc->initialize();
    dbc->activate();
  }

  if ( fs_sets.draconic_might )
  {
    auto effect                   = new special_effect_t( this );
    effect->spell_id              = 1318;
    effect->name_str              = "draconic_might";
    effect->proc_flags_           = PF_ALL_DAMAGE;
    effect->proc_flags2_          = PF2_CRIT;
    effect->has_use_buff_override = true;
    effect->cooldown_             = fs_sets.draconic_might_cooldown;
    effect->ppm_                  = -fs_sets.draconic_might_ppm;
    effect->rppm_scale_           = rppm_scale_e::RPPM_CRIT;
    effect->rppm_blp_             = real_ppm_t::BLP_ENABLED;
    effect->type                  = special_effect_e::SPECIAL_EFFECT_EQUIP;

    special_effects.push_back( effect );

    effect->custom_buff = fs_buffs.draconic_might;

    auto dbc = new dbc_proc_callback_t( this, *effect );

    dbc->initialize();
    dbc->activate();
  }

  struct first_strike_cb_t : dbc_proc_callback_t
  {
    buff_t* first_strike;
    first_strike_cb_t( fs_player_t* p, const special_effect_t& e, buff_t* fs )
      : dbc_proc_callback_t( p, e ), first_strike( fs )
    {
    }

    fs_player_t* p() const
    {
      return static_cast<fs_player_t*>( listener );
    }

    void execute( action_t*, action_state_t* s ) override
    {
      if ( s->target->is_sleeping() )
        return;

      p()->get_target_data( s->target )->debuffs.triggered_first_strike->trigger();
      first_strike->trigger();
    }

    void trigger( action_t* a, action_state_t* state ) override
    {
      if ( p()->get_target_data( state->target )->debuffs.triggered_first_strike->check() || state->result_amount <= 0 )
        return;

      dbc_proc_callback_t::trigger( a, state );
    }
  };

  if ( fs_gems.gem_powers[ GEM_EMERALD ] >= GEM_TIER_1 )
  {
    auto fs_effect                   = new special_effect_t( this );
    fs_effect->spell_id              = 1318;
    fs_effect->name_str              = "first_strike";
    fs_effect->proc_flags_           = PF_ALL_DAMAGE;
    fs_effect->proc_flags2_          = PF2_ALL_HIT;
    fs_effect->proc_chance_          = 1.0;
    fs_effect->set_can_proc_from_procs( true );
    fs_effect->type                  = special_effect_e::SPECIAL_EFFECT_EQUIP;
    fs_effect->has_use_buff_override = true;

    special_effects.push_back( fs_effect );

    auto first_strike = new first_strike_cb_t( this, *fs_effect, fs_buffs.first_strike );
    first_strike->initialize();
    first_strike->activate();
  }

  double overcap = 0.0;

  for ( auto gem_power : fs_gems.gem_powers )
  {
    overcap += std::max( 0.0, gem_power - fs_gems.gem_power_cap );
  }

  if ( overcap > 0.0 )
  {
    auto mul = overcap * fs_gems.gem_power_mult;
    base.attribute_multiplier[ STAT_STRENGTH ] += mul;
    base.attribute_multiplier[ STAT_INTELLECT ] += mul;
    base.attribute_multiplier[ STAT_AGILITY ] += mul;
    base.attribute_multiplier[ STAT_STAMINA ] += mul;
  }

  if ( fs_weapons.willful_momentum > 0 )
  {
    passive.add_stat( STAT_MASTERY_RATING,
                      fs_weapon_trait_values.willful_momentum_spirit[ fs_weapons.willful_momentum ] );
  }

  if ( fs_weapons.kindling )
  {
    struct kindling_dot_t : public actions::fs_proc_spell_t
    {
      kindling_dot_t( std::string_view n, fs_player_t* p ) : actions::fs_proc_spell_t( n, p )
      {
        // random number - use actual ids later.
        id                     = 2289;
        school                 = SCHOOL_FIRE;
        dot_duration           = 9_s;
        dot_behavior           = DOT_REFRESH_DURATION;
        base_tick_time         = 1.5_s;
        tick_on_application    = true;
        hasted_ticks           = true;
        dot_allow_partial_tick = true;

        name_str_reporting = "Kindling";

        spell_power_mod.tick = p->fs_weapon_trait_values.kindling_tick_damage[ p->fs_weapons.kindling ];
      }
    };

    auto effect                   = new special_effect_t( this );
    effect->spell_id              = 2289;
    effect->name_str              = "kindling";
    effect->proc_flags_           = PF_ALL_DAMAGE | PF_PERIODIC;
    effect->proc_flags2_          = PF2_ALL_HIT | PF2_PERIODIC_DAMAGE;
    effect->cooldown_             = 0_s;
    effect->ppm_                  = -2.1;
    effect->set_can_proc_from_procs( true );
    effect->rppm_scale_           = rppm_scale_e::RPPM_HASTE;
    effect->rppm_blp_             = real_ppm_t::BLP_ENABLED;
    effect->type                  = special_effect_e::SPECIAL_EFFECT_EQUIP;

    special_effects.push_back( effect );

    effect->execute_action = create_fs_proc_action<kindling_dot_t>( "kindling_dot" );

    auto dbc = new dbc_proc_callback_t( this, *effect );

    dbc->initialize();
    dbc->activate();
  }

  if ( fs_weapons.emerald_judgement )
  {
    struct emerald_judgement_t : public actions::fs_proc_spell_t
    {
      emerald_judgement_t( std::string_view n, fs_player_t* p ) : actions::fs_proc_spell_t( n, p )
      {
        // random number - use actual ids later.
        id     = 2290;
        school = SCHOOL_NATURE;

        name_str_reporting = "Emerald Judgement";

        spell_power_mod.direct = p->fs_weapon_trait_values.emerald_judgement_dmg[ p->fs_weapons.emerald_judgement ];
      }

      void execute() override
      {
        if ( fs_p()->fs_gems.gem_powers[ GEM_EMERALD ] >= GEM_TIER_1 )
        {
          fs_p()->fs_buffs.first_strike->trigger();
        }
        base_t::execute();
      }
    };

    auto effect          = new special_effect_t( this );
    effect->spell_id     = 2290;
    effect->name_str     = "emerald_judgement";
    effect->proc_flags_  = PF_ALL_DAMAGE | PF_PERIODIC;
    effect->proc_flags2_ = PF2_ALL_HIT | PF2_PERIODIC_DAMAGE;
    effect->cooldown_    = 0_s;
    effect->ppm_         = -fs_weapon_trait_values.emerald_judgement_ppm[ fs_weapons.emerald_judgement ];
    effect->set_can_proc_from_procs( true );
    effect->rppm_scale_  = rppm_scale_e::RPPM_HASTE;
    effect->rppm_blp_    = real_ppm_t::BLP_ENABLED;
    effect->type         = special_effect_e::SPECIAL_EFFECT_EQUIP;

    special_effects.push_back( effect );

    effect->execute_action = create_fs_proc_action<emerald_judgement_t>( "emerald_judgement" );

    auto dbc = new dbc_proc_callback_t( this, *effect );

    dbc->initialize();
    dbc->activate();
  }

  if ( fs_weapons.ruby_storm )
  {
    struct ruby_storm_t : public actions::fs_proc_spell_t
    {
      ruby_storm_t( std::string_view n, fs_player_t* p ) : actions::fs_proc_spell_t( n, p )
      {
        id = 2297;

        name_str_reporting = "Ruby Storm";
        aoe                = -1;
      }

      void execute() override
      {
        base_dd_min = base_dd_max = fs_p()->resources.max[ RESOURCE_HEALTH ] *
                                    fs_p()->fs_weapon_trait_values.ruby_storm_damage[ fs_p()->fs_weapons.ruby_storm ];

        base_t::execute();
      }
    };

    auto effect          = new special_effect_t( this );
    effect->spell_id     = 2297;
    effect->name_str     = "ruby_storm";
    effect->proc_flags_  = PF_ALL_DAMAGE | PF_PERIODIC;
    effect->proc_flags2_ = PF2_ALL_HIT | PF2_PERIODIC_DAMAGE;
    effect->cooldown_    = 0_s;
    effect->ppm_         = -fs_weapon_trait_values.ruby_storm_ppm[ fs_weapons.ruby_storm ];
    effect->set_can_proc_from_procs( true );
    effect->rppm_scale_  = rppm_scale_e::RPPM_HASTE;
    effect->rppm_blp_    = real_ppm_t::BLP_ENABLED;
    effect->type         = special_effect_e::SPECIAL_EFFECT_EQUIP;

    special_effects.push_back( effect );

    effect->execute_action = create_fs_proc_action<ruby_storm_t>( "ruby_storm" );

    auto dbc = new dbc_proc_callback_t( this, *effect );

    dbc->initialize();
    dbc->activate();
  }

  if ( fs_weapons.navigators_intuition )
  {

    auto effect                   = new special_effect_t( this );
    effect->spell_id              = 15123;
    effect->name_str              = "navigators_intuition";
    effect->proc_flags_           = PF_ALL_DAMAGE;
    effect->proc_flags2_          = PF2_ALL_HIT;
    effect->has_use_buff_override = true;
    effect->cooldown_             = fs_weapon_trait_values.navigators_intuition_cd[ fs_weapons.navigators_intuition ];
    effect->proc_chance_ = fs_weapon_trait_values.navigators_intuition_chance[ fs_weapons.navigators_intuition ];
    effect->type         = special_effect_e::SPECIAL_EFFECT_EQUIP;

    static constexpr std::array<stat_e, 4> secondary_ratings = { STAT_VERSATILITY_RATING, STAT_MASTERY_RATING, STAT_HASTE_RATING,
                                                STAT_CRIT_RATING };

    struct navigators_intuition_cb_t : dbc_proc_callback_t
    {
      std::unordered_map<stat_e, buff_t*> buffs;

      navigators_intuition_cb_t( fs_player_t* p, const special_effect_t& e ) : dbc_proc_callback_t( p, e ), buffs{}
      {
        for ( stat_e stat : secondary_ratings )
        {
          buffs[ stat ] =
              make_buff<stat_buff_t>( p, fmt::format( "navigators_intuition_{}", util::stat_type_abbrev( stat ) ) )
                  ->add_stat(
                      stat, p->fs_weapon_trait_values.navigators_intuition_stats[ p->fs_weapons.navigators_intuition ] )
                  ->set_name_reporting( fmt::format( "Navigator's Intuition {}", util::stat_type_abbrev( stat ) ) )
                  ->set_duration(
                      p->fs_weapon_trait_values.navigators_intuition_duration[ p->fs_weapons.navigators_intuition ] );
        }
      }

      void execute( action_t*, action_state_t* s ) override
      {
        listener->sim->print_debug( "Navigator's Intuition proc: checking for highest stat" );
        auto stat = util::highest_stat( listener, secondary_ratings );
        for ( auto [ s, b ] : buffs )
        {
          if ( s == stat )
            b->trigger();
          else
            b->expire();
        }
      }
    };

    auto dbc = new navigators_intuition_cb_t( this, *effect );

    dbc->initialize();
    dbc->activate();
  }

  if ( fs_weapons.diamond_strike )
  {
    struct diamond_strike_t : public actions::fs_proc_spell_t
    {
      diamond_strike_t( std::string_view n, fs_player_t* p ) : base_t( n, p )
      {
        // random number - use actual ids later.
        id     = 2291;
        school = SCHOOL_MAGIC;

        name_str_reporting = "Diamond Strike";

        spell_power_mod.direct = p->fs_weapon_trait_values.diamond_strike_dmg[ p->fs_weapons.diamond_strike ];
      }

      double composite_target_da_multiplier( player_t* t ) const override
      {
        auto m = base_t::composite_target_da_multiplier( t );

        auto dia_m = ab::fs_p()->get_target_data( t )->debuffs.diamond_strike_amp->check_stack_value() +
                     ab::fs_p()->fs_gems.harmonious_diamond_amp * ab::fs_p()->fs_buffs.harmonious_soul->check();

        m *= 1.0 + dia_m;

        return m;
      }

      void impact( action_state_t* s ) override
      {
        base_t::impact( s );
        ab::fs_p()->get_target_data( s->target )->debuffs.diamond_strike_amp->trigger();
      }
    };

    auto effect          = new special_effect_t( this );
    effect->spell_id     = 2291;
    effect->name_str     = "diamond_strike";
    effect->proc_flags_  = PF_ALL_DAMAGE | PF_PERIODIC;
    effect->proc_flags2_ = PF2_ALL_HIT | PF2_PERIODIC_DAMAGE;
    effect->cooldown_    = 0_s;
    effect->ppm_         = -fs_weapon_trait_values.diamond_strike_ppm[ fs_weapons.diamond_strike ];
    effect->set_can_proc_from_procs( true );
    effect->rppm_scale_  = rppm_scale_e::RPPM_HASTE;
    effect->rppm_blp_    = real_ppm_t::BLP_ENABLED;
    effect->type         = special_effect_e::SPECIAL_EFFECT_EQUIP;

    special_effects.push_back( effect );

    effect->execute_action = create_fs_proc_action<diamond_strike_t>( "diamond_strike" );

    auto dbc = new dbc_proc_callback_t( this, *effect );

    dbc->initialize();
    dbc->activate();
  }

  struct stacking_proc_buff_t : dbc_proc_callback_t
  {
    buff_t* blocking_buff;
    buff_t* stacking_buff;
    stacking_proc_buff_t( fs_player_t* p, const special_effect_t& e, buff_t* stacking, buff_t* blocking_buff )
      : dbc_proc_callback_t( p, e ), blocking_buff( blocking_buff ), stacking_buff( stacking )
    {
    }

    fs_player_t* p() const
    {
      return static_cast<fs_player_t*>( listener );
    }

    void execute( action_t*, action_state_t* s ) override
    {
      stacking_buff->trigger();
    }

    void trigger( action_t* a, action_state_t* state ) override
    {
      if ( blocking_buff && blocking_buff->check() || state->result_amount < 0 )
        return;

      dbc_proc_callback_t::trigger( a, state );
    }
  };

  if ( fs_weapons.hidden_power )
  {
    auto fs_effect                   = new special_effect_t( this );
    fs_effect->spell_id              = 1341;
    fs_effect->name_str              = "hidden_power";
    fs_effect->proc_flags_           = PF_ALL_DAMAGE;
    fs_effect->proc_flags2_          = PF2_ALL_HIT;
    fs_effect->ppm_                  = -2.6;
    fs_effect->rppm_scale_           = rppm_scale_e::RPPM_HASTE;
    fs_effect->type                  = special_effect_e::SPECIAL_EFFECT_EQUIP;
    fs_effect->has_use_buff_override = true;

    special_effects.push_back( fs_effect );

    auto cb = new stacking_proc_buff_t( this, *fs_effect, fs_buffs.hidden_power_stacking, fs_buffs.hidden_power );
    cb->initialize();
    cb->activate();
  }

  if ( fs_weapons.seized_opportunity )
  {
    auto fs_effect                   = new special_effect_t( this );
    fs_effect->spell_id              = 1342;
    fs_effect->name_str              = "seized_opportunity";
    fs_effect->proc_flags_           = PF_ALL_DAMAGE | PF_PERIODIC;
    fs_effect->proc_flags2_          = PF2_CRIT;
    fs_effect->proc_chance_          = 1.0;
    fs_effect->set_can_proc_from_procs( true );
    fs_effect->type                  = special_effect_e::SPECIAL_EFFECT_EQUIP;
    fs_effect->has_use_buff_override = true;

    special_effects.push_back( fs_effect );

    auto cb =
        new stacking_proc_buff_t( this, *fs_effect, fs_buffs.seized_opportunity_stacking, fs_buffs.seized_opportunity );
    cb->initialize();
    cb->activate();
  }

  if ( fs_weapons.vengeful_soul )
  {
    auto effect                   = new special_effect_t( this );
    effect->spell_id              = 1343;
    effect->name_str              = "vengeful_soul";
    effect->proc_flags_           = PF_ALL_DAMAGE;
    effect->proc_flags2_          = PF2_CRIT;
    effect->cooldown_             = 0_s;
    effect->has_use_buff_override = true;
    effect->ppm_                  = -fs_weapon_trait_values.vengeful_soul_ppm;
    effect->rppm_scale_           = rppm_scale_e::RPPM_CRIT;
    effect->rppm_blp_             = real_ppm_t::BLP_ENABLED;
    effect->type                  = special_effect_e::SPECIAL_EFFECT_EQUIP;

    special_effects.push_back( effect );

    effect->custom_buff = fs_buffs.vengeful_soul;

    auto cb = new dbc_proc_callback_t( this, *effect );

    cb->initialize();
    cb->activate();
  }

  if ( fs_weapons.hunters_focus )
  {
    auto effect                   = new special_effect_t( this );
    effect->spell_id              = 1347;
    effect->name_str              = "hunters_focus";
    effect->proc_flags_           = PF_ALL_DAMAGE;
    effect->proc_flags2_          = PF2_HIT;
    effect->proc_chance_          = 1.0;
    effect->has_use_buff_override = true;
    effect->type                  = special_effect_e::SPECIAL_EFFECT_EQUIP;

    special_effects.push_back( effect );

    effect->custom_buff = fs_buffs.hunters_focus;

    auto cb = new dbc_proc_callback_t( this, *effect );

    cb->initialize();
    cb->activate();
  }


  if ( fs_weapons.inspired_allegiance )
  {
    struct inspired_allegiance_t: dbc_proc_callback_t
    {
      inspired_allegiance_t( fs_player_t* p, const special_effect_t& e )
        : dbc_proc_callback_t( p, e )
      {
      }

      fs_player_t* p() const
      {
        return static_cast<fs_player_t*>( listener );
      }

      void execute( action_t*, action_state_t* s ) override
      {
        if ( p()->weapon_cd )
        {
          p()->weapon_cd->adjust(
              -p()->fs_weapon_trait_values.inspired_allegiance_cdr[ p()->fs_weapons.inspired_allegiance ], true, true );
        }

        p()->get_target_data( p() )->buffs.inspired_allegiance->trigger();


        // TODO: Cache this.
        std::vector<fs_player_t*> allies;

        for ( auto& player : p()->sim->player_non_sleeping_list )
        {
          if ( player == p() || p()->is_sleeping() || p()->is_pet() || dynamic_cast<fs_player_t*>( player ) == nullptr )
            continue;

          allies.push_back( static_cast<fs_player_t*>( player ) );
        }

        rng().shuffle( allies.begin(), allies.end() );
        size_t max_targets = p()->fs_weapon_trait_values.inspired_allegiance_allies[ p()->fs_weapons.inspired_allegiance ];

        size_t count = 0;
        for ( auto& ally : allies )
        {
          ally->get_target_data( p() )->buffs.inspired_allegiance->trigger();

          if ( ++count >= max_targets )
            break;

        }
      }
    };

    auto fs_effect                   = new special_effect_t( this );
    fs_effect->spell_id              = 1368;
    fs_effect->name_str              = "inspired_allegiance";
    fs_effect->proc_flags_           = PF_ALL_DAMAGE;
    fs_effect->proc_flags2_          = PF2_ALL_HIT;
    fs_effect->ppm_                  = -1.2;
    fs_effect->rppm_scale_           = rppm_scale_e::RPPM_HASTE;
    fs_effect->type                  = special_effect_e::SPECIAL_EFFECT_EQUIP;
    fs_effect->has_use_buff_override = true;

    special_effects.push_back( fs_effect );

    auto cb = new inspired_allegiance_t( this, *fs_effect );
    cb->initialize();
    cb->activate();
  }
}

void fs_player_t::init_assessors()
{
  player_t::init_assessors();

  if ( fs_gems.gem_powers[ GEM_TOPAZ ] >= GEM_TIER_1 )
  {
    assessor_out_damage.add( assessor::TARGET_DAMAGE + 1, [ this ]( result_amount_type, action_state_t* s ) {
      if ( s->target->health_percentage() < 30.0 )
        fs_buffs.adrenaline_rush->trigger();
      return assessor::CONTINUE;
    } );
  }

  if ( fs_weapons.amethyst_splinters > 0 )
  {
    assessor_out_damage.add( assessor::TARGET_DAMAGE + 2, [ this ]( result_amount_type, action_state_t* s ) {
      if ( s->result == RESULT_CRIT )
      {
        residual_action::trigger( fs_actions.amethyst_splinters, s->target, s->result_amount );
      }
      return assessor::CONTINUE;
    } );
  }

  if ( fs_weapons.equipped_weapon == FSWEAPON_VOIDBRINGERS_TOUCH )
  {
    assessor_out_damage.add( assessor::TARGET_DAMAGE + 3, [ this ]( result_amount_type, action_state_t* s ) {
      if ( s->result_amount > 0 )
        voidbringer_accumulate( s->result_amount );

      return assessor::CONTINUE;
    } );
  }

  if ( fs_weapons.sapphire_aurastone )
  {
    assessor_out_damage.add( assessor::TARGET_DAMAGE + 3, [ this ]( result_amount_type, action_state_t* s ) {
      if ( s->result_amount > 0 && s->action && s->action->id != fs_actions.aurastone_dmg->id )
        sapphire_aurastone_accumulate( s->result_amount );

      return assessor::CONTINUE;
    } );
  }

  if ( fs_weapons.patient_soul )
  {
    buffs.movement->add_stack_change_callback( [ this ]( buff_t*, int old, int _new ) {
      if ( _new )
      {
        event_t::cancel( patient_soul_stopped );
        patient_soul_moved =
            make_event( sim, fs_weapon_trait_values.patient_soul_moving[ fs_weapons.patient_soul ], [ this ]() {
              fs_buffs.patient_soul->expire();
              patient_soul_moved = nullptr;
            } );
      }
      else
      {
        patient_soul_stopped =
            make_event( sim, fs_weapon_trait_values.patient_soul_stationary[ fs_weapons.patient_soul ], [ this ]() {
              fs_buffs.patient_soul->trigger();
              patient_soul_stopped = nullptr;
              event_t::cancel( patient_soul_moved );
            } );
      }
    } );
  }
}

// fs_player_t::init_finished ===================================================

void fs_player_t::init_finished()
{
  player_t::init_finished();

  if ( fs_gems.gem_powers[ GEM_RUBY ] >= GEM_TIER_5 && fs_options.ruby_allow_boss_amp )
  {
    sim->target_non_sleeping_list.register_callback(
        [ this ]( player_t* p ) { cache.invalidate( CACHE_PLAYER_DAMAGE_MULTIPLIER ); } );
  }

  if ( fs_gems.gem_powers[ GEM_EMERALD ] >= GEM_TIER_5 )
  {
    for ( auto action : action_list )
    {
      action->base_recharge_multiplier -= fs_gems.gem_powers[ GEM_EMERALD ] >= GEM_TIER_10 ? 0.12: 0.04;
      action->cooldown->adjust_recharge_multiplier();
    }
  }
}

void fs_player_t::spirit_refund()
{
  if ( fs_weapons.willful_momentum )
    fs_buffs.willful_momentum->trigger();
  //
  //if ( finesse_traits[ FINESSE_L ] > 0 )
  //  fs_buffs.finesse_l->trigger();

  if ( finesse_traits[ FINESSE_K ] > 0 )
    fs_buffs.finesse_k->trigger();

  fs_buffs.rising_spirit->trigger();

  resource_gain( RESOURCE_SPIRIT, spirit_refund_mul, fs_gains.spirit_procs );
}

void fs_player_t::used_ultimate()
{
  if ( fs_weapons.visions_of_grandeur > 0 && weapon_cd )
  {
    //weapon_cd->reset( false, -1 );

    unsigned grandeur = fs_weapons.visions_of_grandeur;
    double cdr        = fs_weapon_trait_values.grandeur_weapon_cdr[ grandeur ];
    weapon_cd->adjust( -cdr * cooldown_t::cooldown_duration( weapon_cd ), true, true );
  }

  if ( fs_sets.drakheims_absolution )
  {
    fs_buffs.drakheims_absolution->trigger();
  }

  if ( fs_weapons.sapphire_aurastone )
  {
    get_target_data( this )->debuffs.aurastone_accumulator->trigger();
  }
}

void fs_player_t::init_background_actions()
{
  player_t::init_background_actions();

  if ( fs_weapons.amethyst_splinters )
    fs_actions.amethyst_splinters = new actions::amethyst_splinters_t( "amethyst_splinters", this );
  if ( fs_weapons.equipped_weapon == FSWEAPON_VOIDBRINGERS_TOUCH )
    fs_actions.voidbringer_dmg = new actions::voidbringers_touch_dmg_t( "voidbringers_touch_dmg", this );
  if ( fs_weapons.sapphire_aurastone )
    fs_actions.aurastone_dmg = new actions::sapphire_aurastone_dmg_t( "sapphire_aurastone_dmg", this );
  if ( finesse_traits[ FINESSE_F ] )
    fs_actions.finesse_f = new actions::finesse_f_dmg_t( "finesse_f_dmg", this );
  if ( finesse_traits[ FINESSE_L ] )
    fs_actions.finesse_l = new actions::finesse_l_dmg_t( "finesse_l_dmg", this );
}

void fs_player_t::init_rng()
{
  player_t::init_rng();

  if ( finesse_traits[ FINESSE_D ] > 0 )
  {
    fs_rng_objects.finesse_d = get_accumulated_rng(
        "finesse_d", rng::CfromP( finesse_trait_values.finesse_d_chance[ finesse_traits[ FINESSE_D ] ] ) );
  }

  if ( finesse_traits[ FINESSE_F ] > 0 )
  {
    fs_rng_objects.finesse_f =
        get_accumulated_rng( "finesse_f", rng::CfromP( finesse_trait_values.finesse_f_drain_chance ) );
  }
}

// fs_player_t::reset ===========================================================

void fs_player_t::reset()
{
  player_t::reset();

  active_voidbringer_buffs.clear();
  brave_machinations_available = false;
  finesse_i_event              = nullptr;
  patient_soul_moved           = nullptr;
  patient_soul_stopped         = nullptr;
}

// fs_player_t::activate ========================================================

void fs_player_t::activate()
{
  player_t::activate();
}

// fs_player_t::arise ===========================================================

void fs_player_t::arise()
{
  player_t::arise();

  if ( finesse_traits[ FINESSE_I ] )
  {
    finesse_i_event = make_event( sim, finesse_trait_values.finesse_i_interval, [ this ]() { finesse_i_event_fn(); } );
  }

  if ( fs_weapons.patient_soul )
  {
    fs_buffs.patient_soul->trigger();
  }
}

void fs_player_t::finesse_i_event_fn()
{
  finesse_i_event = nullptr;
  if ( is_sleeping() )
    return;

  fs_buffs.finesse_i->trigger();
  finesse_i_event = make_event( sim, finesse_trait_values.finesse_i_interval, [ this ]() { finesse_i_event_fn(); } );
}

void fs_player_t::finesse_i_cdr( timespan_t cdr )
{
  if ( !finesse_i_event )
    return;

  
  // sim->print_debug( "{} reduces cooldown of Finesse I by {}", *this, cdr );

  if ( finesse_i_event->remains() < cdr )
  {
    event_t::cancel( finesse_i_event );
    finesse_i_event_fn();
    // sim->print_debug( "{} Finesse I is less than CDR away. Immediately triggering.", *this );
  }
  else
  {
    auto when = finesse_i_event->remains() - cdr;
    finesse_i_event->reschedule( when );
    // sim->print_debug( "{} Finesse I rescheduling event for {} ", *this, when + sim->current_time() );
  }
}


// fs_player_t::combat_begin ====================================================

void fs_player_t::combat_begin()
{
  player_t::combat_begin();
}

// fs_player_t::energy_regen_per_second =========================================

double fs_player_t::resource_regen_per_second( resource_e r ) const
{
  double reg = player_t::resource_regen_per_second( r );

  if ( r == RESOURCE_MANA && ( type == AEONA || type == XAVIAN ) && fs_buffs.spirit_of_heroism->check() )
  {
    // Half Tick Time and 2x regen rate.
    reg *= 6;
  }

  return reg;
}

double fs_player_t::resource_gain( resource_e resource_type, double amount, gain_t* source, action_t* action )
{
  double actual_amount = player_t::resource_gain( resource_type, amount, source, action );

  return actual_amount;
}

// fs_player_t::non_stacking_movement_modifier ==================================

double fs_player_t::non_stacking_movement_modifier() const
{
  double ms = player_t::non_stacking_movement_modifier();

  return ms;
}

// fs_player_t::stacking_movement_modifier===================================

double fs_player_t::stacking_movement_modifier() const
{
  double ms = player_t::stacking_movement_modifier();

  return ms;
}

void fs_player_t::voidbringer_accumulate( double damage )
{
  auto accumulated = fs_weapon_values.voidbringer_acc * damage;

  for ( size_t i = 0; i < active_voidbringer_buffs.size(); )
  {
      
    auto* buff = debug_cast<voidbringer_debuff_t*>( active_voidbringer_buffs[ i ] );
    
    if ( !buff )
    {
      sim->error( "{} has invalid voidbringer debuff in active_voidbringer_buffs vector.", *this );
      continue;
    }

    if ( !buff->check() )
    {
      sim->print_debug( "{} has voidbringer debuff on target {} in vector while buff is not active.", *this,
                        *buff->player );
      i++;
      continue;
    }
    auto cap = buff->current_cap;

    auto old_value = buff->current_value;
    buff->current_value = std::min( cap, buff->current_value + accumulated );
    sim->print_debug( "{} voidbringer accumulates {} dmg on target {}. (Stored: {} was: {})", *this, accumulated,
                      *buff->player, buff->current_value, old_value );
    if ( buff->current_value >= cap || buff->current_value >= buff->player->current_health() )
    {
      // This will remove the buff from active_voidbringer_buffs by swapping with last element.
      buff->expire();
    }
    else
    {
      i++;
    }
  }
}

void fs_player_t::sapphire_aurastone_accumulate( double damage )
{
  if ( !fs_buffs.spirit_of_heroism->check() )
    return;

  auto accumulated = fs_weapon_trait_values.sapphire_aurastone_dmg_acc[ fs_weapons.sapphire_aurastone ] * damage;

  for ( size_t i = 0; i < active_aurastone_buffs.size(); i++ )
  {
    auto* buff = debug_cast<aurastone_buff_t*>( active_aurastone_buffs[ i ] );

    if ( !buff )
    {
      sim->error( "{} has invalid aurastone debuff in active_aurastone_buffs vector.", *this );
      continue;
    }

    if ( !buff->check() )
    {
      sim->print_debug( "{} has aurastone debuff on target {} in vector while buff is not active.", *this,
                        *buff->player );
      continue;
    }


    auto old_value = buff->current_value;

    // auto cap = buff->current_cap;
    // buff->current_value = std::min( cap, buff->current_value + accumulated );

    buff->current_value += accumulated;
    sim->print_debug( "{} aurastone accumulates {} dmg on target {}. (Stored: {} was: {})", *this, accumulated,
                      *buff->player, buff->current_value, old_value );
  }
}

// fs_player_t::regen ===========================================================

void fs_player_t::regen( timespan_t periodicity )
{
  player_t::regen( periodicity );
}

}  // namespace fellowship