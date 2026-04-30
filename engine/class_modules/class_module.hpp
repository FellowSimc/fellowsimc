// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#pragma once

#include "config.hpp"

#include "sc_enums.hpp"
#include "util/generic.hpp"
#include "util/string_view.hpp"

#include <vector>

struct player_t;
struct sim_t;

// Class module entry point
struct module_t
{
  player_e type;

  module_t( player_e t ) : type( t )
  {
  }

  virtual ~module_t()                                                                               = default;
  virtual player_t* create_player( sim_t* sim, util::string_view name, race_e r = RACE_NONE ) const = 0;
  virtual bool valid() const                                                                        = 0;
  virtual void init( player_t* ) const                                                              = 0;
  virtual void static_init() const
  {
  }
  virtual void register_hotfixes() const
  {
  }
  virtual void create_actions( player_t* ) const
  {
  }
  virtual void combat_begin( sim_t* ) const = 0;
  virtual void combat_end( sim_t* ) const   = 0;

  static const module_t* mara();
  static const module_t* rime();
  static const module_t* ardeos();
  static const module_t* aeona();
  static const module_t* elarion();
  static const module_t* xavian();
  static const module_t* enemy();
  static const module_t* tank_dummy_enemy();
  static const module_t* heal_enemy();

  static const module_t* get( player_e t )
  {
    switch ( t )
    {
      case MARA:
        return mara();
      case RIME:
        return rime();
      case ARDEOS:
        return ardeos();
      case AEONA:
        return aeona();
      case XAVIAN:
        return xavian();
      case ELARION:
        return elarion();
      case ENEMY:
        return enemy();
      case TANK_DUMMY:
        return tank_dummy_enemy();
      default:
        break;
    }
    return nullptr;
  }

  static void init()
  {
    for ( player_e i = PLAYER_NONE; i < PLAYER_MAX; i++ )
    {
      const module_t* m = get( i );
      if ( m )
      {
        m->static_init();
        m->register_hotfixes();
      }
    }
  }
};
