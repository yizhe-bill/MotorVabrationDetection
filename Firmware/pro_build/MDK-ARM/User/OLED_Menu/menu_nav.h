#ifndef MENU_NAV_H
#define MENU_NAV_H

#include "menu_core.h"

// 按键到菜单事件的转换
MenuEvent Menu_ConvertKeyToEvent(uint8_t keyCode);

// 菜单事件处理
void Menu_HandleEvent(MenuManager *manager, MenuEvent event);

// 导航控制
void Menu_NavigateTo(MenuManager *manager, MenuItem *target);
void Menu_GoBack(MenuManager *manager);
void Menu_EnterSubmenu(MenuManager *manager);
void Menu_ExecuteCurrentItem(MenuManager *manager);

// 选中项控制
void Menu_SelectNext(MenuManager *manager);
void Menu_SelectPrev(MenuManager *manager);
void Menu_SelectFirst(MenuManager *manager);
void Menu_SelectLast(MenuManager *manager);

// 历史栈操作
uint8_t Menu_PushHistory(MenuManager *manager, MenuItem *item);
MenuItem* Menu_PopHistory(MenuManager *manager);
MenuItem* Menu_PeekHistory(MenuManager *manager);
void Menu_ClearHistory(MenuManager *manager);

// 状态查询
uint8_t Menu_CanGoBack(MenuManager *manager);
uint8_t Menu_IsInFunction(MenuManager *manager);
MenuItem* Menu_GetSelectedItem(MenuManager *manager);
MenuItem* Menu_GetCurrentMenu(MenuManager *manager);

#endif // MENU_NAV_H