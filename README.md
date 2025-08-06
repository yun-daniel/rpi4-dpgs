# Dynamic Parking Guidance System on RPi4 Server

## Project Overview

### Project
The **Dynamic Parking Guidance System** is a real-time parking assistance solution based on **expected departure status**. Unlike conventional systems that only indicate available spaces, this system also provides information about parking slots that are expected to be vacated soon, using LED displays. The main objective is to **reduce internal congestion** by offering more intelligent and proactive guidance to drivers.

### Background
In large indoor parking facilities such as those in shopping complexes or public institutions, it is common for drivers to repeatedly circle inside the parking lot (known as cruising for parking) because they cannot easily locate available spaces after entering.
This leads to several issues:
- Internal traffic congestion within the parking lot
- Increased risk of accidents
- Entry delays and user inconvenience

To address these challenges, the system provides:
- Real-time, map-based parking status information displayed on LED screens installed at each floor entrance
- Drivers can check vacant spots before entering, allowing them to move directly to an available area
- Parking administrators can monitor each floor's occupancy status and CCTV footage in real-time through a Client Application

### Key Features
1. Detection of vehicle and parking slot state classification
2. Parking Guidance via LED Display
3. CCTV video processing and real-time streaming
4. Integrated management server to handle subsystems, communication, and processing
5. GUI-based Client Application
