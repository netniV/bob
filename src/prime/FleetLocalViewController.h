#pragma once

#include <il2cpp/il2cpp_helper.h>

#include "ActionData.h"
#include "FleetPlayerData.h"
#include "ShipBarItemLocalViewController.h"

struct FleetLocalViewController {
public:
  __declspec(property(get = __get_fleet)) FleetPlayerData* fleet;
  __declspec(property(get = __get__shipBarItemLocalViewController))
      ShipBarItemLocalViewController* shipBarItemLocalViewController;

  void CancelWarpClicked()
  {
    static auto CancelWarpClicked = get_class_helper().GetMethod<void(FleetLocalViewController*)>("CancelWarpClicked");
    CancelWarpClicked(this);
  }

  bool RequestAction(IActionData target, ActionType type, int index, ActionBehaviour behaviourMask,
                     void* callback = nullptr)
  {
    static auto RequestAction =
        get_class_helper()
            .GetMethodSpecial<bool(FleetLocalViewController*, IActionData, ActionType, int, ActionBehaviour, void*)>(
                "RequestAction", [](int param_count, const ParameterInfo* /*param*/) {
                  if (param_count == 5) {
                    return true;
                  }
                  return false;
                });
    return RequestAction(this, target, type, index, behaviourMask, callback);
  }

private:
  friend class ObjectFinder<FleetLocalViewController>;
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Ships", "FleetLocalViewController");
    return class_helper;
  }

public:
  FleetPlayerData* __get_fleet()
  {
    static auto field = get_class_helper().GetProperty("fleet");
    return field.GetRaw<FleetPlayerData>(this);
  }

  ShipBarItemLocalViewController* __get__shipBarItemLocalViewController()
  {
    static auto field = get_class_helper().GetProperty("_shipBarItemLocalViewController");
    return field.GetRaw<ShipBarItemLocalViewController>(this);
  }
};
