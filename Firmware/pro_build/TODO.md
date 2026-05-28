# OLED菜单系统重构 - 当前 TODO

更新时间：2026-05-10

## 已完成

- [x] 收敛菜单重构主线到当前工程使用的 `MDK-ARM/User/OLED_Menu.c` 和 `MDK-ARM/User/OLED_Menu/` 模块。
- [x] 为 `menu_core.c`、`menu_nav.c`、`menu_display.c` 增加真正可防重复包含的实现保护宏。
- [x] 检查并整理菜单头文件依赖，避免宏重复定义造成后续维护混乱。
- [x] 修复菜单返回栈逻辑，避免主菜单 BACK 退到内部 Root 菜单。
- [x] 增加每级菜单的选中项记忆，返回上级菜单后恢复之前选中的项。
- [x] 修复 64px OLED 上 `MENU_MAX_VISIBLE_ITEMS = 6` 造成的显示越界，当前按 16px 标题 + 3 行菜单显示。
- [x] 调整选中项绘制顺序，先写文本再反色，保证选中项可读。
- [x] 恢复旧兼容变量同步：`Num_Choose`、`flag_subMenu`、`flag_test`、各模块测试 flag。
- [x] 保留旧 API：`Choose_menu()`、`function_test()`、`Work_Mode()`、`display_main_menu()`、`display_sub_menu()`、`display_about_me()`。
- [x] 修复 DSP 集成里的高风险问题：
  - FFT 输出缓冲区由 `LENGTH_SAMPLE/2+1` 改为 `LENGTH_SAMPLE`，避免 CMSIS RFFT 写越界。
  - 三轴 FIR 使用独立 state，避免 X/Y/Z 轴滤波状态互相污染。
  - FIR 初始化 blockSize 改回 `BLOCK_SIZE`，避免初始化过程越界清零。
  - 峰值检测去掉 VLA，改为固定静态数组，降低栈风险。
  - 频率步进改为浮点计算，避免 `3200/1024` 被整数截断。
  - 默认关闭每阶段 1024 点原始数据串口刷屏，保留摘要输出。
- [x] 清理未接入工程且容易误导阅读的重构残留：
  - `OLED_Menu_old.c/h`
  - `OLED_Menu_new.c/h`
  - `TEST_DSP.c/h`

## 已验证

- [x] 使用 ARMCC 5.06 单独编译以下关键文件通过：
  - `MDK-ARM/User/OLED_Menu.c`
  - `MDK-ARM/User/MyDSP.C`
  - `Core/Src/main.c`
  - `MDK-ARM/User/OLED.c`
  - `MDK-ARM/User/ADXL345.c`
  - `MDK-ARM/User/ds18b20.c`

## 未完成 / 需要硬件或 IDE 验证

- [ ] 在 Keil uVision 或 EIDE 中执行完整 Rebuild，确认链接阶段也通过。
  - 本次尝试调用 `UV4.exe` 命令行接口，但它没有生成日志，也没有更新 `axf`，所以不能把它算作完整构建验证。
- [ ] 上板验证 OLED 菜单显示：
  - 主菜单显示 3 项：Function Test / Work Mode / About Me。
  - Function Test 子菜单显示 5 项，超过 3 项时可以滚动。
  - UP/DOWN 选择、ENTER 进入、BACK 返回。
  - 从子菜单返回后记住之前选中的项。
- [ ] 上板验证各模块测试：
  - ADXL345
  - OLED
  - DS18B20
  - USART
  - KEY
- [ ] 上板验证 Work Mode：
  - 能进入 DSP 处理。
  - OLED 能显示频谱摘要。
  - BACK 能在一次 DSP 处理完成后退出。
  - 串口输出量不会明显拖慢系统。

## 后续建议

- Work Mode 目前仍是“采样 + FIR + 加窗 + FFT + 摘要显示”的演示型流程，还没有形成稳定的故障诊断判据。
- `HAL_TIM_PeriodElapsedCallback()` 里每 5 秒调用 `ADXL345_TEMP_Calibrate()`，该函数可能比较重，建议后续评估是否适合放在中断上下文。
- 如果后续要继续增强菜单，建议只扩展 `OLED_Menu.c` 的菜单构建部分，不再新建 `OLED_Menu_new/old` 这类平行实现。
