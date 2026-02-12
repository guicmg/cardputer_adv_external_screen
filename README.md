🖥️ M5Stack Cardputer + ILI9341 Dual Display
https://docs.m5stack.com/en/core/Cardputer-Adv
https://www.espressif.com/en/products/socs/esp32-s3
https://www.arduino.cc/en/software

*Complete guide and minimal example for using an external ILI9341 2.4" display alongside the Cardputer's built-in ST7789 screen*

📋 Overview
This project demonstrates how to drive two displays simultaneously on the M5Stack Cardputer ADV:
Internal: ST7789 240×135 (built-in)
External: ILI9341 240×320 (via GPIO header)


🎯 Features
✅ Dual Display Support - Run both screens independently
✅ Hardware SPI
✅ Sprite Buffering - Smooth, flicker-free rendering
✅ Critical Init Order - Proper sequence prevents conflicts
✅ Minimal Example - Clean, well-commented starter code

🔌 Wiring Diagram

Connection Table

Carduter ADV		ILI9341	Function
Pin 2	  5VIN	         VCC
Pin 4	  GND	           GND	
Pin 13	GPIO 5	       CS	
Pin 12	GPIO 13	       RESET	
Pin 14	GPIO 15	       DC	
Pin 9	  GPIO 14	       MOSI	
Pin 7	  GPIO 40	       SCK	
Pin 6	  5VOUT	         LED

🚀 Quick Start
1. Hardware Setup
Connect wires according to the table

2. Software Requirements
# Arduino IDE Libraries Required:
- M5Cardputer (by M5Stack)
- M5GFX (by M5Stack)
- M5Unified (by M5Stack)

3. Upload Code
Open Cardputer_ILI9341_DualDisplay.ino in Arduino IDE
Select M5Cardputer board
Select your COM por
CPU Frequency 240mhz
Flash mode QIO 80mhz
Flash size 8mb
PSRAM OPI PSRAM (If you need in your firmware)
Upload to your Cardputer

4. Critical Initialization Order
// ⚠️ THIS SEQUENCE IS MANDATORY!

// STEP 1: Initialize Cardputer (Internal First!)
M5Cardputer.begin(cfg, true);

// STEP 2: Initialize External ILI9341
delay(100);  // Wait for power stabilization
externalDisplay.init();
externalDisplay.setRotation(7);

// STEP 3: Create Sprite Buffers
intSprite.createSprite(240, 135);   // Internal resolution
extSprite.createSprite(320, 240);   // External resolution

📁 Project Structure
cardputer-ili9341-dual-display/
├── examples/
│   └── Minimal_Dual_Display/
│       └── Minimal_Dual_Display.ino    # Starter example
├── docs/
│   └── pinout_diagram.html             # Interactive wiring guide     
├── README.md                           # This file
└── LICENSE                             # MIT License

🎨 Display Specifications
Feature	Internal (ST7789) | External (ILI9341)
Resolution	240 × 135 | 240 × 320
Controller	ST7789 | ILI9341
Interface	SPI (dedicated) | SPI (shared)
Colors	16-bit RGB565 | 16-bit RGB565
SPI Speed	80 MHz | 20 MHz
Buffer	Sprite recommended | Sprite required

🔄 Display Rotation

Value	Orientation	Mirror	Description
0	Portrait (0°)	No	Vertical
1	Landscape (90°)	No	Standard horizontal
2	Portrait (180°)	No	Upside-down
3	Landscape (270°)	No	Reverse horizontal
7	Landscape (90°)	Yes	Recommended

// Recommended rotation for  ILI9341
externalDisplay.setRotation(7);

🐛 Troubleshooting
White Screen
[ ] Check 5V output with multimeter (should be ~5.1V)
[ ] Check all SPI connections (MOSI, SCK, CS)

Mirrored/Flipped Image
[ ] Try rotation values: 1, 3, 5, or 7

Slow Performance
[ ] Use sprite buffers (don't draw directly to display)
[ ] Increase SPI frequency: b.freq_write = 40000000; (test stability)

No Backlight
[ ] Check the 5V


📜 License
This project is licensed under the MIT License - see the LICENSE file for details.

Contact
Email: guilhermecamargo00@gmail.com

Who made the 3D Shell (AndyAICardputer):
https://www.youtube.com/@AndyLong14\
https://www.youtube.com/watch?v=-N_J1kQnoHg
Go to his channel and subscribe :D

Contributions are welcome! Please feel free to submit a Pull Request.
⭐ Star this repo if you found it helpful!
Made with ❤️ for the M5Stack community
