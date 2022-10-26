#define USE_DEBUG

#include <ReelTwo.h>
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include "wifi/WifiAccess.h"
#include "wifi/WifiWebServer.h"

#include "DFRobotDFPlayerMini.h"

#define SERIAL1_RX_PIN  16
#define SERIAL1_TX_PIN  17

#define DFPLAYER_TIMEOUT 500

#define WIFI_AP_NAME                    "ESP_DFPlayer"
#define WIFI_AP_PASSPHRASE              "123456789"

WifiAccess wifiAccess;
TaskHandle_t wifiTask;
DFRobotDFPlayerMini myDFPlayer;

WElement mainContents[] = {
    W1("ESP32 WiFi DFPlayer"),
    WHorizontalAlign(),
    WButton("Prev", "prev", []() {
        DEBUG_PRINTLN("Prev Pressed");
        myDFPlayer.previous();
    }),
    WButton("Next", "next", []() {
        DEBUG_PRINTLN("Next Pressed");
        myDFPlayer.next();
    }),
    WButton("Pause", "pause", []() {
        DEBUG_PRINTLN("Volume Down Pressed");
        myDFPlayer.pause();
    }),
    WHR(),
    WVerticalAlign(),
    WButton("+", "volup", []() {
        DEBUG_PRINTLN("Volume Up Pressed");
        myDFPlayer.volumeUp();
    }),
    WHorizontalAlign(),
    WButton("-", "voldown", []() {
        DEBUG_PRINTLN("Volume Down Pressed");
        myDFPlayer.volumeDown();
    }),
    WVerticalAlign(),
    WButton("Start", "start", []() {
        DEBUG_PRINTLN("Start Pressed");
        myDFPlayer.start();
    }),
    WButton("Reset", "reset", []() {
        DEBUG_PRINTLN("Reset Pressed");
        myDFPlayer.reset();
    }),
    WButton("Track 001", "track001", []() {
        DEBUG_PRINTLN("Track 001 Activated");
        myDFPlayer.play(1);
    }),
};

WPage pages[] = {
    WPage("/", mainContents, SizeOfArray(mainContents)),
};

WifiWebServer<10,SizeOfArray(pages)> webServer(pages, wifiAccess);

// WiFi Task runs on other core
void wifiLoopTask(void* /*ignore*/)
{
    for (;;)
    {
        webServer.handle();
        vTaskDelay(1);
    }
}

void setup()
{
    REELTWO_READY();

    SetupEvent::ready();

    Serial1.begin(9600, SERIAL_8N1, SERIAL1_RX_PIN, SERIAL1_TX_PIN);

	if (!myDFPlayer.begin(Serial1))
	{
		Serial.println(myDFPlayer.readType(),HEX);
		Serial.println(F("Unable to begin:"));
		Serial.println(F("1.Please recheck the connection!"));
		Serial.println(F("2.Please insert the SD card!"));
		// while(true);
	}
	//Set serial communictaion time out 500ms
	myDFPlayer.setTimeOut(DFPLAYER_TIMEOUT);

	//----Set volume----
	myDFPlayer.volume(10);  //Set volume value (0~30).
	myDFPlayer.volumeUp(); //Volume Up
	myDFPlayer.volumeDown(); //Volume Down

	//----Set different EQ----
	myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
	// myDFPlayer.EQ(DFPLAYER_EQ_POP);
	// myDFPlayer.EQ(DFPLAYER_EQ_ROCK);
	// myDFPlayer.EQ(DFPLAYER_EQ_JAZZ);
	// myDFPlayer.EQ(DFPLAYER_EQ_CLASSIC);
	// myDFPlayer.EQ(DFPLAYER_EQ_BASS);

	//----Set device we use SD as default----
	//  myDFPlayer.outputDevice(DFPLAYER_DEVICE_U_DISK);
	myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
	//  myDFPlayer.outputDevice(DFPLAYER_DEVICE_AUX);
	//  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SLEEP);
	//  myDFPlayer.outputDevice(DFPLAYER_DEVICE_FLASH);

	//----Mp3 control----
	//  myDFPlayer.sleep();     //sleep
	  myDFPlayer.reset();     //Reset the module
	//  myDFPlayer.enableDAC();  //Enable On-chip DAC
	//  myDFPlayer.disableDAC();  //Disable On-chip DAC
	//  myDFPlayer.outputSetting(true, 15); //output setting, enable the output and set the gain to 15

    wifiAccess.setNetworkCredentials(
        WIFI_AP_NAME,
        WIFI_AP_PASSPHRASE,
        true, /* WiFi Access Point */
        true  /* WiFi Enabled */);
        

    wifiAccess.notifyWifiConnected([](WifiAccess &wifi) {
    	// WiFi is active
        Serial.print("Connect to http://"); Serial.println(wifi.getIPAddress());
    });

    xTaskCreatePinnedToCore(
          wifiLoopTask,
          "WiFi",
          5000,    // 5K stack size
          nullptr,
          1,
          &wifiTask,
          0);
//myDFPlayer.play(p1);  //Play the first track as a bootup sound      
}

void loop()
{
    AnimatedEvent::process();

	if (myDFPlayer.available())
	{
		if (myDFPlayer.readType() == DFPlayerPlayFinished)
		{
			Serial.println(myDFPlayer.read());
			Serial.println(F("next--------------------"));
			myDFPlayer.next();  //Play next mp3 every 3 second.
			Serial.println(F("readCurrentFileNumber--------------------"));
			Serial.println(myDFPlayer.readCurrentFileNumber()); //read current play file number
			delay(500);
		}
	}  
}

