// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================
#ifndef ITEM_HPP
#define ITEM_HPP

#include "util/span.hpp"

#include "client_data.hpp"
#include "data_enums.hh"

#define MAX_ITEM_EFFECT 5
#define MAX_ITEM_STAT 10
#define MAX_ITEM_SOCKET_SLOT 3


constexpr double primary_divisible              = 15;
constexpr double primary_power_base             = 1.03;
constexpr double primary_and_stamina_multiplier = 10;
constexpr double secondary_multiplier           = 2.4;
constexpr double secondary_base_multiplier      = 72;
constexpr double slot_rarity_base               = 0.92;

struct item_calculation_settings_t
{
  double primary_weight;
  double secondary_weight;
  double stamina_weight;
  double primary_multiplier;
  double secondary_multiplier;
  double weight_multiplier;
};

constexpr double fixed_slot_primary_weight       = 0.25;
constexpr double fixed_slot_secondary_weight     = 0.3;
constexpr double fixed_slot_stamina_weight       = 0.5;
constexpr double fixed_slot_primary_multiplier   = 0.7;
constexpr double fixed_slot_secondary_multiplier = 0.6;  // Fixed Affix are 0.6. Random affix are 1.5.
constexpr double fixed_slot_weight_multiplier    = 0.15;

constexpr item_calculation_settings_t fixed_slot_settings = {
    fixed_slot_primary_weight,     fixed_slot_secondary_weight,     fixed_slot_stamina_weight,
    fixed_slot_primary_multiplier, fixed_slot_secondary_multiplier, fixed_slot_weight_multiplier };

constexpr double dynamic_slot_primary_weight       = 0.25;
constexpr double dynamic_slot_secondary_weight     = 0.3;
constexpr double dynamic_slot_stamina_weight       = 0.5;
constexpr double dynamic_slot_primary_multiplier   = 0.7;
constexpr double dynamic_slot_secondary_multiplier = 1.5;  // Fixed Affix are 0.6. Random affix are 1.5.
constexpr double dynamic_slot_weight_multiplier    = 0.15;

constexpr item_calculation_settings_t dynamic_slot_settings = {
    dynamic_slot_primary_weight,     dynamic_slot_secondary_weight,     dynamic_slot_stamina_weight,
    dynamic_slot_primary_multiplier, dynamic_slot_secondary_multiplier, dynamic_slot_weight_multiplier };

struct dbc_item_data_t {
  struct stats_t {
    int16_t type_e; // item_mod_type
    int     alloc;
    float   socket_mul;
  };

  const char* name;
  unsigned id;
  unsigned flags_1;
  unsigned flags_2;
  unsigned type_flags;
  int      level;                 // Ilevel
  int      req_level;
  int      req_skill;
  int      req_skill_level;
  int      quality;
  int      inventory_type;
  int      item_class;
  int      item_subclass;
  int      bind_type;
  float    delay;
  float    dmg_range;
  float    item_modifier;
  const stats_t* _dbc_stats;
  uint8_t  _dbc_stats_count;
  unsigned class_mask;
  uint64_t race_mask;
  int      socket_color[MAX_ITEM_SOCKET_SLOT];       // item_socket_color
  int      gem_properties;
  int      id_socket_bonus;
  int      id_set;
  int      id_curve;
  unsigned id_artifact;
  int      crafting_quality;

  bool is_armor()
  { return item_class == ITEM_CLASS_ARMOR && ( item_subclass >= ITEM_SUBCLASS_ARMOR_CLOTH && item_subclass <= ITEM_SUBCLASS_ARMOR_SHIELD ); }
  bool warforged() const
  { return ( type_flags & RAID_TYPE_WARFORGED ) == RAID_TYPE_WARFORGED; }
  bool lfr() const
  { return ( type_flags & RAID_TYPE_LFR ) == RAID_TYPE_LFR; }
  bool normal() const
  { return type_flags == 0; }
  bool heroic() const
  { return ( type_flags & RAID_TYPE_HEROIC ) == RAID_TYPE_HEROIC; }
  bool mythic() const
  { return ( type_flags & RAID_TYPE_MYTHIC ) == RAID_TYPE_MYTHIC; }

  static const dbc_item_data_t& find( unsigned id, bool ptr );

  static const dbc_item_data_t& nil()
  { return dbc::nil<dbc_item_data_t>; }

  static util::span<const util::span<const dbc_item_data_t>> data( bool ptr );
};

#endif /* ITEM_HPP */
