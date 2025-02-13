# 👾 PiTalkster

### 🍿 Disclaimer

> _Before I started this project, I decided to write a short disclaimer to present some context of its creation. I wanted to create my own mini-product, starting with the definition of "customer" requirements (that is, de facto mine) until the implementation of a prototype that meets these requirements. Since now there is hype on LLM (especially curious about models that can even be run on Raspberry Pi), I came up with the idea of making a mini communicator/chat on the RPi, with the ability to enter voice prompts, and receiving the result on mini-display or using audio. I would like to go through the architecture design stage, and write the main app mostly in C (due to the potential interaction with HW, e.g. LCD and microphone), preferably also using TDD. Time will tell if this works out!_

![Status](https://img.shields.io/badge/Project_status-Defining_requirements-blue)

---

### 🦐 Customer requirements

The system: 
- must be very simple to use,
- should allow user to speak prompts using the microphone,
- must run local AI model to process provided prompt (without requiring an internet connection),
- should display results on the LCD screen,
- must provide basic user controls,
- should include a menu for additional functions such as configuration and history viewing,
- could be designed for desktop use (does not need to be portable).

> _Customer does not impose a specific AI model, menu view, configuration/history options etc._

### 💾 Technical requirements

- Platform: Raspberry Pi 4B+
- Power: 5V/3A, low power consumption is not a priority
- Display: LCD IPS 1.3" 240×240 with ST7789 controller
- Audio input: SPH0645LM4H-B microphone MEMS I2S
- Controls: min. 3 physical buttons (2 for navigation, 1 for applying)
- LLM model: deepseek-r1:1.5b
- Enclosure: optional for demo

### ☑️ Acceptance criteria

*Comming soon...*

---

### 🪅 Implementation

*Comming soon... It will be fun!*

### 📟 Usage

*Comming soon...*
