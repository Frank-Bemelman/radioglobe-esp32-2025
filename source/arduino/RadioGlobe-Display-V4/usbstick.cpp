#include "USB.h"
#include "USBMSC.h"
#include "SD_MMC.h"

USBMSC MSC;
bool computerHasAccess = true;

// Callback when the PC reads blocks of data from the SD card
static int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
  if (!computerHasAccess) return -1; // Block PC if ESP32 is using the card
  if (SD_MMC.readRAW((uint8_t*)buffer, lba)) {
    return bufsize;
  }
  return -1;
}

// Callback when the PC writes blocks of data to the SD card
static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
  if (!computerHasAccess) return -1; // Block PC if ESP32 is using the card
  if (SD_MMC.writeRAW(buffer, lba)) {
    return bufsize;
  }
  return -1;
}

// Custom function to safely hand control to the ESP32
void beginEsp32FileAccess() {
  Serial.println(">>> Disconnecting USB Drive from PC...");
  
  // 1. Tell the PC the media was removed
  MSC.mediaPresent(false); 
  computerHasAccess = false;
  delay(1000); // Give the PC OS a moment to unmount the drive safely
  
  // 2. Remount the standard FAT File System locally on the ESP32
  // We use standard SD_MMC.begin() parameters to re-index the file layout
  SD_MMC.begin("/sdcard", true, false, 20000); 
}

// Custom function to return control back to the Computer
void endEsp32FileAccess() {
  Serial.println(">>> Re-linking USB Drive to PC...");
  
  // 1. Close local file system links completely
  SD_MMC.end(); 
  
  // 2. Alert the PC that the storage device is plugged back in
  computerHasAccess = true;
  MSC.mediaPresent(true); 
}

void setupusb() {
  //Serial.begin(115200);
  //delay(2000);

  // Setup SD_MMC Pin assignments (1-bit MMC mode)
  //pinMode(12, INPUT_PULLUP);
  //pinMode(11, INPUT_PULLUP);
  //pinMode(13, INPUT_PULLUP);
  //SD_MMC.setPins(12, 11, 13); // CLK, CMD, D0

  // Standard Initial Connection (Give to PC first)
  //if (!SD_MMC.begin("/sdcard", true, false, 20000)) {
  //  Serial.println("SD Card initialization failed!");
  //  return;
  //}

 // uint32_t block_count = SD_MMC.totalBytes() / 16384;
  SD_MMC.end(); // Close down local file layer so MSC can safely take over

  // Configure the USB Mass Storage device
  MSC.vendorID("ESP32-S3");
  MSC.productID("Shared_SD");
  MSC.productRevision("1.0");
  MSC.onRead(onRead);
  MSC.onWrite(onWrite);
  MSC.mediaPresent(true);
  MSC.begin(SD_MMC.numSectors(), SD_MMC.sectorSize()); 

  USB.begin();
  Serial.println("System Ready. Connected to PC as USB Storage Drive.");
}

void loopnever() {
  // Simulating an interval where the ESP32 needs to log data locally every 30 seconds
  delay(30000);

  // --- STEP 1: Disconnect PC and claim SD card ---
  beginEsp32FileAccess();

  // --- STEP 2: Execute ESP32 local read/write actions ---
  Serial.println("ESP32: Opening log file...");
  File file = SD_MMC.open("/log.txt", FILE_APPEND);
  if (file) {
    file.println("Sensor Log Entry: Data successfully saved by ESP32!");
    file.close();
    Serial.println("ESP32: Successfully appended text to /log.txt!");
  } else {
    Serial.println("ESP32: Failed to open file for writing.");
  }

  // Reading the file back to show it worked via the Serial Monitor
  file = SD_MMC.open("/log.txt", FILE_READ);
  if (file) {
    Serial.println("--- Reading file content from ESP32 ---");
    while (file.available()) {
      Serial.write(file.read());
    }
    file.close();
    Serial.println("----------------------------------------");
  }

  // --- STEP 3: Relinquish control back to PC ---
  endEsp32FileAccess();
}