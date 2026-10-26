from math import pow
import os
import json
import subprocess
import uuid
from enum import Enum
from random import choice, random, sample, choices
from functools import reduce
from operator import attrgetter
from copy import deepcopy
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

class Slot(Enum):
  NONE = 0
  HEAD = 1
  NECK = 2
  SHOULDERS = 3
  BACK = 4
  CHEST = 5
  WRISTS = 6
  GLOVES = 7
  LEGS = 8
  FEET = 9
  RING_1 = 10
  RING_2 = 11
  RELIC_1 = 12
  RELIC_2 = 13
  WEAPON = 14

SlotToSimcMap: dict[Slot, str] = {
  Slot.NONE: "",
  Slot.HEAD: "head",
  Slot.NECK: "neck",
  Slot.SHOULDERS: "shoulders",
  Slot.BACK: "back",
  Slot.CHEST: "chest",
  Slot.WRISTS: "wrists",
  Slot.GLOVES: "hands",
  Slot.LEGS: "legs",
  Slot.FEET: "feet",
  Slot.RING_1: "finger1",
  Slot.RING_2: "finger2",
  Slot.RELIC_1: "trinket1",
  Slot.RELIC_2: "trinket2",
  Slot.WEAPON: "main_hand",
}

class WeightingPool(Enum):
  WEIGHTING_POOL_AUTO = 0,
  WEIGHTING_POOL_STAMINA = 1
  WEIGHTING_POOL_PRIMARY = 2
  WEIGHTING_POOL_SECONDARY_1 = 3
  WEIGHTING_POOL_SECONDARY_2 = 4
  WEIGHTING_POOL_SECONDARY_3 = 5
  WEIGHTING_POOL_SECONDARY_4 = 6
  WEIGHTING_POOL_MAX = 7


class Characters(Enum):
  NONE = 0
  
  HELENA = 1
  MEIKO = 2
  XAVIAN = 3

  GUNDE = 4
  TARIQ = 5
  MARA = 6
  RIME = 7
  ELARION = 8
  ARDEOS = 9

  SYLVIE = 10
  VIGOUR = 11
  AEONA = 12

class Stats(Enum):
  NONE = 0
  PRIMARY = 1
  STAMINA = 2
  HASTE = 3
  CRIT = 4
  EXPERTISE = 5
  SPIRIT = 6

StatsToSimcMap: dict[Stats, str] = {
  Stats.NONE: "",
  Stats.PRIMARY: "primary",
  Stats.STAMINA: "stamina",
  Stats.HASTE: "haste",
  Stats.CRIT: "crit",
  Stats.EXPERTISE: "expertise",
  Stats.SPIRIT: "spirit",
}

class Role(Enum):
  NONE = 0
  DPS  = 1
  TANK = 2
  HEAL = 3


class ItemCalculationSettings:
  primary_weight: float = 0.0
  secondary_weight: float = 0.0
  stamina_weight: float = 0.0
  primary_multiplier: float = 0.0
  secondary_multiplier: float = 0.0
  weight_multiplier: float = 0.0

  def __init__(self, primary_weight: float = 0, secondary_weight: float = 0, stamina_weight: float = 0, primary_multiplier: float = 0, secondary_multiplier: float = 0, weight_multiplier: float = 0):
    self.primary_weight = primary_weight
    self.secondary_weight = secondary_weight
    self.stamina_weight = stamina_weight
    self.primary_multiplier = primary_multiplier
    self.secondary_multiplier = secondary_multiplier
    self.weight_multiplier = weight_multiplier


primary_divisible              = 15;
primary_power_base             = 1.03;
primary_and_stamina_multiplier = 10;
secondary_multiplier           = 2.4;
secondary_base_multiplier      = 72;
slot_rarity_base               = 1.0;

base_settings = ItemCalculationSettings(0.25, 0.3, 0.5, 0.7, 0.6, 0.15)


slot_weights = {
  Slot.NONE: 0,
  Slot.HEAD: 0.088,
  Slot.NECK: 0.0616,
  Slot.SHOULDERS: 0.0792,
  Slot.BACK: 0.0528,
  Slot.CHEST: 0.088,
  Slot.WRISTS: 0.0528,
  Slot.GLOVES: 0.0616,
  Slot.LEGS: 0.088,
  Slot.FEET: 0.0704,
  Slot.RING_1: 0.0528,
  Slot.RING_2: 0.0528,
  Slot.RELIC_1: 0.0528,
  Slot.RELIC_2: 0.0528,
  Slot.WEAPON: 0.132,
}


minimum_ilvl = {
  Slot.NONE: 0,
  Slot.HEAD: 0,
  Slot.NECK: 45,
  Slot.SHOULDERS: 0,
  Slot.BACK: 0,
  Slot.CHEST: 0,
  Slot.WRISTS: 45,
  Slot.GLOVES: 0,
  Slot.LEGS: 0,
  Slot.FEET: 0,
  Slot.RING_1: 45,
  Slot.RING_2: 45,
  Slot.RELIC_1: 105,
  Slot.RELIC_2: 105,
  Slot.WEAPON: 150,
}

class WeightingPool(Enum):
  WEIGHTING_POOL_AUTO = 0,
  WEIGHTING_POOL_STAMINA = 1
  WEIGHTING_POOL_PRIMARY = 2
  WEIGHTING_POOL_SECONDARY_1 = 3
  WEIGHTING_POOL_SECONDARY_2 = 4
  WEIGHTING_POOL_SECONDARY_3 = 5
  WEIGHTING_POOL_SECONDARY_4 = 6
  WEIGHTING_POOL_MAX = 7


class ItemVariant:
  variant_name: str
  simc_name: str
  stamina_weight: float = 0
  primary_weight: float = 0
  secondary_1_weight: float = 0
  secondary_2_weight: float = 0
  secondary_3_weight: float = 0
  secondary_4_weight: float = 0

  def __init__(self, variant_name, stamina_weight, primary_weight, secondary_1_weight, secondary_2_weight, secondary_3_weight, secondary_4_weight, simc_name = "" ):
    self.variant_name = variant_name
    self.stamina_weight = stamina_weight
    self.primary_weight = primary_weight
    self.secondary_1_weight = secondary_1_weight
    self.secondary_2_weight = secondary_2_weight
    self.secondary_3_weight = secondary_3_weight
    self.secondary_4_weight = secondary_4_weight
    self.simc_name = simc_name
  
  def __repr__(self):
    return self.variant_name

null_variant: ItemVariant = ItemVariant("None", 0, 0, 0, 0, 0, 0)

item_variants: list[ItemVariant] = [
  ItemVariant("Legendary",  0.22, 0.22, 0.36, 0.36, 0.36, 0.36, "legendary" ),
  ItemVariant("55_45",      0.22, 0.22, 0.22, 0.18, 0.00, 0.00, "ffff" ),
  ItemVariant("70_30",      0.22, 0.22, 0.28, 0.12, 0.00, 0.00, "svth"),
  ItemVariant("55_45_Neck", 0.40, 0.00, 0.22, 0.18, 0.00, 0.00, "ffffneck" ),
  ItemVariant("70_30_Neck", 0.40, 0.00, 0.28, 0.12, 0.00, 0.00, "svthneck"),
  ItemVariant("55_45_Ring", 0.00, 0.24, 0.33, 0.27, 0.00, 0.00, "ffffring" ),
  ItemVariant("70_30_Ring", 0.00, 0.24, 0.42, 0.18, 0.00, 0.00, "svthring" ),
  ItemVariant("Weap",       0.00, 0.44, 0.24, 0.24, 0.24, 0.24, "weap" ),
  ItemVariant("Weap_Tank",  0.22, 0.22, 0.24, 0.24, 0.24, 0.24, "weaptank" ),
  ItemVariant("Relic",      0.00, 0.00, 0.84, 0.00, 0.00, 0.00, "relic"),
]

class ItemRarity(Enum):
  UNCOMMON = 1
  RARE = 2
  EPIC = 3
  CHAMPION = 4
  HEROIC = 5

RarityToSimcMap: dict[ItemRarity, str] = {
  ItemRarity.UNCOMMON: "uncommon",
  ItemRarity.RARE: "rare",
  ItemRarity.EPIC: "epic",
  ItemRarity.CHAMPION: "heroic",
  ItemRarity.HEROIC: "regal"
}

maximum_modifiers: dict[ItemRarity, int] = {
  ItemRarity.UNCOMMON: 0,
  ItemRarity.RARE: 1,
  ItemRarity.EPIC: 2,
  ItemRarity.CHAMPION: 3,
  ItemRarity.HEROIC: 3,
}

def RandomSecondary(quantity: int = 1) -> Stats:
  if quantity < 1:
    quantity = 1
  if quantity > 4:
    quantity = 4

  stats = [Stats.HASTE, Stats.CRIT, Stats.EXPERTISE, Stats.SPIRIT]
  return sample(stats, quantity)

class Item:
  item_type: Slot = Slot.NONE
  item_level: int = 0
  tempers: int = 0
  variant: ItemVariant = null_variant
  item_secondarys: list[Stats, 4] = [Stats.NONE, Stats.NONE, Stats.NONE, Stats.NONE]
  rarity: ItemRarity = ItemRarity.UNCOMMON
  modifier_strings: list[str] = []

  def get_variant(self, name: str):
    for variant in item_variants:
      if variant.variant_name == name:
        return variant

    return null_variant


  def is_left_side(self):
    match self.item_type:
      case Slot.HEAD:
        return True
      case Slot.SHOULDERS:
        return True
      case Slot.BACK:
        return True
      case Slot.CHEST:
        return True
      case Slot.GLOVES:
        return True
      case Slot.LEGS:
        return True
      case Slot.FEET:
        return True
      case _:
        return False
      
      

  # DT_DynamicHeroItemSlotDefinition

  def set_random_modifiers(self):

    mod_number = self.modifiers()

    if mod_number < 1:
      return

    pools_by_mod = {
      "gems": ["ruby", "amethyst", "topaz", "emerald", "sapphire", "diamond"],
      "aspects": ["amethyst_splinters", "brave_machinations", "diamond_strike", "emerald_judgement", "martial_initiative", "ruby_storm", "sapphire_aurastone", "visions_of_grandeur"],
      "stats": ["undines_cunning", "sphinxs_cunning", "chimeras_cunning", "salamanders_cunning", "unicorns_cunning", "banshees_cunning"],
      "stats_150": ["undines_cunning:1", "sphinxs_cunning:1", "chimeras_cunning:1", "salamanders_cunning:1", "unicorns_cunning:1", "banshees_cunning:1"],
      "stats_300": ["undines_cunning:2", "sphinxs_cunning:2", "chimeras_cunning:2", "salamanders_cunning:2", "unicorns_cunning:2", "banshees_cunning:2"],
      "glyphs": ["the_trickster", "the_subduer", "the_sinister", "the_vehement", "the_philosopher", "the_mystic", "the_monarch", "the_wayfarer"],
      "sigils": ["inspired_allegiance", "kindling", "seized_opportunity", "hidden_power", "hunters_focus", "patient_soul", "vengeful_soul", "willful_momentum"]
    }
    

    pools = []

    pools.extend(["gems"]*2)

    if self.item_level >= 300:
      pools.extend(["stats_300"] * 2)
    elif self.item_level >= 150:
      pools.extend(["stats_150"] * 2)
    else:
      pools.extend(["stats"] * 2)

    if (self.is_left_side()):
      if (self.item_level >= 120):
        pools.extend(["glyphs"] * 2)      
      if (self.item_level >= 240):
        pools.extend(["sigils"] * 2)
      if (self.item_level >= 270):
        pools.extend(["aspects"] * 2)

    sampled_pools = sample(pools, mod_number)

    mods = []

    for pool in sampled_pools:
      mods.append(choice(pools_by_mod[pool]))

    self.modifier_strings = mods

  def simc_string(self) -> str:
    stats = []

    if self.variant.secondary_1_weight:
      stats.append(self.item_secondarys[0])
    if self.variant.secondary_2_weight:
      stats.append(self.item_secondarys[1])
    if self.variant.secondary_3_weight:
      stats.append(self.item_secondarys[2])
    if self.variant.secondary_4_weight:
      stats.append(self.item_secondarys[3])

    stats = [StatsToSimcMap[x] for x in stats]

    affix_string = ""

    if (self.modifiers() > 0):
      aff_list = "/".join(self.modifier_strings)
      affix_string = f",affixes={aff_list}"

    secondary_string = len(stats) and ",main_secondary=" + "/".join(stats) or ""

    return f"{SlotToSimcMap[self.item_type]}={SlotToSimcMap[self.item_type]},ilevel={self.item_level},variant={self.variant.simc_name},rarity={RarityToSimcMap[self.rarity]}{secondary_string},tempering={self.tempers}{affix_string}"

  def __repr__(self):
    return f"ILVL: {self.item_level:3} | {self.item_type:14} | {self.variant.__repr__():10} | Stam: {self.getItemStamina():2.0f} | Prim: {self.getItemPrimary():2.0f} | Secondary: {self.getItemTotalSecondary():2.0f}"

  def hasSocket(self):
    if self.item_level < 15:
      return False

    if self.item_type == Slot.WRISTS or self.item_type == Slot.NECK or self.item_type == Slot.RING_1 or self.item_type == Slot.RING_2:
      return True

    return False

  def set_variant(self, variant: str):
    self.variant = self.get_variant(variant)

    if self.item_type == Slot.NECK:
      if (resolved_variant := self.get_variant(variant+"_Neck")) != null_variant:
        self.variant = resolved_variant
    if self.item_type == Slot.RING_1 or self.item_type == Slot.RING_2:
      if (resolved_variant := self.get_variant(variant+"_Ring")) != null_variant:
        self.variant = resolved_variant


  def __init__(self, slot: Slot, variant: str, ilvl: int = 0, secondary_1: Stats = Stats.NONE, secondary_2: Stats = Stats.NONE, rarity: ItemRarity = ItemRarity.UNCOMMON, tempers: int = 0):
    self.item_type = slot
    self.item_level = ilvl
    self.tempers = tempers
    self.rarity = rarity

    self.item_secondarys = [secondary_1, secondary_2, Stats.NONE, Stats.NONE]

    self.set_variant(variant)

    if (self.item_type == Slot.RELIC_1 or self.item_type == Slot.RELIC_2):
      self.variant = self.get_variant("Relic")

    if (self.variant.variant_name == "Legendary" or self.variant.variant_name == "Weap" or self.variant.variant_name == "Weap_Tank"):
      self.item_secondarys = [Stats.HASTE, Stats.CRIT, Stats.EXPERTISE, Stats.SPIRIT]

    self.set_random_modifiers()

  def weighting_pool_multiplier(self, pool: WeightingPool) -> float:
    match pool:
      case WeightingPool.WEIGHTING_POOL_PRIMARY:
        return self.variant.primary_weight;
      case WeightingPool.WEIGHTING_POOL_STAMINA:
        return self.variant.stamina_weight;
      case WeightingPool.WEIGHTING_POOL_SECONDARY_1:
        return self.variant.secondary_1_weight;
      case WeightingPool.WEIGHTING_POOL_SECONDARY_2:
        return self.variant.secondary_2_weight;
      case WeightingPool.WEIGHTING_POOL_SECONDARY_3:
        return self.variant.secondary_3_weight;
      case WeightingPool.WEIGHTING_POOL_SECONDARY_4:
        return self.variant.secondary_4_weight;

    return 0.0

  def modifiers(self) -> int:
    return maximum_modifiers[self.rarity]

  def slotMultiplier(self) -> float:
    return slot_weights[self.item_type]
  
  def getStatValue( self, stat: Stats, pool: WeightingPool = WeightingPool.WEIGHTING_POOL_AUTO ) -> float:

    if ( stat == Stats.NONE ):
      return 0.0

    amount: float = 0.0

    if ( self.item_level < 1 ):
      return 0.0

    if ( self.variant == null_variant ):
      return 0.0

    # Make it easier to change in the future if multiple exist again.
    settings: ItemCalculationSettings = base_settings

    if ( stat == Stats.PRIMARY or stat == Stats.STAMINA ):
      amount = pow( primary_power_base, self.item_level / primary_divisible ) * pow( self.item_level, 0.5 ) * primary_and_stamina_multiplier * settings.primary_multiplier / settings.weight_multiplier
      if ( stat == Stats.STAMINA):
        amount *= settings.stamina_weight
        if ( self.item_level >= 315 ):
          amount *= 1.5
      else:
        amount *= settings.primary_weight
    else:
      amount = pow( self.item_level * secondary_base_multiplier, 0.5 ) * secondary_multiplier * settings.secondary_weight * settings.secondary_multiplier / settings.weight_multiplier

    if ( pool == WeightingPool.WEIGHTING_POOL_AUTO ):
      if ( stat == Stats.STAMINA ):
        pool = WeightingPool.WEIGHTING_POOL_STAMINA
      elif ( stat == Stats.PRIMARY ):
        pool = WeightingPool.WEIGHTING_POOL_PRIMARY
      else:
        pool = WeightingPool.WEIGHTING_POOL_SECONDARY_1
    
    amount *= self.weighting_pool_multiplier( pool )

    # amount /= pow( slot_rarity_base, RARITY_DATA[ parsed.rarity - 1 ].stat_scale_factor );

    amount *= self.slotMultiplier()
    
    return round(amount)

  def getItemStamina(self):
    if self.weighting_pool_multiplier(WeightingPool.WEIGHTING_POOL_STAMINA) > 0.0:
      
      tempering_stats = self.tempers
      if self.item_level >= 315:
        tempering_stats *= 2

      return self.getStatValue( Stats.STAMINA ) + tempering_stats
    
    return 0.0
  
  def getItemPrimary(self):
    if self.weighting_pool_multiplier(WeightingPool.WEIGHTING_POOL_PRIMARY) > 0.0:
      return self.getStatValue( Stats.PRIMARY ) + self.tempers

    return 0.0
    
  def getItemTotalSecondary(self):
    stats = 0.0

    for pool in [WeightingPool.WEIGHTING_POOL_SECONDARY_1, WeightingPool.WEIGHTING_POOL_SECONDARY_2, WeightingPool.WEIGHTING_POOL_SECONDARY_3, WeightingPool.WEIGHTING_POOL_SECONDARY_4]:
      if self.weighting_pool_multiplier( pool ) > 0.0:
        stats += self.getStatValue( Stats.HASTE, pool ) + self.tempers

    return stats
  
  def getItemSecondary(self, stat: Stats):
    stats = 0.0
    index = -1
    for pool in [WeightingPool.WEIGHTING_POOL_SECONDARY_1, WeightingPool.WEIGHTING_POOL_SECONDARY_2, WeightingPool.WEIGHTING_POOL_SECONDARY_3, WeightingPool.WEIGHTING_POOL_SECONDARY_4]:
      index += 1
      if (self.item_secondarys[index] != stat):
        continue
      if self.weighting_pool_multiplier( pool ) > 0.0:
        stats += self.getStatValue( Stats.HASTE, pool ) + self.tempers

    return stats

  def getItemStat(self, stat: Stats):
    if stat == Stats.PRIMARY:
      return self.getItemPrimary()
    elif stat == Stats.STAMINA:
      return self.getItemStamina()
    else:
      return self.getItemSecondary(stat)

  def getItemHaste(self):
    self.getItemSecondary(Stats.HASTE)
  def getItemCrit(self):
    self.getItemSecondary(Stats.CRIT)
  def getItemExpertise(self):
    self.getItemSecondary(Stats.EXPERTISE)
  def getItemSpirit(self):
    self.getItemSecondary(Stats.SPIRIT)


class HealthScaling:
  hp_per_stamina: float
  base_hp: float

  def __init__(self, base_hp, hp_per_stamina):
    self.base_hp = base_hp
    self.hp_per_stamina = hp_per_stamina

Elarion_HP = HealthScaling(1687, 51.857)
Mara_HP = HealthScaling(1724, 53.499)

class AbilityUnlocks(Enum):
  ULTIMATE = 1
  ABILITY_1 = 2
  ABILITY_2 = 3
  ABILITY_3 = 4
  ABILITY_4 = 5

PowerForAbility: dict[AbilityUnlocks, float] = {
  AbilityUnlocks.ULTIMATE: 0.03,
  AbilityUnlocks.ABILITY_1: 0.03,
  AbilityUnlocks.ABILITY_2: 0.00,
  AbilityUnlocks.ABILITY_3: 0.03,
  AbilityUnlocks.ABILITY_4: 0.00,
}

GemTotalCostBySize = [1, 3, 9, 27]
GemPowerBySize = [150, 250, 350, 500]

class Difficulty(Enum):
  NONE = 0
  EASY = 1
  MEDIUM = 2
  HARD = 3
  MAX = 4

lootPerDifficulty: dict[Difficulty, list[int]] = {
  Difficulty.EASY: [15, 15],
  Difficulty.MEDIUM: [30, 15],
  Difficulty.HARD: [45, 30, 15],
}

class GemData:
  minChance: float = 0
  maxChance: float = 0
  minIlvl: float = 0
  maxIlvl: float = 0

  def __init__(self, minChance, maxChance, minIlvl= 45, maxIlvl = 300):
    self.minChance = minChance
    self.maxChance = maxChance
    self.minIlvl = minIlvl
    self.maxIlvl = maxIlvl

gemDataPerDifficulty: dict[Difficulty, GemData]  = {
  Difficulty.EASY: GemData(0.05, 0.07),
  Difficulty.MEDIUM: GemData(0.075, 0.105),
  Difficulty.HARD: GemData(0.1, 0.14),
}

class Character:
  role: Role = Role.DPS
  items: dict[Slot, Item] = {}
  base_stats: dict[Stats, float] = {}
  base_stat_percent: dict[Stats, float] = {}
  offence_stat_weight: dict[Stats, float] = {}

  hp_scaling: HealthScaling = Mara_HP
  completed_runs: int = 0
  talents: int = 0
  abilityUnlocks: dict[AbilityUnlocks, bool] = {}
  gems: int = 0
  best_ilvl: int = 0
  character_uuid: str = uuid.uuid4().hex

  def simc_output(self, suffix=""):
    out = f"""rime={self.character_uuid}{suffix}

apl/rime/rime_apl.simc

"""

    if self.talents in talent_strings:
      out += talent_strings[self.talents] + "\n"

    if self.items[Slot.WEAPON].item_level > 0:
      out += "weapon=icicles_of_anzhyr\n"

    if self.items[Slot.RELIC_1].item_level > 0:
      out += "sets.draconic_might=1\n"
    
    if self.items[Slot.RELIC_2].item_level > 0:
      out += "sets.dark_prophecy=1\n"

    out += "\n"

    gem_power = self.optimalGemPower()

    gem_out = ""

    amethyst = False
    ruby = False
    diamond = False
    topaz = False
    emerald = False

    if (gem_power >= 600):
      gem_out += "gems.amethyst_power=600\n"
      gem_power -= 600
      amethyst = True
    if (gem_power >= 600):
      gem_out += "gems.ruby_power=600\n"
      gem_power -= 600
      ruby = True
    if (gem_power >= 300):
      gem_out += "gems.diamond_power=300\n"
      gem_power -= 300
      diamond = True
    if (gem_power >= 300 and not amethyst):
      gem_out += "gems.amethyst_power=300\n"
      gem_power -= 300
      amethyst = True
    if (gem_power >= 300 and not topaz):
      gem_out += "gems.topaz_power=300\n"
      gem_power -= 300
      topaz = True
    if (gem_power >= 300 and not emerald):
      gem_out += "gems.emerald_power=300\n"
      gem_power -= 300
      emerald = True
    if (gem_power >= 150 and not diamond):
      gem_out += "gems.diamond_power=150\n"
      gem_power -= 150
      diamond = True
    if (gem_power >= 150 and not ruby):
      gem_out += "gems.ruby_power=150\n"
      gem_power -= 150
      ruby = True
    if (gem_power >= 150 and not amethyst):
      gem_out += "gems.amethyst_power=150\n"
      gem_power -= 150
      amethyst = True
    if (gem_power >= 150 and not topaz):
      gem_out += "gems.topaz_power=150\n"
      gem_power -= 150
      topaz = True
    if (gem_power >= 150 and not emerald):
      gem_out += "gems.emerald_power=150\n"
      gem_power -= 150
      emerald = True

    out += "\n"
    out += gem_out
    out += "\n"

    for item in self.items.values():
      if (item.item_level > 0):
        out += item.simc_string() + "\n"

    return out

  def run_sim(self):
    os.makedirs("apl/generated/harness_sims/input",exist_ok=True)
    os.makedirs("apl/generated/harness_sims/output",exist_ok=True)

    with open(f"apl/generated/harness_sims/input/{self.character_uuid}.simc", "w") as sim_file:
      sim_file.write(self.simc_output())

    run_sim_file(self.character_uuid)

    with open(f"apl/generated/harness_sims/output/{self.character_uuid}.json", "r") as f:
      output_data = json.load(f)
      return output_data["sim"]["players"][0]["collected_data"]["dps"]["mean"]    
    

  def optimalGemPower(self):
    itemsWithSocket = 0

    for item in self.items.values():
      if item.hasSocket():
        itemsWithSocket += 1

    if itemsWithSocket <= 0:
      return 0

    if self.gems <= itemsWithSocket:
      return self.gems * GemPowerBySize[0]

    minimalState = self.gems // itemsWithSocket
    
    index = -1
    for i, cost in enumerate(GemTotalCostBySize):
      if cost > minimalState:
        break

      index = i

    gem_power = GemPowerBySize[index] * itemsWithSocket
    remaining_gems = self.gems - GemTotalCostBySize[index] * itemsWithSocket

    if index < len(GemTotalCostBySize) - 1:
      upgrade_cost = GemTotalCostBySize[index+1] - GemTotalCostBySize[index]
      gained_power = GemPowerBySize[index+1] - GemPowerBySize[index]
      valid_upgrades = min(remaining_gems // upgrade_cost, itemsWithSocket)
      gem_power += gained_power * valid_upgrades

    return gem_power

  def __repr__(self):
    return f"Avg ILVL: {self.averageItemLevel():5.1f} | Power Estimate: {self.getCharacterOffencePowerEstimate():5.1f} | Current Health: {self.getCharacterHealth():5.0f} | Primary Stat: {self.getStat(Stats.PRIMARY):3.0f} | Haste: {self.getSecondaryPercent(Stats.HASTE)*100:6.2f}% | Crit: {self.getSecondaryPercent(Stats.EXPERTISE)*100:6.2f}% | Exp: {self.getSecondaryPercent(Stats.CRIT)*100:6.2f}% | Spirit: {self.getSecondaryPercent(Stats.SPIRIT)*100:6.2f}% | Gems {self.gems:3} | Optimal Gem Power: {self.optimalGemPower():4}"

  def maximumDifficulty( self, maximumDifficulty: Difficulty ):
    if self.completed_runs < 1 or maximumDifficulty == Difficulty.EASY:
      return Difficulty.EASY
    elif self.completed_runs < 2 or maximumDifficulty == Difficulty.MEDIUM:
      return Difficulty.MEDIUM
    else:
      return Difficulty.HARD

  def __init__( self, role: Role ):
      self.role = role

      for ability in AbilityUnlocks:
        self.abilityUnlocks[ability] = False

      for slot in Slot:
        if slot == Slot.NONE:
          continue
        
        self.items[slot] = self.createRandomItem(slot, 0)

      for stat in Stats:
        if stat == Stats.NONE:
          continue
        
        self.base_stats[stat] = 0
        self.base_stat_percent[stat] = 0
        self.offence_stat_weight[stat] = 1.0

      # self.offence_stat_weight[Stats.SPIRIT] = 0.7
      # self.offence_stat_weight[Stats.HASTE] = 0.3

      self.base_stats[Stats.PRIMARY] = 100
      self.base_stats[Stats.STAMINA] = 100
      self.base_stat_percent[Stats.CRIT] = 0.05

  def randomRarity(self, ilvl: int) -> ItemRarity:
    if (ilvl <= 15):
      return ItemRarity.UNCOMMON

    uncommonWeight: int = 0
    if ( ilvl < 60 ):
      uncommonWeight = 24
    elif ( ilvl == 60 ):
      uncommonWeight = 18
    elif ( ilvl == 75 ):
      uncommonWeight = 12
    elif ( ilvl >= 90 ):
      uncommonWeight = 0

    rareWeight: int = 0
    if ( ilvl < 30 ):
      rareWeight = 0
    elif ( ilvl == 30 ):
      rareWeight = 12
    elif ( ilvl <= 150 ):
      rareWeight = 24
    elif ( ilvl == 180 ):
      rareWeight = 12

    epicWeight: int = 0

    if (ilvl >= 90 and ilvl <= 135):
      epicWeight = ((ilvl - 75)//15) * 4
    elif (ilvl >= 150 and ilvl <= 255):
      epicWeight = 24
    elif (ilvl == 270):
      epicWeight = 16
    elif (ilvl == 285):
      epicWeight = 8

    regalWeight: int = 0

    if (ilvl >= 225 and ilvl <= 240):
      regalWeight = ((ilvl - 200)//15) * 6
    elif (ilvl > 240):
      regalWeight = 24

    weights = (uncommonWeight, rareWeight, epicWeight, 0, regalWeight)

    return choices([ItemRarity.UNCOMMON, ItemRarity.RARE, ItemRarity.EPIC, ItemRarity.CHAMPION, ItemRarity.HEROIC], weights=weights)[0]

  def createRandomItem(self, slot: Slot, ilvl: int, equip: bool = False) -> Item | None:
    if slot == Slot.NONE:
      return
    secondary_1, secondary_2 = RandomSecondary(2)
    variant = choice(["70_30", "55_45"])

    if slot == Slot.WEAPON:
      variant = self.role == Role.TANK and "Weap_Tank" or "Weap"
    
    tempers = 0 
    if ilvl <= 60 and ilvl > 0:
      tempers = 1
    # elif ilvl >= 105 and ilvl < 150:
    #   tempers = 1
    # elif ilvl >= 195:
    #   tempers = 1

    item = Item(slot, variant, secondary_1=secondary_1, secondary_2=secondary_2, ilvl=ilvl, rarity=self.randomRarity(ilvl), tempers=tempers)

    if equip:
      self.items[slot] = item

    return item

  def getStat(self, stat: Stats):
    amount = self.base_stats[stat]
    
    for item in self.items.values():
      amount += item.getItemStat(stat)

    return amount

  def secondaryToPercent(self, amount: float) -> float:
    return self.applyStatDR( amount * 0.0016)

  def applyStatDR(self, percent: float) -> float:
    breakPoints: list[float] = [ 0.10, 0.15, 0.20, 0.25 ]
    breakPointMultipliers: list[float] = [ 1, 0.98, 0.96 * 0.98, 0.94 * 0.96 * 0.98, 0.92 * 0.94 * 0.96 * 0.98 ]
  
    if ( percent <= breakPoints[ 0 ] ):
      return percent * breakPointMultipliers[ 0 ]

    out = breakPoints[ 0 ] * breakPointMultipliers[ 0 ]
    percent -= out
    
    current_breakpoint: float  = out
    previous_breakpoint: float = 0.0
    i:int                   = 1

    while (i < len(breakPoints)):
      previous_breakpoint = current_breakpoint
      current_breakpoint += ( breakPoints[ i ] - breakPoints[ i - 1 ] ) / breakPointMultipliers[ i ]
      
      if ( percent <= current_breakpoint - previous_breakpoint ):
        out += percent * breakPointMultipliers[ i ]
        return out
      else:
        diff: float = ( current_breakpoint - previous_breakpoint )
        out += diff * breakPointMultipliers[ i ]
        percent -= diff
      i += 1

    if ( percent > 0 ):
      out += percent * breakPointMultipliers[ i ]
    
    return out

  def rollForGem(self, diff: Difficulty, ilvl: int):
    gemDropData: GemData = gemDataPerDifficulty[diff]

    if ilvl < gemDropData.minIlvl:
      return False

    ilvl = min(gemDropData.maxIlvl, ilvl)
    normalised_ilvl = (ilvl - gemDropData.minIlvl)/(gemDropData.maxIlvl - gemDropData.minIlvl)
    drop_chance = gemDropData.minChance + normalised_ilvl * gemDropData.maxChance

    return random() <= drop_chance


  def averageItemLevel(self) -> float:
    return reduce(lambda acc, item: acc + item.item_level, self.items.values(),0) / 14

  def minimumItemLevel(self) -> float:
    return min(self.items.values(), key=attrgetter("item_level")).item_level
  
  def minimumItemLevelUpgrade(self, upgrade_amount: float = 0 ) -> float:
    current_minimum = 2 ** 16
    current_minimum_item: Item = self.items[Slot.HEAD]

    for item in self.items.values():
      if item.item_level + upgrade_amount >= minimum_ilvl[item.item_type]:
        if item.item_level < current_minimum:
          current_minimum = item.item_level
          current_minimum_item = item

    created_item_level = current_minimum + upgrade_amount

    for item in self.items.values():
      if created_item_level >= minimum_ilvl[item.item_type]:
        if item.item_level < current_minimum:
          current_minimum = item.item_level
          current_minimum_item = item

    return current_minimum, created_item_level

  def getRandomUpgradeSlot(self, upgrade_amount: float):
    min_ilvl, created_ilvl = self.minimumItemLevelUpgrade(upgrade_amount)

    restricted_upgrades: list[Item] = []
    unrestricted_upgrades: list[Item] = []

    for item in self.items.values():
      if (item.item_level == min_ilvl and minimum_ilvl[item.item_type] <= created_ilvl):
        if minimum_ilvl[item.item_type] > 0 and item.item_level == 0:
          restricted_upgrades.append(item)
        else:
          unrestricted_upgrades.append(item)

    if len(restricted_upgrades) > 0:
      return choice(restricted_upgrades).item_type, created_ilvl
    else:
      return choice(unrestricted_upgrades).item_type, created_ilvl

  

  def generateRandomUpgrades(self, upgrade_amounts: list[int], max_ilvl = 300):
    upgrade_amounts = sorted(upgrade_amounts, reverse=True)

    old_ilvls = []
    new_ilvls = []
    slots = []

    for upgrade_amount in upgrade_amounts:
      slot, ilvl = self.getRandomUpgradeSlot(upgrade_amount)
      ilvl = min(ilvl, max_ilvl)

      if (self.items[slot].item_level >= ilvl):
        break

      old_ilvls.append(self.items[slot].item_level)
      new_ilvls.append(ilvl)
      slots.append(slot)

      self.items[slot] = self.createRandomItem(slot, ilvl)

    return tuple(zip(slots, old_ilvls, new_ilvls))


  def getSecondaryPercent(self, stat: Stats):
    return self.base_stat_percent[stat] + self.secondaryToPercent(self.getStat(stat))

  def getCharacterHealth(self):
    stamina: float = self.getStat(Stats.STAMINA)
    return self.hp_scaling.base_hp + self.hp_scaling.hp_per_stamina * stamina

  def getCharacterOffencePowerEstimate(self):

    power: float = self.getStat(Stats.PRIMARY)


    for secondary in [Stats.HASTE, Stats.CRIT, Stats.EXPERTISE, Stats.SPIRIT]:
      effective_multiplier: float = self.getSecondaryPercent(secondary) * self.offence_stat_weight[secondary]
      power *= 1.0 + effective_multiplier

    for ability, unlocked in self.abilityUnlocks.items():
      if unlocked:
        power *= 1.0 + PowerForAbility[ability]

    power *= pow(1.04, self.talents)

    if self.items[Slot.RELIC_1].item_level > 0:
      power *= 1.04
    if self.items[Slot.RELIC_2].item_level > 0:
      power *= 1.04
    if self.items[Slot.WEAPON].item_level > 0:
      power *= 1.04

    power *= pow(1.1, self.optimalGemPower() / 600)

    for item in self.items.values():
      power *= pow(1.012, item.modifiers())

    return power

  def completeDungeon( self, diff: Difficulty = Difficulty.MAX ):
    diff = self.maximumDifficulty( diff )
      
    self.completed_runs += 1
    
    generated = self.generateRandomUpgrades(lootPerDifficulty[diff])

    best_item_new_ilvl = max([x[2] for x in generated])
    
    if self.rollForGem(diff, best_item_new_ilvl):
      self.gems += 1

    if self.best_ilvl < 15 and best_item_new_ilvl >= 15:
      pass
    elif self.best_ilvl < 30 and best_item_new_ilvl >= 30:
      pass
    elif self.best_ilvl < 45 and best_item_new_ilvl >= 45:
      self.gems += 3
      slot = Slot.NONE
      if self.items[Slot.RING_1].item_level < 45:
        slot = Slot.RING_1
      elif self.items[Slot.RING_2].item_level < 45:
        slot = Slot.RING_2
      if slot != Slot.NONE:
        self.items[slot] = self.createRandomItem(slot, 45)
        self.items[slot].rarity = ItemRarity.RARE        
    elif self.best_ilvl < 60 and best_item_new_ilvl >= 60:
      pass
    elif self.best_ilvl < 75 and best_item_new_ilvl >= 75:
      self.gems += 3
    elif self.best_ilvl < 90 and best_item_new_ilvl >= 90:
      pass
    elif self.best_ilvl < 105 and best_item_new_ilvl >= 105:
      slot = Slot.NONE
      if self.items[Slot.RELIC_1].item_level < 105:
        slot = Slot.RELIC_1
      elif self.items[Slot.RELIC_2].item_level < 105:
        slot = Slot.RELIC_2
      if slot != Slot.NONE:
        self.items[slot] = self.createRandomItem(slot, 105)
        self.items[slot].rarity = ItemRarity.RARE      
    elif self.best_ilvl < 120 and best_item_new_ilvl >= 120:
      pass
    elif self.best_ilvl < 135 and best_item_new_ilvl >= 135:
      self.gems += 3
    elif self.best_ilvl < 150 and best_item_new_ilvl >= 150:
      pass
    elif self.best_ilvl < 165 and best_item_new_ilvl >= 165:
      pass
    elif self.best_ilvl < 180 and best_item_new_ilvl >= 180:
      self.gems += 3
      if self.items[Slot.NECK].item_level < 180:
        self.items[Slot.NECK] = self.createRandomItem(Slot.NECK, 180)
        self.items[Slot.NECK].rarity = ItemRarity.EPIC
    elif self.best_ilvl < 195 and best_item_new_ilvl >= 195:
      pass
    elif self.best_ilvl < 210 and best_item_new_ilvl >= 210:
      if self.items[Slot.WEAPON].item_level < 210:
        self.items[Slot.WEAPON] = self.createRandomItem(Slot.WEAPON, 210)
        self.items[Slot.WEAPON].rarity = ItemRarity.EPIC
    elif self.best_ilvl < 225 and best_item_new_ilvl >= 225:
      self.gems += 3
    elif self.best_ilvl < 240 and best_item_new_ilvl >= 240:
      pass
    elif self.best_ilvl < 255 and best_item_new_ilvl >= 255:
      pass
    elif self.best_ilvl < 270 and best_item_new_ilvl >= 270:
      if self.items[Slot.WEAPON].item_level < 270:
        self.items[Slot.WEAPON] = self.createRandomItem(Slot.WEAPON, 270)
        self.items[Slot.WEAPON].rarity = ItemRarity.EPIC
    elif self.best_ilvl < 285 and best_item_new_ilvl >= 285:
      self.gems += 3
    elif self.best_ilvl < 300 and best_item_new_ilvl >= 300:
      pass

    self.best_ilvl = best_item_new_ilvl
    
    if (self.completed_runs == 1):
      self.abilityUnlocks[AbilityUnlocks.ULTIMATE] = True
    if (self.completed_runs == 2):
      self.abilityUnlocks[AbilityUnlocks.ABILITY_1] = True
      self.abilityUnlocks[AbilityUnlocks.ABILITY_2] = True
    if (self.completed_runs == 3):
      self.abilityUnlocks[AbilityUnlocks.ABILITY_3] = True
      self.abilityUnlocks[AbilityUnlocks.ABILITY_4] = True
    if (self.completed_runs == 4):
      self.talents += 2
    if (self.completed_runs == 5):
      self.talents += 2
    if (self.completed_runs == 7):
      self.talents += 1
    if (self.completed_runs == 9):
      self.talents += 2
    if (self.completed_runs == 10):
      self.talents += 1
    if (self.completed_runs == 12):
      self.talents += 2
    if (self.completed_runs == 14):
      self.talents += 2
    if (self.completed_runs == 16):
      self.talents += 2
      if self.items[Slot.WEAPON].item_level < 150:
        self.items[Slot.WEAPON] = self.createRandomItem(Slot.WEAPON, 150)
        self.items[Slot.WEAPON].rarity = ItemRarity.EPIC




null_character = Character(Role.NONE)


def generate_run_data(upgrade_amounts: list[int], record_all_runs: bool = False, return_character: bool = False, log_output: bool = False):
  character = Character(Role.DPS)
  runs = 0
  output_data = []
  if (record_all_runs):
    output_data.append((character.averageItemLevel(), character.getCharacterOffencePowerEstimate(), character.getCharacterHealth(), runs))
  while (upgrades := character.generateRandomUpgrades(upgrade_amounts)):
    runs += 1
    if (record_all_runs):
      output_data.append((character.averageItemLevel(), character.getCharacterOffencePowerEstimate(), character.getCharacterHealth(), runs))
    if log_output:
      print(f"Run: {runs}")
      for upg in upgrades:
        print(f"{upg[0].__repr__()+':':20} {upg[1]:3} => {upg[2]:3}")
    
    if log_output:
      print(character)
  
  if log_output:
    print(runs)

  if not (record_all_runs):
    output_data.append((character.averageItemLevel(), character.getCharacterOffencePowerEstimate(), character.getCharacterHealth(), runs))

  if return_character:
    return(output_data, character)


  return(output_data)

def generate_run_data_new(record_all_runs: bool = False, return_character: bool = False, log_output: bool = False, difficulty: Difficulty = Difficulty.MAX):
  character = Character(Role.DPS)
  runs = 0
  output_data = []
  if (record_all_runs):
    output_data.append((character.averageItemLevel(), character.getCharacterOffencePowerEstimate(), character.getCharacterHealth(), runs, character.gems))
  while (character.averageItemLevel() < 300):
    character.completeDungeon(diff=difficulty)
    if (record_all_runs):
      output_data.append((character.averageItemLevel(), character.getCharacterOffencePowerEstimate(), character.getCharacterHealth(), character.completed_runs, character.gems))
  
  if log_output:
    print(runs)

  if not (record_all_runs):
    output_data.append((character.averageItemLevel(), character.getCharacterOffencePowerEstimate(), character.getCharacterHealth(), runs, character.gems))

  if return_character:
    return(output_data, character)


  return(output_data)

def debug_runs(upgrade_amounts: list[int]):
  d,c = generate_run_data(upgrade_amounts, record_all_runs=False, return_character=True)

  total_sec = 0
  total_haste = 0
  total_crit = 0 
  total_spirit = 0
  total_exp = 0


  for item in c.items.values():
    print(item)
    total_sec += item.getItemTotalSecondary()
    total_haste += item.getItemStat(Stats.HASTE)
    total_crit += item.getItemStat(Stats.CRIT)
    total_spirit += item.getItemStat(Stats.SPIRIT)
    total_exp += item.getItemStat(Stats.EXPERTISE)

  print(total_sec, total_haste, total_crit, total_spirit, total_exp)
  print(total_sec == (total_haste + total_crit + total_spirit + total_exp))

  # high_run = None
  # low_run = None
  
  # for _ in range(1000):
  #   d,c = generate_run_data(upgrade_amounts, record_all_runs=False, return_character=True)
  #   if d[-1][1] >= 245 and not high_run:
  #     high_run = c
  #     print(high_run)
  #     for item in high_run.items.values():
  #       print(item)
      
  #   if d[-1][1] <= 240 and not low_run:
  #     low_run = c
  #     print(low_run)
  #     for item in low_run.items.values():
  #       print(item)

  #   if high_run and low_run:
  #     break

def format_equation(poly, variable="x", precision=3):
  terms = []
  degree = len(poly.coefficients) - 1

  for i, coefficient in enumerate(poly.coefficients):
      power = degree - i

      if abs(coefficient) < 1e-10:
          continue

      sign = "+" if coefficient > 0 else "-"
      value = f"{abs(coefficient):.{precision}f}"

      if power == 0:
          term = value
      elif power == 1:
          term = f"{value}{variable}"
      else:
          term = f"{value}{variable}^{power}"

      if not terms:
          terms.append(f"-{term}" if coefficient < 0 else term)
      else:
          terms.append(f" {sign} {term}")

  return "y = " + "".join(terms) if terms else "y = 0"

def create_runs_plot(difficulty: Difficulty, file_prefix="", iterations = 1000):
  data = []

  for _ in range(iterations):
    data.extend(generate_run_data_new(record_all_runs=True, difficulty=difficulty))
    # data.extend(generate_run_data(upgrade_amounts, record_all_runs=True))

  df = pd.DataFrame(data, columns=["ilvl", "power", "health", "runs", "gems"])

  # Generate smooth x values for the regression lines
  x_fit = np.linspace(df["ilvl"].min(), df["ilvl"].max(), 200)

  degree = 5

  power_plot = df["power"]/min(df["power"]) * 3

  poly1 = np.poly1d(np.polyfit(df["ilvl"], power_plot, degree+3))
  poly2 = np.poly1d(np.polyfit(df["ilvl"], df["health"], degree))

  y1_fit = poly1(x_fit)
  y2_fit = poly2(x_fit)

  equation1 = format_equation(poly1)
  equation2 = format_equation(poly2)

  # Create two separate graphs
  fig, axes = plt.subplots(1, 2, figsize=(16, 8))

  data_x = [0,15,30,45,60,75,90,105,120,135,150,165,180,195,210,225,240,255,270,285,300,315,318]
  data_y = np.array([2.652,3.871,5.307,6.006,6.874,7.484,9.361,11.167,14.333,15.009,16.254,17.656,18.984,20.013,21.368,22.782,24.554,26.449,28.286,30.138,31.200,35.100,48.750])
  
  # Graph 1: x vs df["power"]
  axes[0].scatter(df["ilvl"], power_plot, color="blue", label="Data")
  # axes[0].plot(
  #     x_fit, y1_fit, color="red",
  #     label=equation1
  # )
  axes[0].plot(data_x, data_y, color="red", label="Bretton Curve")
  axes[0].set_xlabel("Item Level")
  axes[0].set_ylabel("Combat Power Heuristic")
  axes[0].set_title("Item Level vs Combat Power")
  axes[0].legend()
  axes[0].grid(True)

  # Graph 2: x vs df["health"]
  axes[1].scatter(df["ilvl"], df["health"], color="green", label="Data")
  axes[1].plot(
      x_fit, y2_fit, color="red",
      label=equation2
  )
  axes[1].set_xlabel("Item Level")
  axes[1].set_ylabel("Health")
  axes[1].set_title("Item Level vs Health")
  axes[1].legend()
  axes[1].grid(True)

  # # Graph 2: x vs df["health"]
  # axes[2].scatter(df["ilvl"], df["gems"], color="red", label="Data")
  # axes[2].set_xlabel("Item Level")
  # axes[2].set_ylabel("Gems")
  # axes[2].set_title("Item Level vs Gems")
  # axes[2].legend()
  # axes[2].grid(True)

  # Print the regression equations
  print(f"power fit: {equation1}")
  print(f"health fit: {equation2}")
  
  # # Convert to a DataFrame
  # df = pd.DataFrame(data, columns=["x", "df["power"]", "df["health"]", "misc"])

  # # Calculate medians for each distinct x

  group_df = df.groupby("ilvl", as_index=False)[
        ["power", "health", "gems", "runs"]
    ]
  
  median_df = group_df.median()

  median_df["power"] /= min(median_df["power"])
  median_df["health"] /= min(median_df["health"])

  median_df["power"] *= 3

  # print(median_df)

  median_df.to_csv(f"{file_prefix}scaling.csv")

  ilv = 0

  output_numbers = [(0, float(median_df.iloc[[0]]["power"].values[0]))]


  for row in median_df.itertuples(index=True):
    id = row.Index
    if row.ilvl > (ilv // 15 + 1)*15:
      ilv =  (ilv // 15 + 1)*15
      prev_row = median_df.iloc[[id-1]]

      ilvl_diff = row.ilvl - prev_row["ilvl"].values[0]
      fraction = (ilv-prev_row["ilvl"].values[0])/ilvl_diff

      pow_diff = row.power - prev_row["power"].values[0]

      output_numbers.append((ilv,round(float(prev_row["power"].values[0] + pow_diff * fraction),4)))
  print(output_numbers)
  print("Difficulty for Bee Curve")
  print(",".join([str(x[1]) for x in output_numbers]))
      


  
  plt.tight_layout()
  plt.savefig(f"{file_prefix}plot.png")
  # plt.show()
  
  plt.close()
  fig, axes = plt.subplots(1, 1, figsize=(16, 4))

  # Graph 1: x vs df["power"]
  axes.plot(median_df["ilvl"], group_df.quantile(0.25)["gems"], color="black", label="25th")
  axes.plot(median_df["ilvl"], median_df["gems"], color="blue", label="Mean")
  axes.plot(median_df["ilvl"], group_df.quantile(0.75)["gems"], color="black", label="75th")
  axes.set_xlabel("Item Level")
  axes.set_ylabel("Gems")
  axes.set_title("Item Level vs Gems")
  axes.legend()
  axes.grid(True)

  plt.tight_layout()
  plt.savefig(f"{file_prefix}gem_plot.png")

  plt.close()
  
  # Create two separate graphs
  fig, axes = plt.subplots(1, 2, figsize=(16, 8))

  # Graph 1: x vs df["power"]
  axes[0].scatter(df["runs"], power_plot, color="blue", label="Data")
  axes[0].set_xlabel("Runs")
  axes[0].set_ylabel("Combat Power Heuristic")
  axes[0].set_title("Runs vs Combat Power")
  axes[0].legend()
  axes[0].grid(True)

  # Graph 2: x vs df["health"]
  axes[1].scatter(df["runs"], df["health"], color="green", label="Data")
  axes[1].set_xlabel("Runs")
  axes[1].set_ylabel("Health")
  axes[1].set_title("Runs vs Health")
  axes[1].legend()
  axes[1].grid(True)

  plt.tight_layout()
  plt.savefig(f"{file_prefix}runs_plot.png")

  plt.close()


def progression():
  # data = generate_run_data_new(record_all_runs=True, log_output=True)
  # print(data)

  base_character = Character(Role.DPS)

  print(base_character.getCharacterOffencePowerEstimate())

  print(base_character.getStat(Stats.PRIMARY))
  for item in base_character.items.values():
    print(item)

  for i in range(40):
    base_character.completeDungeon()

  print(base_character.simc_output())

  print(base_character.run_sim())


def testGems():
  character = Character(Role.DPS)

  character.createRandomItem(Slot.RING_1, 45, True)
  character.createRandomItem(Slot.RING_2, 45, True)
  character.createRandomItem(Slot.NECK, 45, True)
  character.createRandomItem(Slot.WRISTS, 45, True)

  character.gems = 27 * 4
  print(character.optimalGemPower())
  character.gems = 9 * 4
  print(character.optimalGemPower())
  character.gems = 3 * 4
  print(character.optimalGemPower())
  character.gems = 4
  print(character.optimalGemPower())


def run_sim_file(input_uuid):
    subprocess.call(f"simc.exe chart_show_relative_difference=1 iterations=10000 target_error=0.2 single_actor_batch=1 max_time=720 apl/generated/harness_sims/input/{input_uuid}.simc html=apl/generated/harness_sims/output/{input_uuid}.html json=apl/generated/harness_sims/output/{input_uuid}.json")

    # with open(f"apl/generated/harness_sims/output/{self.character_uuid}.json", "r") as f:
    #   output_data = json.load(f)
    #   return output_data["sim"]["players"][0]["collected_data"]["dps"]["mean"]


class DataPoint:
  ilvl: float  = 0
  run: int = 0
  sim_dps: float = 0
  combat_power_heuristic: float = 0

  def __init__(self, ilvl = 0, run = 0, sim_dps = 0, combat_power_heuristic = 0):
    self.ilvl = ilvl
    self.run = run
    self.sim_dps = sim_dps
    self.combat_power_heuristic = combat_power_heuristic

  def __repr__(self):
    return f"{self.run},{self.ilvl},{self.sim_dps},{self.combat_power_heuristic}"

def custom_json(obj):
    if isinstance(obj, DataPoint):
      return {"run": obj.run, "ilvl": obj.ilvl, "sim_dps": obj.sim_dps, "combat_power_heuristic": obj.combat_power_heuristic}

def progress_sim_basic(max_ilvl: float = 300):

  if max_ilvl > 300:
    max_ilvl = 300

  base_character = Character(Role.DPS)

  data: list[DataPoint] = []
  data.append(DataPoint(ilvl=base_character.averageItemLevel(), run=base_character.completed_runs, combat_power_heuristic=base_character.getCharacterOffencePowerEstimate(), sim_dps=base_character.run_sim()))
  
  while (base_character.averageItemLevel() < max_ilvl):
    base_character.completeDungeon()
    data.append(DataPoint(ilvl=base_character.averageItemLevel(), run=base_character.completed_runs, combat_power_heuristic=base_character.getCharacterOffencePowerEstimate(), sim_dps=base_character.run_sim()))

  with open(f"apl/generated/harness_sims/output/{base_character.character_uuid}_prog.json", "w") as f:
    json.dump(data, f, default=custom_json)

def progress_sim(max_ilvl: float = 300):

  if max_ilvl > 300:
    max_ilvl = 300
    
  base_character = Character(Role.DPS)

  data: list[DataPoint] = []
  # data.append(DataPoint(ilvl=base_character.averageItemLevel(), run=base_character.completed_runs, combat_power_heuristic=base_character.getCharacterOffencePowerEstimate(), sim_dps=base_character.run_sim()))

  sim_actors = []


  sim_actors.append(base_character.simc_output(f"_{base_character.completed_runs}"))

  
  while (base_character.averageItemLevel() < max_ilvl):
    base_character.completeDungeon()
    sim_actors.append(base_character.simc_output(f"_{base_character.completed_runs}"))
    # data.append(DataPoint(ilvl=base_character.averageItemLevel(), run=base_character.completed_runs, combat_power_heuristic=base_character.getCharacterOffencePowerEstimate(), sim_dps=base_character.run_sim()))

  with open(f"apl/generated/harness_sims/input/{base_character.character_uuid}.simc", "w") as f:
    f.write("\n".join(sim_actors))

  run_sim_file(base_character.character_uuid)

  # with open(f"apl/generated/harness_sims/output/{base_character.character_uuid}_prog.json", "w") as f:
  #   json.dump(data, f, default=custom_json)
    

    
# apl/generated/rime_row1_2pt.simc
# apl/generated/rime_row2_4pt.simc
# apl/generated/rime_row3_5pt.simc
# apl/generated/rime_row3_7pt.simc
# apl/generated/rime_row4_8pt.simc
# apl/generated/rime_row5_10pt.simc
# apl/generated/rime_row6_12pt.simc
# apl/generated/rime_row6_14pt.simc

talent_strings = {
  2: "talents=avalanche:1\n",
  4: "talents=avalanche:1/burstbolter:1/cold_shower:1\n",
  5: "talents=avalanche:1/burstbolter:1/cascading_blitz:1\n",
  7: "talents=avalanche:1/burstbolter:1/cascading_blitz:1/chilling_finesse:1/cold_shower:1\n",
  8: "talents=avalanche:1/burstbolter:1/bursting_swallows:1/cascading_blitz:1/cold_shower:1/greater_glacial_blast:1\n",
  10: "talents=burstbolter:1/bursting_swallows:1/cascading_blitz:1/glacial_assault:1/greater_glacial_blast:1/icy_talons:1\n",
  12: "talents=burstbolter:1/bursting_swallows:1/cascading_blitz:1/glacial_assault:1/greater_glacial_blast:1/icy_talons:1/navirs_keeper:1/wisdom_of_the_north:1\n",
  14: "talents=burstbolter:1/bursting_swallows:1/cascading_blitz:1/glacial_assault:1/greater_glacial_blast:1/icy_flow:1/icy_talons:1/navirs_keeper:1/wisdom_of_the_north:1\n"
}


def testCharStuff():

  c = Character(Role.DPS)

  for i in range(23):
    c.completeDungeon()

  print(c.simc_output())

  c.items[Slot.RING_1].item_secondarys[0] = Stats.HASTE
  c.items[Slot.RING_2].item_secondarys[0] = Stats.CRIT
  c.items[Slot.FEET].item_secondarys[0] = Stats.EXPERTISE
  
  print(c.simc_output())

def check_again():
  c = Character(Role.DPS)

  print(f"0: {c.getCharacterOffencePowerEstimate()}")
  c.completeDungeon()
  print(f"1: {c.getCharacterOffencePowerEstimate()}")
  c.completeDungeon()
  print(f"2: {c.getCharacterOffencePowerEstimate()}")
  c.completeDungeon()
  print(f"3: {c.getCharacterOffencePowerEstimate()}")


def secondary_stat_by_variant():

  c = Character(Role.DPS)

  stats_per_slot = {}

  for slot in Slot:
    if slot == Slot.NONE:
      continue
    c.createRandomItem(slot, 315, True)
    
    if slot == Slot.RELIC_1 or slot == Slot.RELIC_2 or slot == Slot.WEAPON:
      continue

    secondaries = [0,0]
    c.items[slot].set_variant("55_45")
    secondaries[0] = c.items[slot].getItemTotalSecondary()
    c.items[slot].set_variant("70_30")
    secondaries[1] = c.items[slot].getItemTotalSecondary()
    stats_per_slot[slot] = tuple(secondaries)

  print("Slot            | 55_45 | 70_30")
  for slot, stats in stats_per_slot.items():
    print(f"{slot:15} |   {stats[0]:3.0f} |   {stats[1]:3.0f}")


  # for i in range(1, 50):
  #   print(i, round(0.3*i)+round(0.7*i)-i)

def fully_geared_guy():
  c = Character(Role.TANK)
  
  for slot in Slot:
    if slot == Slot.NONE:
      continue
    c.createRandomItem(slot, 315, True)
    c.items[slot].tempers = 8

  print(c.simc_output())

if __name__ == "__main__":
  # check_again()
  # testGems()

  # debug_runs([45,30,15])
  # os.mkdir("growth_sims")
  # create_runs_plot([15,15], file_prefix="growth_sims/easy_", iterations=1000)
  # create_runs_plot([30,15], file_prefix="growth_sims/medium_", iterations=1000)
  # create_runs_plot(Difficulty.EASY, file_prefix="growth_sims/easy_", iterations=1000)
  # create_runs_plot(Difficulty.MEDIUM, file_prefix="growth_sims/medium_", iterations=1000)
  # create_runs_plot(Difficulty.HARD, file_prefix="growth_sims/hard_", iterations=1000)
  # generate_run_data([45,30,15], log_output=True)



  # progression()
  # progress_sim(100)

  # secondary_stat_by_variant()


  # testCharStuff()

  fully_geared_guy()