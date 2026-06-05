


rime = {
    "chilling_finesse": 2,
    "harrowing_ice": 2,
    "glacial_assault": 2,

    "burstbolter": 1,
    "icy_flow": 1,
    "navirs_keeper": 1,

    "cascading_blitz": 2,
    "avalanche": 2,
    "coalescing_frost": 2,

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
    forbidden_pairs=None ):
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
            if profilesets:
                header = f"profileset.\"{prefix}{indicator}{index}\"="
            else:
                header = f"copy=\"{prefix}{indicator}{index}\",\"Base\"\n"
            if len(combo) == 0:
                continue
            line = "talents="+":1/".join(combo)+":1\n"
            #print(header+line)
            f.write(header)
            f.write(line)
        

# required_talents=None, excluded_talents=None, forbidden_pairs=None
combos_to_file("Rime", rime, rime_indicators, 11, "talent_output.simc", 
               required_talents=[
                #    "icy_talons",
                #    "greater_glacial_blast",
                #    "glacial_assault"
               ],
               excluded_talents=[
                #  "frostweavers_wrath",
                #  "cascading_blitz",
                #  "avalanche"
               ],
               forbidden_pairs=[
       ("icy_talons", "frostweavers_wrath")
    ],)

# for combo in combos:
#     print(combo)