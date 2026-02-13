/*
 * ═══════════════════════════════════════════════════════════════════
 *  CARDPUTER DUAL DISPLAY - SEPARATED SCREENS v3.1 (CORRECTED)
 *  
 *  EXTERNAL DISPLAY (ILI9341 320x240):
 *  ⏱️  Stopwatch - ENTER = Start/Stop, BACKSPACE = Exit
 *  🎨 Color Test - ;/. = Change, BACKSPACE = Exit
 *  🎲 Dice Roller - ;/. = Select, ENTER = Roll, BACKSPACE = Exit
 *  📊 Bar Chart - Auto, BACKSPACE = Exit
 *  
 *  INTERNAL DISPLAY (ST7789 240x135):
 *  📝 Notepad - Text editor
 *  🎯 Pointer Test - E/S/D/A = Move
 *  ℹ️  System Info - Monitor
 *  🌈 Animation - Visual effects
 *  
 *  NO INTERFERENCE - Completely separated controls!
 * ═══════════════════════════════════════════════════════════════════
 */

#include <M5Cardputer.h>
#include <M5GFX.h>
#include <lgfx/v1/panel/Panel_LCD.hpp>

// =============================================================
// ILI9341 DRIVER
// =============================================================
struct Panel_ILI9341_Local : public lgfx::v1::Panel_LCD {
    Panel_ILI9341_Local(void) {
        _cfg.memory_width  = _cfg.panel_width  = 240;
        _cfg.memory_height = _cfg.panel_height = 320;
    }
    void setColorDepth_impl(lgfx::v1::color_depth_t depth) override {
        _write_depth = lgfx::v1::rgb565_2Byte;
        _read_depth  = lgfx::v1::rgb888_3Byte;
    }
protected:
    const uint8_t* getInitCommands(uint8_t listno) const override {
        static constexpr uint8_t list0[] = {
            0x01, 0+CMD_INIT_DELAY, 150,
            0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
            0xCF, 3, 0x00, 0xC1, 0x30,
            0xE8, 3, 0x85, 0x00, 0x78,
            0xEA, 2, 0x00, 0x00,
            0xED, 4, 0x64, 0x03, 0x12, 0x81,
            0xF7, 1, 0x20,
            0xC0, 1, 0x23, 0xC1, 1, 0x10,
            0xC5, 2, 0x3e, 0x28, 0xC7, 1, 0x86,
            0x36, 1, 0x48, 0x3A, 1, 0x55,
            0xB1, 2, 0x00, 0x18,
            0xB6, 3, 0x08, 0x82, 0x27,
            0xF2, 1, 0x00, 0x26, 1, 0x01,
            0xE0, 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 
                       0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
            0xE1, 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
                       0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
            0x11, 0+CMD_INIT_DELAY, 120,
            0x29, 0+CMD_INIT_DELAY, 120,
            0xFF, 0xFF,
        };
        return (listno == 0) ? list0 : nullptr;
    }
};

class LGFX_ILI9341 : public lgfx::v1::LGFX_Device {
    Panel_ILI9341_Local panel;
    lgfx::v1::Bus_SPI   bus;
public:
    LGFX_ILI9341() {
        auto b = bus.config();
        b.spi_host    = SPI2_HOST;
        b.spi_mode    = 0;
        b.freq_write  = 20000000;
        b.freq_read   = 16000000;
        b.spi_3wire   = true;
        b.use_lock    = true;
        b.dma_channel = SPI_DMA_CH_AUTO;
        b.pin_sclk = 40;
        b.pin_mosi = 14;
        b.pin_miso = -1;
        b.pin_dc   = 15;
        bus.config(b);
        panel.setBus(&bus);
        auto p = panel.config();
        p.pin_cs    = 5;
        p.pin_rst   = 13;
        p.pin_busy  = -1;
        p.readable  = false;
        p.invert    = false;
        p.rgb_order = false;
        p.memory_width  = 240;
        p.memory_height = 320;
        p.panel_width   = 240;
        p.panel_height  = 320;
        panel.config(p);
        setPanel(&panel);
    }
};

// =============================================================
// GLOBAL OBJECTS
// =============================================================
LGFX_ILI9341 externalDisplay;
M5Canvas extSprite(&externalDisplay);
M5Canvas intSprite(&M5Cardputer.Display);

// =============================================================
// EXTERNAL DISPLAY STATES
// =============================================================
enum ExtState {
    EXT_MENU,
    EXT_STOPWATCH,
    EXT_COLORTEST,
    EXT_DICE,
    EXT_CHART
};

ExtState extState = EXT_MENU;
int extMenuIndex = 0;
const char* extMenuItems[] = {"Stopwatch", "Color Test", "Dice Roller", "Bar Chart"};
const int EXT_MENU_ITEMS = 4;

// External: Stopwatch
unsigned long extStopwatchStart = 0;
unsigned long extStopwatchElapsed = 0;
bool extStopwatchRunning = false;

// External: Color test
int extColorMode = 0;
const int EXT_COLOR_MODES = 5;

// External: Dice
int extDiceType = 6;
int extDiceResult = 1;
bool extDiceRolling = false;
unsigned long extDiceRollTime = 0;

// External: Chart
float extChartData[10] = {65, 45, 80, 30, 95, 55, 70, 40, 85, 60};

// =============================================================
// INTERNAL DISPLAY STATES
// =============================================================
enum IntState {
    INT_MENU,
    INT_NOTEPAD,
    INT_POINTER,
    INT_SYSINFO,
    INT_ANIMATION
};

IntState intState = INT_MENU;
int intMenuIndex = 0;
const char* intMenuItems[] = {"Notepad", "Pointer Test", "System Info", "Animation"};
const int INT_MENU_ITEMS = 4;

// Internal: Notepad
String intNotepadLines[8];
int intNotepadCursorLine = 0;
int intNotepadCursorPos = 0;
bool intNotepadSaved = true;

// Internal: Pointer
int intPointerX = 120;
int intPointerY = 67;

// Internal: Animation
float intAnimPhase = 0;

// =============================================================
// EXTERNAL DISPLAY DRAW FUNCTIONS
// =============================================================
void drawExt_Menu() {
    extSprite.fillScreen(TFT_BLACK);
    extSprite.fillRect(0, 0, 320, 45, 0x10A2);
    extSprite.setTextColor(TFT_WHITE);
    extSprite.setTextSize(2);
    extSprite.setCursor(70, 12);
    extSprite.print("EXTERNAL DISPLAY");

    for (int i = 0; i < EXT_MENU_ITEMS; i++) {
        int y = 60 + (i * 38);
        if (i == extMenuIndex) {
            extSprite.fillRect(20, y - 3, 280, 30, 0x2C5F);
            extSprite.setTextColor(TFT_YELLOW);
        } else {
            extSprite.setTextColor(TFT_WHITE);
        }
        extSprite.setTextSize(2);
        extSprite.setCursor(40, y);
        extSprite.printf("%d. %s", i + 1, extMenuItems[i]);
    }

    extSprite.setTextColor(TFT_CYAN);
    extSprite.setTextSize(1);
    extSprite.setCursor(20, 220);
    extSprite.print("External: ;/. = UP/DOWN | ENTER = Select | BACKSPACE = Back");
}

void drawExt_Stopwatch() {
    extSprite.fillScreen(TFT_BLACK);
    extSprite.fillRect(0, 0, 320, 50, 0x0A3D);
    extSprite.setTextColor(TFT_WHITE);
    extSprite.setTextSize(2);
    extSprite.setCursor(70, 15);
    extSprite.print("STOPWATCH");
    extSprite.setTextSize(1);
    extSprite.setCursor(230, 20);
    extSprite.print("[EXTERNAL]");

    unsigned long elapsed = extStopwatchRunning ? 
        (millis() - extStopwatchStart + extStopwatchElapsed) : extStopwatchElapsed;
    int minutes = (elapsed / 60000) % 60;
    int seconds = (elapsed / 1000) % 60;
    int ms = elapsed % 1000;

    extSprite.setTextSize(7);
    extSprite.setTextColor(extStopwatchRunning ? TFT_GREEN : TFT_YELLOW);
    extSprite.setCursor(30, 80);
    extSprite.printf("%02d:%02d", minutes, seconds);

    extSprite.setTextSize(3);
    extSprite.setTextColor(TFT_CYAN);
    extSprite.setCursor(130, 160);
    extSprite.printf(".%03d", ms);

    extSprite.setTextSize(2);
    extSprite.setTextColor(TFT_WHITE);
    extSprite.setCursor(80, 195);
    extSprite.printf("[%s]", extStopwatchRunning ? "RUNNING" : "STOPPED");

    extSprite.setTextColor(TFT_CYAN);
    extSprite.setTextSize(1);
    extSprite.setCursor(20, 220);
    extSprite.print("ENTER = Start/Stop | R = Reset | BACKSPACE = Back");
}

void drawExt_ColorTest() {
    const char* modes[] = {"Gradient", "Color Bars", "Checkerboard", "Rainbow", "Solid Colors"};

    switch(extColorMode) {
        case 0:
            for (int y = 0; y < 200; y++) {
                uint8_t r = (y * 255) / 200;
                extSprite.drawFastHLine(0, y, 320, extSprite.color565(r, 255-r, 128));
            }
            break;
        case 1:
            for (int i = 0; i < 8; i++) {
                uint16_t cols[] = {TFT_WHITE, TFT_YELLOW, TFT_CYAN, TFT_GREEN, 
                                  TFT_MAGENTA, TFT_RED, TFT_BLUE, TFT_BLACK};
                extSprite.fillRect(i * 40, 0, 40, 200, cols[i]);
            }
            break;
        case 2:
            for (int y = 0; y < 200; y += 20)
                for (int x = 0; x < 320; x += 20)
                    extSprite.fillRect(x, y, 20, 20, ((x/20 + y/20) % 2) ? TFT_WHITE : TFT_BLACK);
            break;
        case 3:
            for (int x = 0; x < 320; x++) {
                uint8_t h = (x * 255) / 320;
                extSprite.drawFastVLine(x, 0, 200, extSprite.color565(h, 255-h, 128));
            }
            break;
        case 4:
            static int sc = 0;
            static unsigned long lc = 0;
            if (millis() - lc > 800) { sc = (sc + 1) % 7; lc = millis(); }
            uint16_t cols[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_CYAN, TFT_MAGENTA, TFT_WHITE};
            extSprite.fillScreen(cols[sc]);
            break;
    }

    extSprite.fillRect(0, 200, 320, 40, TFT_BLACK);
    extSprite.setTextColor(TFT_WHITE);
    extSprite.setTextSize(2);
    extSprite.setCursor(10, 208);
    extSprite.printf("Mode: %s", modes[extColorMode]);
    extSprite.setTextColor(TFT_CYAN);
    extSprite.setTextSize(1);
    extSprite.setCursor(10, 230);
    extSprite.print("Color: ;/. = Change Mode | BACKSPACE = Back");
}

void drawExt_Dice() {
    extSprite.fillScreen(TFT_BLACK);
    extSprite.fillRect(0, 0, 320, 50, 0x3D5C);
    extSprite.setTextColor(TFT_WHITE);
    extSprite.setTextSize(2);
    extSprite.setCursor(80, 15);
    extSprite.print("DICE ROLLER");
    extSprite.setTextSize(1);
    extSprite.setCursor(230, 20);
    extSprite.print("[EXTERNAL]");

    const char* dNames[] = {"D6", "D20", "D100"};
    int dTypes[] = {6, 20, 100};
    for (int i = 0; i < 3; i++) {
        int x = 40 + (i * 90);
        bool selected = (extDiceType == dTypes[i]);
        extSprite.fillRect(x, 60, 70, 30, selected ? 0x4A3F : 0x2C5F);
        extSprite.setTextColor(selected ? TFT_YELLOW : TFT_WHITE);
        extSprite.setTextSize(2);
        extSprite.setCursor(x + 22, 67);
        extSprite.print(dNames[i]);
    }

    int showResult = extDiceResult;
    if (extDiceRolling) {
        if (millis() - extDiceRollTime < 1000) {
            showResult = random(1, extDiceType + 1);
        } else {
            extDiceRolling = false;
            extDiceResult = random(1, extDiceType + 1);
            showResult = extDiceResult;
        }
    }

    extSprite.fillRect(110, 110, 100, 80, TFT_WHITE);
    extSprite.drawRect(108, 108, 104, 84, TFT_YELLOW);
    extSprite.setTextColor(TFT_BLACK);
    extSprite.setTextSize(4);
    extSprite.setCursor(130, 130);
    extSprite.printf("%d", showResult);

    extSprite.fillRect(130, 200, 60, 25, extDiceRolling ? TFT_ORANGE : TFT_GREEN);
    extSprite.setTextColor(TFT_WHITE);
    extSprite.setTextSize(2);
    extSprite.setCursor(135, 205);
    extSprite.print(extDiceRolling ? "..." : "ROLL");

    extSprite.setTextColor(TFT_CYAN);
    extSprite.setTextSize(1);
    extSprite.setCursor(20, 230);
    extSprite.print("Dice: ;/. = Select | ENTER = Roll | BACKSPACE = Back");
}

void drawExt_Chart() {
    extSprite.fillScreen(TFT_BLACK);
    extSprite.fillRect(0, 0, 320, 40, 0x1B4D);
    extSprite.setTextColor(TFT_WHITE);
    extSprite.setTextSize(2);
    extSprite.setCursor(90, 10);
    extSprite.print("BAR CHART");
    extSprite.setTextSize(1);
    extSprite.setCursor(230, 15);
    extSprite.print("[EXTERNAL]");

    if (millis() % 500 < 50) {
        for (int i = 0; i < 9; i++) extChartData[i] = extChartData[i+1];
        extChartData[9] = random(10, 100);
    }

    for (int i = 0; i < 10; i++) {
        int x = 25 + (i * 28);
        int h = (extChartData[i] * 140) / 100;
        int y = 180 - h;
        uint16_t color = extSprite.color565((extChartData[i] * 255) / 100, 255 - ((extChartData[i] * 255) / 100), 100);
        extSprite.fillRect(x, y, 22, h, color);
        extSprite.drawRect(x, y, 22, h, TFT_WHITE);
        extSprite.setTextSize(1);
        extSprite.setTextColor(TFT_WHITE);
        extSprite.setCursor(x + 5, y - 12);
        extSprite.printf("%d", (int)extChartData[i]);
    }

    extSprite.drawLine(20, 180, 300, 180, TFT_WHITE);

    extSprite.setTextColor(TFT_CYAN);
    extSprite.setTextSize(1);
    extSprite.setCursor(20, 220);
    extSprite.print("Live chart - Auto-updating | BACKSPACE = Back");
}

// =============================================================
// INTERNAL DISPLAY DRAW FUNCTIONS
// =============================================================
void drawInt_Menu() {
    intSprite.fillScreen(TFT_BLACK);
    intSprite.fillRect(0, 0, 240, 28, TFT_DARKGREY);
    intSprite.setTextColor(TFT_WHITE);
    intSprite.setTextSize(2);
    intSprite.setCursor(5, 5);
    intSprite.print("INTERNAL DISPLAY");
    intSprite.setTextSize(1);
    intSprite.setCursor(180, 10);
    intSprite.printf("BAT:%d%%", M5Cardputer.Power.getBatteryLevel());

    for (int i = 0; i < INT_MENU_ITEMS; i++) {
        int y = 40 + (i * 24);
        if (i == intMenuIndex) {
            intSprite.fillRect(10, y - 2, 220, 22, 0x2C5F);
            intSprite.setTextColor(TFT_YELLOW);
        } else {
            intSprite.setTextColor(TFT_WHITE);
        }
        intSprite.setTextSize(2);
        intSprite.setCursor(20, y);
        intSprite.printf("> %s", intMenuItems[i]);
    }

    intSprite.setTextColor(TFT_CYAN);
    intSprite.setTextSize(1);
    intSprite.setCursor(10, 118);
    intSprite.print("Internal: E/S = UP/DOWN | CTRL = Select | TAB = Back");
}

void drawInt_Notepad() {
    intSprite.fillScreen(TFT_BLACK);
    intSprite.fillRect(0, 0, 240, 22, 0x2C5F);
    intSprite.setTextColor(TFT_WHITE);
    intSprite.setTextSize(1);
    intSprite.setCursor(5, 6);
    intSprite.print("NOTEPAD [INTERNAL]");
    intSprite.setCursor(160, 6);
    intSprite.printf("%s", intNotepadSaved ? "[SAVED]" : "[EDIT]");

    intSprite.fillRect(5, 28, 230, 82, 0x0A2E);
    intSprite.setTextColor(TFT_WHITE);
    intSprite.setTextSize(1);

    for (int i = 0; i < 6; i++) {
        int y = 32 + (i * 13);
        int lineIdx = intNotepadCursorLine - 3 + i;
        if (lineIdx >= 0 && lineIdx < 8) {
            if (lineIdx == intNotepadCursorLine) {
                intSprite.fillRect(8, y - 1, 224, 12, 0x1B5E);
                intSprite.setTextColor(TFT_YELLOW);
                if ((millis() / 500) % 2 == 0) {
                    int cx = 10 + (intNotepadCursorPos * 6);
                    intSprite.drawLine(cx, y + 10, cx + 4, y + 10, TFT_YELLOW);
                }
            } else {
                intSprite.setTextColor(TFT_WHITE);
            }
            intSprite.setCursor(10, y);
            String line = intNotepadLines[lineIdx];
            if (line.length() > 35) line = line.substring(0, 35);
            intSprite.print(line);
        }
    }

    intSprite.fillRect(5, 115, 230, 17, 0x1B3E);
    intSprite.setTextColor(TFT_CYAN);
    intSprite.setTextSize(1);
    intSprite.setCursor(10, 119);
    intSprite.printf("Line %d/8 | Type to edit | TAB = Save & Exit", intNotepadCursorLine + 1);
}

void drawInt_Pointer() {
    intSprite.fillScreen(TFT_BLACK);
    intSprite.fillRect(0, 0, 240, 22, 0x4A2F);
    intSprite.setTextColor(TFT_WHITE);
    intSprite.setTextSize(1);
    intSprite.setCursor(5, 6);
    intSprite.print("POINTER TEST [INTERNAL]");

    for (int x = 0; x < 240; x += 30) {
        intSprite.drawLine(x, 30, x, 100, 0x2C5F);
        intSprite.setTextColor(0x5C7F);
        intSprite.setCursor(x + 2, 102);
        intSprite.print(x / 30);
    }
    for (int y = 30; y < 100; y += 20) {
        intSprite.drawLine(0, y, 240, y, 0x2C5F);
    }

    intSprite.fillCircle(intPointerX, intPointerY, 4, TFT_RED);
    intSprite.drawLine(intPointerX - 6, intPointerY, intPointerX + 6, intPointerY, TFT_YELLOW);
    intSprite.drawLine(intPointerX, intPointerY - 6, intPointerX, intPointerY + 6, TFT_YELLOW);

    intSprite.fillRect(10, 108, 220, 24, 0x1B3E);
    intSprite.setTextColor(TFT_CYAN);
    intSprite.setTextSize(2);
    intSprite.setCursor(70, 112);
    intSprite.printf("X:%d Y:%d", intPointerX, intPointerY);

    intSprite.setTextColor(TFT_CYAN);
    intSprite.setTextSize(1);
    intSprite.setCursor(10, 118);
    intSprite.print("Pointer: E/S/D/A = Move | TAB = Back");
}

void drawInt_SysInfo() {
    intSprite.fillScreen(TFT_BLACK);
    intSprite.fillRect(0, 0, 240, 22, 0x4A3F);
    intSprite.setTextColor(TFT_WHITE);
    intSprite.setTextSize(1);
    intSprite.setCursor(5, 6);
    intSprite.print("SYSTEM INFO [INTERNAL]");

    intSprite.setTextColor(TFT_CYAN);
    intSprite.setTextSize(1);
    intSprite.setCursor(10, 35);
    intSprite.print("CPU: ESP32-S3 Dual Core");

    intSprite.setCursor(10, 55);
    intSprite.setTextColor(TFT_YELLOW);
    intSprite.printf("Free Heap: %d KB", ESP.getFreeHeap() / 1024);

    intSprite.setCursor(10, 75);
    intSprite.setTextColor(TFT_GREEN);
    intSprite.printf("Uptime: %ds", millis() / 1000);

    intSprite.setCursor(10, 95);
    intSprite.setTextColor(TFT_MAGENTA);
    intSprite.printf("Battery: %d%%", M5Cardputer.Power.getBatteryLevel());

    int bat = M5Cardputer.Power.getBatteryLevel();
    intSprite.drawRect(10, 115, 100, 12, TFT_WHITE);
    intSprite.fillRect(12, 117, (bat * 96) / 100, 8, bat > 20 ? TFT_GREEN : TFT_RED);

    intSprite.setTextColor(TFT_CYAN);
    intSprite.setCursor(120, 118);
    intSprite.print("TAB = Back");
}

void drawInt_Animation() {
    intSprite.fillScreen(TFT_BLACK);

    for (int i = 0; i < 3; i++) {
        int x = 120 + (int)(cos(intAnimPhase + i * 2) * 50);
        int y = 50 + (int)(sin(intAnimPhase * 0.7 + i) * 30);
        int r = 8 + (int)(sin(intAnimPhase + i) * 5);
        uint16_t color = intSprite.color565(100 + (int)(sin(intAnimPhase + i) * 100), 50, 200);
        intSprite.fillCircle(x, y, r, color);
    }

    for (int x = 0; x < 240; x++) {
        int y = 100 + (int)(sin((x / 15.0) + intAnimPhase) * 15);
        intSprite.drawPixel(x, y, TFT_YELLOW);
        intSprite.drawPixel(x, y + 1, TFT_ORANGE);
    }

    intSprite.setTextColor(TFT_WHITE);
    intSprite.setTextSize(1);
    intSprite.setCursor(60, 125);
    intSprite.print("Internal Animation");

    intAnimPhase += 0.08;
}

// =============================================================
// INPUT HANDLERS
// =============================================================
void handleIntNotepad(char c) {
    if (c == '\n') {
        if (intNotepadCursorLine < 7) {
            intNotepadCursorLine++;
            intNotepadCursorPos = 0;
        }
    } else if (c == '\b') {
        if (intNotepadCursorPos > 0) {
            String line = intNotepadLines[intNotepadCursorLine];
            String before = line.substring(0, intNotepadCursorPos - 1);
            String after = line.substring(intNotepadCursorPos);
            intNotepadLines[intNotepadCursorLine] = before + after;
            intNotepadCursorPos--;
        } else if (intNotepadCursorLine > 0) {
            intNotepadCursorLine--;
            intNotepadCursorPos = intNotepadLines[intNotepadCursorLine].length();
        }
    } else if (c >= 32 && c <= 126) {
        if (intNotepadLines[intNotepadCursorLine].length() < 35) {
            String line = intNotepadLines[intNotepadCursorLine];
            String before = line.substring(0, intNotepadCursorPos);
            String after = line.substring(intNotepadCursorPos);
            intNotepadLines[intNotepadCursorLine] = before + String(c) + after;
            intNotepadCursorPos++;
        }
    }
    intNotepadSaved = false;
}

// =============================================================
// SETUP & LOOP
// =============================================================
void setup() {
    Serial.begin(115200);

    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);

    intSprite.createSprite(240, 135);

    delay(100);
    externalDisplay.init();
    externalDisplay.setRotation(7);
    externalDisplay.fillScreen(TFT_BLACK);

    extSprite.setColorDepth(16);
    extSprite.createSprite(320, 240);

    for (int i = 0; i < 8; i++) intNotepadLines[i] = "";

    randomSeed(analogRead(0));

    Serial.println("Dual Display - Separated Controls Started!");
    Serial.println("External: Stopwatch, Color Test, Dice, Chart");
    Serial.println("Internal: Notepad, Pointer, SysInfo, Animation");
}

void loop() {
    M5Cardputer.update();

    if (M5Cardputer.Keyboard.isChange()) {
        // ═══════════════════════════════════════════════════════
        // EXTERNAL DISPLAY CONTROLS
        // ═══════════════════════════════════════════════════════
        switch(extState) {
            case EXT_MENU:
                if (M5Cardputer.Keyboard.isKeyPressed('.')) 
                    extMenuIndex = (extMenuIndex + 1) % EXT_MENU_ITEMS;
                if (M5Cardputer.Keyboard.isKeyPressed(';')) 
                    extMenuIndex = (extMenuIndex - 1 + EXT_MENU_ITEMS) % EXT_MENU_ITEMS;
                if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
                    switch(extMenuIndex) {
                        case 0: extState = EXT_STOPWATCH; break;
                        case 1: extState = EXT_COLORTEST; break;
                        case 2: extState = EXT_DICE; break;
                        case 3: extState = EXT_CHART; break;
                    }
                }
                break;

            case EXT_STOPWATCH:
                // ENTER = Start/Stop (not S!)
                if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
                    if (extStopwatchRunning) {
                        extStopwatchElapsed += millis() - extStopwatchStart;
                        extStopwatchRunning = false;
                    } else {
                        extStopwatchStart = millis();
                        extStopwatchRunning = true;
                    }
                }
                // R = Reset
                if (M5Cardputer.Keyboard.isKeyPressed('r') || M5Cardputer.Keyboard.isKeyPressed('R')) {
                    extStopwatchRunning = false;
                    extStopwatchElapsed = 0;
                }
                // BACKSPACE = Exit (not TAB!)
                if (M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE)) 
                    extState = EXT_MENU;
                break;

            case EXT_COLORTEST:
                if (M5Cardputer.Keyboard.isKeyPressed('.')) 
                    extColorMode = (extColorMode + 1) % EXT_COLOR_MODES;
                if (M5Cardputer.Keyboard.isKeyPressed(';')) 
                    extColorMode = (extColorMode - 1 + EXT_COLOR_MODES) % EXT_COLOR_MODES;
                if (M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE)) 
                    extState = EXT_MENU;
                break;

            case EXT_DICE:
                if (M5Cardputer.Keyboard.isKeyPressed('.')) {
                    const int types[] = {6, 20, 100};
                    int idx = 0;
                    for (int i = 0; i < 3; i++) if (extDiceType == types[i]) idx = i;
                    extDiceType = types[(idx + 1) % 3];
                }
                if (M5Cardputer.Keyboard.isKeyPressed(';')) {
                    const int types[] = {6, 20, 100};
                    int idx = 0;
                    for (int i = 0; i < 3; i++) if (extDiceType == types[i]) idx = i;
                    extDiceType = types[(idx - 1 + 3) % 3];
                }
                if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER) && !extDiceRolling) {
                    extDiceRolling = true;
                    extDiceRollTime = millis();
                }
                if (M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE)) 
                    extState = EXT_MENU;
                break;

            case EXT_CHART:
                if (M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE)) 
                    extState = EXT_MENU;
                break;
        }

        // ═══════════════════════════════════════════════════════
        // INTERNAL DISPLAY CONTROLS (Independent!)
        // ═══════════════════════════════════════════════════════
        switch(intState) {
            case INT_MENU:
                if (M5Cardputer.Keyboard.isKeyPressed('e')) 
                    intMenuIndex = (intMenuIndex - 1 + INT_MENU_ITEMS) % INT_MENU_ITEMS;
                if (M5Cardputer.Keyboard.isKeyPressed('s')) 
                    intMenuIndex = (intMenuIndex + 1) % INT_MENU_ITEMS;
                if (M5Cardputer.Keyboard.isKeyPressed(KEY_LEFT_CTRL)) {
                    switch(intMenuIndex) {
                        case 0: intState = INT_NOTEPAD; break;
                        case 1: intState = INT_POINTER; break;
                        case 2: intState = INT_SYSINFO; break;
                        case 3: intState = INT_ANIMATION; break;
                    }
                }
                break;

            case INT_NOTEPAD:
                for (char c = 'a'; c <= 'z'; c++) {
                    if (M5Cardputer.Keyboard.isKeyPressed(c)) handleIntNotepad(c);
                }
                for (char c = '0'; c <= '9'; c++) {
                    if (M5Cardputer.Keyboard.isKeyPressed(c)) handleIntNotepad(c);
                }
                if (M5Cardputer.Keyboard.isKeyPressed(' ')) handleIntNotepad(' ');
                if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) handleIntNotepad('\n');
                if (M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE)) handleIntNotepad('\b');
                if (M5Cardputer.Keyboard.isKeyPressed(KEY_TAB)) {
                    intNotepadSaved = true;
                    intState = INT_MENU;
                }
                break;

            case INT_POINTER:
                if (M5Cardputer.Keyboard.isKeyPressed('e') && intPointerY > 30) 
                    intPointerY -= 3;
                if (M5Cardputer.Keyboard.isKeyPressed('s') && intPointerY < 105) 
                    intPointerY += 3;
                if (M5Cardputer.Keyboard.isKeyPressed('a') && intPointerX > 5) 
                    intPointerX -= 3;
                if (M5Cardputer.Keyboard.isKeyPressed('d') && intPointerX < 235) 
                    intPointerX += 3;
                if (M5Cardputer.Keyboard.isKeyPressed(KEY_TAB)) 
                    intState = INT_MENU;
                break;

            case INT_SYSINFO:
                if (M5Cardputer.Keyboard.isKeyPressed(KEY_TAB)) 
                    intState = INT_MENU;
                break;

            case INT_ANIMATION:
                if (M5Cardputer.Keyboard.isKeyPressed(KEY_TAB)) 
                    intState = INT_MENU;
                break;
        }
    }

    // Draw external
    switch(extState) {
        case EXT_MENU:       drawExt_Menu(); break;
        case EXT_STOPWATCH:  drawExt_Stopwatch(); break;
        case EXT_COLORTEST:  drawExt_ColorTest(); break;
        case EXT_DICE:       drawExt_Dice(); break;
        case EXT_CHART:      drawExt_Chart(); break;
    }
    extSprite.pushSprite(0, 0);

    // Draw internal
    switch(intState) {
        case INT_MENU:      drawInt_Menu(); break;
        case INT_NOTEPAD:   drawInt_Notepad(); break;
        case INT_POINTER:   drawInt_Pointer(); break;
        case INT_SYSINFO:   drawInt_SysInfo(); break;
        case INT_ANIMATION: drawInt_Animation(); break;
    }
    intSprite.pushSprite(0, 0);

    delay(50);
}