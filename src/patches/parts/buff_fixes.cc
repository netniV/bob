#include "config.h"
#include "prime_types.h"

#include <spud/detour.h>

#include <il2cpp/il2cpp_helper.h>
#include <prime/IBuffComparer.h>
#include <prime/IBuffData.h>

static bool IsImportantFactionBuffThatNeedsFix(auto original, ClientModifierType modifierType)
{
  return
      // Cost
      modifierType == ClientModifierType::ModComponentCost
      // Cargo/Protected
      || modifierType == ClientModifierType::ModCargoCapacity
      || modifierType == ClientModifierType::ModCargoProtection
      // Mining stuff
      || modifierType == ClientModifierType::FleetMiningBonus || modifierType == ClientModifierType::ModMiningRate
      || modifierType == ClientModifierType::ModMiningReward
      || modifierType == ClientModifierType::ModMiningRateParsteel
      || modifierType == ClientModifierType::ModMiningRateTritanium
      || modifierType == ClientModifierType::ModMiningRateDilithium
      || modifierType == ClientModifierType::ModMiningRateOre
      || modifierType == ClientModifierType::ModMiningRateHydrocarbon
      || modifierType == ClientModifierType::ModMiningRewardParsteel
      || modifierType == ClientModifierType::ModMiningRewardTritanium
      || modifierType == ClientModifierType::ModMiningRewardDilithium
      || modifierType == ClientModifierType::ModMiningRewardOre
      || modifierType == ClientModifierType::ModMiningRewardHydrocarbon
      // Repair cost and time
      || modifierType == ClientModifierType::ModRepairCostsParsteel
      || modifierType == ClientModifierType::ModRepairCostsTritanium
      || modifierType == ClientModifierType::ModRepairCostsDilithium
      || modifierType == ClientModifierType::ModRepairCostsAll || modifierType == ClientModifierType::ModRepairTime
      || modifierType == ClientModifierType::ModRepairCosts;
}

static bool BuffService_IsBuffConditionMet_Hook(auto original, int64_t _unused, BuffCondition condition,
                                                IBuffComparer *comparer, IBuffData *buffToCompare,
                                                bool excludeFactionBuffs)
{
  switch (condition) {
      // Enable to show same power in station as out of station
    case BuffCondition::CondSelfAtStation: {
      if (Config::Get().use_out_of_dock_power) {
        return false;
      }
      // Call the original function without further intervention
      return original(_unused, condition, comparer, buffToCompare, excludeFactionBuffs);
    }
    // case BuffCondition::CondSelfIsMantis: {
    // 	return false;
    // }
    /*case BuffCondition::CondSelfIsSynciate30: {
      return true;
    }*/
    default: {
      return original(_unused, condition, comparer, buffToCompare, excludeFactionBuffs);
    }
  }
}

struct BuffModifierValues {
  float Add;
  float Mul;
};

static double RoundDouble(double v)
{
  if (v > 0.0)
    return floor(v + 0.5);
  else
    return ceil(v - 0.5);
}

float BuffService_ApplyBuffModifiersToCostVal_1_Hook(auto original, int64_t _unused, double baseCost,
                                                     BuffModifierValues buffMods)
{
  /*
    The original code does something along the lines of
    var val = baseCost / Math.Round(((1 + buffMods.Add) * (1 + buffMods.Mul)), 2, MidpointRounding);
    return MathUtils.RoundDouble(val);

    This is wrong for one particular reason, there are cost reduction researches that do half percentages. Like Pure
    Crystal which can be 11.5% Since the buff values are expressed in a scale of 0 to 1, with 1 being 100% this means
    that Pure Crystal with 11.5% is actually 0.115, the above code rounds away the third decimal resulting in (due to
    float error), either too low or too high cost displayed.
  */

  return original(_unused, baseCost, buffMods);
  /*const auto mul = (double)buffMods.Mul;
  const auto add = (double)buffMods.Add;
  return (float)(RoundDouble(baseCost / ((1.0 + add) * (1.0 + mul))));*/
}

bool FleetService_ResolveOfficerAbilityBuffs_Hook(auto original, int64_t _fleet)
{
  return original(_fleet);
}

void InstallBuffFixHooks()
{
  auto screen_manager_helper =
      il2cpp_get_class_helper("Digit.Client.PrimeLib.Runtime", "Digit.PrimeServer.Services", "BuffService");
  auto ptr = screen_manager_helper.GetMethodXor("IsBuffConditionMet");
  if (!ptr) {
    return;
  }
  SPUD_STATIC_DETOUR(ptr, BuffService_IsBuffConditionMet_Hook);

  screen_manager_helper =
      il2cpp_get_class_helper("Digit.Client.PrimeLib.Runtime", "Digit.PrimeServer.Services", "FleetService");
  ptr = screen_manager_helper.GetMethodXor("ResolveOfficerAbilityBuffs");
  if (!ptr) {
    return;
  }
  SPUD_STATIC_DETOUR(ptr, FleetService_ResolveOfficerAbilityBuffs_Hook);

  screen_manager_helper =
      il2cpp_get_class_helper("Digit.Client.PrimeLib.Runtime", "Digit.PrimeServer.Services", "BuffService");
  ptr = screen_manager_helper.GetMethodSpecial(xorstr_("ApplyBuffModifiersToCostVal"), [](auto count, auto params) {
    if (count != 2) {
      return false;
    }
    auto p2 = params[1].parameter_type->type;
    if (p2 == IL2CPP_TYPE_VALUETYPE) {
      return true;
    }
    return false;
  });
  if (!ptr) {
    return;
  }
  SPUD_STATIC_DETOUR(ptr, BuffService_ApplyBuffModifiersToCostVal_1_Hook);
}