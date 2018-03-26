# H1BR60 Module #

**H1BR60 is a nifty little SPI-based, micro-SD card module** based on STM32F0 MCU and running Fatfs file system. It is part of Hexabitz modular prototyping system.

- Use as a stand-alone data-logger or easily integrate with other hardware via serial interfaces.
- Setup and control logging via a Command Line Interface (CLI).
- Log digital inputs, memory locations, program variables, serial streams and button actions.
- Start multiple simultaneous logs with different log variables each.
- Firmware has Fatfs embedded file system and advanced logging functionality (event logs, rate logs, sample-based, time-based, sequential naming, etc.)
- Supports SDHC micro-SD memory cards (FAT32, up to 32GB size).
- Program advanced C code with our easy-to-use APIs.
- Connect to external hardware or combine with other Hexabitz modules!

**Note: This module name/part number (PN) was updated from H05R00 to H1BR60.**

===============================================

## About Hexabitz ##

Hexabitz is a new kind of electronic prototyping platforms with game-changing modularity and a hint of biomimicry. Hexabitz modules connect with each other using a novel edge-soldering system and allow you to build tidy, compact and completely re-configurable electronic boards. **Learn more about Hexabitz [here](https://www.hexabitz.com/)**.

===============================================

## Submodules ##

- Bitz Operating System - [BOS](https://bitbucket.org/hexabitz/bos)
- [Thirdparty](https://bitbucket.org/hexabitz/thirdparty)

## Useful Links ##

- Buy H1DR60 module on [Tindie Store](https://www.tindie.com/products/11614/)
- Check Hexabitz main [website](https://www.hexabitz.com/) and available and planned [modules](https://www.hexabitz.com/modules/).
- Read the intro to Hexabitz modular prototyping platform on [Hackaday.io](https://hackaday.io/project/76446-hexabitz-modular-electronics-for-real)
- Overview of hexabitz software [architecture](https://hackaday.io/project/76446-hexabitz-modular-electronics-for-real/log/117213-hexabitz-software-architecture)
- Check and in-depth overview of Hexabitz code in this series of [article]().
- Hexabitz demo [projects](https://hackaday.io/list/87488-hexabitz-projects)

## Documentation ##
We developed a new form of documentation we call it the Factsheet: it's a mix between a hardware datasheet listing electrical properties and a software cheat-sheet showing you quickly the most important software functions and commands. Factsheets are color-coded and designed to be printed double-sided. We think they will come in handy for your Hexabitz projects. Let us know if you like them! 

- [Module factsheet](https://d3s5r33r268y59.cloudfront.net/datasheets/11614/2018-03-04-21-48-41/H05R00%20Factsheet.pdf)

## Design Files ##
- Module schematics [[PDF](https://www.dropbox.com/s/31tct2bwhd8ben2/H05R00%20Schematics.pdf?dl=0)]

===============================================

## How do I get set up? ##

### If you want to load a precompiled HEX file ###

1- Navigate to MDK-ARM/Objects and load the *Module 1.hex* HEX file using any firmware update method described [here]().

### If you want to compile the code: ###

1- If you didn't already, download Keil uVision MDK toolchain from [here](http://www2.keil.com/mdk5/uvision/). Get your free [license](http://www.keil.com/) for STM32F0 MCUs!

2- Download or clone this repository source code and open the uVion projext in MDK-ARM folder (it has .uvprojx extension).

3- If you are loading a single module, simple compile the code and load it to module MCU via one of the firmware update methods explaind [here]().

4- If you are loading multiple modules of the same type (connected in an array), then manually modify the module ID in Options for Target >> C/C++ >> Preprocessor Symbols >> Define >> _module=x (where x the module ID) and in Output >> Name of Executable. Recompile the project and load each module according to its ID. You can also create multiple targets as explained in the firmware update [guide]().

### How do I test? ###

1- If code is loaded correctly, you should see one or few indicator LED blinks when you power-cycle.

2- Use [RealTerm](https://sourceforge.net/projects/realterm/) or any other terminal emulator with a USB-to-USRT cable to access the module CLI. Learn about using the CLI [here]().

3- Check available CLI commands by typing *help* or use the module [factsheet](https://d3s5r33r268y59.cloudfront.net/datasheets/11614/2018-03-04-21-48-41/H05R00%20Factsheet.pdf). Make sure the factsheet BOS version number (at the footer) matches the source code version you have.

### How do I update the source code for an old project? ###

1- If your project follows portability guidelines, then just keep all files in the *User* folder and replace all other folders with the newer source code.

### Where should I add my code? ###

To preseve maximum code portability between projects, we advise you to:

1- Keep all you custom source and header files in the *User* folder.

2- Add your code to the *FrontEndTask()* in *main.c* and add other custom function there including custom interrupt callbacks.

3- Add any external header files you want to include to the *project.h* file.

4- Add any topology header files to *User* folder and include them in *project.h*.

===============================================

## Software FAQ ##

### Q: ###
A:

## Hardware FAQ ##

### Q: How do I use H1BR60 to add logging capability to my projects?
A: H05R00 module logs all sorts of signals connected to its array ports. You can stream digital data from external hardware using serial ports (UART), connect digital sensors (3.3V max) directly, connect external switches of any type (mechanical, optical, magnetic) or momentary and toggle buttons.

### Q: Can I log same signal in different ways?
A: Yes! You can start two simultaneous logs of different types (rate or event) and log same or different signals.

### Q: Can I log a complicated condition/combination of signals?
A: Yes! You can write C code to combine signals in complex ways and write the output to an internal RAM buffer. This buffer / memory location can then be logged either on rate- or event-basis.

### Q: What's the maximum logging rate in H1BR60?
A: Maximum logging rate is 1KHz. However, it depends on number of simultaneous logs and number of variables per log. Usually it is several hundred Hz in complicated scenarios.

Check our [website](https://www.hexabitz.com/faq/) for more information or contact us about any questions or feedback!

===============================================

## See also these module repositories ##

- [H01R00](https://bitbucket.org/hexabitz/h01r0) - RGB LED Module

- [H09R00]() - 600VAC / 1.2A Solid State Relay Module

===============================================

## Who do I talk to? ##

* Email us at info@hexabitz.com for any inquiries
* Or connect with us on [Hackaday.io](https://hackaday.io/Hexabitz)

## How do I contribute? ##

* We welcome any and all help you can provide with testing, bug fixing and adding new features! Feel free to contact us and share what's going on in your mind.
* Please send us a pull request if you have some useful code to add!

===============================================

## License ##
This software / firmware is released with [MIT license](https://opensource.org/licenses/MIT). This means you are free to use it in your own projects/products for personal or commercial applications. You are not required to open-source your projects/products as a result of using Hexabitz code inside it.

To our best knowledge, all third-party components currently included with Hexabitz software follow similar licenses (MIT, modified GPL, etc.). We will do our best to not include third-party components that require licensing or have restricted open-source terms (i.e., forcing you to open-source your project). There is no guarantee, however, that this does not happen. If we ever include a software component that requires buying a license or one that forces restrictive, open-source terms, we will mention this clearly. We advise you to verify the license of each third-party component with its vendor. 

## Disclaimer ##
HEXABITZ SOFTWARE AND HARDWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE AND HARDWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE AND HARDWARE.
