#include "My_FreeRTOS.h"
#include "FreeRTOS.h"
#include "task.h"
#include "LED.h"
#include "Key.h"
#include "UART.h"
#include "ESP_12F.h"

// 动画索引
volatile uint8_t Current_Anim_Index = 0;
uint32_t Mode2_Start_Tick = 0;
uint8_t Current_Scene_Mode_2 = 0;
TimeAnim_Handle_t ClockAnimHandle = {0};
// 0: 时钟页
volatile uint8_t Current_Scene_Mode = 0;
// 场景页子模式
volatile uint8_t Current_Scene_Mode_1 = 0;
// 亮度
uint8_t luminance_set = 0;
unsigned char fan = 20;
AHT20_Data_t AHT20_data;

static RGB_Color_t Screen_Old[PIXEL_MAX];
static RGB_Color_t Screen_New[PIXEL_MAX];

static uint8_t Last_Mode = 0;
static uint8_t Last_Mode_1 = 0;
static uint8_t Last_Mode_2 = 0;
static uint8_t Last_Anim_Index = 0;

static uint16_t Get_Index_Local(uint16_t x, uint16_t y)
{
    if (x >= 32 || y >= 8)
        return 0xFFFF;

    // 按灯板走线映射坐标
    if (x % 2 == 0)
        return x * 8 + y;
    else
        return x * 8 + (7 - y);
}

void Draw_Scene_Content(uint32_t tick)
{
    if (Current_Scene_Mode == 0)
    {
        RTC_TimeTypeDef gTime = {0};
        RTC_DateTypeDef gDate = {0};
        HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BIN);
        WS2812_DrawTime_Final_Clipped(&ClockAnimHandle, gTime.Hours, gTime.Minutes, gTime.Seconds, 8, 1, COLOR_MAGENTA, COLOR_PINK, COLOR_ORANGE, COLOR_CYAN, tick, 400);
        WS2812_show_calendar_or_week(0, 0, gDate.WeekDay);
    }
    else if (Current_Scene_Mode == 1)
    {
        char str[20];
        switch (Current_Scene_Mode_1)
        {
        case 1: // 风扇
            WS2812_DrawFan_Animate(2, 0, COLOR_CYAN, COLOR_BLACK, tick, 100);
            WS2812_DrawLine(10, 2, 10, 3, COLOR_GREEN);
            WS2812_DrawLine(10, 5, 10, 6, COLOR_GREEN);
            sprintf(str, "%d", fan);
            WS2812_ShowString(12, 1, str, COLOR_GREEN, COLOR_BLACK);
            break;
        case 2: // 天气
            WS2812_ShowWeather_Scene(CurrentWeather.code, CurrentWeather.temperature, tick);
            break;
        case 3: // 温湿度
            WS2812_Show_Env_Scene(AHT20_data.Temperature, AHT20_data.Humidity, tick);
            break;
        case 4: // 亮度
            WS2812_Show_Brightness_Scene(luminance_set);
            break;
        }
    }
    else if (Current_Scene_Mode == 2)
    {
        if (Current_Scene_Mode_2 == 0)
        {
            WS2812_Show_Animation_Scene(Current_Anim_Index, tick);
        }
        else
        {
            RTC_TimeTypeDef gTime = {0};
            RTC_DateTypeDef gDate = {0};
            HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
            HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BIN);
            WS2812_DrawTime_Final_Clipped(&ClockAnimHandle, gTime.Hours, gTime.Minutes, gTime.Seconds, 8, 1, COLOR_MAGENTA, COLOR_PINK, COLOR_ORANGE, COLOR_CYAN, tick, 400);
            WS2812_show_calendar_or_week(0, 0, gDate.WeekDay);
            WS2812_DrawPoint(31, 7, COLOR_BLUE);
        }
    }
}

#define Start_Task_Stack 256
#define Start_Task_Priority 1
TaskHandle_t Start_Task_Handle;
void start_task(void *pvParameters); // 启动任务
void freertos_start(void)
{
    xTaskCreate((TaskFunction_t)start_task,
                (char *)"start_task",
                (configSTACK_DEPTH_TYPE)Start_Task_Stack,
                (void *)NULL,
                (UBaseType_t)Start_Task_Priority,
                (TaskHandle_t *)&Start_Task_Handle);
    vTaskStartScheduler();
}

#define boot_Toggle_led_Task_Stack 48
#define boot_Toggle_led_Priority 1
TaskHandle_t boot_Toggle_led_Task_Handle;
void boot_Toggle_led_Task(void *pvParameters);
#define print_information_Task_Stack 256
#define print_information_Task_Priority 1
TaskHandle_t print_information_Task_Handle;
void print_information_Task(void *pvParameters);
#define temperature_humidity_read_Task_Stack 256
#define temperature_humidity_read_Task_Priority 2
TaskHandle_t temperature_humidity_read_Task_Handle;
void temperature_humidity_read_Task_Task(void *pvParameters);
#define ESP12F_init_Task_Stack 256
#define ESP12F_init_Task_Priority 8
TaskHandle_t ESP12F_init_Task_Handle;
void ESP12F_init_Task_Task(void *pvParameters);
#define refresh_ws28121b_data_task_Stack 128
#define refresh_ws28121b_data_task_Priority 7
TaskHandle_t refresh_ws28121b_data_Handle;
void refresh_ws28121b_data_task(void *pvParameters);
#define key_pool_task_Stack 128
#define key_pool_task_Priority 1
TaskHandle_t key_pool_Handle;
void key_pool_task(void *pvParameters);
#define Show_other_scenes_task_Stack 512
#define Show_other_scenes_Priority 8
TaskHandle_t Show_other_scenes_Handle;
void Show_other_scenes_task(void *pvParameters);
#define Fan_Control_task_Stack 128
#define Fan_Control_Priority 3
TaskHandle_t Fan_Control_Handle;
void Fan_Control_task(void *pvParameters);
// 启动任务


void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();
    xTaskCreate((TaskFunction_t)boot_Toggle_led_Task,
                (char *)"boot_Toggle_led_Task",
                (configSTACK_DEPTH_TYPE)boot_Toggle_led_Task_Stack,
                (void *)NULL,
                (UBaseType_t)boot_Toggle_led_Priority,
                (TaskHandle_t *)&boot_Toggle_led_Task_Handle);
    xTaskCreate((TaskFunction_t)print_information_Task,
                (char *)"print_information_Task",
                (configSTACK_DEPTH_TYPE)print_information_Task_Stack,
                (void *)NULL,
                (UBaseType_t)print_information_Task_Priority,
                (TaskHandle_t *)&print_information_Task_Handle);
    xTaskCreate((TaskFunction_t)temperature_humidity_read_Task_Task,
                (char *)"print_information_Task",
                (configSTACK_DEPTH_TYPE)temperature_humidity_read_Task_Stack,
                (void *)NULL,
                (UBaseType_t)temperature_humidity_read_Task_Priority,
                (TaskHandle_t *)&temperature_humidity_read_Task_Handle);
    xTaskCreate((TaskFunction_t)ESP12F_init_Task_Task,
                (char *)"ESP12F_init_Task",
                (configSTACK_DEPTH_TYPE)ESP12F_init_Task_Stack,
                (void *)NULL,
                (UBaseType_t)ESP12F_init_Task_Priority,
                (TaskHandle_t *)&ESP12F_init_Task_Handle);
    xTaskCreate((TaskFunction_t)refresh_ws28121b_data_task,
                (char *)"refresh_ws28121b_data_task",
                (configSTACK_DEPTH_TYPE)refresh_ws28121b_data_task_Stack,
                (void *)NULL,
                (UBaseType_t)refresh_ws28121b_data_task_Priority,
                (TaskHandle_t *)&refresh_ws28121b_data_Handle);
    xTaskCreate((TaskFunction_t)key_pool_task,
                (char *)"key_pool_task",
                (configSTACK_DEPTH_TYPE)key_pool_task_Stack,
                (void *)NULL,
                (UBaseType_t)key_pool_task_Priority,
                (TaskHandle_t *)&key_pool_Handle);
    xTaskCreate((TaskFunction_t)Show_other_scenes_task,
                (char *)"Show_other_scenes_task",
                (configSTACK_DEPTH_TYPE)Show_other_scenes_task_Stack,
                (void *)NULL,
                (UBaseType_t)Show_other_scenes_Priority,
                (TaskHandle_t *)&Show_other_scenes_Handle);
    xTaskCreate((TaskFunction_t)Fan_Control_task,
                (char *)"Fan_Control_task",
                (configSTACK_DEPTH_TYPE)Fan_Control_task_Stack,
                (void *)NULL,
                (UBaseType_t)Fan_Control_Priority,
                (TaskHandle_t *)&Fan_Control_Handle);

    taskEXIT_CRITICAL();

    vTaskDelay(pdMS_TO_TICKS(100));
    vTaskDelete(NULL);
}
// 开机闪灯




void boot_Toggle_led_Task(void *pvParameters)
{
    while (1)
    {
        boot_toggle_green_led();
        vTaskDelay(1000);
    }
}
// 调试打印




void print_information_Task(void *pvParameters)
{
    while (1)
    {
        // 任务调试信息
        // char task_list[200];
        // vTaskList(task_list);
        // U1_print("task_list:%s\r\n", task_list);
        // UBaseType_t surplus_size = uxTaskGetStackHighWaterMark(ESP12F_init_Task_Handle);
        // U1_print("led_surplus_size:%d\r\n", surplus_size);
        vTaskDelay(5000);
    }
}
// 温湿度采样




void temperature_humidity_read_Task_Task(void *pvParameters)
{
    while (1)
    {
        AHT20_Read_Measure(&AHT20_data);
        U1_print("humidity:%f\r\n", AHT20_data.Humidity);
        U1_print("temperature:%f\r\n", AHT20_data.Temperature);
        xTaskNotifyGive(Fan_Control_Handle);
        vTaskDelay(5000);
    }
}
// 时间同步任务
#define get_esp12f_time_task_Stack 256
#define get_esp12f_time_task_Priority 5
TaskHandle_t get_esp12f_time_Handle;
void get_esp12f_time_task(void *pvParameters);
// 天气更新任务
#define get_esp12f_weather_task_Stack 256
#define get_esp12f_weather_task_Priority 5
TaskHandle_t get_esp12f_weather_Handle;
void get_esp12f_weather_task(void *pvParameters);
QueueHandle_t WiFi_connection_statusBinary_Handle; // WiFi 连接状态
SemaphoreHandle_t ESP8266_Mutex_Handle = NULL;     // ESP8266 互斥锁
void ESP12F_init_Task_Task(void *pvParameters)
{

    WiFi_connection_statusBinary_Handle = xQueueCreate(1, sizeof(unsigned char));
    ESP8266_Mutex_Handle = xSemaphoreCreateMutex();
    if (ESP8266_Mutex_Handle == NULL)
    {
        U1_print("Error: ESP8266 Mutex Create Failed!\r\n");
    }
    unsigned char WiFi_connection_flag = 0;
    ESP8266_Init_Setup(); // 初始化 ESP8266
    if (ESP8266_kill_WiFi())
    { // 断开当前 WiFi
        U1_print("Kill_wifi_ok\r\n");
    }
    ESP8266_scan_wifi();
    for (unsigned char i = 0; i < WifiListCount; i++)
    {
        U1_print("wifi_name%d:%s\r\n", i, WifiList[i].ssid);
    }

    if (ESP8266_ConnectWiFi("Pi", "88888888")) // 连接 WiFi
    {
        U1_print("wife_online\r\n");
        xTaskCreate((TaskFunction_t)get_esp12f_time_task,
                    (char *)"get_esp12f_time_task",
                    (configSTACK_DEPTH_TYPE)get_esp12f_time_task_Stack,
                    (void *)NULL,
                    (UBaseType_t)get_esp12f_time_task_Priority,
                    (TaskHandle_t *)&get_esp12f_time_Handle);
        xTaskCreate((TaskFunction_t)get_esp12f_weather_task,
                    (char *)"get_esp12f_weather_task",
                    (configSTACK_DEPTH_TYPE)get_esp12f_weather_task_Stack,
                    (void *)NULL,
                    (UBaseType_t)get_esp12f_weather_task_Priority,
                    (TaskHandle_t *)&get_esp12f_weather_Handle);
        WiFi_connection_flag = 1;
    }
    else
    {
        ;
    }
    xQueueSend(WiFi_connection_statusBinary_Handle, &WiFi_connection_flag, 0xfffff);
    vTaskDelete(NULL); // 初始化结束后删除自身
}

// 定时同步时间



void get_esp12f_time_task(void *pvParameters)
{
    time_t ntp_timestamp = 0;
    while (1)
    {
        // 先拿串口互斥锁
        if (xSemaphoreTake(ESP8266_Mutex_Handle, pdMS_TO_TICKS(10000)) == pdTRUE)
        {
            U1_print("[Time Task] Got Mutex, Starting NTP...\r\n");

            // 取 NTP 时间
            ntp_timestamp = ESP8266_Get_NTP_Time();

            // 释放互斥锁
            xSemaphoreGive(ESP8266_Mutex_Handle);
            U1_print("[Time Task] Released Mutex.\r\n");

            // 更新时间
            if (ntp_timestamp > 0)
            {
                U1_print("NTP Success! Timestamp: %ld\r\n", ntp_timestamp);
                RTC_Set_Time_From_NTP(&now_time);
                HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, RTC_BKP_DR_VALID);
            }
            else
            {
                U1_print("NTP Failed.\r\n");
            }
        }
        else
        {
            U1_print("[Time Task] Failed to get Mutex (Timeout)\r\n");
        }

        vTaskDelay(pdMS_TO_TICKS(1800000)); // 30 分钟
    }
}

// 定时更新天气


void get_esp12f_weather_task(void *pvParameters)
{
    // 先错开启动时间
    vTaskDelay(pdMS_TO_TICKS(5000));
    // 预留首轮启动标记
    // static uint8_t is_first_boot = 1;
    while (1)
    {
        // 先拿串口互斥锁
        if (xSemaphoreTake(ESP8266_Mutex_Handle, pdMS_TO_TICKS(10000)) == pdTRUE)
        {
            U1_print("[Weather Task] Got Mutex, Connecting TCP...\r\n");

            // 建 TCP 连接
            if (ESP8266_Connect_tcp("api.seniverse.com", "80"))
            {
                U1_print("TCP Connected. Sending Request...\r\n");

                // 发天气请求并解析
                if (ESP8266_send_tcp())
                {
                    U1_print("--- Weather Data ---\r\n");
                    U1_print("City: %s\r\n", CurrentWeather.name);
                    U1_print("Weather_code: %s\r\n", CurrentWeather.code);
                    U1_print("Weather: %s\r\n", CurrentWeather.weather);
                    U1_print("Temp: %s C\r\n", CurrentWeather.temperature);
                    U1_print("Update: %ld\r\n", CurrentWeather.up_data_time);
                    U1_print("--------------------\r\n");
                }
                else
                {
                    U1_print("Weather Request Failed.\r\n");
                }

                uint8_t last_time_mode = Current_Scene_Mode;
                uint8_t last_time_mode_1 = Current_Scene_Mode_1;
                uint8_t last_time_mode_2 = Current_Scene_Mode_2;
                // 切到天气页
                Current_Scene_Mode = 1;   // 场景页
                Current_Scene_Mode_1 = 2; // 天气

                // 保持显示 5 秒
                vTaskDelay(pdMS_TO_TICKS(5000));

                // 如果期间没切页就恢复
                // 还在天气页就切回原页面
                if (Current_Scene_Mode == 1 && Current_Scene_Mode_1 == 2)
                {
                    Current_Scene_Mode = last_time_mode; // 恢复上一个页面
                    Current_Scene_Mode_1 = last_time_mode_1;
                    Current_Scene_Mode_2 = last_time_mode_2;
                }
            }
            else
            {
                U1_print("TCP Connect Failed.\r\n");
            }

            // 释放互斥锁
            xSemaphoreGive(ESP8266_Mutex_Handle);
            U1_print("[Weather Task] Released Mutex.\r\n");
        }
        else
        {
            U1_print("[Weather Task] Failed to get Mutex (Timeout)\r\n");
        }

        vTaskDelay(pdMS_TO_TICKS(1800000)); // 30 分钟
    }
}

// 刷新灯板显示
void refresh_ws28121b_data_task(void *pvParameters)
{
    unsigned char WiFi_connection_flag = 0;
    WS2812_Clear();
    WS2812_ShowString(0, 0, "WIFI", COLOR_BLUE, COLOR_BLACK);
    WS2812_DrawImage(24, 0, 8, 8, Icon_arrowhead, COLOR_RED);
    WS2812_Update();
    xQueueReceive(WiFi_connection_statusBinary_Handle, &WiFi_connection_flag, 10000);
    if (WiFi_connection_flag == 1)
    {
        WS2812_Clear();
        WS2812_ShowString(0, 0, "WIFI", COLOR_BLUE, COLOR_BLACK);
        WS2812_DrawImage(24, 0, 8, 8, Icon_correct, COLOR_GREEN);
        WS2812_Update();
        vTaskDelay(1500);
    }
    else if (WiFi_connection_flag == 0)
    {
        WS2812_Clear();
        WS2812_ShowString(0, 0, "WIFI", COLOR_BLUE, COLOR_BLACK);
        WS2812_DrawImage(24, 0, 8, 8, Icon_Wrong, COLOR_RED);
        WS2812_Update();
        vTaskDelay(1500);
    }

    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(15);
    xLastWakeTime = xTaskGetTickCount();

    // 记录上一次状态
    Last_Mode = Current_Scene_Mode;
    Last_Mode_1 = Current_Scene_Mode_1;
    Last_Mode_2 = Current_Scene_Mode_2;
    Last_Anim_Index = Current_Anim_Index;

    for (;;)
    {
        uint32_t tick = HAL_GetTick();

        // 状态切换时做过渡


        if (Current_Scene_Mode != Last_Mode ||
            Current_Scene_Mode_1 != Last_Mode_1 ||
            Current_Scene_Mode_2 != Last_Mode_2 ||
            (Current_Scene_Mode == 2 && Current_Anim_Index != Last_Anim_Index))
        {
            // 备份旧画面
            memcpy(Screen_Old, PixelBuffer, sizeof(PixelBuffer));

            // 生成新画面
            WS2812_Clear();
            Draw_Scene_Content(tick);
            memcpy(Screen_New, PixelBuffer, sizeof(PixelBuffer));

            // 上滑过渡
            for (int offset = 1; offset <= 8; offset++)
            {
                WS2812_Clear();

                // 遍历整屏
                for (int x = 0; x < 32; x++)
                {
                    for (int y = 0; y < 8; y++)
                    {
                        // 计算旧画面和新画面对应位置



                        int y_old_src = y + offset;
                        int y_new_src = y - (8 - offset);

                        // 旧画面还在显示区
                        if (y_old_src < 8)
                        {
                            uint16_t idx = Get_Index_Local(x, y_old_src);
                            WS2812_DrawPoint(x, y, Screen_Old[idx]);
                        }
                        // 新画面开始进入
                        else if (y_new_src >= 0)
                        {
                            uint16_t idx = Get_Index_Local(x, y_new_src);
                            WS2812_DrawPoint(x, y, Screen_New[idx]);
                        }
                    }
                }
                WS2812_Update();
                vTaskDelay(50); // 过渡速度
            }

            // 更新状态记录
            Last_Mode = Current_Scene_Mode;
            Last_Mode_1 = Current_Scene_Mode_1;
            Last_Mode_2 = Current_Scene_Mode_2;
            Last_Anim_Index = Current_Anim_Index;
        }

        // 常规刷新



        // 动画页和时钟页切换
        if (Current_Scene_Mode == 2)
        {
            if (Current_Scene_Mode_2 == 0) // 动画页
            {
                if (tick - Mode2_Start_Tick >= 60000)
                {
                    Current_Scene_Mode_2 = 1; // 切到时钟
                    Mode2_Start_Tick = tick;
                }
            }
            else // 时钟页
            {
                if (tick - Mode2_Start_Tick >= 5000)
                {
                    Current_Scene_Mode_2 = 0; // 切回动画
                    Mode2_Start_Tick = tick;
                }
            }
        }

        // 正常刷新
        WS2812_Clear();
        Draw_Scene_Content(tick);
        WS2812_Update();

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// 按键轮询
void key_pool_task(void *argument)
{
    // 驱动里一直轮询
    BSP_Key_Poll_Task(NULL);
}
// 按键切场景
void Show_other_scenes_task(void *pvParameters)
{
    Key_ID_t key;
    for (;;)
    {
        // 等按键消息
        if (BSP_Key_GetMessage(&key, portMAX_DELAY) == pdTRUE)
        {
            U1_print("Key Pressed: %d\r\n", key);

            switch (key)
            {
            case KEY_PC1:
                Current_Scene_Mode = 1;
                Current_Scene_Mode_1++;
                Current_Scene_Mode_1 = (Current_Scene_Mode_1 >= 4) ? 4 : Current_Scene_Mode_1;
                U1_print("Switch to Heart Mode\r\n");
                break;

            case KEY_PC2:
                Current_Scene_Mode = 1; // 切到场景页
                Current_Scene_Mode_1 = (Current_Scene_Mode_1 - 1 >= 0) ? (Current_Scene_Mode_1 - 1) : Current_Scene_Mode_1;
                Current_Scene_Mode_1 = (Current_Scene_Mode_1 <= 1) ? 1 : Current_Scene_Mode_1;
                break;
            case KEY_PC3:
                if (Current_Scene_Mode == 2)
                {
                    Current_Anim_Index++;
                    if (Current_Anim_Index > 4)
                        Current_Anim_Index = 0;

                    // 切动画时重置计时
                    Mode2_Start_Tick = HAL_GetTick();
                    Current_Scene_Mode_2 = 0; // 强制回到动画
                }
                else
                {
                    // 从别的页面切进来
                    Current_Scene_Mode = 2;
                    Current_Anim_Index = 0;           // 默认第一组动画
                    Mode2_Start_Tick = HAL_GetTick(); // 重置计时
                    Current_Scene_Mode_2 = 0;         // 先显示动画
                }
                break;
            case KEY_PC4:               
                Current_Scene_Mode = 0; // 回到时钟
                uint8_t data;
                AT24C512_ReadBuffer(0x0010, &data, 1);
                if (data != luminance_set)
                {
                    AT24C512_WriteBuffer(0x0010, &luminance_set, 1);
                    // 保存亮度
                }
                U1_print("Back to Clock Mode\r\n");
                break;
            case KEY_ENC_CW: // 顺时针
                switch (Current_Scene_Mode_1)
                {
                case 1:
                    fan = (fan - 1 >= 0) ? (fan - 1) : fan;
                    fan = (fan <= 20) ? 20 : fan;
                    xTaskNotifyGive(Fan_Control_Handle);
                    break;
                case 4:
                    luminance_set = (luminance_set - 1 >= 0) ? (luminance_set - 1) : luminance_set;
                    luminance_set = (luminance_set <= 0) ? 0 : luminance_set;
                    break;
                default:
                    break;
                }
                break;
            case KEY_ENC_CCW: // 逆时针
                switch (Current_Scene_Mode_1)
                {
                case 1:
                    fan++;
                    fan = (fan >= 100) ? 100 : fan;
                    xTaskNotifyGive(Fan_Control_Handle);
                    break;
                case 4:
                    luminance_set++;
                    luminance_set = (luminance_set >= 100) ? 100 : luminance_set;
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }
    }
}

// 温控策略
uint8_t Calculate_Auto_Fan_Speed(float temp)
{
    if (temp < 30.0f)
        return 0; // 低温停转
    if (temp > 60.0f)
        return 100; // 高温全速

    // 30~60 度线性调整
    float result = 20.0f + (temp - 30.0f) * 2.0f;
    return result;
}

// 风扇控制
void Fan_Control_task(void *pvParameters)
{
    Fan_Set_Speed(0);
    while (1)
    {
        // 等待通知
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // 判断当前控制方式

        // 场景页下手动控制
        if (Current_Scene_Mode == 1 && Current_Scene_Mode_1 == 1)
        {
            Fan_Set_Speed(fan); // 直接用当前风速
        }
        // 其他页面走自动温控
        else
        {
            // 用当前温度算风速
            uint8_t auto_speed = Calculate_Auto_Fan_Speed(AHT20_data.Temperature);
            Fan_Set_Speed(auto_speed);
        }
    }
}
