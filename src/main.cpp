#include <M5StickC.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char *ssid = "YOSHIDALAB";
const char *password = "44754921";

const int capacity = JSON_OBJECT_SIZE(2);
StaticJsonDocument<capacity> json_request;
char buffer[255];

const char *host = "http://172.31.50.20:3000/sensors/";
const char *mac_address;

const char ble_tag_mac_address[] = {};

void sendServer()
{
  json_request["room_id"] = 5;
  json_request["mac_address"] = mac_address;

  serializeJson(json_request, Serial);
  Serial.println("");

  serializeJson(json_request, buffer, sizeof(buffer));

  HTTPClient http;
  http.begin(host);
  http.addHeader("Content-Type", "application/json");
  int status_code = http.POST((uint8_t *)buffer, strlen(buffer));
  Serial.printf("status_code=%d\r\n", status_code);
  if (status_code == 200)
  {
    Stream *resp = http.getStreamPtr();

    DynamicJsonDocument json_response(255);
    deserializeJson(json_response, *resp);

    serializeJson(json_response, Serial);
    Serial.println("");
  }
  http.end();
}

class IBeaconAdvertised : public BLEAdvertisedDeviceCallbacks
{
public:
  // BLE検出時のコールバック
  void onResult(BLEAdvertisedDevice device)
  {
    if (!isIBeacon(device))
    {
      return;
    }
    printIBeacon(device);
  }

private:
  // iBeaconパケット判定
  bool isIBeacon(BLEAdvertisedDevice device)
  {
    if (device.getManufacturerData().length() < 25)
    {
      return false;
    }
    if (getCompanyId(device) != 0x004C)
    {
      return false;
    }
    if (getIBeaconHeader(device) != 0x1502)
    {
      return false;
    }
    return true;
  }

  // CompanyId取得
  unsigned short getCompanyId(BLEAdvertisedDevice device)
  {
    const unsigned short *pCompanyId = (const unsigned short *)&device
                                           .getManufacturerData()
                                           .c_str()[0];
    return *pCompanyId;
  }

  // iBeacon Header取得
  unsigned short getIBeaconHeader(BLEAdvertisedDevice device)
  {
    const unsigned short *pHeader = (const unsigned short *)&device
                                        .getManufacturerData()
                                        .c_str()[2];
    return *pHeader;
  }

  // UUID取得
  String getUuid(BLEAdvertisedDevice device)
  {
    const char *pUuid = &device.getManufacturerData().c_str()[4];
    char uuid[64] = {0};
    sprintf(
        uuid,
        "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        pUuid[0], pUuid[1], pUuid[2], pUuid[3], pUuid[4], pUuid[5], pUuid[6], pUuid[7],
        pUuid[8], pUuid[9], pUuid[10], pUuid[11], pUuid[12], pUuid[13], pUuid[14], pUuid[15]);
    return String(uuid);
  }

  // TxPower取得
  signed char getTxPower(BLEAdvertisedDevice device)
  {
    const signed char *pTxPower = (const signed char *)&device
                                      .getManufacturerData()
                                      .c_str()[24];
    return *pTxPower;
  }

  // iBeaconの情報をシリアル出力
  void printIBeacon(BLEAdvertisedDevice device)
  {
    mac_address = device.getAddress().toString().c_str();
    if (mac_address == "f5:3a:a2:cb:1d:1f")
    {
      Serial.println("BLEタグ 1");
    }
    else if (mac_address == "d4:43:39:17:43:12")
    {
      Serial.println("BLEタグ 3");
    }
    else if (mac_address == "d5:be:bd:65:92:34")
    {
      Serial.println("BLEタグ 4");
    }
    else if (mac_address == "f4:cd:38:6e:d3:f8")
    {
      Serial.println("BLEタグ 5");
    }
    else if (mac_address == "db:17:6a:06:15:99")
    {
      Serial.println("BLEタグ 8");
    }
    else
    {
      Serial.printf("MAC Address: %s\n  RSSI: %d\n",
                    device.getAddress().toString().c_str(),
                    device.getRSSI());
    }
  }
};

void setup()
{
  M5.begin();
  Serial.begin(115200);
  BLEDevice::init("");

  Serial.println("Start Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }
  Serial.println("Connected");
}

void loop()
{
  Serial.println("Start.");
  BLEScan *scan = BLEDevice::getScan();
  scan->setAdvertisedDeviceCallbacks(new IBeaconAdvertised(), true);
  scan->setActiveScan(true);
  scan->start(5);
  Serial.println("Complete.");
}
