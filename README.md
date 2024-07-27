# ESP32 MSC to SPI Flash demo

This code makes 1MB working Flash drive from ESP32-S2.
How it works:
https://youtu.be/wCQvbELoyOI


Here's the point:
MSC Is access memory by LBA num and Offset. 
LBA=512bytes.
but
Flash chip is accessed by pages. And before every WRITE, there's necessary to CLEAR all corresponding page.
Page = 4096 bytes.