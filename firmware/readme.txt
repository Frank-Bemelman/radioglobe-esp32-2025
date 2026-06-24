These two bin files for globe and display can be uploaded using a chrome browser on PC and a webtool

The software can be updated for both globe and puck. This requires a bit of an effort. 
Remove the globe’s ESP32 module, and connect it with a USB cable to your PC. 

Don’t even try with a Mac, use a PC and use Chrome browser. 
Browse to: https://thelastoutpostworkshop.github.io/ESPConnect/

Use the blue connect button, select the USB port, and connect.
Select Flash Tools on the left.
Load the provided bin file with the new software.
Select Recommended Offsets -> App0 0x10000 3MB
Do NOT select anything else!!!
Hit the large FLASH FIRMWARE bar/button.
Software is now loaded into the device. This may take a minute.
Unplug the ESP32 module and put it back in the globe. Check the orientation!

Follow the same procedure for the puck, plug it it, connect, load the provided bin file for the puck, 
set the recommend offset and flash the firmware. Reset the puck with the side button. 


