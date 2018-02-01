# ClickClackClock
This was a hobby project to create an impractically loud and large clock constructed from salvaged electromechanical seven-segment flip digits. To be slightly less obnoxious, the clock automatically sleeps in dark rooms, and wakes up with the correct time when exposed to light -- great for people who like to wake up with the dawn! 
<!--- The clock can also be put into a countdown/stopwatch mode, appropriate for games, New Year's Eve, the apocalypse, etc. --->

The [flip digits](http://www.scoretronics.com/components/Digits_Brochure.pdf) came from a retired aquatics scoreboard/timing system. The segments change position when current is pulsed through a magnetic coil. 

## Software
__Required dependencies:__

**_More info TBA_**


## CAD Contents
We designed custom 2-layer PCBs for this project using [Eagle](https://www.autodesk.com/products/eagle/overview) and fabbed them using [OSHPark](https://oshpark.com/), with all through-hole components for ease of assembly:
- __clock_driver_board:__ Drives a single digit, and can be daisy-chained together for as many digits as needed.
- __clock_motherboard:__ Slightly larger than the Arduino UNO but with the same microcontroller and pad layout for future shield compatibility. Integrates the standard DS1307 RTC on the board and controls the whole system - digit drivers, buttons, photoresistor, etc.
- __clock_system:__ Schematic that describes the overall system of PCBs and auxiliary components.

## Assembly
The digits are supported in a frame made from MDF. 

**_More info TBA_**


