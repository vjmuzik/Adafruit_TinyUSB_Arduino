/*********************************************************************
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 Copyright (c) 2019 Ha Thach for Adafruit Industries
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

/* This example expose SD card as mass storage using
 * default SD Library
 */

#include "SD.h"
#include "Adafruit_TinyUSB.h"

const uint8_t chipSelect = BUILTIN_SDCARD;

Adafruit_USBD_MSC usb_msc;

Sd2Card card;
SdVolume volume;

// the setup function runs once when you press reset or power the board
void setup()
{
  USBDevice.setVersion(0x0200);
  USBDevice.setDeviceVersion(0x0280);
  USBDevice.setLanguageDescriptor(0x0409);
  USBDevice.setManufacturerDescriptor("Teensyduino");
  USBDevice.setProductDescriptor("USB Serial + MSC");

  Adafruit_TinyUSB_Core_init();
  // Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
  usb_msc.setID("Adafruit", "SD Card", "1.0");

  // Set read write callback
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);

  // Still initialize MSC but tell usb stack that MSC is not ready to read/write
  // If we don't initialize, board will be enumerated as CDC only
  usb_msc.setUnitReady(false);
  usb_msc.begin();

  USBSerial.begin(115200);
  while ( !USBSerial ) delay(10);   // wait for native usb

  USBSerial.println("Adafruit TinyUSB Mass Storage SD Card example");

  USBSerial.println("\nInitializing SD card...");
  USBSerial.flush();

  if ( !card.init(SPI_FULL_SPEED, chipSelect) )
  {
    USBSerial.println("initialization failed. Things to check:");
    USBSerial.println("* is a card inserted?");
    USBSerial.println("* is your wiring correct?");
    USBSerial.println("* did you change the chipSelect pin to match your shield or module?");
    USBSerial.flush();
    while (1) delay(1);
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    USBSerial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    USBSerial.flush();
    while (1) delay(1);
  }
  
  uint32_t block_count = volume.blocksPerCluster()*volume.clusterCount();

  USBSerial.print("Volume size (MB):  ");
  USBSerial.println((block_count/2) / 1024);
  USBSerial.flush();

  // Set disk size, SD block size is always 512
  usb_msc.setCapacity(block_count, 512);

  // MSC is ready for read/write
  usb_msc.setUnitReady(true);
}

void loop()
{
  // nothing to do
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and
// return number of copied bytes (must be multiple of block size)
int32_t msc_read_cb (uint32_t lba, void* buffer, uint32_t bufsize)
{
  (void) bufsize;
  #if !defined(__has_include) || !__has_include(<SdFat.h>)
  return card.readBlock(lba, (uint8_t*) buffer) ? 512 : -1;
  #else
  return SD.sdfs.card()->readSector(lba, (uint8_t*) buffer) ? 512 : -1;
  #endif
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and 
// return number of written bytes (must be multiple of block size)
int32_t msc_write_cb (uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
  #if !defined(__has_include) || !__has_include(<SdFat.h>)
  return card.writeBlock(lba, buffer) ? 512 : -1;
  #else
  return SD.sdfs.card()->writeSector(lba, buffer) ? 512 : -1;
  #endif
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void msc_flush_cb (void)
{
  // nothing to do
}

void yield(){
  tud_task();
}
