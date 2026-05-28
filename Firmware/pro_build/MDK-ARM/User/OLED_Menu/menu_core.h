#ifndef MENU_CORE_H
#define MENU_CORE_H

#include "main.h"
#include "menu_config.h"
#include <stdint.h>

// 菜单项类型枚举
typedef enum {
    MENU_ITEM_TYPE_NORMAL,      // 普通菜单项
    MENU_ITEM_TYPE_FUNCTION,    // 执行函数
    MENU_ITEM_TYPE_SUBMENU      // 子菜单
} MenuItemType;

// 菜单项结构体（双向链表节点）
typedef struct MenuItem {
    char name[32];              // 菜单项名称（存储常量字符串副本）
    MenuItemType type;          // 菜单项类型
    void (*action)(void);       // 执行函数（如果类型为FUNCTION）

    // 树形结构指针
    struct MenuItem *parent;    // 父节点
    struct MenuItem *firstChild;// 第一个子节点
    struct MenuItem *lastChild; // 最后一个子节点（用于快速添加）
    struct MenuItem *nextSibling;// 下一个兄弟节点
    struct MenuItem *prevSibling;// 上一个兄弟节点
    struct MenuItem *lastSelectedChild; // 记录该菜单下次进入时应恢复的选中项

    // 显示状态（使用位域节省内存）
    uint8_t visible : 1;        // 是否可见
    uint8_t selected : 1;       // 是否被选中
    uint8_t expanded : 1;       // 是否展开（用于子菜单）
} MenuItem;

// 菜单管理器结构体
#ifndef MENU_HISTORY_DEPTH
#define MENU_HISTORY_DEPTH 10   // 历史栈深度
#endif

#ifndef MENU_MAX_VISIBLE_ITEMS
#define MENU_MAX_VISIBLE_ITEMS 3 // 64px OLED: 16px标题 + 3行16px菜单项
#endif

typedef struct MenuManager {
    MenuItem *root;             // 根菜单
    MenuItem *currentMenu;      // 当前显示的菜单
    MenuItem *selectedItem;     // 当前选中的菜单项

    // 历史栈（用于返回功能）
    MenuItem *historyStack[MENU_HISTORY_DEPTH];
    uint8_t historyTop;         // 栈顶指针

    // 显示状态
    uint8_t scrollOffset;       // 滚动偏移
    uint8_t needsRedraw;        // 需要重绘标志
    uint8_t inFunction;         // 正在执行函数标志

    // 配置
    uint8_t maxVisibleItems;    // 最大可见菜单项数
} MenuManager;

// 菜单状态枚举
typedef enum {
    MENU_STATE_IDLE,            // 空闲状态
    MENU_STATE_NAVIGATING,      // 导航状态
    MENU_STATE_EXECUTING,       // 执行函数状态
    MENU_STATE_CONFIRMING       // 确认状态
} MenuState;

// 按键事件枚举
typedef enum {
    MENU_EVENT_KEY_UP,
    MENU_EVENT_KEY_DOWN,
    MENU_EVENT_KEY_ENTER,
    MENU_EVENT_KEY_BACK,
    MENU_EVENT_TIMEOUT,
    MENU_EVENT_FUNCTION_DONE
} MenuEvent;

// 初始化函数
void Menu_Init(MenuManager *manager);

// 菜单项创建和管理
MenuItem* Menu_CreateItem(const char *name, MenuItemType type, void (*action)(void));
void Menu_DestroyItem(MenuItem *item);

// 树形结构操作
void Menu_AddChild(MenuItem *parent, MenuItem *child);
void Menu_RemoveChild(MenuItem *parent, MenuItem *child);
void Menu_AddSibling(MenuItem *item, MenuItem *sibling);
MenuItem* Menu_GetFirstVisibleChild(MenuItem *parent);
MenuItem* Menu_GetNextVisibleItem(MenuItem *item);
MenuItem* Menu_GetPrevVisibleItem(MenuItem *item);
uint8_t Menu_CountVisibleChildren(MenuItem *parent);

// 菜单管理器操作
void Menu_Reset(MenuManager *manager);
void Menu_SetRoot(MenuManager *manager, MenuItem *root);
void Menu_RefreshDisplayState(MenuManager *manager);

#endif // MENU_CORE_H
