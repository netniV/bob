#pragma once

#include <il2cpp/il2cpp_helper.h>

#include "BattleTargetData.h"
#include "ScanEngageButtonsWidget.h"
#include "ViewController.h"

class ShopSummaryDirector
{
public:
  __declspec(property(get = __get__backLogicSkipSectionIds)) Il2CppArraySize* _backLogicSkipSectionIds;
  __declspec(property(get = __get_backlogicCache)) Il2CppArraySize* backlogicCache;

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Shop", "ShopSummaryDirector");
    return class_helper;
  }

public:
  Il2CppArraySize* __get__backLogicSkipSectionIds()
  {
    static auto field = get_class_helper().GetField(xorstr_("_backLogicSkipSectionIds")).offset();
    return *(Il2CppArraySize**)((char*)this + field);
  }

  Il2CppArraySize* __get_backlogicCache()
  {
    static auto field = get_class_helper().GetField(xorstr_("backlogicCache")).offset();
    return *(Il2CppArraySize**)((char*)this + field);
  }
};
