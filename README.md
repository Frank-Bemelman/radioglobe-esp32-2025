# radioglobe-esp32-2025
Radio Globe using ESP32 and Waveshare round touch LCD 2.1 inch

Not a small project, will take a lot of your time and possibly quite some frustration along the way. Just a gentle warning.

The idea of a 'Radio Globe' is not mine. I was inspired by the works of Jude Pullen. His brilliant design can be found here:
https://github.com/DesignSparkRS/RadioGlobe

Since I am more of an ESP32 guy, I decided to do a complete redesign. New design of 3d printable parts, which you find in de fusion-ed folder here.
You can use Fusion to select the body you want to print on your 3d printer. Or edit them if you wish. I am not very matured in Fusion, so the Fusion timeline is a nice mess.

The globe uses two ESP32 constantly talking to each other using ESPNOW. 

The first ESP32 is in the base of the globe, and deals with actual streaming of a internet radio station, encoders for the coordinates.
It also retrieving the timezone when new coordinates/position is chosen. The timezone is then sent back to the display puck.

The other ESP32 is part of the display puck. I used a Waveshare touch display 2.1 inch. The display puck deals
with volume control, display of what is going on, like coordinates, local time, station name, what song is playing etc. 
It communicates with the Globe's ESP32, using ESPNOW. It receives the exact coordinates NS and EW, remaps them 
as the globes you can buy aren't very accurate. It sends the remapped coordinates back to the globe. Together with the
remapped coordinates, it sends a radiostation url. The urls are stored on a micro SD card. 

There are two directories with source files. One for the globe and one for the display. It is all based on Arduino IDE 2.3.6 

Essential hardware for the globe:<br/>
ESP32-WROOM-32 -> https://www.aliexpress.com/item/1005006456519790.html<br/>
MP3 decoder -> https://www.aliexpress.com/item/32893187079.html<br/>
2x absolute encoder -> https://www.aliexpress.com/item/1005007718766639.html<br/>
<br/>
Essential hardware for the display:<br/>
ESP32S3 Waveshare 2.1 inch LCD -> https://www.aliexpress.com/item/1005008679530480.html<br/>
Lithium battery -> https://www.aliexpress.com/item/1005008575678945.html<br/>
2x Magnets 10mmx2mm -> https://www.aliexpress.com/item/1005008882136145.html<br/>
Pogo 3 pin connector -> https://www.aliexpress.com/item/1005007457425590.html<br/> 
MicroSD card -> I used a 32GB one<br/>









[work in progress]



