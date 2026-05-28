#if defined(MENU_CORE_IMPLEMENTATION) && !defined(MENU_CORE_IMPLEMENTATION_INCLUDED)
#define MENU_CORE_IMPLEMENTATION_INCLUDED

#include "menu_core.h"
#include <string.h>

// 内存池配置
static MenuItem g_menuItemPool[MENU_ITEM_POOL_SIZE];
static uint8_t g_menuItemUsed[MENU_ITEM_POOL_SIZE] = {0};

// 私有函数声明
static MenuItem* Menu_AllocateItem(void);
static void Menu_FreeItem(MenuItem *item);
static void Menu_InitItem(MenuItem *item, const char *name, MenuItemType type, void (*action)(void));

// 初始化菜单管理器
void Menu_Init(MenuManager *manager)
{
    if (manager == NULL) return;

    memset(manager, 0, sizeof(MenuManager));
    manager->maxVisibleItems = MENU_MAX_VISIBLE_ITEMS;
    manager->historyTop = 0;
    manager->needsRedraw = 1; // 初始需要重绘
}

// 创建菜单项
MenuItem* Menu_CreateItem(const char *name, MenuItemType type, void (*action)(void))
{
    MenuItem *item = Menu_AllocateItem();
    if (item == NULL) return NULL;

    Menu_InitItem(item, name, type, action);
    return item;
}

// 销毁菜单项
void Menu_DestroyItem(MenuItem *item)
{
    if (item == NULL) return;

    // 先移除所有子节点
    MenuItem *child = item->firstChild;
    while (child != NULL) {
        MenuItem *next = child->nextSibling;
        Menu_DestroyItem(child);
        child = next;
    }

    // 从父节点中移除自己
    if (item->parent != NULL) {
        Menu_RemoveChild(item->parent, item);
    }

    // 释放内存
    Menu_FreeItem(item);
}

// 添加子节点
void Menu_AddChild(MenuItem *parent, MenuItem *child)
{
    if (parent == NULL || child == NULL) return;

    // 移除子节点原来的父节点关系
    if (child->parent != NULL) {
        Menu_RemoveChild(child->parent, child);
    }

    // 设置新的父节点
    child->parent = parent;

    // 添加到子节点链表末尾
    if (parent->firstChild == NULL) {
        parent->firstChild = child;
        parent->lastChild = child;
        child->prevSibling = NULL;
        child->nextSibling = NULL;
    } else {
        child->prevSibling = parent->lastChild;
        child->nextSibling = NULL;
        parent->lastChild->nextSibling = child;
        parent->lastChild = child;
    }

    // 默认可见
    child->visible = 1;
}

// 移除子节点
void Menu_RemoveChild(MenuItem *parent, MenuItem *child)
{
    if (parent == NULL || child == NULL) return;
    if (child->parent != parent) return;

    // 更新兄弟节点指针
    if (child->prevSibling != NULL) {
        child->prevSibling->nextSibling = child->nextSibling;
    } else {
        // 如果是第一个子节点
        parent->firstChild = child->nextSibling;
    }

    if (child->nextSibling != NULL) {
        child->nextSibling->prevSibling = child->prevSibling;
    } else {
        // 如果是最后一个子节点
        parent->lastChild = child->prevSibling;
    }

    // 清除子节点的父节点指针
    child->parent = NULL;
    child->prevSibling = NULL;
    child->nextSibling = NULL;
}

// 添加兄弟节点
void Menu_AddSibling(MenuItem *item, MenuItem *sibling)
{
    if (item == NULL || sibling == NULL) return;
    if (item->parent == NULL) return;

    Menu_AddChild(item->parent, sibling);
}

// 获取第一个可见子节点
MenuItem* Menu_GetFirstVisibleChild(MenuItem *parent)
{
    if (parent == NULL) return NULL;

    MenuItem *child = parent->firstChild;
    while (child != NULL) {
        if (child->visible) return child;
        child = child->nextSibling;
    }

    return NULL;
}

// 获取下一个可见项
MenuItem* Menu_GetNextVisibleItem(MenuItem *item)
{
    if (item == NULL) return NULL;

    // 先检查兄弟节点
    MenuItem *sibling = item->nextSibling;
    while (sibling != NULL) {
        if (sibling->visible) return sibling;
        sibling = sibling->nextSibling;
    }

    // 如果没有兄弟节点，返回NULL
    return NULL;
}

// 获取上一个可见项
MenuItem* Menu_GetPrevVisibleItem(MenuItem *item)
{
    if (item == NULL) return NULL;

    // 检查兄弟节点
    MenuItem *sibling = item->prevSibling;
    while (sibling != NULL) {
        if (sibling->visible) return sibling;
        sibling = sibling->prevSibling;
    }

    // 如果没有兄弟节点，返回NULL
    return NULL;
}

// 计算可见子节点数量
uint8_t Menu_CountVisibleChildren(MenuItem *parent)
{
    if (parent == NULL) return 0;

    uint8_t count = 0;
    MenuItem *child = parent->firstChild;
    while (child != NULL) {
        if (child->visible) count++;
        child = child->nextSibling;
    }

    return count;
}

// 重置菜单管理器
void Menu_Reset(MenuManager *manager)
{
    if (manager == NULL) return;

    // 释放所有菜单项
    if (manager->root != NULL) {
        Menu_DestroyItem(manager->root);
    }

    // 重置管理器
    Menu_Init(manager);
}

// 设置根菜单
void Menu_SetRoot(MenuManager *manager, MenuItem *root)
{
    if (manager == NULL || root == NULL) return;

    manager->root = root;
    manager->currentMenu = root;
    manager->selectedItem = Menu_GetFirstVisibleChild(root);
    root->lastSelectedChild = manager->selectedItem;
    manager->needsRedraw = 1;
}

// 刷新显示状态
void Menu_RefreshDisplayState(MenuManager *manager)
{
    if (manager == NULL || manager->currentMenu == NULL) return;

    // 确保有选中的项
    if (manager->selectedItem == NULL) {
        manager->selectedItem = Menu_GetFirstVisibleChild(manager->currentMenu);
    }

    // 计算滚动偏移
    uint8_t visibleCount = Menu_CountVisibleChildren(manager->currentMenu);
    if (visibleCount <= manager->maxVisibleItems) {
        manager->scrollOffset = 0;
    } else {
        // 需要实现滚动逻辑（在menu_nav.c中）
    }

    manager->needsRedraw = 1;
}

// === 私有函数实现 ===

// 分配菜单项内存
static MenuItem* Menu_AllocateItem(void)
{
    for (uint8_t i = 0; i < MENU_ITEM_POOL_SIZE; i++) {
        if (!g_menuItemUsed[i]) {
            g_menuItemUsed[i] = 1;
            return &g_menuItemPool[i];
        }
    }
    return NULL; // 内存池已满
}

// 释放菜单项内存
static void Menu_FreeItem(MenuItem *item)
{
    if (item == NULL) return;

    // 计算索引
    uint32_t index = item - g_menuItemPool;
    if (index < MENU_ITEM_POOL_SIZE) {
        g_menuItemUsed[index] = 0;
    }

    // 清除内容
    memset(item, 0, sizeof(MenuItem));
}

// 初始化菜单项
static void Menu_InitItem(MenuItem *item, const char *name, MenuItemType type, void (*action)(void))
{
    if (item == NULL) return;

    memset(item, 0, sizeof(MenuItem));

    // 复制名称（确保不超过缓冲区）
    if (name != NULL) {
        strncpy(item->name, name, sizeof(item->name) - 1);
        item->name[sizeof(item->name) - 1] = '\0';
    }

    item->type = type;
    item->action = action;
    item->visible = 1; // 默认可见
    item->selected = 0;
    item->expanded = 0;
}

#endif // MENU_CORE_IMPLEMENTATION_INCLUDED
