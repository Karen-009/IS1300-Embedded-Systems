# Embedded Systems and Real-Time Programming

This repository contains coursework, labs, and projects focused on embedded systems development, microcontrollers, and real-time programming.

The projects combine both hardware and software design to demonstrate how embedded systems are developed, implemented, and tested in time-critical environments.

---

## Project Overview

The course introduces the development process of embedded systems, including system architecture, low-level programming, hardware interaction, and real-time operating systems.

Main topics include:
- Embedded system design
- Microcontrollers and processor architecture
- Hardware and software integration
- Input/output communication
- Real-time systems
- Process scheduling
- Inter-process communication
- System testing and validation

The repository includes practical labs and programming assignments using embedded development tools and real-time operating systems.

---

## Technologies Used

- C
- Embedded C
- Real-Time Operating Systems (RTOS)
- Microcontrollers
- Hardware Interfaces
- Embedded Development Tools

---

## Key Concepts

- Embedded System Architecture
- Real-Time Systems
- Hard and Soft Real-Time Constraints
- Scheduling Algorithms
- Process Communication
- I/O Communication
- Microcontroller Programming
- Hardware-Software Integration
- System Testing and Debugging

---

## Learning Outcomes

Through this course, the projects demonstrate:
- Design and implementation of embedded systems
- Development of low-level software for microcontrollers
- Analysis of real-time constraints and scheduling strategies
- Hardware and software integration
- Use of real-time operating systems in time-critical systems
- Testing, debugging, and system validation
- Technical documentation and reporting

---

## How to Run

1. Clone the repository
2. Open the project in the appropriate embedded development environment
3. Build and upload the software to the target hardware or simulator
4. Run and test the system

Compilation and execution steps may vary depending on the microcontroller platform and development tools used.

---

## About the Course

This course focuses on the design and implementation of embedded and real-time systems, combining software engineering principles with hardware-oriented development for time-critical applications.

## Project Overview
Each task under the Application Layer has a file for itself. Under that file the main funcation that puts it all together is named after the file name. The other functions needed for   the main function is declared and implemented in the same file or its corresponding header file.

Service Layer: Handels the inter-task sync & communication

Drivers Layer: Handels the hardware controls; SPI, I2C, GPIO etc.

HAL Layer: CubeMX-generated initialization
