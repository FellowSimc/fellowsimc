#pragma once

#include "util/span.hpp"
#include "util/generic.hpp"
#include <string>
#include "util/timespan.hpp"
#include "util/string_view.hpp"

struct character_attribute_data_t
{
  const char* character_reference_name;
  double agility;
  double armor;
  double balancingDamageAura;
  double balancingHealAura;
  double baseHealth;
  double blockChance;
  double blockDamageReduction;
  double blockDamageReductionMultiplier;
  double cooldownRecovery;
  double cooldownReduction;
  double critChance;
  double critChanceAgainstHealthyTarget;
  double critChanceAgainstLowHealthTarget;
  double critMultiplier;
  double damageThreatMultiplier;
  double defaultBaseHealthMultiplier;
  double detection;
  double dodgeChance;
  double expertise;
  double haste;
  double healThreatMultiplier;
  double healthRegenRate;
  double healthRegenTickTime;
  double incomingDamageMultiplier;
  double intellect;
  double killScore;
  double lifeSteal;
  double mana;
  double manaRegenRate;
  double manaRegenTickTime;
  double maxMana;
  double maxResources;
  double maxResourcesSecond;
  double maxResourcesTertiary;
  double maxSpiritPoints;
  double minHealth;
  double moveSpeed;
  double outOfCombatHealthRegenRate;
  double outOfCombatHealthRegenTickTime;
  double outOfCombatManaRegenRate;
  double outOfCombatManaRegenTickTime;
  double parryChance;
  double passiveMoveSpeed;
  double physicalDamageReduction;
  double power;
  double reflectHeal;
  double resist;
  double resources;
  double resourcesSecond;
  double resourcesTertiary;
  double sourceDamageReflect;
  double spirit;
  double spiritGenerationDamage;
  double spiritGenerationHeal;
  double spiritPointValue;
  double spiritPoints;
  double spiritRefundChanceScale;
  double spiritRegenPeriod;
  double spiritRegenRate;
  double stamina;
  double staminaToMaxHealthMultiplier;
  double strength;
  double subtlety;
  double weaponDamageMax;
  double weaponDamageMin;

  const char* name_cstr() const
  {
    return character_reference_name;
  }

  static const character_attribute_data_t* find( util::string_view name );
  static util::span<const character_attribute_data_t> data();
  static const character_attribute_data_t& nil();
};

struct character_attribute_data_nil_t : public character_attribute_data_t
{
  constexpr character_attribute_data_nil_t() : character_attribute_data_t()
  {
    character_reference_name         = "null";
    agility                          = 0.0;
    armor                            = 0.0;
    balancingDamageAura              = 0.0;
    balancingHealAura                = 0.0;
    baseHealth                       = 0.0;
    blockChance                      = 0.0;
    blockDamageReduction             = 0.0;
    blockDamageReductionMultiplier   = 0.0;
    cooldownRecovery                 = 0.0;
    cooldownReduction                = 0.0;
    critChance                       = 0.0;
    critChanceAgainstHealthyTarget   = 0.0;
    critChanceAgainstLowHealthTarget = 0.0;
    critMultiplier                   = 0.0;
    damageThreatMultiplier           = 0.0;
    defaultBaseHealthMultiplier      = 0.0;
    detection                        = 0.0;
    dodgeChance                      = 0.0;
    expertise                        = 0.0;
    haste                            = 0.0;
    healThreatMultiplier             = 0.0;
    healthRegenRate                  = 0.0;
    healthRegenTickTime              = 0.0;
    incomingDamageMultiplier         = 0.0;
    intellect                        = 0.0;
    killScore                        = 0.0;
    mana                             = 0.0;
    manaRegenRate                    = 0.0;
    manaRegenTickTime                = 0.0;
    maxMana                          = 0.0;
    maxResources                     = 0.0;
    maxResourcesSecond               = 0.0;
    maxResourcesTertiary             = 0.0;
    maxSpiritPoints                  = 0.0;
    minHealth                        = 0.0;
    moveSpeed                        = 0.0;
    outOfCombatHealthRegenRate       = 0.0;
    outOfCombatHealthRegenTickTime   = 0.0;
    outOfCombatManaRegenRate         = 0.0;
    outOfCombatManaRegenTickTime     = 0.0;
    parryChance                      = 0.0;
    passiveMoveSpeed                 = 0.0;
    physicalDamageReduction          = 0.0;
    power                            = 0.0;
    reflectHeal                      = 0.0;
    resist                           = 0.0;
    resources                        = 0.0;
    resourcesSecond                  = 0.0;
    resourcesTertiary                = 0.0;
    sourceDamageReflect              = 0.0;
    spirit                           = 0.0;
    spiritGenerationDamage           = 0.0;
    spiritGenerationHeal             = 0.0;
    spiritPointValue                 = 0.0;
    spiritPoints                     = 0.0;
    spiritRefundChanceScale          = 0.0;
    spiritRegenPeriod                = 0.0;
    spiritRegenRate                  = 0.0;
    stamina                          = 0.0;
    staminaToMaxHealthMultiplier     = 0.0;
    strength                         = 0.0;
    subtlety                         = 0.0;
    weaponDamageMax                  = 0.0;
    weaponDamageMin                  = 0.0;
  }
};

inline constexpr character_attribute_data_nil_t character_attribute_data_nil_v{};
inline const character_attribute_data_t& character_attribute_data_t::nil()
{
  return character_attribute_data_nil_v;
}
