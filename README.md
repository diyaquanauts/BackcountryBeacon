# Backcountry Beacon

![Backcountry Beacon Device](https://i.ibb.co/DCBghrZ/thumbnail.png)

The Backcountry Beacon is a portable, offline mapping and file server device designed for adventurers who need reliable access to topographical maps and important files without relying on cellular or internet connectivity. Unlike traditional phone apps, which require constant updates and storage management, the Backcountry Beacon offers a hassle-free solution. You can compile large areas of maps, store them on a microSD card, and trust that the device will work perfectly even after years of storage. Additionally, the device serves files over Wi-Fi, allowing you to access your maps and documents on any connected device. Built on the LilyGo S3 USB dongle, this lightweight, USB-powered device is ideal for long-term use, providing you with reliable maps and file access whenever and wherever you need them.

### Repository Structure
The Backcountry Beacon project is organized into several key folders, each serving a specific purpose:

#### 1. `firmware/`

This folder contains all the necessary source code, configuration files, and scripts to build and deploy the firmware onto the LilyGo S3 USB dongle. This is where you'll find everything related to the core functionality of the Backcountry Beacon device.

#### 2. `parts3D/`

This folder includes 3D models and related resources for any physical components or enclosures used with the Backcountry Beacon. It contains design files, 3D printable parts, and scripts to generate README files and thumbnails for easy reference.

#### 3. `tileDownloader/`

This folder contains scripts and data related to downloading and managing offline map tiles. It includes tools to calculate storage requirements, cache map tiles, and manage the geographic data used by the Backcountry Beacon to provide offline map services.
