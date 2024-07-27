# ESP32 MSC to SPI Flash demo

<p align="center">
<img src="photo_2024-07-27_22-37-08.jpg" width="700"/>
</p>

This code makes **1MB working Flash drive from ESP32-S2**.    <br>
How it works:    <br>
https://youtu.be/wCQvbELoyOI    <br>
  <br>
  <br>

**Here's the point:**    <br>
MSC Is access memory by LBA num and Offset.     <br>
LBA=512bytes.    <br>
but    <br>
Flash chip is accessed by pages. And before every WRITE, there's necessary to CLEAR all corresponding page.    <br>
Page = 4096 bytes.    <br>
    <br>


How to make USB flash drive from ESP32-S2?    <br>
ESP32-S2 USB MSC to SPI Flash Demo    <br>
This example shows:    <br>
1) How to get partition     <br>
2) How to read directly from partition     <br>
3) How to write directly to partition     <br>
4) How to initialize USB MSC protocol    <br>
5) How to convert LBA-based access requests from USB to pages-based access to SPI Flash     <br>
6) How to build and connect my example     <br>
7) How to format and use this drive    <br>