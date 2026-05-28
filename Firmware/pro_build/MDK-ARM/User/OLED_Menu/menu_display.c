#if defined(MENU_DISPLAY_IMPLEMENTATION) && !defined(MENU_DISPLAY_IMPLEMENTATION_INCLUDED)
#define MENU_DISPLAY_IMPLEMENTATION_INCLUDED

#include "menu_display.h"
#include <stdio.h>
#include <string.h>

// 显示初始化
void MenuDisplay_Init(void)
{
    // 无需特殊初始化，OLED已经在主程序中初始化
}

// 绘制完整菜单
void MenuDisplay_DrawMenu(MenuManager *manager)
{
    if (manager == NULL) return;
    if (!manager->needsRedraw) return;

    // 清除菜单区域
    MenuDisplay_ClearMenuArea();

    // 绘制标题
    MenuDisplay_DrawTitle(manager);

    // 绘制菜单项
    MenuDisplay_DrawItems(manager);

    // 如果需要，绘制滚动条
    if (MenuDisplay_CalculateVisibleCount(manager) > manager->maxVisibleItems) {
        MenuDisplay_DrawScrollbar(manager);
    }

    // 更新屏幕
    MenuDisplay_UpdateScreen();

    // 清除重绘标志
    manager->needsRedraw = 0;
}

// 绘制标题
void MenuDisplay_DrawTitle(MenuManager *manager)
{
    if (manager == NULL || manager->currentMenu == NULL) return;

    char title[32] = "Menu";

    // 根据当前菜单设置标题
    if (manager->currentMenu->parent == NULL) {
        strcpy(title, "Main Menu");
    } else {
        // 使用父菜单的名称作为标题，但限制长度
        strncpy(title, manager->currentMenu->name, sizeof(title) - 1);
        title[sizeof(title) - 1] = '\0';
    }

    // 居中显示标题
    uint8_t titleLength = strlen(title);
    uint8_t titleWidth = titleLength * 8; // 假设8x16字体
    uint8_t titleX = (titleWidth < MENU_DISPLAY_WIDTH) ? (MENU_DISPLAY_WIDTH - titleWidth) / 2 : 0;

    // 清除标题区域
    OLED_ClearArea(0, 0, MENU_DISPLAY_WIDTH, MENU_TITLE_HEIGHT);

    // 绘制标题
    OLED_ShowString(titleX, 0, title, MENU_FONT_SIZE);

    // 绘制分隔线
    for (uint8_t x = 0; x < MENU_DISPLAY_WIDTH; x++) {
        OLED_DrawPoint(x, MENU_TITLE_HEIGHT - 1);
    }
}

// 绘制菜单项
void MenuDisplay_DrawItems(MenuManager *manager)
{
    if (manager == NULL || manager->currentMenu == NULL) return;

    // 获取第一个可见子节点
    MenuItem *item = Menu_GetFirstVisibleChild(manager->currentMenu);
    if (item == NULL) return;

    // 跳过滚动偏移
    uint8_t skipped = 0;
    while (item != NULL && skipped < manager->scrollOffset) {
        if (item->visible) skipped++;
        item = Menu_GetNextVisibleItem(item);
    }

    // 绘制可见的菜单项
    uint8_t position = 0;
    while (item != NULL && position < manager->maxVisibleItems) {
        if (item->visible) {
            MenuDisplay_DrawMenuItem(manager, item, position);
            position++;
        }
        item = Menu_GetNextVisibleItem(item);
    }
}

// 绘制菜单项
void MenuDisplay_DrawMenuItem(MenuManager *manager, MenuItem *item, uint8_t position)
{
    if (item == NULL) return;

    // 计算Y位置
    uint8_t y = MenuDisplay_GetItemYPosition(position);

    // 清除该项区域
    OLED_ClearArea(0, y, MENU_DISPLAY_WIDTH - MENU_SCROLLBAR_WIDTH, MENU_ITEM_HEIGHT);

    if (item == manager->selectedItem) {
        MenuDisplay_DrawSelectedItem(item, position);
    } else {
        MenuDisplay_DrawNormalItem(item, position);
    }
}

// 绘制选中的菜单项
void MenuDisplay_DrawSelectedItem(MenuItem *item, uint8_t position)
{
    if (item == NULL) return;

    uint8_t y = MenuDisplay_GetItemYPosition(position);

    // 绘制菜单项文本
    char displayText[40];
    char prefix[4] = "";

    // 根据类型添加前缀
    switch (item->type) {
        case MENU_ITEM_TYPE_SUBMENU:
            strcpy(prefix, "> ");
            break;
        case MENU_ITEM_TYPE_FUNCTION:
            strcpy(prefix, "* ");
            break;
        default:
            strcpy(prefix, "  ");
            break;
    }

    snprintf(displayText, sizeof(displayText), "%s%s", prefix, item->name);

    // 显示文本
    OLED_ShowString(2, y + 4, displayText, MENU_SUB_FONT_SIZE);

    // 先写文本再反色，这样选中项能形成白底黑字效果
    OLED_ReverseArea(0, y, MENU_DISPLAY_WIDTH - MENU_SCROLLBAR_WIDTH, MENU_ITEM_HEIGHT);
}

// 绘制普通菜单项
void MenuDisplay_DrawNormalItem(MenuItem *item, uint8_t position)
{
    if (item == NULL) return;

    uint8_t y = MenuDisplay_GetItemYPosition(position);

    // 绘制菜单项文本
    char displayText[40];
    char prefix[4] = "";

    // 根据类型添加前缀
    switch (item->type) {
        case MENU_ITEM_TYPE_SUBMENU:
            strcpy(prefix, "> ");
            break;
        case MENU_ITEM_TYPE_FUNCTION:
            strcpy(prefix, "* ");
            break;
        default:
            strcpy(prefix, "  ");
            break;
    }

    snprintf(displayText, sizeof(displayText), "%s%s", prefix, item->name);

    // 显示文本
    OLED_ShowString(2, y + 4, displayText, MENU_SUB_FONT_SIZE);
}

// 绘制滚动条
void MenuDisplay_DrawScrollbar(MenuManager *manager)
{
    if (manager == NULL) return;

    uint8_t totalItems = Menu_CountVisibleChildren(manager->currentMenu);
    if (totalItems <= manager->maxVisibleItems) return;

    // 计算滚动条参数
    uint8_t scrollbarHeight = MENU_DISPLAY_HEIGHT - MENU_TITLE_HEIGHT;
    uint8_t thumbHeight = (manager->maxVisibleItems * scrollbarHeight) / totalItems;
    if (thumbHeight < 4) thumbHeight = 4; // 最小高度

    uint8_t thumbY = MENU_TITLE_HEIGHT + (manager->scrollOffset * (scrollbarHeight - thumbHeight)) /
                     (totalItems - manager->maxVisibleItems);

    // 绘制滚动条背景
    for (uint8_t y = MENU_TITLE_HEIGHT; y < MENU_DISPLAY_HEIGHT; y++) {
        for (uint8_t x = MENU_DISPLAY_WIDTH - MENU_SCROLLBAR_WIDTH; x < MENU_DISPLAY_WIDTH; x++) {
            if (x == MENU_DISPLAY_WIDTH - 1) {
                OLED_DrawPoint(x, y);
            }
        }
    }

    // 绘制滚动条滑块
    for (uint8_t y = thumbY; y < thumbY + thumbHeight; y++) {
        for (uint8_t x = MENU_DISPLAY_WIDTH - MENU_SCROLLBAR_WIDTH; x < MENU_DISPLAY_WIDTH; x++) {
            OLED_DrawPoint(x, y);
        }
    }
}

// 计算可见项总数
uint8_t MenuDisplay_CalculateVisibleCount(MenuManager *manager)
{
    if (manager == NULL || manager->currentMenu == NULL) return 0;
    return Menu_CountVisibleChildren(manager->currentMenu);
}

// 获取菜单项的Y位置
uint8_t MenuDisplay_GetItemYPosition(uint8_t position)
{
    return MENU_ITEM_START_Y + (position * MENU_ITEM_HEIGHT);
}

// 清除菜单区域
void MenuDisplay_ClearMenuArea(void)
{
    OLED_ClearArea(0, MENU_TITLE_HEIGHT, MENU_DISPLAY_WIDTH,
                   MENU_DISPLAY_HEIGHT - MENU_TITLE_HEIGHT);
}

// 更新屏幕
void MenuDisplay_UpdateScreen(void)
{
    OLED_Update();
}

// 显示消息
void MenuDisplay_ShowMessage(const char *message)
{
    OLED_Clear();

    // 居中显示消息
    uint8_t msgLength = strlen(message);
    uint8_t msgWidth = msgLength * 6; // 假设6x8字体
    uint8_t msgX = (msgWidth < MENU_DISPLAY_WIDTH) ? (MENU_DISPLAY_WIDTH - msgWidth) / 2 : 0;
    uint8_t msgY = (MENU_DISPLAY_HEIGHT - 8) / 2;

    OLED_ShowString(msgX, msgY, (char*)message, OLED_6X8);
    OLED_Update();
}

// 显示进度条
void MenuDisplay_ShowProgressBar(const char *title, uint8_t percent)
{
    OLED_Clear();

    // 显示标题
    uint8_t titleLength = strlen(title);
    uint8_t titleWidth = titleLength * 6;
    uint8_t titleX = (titleWidth < MENU_DISPLAY_WIDTH) ? (MENU_DISPLAY_WIDTH - titleWidth) / 2 : 0;
    OLED_ShowString(titleX, 10, (char*)title, OLED_6X8);

    // 绘制进度条边框
    uint8_t barWidth = 100;
    uint8_t barHeight = 10;
    uint8_t barX = (MENU_DISPLAY_WIDTH - barWidth) / 2;
    uint8_t barY = 30;

    OLED_DrawRectangle(barX, barY, barWidth, barHeight, OLED_UNFILLED);

    // 绘制进度
    if (percent > 100) percent = 100;
    uint8_t progressWidth = (barWidth * percent) / 100;

    if (progressWidth > 2) {
        OLED_DrawRectangle(barX + 1, barY + 1, progressWidth - 2, barHeight - 2, OLED_FILLED);
    }

    // 显示百分比
    char percentStr[10];
    snprintf(percentStr, sizeof(percentStr), "%d%%", percent);

    uint8_t percentLength = strlen(percentStr);
    uint8_t percentWidth = percentLength * 6;
    uint8_t percentX = (percentWidth < MENU_DISPLAY_WIDTH) ? (MENU_DISPLAY_WIDTH - percentWidth) / 2 : 0;
    OLED_ShowString(percentX, 45, percentStr, OLED_6X8);

    OLED_Update();
}

// 显示函数执行屏幕
void MenuDisplay_ShowFunctionScreen(const char *functionName)
{
    OLED_Clear();

    // 显示"正在执行..."
    OLED_ShowString(40, 20, "Executing...", OLED_6X8);

    // 显示函数名称
    char funcDisplay[40];
    snprintf(funcDisplay, sizeof(funcDisplay), "Function: %s", functionName);

    uint8_t funcLength = strlen(funcDisplay);
    if (funcLength > 21) funcDisplay[21] = '\0'; // 限制长度

    uint8_t funcWidth = strlen(funcDisplay) * 6;
    uint8_t funcX = (funcWidth < MENU_DISPLAY_WIDTH) ? (MENU_DISPLAY_WIDTH - funcWidth) / 2 : 0;

    OLED_ShowString(funcX, 35, funcDisplay, OLED_6X8);

    // 显示提示
    OLED_ShowString(30, 50, "Press BACK to cancel", OLED_6X8);

    OLED_Update();
}

#endif // MENU_DISPLAY_IMPLEMENTATION_INCLUDED
