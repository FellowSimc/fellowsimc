
# gear_crit_rating=98
# gear_haste_rating=202
# gear_mastery_rating=165
# gear_versatility_rating=107

budget = 572

steps = 12
max_steps = int(steps * 0.8)
min_steps = 1

with open("stat_outputs.txt", "w") as f:

    for haste in range(1, max_steps - min_steps * 3):
        for crit in range(1, max_steps - min_steps * 2 ):
            if (haste+crit>steps-min_steps*2):
                break
            for exp in range(1, max_steps - min_steps ):
                if (haste+crit+exp>steps-min_steps):
                    break
                spirit = steps-haste-crit-exp
                
                if (spirit > max_steps):
                    break

                f.write(f"copy=\"{haste}H_{crit}C_{exp}E_{spirit}S Xav\",\"Xavian\"\n")
                f.write(f"gear_haste_rating={round(haste/steps*budget)}\n")
                f.write(f"gear_crit_rating={round(crit/steps*budget)}\n")
                f.write(f"gear_versatility_rating={round(exp/steps*budget)}\n")
                f.write(f"gear_mastery_rating={round(spirit/steps*budget)}\n\n")


