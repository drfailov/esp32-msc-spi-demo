//Compile for LOLIN S2 Mini

#include "USB.h"
#include "USBMSC.h"


/*
Here's the point:
MSC Is access memory by LBA num and Offset. 
LBA=512bytes.
but
Flash chip is accessed by pages. And before every WRITE, there's necessary to CLEAR all corresponding page.
Page = 4096 bytes.
*/
USBMSC MSC;

// https://github.com/espressif/esp-idf/blob/master/examples/storage/partition_api/partition_ops/main/main.c
//  Find the partition map in the partition table
const esp_partition_t *spifsPartition;
uint8_t pageBuffer[SPI_FLASH_SEC_SIZE]; // flsah can be written only by this blocks
esp_err_t res;                          // used many times where patition operations occur

static const uint32_t DISK_SECTOR_COUNT = 2 * 1000; // 2*8 8KB is the smallest size that windows allow to mount 
static const uint16_t DISK_SECTOR_SIZE = 512;      // Should be 512   (same as LBA)


void setup() {
  //Serial.begin(115200);
  pinMode(15, OUTPUT);
  digitalWrite(15, HIGH);
  MSC.vendorID("DrFailov");     // max 8 chars
  MSC.productID("MSC Demo"); // max 16 chars
  MSC.productRevision("1.0");   // max 4 chars
  MSC.onStartStop(onStartStop);
  MSC.onRead(onRead);
  MSC.onWrite(onWrite);
  MSC.mediaPresent(true);
  MSC.begin(DISK_SECTOR_COUNT, DISK_SECTOR_SIZE);

  spifsPartition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "spiffs");  //for spiffs partition scheme
  //spifsPartition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "fatffs");  //for spiffs partition scheme
  //spifsPartition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, "app1");  //for second app space
  clearPartition();
}

void loop() {
  digitalWrite(15, LOW);
}




void writeFlash(uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
  uint32_t pageOffset = offset % SPI_FLASH_SEC_SIZE;
  Serial.printf("  pageOffset: %d\n", pageOffset);
  uint32_t pageStart = offset - pageOffset;
  Serial.printf("  pageStart: %d\n", pageStart);
  uint32_t endOfBuf = offset + bufsize;
  Serial.printf("  endOfBuf: %d\n", endOfBuf);
  uint32_t endOfPage = pageStart + SPI_FLASH_SEC_SIZE;
  Serial.printf("  endOfPage: %d\n", endOfPage);
  uint32_t bufSz = min(endOfBuf, endOfPage) - offset;
  Serial.printf("  bufSz: %d\n", bufSz);

  Serial.printf("  Read: %d-%d...", pageStart, endOfPage);
  res = esp_partition_read(spifsPartition, pageStart, pageBuffer, SPI_FLASH_SEC_SIZE);
  if (res == ESP_OK)
    Serial.printf("OK\n");
  else if (res == ESP_ERR_INVALID_ARG)
    Serial.printf("ESP_ERR_INVALID_ARG\n");
  else if (res == ESP_ERR_INVALID_SIZE)
    Serial.printf("ESP_ERR_INVALID_SIZE\n");
  else if (res == ESP_OK)
    Serial.printf("ERR %d\n", res);

  Serial.printf("  Modify : %d-%d...", offset, offset + bufSz);
  memcpy(/*dst*/ pageBuffer+pageOffset, /*src*/ buffer, /*len*/ bufSz);
  Serial.printf("OK\n");

  Serial.printf("  Erase: %d-%d...", pageStart, endOfPage);
  res = esp_partition_erase_range(spifsPartition, pageStart, SPI_FLASH_SEC_SIZE);
  if (res == ESP_OK)
    Serial.printf("OK\n");
  else if (res == ESP_ERR_INVALID_ARG)
    Serial.printf("ESP_ERR_INVALID_ARG\n");
  else if (res == ESP_ERR_INVALID_SIZE)
    Serial.printf("ESP_ERR_INVALID_SIZE\n");
  else if (res == ESP_OK)
    Serial.printf("ERR %d\n", res);

  Serial.printf("  Write: %d-%d...", pageStart, endOfPage);
  res = esp_partition_write(spifsPartition, pageStart, pageBuffer, SPI_FLASH_SEC_SIZE); // sizeof(store_data)
  if (res == ESP_OK)
    Serial.printf("OK\n");
  else if (res == ESP_ERR_INVALID_ARG)
    Serial.printf("ESP_ERR_INVALID_ARG\n");
  else if (res == ESP_ERR_INVALID_SIZE)
    Serial.printf("ESP_ERR_INVALID_SIZE\n");
  else if (res == ESP_OK)
    Serial.printf("ERR %d\n", res);

  if (endOfPage < endOfBuf) // write next page
    writeFlash(/*offset*/ offset + bufSz, /*buffer*/ buffer + bufSz, /*bufsize*/ bufsize - bufSz);
}


static int32_t onWrite(uint32_t lba, uint32_t lba_offset, uint8_t *buffer, uint32_t bufsize)
{
  digitalWrite(15, HIGH);
  Serial.printf("MSC WRT lba: %u, offs: %u, bufsz: %u:\n", lba, lba_offset, bufsize);
  if (spifsPartition == NULL)
  {
    Serial.printf("\nERR! NO PARTITION!");
    return 0;
  }
  uint32_t offset = lba * DISK_SECTOR_SIZE + lba_offset;
  Serial.printf("  offset: %d\n", offset);
  writeFlash(offset, buffer, bufsize);
  return bufsize;
}

static int32_t onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{
  digitalWrite(15, HIGH);
  Serial.printf("MSC RD: lba: %u, offs: %u, bufsz: %u...", lba, offset, bufsize);
  if (spifsPartition == NULL)
  {
    Serial.printf("ERR! NO PARTITION!");
    return 0;
  }
  res = esp_partition_read(spifsPartition, lba * DISK_SECTOR_SIZE + offset, buffer, bufsize);
  if (res == ESP_OK)
    Serial.printf("OK\n");
  else if (res == ESP_ERR_INVALID_ARG)
    Serial.printf("ESP_ERR_INVALID_ARG\n");
  else if (res == ESP_ERR_INVALID_SIZE)
    Serial.printf("ESP_ERR_INVALID_SIZE\n");
  else if (res == ESP_OK)
    Serial.printf("ERR %d\n", res);
  return bufsize;
}

static bool onStartStop(uint8_t power_condition, bool start, bool load_eject)
{
  Serial.printf("\nMSC START/STOP: power: %u, start: %u, eject: %u\n", power_condition, start, load_eject);
  return true;
}


void clearPartition()
{
  if (spifsPartition == NULL)
  {
    Serial.printf("ERR! NO PARTITION!");
    return;
  }
  Serial.printf("Start\n");
  uint8_t lbaBuffer[DISK_SECTOR_SIZE];
  for (uint32_t lba = 0; lba  < DISK_SECTOR_COUNT; lba++)
  {
    uint32_t lbaStart = lba * DISK_SECTOR_SIZE;
    uint32_t lbaEnd = lbaStart + DISK_SECTOR_SIZE;
    Serial.printf(">Write: %d-%d...\n", lbaStart, lbaEnd);
    onWrite(lba, 0, lbaBuffer, DISK_SECTOR_SIZE);
  }
  Serial.printf("== FINISHED. ==\n\n");
}