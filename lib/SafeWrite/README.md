# SafeWrite

A library for micro controllers to handle writing to SD cards. To avoid potential data loss due to power loss mid-write, the library opens a main and backup file, and each time write is called, the data is written to both files sequentially, therefore ensuring both files are never open at the same time. See the included basic example for useage.

The code has been tested with Teensy 3.6 and an Adafuit Feather M0 with an eternal micro-SD breakout board.