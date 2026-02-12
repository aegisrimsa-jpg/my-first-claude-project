#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Button2.h>

// --- Pin definitions ---
#define BUTTON_A_PIN  0
#define BUTTON_B_PIN  35

// --- Screen dimensions (landscape) ---
#define SCREEN_W  240
#define SCREEN_H  135

// --- Objects ---
TFT_eSPI tft = TFT_eSPI();
Button2 btnA;
Button2 btnB;

// --- Color palettes ---
const uint16_t bgColors[] = {
    TFT_BLACK, TFT_NAVY, TFT_DARKGREEN, TFT_MAROON,
    TFT_PURPLE, TFT_OLIVE, TFT_DARKGREY
};
const uint16_t fgColors[] = {
    TFT_GREEN, TFT_YELLOW, TFT_CYAN, TFT_WHITE,
    TFT_MAGENTA, TFT_ORANGE, TFT_RED
};
const uint8_t BG_COUNT = sizeof(bgColors) / sizeof(bgColors[0]);
const uint8_t FG_COUNT = sizeof(fgColors) / sizeof(fgColors[0]);

// --- State ---
uint8_t bgIndex = 0;
uint8_t fgIndex = 0;
bool needsRedraw = true;

// --- Forward declarations ---
void drawScreen();
void drawButtonStatus();
void onButtonA(Button2 &btn);
void onButtonB(Button2 &btn);

// ============================================================
void setup() {
    Serial.begin(115200);
    Serial.println("TTGO T-Display Test Starting...");

    // Backlight
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    // TFT init (rotation 1 = landscape, USB on the left)
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(bgColors[bgIndex]);

    // Button init
    btnA.begin(BUTTON_A_PIN);
    btnB.begin(BUTTON_B_PIN);
    btnA.setClickHandler(onButtonA);
    btnB.setClickHandler(onButtonB);

    drawScreen();
}

// ============================================================
void loop() {
    btnA.loop();
    btnB.loop();

    if (needsRedraw) {
        drawScreen();
        needsRedraw = false;
    }

    drawButtonStatus();
}

// ============================================================
// Full screen redraw: background + title + button status area
// ============================================================
void drawScreen() {
    uint16_t bg = bgColors[bgIndex];
    uint16_t fg = fgColors[fgIndex];

    tft.fillScreen(bg);

    // --- "Hello TTGO!" centred ---
    tft.setTextColor(fg, bg);
    tft.setTextDatum(MC_DATUM);      // middle-centre anchor
    tft.setTextSize(1);
    tft.setFreeFont(&FreeSansBold18pt7b);
    tft.drawString("Hello TTGO!", SCREEN_W / 2, SCREEN_H / 2 - 10);

    // --- Hint line ---
    tft.setFreeFont(&FreeSans9pt7b);
    tft.setTextDatum(BC_DATUM);      // bottom-centre
    tft.drawString("A:bg  B:text", SCREEN_W / 2, SCREEN_H - 5);

    drawButtonStatus();
}

// ============================================================
// Show real-time pressed / released state for both buttons
// ============================================================
void drawButtonStatus() {
    uint16_t bg = bgColors[bgIndex];

    bool aPressed = (digitalRead(BUTTON_A_PIN) == LOW);
    bool bPressed = (digitalRead(BUTTON_B_PIN) == LOW);

    static bool prevA = false;
    static bool prevB = false;

    // Only update when state changes to avoid flicker
    if (aPressed == prevA && bPressed == prevB) return;
    prevA = aPressed;
    prevB = bPressed;

    tft.setFreeFont(&FreeSans9pt7b);
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM);

    // Clear the status area
    tft.fillRect(0, 0, SCREEN_W, 24, bg);

    // Button A status (top-left)
    tft.setTextColor(aPressed ? TFT_GREEN : TFT_DARKGREY, bg);
    tft.drawString(aPressed ? "A: PRESSED" : "A: ---", 8, 4);

    // Button B status (top-right)
    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(bPressed ? TFT_GREEN : TFT_DARKGREY, bg);
    tft.drawString(bPressed ? "B: PRESSED" : "B: ---", SCREEN_W - 8, 4);
}

// ============================================================
// Button callbacks
// ============================================================
void onButtonA(Button2 &btn) {
    bgIndex = (bgIndex + 1) % BG_COUNT;
    needsRedraw = true;
    Serial.printf("Button A -> bg #%d\n", bgIndex);
}

void onButtonB(Button2 &btn) {
    fgIndex = (fgIndex + 1) % FG_COUNT;
    needsRedraw = true;
    Serial.printf("Button B -> fg #%d\n", fgIndex);
}
