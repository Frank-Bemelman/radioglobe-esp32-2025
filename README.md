<img src="/images/IMG_3491.JPG" width="500">

# radioglobe-esp32-2025
Radio Globe using ESP32 and Waveshare round touch LCD 2.1 inch<br><br>

UPDATE 15-JUL-2026<br>Improved handling of retrieving timezone and geolocation. Race conditions could occur, now hopefully solved. Battery percentage indicator, better than that silly voltage level. <br><br>
UPDATE 12-JUL-2026<br>Automatic Auto Power Off. If you only set the volume to 0, the globe keeps streaming in the background. Which is a waste of bandwidth, for you and for the radio station. So, after 5 minutes, the globe switches off and the clock is shown. Also, after 8 hours normal playing but without any user interaction, the globe will turn the volume down and switches off silently.<br><br> 
UPDATE 7-JUL-2026<br>Switching between Radio and SD music files, can now be done by long pressing the RADIO GLOBE / MUSIC GLOBE text on the main screen. When playing SD card files, the actual path GLOBEMUSIC/xxxx is shown.<br><br>
UPDATE 5-JUL-2026<br>Added a Globe webserver to make it easier to add/delete music MP3 files. Navigate to radioglobe-XX.local where XX is the serialnumber of the globe.<br><br>
UPDATE 3-JUL-2026<br>Fixed a FTP bug as it refused to work with filenames containing single quotes<br><br>
UPDATE 2-JUL-2026<br>
Added FTP servers for globe and puck. For the puck, activate it with secret code BOBOB in setup menu. IP address will be shown once FTP server is launched. Use Filezilla, plain FTP, user globe password globe, transfer settings default, limit max connections 1.<br><br>
UPDATE 1-JUL-2026<br>
New stations150K.json file for the puck SD card. After you replace it, rebuild the database, in puck menu.<br><br>
UPDATE 1-JUL-2026<br>Added the ESP32_VS1053_Stream library files in the Arduino RADIOGLOBE-GLOBE project folder - these are slightly modified<br><br>
UPDATE 30-JUN-2026<br>Added auto update feature -> long press serial number in setup menu for auto update globe and puck<br><br>
UPDATE 24-MRT-2026<br>Added lots of features, worldmap, clock, weather status, currency exchange<br><br>
UPDATE 15-JULY-2025<br>Lots of improvements made. Most stations now play without issues. Happy with the result.<br><br>
UPDATE 6-JULY-2025<br>It actually works! Performance is improved, few bugs, some extras. <br><br>
UPDATE 1-JULY-2025<br>It actually works! Performance is perhaps not perfect but certainly acceptable.<br><br>

Not a small project, will take a lot of your time and possibly quite some frustration along the way. Just a gentle warning.

The idea of a 'Radio Globe' is not mine. I was inspired by the works of Jude Pullen. His brilliant design can be found here:
https://github.com/DesignSparkRS/RadioGlobe

Since I am more of an ESP32 guy, I decided to do a complete redesign. New design of 3d printable parts, which you find in de fusion-3d folder here.
You can use Fusion to select the body you want to print on your 3d printer. Or edit them if you wish. I am not very matured in Fusion, so the Fusion timeline is a nice mess.

The globe uses two ESP32 constantly talking to each other using ESPNOW. 

The first ESP32 is in the base of the globe, and deals with actual streaming of a internet radio station, encoders for the coordinates.
It also retrieving the timezone when new coordinates/position is chosen. The timezone is then sent back to the display puck.

The other ESP32 is part of the display puck. I used a Waveshare touch display 2.1 inch. The display puck deals
with volume control, display of what is going on, like coordinates, local time, station name, what song is playing etc. 
It communicates with the Globe's ESP32, using ESPNOW. It receives the exact coordinates NS and EW, remaps them 
as the globes you can buy aren't very accurate. It sends the remapped coordinates back to the globe. Together with the
remapped coordinates, it sends a radiostation url. The urls are stored on a micro SD card. 

There are two directories with source files. One for the globe and one for the display. It is all based on Arduino IDE 2.3.10

Essential hardware for the globe:<br/>
ESP32S3-N16R8 
MP3 decoder -> https://www.aliexpress.com/item/32893187079.html<br/>
2x magnetic encoder AS5600
<br/>
Essential hardware for the display:<br/>
ESP32S3 Waveshare 2.1 inch LCD -> https://www.aliexpress.com/item/1005008679530480.html<br/>
Lithium battery -> https://www.aliexpress.com/item/1005008575678945.html<br/>
2x Magnets 10mmx2mm
Pogo 3 pin connector -> https://www.aliexpress.com/item/1005007457425590.html<br/> 
MicroSD card -> I used a 32GB one<br/>

1) Install Fusion 360 and load the designs provided here. Print all the individual bodies and assemble the lot.
2) Prepare the SD card and fill it with the files provided in SDCARD. This is for the database with 150K stations and two folders with images for the flags of all countries in the world. Insert the card in the Waveshare display. Once you have loaded the software into the Waveshare, you can create a presorted database arrangement in a folder structure on the SD card. This takes perhaps 45 minutes, but then you're good to go.
3) Prepare the ESP32 module with the software for the globe itself. Compile in Arduino IDE, don't forget to fill the secrets.h file with your wifi SSID and password.
4) Get an API key at Google Developer platform, for the TimeZone API and Geo Geocoding API, both use the same API key. It is used to find the right timezone for the clock, and also to get the right two-letter country code to be able to display the right country flag. It is a bit of hassle, but very worthwhile. And it is free.

I don't plan to give a step-by-step instruction here. That is too much effort and I don't know if there is a lot of interrest in this project. But if you start with it, just ask. In which case I may elaborate a bit more.


Cheers,
Frank














[work in progress]



