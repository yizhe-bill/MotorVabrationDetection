#if defined(MENU_NAV_IMPLEMENTATION) && !defined(MENU_NAV_IMPLEMENTATION_INCLUDED)
#define MENU_NAV_IMPLEMENTATION_INCLUDED

#include "menu_nav.h"
#include "Key.h"
#include <string.h>

// 私有函数声明
static void Menu_UpdateSelectedItem(MenuManager *manager, MenuItem *newSelected);
static void Menu_AdjustScrollOffset(MenuManager *manager);
static uint8_t Menu_GetItemPosition(MenuManager *manager, MenuItem *item);

// 按键到菜单事件的转换
MenuEvent Menu_ConvertKeyToEvent(uint8_t keyCode)
{
    switch (keyCode) {
        case KEY_UP:    return MENU_EVENT_KEY_UP;
        case KEY_DOWN:  return MENU_EVENT_KEY_DOWN;
        case KEY_ENTER: return MENU_EVENT_KEY_ENTER;
        case KEY_BACK:  return MENU_EVENT_KEY_BACK;
        default:        return MENU_EVENT_TIMEOUT; // 其他情况视为超时
    }
}

// 菜单事件处理
void Menu_HandleEvent(MenuManager *manager, MenuEvent event)
{
    if (manager == NULL) return;
    if (manager->inFunction) {
        // 如果正在执行函数，只有BACK事件可以中断
        if (event == MENU_EVENT_KEY_BACK) {
            manager->inFunction = 0;
            manager->needsRedraw = 1;
        }
        return;
    }

    switch (event) {
        case MENU_EVENT_KEY_UP:
            Menu_SelectPrev(manager);
            break;

        case MENU_EVENT_KEY_DOWN:
            Menu_SelectNext(manager);
            break;

        case MENU_EVENT_KEY_ENTER:
            Menu_ExecuteCurrentItem(manager);
            break;

        case MENU_EVENT_KEY_BACK:
            Menu_GoBack(manager);
            break;

        case MENU_EVENT_TIMEOUT:
            // 超时处理（可选）
            break;

        case MENU_EVENT_FUNCTION_DONE:
            manager->inFunction = 0;
            manager->needsRedraw = 1;
            break;

        default:
            break;
    }
}

// 导航到目标菜单项
void Menu_NavigateTo(MenuManager *manager, MenuItem *target)
{
    if (manager == NULL || target == NULL) return;

    if (manager->currentMenu != NULL) {
        manager->currentMenu->lastSelectedChild = manager->selectedItem;
    }

    // 保存当前菜单到历史栈
    if (manager->currentMenu != NULL && manager->currentMenu != target) {
        Menu_PushHistory(manager, manager->currentMenu);
    }

    // 更新当前菜单和选中项
    manager->currentMenu = target;
    manager->selectedItem = target->lastSelectedChild;
    if (manager->selectedItem == NULL || manager->selectedItem->parent != target || !manager->selectedItem->visible) {
        manager->selectedItem = Menu_GetFirstVisibleChild(target);
    }
    target->lastSelectedChild = manager->selectedItem;
    manager->scrollOffset = 0;
    manager->needsRedraw = 1;
    Menu_AdjustScrollOffset(manager);
}

// 返回上一级菜单
void Menu_GoBack(MenuManager *manager)
{
    if (manager == NULL) return;
    if (!Menu_CanGoBack(manager)) return;

    if (manager->currentMenu != NULL) {
        manager->currentMenu->lastSelectedChild = manager->selectedItem;
    }

    // 从历史栈弹出上一级菜单
    MenuItem *prevMenu = Menu_PopHistory(manager);
    if (prevMenu != NULL) {
        manager->currentMenu = prevMenu;

        // 尝试恢复之前选中的项
        manager->selectedItem = prevMenu->lastSelectedChild;

        // 如果没有选中的项，选择第一个
        if (manager->selectedItem == NULL ||
            manager->selectedItem->parent != prevMenu ||
            !manager->selectedItem->visible) {
            manager->selectedItem = Menu_GetFirstVisibleChild(prevMenu);
        }
        prevMenu->lastSelectedChild = manager->selectedItem;

        manager->needsRedraw = 1;
        Menu_AdjustScrollOffset(manager);
    }
}

// 进入当前选中项的子菜单
void Menu_EnterSubmenu(MenuManager *manager)
{
    if (manager == NULL || manager->selectedItem == NULL) return;

    MenuItem *selected = manager->selectedItem;

    // 只有SUBMENU类型的项才能进入
    if (selected->type == MENU_ITEM_TYPE_SUBMENU && selected->firstChild != NULL) {
        Menu_NavigateTo(manager, selected);
    }
}

// 执行当前选中项的函数
void Menu_ExecuteCurrentItem(MenuManager *manager)
{
    if (manager == NULL || manager->selectedItem == NULL) return;

    MenuItem *selected = manager->selectedItem;

    switch (selected->type) {
        case MENU_ITEM_TYPE_FUNCTION:
            if (selected->action != NULL) {
                manager->inFunction = 1;
                selected->action(); // 执行函数
                manager->inFunction = 0;
                manager->needsRedraw = 1;
            }
            break;

        case MENU_ITEM_TYPE_SUBMENU:
            Menu_EnterSubmenu(manager);
            break;

        case MENU_ITEM_TYPE_NORMAL:
            // 普通项没有动作
            break;

        default:
            break;
    }
}

// 选择下一个可见项
void Menu_SelectNext(MenuManager *manager)
{
    if (manager == NULL || manager->selectedItem == NULL) return;

    MenuItem *next = Menu_GetNextVisibleItem(manager->selectedItem);
    if (next != NULL) {
        Menu_UpdateSelectedItem(manager, next);
        Menu_AdjustScrollOffset(manager);
    }
}

// 选择上一个可见项
void Menu_SelectPrev(MenuManager *manager)
{
    if (manager == NULL || manager->selectedItem == NULL) return;

    MenuItem *prev = Menu_GetPrevVisibleItem(manager->selectedItem);
    if (prev != NULL) {
        Menu_UpdateSelectedItem(manager, prev);
        Menu_AdjustScrollOffset(manager);
    }
}

// 选择第一个可见项
void Menu_SelectFirst(MenuManager *manager)
{
    if (manager == NULL || manager->currentMenu == NULL) return;

    MenuItem *first = Menu_GetFirstVisibleChild(manager->currentMenu);
    if (first != NULL) {
        Menu_UpdateSelectedItem(manager, first);
        manager->scrollOffset = 0;
        manager->needsRedraw = 1;
    }
}

// 选择最后一个可见项
void Menu_SelectLast(MenuManager *manager)
{
    if (manager == NULL || manager->currentMenu == NULL) return;

    // 找到最后一个可见项
    MenuItem *last = NULL;
    MenuItem *child = Menu_GetFirstVisibleChild(manager->currentMenu);
    while (child != NULL) {
        if (child->visible) last = child;
        child = Menu_GetNextVisibleItem(child);
    }

    if (last != NULL) {
        Menu_UpdateSelectedItem(manager, last);

        // 调整滚动偏移
        uint8_t visibleCount = Menu_CountVisibleChildren(manager->currentMenu);
        if (visibleCount > manager->maxVisibleItems) {
            manager->scrollOffset = visibleCount - manager->maxVisibleItems;
        }
        manager->needsRedraw = 1;
    }
}

// 推入历史栈
uint8_t Menu_PushHistory(MenuManager *manager, MenuItem *item)
{
    if (manager == NULL || item == NULL) return 0;
    if (manager->historyTop >= MENU_HISTORY_DEPTH) return 0;

    manager->historyStack[manager->historyTop++] = item;
    return 1;
}

// 弹出历史栈
MenuItem* Menu_PopHistory(MenuManager *manager)
{
    if (manager == NULL) return NULL;
    if (manager->historyTop == 0) return NULL;

    return manager->historyStack[--manager->historyTop];
}

// 查看历史栈顶部
MenuItem* Menu_PeekHistory(MenuManager *manager)
{
    if (manager == NULL) return NULL;
    if (manager->historyTop == 0) return NULL;

    return manager->historyStack[manager->historyTop - 1];
}

// 清空历史栈
void Menu_ClearHistory(MenuManager *manager)
{
    if (manager == NULL) return;

    memset(manager->historyStack, 0, sizeof(manager->historyStack));
    manager->historyTop = 0;
}

// 检查是否可以返回
uint8_t Menu_CanGoBack(MenuManager *manager)
{
    if (manager == NULL) return 0;
    return (manager->historyTop > 0);
}

// 检查是否正在执行函数
uint8_t Menu_IsInFunction(MenuManager *manager)
{
    if (manager == NULL) return 0;
    return manager->inFunction;
}

// 获取当前选中的菜单项
MenuItem* Menu_GetSelectedItem(MenuManager *manager)
{
    if (manager == NULL) return NULL;
    return manager->selectedItem;
}

// 获取当前菜单
MenuItem* Menu_GetCurrentMenu(MenuManager *manager)
{
    if (manager == NULL) return NULL;
    return manager->currentMenu;
}

// === 私有函数实现 ===

// 更新选中的项
static void Menu_UpdateSelectedItem(MenuManager *manager, MenuItem *newSelected)
{
    if (manager == NULL || newSelected == NULL) return;

    // 取消之前选中项的状态
    if (manager->selectedItem != NULL) {
        manager->selectedItem->selected = 0;
    }

    // 设置新的选中项
    manager->selectedItem = newSelected;
    newSelected->selected = 1;
    if (manager->currentMenu != NULL) {
        manager->currentMenu->lastSelectedChild = newSelected;
    }
    manager->needsRedraw = 1;
}

// 调整滚动偏移
static void Menu_AdjustScrollOffset(MenuManager *manager)
{
    if (manager == NULL || manager->selectedItem == NULL) return;
    if (manager->currentMenu == NULL) return;

    uint8_t position = Menu_GetItemPosition(manager, manager->selectedItem);

    if (position < manager->scrollOffset) {
        // 选中的项在可视区域上方，需要向上滚动
        manager->scrollOffset = position;
        manager->needsRedraw = 1;
    } else if (position >= manager->scrollOffset + manager->maxVisibleItems) {
        // 选中的项在可视区域下方，需要向下滚动
        manager->scrollOffset = position - manager->maxVisibleItems + 1;
        manager->needsRedraw = 1;
    }
}

// 获取菜单项在兄弟节点中的位置（从0开始）
static uint8_t Menu_GetItemPosition(MenuManager *manager, MenuItem *item)
{
    if (manager == NULL || item == NULL) return 0;

    uint8_t position = 0;
    MenuItem *sibling = Menu_GetFirstVisibleChild(manager->currentMenu);

    while (sibling != NULL && sibling != item) {
        if (sibling->visible) position++;
        sibling = Menu_GetNextVisibleItem(sibling);
    }

    return position;
}

#endif // MENU_NAV_IMPLEMENTATION_INCLUDED
