# Centarus Modbus RTU Project

This project implements a **Modbus RTU** interface on the **nRF52840DK** using the **Zephyr RTOS**. It uses a flexible build system to switch between **Client** and **Server** roles using Kconfig fragments and Devicetree overlays.

---

## 1. Zephyr Environment Setup

This project requires a standard Zephyr development environment. Follow the official [Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html) for detailed instructions.

### Install System Dependencies (Linux)

```bash
sudo apt update
sudo apt install --no-install-recommends git cmake ninja-build gperf \
  ccache dfu-util device-tree-compiler wget \
  python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file \
  make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1
```

### Initialize the Workspace

```bash
# Install west
pip3 install --user -U west

# Initialize zephyrproject folder
cd ~
west init zephyrproject
cd zephyrproject
west update
west zephyr-export

# Install python requirements
pip3 install --user -r ~/zephyrproject/zephyr/scripts/requirements.txt
```

### Install Zephyr SDK

```bash
cd ~
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.5/zephyr-sdk-0.16.5_linux-x86_64.tar.xz
tar xvf zephyr-sdk-0.16.5_linux-x86_64.tar.xz
cd zephyr-sdk-0.16.5
./setup.sh
```

---

## 2. Install nRF-Util (Nordic Tools)

`nrfutil` is required for device discovery and flashing on Nordic hardware.

**Download:** Get the Linux x86_64 binary from the [Nordic nRF-Util Page](https://www.nordicsemi.com/Products/Development-tools/nRF-Util).

### Install to Path

```bash
chmod +x nrfutil
sudo mv nrfutil /usr/local/bin/
```

### Install Subcommands

```bash
nrfutil install device
nrfutil install completion
```

---

## 3. Hardware Configuration

The application is configured via `app.overlay` to use the following hardware mapping:

| Parameter          | Value                          |
| ------------------ | ------------------------------ |
| **Peripheral**     | UART1                          |
| **Baud Rate**      | 19200                          |
| **Data Bits/Parity** | 8N1 (Default)                |
| **Flow Control**   | RS-485 via DE Pin              |
| **DE Pin**         | GPIO 0, Pin 19 (Active High)   |

---

## 4. Building and Flashing

The project uses `OVERLAY_CONFIG` in `CMakeLists.txt` to determine the Modbus role at compile-time.

### Build as Client

```bash
west build -p -b nrf52840dk/nrf52840 -- -DOVERLAY_CONFIG="client.conf"
```

### Build as Server

```bash
west build -p -b nrf52840dk/nrf52840 -- -DOVERLAY_CONFIG="server.conf"
```

### Flash the Board

```bash
west flash
```
