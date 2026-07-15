<img src="https://github.com/user-attachments/assets/f1d43968-a35d-4d61-a50d-f554358caff8" width="300">
<img src="https://github.com/user-attachments/assets/03316bd5-34af-4cd4-a775-c4ee38892583" width="300">
<img src="https://github.com/user-attachments/assets/848aee54-d009-4d05-8b49-f248022906e4" width="300">
<img src="https://github.com/user-attachments/assets/c4f0300b-de68-4fb9-9e47-872b19d72af5" width="300">
<img src="https://github.com/user-attachments/assets/228a822e-c34e-4dff-bb57-e918d0416ad8" width="300">


# 🖥️ M5Stack Cardputer + ILI9341 Dual Display

https://docs.m5stack.com/en/core/Cardputer-Adv
https://www.espressif.com/en/products/socs/esp32-s3
https://www.arduino.cc/en/software

*Complete guide and minimal example for using an external ILI9341 2.4" display alongside the Cardputer's built-in ST7789 screen*

## 📋 Overview
This project demonstrates how to drive two displays simultaneously on the M5Stack Cardputer ADV:
* **Internal:** ST7789 240×135 (built-in)
* **External:** ILI9341 240×320 (via GPIO header)

## 🎯 Features
* ✅ Dual Display Support - Run both screens independently
* ✅ Hardware SPI
* ✅ Sprite Buffering - Smooth, flicker-free rendering
* ✅ Critical Init Order - Proper sequence prevents conflicts
* ✅ Minimal Example - Clean, well-commented starter code

## 🔌 Wiring Diagram

### Connection Table

| Carduter ADV | ILI9341 | Function |
| :--- | :--- | :--- |
| Pin 2 | 5VIN | VCC |
| Pin 4 | GND | GND |
| Pin 13 | GPIO 5 | CS |
| Pin 12 | GPIO 13 | RESET |
| Pin 14 | GPIO 15 | DC |
| Pin 9 | GPIO 14 | MOSI |
| Pin 7 | GPIO 40 | SCK |
| Pin 6 | 5VOUT | LED |

## 🚀 Quick Start

### Download this repo
### Wiring in docs .html file

### 1. Hardware Setup
Connect wires according to the table

### 2. Software Requirements
**Arduino IDE Libraries Required:**
* M5Cardputer (by M5Stack)
* M5GFX (by M5Stack)
* M5Unified (by M5Stack)

### 3. Upload Code
* Open `dual_screen_test.ino` in Arduino IDE
* Select **M5Cardputer** board
* Select your **COM port**
* **CPU Frequency:** 240mhz
* **Flash mode:** QIO 80mhz
* **Flash size:** 8mb
* **PSRAM:** OPI PSRAM (If you need in your firmware)
* Upload to your Cardputer

### 4. Critical Initialization Order
```
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

Project Structure
Card_Dual_Screen/
├── examples/
│   └── Dual_screen_test/
│       ├── Dual_screen_test_tools.ino   
│       └──Templete.ino
├── docs/
│   └── Cardputer + ILI9341 Dual Display Setup Guide.html             # Interactive wiring guide     
├── README.md                           # This file
└── LICENSE                             # MIT License




🔄Display Rotation
// Recommended rotation for ILI9341
externalDisplay.setRotation(7);

🐛Troubleshooting


- White Screen
 [] Check 5V with a multimeter

 -Mirrored/Flipped image
  [] Try 1, 3, 5, or 7 values
  7 is recommended

  -No Backlight
  [] Check 5VOUT

📜 License
This project is licensed under the MIT License - see the LICENSE file for details.

Contact
Email: guilhermecamargo00@gmail.com

Who made the 3D Shell (AndyAICardputer):
https://www.youtube.com/@AndyLong14\
https://www.youtube.com/watch?v=-N_J1kQnoHg
https://github.com/AndyAiCardputer/zx-spectrum-cardputer-external
Go to his channel and subscribe :D

Contributions are welcome! Please feel free to submit a Pull Request. ⭐ Star this repo if you found it helpful!

Made with ❤️ for the M5Stack community


   
     
