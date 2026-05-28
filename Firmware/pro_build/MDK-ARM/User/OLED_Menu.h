#ifndef __OLED_MENU_H_
#define __OLED_MENU_H_

/* 重构后的菜单系统头文件 */

#include "Key.h"
#include "OLED_Menu/menu_core.h"
#include "OLED_Menu/menu_nav.h"
#include "OLED_Menu/menu_display.h"
#include "OLED_Menu/menu_config.h"

/* 兼容性定义 - 保持原有接口 */
#define OLED_RECT_WIDTH 128      // OLED矩形宽度
#define OLED_RECT_HEIGHT 16      // OLED矩形高度

/* 兼容性枚举 - 保持原有定义 */
typedef enum {
    MENU_FUNC_TEST = 1,
    MENU_WORK_MODE,
    MENU_ABOUT_ME
} MenuState_Compat;

typedef enum {
    ADXL345_TEST = 2,
    OLED_TEST,
    DS18B20_TEST,
    USART_TEST,
    KEY_TEST
} TestMenu_Compat;

typedef enum {
    MENU_INDEX_MAIN = 1, // 主菜单索引值
    MENU_INDEX_SUB,      // 子菜单索引值
    MENU_INDEX_TEST      // 测试菜单索引值
} MenuIndex_Compat;

/* 外部变量声明 - 保持原有变量名（用于兼容性） */
extern uint8_t Num_Choose;                 // 菜单选择值（兼容性）
extern uint16_t flag_subMenu, flag_test;   // 子菜单和测试标志位（兼容性）
extern uint16_t flag_adxl345, flag_oled, flag_ds18b20, flag_usart, flag_key; // 功能标志位（兼容性）

/* 全局菜单管理器（新系统） */
extern MenuManager g_menuManager;

/* 兼容性函数声明 - 保持原有API */
void Choose_menu(void);              // 菜单选择函数
void function_test(void);            // 功能测试函数（兼容性）
void Work_Mode(void);                // 工作模式函数（兼容性）

/* 新菜单系统函数 */
void OLED_Menu_Init(void);           // 新菜单系统初始化
void OLED_Menu_Update(void);         // 菜单更新函数（替代Choose_menu）
uint8_t OLED_Menu_IsActive(void);    // 检查菜单是否激活

/* 显示函数（兼容性） */
void display_main_menu(void);        // 显示主菜单（兼容性）
void display_sub_menu(void);         // 显示子菜单（兼容性）
void display_about_me(void);         // 显示关于信息（兼容性）

#endif /* __OLED_MENU_H_ */