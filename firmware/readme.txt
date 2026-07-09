Latest version has an easier auto update function. It will signal a new update is available, if the cogwheel setup button is red. 
Go into settings on the puck, long press the serial number and it will upgrade both globe and puck, using the bin files from the release. 
First the globe upgrades automatically, then the puck will upgrade also.
The puck display goes black during the upgrade, just hang in there for a few minutes - it will reboot after a few minutes.

HOWEVER!!! If your version does NOT have the auto-update feature yet, it takes a bit more work.
Good news is, you only have to do this once.

1) download the three bin files for globe (firmware) and puck (firmware and new partition table) as found in the release.
   These can all be uploaded using a chrome browser on PC and a webtool
2) Browse to: https://thelastoutpostworkshop.github.io/ESPConnect/
3) Connect Puck, use the blue connect button, select the USB port, and connect.
   Select Flash Tools on the left.
4) First flash the new partition file for the puck. Load it in the flashtool, select Recommended Offsets -> Partition Table • 0x8000 • 4 KB
   Hit the large FLASH FIRMWARE bar/button.
5) Load the RadioGlobe-Display.bin, select Recommended Offsets -> App0 0x10000 4MB
   Hit the large FLASH FIRMWARE bar/button.
6) Disconnect Puck, reset it with side button.

7) Connect the Globe ESP32S3 to your PC. Use the blue connect button, select the USB port, and connect.
8) Load the RadioGlobe-Globe.bin, select Recommended Offsets -> App0 0x10000 3MB
   Hit the large FLASH FIRMWARE bar/button.

From now on, you can update automatically, no more fiddling with cables.
Everytime you power down/up the globe using the puck, it will check for an update and the cogwheel will turn red if there is one available.
You can also force a check for new updates, by long pressing the serial number in the setup menu.
During an automatic update the puck goes black, don't worry, it will come back after 2-3 minutes.

Enjoy & Good luck!
