#pragma once
#include "util/util.hpp"

#include "simulationcraft.hpp"
#include "fs_player.hpp"

namespace fellowship
{  // UNNAMED NAMESPACE

// ==========================================================================
// FS Targetdata Definitions
// ==========================================================================

fs_player_td_t::fs_player_td_t( player_t* target, fs_player_t* source ) : actor_target_data_t( target, source ), dots(), debuffs()
{
}

// ==========================================================================
// FS Character Definition
// ==========================================================================

fs_player_t::fs_player_t( sim_t* sim, util::string_view name, race_e r, player_e p )
  : player_t( sim, p, name, r ), target_data(), fs_gems()
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
  double h = player_t::composite_melee_haste();

  if ( fs_gems.gem_powers[ GEM_TOPAZ ] >= 2280 )
  {
    h += 0.09;
  }
  else if ( fs_gems.gem_powers[ GEM_EMERALD ] >= 720 )
  {
    h += 0.03;
  }

  return h;
}

// fs_player_t::composite_spell_haste ==========================================

double fs_player_t::composite_spell_haste() const
{
  double h = player_t::composite_spell_haste();

  if ( fs_gems.gem_powers[ GEM_TOPAZ ] >= 2280 )
  {
    h += 0.09;
  }
  else if ( fs_gems.gem_powers[ GEM_EMERALD ] >= 720 )
  {
    h += 0.03;
  }

  return h;
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

  if ( fs_gems.gem_powers[ GEM_AMETHYST ] >= 2640 )
  {
    cdm *= 1.12;
  }
  else if ( fs_gems.gem_powers[ GEM_AMETHYST ] >= 960 )
  {
    cdm *= 1.04;
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

  if ( in_boss_encounter )
  {
    if ( fs_gems.gem_powers[ GEM_RUBY ] >= 2640 )
    {
      m *= 1.12;
    }
    else if ( fs_gems.gem_powers[ GEM_RUBY ] >= 960 )
    {
      m *= 1.04;
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

  return m;
}

// fs_player_t::composite_player_target_crit_chance =============================

double fs_player_t::composite_player_target_crit_chance( player_t* target ) const
{
  double c = player_t::composite_player_target_crit_chance( target );

  if ( fs_gems.gem_powers[ GEM_AMETHYST ] >= 120 && target->health_percentage() >= 50.0 )
  {
    if ( fs_gems.gem_powers[ GEM_AMETHYST ] >= 1200 )
    {
      c += 0.3;
    }
    else
    {
      c += 0.1;
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

// fs_player_t::create_action  ==================================================

action_t* fs_player_t::create_action( util::string_view name, util::string_view options_str )
{
  using namespace actions;


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

  
  if ( util::str_compare_ci( split[ 0 ], "sets" ) )
  {
    if ( split.size() == 2 )
    {
      if ( util::str_compare_ci( split[ 1 ], "haste_buff_on_ability_use" ) )
      {
        return make_ref_expr( name_str, fs_sets.haste_buff_on_ability_use );
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
      base.rating.heal_versatility = base.rating.spell_crit = base.rating.spell_haste = base.rating.spell_hit = 0.00017;

  /*base.stats.attribute[ STAT_AGILITY ] = 1000;*/
  base.stats.attribute[ STAT_STAMINA ] = 1000;
  resources.base[ RESOURCE_HEALTH ]    = 11480;

  base.health_per_stamina = 36.638;

  base.attack_power_per_strength = 1.0;
  base.attack_power_per_agility  = 1.0;
  base.spell_power_per_intellect = 1.0;

  base.mastery = 0.0;

  resources.base[ RESOURCE_SPIRIT ]                  = 100;
  resources.start_at[ RESOURCE_SPIRIT ]              = 0;
  resources.base_regen_per_second[ RESOURCE_SPIRIT ] = 100.0 / 300 * 1.7;

  //resources.base_regen_per_second[ RESOURCE_ENERGY ] = 10;

  //base_gcd = timespan_t::from_seconds( 1.0 );
  //min_gcd  = timespan_t::from_seconds( 1.0 );
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

  auto heroism_buff          = make_buff<fs_player_buff_t>( this, "spirit_of_heroism" );
  fs_buffs.spirit_of_heroism = heroism_buff;
  fs_buffs.spirit_of_heroism->set_pct_buff_type( STAT_PCT_BUFF_HASTE )->set_default_value( 0.3 )->set_duration( 20_s );

  auto ancestral_surge     = make_buff<fs_player_buff_t>( this, "ancestral_surge" );
  fs_buffs.ancestral_surge = ancestral_surge;
  fs_buffs.ancestral_surge->set_default_value( fs_gems.gem_powers[ GEM_SAPPHIRE ] >= 1200.0 ? 0.24 : 0.08 );

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
}

// fs_player_t::invalidate_cache =========================================

void fs_player_t::invalidate_cache( cache_e c )
{
  player_t::invalidate_cache( c );
}

void fs_player_t::create_options()
{
  player_t::create_options();

  add_option( opt_bool( "sets.haste_buff_on_ability_use", fs_sets.haste_buff_on_ability_use ) );
  add_option( opt_float( "sets.haste_buff_on_ability_use_haste", fs_sets.haste_buff_on_ability_use_haste ) );
  add_option( opt_timespan( "sets.haste_buff_on_ability_use_duration", fs_sets.haste_buff_on_ability_use_duration ) );
  add_option( opt_timespan( "sets.haste_buff_on_ability_use_cooldown", fs_sets.haste_buff_on_ability_use_cooldown ) );

  add_option( opt_float( "gems.ruby_power", fs_gems.gem_powers[ GEM_RUBY ] ) );
  add_option( opt_float( "gems.amethyst_power", fs_gems.gem_powers[ GEM_AMETHYST ] ) );
  add_option( opt_float( "gems.diamond_power", fs_gems.gem_powers[ GEM_DIAMOND ] ) );
  add_option( opt_float( "gems.topaz_power", fs_gems.gem_powers[ GEM_TOPAZ ] ) );
  add_option( opt_float( "gems.emerald_power", fs_gems.gem_powers[ GEM_EMERALD ] ) );
  add_option( opt_float( "gems.sapphire_power", fs_gems.gem_powers[ GEM_SAPPHIRE ] ) );
}

// fs_player_t::copy_from =======================================================

void fs_player_t::copy_from( player_t* source )
{
  fs_player_t* fs_player = static_cast<fs_player_t*>( source );
  player_t::copy_from( source );

  fs_sets    = fs_player->fs_sets;
  fs_options = fs_player->fs_options;
  fs_gems    = fs_player->fs_gems;
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
}

// fs_player_t::init_special_effects ============================================

void fs_player_t::init_special_effects()
{
  player_t::init_special_effects();

  if ( fs_gems.gem_powers[ GEM_RUBY ] >= 1560 )
  {
    passive.add_stat( STAT_STR_AGI_INT, 24 );
    passive.add_stat( STAT_STAMINA, 420 );
  }
  else if ( fs_gems.gem_powers[ GEM_RUBY ] >= 240 )
  {
    passive.add_stat( STAT_STR_AGI_INT, 8 );
    passive.add_stat( STAT_STAMINA, 140 );
  }

  if ( fs_gems.gem_powers[ GEM_AMETHYST ] >= 1560 )
  {
    passive.add_stat( STAT_CRIT_RATING, 300 );
    passive.add_stat( STAT_STAMINA, 300 );
  }
  else if ( fs_gems.gem_powers[ GEM_AMETHYST ] >= 240 )
  {
    passive.add_stat( STAT_CRIT_RATING, 100 );
    passive.add_stat( STAT_STAMINA, 100 );
  }

  if ( fs_gems.gem_powers[ GEM_TOPAZ ] >= 1560 )
  {
    passive.add_stat( STAT_HASTE_RATING, 300 );
    passive.add_stat( STAT_STAMINA, 300 );
  }
  else if ( fs_gems.gem_powers[ GEM_TOPAZ ] >= 240 )
  {
    passive.add_stat( STAT_HASTE_RATING, 100 );
    passive.add_stat( STAT_STAMINA, 100 );
  }

  if ( fs_gems.gem_powers[ GEM_EMERALD ] >= 1560 )
  {
    passive.add_stat( STAT_VERSATILITY_RATING, 300 );
    passive.add_stat( STAT_STAMINA, 300 );
  }
  else if ( fs_gems.gem_powers[ GEM_EMERALD ] >= 240 )
  {
    passive.add_stat( STAT_VERSATILITY_RATING, 100 );
    passive.add_stat( STAT_STAMINA, 100 );
  }

  if ( fs_gems.gem_powers[ GEM_SAPPHIRE ] >= 1560 )
  {
    passive.add_stat( STAT_MASTERY_RATING, 300 );
    passive.add_stat( STAT_STAMINA, 300 );
  }
  else if ( fs_gems.gem_powers[ GEM_SAPPHIRE ] >= 240 )
  {
    passive.add_stat( STAT_MASTERY_RATING, 100 );
    passive.add_stat( STAT_STAMINA, 100 );
  }

  if ( fs_gems.gem_powers[ GEM_DIAMOND ] >= 1560 )
  {
    passive.add_stat( STAT_STR_AGI_INT, 45 );
    passive.add_stat( STAT_STAMINA, 180 );
  }
  else if ( fs_gems.gem_powers[ GEM_DIAMOND ] >= 240 )
  {
    passive.add_stat( STAT_STR_AGI_INT, 15 );
    passive.add_stat( STAT_STAMINA, 60 );
  }

  if ( fs_gems.gem_powers[ GEM_AMETHYST ] >= 2280 )
  {
    base.spell_crit_chance += 0.09;
    base.attack_crit_chance += 0.09;
  }
  else if ( fs_gems.gem_powers[ GEM_AMETHYST ] >= 720 )
  {
    base.spell_crit_chance += 0.03;
    base.attack_crit_chance += 0.03;
  }

  if ( fs_gems.gem_powers[ GEM_SAPPHIRE ] >= 2280 )
  {
    base.mastery += 0.09;
  }
  else if ( fs_gems.gem_powers[ GEM_EMERALD ] >= 720 )
  {
    base.mastery += 0.03;
  }

  if ( fs_gems.gem_powers[ GEM_EMERALD ] >= 2280 )
  {
    base.versatility += 0.09;
  }
  else if ( fs_gems.gem_powers[ GEM_EMERALD ] >= 720 )
  {
    base.versatility += 0.03;
  }

  if ( fs_gems.gem_powers[ GEM_SAPPHIRE ] >= 120 )
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

  if ( fs_gems.gem_powers[ GEM_SAPPHIRE ] >= 2640 )
  {
    fs_buffs.spirit_of_heroism->base_buff_duration += 18_s;
  }
  else if ( fs_gems.gem_powers[ GEM_SAPPHIRE ] >= 960 )
  {
    fs_buffs.spirit_of_heroism->base_buff_duration += 6_s;
  }

  if ( fs_sets.haste_buff_on_ability_use )
  {
    auto effect                   = new special_effect_t( this );
    effect->spell_id              = 1317;
    effect->name_str              = "haste_buff_on_ability_use";
    effect->proc_flags_           = PF_ALL_DAMAGE;
    effect->proc_flags2_          = PF2_ALL_HIT;
    effect->has_use_buff_override = true;
    effect->cooldown_             = fs_sets.haste_buff_on_ability_use_cooldown;
    effect->ppm_                  = -fs_sets.haste_buff_on_ability_use_ppm;
    effect->rppm_scale_           = rppm_scale_e::RPPM_NONE;
    effect->rppm_blp_             = real_ppm_t::BLP_DISABLED;
    effect->type                  = special_effect_e::SPECIAL_EFFECT_EQUIP;

    special_effects.push_back( effect );

    auto haste_buff = make_buff( this, "tuzari_haste_buff" )
                          ->set_pct_buff_type( STAT_PCT_BUFF_HASTE )
                          ->set_duration( fs_sets.haste_buff_on_ability_use_duration )
                          ->set_default_value( fs_sets.haste_buff_on_ability_use_haste );

    effect->custom_buff = haste_buff;

    auto dbc = new dbc_proc_callback_t( this, *effect );

    dbc->initialize();
    dbc->activate();
  }
}

// fs_player_t::init_finished ===================================================

void fs_player_t::init_finished()
{
  player_t::init_finished();

  if ( fs_gems.gem_powers[ GEM_RUBY ] >= 960.0 )
  {
    sim->target_non_sleeping_list.register_callback( [ this ]( player_t* p ) {
      if ( p->is_boss() )
        cache.invalidate( CACHE_PLAYER_DAMAGE_MULTIPLIER );
    } );
  }
}

void fs_player_t::init_background_actions()
{
  player_t::init_background_actions();

}

// fs_player_t::reset ===========================================================

void fs_player_t::reset()
{
  player_t::reset();
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

  if ( r == RESOURCE_SPIRIT )
  {
    reg *= 1.0 + cache.mastery();
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

// fs_player_t::regen ===========================================================

void fs_player_t::regen( timespan_t periodicity )
{
  player_t::regen( periodicity );
}


}  // namespace fellowship