# NetTime Ecosystem - Client Display (V1.0.0-alpha)

[![License: MIT](https://img.shields.io/badge/License-MIT-42b983.svg)](https://opensource.org/licenses/MIT)

A **smart, network-synchronized OLED display client** based on **ESP8266**. This device acts as a peripheral in the NetTime ecosystem, providing real-time clock, date, and environmental data by synchronizing with a **Local Master Server** or falling back to **NTP**.

---

### 🚀 Key Features

* **Hybrid Synchronization:** Automatically seeks a local Master Server via **UDP** for low-latency updates. If the server is unavailable, it seamlessly falls back to **NTP**.
* **WiFi-Centric UI:** The interface is designed to prioritize network status. **"noWifi" alerts** are tied directly to hardware connectivity to prevent confusing error messages.
* **Placeholder Mechanism:** Displays `--:--` during initial boot or data sync to maintain UI integrity.
* **OTA (Over-The-Air) Updates:** Full support for wireless firmware updates via **ArduinoIDE** or **PlatformIO**.
* **Modular Page System:** Cycles between **Clock/Date** and **Weather/Environment** pages.

---

### 🛠️ Hardware Requirements

* **ESP8266** (NodeMCU or Wemos D1 Mini)
* **SSD1306 / SH1106** OLED Display (128x64)
* *(Optional)* **Local Master Server** for advanced features.

---

### 📂 Project Structure

* **`NetTime-Client-Display.ino`**: Main entry point and system setup.
* **`SyncLogic.ino`**: The heart of the hybrid UDP/NTP sync mechanism.
* **`DisplayAssets.ino`**: Graphics, logos, and UI components.
* **`Clock.ino` & `Weather.ino`**: UI page rendering logic.
* **`secrets_template.h`**: Template for network and API credentials.

---

### 🔧 Installation

1. **Clone** this repository.
2. **Rename** `secrets_template.h` to `secrets.h`.
3. **Fill in** your `WIFI_SSID`, `WIFI_PASS`, and `OTA_PASS`.
4. **Flash** to your ESP8266 using the Arduino IDE.

---

### 📄 License

This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for details.
