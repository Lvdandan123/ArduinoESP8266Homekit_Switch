# 1 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino"
# 2 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino" 2
# 3 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino" 2
# 4 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino" 2
# 5 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino" 2
# 6 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino" 2

/* Put your SSID & Password */
const char* ssid = "NodeMCU"; // Enter SSID here
const char* password = "12345678"; //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

/* use 80 port */
ESP8266WebServer server(80);

String Essid = ""; //EEPROM Network SSID
String Epass = ""; //EEPROM Network Password

bool ConnectRouterOK = false;




/**
 * 初始化时设置功能
 */
void setup()
{
  Serial.begin(115200);
  Serial.println();

  EEPROM.begin(4096);

  /* Reading EEProm SSID-Password */
  for (int i = 0; i < 32; ++i) //Reading SSID
  {
    Essid += char(EEPROM.read(i+1440 /*前1440KB给HomeKit库存储参数使用*/));
  }
  for (int i = 32; i < 96; ++i) //Reading Password
  {
    Epass += char(EEPROM.read(i+1440 /*前1440KB给HomeKit库存储参数使用*/));
  }

  ConnectRouterOK = false;
  if ( Essid.length() > 1 )
  {
    uint8_t cnt = 0;

    Serial.println(String("EEprom ssid: ") + Essid.c_str()); //Print SSID
    Serial.println(String("EEprom pass: ") + Epass.c_str()); //Print Password
    WiFi.mode(WIFI_STA);
    WiFi.begin(Essid.c_str(), Epass.c_str()); //c_str()
    WiFi.setAutoConnect(true); //设置是否自动连接到最近接入点
    WiFi.setAutoReconnect(true); //设置是否自动重新连接到最近接入点
    Serial.println("WiFi connecting...");
    while (!WiFi.isConnected())
    {
      delay(500);
      Serial.print(".");
      if (++cnt > 10) break;
    }

    if (cnt > 10)
    {
      Serial.print("\r\nUse STA mode to connect WiFi failed!\r\n");
    }
    else
    {
      ConnectRouterOK = true;
      Serial.printf("WiFi connected, IP: %s\n", WiFi.localIP().toString().c_str());
    }
  }

  if (ConnectRouterOK == false)
  {
    /* //设置为ap+sta模式 */
    WiFi.mode(WIFI_AP_STA);
    WiFi.disconnect();
    /* //设置ap模式下的ssid、密码、ip、网关、子网掩码参数 */
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    delay(100);

    Serial.println("Set to AP+STA mode to manual connect router\r\n");
    Serial.printf("Please search wifi name: %s, password: %s and access blow ip to set!\r\n", ssid, password);
    Serial.println("local ip: 192.168.1.1\r\n");
  }

  /* //设置server 的url请求回调处理 */
  server.on("/", handle_OnConnect);
  server.on("/wifiscan", handle_wifiscan);
  server.on("/wifiset", handle_wifiset);
  server.on("/rmpair", handle_removepair);
  server.onNotFound(handle_NotFound);

  /* 启动sever */
  server.begin();
  Serial.println("HTTP server started");

  /* 配置homekit */
  //homekit_storage_reset(); // to remove the previous HomeKit pairing storage when you first run this new HomeKit example
 my_homekit_setup();
}

/**
 * 循环处理
 */
void loop()
{
  //循环处理网络服务
  server.handleClient();

  //homekit
  my_homekit_loop();
 delay(10);
}


/**
 * @brief  网页端404
 * @param  none
 * @return none
 */
void handle_NotFound()
{
  server.send(404, "text/plain", "404: Not found");
  Serial.print(WiFi.localIP());
  Serial.println(server.uri()); //串口输出当前客户端的请求路径
}

/**
 * @brief  网页端初始连接时执行的callback
 * @param  none
 * @return none
 */
void handle_OnConnect()
{
  Serial.println("Client connect...");
  server.send(200, "text/html",scan_wifi(false));
}

/**
 * @brief  网页端点击“scanwifi”按钮后执行的callback
 * @param  none
 * @return none
 */
void handle_wifiscan(void)
{
  server.send(200, "text/html",scan_wifi(true));
}


void ClearEeprom(void)
{
  Serial.println("Clearing EEprom");
  for (int i = 0; i < 96; ++i)
  {
    EEPROM.write(i+1440 /*前1440KB给HomeKit库存储参数使用*/, 0);
  }
  EEPROM.commit();
}

/**
 * @brief  网页端点击“submit”按钮后执行的callback
 * @param  none
 * @return none
 */
void handle_wifiset(void)
{
  String sssid="";
  String passs="";
  uint8_t time_out_cnt;

  if (server.hasArg("ssid") && server.hasArg("pass"))
  {
    sssid = server.arg("ssid"); //Get SSID
    passs = server.arg("pass"); //Get Password
  }

  if(sssid.length()>1 && passs.length()>1)
  {
      time_out_cnt = 10; //10 second timeout
      WiFi.begin(sssid.c_str(), passs.c_str()); //c_str()
      while (WiFi.status() != WL_CONNECTED) //Wait for IP to be assigned to Module by Router
      {
        delay(1000);
        Serial.print(".");
        if (!(--time_out_cnt)) break;
      }
      /* 判断是否连接超时 */
      if (time_out_cnt)
      {
        /* EEprom 数据存储 */
        ClearEeprom(); //First Clear Eeprom
        delay(10);
        for (int i = 0; i < sssid.length(); ++i)
        {
          EEPROM.write(i+1440 /*前1440KB给HomeKit库存储参数使用*/, sssid[i]);
        }
        for (int i = 0; i < passs.length(); ++i)
        {
          EEPROM.write(32+i+1440 /*前1440KB给HomeKit库存储参数使用*/, passs[i]);
        }
        EEPROM.commit();
        Serial.println("Save ssid & pass to eeprom OK!");

        /* web显示、串口打印以及STA模式切换 */
        String ipaddr = WiFi.localIP().toString(); //Get ESP8266 IP Adress
        Serial.println("");
        Serial.println("WiFi connected..!");
        Serial.print("Got IP: "); Serial.println(ipaddr);

        String s = "\r\n\r\n<!DOCTYPE HTML>\r\n<html><h1>WiFi Connected!</h1> ";
        s += "<p>IP=";
        s += ipaddr;
        s += "</html>\r\n\r\n";
        server.send(200,"text/html",s);
        delay(1000);

        //切换到STA模式
        WiFi.mode(WIFI_STA);
        while (WiFi.status() != WL_CONNECTED)
        {
          delay(500);
          Serial.print(".");
        }
      }
      else /* 连接超时 */
      {
          Serial.println("WiFi connect failed!\r\n");
      }
  }
}


/**
 * @brief  扫描可用的wifi并在串口和网页端分别显示
 * @param  scanflag-true:刷新网页前扫描wifi；false：直接刷新网页，不扫描wifi以提高速度
 * @return 以字符串形式返回网页html内容
 */
String scan_wifi(bool scanflag)
{
  int num = 0;

 if (scanflag == true)
  {
    Serial.printf("Scan WiFi access point, please wait......\r\n");
    num= WiFi.scanNetworks();
    Serial.printf("Scan done!\r\n");
  }

  //网页端刷新
  String ps = "<!DOCTYPE html> <html><head><meta charset=\"UTF-8\"></meta><title>Homekit设备接入点设置</title></head>\n";
 ps += "<body><h2 >Homekit设备Wifi接入点设置</h2><h3 >by LIV</h3><div>\n";
  ps += "<form method=\"post\" action=\"wifiscan\"><input type=\"submit\" value=\"Scan for networks\">\n";
  ps += "</form></div><div>\n";
 ps += "<form method=\"post\" action=\"wifiset\">\n";
 ps += "<label for=\"SSIDselect\">Select SSID: </label><select id=\"SSIDselect\" name=\"ssid\"><option value="">--Please choose an WiFi--</option>\n";
  for (int i = 0; i < num; i++)
  {
    ps += "<option value=";
    ps += WiFi.SSID(i);
    ps += ">";
    ps += i+1;
    ps += ": ";
    ps += WiFi.SSID(i);
    ps += " (";
    ps += WiFi.RSSI(i);
    ps += "dBm)";
    ps += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*";
    ps += "</option>\n";
  }
 ps += "</select><br><label for=\"password\">Password:</label><input type=\"password\" id=\"password\" name=\"pass\"><br>\n";
 ps += "<input type=\"reset\" value=\"Reset\" >&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type=\"submit\" value=\"Submit\">\n";
  ps += "</form></div>\n";
  ps += "<div><form method='post' action='rmpair'><br><input type='submit' value='RemoveHomekitPairing'></form></div>";
  ps += "</body>\n";
  ps += "</html>\n";

  //串口端刷新
  for (int i = 0; i < num; i++)
  {
    Serial.printf("%d: %s, Ch:%d (%ddBm) %s\n",
                  i+1,
                  WiFi.SSID(i).c_str(),
                  WiFi.channel(i),
                  WiFi.RSSI(i),
                  WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "encryp");
  }

  return ps;
}
void handle_removepair(void)
{
  // to remove the previous HomeKit pairing storage when you first run this new HomeKit example
  printf_P((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "Blink.ino" "." "299" "." "100" "\", \"aSM\", @progbits, 1 #"))) = ("Remove the previous HomeKit pairing storage\n" "\n"); &__c[0];})));;
  homekit_storage_reset();
  server.send(200, "text/html", "Remove HomeKit pairing OK!");
}
# 341 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino"
//==============================
// HomeKit setup and loop
//==============================
// access your HomeKit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_switch_on;

/* D7开关量输出 */




//Called when the switch value is changed by iOS Home APP
void cha_switch_on_setter(const homekit_value_t value) {
 bool on = value.bool_value;
 cha_switch_on.value.bool_value = on; //sync the value
 printf_P((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "Blink.ino" "." "357" "." "101" "\", \"aSM\", @progbits, 1 #"))) = ("Switch: %s" "\n"); &__c[0];})) , on ? "ON" : "OFF");;
 digitalWrite(D7, on ? 0x0 : 0x1);
}

void my_homekit_setup() {
  //初始化IO输出引脚
 pinMode(D7, 0x01);
 digitalWrite(D7, 0x0);

 //Add the .setter function to get the switch-event sent from iOS Home APP.
 //The .setter should be added before arduino_homekit_setup.
 //HomeKit sever uses the .setter_ex internally, see homekit_accessories_init function.
 //Maybe this is a legacy design issue in the original esp-homekit library,
 //and I have no reason to modify this "feature".
 cha_switch_on.setter = cha_switch_on_setter;
 arduino_homekit_setup(&config);

 //report the switch value to HomeKit if it is changed (e.g. by a physical button)
 //bool switch_is_on = true/false;
 //cha_switch_on.value.bool_value = switch_is_on;
 //homekit_characteristic_notify(&cha_switch_on, cha_switch_on.value);
}

void my_homekit_loop()
{
  static uint32_t next_heap_millis = 0;
 uint32_t t;

 arduino_homekit_loop();
  t = millis();

 if (t > next_heap_millis) {
  // show heap info every 5 seconds
  next_heap_millis = t + 5 * 1000;
  printf_P((__extension__({static const char __c[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr."
 "Blink.ino"
# 391 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino"
  "." "392" "." "102" "\", \"aSM\", @progbits, 1 #"))) = ("Free heap: %d, HomeKit clients: %d" "\n"); &__c[0];})) , ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
                                                                 ;
 }
}
