#include "OLED_Menu.h"
#include "OLED.h"
#include "ADXL345.h"
#include "ds18b20.h"
#include "usart.h"
#include "MyDSP.h"
#include <stdio.h>

// 全局变量 - 保持兼容性
uint8_t Num_Choose = 1;
uint16_t flag_subMenu = 1, flag_test = 0;
uint16_t flag_adxl345 = 0, flag_oled = 0, flag_ds18b20 = 0, flag_usart = 0, flag_key = 0;

// 全局菜单管理器（新系统）
MenuManager g_menuManager;

// 菜单节点缓存，方便兼容旧API和调试变量
static MenuItem *s_mainMenu = NULL;
static MenuItem *s_funcTestMenu = NULL;

// 函数原型声明
static void OLED_Menu_CreateDefaultMenu(void);
static void OLED_Menu_SetCurrentMenu(MenuItem *menu, MenuItem *backTarget);
static void OLED_Menu_SyncCompatState(void);
static uint8_t OLED_Menu_GetSelectedIndex(void);
static void OLED_Menu_ClearFunctionFlags(void);
static void Test_ADXL345(void);
static void Test_OLED(void);
static void Test_DS18B20(void);
static void Test_USART(void);
static void Test_KEY(void);
static void Run_Work_Mode(void);
static void Show_About_Info(void);

// 兼容性显示函数实现
void display_main_menu(void)
{
    // 兼容性函数，使用新系统显示主菜单
    if (g_menuManager.root == NULL) {
        OLED_Menu_Init();
    }

    if (s_mainMenu != NULL) {
        OLED_Menu_SetCurrentMenu(s_mainMenu, NULL);
        MenuDisplay_DrawMenu(&g_menuManager);
    }
}

void display_sub_menu(void)
{
    // 兼容性函数，显示功能测试子菜单
    if (g_menuManager.root == NULL) {
        OLED_Menu_Init();
    }

    if (s_funcTestMenu != NULL) {
        OLED_Menu_SetCurrentMenu(s_funcTestMenu, s_mainMenu);
        MenuDisplay_DrawMenu(&g_menuManager);
    }
}

void display_about_me(void)
{
    // 兼容性函数，显示关于信息
    Show_About_Info();
}

// 新菜单系统初始化
void OLED_Menu_Init(void)
{
    if (g_menuManager.root != NULL) {
        Menu_Reset(&g_menuManager);
    } else {
        Menu_Init(&g_menuManager);
    }

    OLED_Menu_CreateDefaultMenu();

    // 初始化显示
    MenuDisplay_Init();
    OLED_Menu_SyncCompatState();
}

// 创建默认菜单结构
static void OLED_Menu_CreateDefaultMenu(void)
{
    s_mainMenu = NULL;
    s_funcTestMenu = NULL;

    // 创建根菜单
    MenuItem *root = Menu_CreateItem("Root", MENU_ITEM_TYPE_SUBMENU, NULL);
    if (root == NULL) return;
    Menu_SetRoot(&g_menuManager, root);

    // 创建主菜单
    MenuItem *mainMenu = Menu_CreateItem("Main Menu", MENU_ITEM_TYPE_SUBMENU, NULL);
    if (mainMenu == NULL) return;
    s_mainMenu = mainMenu;
    Menu_AddChild(root, mainMenu);

    // 创建功能测试菜单项
    MenuItem *funcTest = Menu_CreateItem("1. Function Test", MENU_ITEM_TYPE_SUBMENU, NULL);
    if (funcTest == NULL) return;
    s_funcTestMenu = funcTest;
    Menu_AddChild(mainMenu, funcTest);

    // 创建功能测试子项
    MenuItem *adxl345Test = Menu_CreateItem("1. ADXL345 Test", MENU_ITEM_TYPE_FUNCTION, Test_ADXL345);
    Menu_AddChild(funcTest, adxl345Test);

    MenuItem *oledTest = Menu_CreateItem("2. OLED Test", MENU_ITEM_TYPE_FUNCTION, Test_OLED);
    Menu_AddChild(funcTest, oledTest);

    MenuItem *ds18b20Test = Menu_CreateItem("3. DS18B20 Test", MENU_ITEM_TYPE_FUNCTION, Test_DS18B20);
    Menu_AddChild(funcTest, ds18b20Test);

    MenuItem *usartTest = Menu_CreateItem("4. USART Test", MENU_ITEM_TYPE_FUNCTION, Test_USART);
    Menu_AddChild(funcTest, usartTest);

    MenuItem *keyTest = Menu_CreateItem("5. KEY Test", MENU_ITEM_TYPE_FUNCTION, Test_KEY);
    Menu_AddChild(funcTest, keyTest);

    // 创建工作模式菜单项
    MenuItem *workMode = Menu_CreateItem("2. Work Mode", MENU_ITEM_TYPE_FUNCTION, Run_Work_Mode);
    Menu_AddChild(mainMenu, workMode);

    // 创建关于菜单项
    MenuItem *about = Menu_CreateItem("3. About Me", MENU_ITEM_TYPE_FUNCTION, Show_About_Info);
    Menu_AddChild(mainMenu, about);

    // 设置当前菜单为主菜单
    Menu_NavigateTo(&g_menuManager, mainMenu);
    Menu_ClearHistory(&g_menuManager);
    Menu_RefreshDisplayState(&g_menuManager);
}

// 兼容性函数：菜单选择
void Choose_menu(void)
{
    // 如果菜单系统未初始化，则初始化
    if (g_menuManager.root == NULL) {
        OLED_Menu_Init();
    }

    // 将按键事件转换为菜单事件
    uint8_t key = Key_GetNum();
    if (key != NO_KEY) {
        MenuEvent event = Menu_ConvertKeyToEvent(key);
        Menu_HandleEvent(&g_menuManager, event);
        OLED_Menu_SyncCompatState();
    }

    // 检查是否需要重绘
    if (g_menuManager.needsRedraw) {
        MenuDisplay_DrawMenu(&g_menuManager);
    }

    OLED_Menu_SyncCompatState();
}

// 兼容性函数：功能测试
void function_test(void)
{
    if (flag_adxl345) {
        Test_ADXL345();
    } else if (flag_oled) {
        Test_OLED();
    } else if (flag_ds18b20) {
        Test_DS18B20();
    } else if (flag_usart) {
        Test_USART();
    } else if (flag_key) {
        Test_KEY();
    }
}

// 兼容性函数：工作模式
void Work_Mode(void)
{
    // 这个函数在新系统中不再需要，功能已整合到菜单系统中
    // 保持空实现以保持API兼容性
    // 直接运行工作模式函数
    Run_Work_Mode();
}

// 菜单更新函数（新系统）
void OLED_Menu_Update(void)
{
    Choose_menu();
}

// 检查菜单是否激活
uint8_t OLED_Menu_IsActive(void)
{
    return (g_menuManager.root != NULL);
}

// 直接切换到指定菜单，用于旧显示API，不污染正常导航栈
static void OLED_Menu_SetCurrentMenu(MenuItem *menu, MenuItem *backTarget)
{
    if (menu == NULL) return;

    if (g_menuManager.currentMenu != NULL) {
        g_menuManager.currentMenu->lastSelectedChild = g_menuManager.selectedItem;
    }

    g_menuManager.currentMenu = menu;
    g_menuManager.selectedItem = menu->lastSelectedChild;
    if (g_menuManager.selectedItem == NULL ||
        g_menuManager.selectedItem->parent != menu ||
        !g_menuManager.selectedItem->visible) {
        g_menuManager.selectedItem = Menu_GetFirstVisibleChild(menu);
    }
    menu->lastSelectedChild = g_menuManager.selectedItem;

    Menu_ClearHistory(&g_menuManager);
    if (backTarget != NULL) {
        Menu_PushHistory(&g_menuManager, backTarget);
    }

    g_menuManager.scrollOffset = 0;
    g_menuManager.needsRedraw = 1;
    Menu_RefreshDisplayState(&g_menuManager);
    OLED_Menu_SyncCompatState();
}

static void OLED_Menu_SyncCompatState(void)
{
    uint8_t index = OLED_Menu_GetSelectedIndex();

    if (g_menuManager.currentMenu == s_funcTestMenu) {
        flag_subMenu = 2;
        flag_test = 1;
        Num_Choose = index + 1; // 兼容旧枚举：ADXL345_TEST从2开始
    } else {
        flag_subMenu = 1;
        flag_test = 0;
        Num_Choose = (index == 0) ? 1 : index;
    }
}

static uint8_t OLED_Menu_GetSelectedIndex(void)
{
    if (g_menuManager.currentMenu == NULL || g_menuManager.selectedItem == NULL) {
        return 1;
    }

    uint8_t index = 1;
    MenuItem *item = Menu_GetFirstVisibleChild(g_menuManager.currentMenu);
    while (item != NULL) {
        if (item == g_menuManager.selectedItem) {
            return index;
        }
        index++;
        item = Menu_GetNextVisibleItem(item);
    }

    return 1;
}

static void OLED_Menu_ClearFunctionFlags(void)
{
    flag_adxl345 = 0;
    flag_oled = 0;
    flag_ds18b20 = 0;
    flag_usart = 0;
    flag_key = 0;
}

// === 测试函数实现 ===

// ADXL345测试
static void Test_ADXL345(void)
{
    OLED_Menu_ClearFunctionFlags();
    flag_adxl345 = 1;

    OLED_Clear();
    OLED_Printf(27, 0, OLED_6X8, "ADXL345_TEST");
    OLED_Printf(0, 8, OLED_6X8, "ADXL345_ID:");
    OLED_ShowNum(100, 8, Get_Adxl345_ID(), 2, OLED_6X8);
    OLED_Update();

    // 显示ADXL345数据，直到按下BACK键
    while (1) {
        OLED_ShowNum(0, 24, ADXL345_XTest(), 6, OLED_6X8);
        OLED_ShowNum(0, 32, ADXL345_YTest(), 6, OLED_6X8);
        OLED_ShowNum(0, 40, ADXL345_ZTest(), 6, OLED_6X8);
        OLED_Update();

        if (Key_GetNum() == KEY_BACK) {
            break;
        }

        HAL_Delay(50); // 适当延迟，避免刷新过快
    }

    flag_adxl345 = 0;

    // 返回时标记需要重绘
    g_menuManager.needsRedraw = 1;
}

// OLED测试
static void Test_OLED(void)
{
    OLED_Menu_ClearFunctionFlags();
    flag_oled = 1;

    OLED_Clear();
    OLED_Printf(0, 0, OLED_6X8, "OLED Test Running...");
    OLED_Update();

    // 运行OLED测试
    OLED_Test();

    flag_oled = 0;

    // 返回时标记需要重绘
    g_menuManager.needsRedraw = 1;
}

// DS18B20测试
static void Test_DS18B20(void)
{
    OLED_Menu_ClearFunctionFlags();
    flag_ds18b20 = 1;

    OLED_Clear();
    OLED_Printf(27, 0, OLED_6X8, "DS18B20_TEST");
    OLED_Printf(0, 16, OLED_6X8, "DS18B20 initing...");
    OLED_Update();

    // 初始温度显示
    float temperature = (float)DS18B20_Get_Temperature() / 10.0f;
    OLED_ShowFloatNum(0, 16, temperature, 2, 1, OLED_6X8);
    HAL_Delay(1000);

    OLED_ClearArea(0, 16, 128, 8);

    // 持续显示温度，直到按下BACK键
    while (1) {
        temperature = (float)DS18B20_Get_Temperature() / 10.0f;
        OLED_ShowFloatNum(0, 16, temperature, 2, 1, OLED_6X8);
        OLED_Update();

        if (Key_GetNum() == KEY_BACK) {
            break;
        }

        HAL_Delay(1000); // 1秒更新一次
    }

    flag_ds18b20 = 0;

    // 返回时标记需要重绘
    g_menuManager.needsRedraw = 1;
}

// USART测试
static void Test_USART(void)
{
    OLED_Menu_ClearFunctionFlags();
    flag_usart = 1;

    OLED_Clear();
    OLED_Printf(27, 0, OLED_6X8, "USART_TEST");
    OLED_Printf(0, 10, OLED_6X8, "USART BAUD:");
    OLED_ShowNum(70, 10, 115200, 6, OLED_6X8);
    OLED_Printf(0, 20, OLED_6X8, "USART_Send Running...");
    OLED_Printf(0, 30, OLED_6X8, "Send data: Hello ZYZ!");
    OLED_Printf(0, 40, OLED_6X8, "Press BACK to exit");
    OLED_Update();

    // 发送数据，直到按下BACK键
    while (1) {
        printf("Hello, I'm ZYZ!\r\n");

        if (Key_GetNum() == KEY_BACK) {
            break;
        }

        HAL_Delay(1000); // 1秒发送一次
    }

    flag_usart = 0;

    // 返回时标记需要重绘
    g_menuManager.needsRedraw = 1;
}

// 按键测试
static void Test_KEY(void)
{
    OLED_Menu_ClearFunctionFlags();
    flag_key = 1;

    OLED_Clear();
    OLED_Printf(0, 0, OLED_6X8, "Test BACK_KEY?");
    OLED_Printf(0, 8, OLED_6X8, "Press it!");
    OLED_Update();

    // 显示按键状态，直到按下BACK键
    while (1) {
        OLED_ShowNum(63, 24, HAL_GPIO_ReadPin(KEY_UP_GPIO_Port, KEY_UP_Pin), 1, OLED_8X16);
        OLED_ShowNum(63, 48, HAL_GPIO_ReadPin(KEY_DOWN_GPIO_Port, KEY_DOWN_Pin), 1, OLED_8X16);
        OLED_ShowNum(31, 31, HAL_GPIO_ReadPin(KEY_ENTER_GPIO_Port, KEY_ENTER_Pin), 1, OLED_8X16);
        OLED_ShowNum(95, 31, HAL_GPIO_ReadPin(KEY_BACK_GPIO_Port, KEY_BACK_Pin), 1, OLED_8X16);
        OLED_Update();

        if (Key_GetNum() == KEY_BACK) {
            break;
        }

        HAL_Delay(50); // 适当延迟
    }

    flag_key = 0;

    // 返回时标记需要重绘
    g_menuManager.needsRedraw = 1;
}

// 工作模式
static void Run_Work_Mode(void)
{
    OLED_Clear();
    OLED_Printf(0, 0, OLED_6X8, "Work Mode Running...");
    OLED_Printf(0, 8, OLED_6X8, "Press BACK to exit");
    OLED_Update();

    ADXL345_Calibrate(500); // 进入工作模式时校准一次

    // 运行信号处理，直到按下BACK键
    while (1) {
        MyDSP_Process();         // 运行DSP处理

        if (Key_GetNum() == KEY_BACK) {
            break;
        }

        HAL_Delay(100); // 适当延迟
    }

    // 返回时标记需要重绘
    g_menuManager.needsRedraw = 1;
}

// 关于信息
static void Show_About_Info(void)
{
    OLED_Clear();
    OLED_Printf(0, 20, OLED_6X8, "Autor: Zhang Yizhe");
    OLED_Printf(0, 40, OLED_6X8, "DesignFor25~Graduation");
    OLED_Update();

    // 等待按键返回
    while (Key_GetNum() != KEY_BACK) {
        HAL_Delay(50);
    }

    // 返回时标记需要重绘
    g_menuManager.needsRedraw = 1;
}

// ==============================================
// 包含新菜单系统的核心实现文件
// 这种方式避免需要单独编译多个源文件
// ==============================================

// 包含菜单核心实现
#define MENU_CORE_IMPLEMENTATION
#include "OLED_Menu/menu_core.c"
#undef MENU_CORE_IMPLEMENTATION

// 包含菜单导航实现
#define MENU_NAV_IMPLEMENTATION
#include "OLED_Menu/menu_nav.c"
#undef MENU_NAV_IMPLEMENTATION

// 包含菜单显示实现
#define MENU_DISPLAY_IMPLEMENTATION
#include "OLED_Menu/menu_display.c"
#undef MENU_DISPLAY_IMPLEMENTATION
