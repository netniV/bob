#include "config.h"
#include "prime_types.h"

#include <spud/detour.h>

#include "utils.h"

#include "prime/BundleDataWidget.h"
#include "prime/ClientModifierType.h"
#include "prime/Hub.h"
#include "prime/IList.h"
#include "prime/InventoryForPopup.h"
#include "prime/ShopSummaryDirector.h"

#include <il2cpp/il2cpp_helper.h>

#include <algorithm>
#include <chrono>
#include <iostream>

int64_t InventoryForPopup_set_MaxItemsToUse(auto original, InventoryForPopup* a1, int64_t a2)
{
  auto d = a1->IsDonationUse;
  if (a1->IsDonationUse && a2 == 50 && Config::Get().extend_donation_slider) {
    return 0;
  }
  return original(a1, a2);
}

void BundleDataWidget_OnActionButtonPressedCallback(auto original, BundleDataWidget* _this)
{
  if (_this->CurrentState & BundleDataWidget::ItemState::CooldownTimerOn) {
    _this->AuxViewButtonPressedHandler();
  } else {
    original(_this);
  }
}

void InstallMiscPatches()
{
  auto h   = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Inventories", "InventoryForPopup");
  auto ptr = h.GetMethodXor("set_MaxItemsToUse");
  if (!ptr) {
    return;
  }
  SPUD_STATIC_DETOUR(ptr, InventoryForPopup_set_MaxItemsToUse);

  auto bundle_data_widget = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Shop", "BundleDataWidget");
  ptr                     = bundle_data_widget.GetMethod("OnActionButtonPressedCallback");
  if (!ptr) {
    return;
  }
  SPUD_STATIC_DETOUR(ptr, BundleDataWidget_OnActionButtonPressedCallback);
}

struct Resolution {
  int m_Width;
  int m_Height;
  int m_RefreshRate;

  bool operator==(const Resolution& other) const
  {
    return this->m_Height == other.m_Height && this->m_Width == other.m_Width;
  }
};

struct ResolutionArray {
  Il2CppObject obj;
  void*        bounds;
  size_t       maxlength;
  Resolution   data[1];
};

ResolutionArray* GetResolutions_Hook(auto original)
{
  auto resolutions = original();

  // Modify
  auto screenWidth  = GetSystemMetrics(SM_CXSCREEN);
  auto screenHeight = GetSystemMetrics(SM_CYSCREEN);

  int targetRefreshRate = 0;
  for (int i = 0; i < resolutions->maxlength; ++i) {
    auto ores = resolutions->data[i];
    if (ores.m_Width == screenWidth && ores.m_Height == screenHeight) {
      targetRefreshRate = std::max(ores.m_RefreshRate, targetRefreshRate);
    }
  }

  std::vector<Resolution> res;
  for (int i = 0; i < resolutions->maxlength; ++i) {
    if (Config::Get().show_all_resolutions)
      resolutions->data[i].m_RefreshRate = targetRefreshRate;

    auto ores = resolutions->data[i];
    if (Config::Get().show_all_resolutions || (ores.m_RefreshRate == targetRefreshRate || targetRefreshRate == 0)) {
      res.push_back(ores);
    }
  }

  res.erase(unique(res.begin(), res.end()), res.end());

  int i = 0;
  for (const auto& resultRes : res) {
    resolutions->data[i] = resultRes;
    ++i;
  }
  resolutions->maxlength = res.size();

  return resolutions;
}

void InstallResolutionListFix()
{
  SPUD_STATIC_DETOUR(il2cpp_resolve_icall<ResolutionArray*()>("UnityEngine.Screen::get_resolutions()"),
                     GetResolutions_Hook);
}

IList* ExtractBuffsOfType_Hook(auto original, ClientModifierType modifier, IList* list)
{
  if (list) {
    for (int i = 0; i < list->Count; ++i) {
      auto item = list->Get(i);
      if (item == 0) {
        return nullptr;
      }
    }
  }
  return original(modifier, list);
}

bool ShouldShowRevealHook(auto original, void* _this, bool ignore)
{
  if (Config::Get().always_skip_reveal_sequence) {
    return false;
  }
  return original(_this, ignore);
}

struct ShopCategory {
public:
  __declspec(property(get = __get__flagValue)) int Value;

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("Digit.Client.PrimeLib.Runtime", "Digit.Prime.Shop", "ShopCategory");
    return class_helper;
  }

public:
  int __get__flagValue()
  {
    static auto field = get_class_helper().GetProperty("Value");
    return *field.GetUnboxedSelf<int>(this);
  }
};

struct CurrencyType {
public:
  __declspec(property(get = __get__flagValue)) int Value;
  //

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("Digit.Client.PrimeLib.Runtime", "Digit.PrimePlatform.Content", "CurrencyType");
    return class_helper;
  }

public:
  int __get__flagValue()
  {
    static auto field = get_class_helper().GetProperty("Value");
    return *field.GetUnboxedSelf<int>(this);
  }
};

struct BundleGroupConfig {
public:
  __declspec(property(get = __get__category)) int _category;
  __declspec(property(get = __get__currency)) int _currency;

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Shop", "BundleGroupConfig");
    return class_helper;
  }

public:
  int __get__category()
  {
    static auto field = get_class_helper().GetField("_category");
    return *(int*)((ptrdiff_t)this + field.offset());
  }

  int __get__currency()
  {
    static auto field = get_class_helper().GetField("_currency");
    return *(int*)((ptrdiff_t)this + field.offset());
  }
};

class ShopSectionContext
{
public:
  __declspec(property(get = __get__bundleConfig)) BundleGroupConfig* _bundleConfig;

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Shop", "ShopSectionContext");
    return class_helper;
  }

public:
  BundleGroupConfig* __get__bundleConfig()
  {
    static auto field = get_class_helper().GetProperty("BundleGroup");
    return field.GetRaw<BundleGroupConfig>(this);
  }
};

SectionID ShopSummaryDirectorGoBackBehavior(auto original, ShopSummaryDirector* _this, void* status,
                                            SectionNavHistory* history)
{
  if (Config::Get().stay_in_bundle_after_summary == false) {
    return original(_this, status, history);
  }

  if (strcmp(((Il2CppObject*)(_this))->klass->name, "ShopSummaryDirector") == 0) {
    auto section_data = (ShopSectionContext*)Hub::get_SectionManager()->_sectionStorage->GetState(SectionID::Shop_List);
    section_data      = section_data;

    auto suppress_go_back = false;
    if (section_data) {
      auto bundle_config = section_data->_bundleConfig;
      if (bundle_config) {
        auto f           = bundle_config->_category;
        f                = f;
        suppress_go_back = (f == 3 || f == 10 || f == 22);
      }
    }

    if (suppress_go_back) {
      auto id_array = _this->_backLogicSkipSectionIds;
      auto ids      = (SectionID*)id_array->vector;
      for (size_t i = 0; i < id_array->max_length; ++i) {
        auto id = ids[i];
        if (id == SectionID::Shop_Showcase) {
          ids[i] = SectionID::Navigation_Combat_Debug;
        }
      }

      if (auto cache_id_array = _this->backlogicCache; cache_id_array) {
        auto cache_ids = (SectionID*)cache_id_array->vector;
        for (size_t i = 0; i < cache_id_array->max_length; ++i) {
          auto id = cache_ids[i];
          if (id == SectionID::Shop_Showcase) {
            cache_ids[i] = SectionID::Navigation_Combat_Debug;
          }
        }
      }

      auto sectionID = original(_this, status, history);

      for (size_t i = 0; i < id_array->max_length; ++i) {
        auto id = ids[i];
        if (id == SectionID::Navigation_Combat_Debug) {
          ids[i] = SectionID::Shop_Showcase;
        }
      }

      if (auto cache_id_array = _this->backlogicCache; cache_id_array) {
        auto cache_ids = (SectionID*)cache_id_array->vector;
        for (size_t i = 0; i < cache_id_array->max_length; ++i) {
          auto id = cache_ids[i];
          if (id == SectionID::Navigation_Combat_Debug) {
            cache_ids[i] = SectionID::Shop_Showcase;
          }
        }
      }
      return sectionID;
    }
  }
  return original(_this, status, history);
}

void ShopSummaryDirectorCtr(auto original, ShopSummaryDirector* _this)
{
  original(_this);
}

//   const auto section_data = Hub::get_SectionManager()->_sectionStorage->GetState(sectionID);

void InstallTempCrashFixes()
{
  auto BuffService_helper =
      il2cpp_get_class_helper("Digit.Client.PrimeLib.Runtime", "Digit.PrimeServer.Services", "BuffService");

  auto ptr_extract_buffs_of_type = BuffService_helper.GetMethod("ExtractBuffsOfType");
  SPUD_STATIC_DETOUR(ptr_extract_buffs_of_type, ExtractBuffsOfType_Hook);

  auto shop_scene_manager = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Shop", "ShopSceneManager");

  auto reveal_show = shop_scene_manager.GetMethod("ShouldShowRevealSequence");
  SPUD_STATIC_DETOUR(reveal_show, ShouldShowRevealHook);

  auto shop_summary_director = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Shop", "ShopSummaryDirector");
  reveal_show                = shop_summary_director.GetMethod("Start");
  SPUD_STATIC_DETOUR(reveal_show, ShopSummaryDirectorCtr);
  reveal_show = shop_summary_director.GetMethod("GoBackBehaviour");
  SPUD_STATIC_DETOUR(reveal_show, ShopSummaryDirectorGoBackBehavior);
}
