#include <Arduino.h>
#include <NimBLEDevice.h>
#include <mbedtls/aes.h>
#include <vector>
#include <stdexcept>

#include "batmon.h"

// AES key (16 bytes for AES-128)
const uint8_t key[16] = {108, 101, 97, 103, 101, 110, 100, 255, 254, 48, 49, 48, 48, 48, 48, 57}; 

// Precomputed encrypted command
uint8_t encryptedCommandBytes[16];

BLEClient* client = nullptr;
BM6Data bm6_data = {0, 0};
unsigned long lastDataTimestamp = 0; // Timestamp of last received data
bool isConnectedToBM6 = false; // Track connection status
const unsigned long DATA_FRESHNESS_THRESHOLD = 5000; // 5 seconds in milliseconds


struct Config {  
  std::vector<String> devices;
};

Config config = {
    {"C8:17:F5:29:36:77"}  // Use BLE Scanner app to find the mac of the BM6 device
};


//------------Ringbuffer-----------------

// 16k samples ring buffer -> Store 22h of samples (0.2Hz sampling rate)
#define BUFFER_SIZE 1024*16
#define BUFFER_MASK (BUFFER_SIZE - 1)

BM6Data* ringBuffer;
uint16_t writeIndex = 0;
uint32_t writeCount = 0; 
bool bufferFull = false;

void batmonRing_addValue(BM6Data* value)
{
    ringBuffer[writeIndex] = *value;
    writeIndex = (writeIndex + 1) & BUFFER_MASK;
    writeCount++;

    if (writeIndex == 0)
        bufferFull = true;
}

BM6Data* batmonRing_getValue(uint16_t i)
{
    uint16_t index;

    if (bufferFull)
        index = (writeIndex + i) & BUFFER_MASK;
    else
        index = i;

    return &ringBuffer[index];
}

uint32_t batmonRing_getFillLevel()
{
    if (bufferFull)
        return BUFFER_SIZE;
    else
        return writeIndex;
}

uint32_t batmonRing_getWriteCount()
{
    return writeCount;
}


//------------https://github.com/Goodwillson/Batmon----------------


// Function to decrypt data using AES
String decrypt(uint8_t* crypted, size_t length) {
    if (length != 16) {
        Serial.println("Error: Decrypt function received incorrect length data.");
        return "";
    }

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_dec(&aes, key, 128); // Set key for decryption

    uint8_t decrypted[16]; // Buffer for decrypted data
    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, crypted, decrypted);

    mbedtls_aes_free(&aes); // Clean up

    // Convert decrypted data to a hex string
    String decryptedHex = "";
    for (int i = 0; i < 16; i++) {
        if (decrypted[i] < 0x10) {
            decryptedHex += '0';
        }
        decryptedHex += String(decrypted[i], HEX);
    }

    return decryptedHex;
}

void encrypt(uint8_t* plaintext, size_t length, uint8_t* outputBuffer) {
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, key, 128); // Set key for encryption

    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, plaintext, outputBuffer);
    mbedtls_aes_free(&aes); // Clean up
}

void precomputeEncryptedCommand() {
    // The d15507 command tells the BM6 to start sending voltage/temp notifications
    uint8_t command[] = {0xd1, 0x55, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    
    // Encrypt the command once and store it in the global encryptedCommandBytes array
    encrypt(command, sizeof(command), encryptedCommandBytes);
    Serial.println("Encrypted command precomputed.");
}

// Notification handler for receiving BM6 data
void notificationHandler(BLERemoteCharacteristic* characteristic, uint8_t* data, size_t length, bool isNotify) {
  if (data == NULL || length == 0) {
    Serial.println("Received NULL data in notification.");
    return;
  }
  
  String message = decrypt(data, length);
  // Serial.println("Message received: " + message);

  if (message.startsWith("d15507")) {
    bm6_data.voltage = (uint16_t)strtol(message.substring(15, 18).c_str(), NULL, 16);
    bm6_data.temperature = (int16_t)strtol(message.substring(8, 10).c_str(), NULL, 16) -3; // -3 is a dave calibration
    lastDataTimestamp = millis(); // Update timestamp when data is received
  }    
}

/* Connect, get one datapoint and disconnect... */
void getBM6Data(const char* address) {
  
  Serial.println("Starting BM6 data retrieval...");
  bm6_data.voltage = 0;
  bm6_data.temperature = 0;
  lastDataTimestamp = 0;

  client = nullptr;
  try {
    client = BLEDevice::createClient();
    BLEAddress bleAddress(address);

    Serial.print("Connecting to BLE device at address: ");
    Serial.println(address);

    if (!client->connect(bleAddress)) {
      Serial.println("Failed to connect to the BLE device.");      
      return;
    }

    Serial.println("Connected to BLE device.");
    isConnectedToBM6 = true;

    NimBLERemoteService* service = client->getService("FFF0");
    if (service == nullptr) {
      Serial.println("Failed to find the service with UUID FFF0.");
      client->disconnect();
      isConnectedToBM6 = false;
      vTaskDelay(pdMS_TO_TICKS(1000));
      return;
    }

    NimBLERemoteCharacteristic* charFF3 = service->getCharacteristic("FFF3");
    NimBLERemoteCharacteristic* charFF4 = service->getCharacteristic("FFF4");
    if (charFF3 == nullptr || charFF4 == nullptr) {
      Serial.println("Failed to find the characteristics with UUID FFF3 or FFF4.");
      client->disconnect();
      isConnectedToBM6 = false;
      vTaskDelay(pdMS_TO_TICKS(1000));
      return;
    }

    // Write the precomputed encrypted command to the characteristic
    charFF3->writeValue(encryptedCommandBytes, sizeof(encryptedCommandBytes), true);
    Serial.println("Sent encrypted command to start sending notifications.");

    // Subscribe to notifications
    if (charFF4->canNotify()) {
        charFF4->subscribe(true, notificationHandler);
        Serial.println("Subscribed to notifications.");
    }

    // Wait for data
    unsigned long startTime = millis();
    while (bm6_data.voltage == 0 && bm6_data.temperature == 0) {
      if (millis() - startTime > 10000) {
        Serial.println("Timeout: No data received.");
        client->disconnect();
        isConnectedToBM6 = false;
        vTaskDelay(pdMS_TO_TICKS(1000));
        return;
      }
      vTaskDelay(pdMS_TO_TICKS(100));
    }
    Serial.println("Data received.");
    Serial.print("Voltage: ");
    Serial.println(bm6_data.voltage);
    Serial.print("Temp: ");
    Serial.println(bm6_data.temperature);

    // Unsubscribe from notifications before disconnecting
    if(charFF4->canNotify()) {
        charFF4->unsubscribe();
        Serial.println("Unsubscribed from notifications.");
        vTaskDelay(pdMS_TO_TICKS(500)); // Wait for the unsubscribe operation to complete
    }

  } catch (const std::exception& e) {
    Serial.print("Exception: ");
    Serial.println(e.what());
    if (client) {
      client->disconnect();
      isConnectedToBM6 = false;
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  } catch (...) {
    Serial.println("An unknown error occurred.");
    if (client) {
      client->disconnect();
      isConnectedToBM6 = false;
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  if (client) {
    if (client->isConnected()) {
      client->disconnect();
    }    
    client = nullptr;
    isConnectedToBM6 = false;
    Serial.println("Disconnected from BLE device.");    
    vTaskDelay(pdMS_TO_TICKS(500)); // Wait for the deinit and disconnect to complete
  } 
}

void initBM6(const char* address) {
  
  Serial.println("Starting BM6 init...");
  bm6_data.voltage = 0;
  bm6_data.temperature = 0;
  lastDataTimestamp = 0;

  client = nullptr;
  try {
    client = BLEDevice::createClient();
    BLEAddress bleAddress(address);

    Serial.print("Connecting to BLE device at address: ");
    Serial.println(address);

    if (!client->connect(bleAddress)) {
      Serial.println("Failed to connect to the BLE device.");      
      return;
    }

    Serial.println("Connected to BLE device.");
    isConnectedToBM6 = true;

    NimBLERemoteService* service = client->getService("FFF0");
    if (service == nullptr) {
      Serial.println("Failed to find the service with UUID FFF0.");
      client->disconnect();
      isConnectedToBM6 = false;
      vTaskDelay(pdMS_TO_TICKS(1000));
      return;
    }

    NimBLERemoteCharacteristic* charFF3 = service->getCharacteristic("FFF3");
    NimBLERemoteCharacteristic* charFF4 = service->getCharacteristic("FFF4");
    if (charFF3 == nullptr || charFF4 == nullptr) {
      Serial.println("Failed to find the characteristics with UUID FFF3 or FFF4.");
      client->disconnect();
      isConnectedToBM6 = false;
      vTaskDelay(pdMS_TO_TICKS(1000));
      return;
    }

    // Write the precomputed encrypted command to the characteristic
    charFF3->writeValue(encryptedCommandBytes, sizeof(encryptedCommandBytes), true);
    Serial.println("Sent encrypted command to start sending notifications.");

    // Subscribe to notifications
    if (charFF4->canNotify()) {
        charFF4->subscribe(true, notificationHandler);
        Serial.println("Subscribed to notifications.");
    }

    // Wait for data
    unsigned long startTime = millis();
    while (bm6_data.voltage == 0 && bm6_data.temperature == 0) {
      if (millis() - startTime > 10000) {
        Serial.println("Timeout: No data received.");
        // client->disconnect();
        vTaskDelay(pdMS_TO_TICKS(1000));
        return;
      }
      vTaskDelay(pdMS_TO_TICKS(100));
    }
    Serial.println("Data received.");
    Serial.print("Voltage: ");
    Serial.println(bm6_data.voltage);
    Serial.print("Temp: ");
    Serial.println(bm6_data.temperature);

    // // Unsubscribe from notifications before disconnecting
    // if(charFF4->canNotify()) {
    //     charFF4->unsubscribe();
    //     Serial.println("Unsubscribed from notifications.");
    //     vTaskDelay(pdMS_TO_TICKS(500)); // Wait for the unsubscribe operation to complete
    // }

  } catch (const std::exception& e) {
    Serial.print("Exception: ");
    Serial.println(e.what());
    if (client) {
      client->disconnect();
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  } catch (...) {
    Serial.println("An unknown error occurred.");
    if (client) {
      client->disconnect();
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  // if (client) {
  //   if (client->isConnected()) {
  //     client->disconnect();
  //   }    
  //   client = nullptr;
  //   Serial.println("Disconnected from BLE device.");    
  //   vTaskDelay(pdMS_TO_TICKS(500)); // Wait for the deinit and disconnect to complete
  // } 
}

void reconnectBM6IfNeeded(const char* address) {
  unsigned long currentTime = millis();
  unsigned long timeSinceLastData = currentTime - lastDataTimestamp;
  
  // Check if data is stale (older than 5 seconds)
  if (lastDataTimestamp > 0 && timeSinceLastData > DATA_FRESHNESS_THRESHOLD) {
    Serial.print("Data is stale (");
    Serial.print(timeSinceLastData);
    Serial.println("ms old). Reconnecting to BM6...");
    
    // Disconnect if connected
    if (client) {
      client->disconnect();
      isConnectedToBM6 = false;
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    // Reconnect
    initBM6(address);
  }
}

void get_batmon_data(struct BM6Data* data)
{
  data->voltage = bm6_data.voltage;          /* get the newest datapoint (sampling timepoint unknown)*/
  data->temperature = bm6_data.temperature;  
}

void get_batmon_data_and_store(struct BM6Data* data)
{
  reconnectBM6IfNeeded(config.devices[0].c_str());
  data->voltage = bm6_data.voltage;          /* get the newest datapoint (sampling timepoint unknown)*/
  data->temperature = bm6_data.temperature;  

  batmonRing_addValue(data); // Add the data to the ring buffer
}

void batmon_init() {
    NimBLEDevice::init("");  
    precomputeEncryptedCommand(); // Precompute the encrypted command 

    Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
    Serial.printf("Min free heap: %u bytes\n", ESP.getMinFreeHeap());
    Serial.printf("Largest free block: %u bytes\n", ESP.getMaxAllocHeap());

    ringBuffer = (struct BM6Data*)calloc(BUFFER_SIZE, sizeof(struct BM6Data));
    if(ringBuffer == nullptr)
    {
      Serial.print("ERROR! Ringbuffer calloc failed!");
    }

/*
    Serial.print("DEBUG! Add 400 elements to ringbuffer");
    for(int i=0; i<400; i++)
    {
      struct BM6Data data;
      data.voltage = i*10+1000;
      data.temperature = i%40;
      batmonRing_addValue(&data);
    }
*/

  initBM6(config.devices[0] .c_str());
}
