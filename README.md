## **BUKREK — USB Headphone Soundcard**

![System Block Diagram](https://github.com/ArdaNoyanKacar/Headphone_Soundcard/blob/bd16e1275c5a484b2d0ee54fc61b5d528ffb676d/Demo_Picture_Video/Headphone%20Soundcard%20PCBA.jpg)

## **Summary**
**USB Headphone Soundcard** with **integrated audio effects** controlloable in runtime, utilizing **analog Baxandall volume** front end for audio taper volume control. Includes **UART Command Interface + Python GUI** for easier adjustments to audio effects. Able to drive 16Ω+ headphones

---

## **Hardware Overview**

* **MCU:** STM32F407
* **Main Audio Codec:** SGTL5000
* **USB Audio Codec:** PCM2903
* **Volume Control:** Baxandall volume control analog front end (audio taper)

## **Power**
* **16V Board Input** from DC Jack
* **15V Analog Voltage** LDO (For better noise performance)
* **5V** Switching Regulator
* **3V3 Digital Voltage** LDO (For better noise performance)

## **System Block Diagram**
![System Block Diagram](https://github.com/ArdaNoyanKacar/Headphone_Soundcard/blob/55b23ce5163afa6f2f043c1ad53847dd03ad0e53/PCB_Design/Headphone%20Soundcard%20System%20Block%20Diagram.png)

## **Board Layout**
![Board Layout](https://github.com/ArdaNoyanKacar/Headphone_Soundcard/blob/bd16e1275c5a484b2d0ee54fc61b5d528ffb676d/PCB_Design/Headphone%20Soundcard%20Layout.png)


---

## **Audio Pathways**

* **USB → LINEOUT (PCM2703C) → SGTL5000 LINEIN → LINEOUT/HP** — current playback/effects path
* **USB → I²S (STM32) → SGTL5000 I2S IN** — **planned** (conversion on MCU; implementation in progress)

---

## **Feature Highlights**

* **Analog Baxandall Volume Control**
  * **Audio Taper** volume control

* **Runtime Audio Effects**

 ![Soundcard GUI](https://github.com/ArdaNoyanKacar/Headphone_Soundcard/blob/bd16e1275c5a484b2d0ee54fc61b5d528ffb676d/Soundcard_GUI/Soundcard%20Effect%20GUI%20Homepage.png)

  * **5-band EQ**
  * **Bass enhancement**
  * **Surround**
  * **Volume**

* **Host Control & GUI**
  * **UART** device shell
  * **Python GUI** for quick, visual tuning

---

## **Quick Start**

1. **Hardware**
   * Connect board over **USB** (power + audio device)
   * Connect **UART** to PC (e.g., ST-Link VCP or USB-to-UART)

2. **Firmware**
   * Build/flash the STM32F407 project
   * On reset, codec initializes and the UART shell is available

3. **GUI**
   * Install **Python 3** and `pyserial`
   * Run the **Python GUI**, select the serial port, **Connect**
   * Adjust **EQ / Bass / Surround / Volume** live

---

## **UART Commands**

* **help** — list commands
* **version** — print firmware version
* **dumpregs** — dump codec registers (debug)
* **setEQ _b0 b1 b2 b3 b4_** — set 5-band EQ gains (−12…+12 dB)
* **setEQProfile _NAME_** — one of: `flat, rock, pop, classical, rap, jazz, edm, vocal, bright, warm, bassboost, trebleboost, maxsmile, midspike`
* **setBassEnhance _on|off [lr bass]_** — optional `lr 0..63`, `bass 0..127`
* **setSurround _on|off [width]_** — width `0..7`
* **setVolume _N_** — DAC volume percent `0..100`

---

## **Python GUI**

* **Serial connect** to device (port picker)
* **EQ profile** dropdown + **5 EQ sliders**
* **Bass** and **Surround** controls
* **Volume** slider
* **Console** for device responses + **manual command** input

---

 ## Repository Layout
```text
/firmware
├── Core/Src/main.c               # Peripherals, codec init, loop
├── Core/Src/cmd_ctrl.c           # UART command shell
├── Core/Src/sgtl5000.c           # Codec control & effects
└── Drivers/...                   # STM32 HAL

/host
├── soundcard.py                  # Python GUI
├── serial_client.py              # Serial transport
└── codec_client.py               # Command helpers
```



---

## **Current Project Status**

* **Working:** USB → Codec path with runtime effects; UART shell; Python GUI
* **Planned:** USB → **I²S** (STM32) → Codec streaming path

---



