

budget = 4529

steps = 10
max_steps = int(steps * 0.8)

with open("stat_outputs.txt", "w") as f:

    for haste in range(max_steps):
        for crit in range(max_steps):
            if (haste+crit>steps):
                break
            for exp in range(max_steps):
                if (haste+crit+exp>steps):
                    break
                spirit = steps-haste-crit-exp
                
                if (spirit > max_steps):
                    break

                f.write(f"copy=\"{haste}H_{crit}C_{exp}E_{spirit}S Spender_Drenched\",\"Mara_Spender_Drenched\"\n")
                f.write(f"gear_haste_rating={round(haste/steps*budget)}\n")
                f.write(f"gear_crit_rating={round(crit/steps*budget)}\n")
                f.write(f"gear_versatility_rating={round(exp/steps*budget)}\n")
                f.write(f"gear_mastery_rating={round(spirit/steps*budget)}\n\n")


