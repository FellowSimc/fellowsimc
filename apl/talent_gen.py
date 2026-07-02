import json
from collections import defaultdict


rime = {
    "avalanche": 2,
    "coalescing_frost": 2,
    "glacial_assault": 2,

    "burstbolter": 1,
    "chilling_finesse": 1,
    "navirs_keeper": 1,

    "cascading_blitz": 2,
    "harrowing_ice": 2,
    "icy_flow": 2,

    "bursting_swallows": 1,
    "greater_glacial_blast": 1,
    "cold_shower": 1,

    "icy_talons": 3,
    "frostweavers_wrath": 3,
    "soulfrost_torrent": 3,

    "biting_cold": 1,
    "supreme_torrent": 1,
    "wisdom_of_the_north": 1,
}

rime_indicators = {
    "cascading_blitz": "CB",
    "icy_talons": "IT",
    "frostweavers_wrath": "FWW",
    "soulfrost_torrent": "SFT",
}


ardeos = {
    "slow_burn": 2,
    "frog_squad": 2,
    "great_balls_of_fire": 2,

    "backdraft": 1,
    "flare_up": 1,
    "crash_and_burn": 1,

    "agonizing_blaze": 2,
    "firestarter": 2,
    "undying_flame": 2,

    "cascading_inferno": 1,
    "crackling_inferno": 1,
    "apocalyptic_surge": 1,

    "rolling_flames": 3,
    "pyrophibian_frenzy": 3,
    "reign_of_fire": 3,

    "intensifying_inferno": 1,
    "burning_initiative": 1,
    "spontaneous_combustion": 1,
}

ardeos_indicators = {
    "rolling_flames": "Rolling",
    "pyrophibian_frenzy": "Pyrophibian",
    "reign_of_fire": "ROF",
    "flare_up": "FU",
    "undying_flame": "UF",
}


mara = {
    "red_ledger": 2,
    "corrosive_spill": 2,
    "assassins_guile": 2,

    "bloodrush": 1,
    "venomous_delight": 1,
    "efficient_killer": 1,

    "gushing_blood": 2,
    "feed_the_queen": 2,
    "deadly_scheme": 2,

    "macabre_stratagem": 1,
    "maidens_doom": 1,
    "seething_burst": 1,

    "hemotoxin": 3,
    "sinners_pride": 3,
    "malevolence": 3,

    "arachnid_onslaught": 1,
    "caustic_wounds": 1,
    "puncture": 1,
}

mara_indicators = {
    "hemotoxin": "HT",
    "sinners_pride": "Sinners",
    "malevolence": "MV",
    "deadly_scheme": "DS",
    "gushing_blood": "GB",
    "feed_the_queen": "FTQ",
    "assassins_guile": "AG",
    "venomous_delight": "VD",
    "efficient_killer": "EK",
    "macabre_stratagem": "MS",
    "maidens_doom": "MD",
}

elarion = {
    "focused_expanse": 2,
    "piercing_seekers": 2,
    "resurgent_winds": 2,

    "skylit_grace":1,
    "fusillade": 1,
    "skyward_munitions": 1,

    "repeating_stars": 2,
    "lunar_fury": 2,
    "lethal_shots": 2,

    "rising_moon": 1,
    "lunarlight_affinity": 1,
    "strikers_aim": 1,

    "fervent_supremacy": 3,
    "impending_heartseeker": 3,
    "final_crescendo": 3,

    "last_lights": 1,
    "swift_reload": 1,
    "deadly_focus": 1,
}

elarion_indicators = {
    "fervent_supremacy": "FS",
    "impending_heartseeker": "IH",
    "resurgent_winds": "RW",
    "skyward_munitions": "SM",
}

gunde = {
    "deaths_arc": 2,
    "ravens_precision": 2,
    "grim_harvest": 2,

    "sundered_flesh": 1,
    "crimson_strikes": 1,
    "bloodbath": 1,

    "darkening_hearts": 2,
    "murder_of_crows": 2,
    "massacre": 2,

    "slayers_grin": 1,
    "frenzied_reign": 1,
    "deep_rend": 1,

    "bloodcraze": 3,
    "oathshatter": 3,
    "carnage": 3,

    "superior_serration": 1,
    "ancestral_instinct": 1,
    "harvesters_toll":1,
}

gunde_indicators = {
    "grim_harvest": "GH",
    "bloodcraze": "BC",
    "oathshatter": "OATH",
    "carnage": "CRN",
}


tariq = {
    "left_hand_path": 2,
    "ride_the_lightning": 2,
    "square_hammer": 2,

    "high_road": 1,
    "blood_and_thunder": 1,
    "bloodline": 1,

    "the_motherload": 2,
    "mouth_for_war": 2,
    "thunderstruck": 2,

    "pneuma": 1,
    "sledgehammer": 1,
    "far_beyond_driven": 1,

    "kill_em_all": 3,
    "ace_of_spades": 3,
    "schism": 3,

    "them_bones": 1,
    "crack_the_sky": 1,
    "killing_in_the_name":1,
}

tariq_indicators = {
    "kill_em_all": "KEA",
    "ace_of_spades": "AoS",
    "schism": "Schism",
}


gems = [
    "ruby", "amethyst", "topaz", "emerald", "sapphire", "diamond"
]

gem_power_values = [100, 300, 600, 800, 1500]

traits = [
    "emerald_judgement",
    # "ruby_storm",
    "amethyst_splinters",
    "brave_machinations",
    "diamond_strike",
    "heroic_brand",
    "martial_initiative",
    "sapphire_aurastone",
    "visions_of_grandeur",


    "kindling",
    "seized_opportunity",
    "hidden_power",
    "hunters_focus",
    "inspired_allegiance",
    "navigators_intuition",
    "patient_soul",
    "vengeful_soul",
    "willful_momentum",

    # "iron_spikes",
    # "king_of_the_hill",
    # "divine_mediation",
    # "first_man_standing",
    # "grounded_spirit",
    # "heart_of_stone",
    # "latent_resurgence",
    # "stalwart_readiness",
    # "treasure_hunters_delight",
]


trait_ranks = [1,4]


sets = [
    # "seal_of_the_heskyr",
    "tuzari_grace",
    "torment_of_baelaurum",
    "sin_warding",
    "eldrin_fury",
    "draconic_might",
    "deaths_grasp",
    "dark_prophecy",
    "drakheims_absolution"
]


finesses = [
    "finesse_a",
    "finesse_b",
    "finesse_c",
    "finesse_d",
    "finesse_e",
    "finesse_f",
    "finesse_g",
    "finesse_h",
    "finesse_i",
    "finesse_j",
    "finesse_k",
    "finesse_l",
    "finesse_n",
]

finesse_ranks = [1,4]

stats = [
    "primary",
    "haste",
    "crit",
    "spirit",
    "expertise"
]


def make_sim_entry(prefix, profile_name, changes, profilesets = False):

    entry = ""

    if not profilesets:
        entry = f"copy=\"{profile_name}\",\"Base {prefix}\"\n"

    for change in changes:
        if profilesets:
            entry += f"profileset.\"{profile_name}\"="
        entry += change + "\n"
    
    return entry






def generate_item_checks(prefix, fodder_slot, file_path, profilesets = False):
    with open(file_path, "w") as f:
        header = ""

        for gem_color in gems:
            for gem_power in gem_power_values:
                profile_name = f"{prefix}_{gem_color}_{gem_power}"
                line = make_sim_entry(prefix, profile_name, [f"gems.{gem_color}_power={gem_power}"], profilesets=profilesets)
                f.write(line)

        for trait in traits:
            for rank in trait_ranks:
                profile_name = f"{prefix}_{trait}_{rank}"
                line = make_sim_entry(prefix, profile_name, [f"weapon_trait.{trait}={rank}"], profilesets=profilesets)
                f.write(line)
                
        for set_bonus in sets:
            profile_name = f"{prefix}_{set_bonus}"
            line = make_sim_entry(prefix, profile_name, [f"sets.{set_bonus}=1"], profilesets=profilesets)
            f.write(line)

        for finesse in finesses:
            for rank in finesse_ranks:
                profile_name = f"{prefix}_{finesse}{rank}"
                extra = "affixes=" + "/".join([finesse]*rank)
                line = make_sim_entry(prefix, profile_name, [f"{fodder_slot},{extra}"], profilesets=profilesets)
                f.write(line)

        for stat in stats:
            profile_name = f"{prefix}_stat_{stat}"
            extra = f"affixes={stat}"
            line = make_sim_entry(prefix, profile_name, [f"{fodder_slot},{extra}"], profilesets=profilesets)
            f.write(line)

    
  #trinket2=relic2,rarity=regal,variant=evens,ilevel=315,main_secondary=spirit/crit,fixed_secondary=haste/haste


# def talent_combinations(
#     talents,
#     target_points,
#     required_talents=None,
#     excluded_talents=None,
# ):
#     """
#     talents: dict[str, int]
#         Mapping of talent_name -> point_cost

#     target_points: int
#         Desired total point cost

#     required_talents: iterable[str] | None
#         Talents that must appear in every result

#     excluded_talents: iterable[str] | None
#         Talents that must not appear in any result

#     Returns:
#         list[list[str]]
#     """
#     required_talents = set(required_talents or [])
#     excluded_talents = set(excluded_talents or [])

#     # Impossible request
#     if required_talents & excluded_talents:
#         return []

#     # Verify all required talents exist
#     if not required_talents.issubset(talents):
#         raise ValueError(
#             f"Unknown required talents: {required_talents - set(talents)}"
#         )

#     # Remove excluded talents from consideration
#     available = {
#         name: cost
#         for name, cost in talents.items()
#         if name not in excluded_talents
#     }

#     # Pre-select required talents
#     required_cost = sum(available[name] for name in required_talents)

#     if required_cost > target_points:
#         return []

#     items = [
#         (name, cost)
#         for name, cost in available.items()
#         if name not in required_talents
#     ]

#     results = []

#     def backtrack(index, current_names, current_cost):
#         if current_cost == target_points:
#             results.append(sorted(current_names))
#             return

#         if current_cost > target_points or index >= len(items):
#             return

#         name, cost = items[index]

#         # Include
#         current_names.append(name)
#         backtrack(index + 1, current_names, current_cost + cost)
#         current_names.pop()

#         # Exclude
#         backtrack(index + 1, current_names, current_cost)

#     backtrack(
#         0,
#         list(required_talents),
#         required_cost,
#     )

#     return results

def talent_combinations(
    talents,
    target_points,
    required_talents=None,
    excluded_talents=None,
    forbidden_pairs=None,
):
    required_talents = set(required_talents or [])
    excluded_talents = set(excluded_talents or [])

    # Normalize forbidden pairs
    forbidden_pairs = {
        frozenset(pair)
        for pair in (forbidden_pairs or [])
    }

    if required_talents & excluded_talents:
        return []

    if not required_talents.issubset(talents):
        raise ValueError(
            f"Unknown required talents: {required_talents - set(talents)}"
        )

    available = {
        name: cost
        for name, cost in talents.items()
        if name not in excluded_talents
    }

    required_cost = sum(available[name] for name in required_talents)

    if required_cost > target_points:
        return []

    # Check if required talents already violate a forbidden pair
    for pair in forbidden_pairs:
        if pair.issubset(required_talents):
            return []

    items = [
        (name, cost)
        for name, cost in available.items()
        if name not in required_talents
    ]

    results = []

    def violates_pair(selected, new_talent):
        for talent in selected:
            if frozenset((talent, new_talent)) in forbidden_pairs:
                return True
        return False

    def backtrack(index, current_names, current_cost):
        if current_cost == target_points:
            results.append(sorted(current_names))
            return

        if current_cost > target_points or index >= len(items):
            return

        name, cost = items[index]

        # Include only if it does not create a forbidden pair
        if not violates_pair(current_names, name):
            current_names.append(name)
            backtrack(
                index + 1,
                current_names,
                current_cost + cost,
            )
            current_names.pop()

        # Exclude
        backtrack(
            index + 1,
            current_names,
            current_cost,
        )

    backtrack(
        0,
        list(required_talents),
        required_cost,
    )

    return results


def combos_to_file(prefix, talents, indicator_dict, target_points, file_path, profilesets=True,
    required_talents=None,
    excluded_talents=None,
    forbidden_pairs=None, name_filters=None, extra_options=None):
    combos = talent_combinations(talents, target_points, required_talents=required_talents, excluded_talents=excluded_talents, forbidden_pairs=forbidden_pairs )

    with open(file_path, "w") as f:
        index = 0

        for combo in combos:
            index += 1

            indicators = []

            for indicator, short in indicator_dict.items():
                if indicator in combo:
                    indicators.append(short)

            indicator = "_" + "_".join(indicators)

            if len(indicators) > 0:
                indicator += "_"

            header = ""
            
            profile_name = f"{prefix}{indicator}{index}"

            if name_filters:
                if not profile_name in name_filters:
                    continue

            if profilesets:
                header = f"profileset.\"{profile_name}\"="
            else:
                header = f"copy=\"{profile_name}\",\"Base {prefix}\"\n"
            if len(combo) == 0:
                continue
            line = "talents="+":1/".join(combo)+":1\n"

            if extra_options:
                for option in extra_options:
                    if profilesets:
                        line += f"profileset.\"{profile_name}\"+="
                    line += option + "\n"

            #print(header+line)
            f.write(header)
            f.write(line)
        

# required_talents=None, excluded_talents=None, forbidden_pairs=None

def all_points_to_File(prefix, talents, file_path):
    with open(file_path, "w") as f:
            for talent in talents.keys():
                profile_name = f"{prefix} {talent}"
                header = f"copy=\"{profile_name}\",\"Base {prefix}\"\n"
                line = f"talents+=/{talent}:1\n"
                f.write(header)
                f.write(line)
                



def top_sim_results(path_to_json, total_runs=100):
    with open(path_to_json,"r") as file_handle:
        js = json.load(file_handle)

        results = sorted(js["sim"]["profilesets"]["results"], key=lambda x: x["mean"], reverse = True)

        top_x = ([x['name'] for x in results[:total_runs]])

        return top_x

def dps_sim_results(paths_to_json, total_runs=-1):
    runs = defaultdict(list)
    for json_path in paths_to_json:
        with open(json_path, "r") as file_handle:
            js = json.load(file_handle)
            
            results = sorted(js["sim"]["profilesets"]["results"], key=lambda x: x["mean"], reverse = True)

            for elem in results[:total_runs]:
                runs[elem['name']].append(elem['mean'])

            # top_x = ([(x['name'], x['mean']) for x in results[:total_runs]])

            # runs[].append(top_x)
    
    return {x: y for x,y in runs.items() if len(y)>1}

    return runs

def sorted_by_overall(build_dicts):
    new_dict = {x: sum(y)/len(y) for x,y in build_dicts.items()}
    return sorted(new_dict.items(), key=lambda x: x[1], reverse=True)
    



if __name__ == "__main__":
    # top_x = top_sim_results("output/rime_et27.json", 500)
    # # print(dps_sim_results(["output/rime_et27.json", "output/rime_et28.json"]))
    # sim_results = dps_sim_results(["output/rime_et27.json", "output/rime_et29.json"])
    # overalls = sorted_by_overall(sim_results)
    # print(overalls)

    combos_to_file("Rime_US", rime, rime_indicators, 14, "generated/rime_talents_talons_us.simc", 
                required_talents=[
                    "cascading_blitz",
                    "burstbolter",
                    "greater_glacial_blast",
                    "icy_talons",
                    "bursting_swallows"
                    # "greater_glacial_blast",
                    # "frostweavers_wrath"
                    #    "icy_talons",
                    #    "greater_glacial_blast",
                    #    "glacial_assault"
                ],
                extra_options=[
                    "legendary.skandis_decree=0",
                    "legendary.undulating_spirit=1",
                    "legendary.frostwyrms_spite=0"
                ],
                excluded_talents=[
                    # "cascading_blitz",
                    #  "frostweavers_wrath",
                    #  "cascading_blitz",
                    #  "avalanche"
                ],
                forbidden_pairs=[
                       ("icy_talons", "frostweavers_wrath")
                ],
                # profilesets=False
                # name_filters=top_x
    )
    combos_to_file("Rime_SK", rime, rime_indicators, 14, "generated/rime_talents_talons_sk.simc", 
                required_talents=[
                    "cascading_blitz",
                    "burstbolter",
                    "greater_glacial_blast",
                    "icy_talons",
                    "bursting_swallows"
                    # "greater_glacial_blast",
                    # "frostweavers_wrath"
                    #    "icy_talons",
                    #    "greater_glacial_blast",
                    #    "glacial_assault"
                ],
                extra_options=[
                    "legendary.skandis_decree=1",
                    "legendary.undulating_spirit=0",
                    "legendary.frostwyrms_spite=0"
                ],
                excluded_talents=[
                    # "cascading_blitz",
                    #  "frostweavers_wrath",
                    #  "cascading_blitz",
                    #  "avalanche"
                ],
                forbidden_pairs=[
                       ("icy_talons", "frostweavers_wrath")
                ],
                # profilesets=False
                # name_filters=top_x
    )

    
    # top_x = top_sim_results("output/ardeos_et104.json", 500)
    # # print(dps_sim_results(["output/rime_et27.json", "output/rime_et28.json"]))
    # sim_results = dps_sim_results(["output/ardeos_et104.json", "output/ardeos_et105.json"])
    # overalls = sorted_by_overall(sim_results)
    # print(overalls)


    # combos_to_file("Ardeos", ardeos, ardeos_indicators, 3, "ardeos/talent_output_1.simc", 
    #             profilesets=False,
    #             required_talents=[
    #                     # "rolling_flames",
    #                     # "undying_flame",
    #                     # "slow_burn"
    #             ],
    #             excluded_talents=[
    #                 #  "frostweavers_wrath",
    #                 #  "cascading_blitz",
    #                 #  "avalanche"
    #             ],
    #             forbidden_pairs=[
    #             ],
    #             # name_filters=top_x
    # )

    # all_points_to_File("Ardeos", ardeos, "ardeos/single_talents.simc")


        # top_x = top_sim_results("output/ardeos_et104.json", 500)
    # # print(dps_sim_results(["output/rime_et27.json", "output/rime_et28.json"]))
    # sim_results = dps_sim_results(["output/ardeos_et104.json", "output/ardeos_et105.json"])
    # overalls = sorted_by_overall(sim_results)
    # print(overalls)


    # combos_to_file("Mara", mara, mara_indicators, 14, "generated/mara_talents14b2.simc", 
    #             profilesets=False,
    #             required_talents=[
    #                 "gushing_blood",
    #                 "bloodrush",
    #                 "hemotoxin",
    #                 "sinners_pride",
    #                 "red_ledger"
    #                     # "deadly_scheme",
    #                     # "venomous_delight",
    #                     # "malevolence",
    #                     # "assassins_guile"
    #             ],
    #             excluded_talents=[
    #                 "efficient_killer",
    #                 # "arachnid_onslaught",
    #                 # "caustic_wounds",
    #                 "feed_the_queen",
    #                 # "puncture",
    #                 "venomous_delight"
    #                 # "gushing_blood"
    #             ],
    #             forbidden_pairs=[
    #                 # ("hemotoxin", "malevolence"),
    #                 # ("gushing_blood", "malevolence"),
    #                 # ("arachnid_onslaught", "malevolence"),
    #             ],
    #             # name_filters=top_x
    # )

    # all_points_to_File("Mara", mara, "generated/mara_talents.simc")
    
    # generate_item_checks("Elarion", "trinket2=relic2,rarity=regal,variant=evens,ilevel=315,main_secondary=haste/spirit,fixed_secondary=haste/haste", "generated/all_gear_options.simc")


    
    combos_to_file("Elarion", elarion, elarion_indicators, 14, "generated/elarion_talents14b5.simc", 
                profilesets=True,
                required_talents=[
                    "fusillade",
                    "last_lights",
                    "lunarlight_affinity",
                    "impending_heartseeker",
                    "piercing_seekers",
                    "lunar_fury"
                ],
                excluded_talents=[
                    # "deadly_focus",
                    # "swift_reload"
                ],
                forbidden_pairs=[
                    # ("hemotoxin", "malevolence"),
                    # ("gushing_blood", "malevolence"),
                    # ("arachnid_onslaught", "malevolence"),
                ],
                # name_filters=top_x
    )
    
    
    combos_to_file("Elarion", elarion, elarion_indicators, 14, "generated/elarion_talents_hwa.simc", 
                profilesets=True,
                required_talents=[
                    "final_crescendo",
                    "resurgent_winds",
                    "strikers_aim",
                    "lethal_shots",
                    "skyward_munitions"
                ],
                excluded_talents=[
                    "lunar_fury",
                    "fervent_supremacy",
                    "focused_expanse"
                    # "deadly_focus",
                    # "swift_reload"
                ],
                forbidden_pairs=[
                    # ("hemotoxin", "malevolence"),
                    # ("gushing_blood", "malevolence"),
                    # ("arachnid_onslaught", "malevolence"),
                ],
                # name_filters=top_x
    )

    combos_to_file("Elarion", elarion, elarion_indicators, 14, "generated/elarion_talents_volley.simc", 
                profilesets=True,
                required_talents=[
                    "repeating_stars",
                    "resurgent_winds",
                    "lethal_shots",
                    "lunarlight_affinity"
                ],
                extra_options=[
                    "legendary.shimmer=0",
                    "legendary.starstrikers_ascent=0",
                    "legendary.astronomers_hail=1"
                ],
                excluded_talents=[
                    # "deadly_focus",
                    # "swift_reload"
                ],
                forbidden_pairs=[
                    # ("hemotoxin", "malevolence"),
                    # ("gushing_blood", "malevolence"),
                    # ("arachnid_onslaught", "malevolence"),
                ],
                # name_filters=top_x
    )

    # all_points_to_File("Elarion", elarion, "generated/elarion_talents.simc")


    # sim_results = dps_sim_results(["output/scenario_et_145.json", "output/scenario_et_146.json"])
    # # print(sim_results)
    # overalls = sorted_by_overall(sim_results)
    # print(overalls[:100])

    all_points_to_File("Gunde", gunde, "generated/gunde_talents.simc")

    generate_item_checks("Gunde", "trinket2=relic2,rarity=regal,variant=evens,ilevel=315,main_secondary=spirit/crit,fixed_secondary=haste/haste", "generated/all_gear_options_gunde.simc")

    combos_to_file("Gunde", gunde, gunde_indicators, 14, "generated/gunde_split_craze.simc", 
                profilesets=True,
                required_talents=[
                    "bloodcraze",
                    "oathshatter",
                    "darkening_hearts"
                ],
                excluded_talents=[
                    "sundered_flesh",
                    # "deadly_focus",
                    # "swift_reload"
                ],
                forbidden_pairs=[
                    # ("hemotoxin", "malevolence"),
                    # ("gushing_blood", "malevolence"),
                    # ("arachnid_onslaught", "malevolence"),
                ],
                # name_filters=top_x
    )

    all_points_to_File("Tariq", tariq, "generated/tariq_talents.simc")

    # generate_item_checks("Tariq", "finger2=ring2,rarity=regal,variant=evens,ilevel=315,main_secondary=haste/crit,fixed_secondary=expertise/crit", "generated/all_gear_options_tariq.simc")
    generate_item_checks("Tariq", "main_hand=weap,rarity=regal,variant=evens,ilevel=315,main_secondary=haste,fixed_secondary=haste/haste", "generated/all_gear_options_tariq.simc")
    
    
    # combos_to_file("Tariq", tariq, tariq_indicators, 14, "generated/tariq_talents14_lightning_aos.simc", 
    #             profilesets=True,
    #             required_talents=[
    #                 "ace_of_spades",
    #                 "crack_the_sky",
    #                 "thunderstruck",
    #                 "the_motherload",
    #                 "kill_em_all",
    #             ],
    #             excluded_talents=[
    #                 "killing_in_the_name",
    #                 # "deadly_focus",
    #                 # "swift_reload"
    #             ],
    #             forbidden_pairs=[
    #                 # ("hemotoxin", "malevolence"),
    #                 # ("gushing_blood", "malevolence"),
    #                 # ("arachnid_onslaught", "malevolence"),
    #             ],
    #             # name_filters=top_x
    # )

    combos_to_file("Tariq", tariq, tariq_indicators, 14, "generated/tariq_talents14_schism.simc", 
                profilesets=True,
                required_talents=[
                    "schism",
                    "sledgehammer",
                    "them_bones"
                ],
                excluded_talents=[
                    "kill_em_all",
                    "square_hammer",
                    "left_hand_path"
                    # "deadly_focus",
                    # "swift_reload"
                ],
                forbidden_pairs=[
                    # ("hemotoxin", "malevolence"),
                    # ("gushing_blood", "malevolence"),
                    # ("arachnid_onslaught", "malevolence"),
                ],
                # name_filters=top_x
    )

    # combos_to_file("Tariq", tariq, tariq_indicators, 14, "generated/tariq_talents14_spender.simc", 
    #             profilesets=True,
    #             required_talents=[
    #                 "schism",
    #                 "sledgehammer",
    #                 "them_bones"
    #             ],
    #             excluded_talents=[
    #                 "killing_in_the_name",
    #                 "the_motherload",
    #                 # "deadly_focus",
    #                 # "swift_reload"
    #             ],
    #             forbidden_pairs=[
    #                 # ("hemotoxin", "malevolence"),
    #                 # ("gushing_blood", "malevolence"),
    #                 # ("arachnid_onslaught", "malevolence"),
    #             ],
    #             # name_filters=top_x
    # )
# for combo in combos:
#     print(combo)