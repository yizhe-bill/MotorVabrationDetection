#ifndef MENU_CONFIG_H
#define MENU_CONFIG_H

// 菜单系统配置

// 内存配置
#define MENU_ITEM_POOL_SIZE 50      // 菜单项内存池大小
#define MENU_HISTORY_DEPTH 10       // 历史栈深度
#define MENU_MAX_VISIBLE_ITEMS 3    // 最大可见菜单项数（64px OLED: 标题16px + 3行菜单）

// 显示配置
#define MENU_DISPLAY_WIDTH 128      // OLED宽度
#define MENU_DISPLAY_HEIGHT 64      // OLED高度
#define MENU_TITLE_HEIGHT 16        // 标题区域高度
#define MENU_ITEM_HEIGHT 16         // 菜单项高度
#define MENU_FONT_SIZE OLED_8X16    // 主字体大小
#define MENU_SUB_FONT_SIZE OLED_6X8 // 子菜单字体大小

// 动画配置
#define MENU_ANIMATION_ENABLED 0    // 是否启用动画（0=禁用，1=启用）
#define MENU_ANIMATION_SPEED 50     // 动画速度（ms）

// 超时配置
#define MENU_TIMEOUT_ENABLED 0      // 是否启用超时返回（0=禁用，1=启用）
#define MENU_TIMEOUT_MS 30000       // 超时时间（ms）

// 调试配置
#define MENU_DEBUG_ENABLED 0        // 是否启用调试输出（0=禁用，1=启用）

// 兼容性配置
#define MENU_COMPATIBILITY_MODE 1   // 是否启用兼容模式（使用旧API）

// 功能配置
#define MENU_ENABLE_SCROLLBAR 1     // 是否启用滚动条
#define MENU_ENABLE_TITLE 1         // 是否显示标题
#define MENU_ENABLE_BACK_CONFIRM 0  // 是否启用返回确认

// 菜单项类型指示符
#define MENU_INDICATOR_SUBMENU "> "  // 子菜单指示符
#define MENU_INDICATOR_FUNCTION "* " // 函数指示符
#define MENU_INDICATOR_NORMAL "  "   // 普通项指示符

// 错误代码
typedef enum {
    MENU_OK = 0,
    MENU_ERROR_NULL_POINTER,
    MENU_ERROR_MEMORY_FULL,
    MENU_ERROR_INVALID_PARAM,
    MENU_ERROR_NOT_FOUND,
    MENU_ERROR_HISTORY_FULL,
    MENU_ERROR_HISTORY_EMPTY,
    MENU_ERROR_NO_CHILDREN,
    MENU_ERROR_ALREADY_EXISTS
} MenuErrorCode;

// 菜单系统版本
#define MENU_VERSION_MAJOR 1
#define MENU_VERSION_MINOR 0
#define MENU_VERSION_PATCH 0

#endif // MENU_CONFIG_H
