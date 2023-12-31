#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include "cert.h"

const char *ssid = "Al-Imaad";
const char *wifiPassword = "7760@_Ahmad";
int status = WL_IDLE_STATUS;
int incomingByte;

String FirmwareVer = {"1.0"};

// #define URL_fw_Version "https://raw.githubusercontent.com/AhmadMahi/QuibbleUpdate/master/Quibbleupdate/bin-version.txt"
// #define URL_fw_Bin "https://raw.githubusercontent.com/AhmadMahi/QuibbleUpdate/master/Quibbleupdate/fw.bin"
#define URL_fw_Version "https://raw.githubusercontent.com/AhmadMahi/QuibbleUpdate/master/Quibbleupdate/bin-version.txt"
#define URL_fw_Bin "https://raw.githubusercontent.com/mek1925/Esp32_HomeWaterTank_Automation/main/Firmware_update/SourceCode.ino.bin"



void setup()
{

    Serial.print("Active Firmware Version:");
    Serial.println(FirmwareVer);
    Serial.begin(115200);

    WiFi.begin(ssid, wifiPassword);

    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");

        int i = 0;
        if (i == 10)
        {
            ESP.restart();
        }
        i++;
    }
    Serial.println("Connected To Wifi");
}

void loop()
{

    delay(1000);
    Serial.print(" Active Firmware Version:");
    Serial.println(FirmwareVer);

    if (WiFi.status() != WL_CONNECTED)
    {
        reconnect();
    }

    if (Serial.available() > 0)
    {
        incomingByte = Serial.read();
        if (incomingByte == 'U')
        {
            Serial.println("Firmware Update In Progress..");
            if (FirmwareVersionCheck())
            {
                firmwareUpdate();
            }
        }
    }
}

void reconnect()
{
    int i = 0;
    // Loop until we're reconnected
    status = WiFi.status();
    if (status != WL_CONNECTED)
    {
        WiFi.begin(ssid, wifiPassword);
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
            if (i == 10)
            {
                ESP.restart();
            }
            i++;
        }
        Serial.println("Connected to AP");
    }
}

void firmwareUpdate(void)
{
    WiFiClientSecure client;
    client.setCACert(rootCACertificate);
    t_httpUpdate_return ret = httpUpdate.update(client, URL_fw_Bin);

    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        break;

    case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;

    case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        break;
    }
}

int FirmwareVersionCheck(void)
{
    String payload;
    int httpCode;
    String FirmwareURL = "";
    FirmwareURL += URL_fw_Version;
    FirmwareURL += "?";
    FirmwareURL += String(rand());
    Serial.println(FirmwareURL);
    WiFiClientSecure *client = new WiFiClientSecure;

    if (client)
    {
        client->setCACert(rootCACertificate);
        HTTPClient https;

        if (https.begin(*client, FirmwareURL))
        {
            Serial.print("[HTTPS] GET...\n");
            // start connection and send HTTP header
            delay(100);
            httpCode = https.GET();
            delay(100);
            if (httpCode == HTTP_CODE_OK) // if version received
            {
                payload = https.getString(); // save received version
            }
            else
            {
                Serial.print("Error Occured During Version Check: ");
                Serial.println(httpCode);
            }
            https.end();
        }
        delete client;
    }

    if (httpCode == HTTP_CODE_OK) // if version received
    {
        payload.trim();
        if (payload.equals(FirmwareVer))
        {
            Serial.print("\nDevice  IS Already on Latest Firmware Version: \n");
            Serial.println(FirmwareVer);
            return 0;
        }
        else
        {
            Serial.println(payload);
            Serial.println("New Firmware Detected");
            return 1;
        }
    }
    return 0;
}
