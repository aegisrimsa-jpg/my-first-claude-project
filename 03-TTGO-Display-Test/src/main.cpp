/*
 * ============================================================
 *  TTGO T-Display 螢幕與按鍵測試程式
 * ============================================================
 *
 *  硬體平台：LilyGO TTGO T-Display v1.1
 *    - 主控晶片：ESP32-D0WDQ6 (雙核 240MHz, Wi-Fi + BLE)
 *    - 螢幕控制器：ST7789V, 1.14 吋 IPS, 解析度 240×135
 *    - Flash：16MB
 *    - USB 轉 UART：CH9102
 *    - 使用者按鍵：2 個（GPIO 0 / GPIO 35，低電位觸發）
 *
 *  功能說明：
 *    1. 螢幕中央顯示 "Hello TTGO!" 文字
 *    2. 螢幕頂部即時顯示兩個按鍵的按壓狀態
 *    3. Button A (GPIO 0)  → 循環切換背景顏色
 *    4. Button B (GPIO 35) → 循環切換文字顏色
 *    5. Serial Monitor 輸出完整的系統診斷與按鍵事件紀錄
 *
 *  開發環境：VS Code + PlatformIO
 *  框架：Arduino (espressif32)
 *  依賴函式庫：TFT_eSPI, Button2
 * ============================================================
 */

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Button2.h>

// ============================================================
//  硬體腳位定義
// ============================================================
//
//  TTGO T-Display 的兩個使用者按鍵：
//    - GPIO 0  (Button A)：位於 USB 孔同側的左邊按鍵
//      注意：GPIO 0 同時也是 ESP32 的 BOOT/STRAPPING 腳位，
//      上電時若被拉低會進入下載模式。正常運作時可當一般輸入使用，
//      板上已有 10kΩ 上拉電阻。
//    - GPIO 35 (Button B)：位於右邊的按鍵
//      注意：GPIO 35 屬於 ESP32 的 input-only 腳位（GPIO 34-39），
//      這些腳位沒有內部上拉/下拉電阻，板上已外接上拉電阻。
//
//  兩個按鍵都是「按下時接地」設計，因此：
//    按下 = LOW，放開 = HIGH
//
#define BUTTON_A_PIN  0
#define BUTTON_B_PIN  35

// ============================================================
//  螢幕參數（橫向模式）
// ============================================================
//
//  ST7789V 原生解析度為 240×320（直向），但 TTGO T-Display
//  的面板實際可視區域只有 240×135。設定 rotation=1 後：
//    寬 = 240 像素，高 = 135 像素，USB 孔朝左
//
#define SCREEN_W  240
#define SCREEN_H  135

// ============================================================
//  系統監控參數
// ============================================================
//
//  HEALTH_INTERVAL_MS：系統健康報告的輸出間隔（毫秒）
//  STATUS_UPDATE_MS：螢幕按鍵狀態的更新間隔，避免過度刷新
//    佔用 SPI 匯流排導致 Button2 的 loop() 被延遲
//
#define HEALTH_INTERVAL_MS  10000
#define STATUS_UPDATE_MS    50

// ============================================================
//  全域物件
// ============================================================

TFT_eSPI tft = TFT_eSPI();   // TFT_eSPI 螢幕驅動物件（腳位由 build_flags 定義）
Button2  btnA;                // 按鍵 A 物件（Button2 函式庫提供消抖與事件管理）
Button2  btnB;                // 按鍵 B 物件

// ============================================================
//  顏色調色盤
// ============================================================
//
//  TFT_eSPI 使用 16-bit RGB565 色彩格式：
//    R:5bit  G:6bit  B:5bit  →  65,536 種顏色
//  以下預定義顏色來自 TFT_eSPI 標頭檔
//
const uint16_t bgColors[] = {
    TFT_BLACK,      // 0x0000 - 黑色（預設）
    TFT_NAVY,       // 0x000F - 深藍
    TFT_DARKGREEN,  // 0x03E0 - 深綠
    TFT_MAROON,     // 0x7800 - 暗紅
    TFT_PURPLE,     // 0x780F - 紫色
    TFT_OLIVE,      // 0x7BE0 - 橄欖綠
    TFT_DARKGREY    // 0x7BEF - 深灰
};

const uint16_t fgColors[] = {
    TFT_GREEN,      // 0x07E0 - 綠色（預設）
    TFT_YELLOW,     // 0xFFE0 - 黃色
    TFT_CYAN,       // 0x07FF - 青色
    TFT_WHITE,      // 0xFFFF - 白色
    TFT_MAGENTA,    // 0xF81F - 洋紅
    TFT_ORANGE,     // 0xFDA0 - 橙色
    TFT_RED         // 0xF800 - 紅色
};

const uint8_t BG_COUNT = sizeof(bgColors) / sizeof(bgColors[0]);
const uint8_t FG_COUNT = sizeof(fgColors) / sizeof(fgColors[0]);

// ============================================================
//  應用程式狀態變數
// ============================================================

uint8_t  bgIndex      = 0;      // 目前背景顏色索引
uint8_t  fgIndex      = 0;      // 目前文字顏色索引
bool     needsRedraw  = true;   // 標記是否需要全螢幕重繪

// --- 按鍵事件統計（用於 Serial Monitor 輸出）---
uint32_t btnACount    = 0;      // Button A 累計觸發次數
uint32_t btnBCount    = 0;      // Button B 累計觸發次數

// --- 計時器 ---
unsigned long lastHealthReport = 0;   // 上次健康報告的時間戳
unsigned long lastStatusUpdate = 0;   // 上次螢幕按鍵狀態更新的時間戳

// ============================================================
//  函式前向宣告
// ============================================================

void printSystemInfo();
void printHealthReport();
void drawScreen();
void drawButtonStatus(bool forceUpdate);
void onButtonAPressed(Button2 &btn);
void onButtonBPressed(Button2 &btn);

// ============================================================
//  printSystemInfo()
//  ────────────────
//  在 Serial Monitor 輸出 ESP32 的硬體資訊與初始化狀態。
//  僅在 setup() 中呼叫一次。
//
//  輸出項目：
//    - ESP32 晶片型號與修訂版本
//    - CPU 核心數與時脈頻率
//    - Flash 大小與速度
//    - 可用堆積記憶體（Heap）
//    - SDK 版本
// ============================================================
void printSystemInfo() {
    Serial.println();
    Serial.println("╔══════════════════════════════════════════╗");
    Serial.println("║    TTGO T-Display 系統資訊              ║");
    Serial.println("╠══════════════════════════════════════════╣");

    // esp_chip_info 提供晶片的詳細硬體資訊
    esp_chip_info_t chip;
    esp_chip_info(&chip);

    Serial.printf("║  晶片型號：ESP32 rev.%d\n", chip.revision);
    Serial.printf("║  CPU 核心：%d 核 @ %d MHz\n", chip.cores, getCpuFrequencyMhz());
    Serial.printf("║  Flash 大小：%d MB (%s)\n",
                  spiFlashGetSize() / (1024 * 1024),
                  (chip.features & CHIP_FEATURE_EMB_FLASH) ? "內建" : "外接");
    Serial.printf("║  可用 Heap：%d bytes (%.1f KB)\n",
                  ESP.getFreeHeap(), ESP.getFreeHeap() / 1024.0);
    Serial.printf("║  最大連續 Heap：%d bytes (%.1f KB)\n",
                  ESP.getMaxAllocHeap(), ESP.getMaxAllocHeap() / 1024.0);
    Serial.printf("║  SDK 版本：%s\n", ESP.getSdkVersion());

    Serial.println("╠══════════════════════════════════════════╣");
    Serial.println("║  螢幕：ST7789V 240x135 (rotation=1)     ║");
    Serial.printf("║  SPI 頻率：%d MHz\n", SPI_FREQUENCY / 1000000);
    Serial.println("║  按鍵 A：GPIO 0  (BOOT, 內建上拉)       ║");
    Serial.println("║  按鍵 B：GPIO 35 (input-only, 外部上拉)  ║");
    Serial.println("╠══════════════════════════════════════════╣");
    Serial.println("║  初始化完成，系統就緒！                  ║");
    Serial.println("╚══════════════════════════════════════════╝");
    Serial.println();
}

// ============================================================
//  printHealthReport()
//  ───────────────────
//  每 HEALTH_INTERVAL_MS（預設 10 秒）輸出一次系統健康報告。
//
//  監控項目：
//    - 系統運行時間（天:時:分:秒）
//    - 可用記憶體與使用率
//    - 按鍵觸發統計
//    - 整體健康評估
//
//  記憶體健康判定標準：
//    - > 100KB 可用 → 正常 (OK)
//    - 50KB ~ 100KB  → 警告 (WARNING)
//    - < 50KB        → 危險 (CRITICAL)
// ============================================================
void printHealthReport() {
    unsigned long uptimeSec = millis() / 1000;
    unsigned long days  = uptimeSec / 86400;
    unsigned long hours = (uptimeSec % 86400) / 3600;
    unsigned long mins  = (uptimeSec % 3600) / 60;
    unsigned long secs  = uptimeSec % 60;

    uint32_t freeHeap    = ESP.getFreeHeap();
    uint32_t totalHeap   = ESP.getHeapSize();
    float    usedPercent  = 100.0f * (1.0f - (float)freeHeap / (float)totalHeap);

    // 健康狀態判定
    const char *healthStatus;
    if (freeHeap > 100000) {
        healthStatus = "OK - 正常";
    } else if (freeHeap > 50000) {
        healthStatus = "WARNING - 記憶體偏低";
    } else {
        healthStatus = "CRITICAL - 記憶體不足！";
    }

    Serial.println("┌──────────── 系統健康報告 ────────────┐");
    Serial.printf("│  運行時間：%lud %02lu:%02lu:%02lu\n", days, hours, mins, secs);
    Serial.printf("│  可用 Heap：%u bytes (%.1f KB)\n", freeHeap, freeHeap / 1024.0);
    Serial.printf("│  記憶體使用率：%.1f%%\n", usedPercent);
    Serial.printf("│  按鍵統計：A=%u 次, B=%u 次\n", btnACount, btnBCount);
    Serial.printf("│  健康狀態：%s\n", healthStatus);
    Serial.println("└──────────────────────────────────────┘");
}

// ============================================================
//  setup()
//  ───────
//  Arduino 框架的初始化函式，開機後執行一次。
//
//  初始化順序：
//    1. Serial（除錯用 UART，115200 baud）
//    2. 背光 GPIO（必須手動開啟，否則螢幕無顯示）
//    3. TFT 螢幕（SPI 初始化 + 方向設定）
//    4. Button2 按鍵（設定腳位、消抖時間、事件回呼）
//    5. 輸出系統資訊
//    6. 首次畫面繪製
// ============================================================
void setup() {
    // --- 1. 初始化 Serial 通訊 ---
    // 115200 baud 是 ESP32 + CH9102 的標準速率
    Serial.begin(115200);
    delay(100);  // 等待 Serial 穩定（CH9102 USB 轉 UART 需要短暫初始化）

    // --- 2. 開啟螢幕背光 ---
    // TFT_BL (GPIO 4) 控制背光 MOSFET，HIGH = 開啟
    // 若不手動設定，螢幕會初始化成功但無法看到畫面
    // 替代方案：可用 analogWrite(TFT_BL, 0~255) 做亮度調節（PWM）
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    // --- 3. 初始化 TFT 螢幕 ---
    // tft.init() 會執行 SPI 初始化 + ST7789V 啟動序列
    // setRotation(1) = 橫向顯示，USB 孔朝左
    //   rotation 值對應：0=直向, 1=橫向, 2=直向翻轉, 3=橫向翻轉
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(bgColors[bgIndex]);

    // --- 4. 初始化按鍵 ---
    //
    // 【重要】按鍵反應不靈敏的根本原因與修復：
    //
    //   原始程式使用 setClickHandler()，此回呼需要完成完整的
    //   「按下 → 放開 → 等待雙擊逾時」流程才會觸發。
    //   Button2 預設雙擊偵測逾時約 300ms，導致：
    //     a) 每次按鍵至少延遲 300ms 才觸發回呼
    //     b) drawScreen() 中的 fillScreen() 是耗時的 SPI 阻塞操作，
    //        執行期間 btnX.loop() 無法被呼叫，Button2 的內部計時器
    //        會漏掉狀態轉換，導致「需要多按幾下」的現象
    //
    //   解決方案（本程式採用的策略）：
    //     1. 改用 setPressedHandler() — 按下瞬間立即觸發，無需等待放開
    //     2. 縮短消抖時間至 35ms（預設 50ms），加快反應速度
    //        消抖仍然足夠過濾機械彈跳，但不會造成明顯延遲
    //     3. loop() 中將 drawButtonStatus() 改為定時更新（每 50ms），
    //        減少 SPI 匯流排佔用，讓 Button2.loop() 有更多執行機會
    //
    //   替代方案：
    //     - 使用 setClickHandler() + setDoubleClickTime(0) 也可降低延遲，
    //       但 setPressedHandler() 的即時性更佳
    //     - 若需要長按功能，可額外設定 setLongClickHandler()
    //
    btnA.begin(BUTTON_A_PIN);
    btnB.begin(BUTTON_B_PIN);

    // 設定消抖時間為 35ms（預設 50ms）
    // 機械按鍵的彈跳通常在 5~25ms 內穩定，35ms 已足夠
    btnA.setDebounceTime(35);
    btnB.setDebounceTime(35);

    // 使用 setPressedHandler：按下瞬間立即觸發，不等待放開
    btnA.setPressedHandler(onButtonAPressed);
    btnB.setPressedHandler(onButtonBPressed);

    // --- 5. 輸出系統資訊到 Serial Monitor ---
    printSystemInfo();

    // --- 6. 首次畫面繪製 ---
    drawScreen();

    Serial.println("[INFO] 進入主迴圈，等待按鍵輸入...");
    Serial.println();
}

// ============================================================
//  loop()
//  ──────
//  Arduino 框架的主迴圈，持續重複執行。
//
//  執行順序與設計考量：
//    1. Button2.loop() 必須被頻繁呼叫（建議 < 10ms 間隔），
//       以確保消抖和事件偵測正常運作
//    2. 螢幕重繪僅在顏色變更時執行（由 needsRedraw 旗標控制）
//    3. 按鍵狀態顯示每 STATUS_UPDATE_MS 更新一次，避免過度佔用 SPI
//    4. 系統健康報告每 HEALTH_INTERVAL_MS 輸出一次到 Serial
//
//  效能注意：
//    drawScreen() 中的 fillScreen() 約耗時 15~30ms（視 SPI 頻率），
//    這段期間 Button2 無法處理事件。改用 setPressedHandler 後，
//    即使偶爾錯過一個 loop 週期，按下事件仍會在下次 loop() 被捕捉。
// ============================================================
void loop() {
    // --- 1. 最優先：更新按鍵狀態機 ---
    // Button2 內部維護每個按鍵的狀態機（idle → pressed → released）
    // 必須在每次 loop 迭代都呼叫，否則會漏接事件
    btnA.loop();
    btnB.loop();

    // --- 2. 條件式全螢幕重繪 ---
    // 僅在按鍵回呼設定 needsRedraw = true 時才執行
    // fillScreen() 是 SPI 阻塞操作，應盡量減少呼叫次數
    if (needsRedraw) {
        drawScreen();
        needsRedraw = false;
    }

    // --- 3. 定時更新按鍵狀態顯示 ---
    // 限制更新頻率，避免每個 loop 迭代都做 SPI 寫入
    unsigned long now = millis();
    if (now - lastStatusUpdate >= STATUS_UPDATE_MS) {
        lastStatusUpdate = now;
        drawButtonStatus(false);  // false = 非強制更新，僅狀態變化時才寫入
    }

    // --- 4. 定時輸出系統健康報告 ---
    if (now - lastHealthReport >= HEALTH_INTERVAL_MS) {
        lastHealthReport = now;
        printHealthReport();
    }
}

// ============================================================
//  drawScreen()
//  ────────────
//  全螢幕重繪函式。清除背景後重新繪製所有 UI 元素。
//
//  繪製層次（由下到上）：
//    1. 背景填滿（fillScreen）
//    2. 主標題 "Hello TTGO!"（螢幕中央偏上）
//    3. 底部操作提示文字
//    4. 頂部按鍵狀態列（透過 drawButtonStatus 強制重繪）
//
//  字型選擇：
//    - 標題：FreeSansBold18pt7b（粗體無襯線，18pt，清晰易讀）
//    - 提示：FreeSans9pt7b（一般無襯線，9pt，不搶主體視覺）
//    這些字型由 TFT_eSPI 的 LOAD_GFXFF flag 啟用
//
//  文字定位：
//    使用 setTextDatum() 設定錨點，避免手動計算文字寬高
//    MC_DATUM = Middle Centre（中心對齊，適合標題）
//    BC_DATUM = Bottom Centre（底部居中，適合提示列）
// ============================================================
void drawScreen() {
    uint16_t bg = bgColors[bgIndex];
    uint16_t fg = fgColors[fgIndex];

    // 清除整個螢幕為目前背景色
    // 注意：fillScreen 會寫入 240×135 = 32,400 個像素（每像素 2 bytes），
    // 透過 SPI 傳輸約 64,800 bytes，在 40MHz SPI 下約耗時 13ms
    tft.fillScreen(bg);

    // --- 繪製主標題 "Hello TTGO!" ---
    tft.setTextColor(fg, bg);   // 第二參數 bg 啟用背景填充，避免文字疊影
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);         // 字型倍率 1x（使用 FreeFont 時建議固定為 1）
    tft.setFreeFont(&FreeSansBold18pt7b);
    tft.drawString("Hello TTGO!", SCREEN_W / 2, SCREEN_H / 2 - 10);

    // --- 繪製底部操作提示 ---
    tft.setFreeFont(&FreeSans9pt7b);
    tft.setTextDatum(BC_DATUM);
    tft.drawString("A:bg  B:text", SCREEN_W / 2, SCREEN_H - 5);

    // --- 強制重繪按鍵狀態列 ---
    // 因為 fillScreen 已清除頂部區域，必須立即重繪
    drawButtonStatus(true);
}

// ============================================================
//  drawButtonStatus(bool forceUpdate)
//  ───────────────────────────────────
//  更新螢幕頂部的按鍵即時狀態顯示。
//
//  參數：
//    forceUpdate - true:  無條件重繪（用於 drawScreen 全螢幕重繪後）
//                  false: 僅在按鍵狀態與上次不同時才重繪（省電省頻寬）
//
//  設計考量：
//    - 使用 static 變數記憶上次狀態，狀態未變時跳過 SPI 寫入
//    - fillRect 僅清除頂部 24px 高的狀態列，而非整個螢幕
//    - 「按下」顯示亮綠色，「放開」顯示暗灰色，視覺對比明確
//
//  GPIO 讀取說明：
//    digitalRead() 讀取 GPIO 的即時電位
//    按鍵按下 = 接地 = LOW，放開 = 上拉 = HIGH
//    GPIO 35 為 input-only 腳位，無需設定 pinMode
//   （Button2.begin() 已處理 pinMode 設定）
// ============================================================
void drawButtonStatus(bool forceUpdate) {
    uint16_t bg = bgColors[bgIndex];

    // 讀取按鍵的即時物理狀態
    bool aPressed = (digitalRead(BUTTON_A_PIN) == LOW);
    bool bPressed = (digitalRead(BUTTON_B_PIN) == LOW);

    // 用 static 變數追蹤上次的狀態，減少不必要的 SPI 傳輸
    static bool prevA = false;
    static bool prevB = false;

    // 非強制更新時，狀態未變化則直接返回
    if (!forceUpdate && aPressed == prevA && bPressed == prevB) return;
    prevA = aPressed;
    prevB = bPressed;

    tft.setFreeFont(&FreeSans9pt7b);
    tft.setTextSize(1);

    // 清除狀態列區域（僅頂部 24 像素）
    tft.fillRect(0, 0, SCREEN_W, 24, bg);

    // --- Button A 狀態（左上角）---
    tft.setTextDatum(TL_DATUM);   // Top-Left 錨點
    tft.setTextColor(aPressed ? TFT_GREEN : TFT_DARKGREY, bg);
    tft.drawString(aPressed ? "A: PRESSED" : "A: ---", 8, 4);

    // --- Button B 狀態（右上角）---
    tft.setTextDatum(TR_DATUM);   // Top-Right 錨點
    tft.setTextColor(bPressed ? TFT_GREEN : TFT_DARKGREY, bg);
    tft.drawString(bPressed ? "B: PRESSED" : "B: ---", SCREEN_W - 8, 4);
}

// ============================================================
//  onButtonAPressed()
//  ──────────────────
//  Button A (GPIO 0) 的「按下」事件回呼函式。
//
//  功能：循環切換背景顏色
//
//  觸發時機：
//    使用 setPressedHandler() 註冊，按下瞬間立即觸發。
//    與 setClickHandler() 的差異：
//      - clickHandler: 按下→放開→等待雙擊逾時(~300ms) 後才觸發
//      - pressedHandler: 按下瞬間觸發，延遲僅有消抖時間(35ms)
//
//  Serial 輸出格式：
//    [事件] 時間戳(ms) | 按鍵名 | 新索引 | 累計次數
// ============================================================
void onButtonAPressed(Button2 &btn) {
    bgIndex = (bgIndex + 1) % BG_COUNT;
    needsRedraw = true;
    btnACount++;

    Serial.printf("[BTN-A] %8lu ms | 背景顏色 → #%d/%d | 累計觸發：%u 次\n",
                  millis(), bgIndex, BG_COUNT, btnACount);
}

// ============================================================
//  onButtonBPressed()
//  ──────────────────
//  Button B (GPIO 35) 的「按下」事件回呼函式。
//
//  功能：循環切換文字顏色
//
//  注意：GPIO 35 為 input-only 腳位（ESP32 的 GPIO 34~39），
//  這些腳位只能讀取，無法輸出。它們也沒有內部上拉/下拉電阻，
//  TTGO T-Display 板上已為此腳位外接上拉電阻。
// ============================================================
void onButtonBPressed(Button2 &btn) {
    fgIndex = (fgIndex + 1) % FG_COUNT;
    needsRedraw = true;
    btnBCount++;

    Serial.printf("[BTN-B] %8lu ms | 文字顏色 → #%d/%d | 累計觸發：%u 次\n",
                  millis(), fgIndex, FG_COUNT, btnBCount);
}
