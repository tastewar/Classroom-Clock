# Setting the Classroom Clock over Serial

This guide explains how to set the clock's UTC time or change its bell offset from macOS Terminal, without replacing the clock firmware.

## Before you connect

Connect the clock to the Mac with its USB cable. Close any other program that may be connected to the clock; only one program can use its serial port at a time.

The clock uses **9600 baud**. Opening its serial port may restart the Nano. After it restarts, the clock prints a menu:

```text
Classroom Clock setup: Set time (T) or change bell offset (B)?
Enter T, B, or ? for this menu.
```

If you do not see the menu, type `?` and press Enter to print it again.

## Find the serial port

In macOS Terminal, list serial ports with:

```sh
ls /dev/cu.*
```

If there are several ports, run the command once with the clock unplugged, then again with it plugged in. The new entry is usually the clock. Use the `/dev/cu.*` name, not a `/dev/tty.*` name.

## Use macOS Terminal with `screen`

macOS includes the `screen` serial terminal. Replace the example port below with the one you found:

```sh
screen /dev/cu.usbserial-10 9600
```

Wait for the menu. Type `T` to set the hardware clock or `B` to change the bell offset, then press Enter. The firmware echoes what you type. Type `?` and press Enter if you need the menu again.

For `T`, enter the year, month, day, hour, minute, and second in **GMT/UTC**, using a 24-hour hour value. Check the proposed date and time, then enter `Y` to apply it or `N` to cancel.

For `B`, enter a signed offset from `-3600` to `3600` seconds. A positive value makes the displayed/schedule time run behind real time; a negative value makes it run ahead. Confirm with `Y` to save it, or `N` to cancel. The saved value remains after power is removed. After either change is confirmed or canceled, the clock returns to the setup menu and keeps displaying the time.

When finished, exit `screen` with **Control-A**, then **K**, then **Y**. This closes the serial connection and releases the port.

## If the port is busy

Close any other serial terminal session connected to the clock, then reconnect. If the menu is missing after connecting, press Enter after sending `?`.
