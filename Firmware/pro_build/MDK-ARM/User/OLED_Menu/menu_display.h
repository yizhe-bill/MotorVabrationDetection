#ifndef MENU_DISPLAY_H
#define MENU_DISPLAY_H

#include "menu_core.h"
#include "OLED.h"

// 显示配置
#ifndef MENU_DISPLAY_WIDTH
#define MENU_DISPLAY_WIDTH 128     // OLED宽度
#endif

#ifndef MENU_DISPLAY_HEIGHT
#define MENU_DISPLAY_HEIGHT 64     // OLED高度
#endif

#ifndef MENU_ITEM_HEIGHT
#define MENU_ITEM_HEIGHT 16        // 每个菜单项的高度（像素）
#endif

#ifndef MENU_FONT_SIZE
#define MENU_FONT_SIZE OLED_8X16   // 菜单字体大小
#endif

#ifndef MENU_SUB_FONT_SIZE
#define MENU_SUB_FONT_SIZE OLED_6X8 // 子菜单字体大小
#endif

// 颜色定义（OLED是单色）
#define MENU_COLOR_NORMAL 0        // 正常颜色
#define MENU_COLOR_SELECTED 1      // 选中颜色（反色）
#define MENU_COLOR_TITLE 0         // 标题颜色

// 布局定义
#ifndef MENU_TITLE_HEIGHT
#define MENU_TITLE_HEIGHT 16       // 标题区域高度
#endif

#ifndef MENU_ITEM_START_Y
#define MENU_ITEM_START_Y 16       // 菜单项起始Y坐标
#endif

#ifndef MENU_ITEM_MARGIN
#define MENU_ITEM_MARGIN 2         // 菜单项间距
#endif

#ifndef MENU_SCROLLBAR_WIDTH
#define MENU_SCROLLBAR_WIDTH 4     // 滚动条宽度
#endif

// 显示初始化
void MenuDisplay_Init(void);

// 菜单绘制函数
void MenuDisplay_DrawMenu(MenuManager *manager);
void MenuDisplay_DrawTitle(MenuManager *manager);
void MenuDisplay_DrawItems(MenuManager *manager);
void MenuDisplay_DrawScrollbar(MenuManager *manager);

// 菜单项绘制
void MenuDisplay_DrawMenuItem(MenuManager *manager, MenuItem *item, uint8_t position);
void MenuDisplay_DrawSelectedItem(MenuItem *item, uint8_t position);
void MenuDisplay_DrawNormalItem(MenuItem *item, uint8_t position);

// 辅助函数
uint8_t MenuDisplay_CalculateVisibleCount(MenuManager *manager);
uint8_t MenuDisplay_GetItemYPosition(uint8_t position);
void MenuDisplay_ClearMenuArea(void);
void MenuDisplay_UpdateScreen(void);

// 特殊显示模式
void MenuDisplay_ShowMessage(const char *message);
void MenuDisplay_ShowProgressBar(const char *title, uint8_t percent);
void MenuDisplay_ShowFunctionScreen(const char *functionName);

#endif // MENU_DISPLAY_H
