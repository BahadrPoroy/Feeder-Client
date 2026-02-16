# 🕒 NetTime Ecosystem - Feeder Client (V1.1.1) 🐟

![C++](https://img.shields.io/badge/C++-00599C?style=flat-square&logo=c%2B%2B&logoColor=white)
![Firebase](https://img.shields.io/badge/Firebase-ffca28?style=flat-square&logo=firebase&logoColor=black)
![ESP8266](https://img.shields.io/badge/ESP8266-414141?style=flat-square&logo=espressif&logoColor=white)
[![License: MIT](https://img.shields.io/badge/License-MIT-ff4500.svg)](https://opensource.org/licenses/MIT)

A **cloud-connected smart actuator node** based on **ESP8266**. This device is a critical peripheral in the NetTime ecosystem, responsible for executing automated feeding cycles and maintaining a persistent global state via **Firebase Realtime Database**.

---

### 🚀 Key Features

* **Asynchronous State Synchronization:** Monitors cloud-based triggers in real-time. It uses a **Universal State Template** to ensure the feeding status is synchronized across all dashboard clients.
* **Hardware Actuation:** Manages high-precision motor/servo control for the physical feeding mechanism.
* **Decoupled Architecture:** Operates as a standalone agent. While it integrates with the [NetTime-Env-Server](https://github.com/BahadrPoroy/NetTime-Env-Server), it can function independently through direct web-dashboard commands.
* **Persistent Event Logging:** Automatically stamps and uploads `lastFedTime` and `isFed` status to the cloud, ensuring data persistence even after power cycles.
* **Network Resilience:** Features an automated reconnection engine for both WiFi and Firebase services to prevent missed feeding cycles.

---

### 🏗️ System Architecture



The Feeder Client follows the **Publisher-Subscriber** model:
1.  **Listen:** Subscribes to the `isFed` trigger on Firebase.
2.  **Act:** Once triggered, executes the motor rotation sequence.
3.  **Confirm:** Updates the database with the success timestamp and resets the trigger.

---

### 🛠️ Hardware Requirements

* **MCU:** ESP8266 (NodeMCU or Wemos D1 Mini)
* **Actuator:** MG90S Servo or Stepper Motor (Configurable in `HardwareConfig.h`)
* **Power:** 5V External DC (Recommended for motor stability)

---

### 🎨 Industrial Design & Prototyping

To ensure a professional and durable finish, the enclosures for all ecosystem nodes (Server, Feeder, and Display) were **custom-designed and 3D printed**.

* **Custom Enclosures:** Specifically tailored to house the ESP8266, sensors, and actuators while maintaining optimal airflow and structural integrity.
* **Rapid Prototyping:** Iterative design process to ensure perfect fitment for the MG90S servo and OLED mounts.
* **Integration:** The hardware is not just a circuit on a breadboard; it is a fully enclosed, desk-ready consumer electronics prototype.

---

## 📂 Project Structure

```
Feeder_Client
├── NetworkManager.h  # Firebase, WiFi, UDP & OTA Updates
```

---

🌐 The NetTime Ecosystem
This client is designed to work within the NetTime IoT Framework. While it can operate as a standalone node, it reaches its full potential when paired with other ecosystem components:

[NetTime-Env-Server](https://github.com/BahadrPoroy/NetTime-Env-Server): The central hub that provides localized time, environmental data, and acts as the primary network master.

[NetTime-Client-Display](https://github.com/BahadrPoroy/NetTime-Client-Display): A dedicated monitoring node that visualizes real-time climate data and synchronized time on a peripheral OLED/TFT display.

NetTime-Feeder-Client (This Repo): The ecosystem's actuator node, responsible for cloud-triggered hardware automation and persistent state logging.

[Web Dashboard](https://bahadrporoy.github.io/NetTime-Env-Server/): A real-time interface (integrated within the Server) that allows manual overrides and global monitoring of all connected clients.

---

### 🔧 Installation & Deployment

1.  **Clone** this repository.
2.  **Configure Credentials:** Create a `secrets.h` file based on the template provided.
3.  **Hardware Calibration:** Adjust motor sweep angles or steps in `config.h`.
4.  **Flash:** Deploy via Arduino IDE or PlatformIO.

---

### 📄 License
This project is licensed under the **MIT License**.