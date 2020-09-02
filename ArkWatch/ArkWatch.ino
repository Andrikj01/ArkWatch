/*
Text box is created on line 113
Text box text is set on line 118
Comment out lines 85 to 92 to disable screen 2
*/
#include "config.h"
#include "EspMQTTClient.h"

#define MQTT_SERVER_IP    "Change.Me"
#define MQTT_USERNAME     "Username"
#define MQTT_PASSWORD     "Paswword"
#define MQTT_CLIENT_NAME  "WatchClient"    //this should be a "random" value. This value is ignored if you call WiFiMQTTclient.enableMACaddress_for_ClientName();
#define MQTT_SERVER_PORT  1883

#define WIFI_SSID         "Telus0357"
#define WIFI_PASS         "77kmm7r7hz"

//MQTT topics
const char* MQTT_Base_Topic = "lighting/bed/patterns";


EspMQTTClient WiFiMQTTclient(
  WIFI_SSID,
  WIFI_PASS,
  MQTT_SERVER_IP,   // MQTT Broker server ip
  MQTT_USERNAME,    // Can be omitted if not needed
  MQTT_PASSWORD,    // Can be omitted if not needed
  MQTT_CLIENT_NAME, // Client name that uniquely identify your device
  MQTT_SERVER_PORT  // The MQTT port, default to 1883. this line can be omitted
);

bool initialConnectionEstablished_Flag = false;   //used to detect first run after power up
bool initialized = false;

//Frequency at which the MQTT packets are published
uint32_t UpdateInterval_MQTT_Publish = 10000;           // 10 seconds
uint32_t previousUpdateTime_MQTT_Publish = millis();


//standard LVGL button width
int button_width = 110;

TTGOClass *ttgo;
TFT_eSPI *tft;
AXP20X_Class *power;
lv_obj_t *label;
lv_obj_t * cpicker;


static void MainScreen_Handler (lv_obj_t *obj, lv_event_t event) {
  Serial.println("Switching to Main Screen");
  load_MainScreen();
  //lv_obj_del(Screen2);
}

static void Screen2_Handler (lv_obj_t *obj, lv_event_t event) {
  Serial.println("Switching to screen 2");
  load_Screen2();
  //lv_obj_del(MainScreen);
}

//load screen 2 and create items on screen
void load_Screen2() {
  lv_obj_t * Screen2 = lv_obj_create(NULL, NULL);
  lv_scr_load(Screen2);

  // load button (switch screen)
  lv_obj_t *btn_MainScreen = lv_btn_create(lv_scr_act(), NULL);
  lv_obj_set_width(btn_MainScreen, button_width);
  lv_obj_set_height(btn_MainScreen, 40);
  lv_obj_set_event_cb(btn_MainScreen, MainScreen_Handler);
  lv_obj_align(btn_MainScreen, NULL, LV_ALIGN_CENTER, -57, 95);
  label = lv_label_create(btn_MainScreen, NULL);
  lv_label_set_text(label, "Main Menu");
}



//load the main screen and create items on screen
void load_MainScreen()  {
  lv_obj_t * MainScreen = lv_obj_create(NULL, NULL);
  lv_scr_load(MainScreen);
  
// Comment this out to disable screen 2
//create button (switch screen)
  lv_obj_t *btn_screen2 = lv_btn_create(lv_scr_act(), NULL);
  lv_obj_set_width(btn_screen2, button_width);
  lv_obj_set_event_cb(btn_screen2, Screen2_Handler);
  lv_obj_align(btn_screen2, NULL, LV_ALIGN_CENTER, 57, 90);
  label = lv_label_create(btn_screen2, NULL);
  lv_label_set_text(label, "Screen 2");


//print current IP at top of screen
  tft->setCursor(50, 3);
  tft->print(WiFi.localIP());

  //tft->setCursor(50, 30);
  //tft->print("Text Box");

  /*
  // Text areas are text boxes that can be selected and typed in using an on screen keyboard and can also run a handler when pressed (like a button)
  lv_obj_t * TextArea = lv_textarea_create(lv_scr_act(), NULL);
  lv_obj_align(TextArea, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_textarea_set_cursor_hidden(TextArea, true);
  lv_textarea_set_text_align(TextArea, LV_LABEL_ALIGN_CENTER);
  lv_textarea_set_max_length(TextArea, 35);
  lv_textarea_set_one_line(TextArea, false);
  lv_textarea_set_text(TextArea, "New text Test 1234@#$%^&*()abcdefgh");
  */
  
  // Labels are just simple text boxes
  lv_obj_t * label1 = lv_label_create(lv_scr_act(), NULL); // create label object
  lv_label_set_long_mode(label1, LV_LABEL_LONG_BREAK);     /*Break the long lines*/
  //lv_label_set_recolor(label1, true);                      //Enable re-coloring by commands in the text
  //lv_label_set_text(label1, "#0000ff Re-color# #ff00ff words# #ff0000 of a# label ""and  wrap long text automatically.");
  lv_label_set_align(label1, LV_LABEL_ALIGN_CENTER);       //Center aligned lines
  lv_label_set_text(label1, "Test Text 1 2 3 4 5 6 7 8 9 0 ! @ # $ % ^ & * & ( )"); // set text in box
  lv_obj_set_width(label1, 200);  // set width of box
  //lv_obj_set_height(label1, 150);
  lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, 0); // move the box around, LV_ALIGN_CENTER will set the center of the object to the given coordinates
}



/********************************************************************************
  This function is called once WiFi and MQTT connections are complete
********************************************************************************/
void onConnectionEstablished() {
  if (!initialConnectionEstablished_Flag) {     //execute this the first time we have established a WiFi and MQTT connection after powerup
    initialConnectionEstablished_Flag = true;
    ttgo->motor->onec();

#ifdef _enableNTP
    //--------------------------------------------
    //  sync local time to NTP server
    configTime(TIME_ZONE * 3600, DST, "pool.ntp.org", "time.nist.gov");
#endif

    //--------------------------------------------
    //  display IP address.
    Serial.println("\nIP address: ");
    Serial.println(WiFi.localIP());
    tft->setCursor(50, 3);
    tft->print(WiFi.localIP());

    Serial.println("");
    Serial.println(WiFiMQTTclient.getMqttClientName());
    Serial.println(WiFiMQTTclient.getMqttServerIp());
    Serial.println(WiFiMQTTclient.getMqttServerPort());
    Serial.println(WiFiMQTTclient.getConnectionEstablishedCount());
    //WiFiMQTTclient.subscribe("lighting/bed/timing", timing_Handler);
#ifdef _enableNTP
    //wait for time to sync from NTP server
    while (time(nullptr) <= 100000) {
      delay(20);
    }
    time_t now = time(nullptr);   //get current time
    struct tm * timeinfo;
    time(&now);
    timeinfo = localtime(&now);
    Serial.print("Unix epoch time is: ");
    Serial.println(now);
    char formattedTime [30];
    strftime (formattedTime, 30, "%r", timeinfo); // http://www.cplusplus.com/reference/ctime/strftime/
    Serial.print("Regular time is: ");
    Serial.println(formattedTime);
#endif
  }
  else {
    // we have a new connection but it is not the first connection after power up.
    ttgo->motor->onec();
    tft->setCursor(50, 3);
    tft->print(WiFi.localIP());
    
  }
}

void setup()  {
  Serial.begin(115200);
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->openBL();
  ttgo->lvgl_begin();

  //--------------------------------------------
  // Optional Features of EspMQTTClient
  //WiFiMQTTclient.enableDebuggingMessages(); // Enable MQTT debugging messages sent to serial output
  WiFiMQTTclient.enableHTTPWebUpdater();    // Enable the web firmware updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overwritten with enableHTTPWebUpdater("user", "password").
  //  WiFiMQTTclient.enableLastWillMessage("IoT/lastwill", "Goodbye");  // You can activate the retain flag by setting the third parameter to true
  WiFiMQTTclient.enableMQTTConnect();       //use this to enable MQTT connections.  If you comment this line out than you can use this with a WiFi only connection.
  WiFiMQTTclient.enableMACaddress_for_ClientName(); //This will use the WiFi hardware MAC address for the client name instead of the value
  power = ttgo->power;
  tft = ttgo->tft;
  ttgo->motor_begin();
  tft->setCursor(3, 3);
  power->adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);
  tft->setTextFont(2);
  tft->setTextColor(TFT_BLACK, TFT_WHITE);
  load_MainScreen();
}


enum State_enum {STATE_0, STATE_1, STATE_2, STATE_3, STATE_4, STATE_5, STATE_6};    //The possible states of the state machine
State_enum state = STATE_0;     //initialize the starting state.


void StateMachine();

void loop() {
  StateMachine();
  WiFiMQTTclient.loop();
  lv_task_handler();
  tft->setCursor(3, 3);
  tft->print(power->getBattPercentage());
  tft->println(" %  ");
}
