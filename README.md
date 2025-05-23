# Super Serial Card

This project is based on [A2Pico](https://github.com/oliverschmidt/a2pico).

This firmware does not __emulate__ a [Super Serial Card (SSC)](https://en.wikipedia.org/wiki/Apple_II_serial_cards#Super_Serial_Card_(Apple_Computer)). Rather, it is __compatible__ to a SSC, primarily because it uses the original, unmodified SSC 6502 firmware. The main differences from a SSC are:
* There is a USB interface instead of an RS-232 interface.
* There is no UART (not even a virtual one). Therefore, the usual connection settings like `9600 Baud` are meaningless.
* The actual connection speed is implicitly always _the highest that both communicating parties can achieve without data loss_. This is usually significantly faster than anything possible with a SSC (incl. its `115.200 Baud` mode).
* Hardware handshake lines are not supported, as they usually don't make any sense without an UART.
* Interrupts are not supported, as they usually don't make any sense without an UART.

Please ensure the A2Pico `USB Pwr` is set to `off` when using this firmware! 

## Getting Started

If you're not sure yet want you want to do with this firmware, please follow these steps:
 
* Install [PuTTY](https://www.putty.org/) on a PC. Connect the A2Pico to the PC. This opens a new virtual serial port (called `COMx` in Windows) on the PC.

* On the Apple II, enter BASIC, type `PR#<n>` followed by `IN#<n>` (where `<n>` is A2Pico's slot). Now press `Ctrl-I`. You'll see the `APPLE SSC:` prompt. Now type `1D` (where `D` is the uppercase letter!).

* On the PC, open PuTTY, set its `Connection type` to `Serial`, enter A2Pico's virtual serial port under `Serial line`, and click `Open`. You can now type both on the Apple II and the PC, and see the output both on the Apple II and the PC. When done, quit PuTTY.

* On the PC, open a command prompt and enter `plink -serial COMx` (where `COMx` is A2Pico's virtual serial port). Any output generated on the Apple II will be displayed in the command prompt. It can be redirected into a file or piped into another program. When done, type `Ctrl-C` to quit.

## Details

The SSC implemented by this firmware is set to `Printer Mode` (as opposed to `Communication Mode`). The main reason for this is that serious communication needs require a dedicated Apple II program. These programs ignore the SSC settings anyway.

As mentioned above, the usual UART connection settings are meaningless. The only exception is the number of data bits. This was already used above to limit the data bits to 7 via the SSC command `1D`.
