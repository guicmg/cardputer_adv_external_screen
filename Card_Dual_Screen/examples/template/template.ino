/*
 * ═══════════════════════════════════════════════════════════════════
 *  CARDPUTER + ILI9341 TEMPLATE - Minimal Starting Point
 *  
 *  Use this as a starting point for your own dual-display projects!
 *  
 *  Hardware: M5Stack Cardputer ADV + ILI9341 2.4" Display
 *  Wiring:
 *    Cardputer Pin 2 (5VIN)  -> ILI9341 VCC
 *    Cardputer Pin 4 (GND)   -> ILI9341 GND
 *    Cardputer Pin 13 (G5)   -> ILI9341 CS
 *    Cardputer Pin 12 (G13)  -> ILI9341 RESET
 *    Cardputer Pin 14 (G15)  -> ILI9341 DC
 *    Cardputer Pin 9 (G14)   -> ILI9341 MOSI
 *    Cardputer Pin 7 (G40)   -> ILI9341 SCK
 *    Cardputer Pin 6 (5VOUT) -> ILI9341 LED
 * ═══════════════════════════════════════════════════════════════════
 */

#include <M5Cardputer.h>
#include <M5GFX.h>
#include <lgfx/v1/panel/Panel_LCD.hpp>

// =============================================================
// ILI9341 DRIVER - Copy this to your project
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
LGFX_ILI9341 externalDisplay;              // External ILI9341 (320x240)
M5Canvas extSprite(&externalDisplay);      // Sprite for external display
M5Canvas intSprite(&M5Cardputer.Display);  // Sprite for internal display (240x135)

// =============================================================
// YOUR VARIABLES HERE
// =============================================================
int counter = 0;  // Example variable

// =============================================================
// SETUP - Run once at startup
// =============================================================
void setup() {
    // Start serial for debugging
    Serial.begin(115200);
    Serial.println("Starting Dual Display Template...");

    // STEP 1: Initialize Cardputer (INTERNAL display first!)
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);

    // Create sprite for internal display
    intSprite.createSprite(240, 135);
    intSprite.setTextSize(2);

    // STEP 2: Initialize external ILI9341 display
    delay(100);  // Wait for power stabilization
    externalDisplay.init();
    externalDisplay.setRotation(7);  // Try 0, 1, 3, or 7 if mirrored
    externalDisplay.fillScreen(TFT_BLACK);

    // STEP 3: Create sprite for external display
    extSprite.setColorDepth(16);
    extSprite.createSprite(320, 240);
    extSprite.setTextSize(2);
    extSprite.setTextColor(TFT_WHITE);

    Serial.println("Setup complete!");
}

// =============================================================
// LOOP - Runs repeatedly
// =============================================================
void loop() {
    // Update keyboard state
    M5Cardputer.update();

    // =========================================================
    // YOUR EXTERNAL DISPLAY CODE HERE (320x240)
    // =========================================================
    extSprite.fillScreen(TFT_BLACK);

    // Example: Draw something on external display
    extSprite.fillRect(0, 0, 320, 40, 0x10A2);
    extSprite.setTextColor(TFT_WHITE);
    extSprite.setTextSize(2);
    extSprite.setCursor(80, 10);
    extSprite.print("EXTERNAL DISPLAY");

    extSprite.setTextSize(3);
    extSprite.setTextColor(TFT_CYAN);
    extSprite.setCursor(120, 100);
    extSprite.print(counter);

    extSprite.setTextSize(1);
    extSprite.setTextColor(TFT_LIGHTGREY);
    extSprite.setCursor(20, 220);
    extSprite.print("External: 320x240 | Edit this template!");

    // Push to external display (REQUIRED!)
    extSprite.pushSprite(0, 0);

    // =========================================================
    // YOUR INTERNAL DISPLAY CODE HERE (240x135)
    // =========================================================
    intSprite.fillScreen(TFT_BLACK);

    // Example: Draw something on internal display
    intSprite.fillRect(0, 0, 240, 25, TFT_DARKGREY);
    intSprite.setTextColor(TFT_WHITE);
    intSprite.setTextSize(1);
    intSprite.setCursor(5, 5);
    intSprite.print("INTERNAL DISPLAY");

    intSprite.setTextSize(2);
    intSprite.setTextColor(TFT_YELLOW);
    intSprite.setCursor(90, 60);
    intSprite.print(counter);

    intSprite.setTextSize(1);
    intSprite.setTextColor(TFT_LIGHTGREY);
    intSprite.setCursor(10, 115);
    intSprite.print("Internal: 240x135");

    // Push to internal display (REQUIRED!)
    intSprite.pushSprite(0, 0);

    // =========================================================
    // YOUR INPUT HANDLING HERE
    // =========================================================
    if (M5Cardputer.Keyboard.isChange()) {
        // Example: Increase counter when ENTER pressed
        if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
            counter++;
            Serial.printf("Counter: %d\n", counter);
        }

        // Example: Reset when BACKSPACE pressed
        if (M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE)) {
            counter = 0;
        }
    }

    // Small delay for ~20 FPS
    delay(50);
}