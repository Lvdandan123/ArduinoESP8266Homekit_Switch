#include <Arduino.h>
#line 1 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

/* Put your SSID & Password */
const char* ssid = "NodeMCU";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);

uint8_t LED1pin = D7;
bool LED1status = LOW;

uint8_t LED2pin = D6;
bool LED2status = LOW;

String Essid = "";                  //EEPROM Network SSID
String Epass = "";                 //EEPROM Network Password

bool ConnectRouterOK = false;

/**
 * initialize function
 */
#line 30 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino"
void setup();
#line 111 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino"
void loop();
#line 144 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino"
void handle_led1on();
#line 149 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino"
void handle_led1off();
#line 154 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino"
void handle_led2on();
#line 159 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino"
void handle_led2off();
#line 166 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino"
void handle_NotFound();
#line 175 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino"
void handle_OnConnect();
#line 180 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino"
void handle_wifiscan(void);
#line 185 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino"
void ClearEeprom();
#line 193 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino"
void handle_wifiset(void);
#line 261 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino"
String SendHTML(uint8_t led1stat,uint8_t led2stat);
#line 300 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino"
String scan_wifi(bool scanflag);
#line 30 "d:\\Users\\Administrator\\Desktop\\Blink\\Blink.ino"
void setup() 
{
  Serial.begin(115200);
  Serial.println();

  EEPROM.begin(512);

  pinMode(LED1pin, OUTPUT);
  pinMode(LED2pin, OUTPUT);

  /* Reading EEProm SSID-Password */
  for (int i = 0; i < 32; ++i)  //Reading SSID
  {
    Essid += char(EEPROM.read(i)); 
  }
  for (int i = 32; i < 96; ++i)  //Reading Password
  {
    Epass += char(EEPROM.read(i)); 
  }

  ConnectRouterOK = false;
  if ( Essid.length() > 1 )
  {
    uint8_t cnt = 0;

    Serial.println(String("EEprom ssid: ") + Essid.c_str());                            //Print SSID
    Serial.println(String("EEprom pass: ") + Epass.c_str());                            //Print Password
    WiFi.mode(WIFI_STA);
    WiFi.begin(Essid.c_str(), Epass.c_str());         //c_str()
    WiFi.setAutoConnect(true);  //设置是否自动连接到最近接入点
    WiFi.setAutoReconnect(true);  //设置是否自动重新连接到最近接入点

    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(1000);
      Serial.print(".");
      if (++cnt > 10) break;
    }

    if (cnt > 10)
    {
      Serial.print("\r\nSTA mode connect WiFi failed!\r\n");
    }
    else
    {
      ConnectRouterOK = true;
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
    Serial.printf("Please search wifi name: %s, password: %s\r\n", ssid, password);
    Serial.println("local ip: 192.168.1.1\r\n");
  }
  
  /* //设置server 的url请求回调处理 */
  // server.on("/led1on", handle_led1on);
  // server.on("/led1off", handle_led1off);
  // server.on("/led2on", handle_led2on);
  // server.on("/led2off", handle_led2off);
  server.on("/", handle_OnConnect);
  server.on("/wifiscan", handle_wifiscan);
  server.on("/wifiset", handle_wifiset);
  server.onNotFound(handle_NotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}

/**
 * infinty loop
 */
void loop() 
{
  //循环处理网络服务
  server.handleClient();

  if(LED1status)
  {
    digitalWrite(LED1pin, HIGH);
  }
  else
  {
    digitalWrite(LED1pin, LOW);
  }
  
  if(LED2status)
  {
    digitalWrite(LED2pin, HIGH);
  }
  else
  {
    digitalWrite(LED2pin, LOW);
  }
}

/**
 * 设备连接到esp8266
 */
// void handle_OnConnect() {
//   LED1status = LOW;
//   LED2status = LOW;
//   Serial.println("GPIO7 Status: OFF | GPIO6 Status: OFF");
//   server.send(200, "text/html",scan_wifi(false)); 
// }
void handle_led1on() {
  LED1status = HIGH;
  Serial.println("GPIO7 Status: ON");
  server.send(200, "text/html", SendHTML(true,LED2status)); 
}
void handle_led1off() {
  LED1status = LOW;
  Serial.println("GPIO7 Status: OFF");
  server.send(200, "text/html", SendHTML(false,LED2status)); 
}
void handle_led2on() {
  LED2status = HIGH;
  Serial.println("GPIO6 Status: ON");
  server.send(200, "text/html", SendHTML(LED1status,true)); 
}
void handle_led2off() {
  LED2status = LOW;
  Serial.println("GPIO6 Status: OFF");
  server.send(200, "text/html", SendHTML(LED1status,false)); 
}


void handle_NotFound(){
  server.send(404, "text/plain", "404: Not found");
  Serial.print(WiFi.localIP()); 
  Serial.println(server.uri());  //串口输出当前客户端的请求路径
  
  // Serial.print("Client requset method :"); 
  // Serial.println(server.method());  //串口输出当前客户端的请求方
}

void handle_OnConnect() {
  Serial.println("Client connect...");
  server.send(200, "text/html",scan_wifi(false)); 
}

void handle_wifiscan(void)
{
  server.send(200, "text/html",scan_wifi(true));
}

void ClearEeprom()
{
        Serial.println("Clearing EEprom");
        for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
}
/**
 * 客户端设置路由器wifi后进行接入
 */
void handle_wifiset(void)
{
  String sssid="";
  String passs="";
  uint8_t time_out_cnt;

  if (server.hasArg("ssid") && server.hasArg("pass"))
  {  
    sssid = server.arg("ssid");   //Get SSID
    passs = server.arg("pass");   //Get Password
  }

  if(sssid.length()>1 && passs.length()>1)
  {
      time_out_cnt = 10;            //10 second timeout
      WiFi.begin(sssid.c_str(), passs.c_str());   //c_str()
      while (WiFi.status() != WL_CONNECTED)  //Wait for IP to be assigned to Module by Router
      {
        delay(1000);
        Serial.print(".");
        if (!(--time_out_cnt)) break;
      }
      /* 判断是否连接超时 */
      if (time_out_cnt)
      {
        /* EEprom 数据存储 */
        ClearEeprom();  //First Clear Eeprom
        delay(10);
        for (int i = 0; i < sssid.length(); ++i)
        {
          EEPROM.write(i, sssid[i]);
        }
        for (int i = 0; i < passs.length(); ++i)
        {
          EEPROM.write(32+i, passs[i]);
        }    
        EEPROM.commit();
        Serial.println("Save ssid & pass to eeprom OK!");

        /* web显示、串口打印以及STA模式切换 */
        String ipaddr = WiFi.localIP().toString();           //Get ESP8266 IP Adress
        Serial.println("");
        Serial.println("WiFi connected..!");
        Serial.print("Got IP: ");  Serial.println(ipaddr);

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
      else    /* 连接超时 */
      {
          Serial.println("WiFi connect failed!\r\n");
      }
  }
}

#if 1
String SendHTML(uint8_t led1stat,uint8_t led2stat)
{
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #1abc9c;}\n";
  ptr +=".button-on:active {background-color: #16a085;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP8266 Web Server</h1>\n";
  ptr +="<h3>Using Access Point(AP) Mode</h3>\n";
  
  if(led1stat)
    {ptr +="<p>LED1 Status: ON</p><a class=\"button button-off\" href=\"/led1off\">OFF</a>\n";}
  else
    {ptr +="<p>LED1 Status: OFF</p><a class=\"button button-on\" href=\"/led1on\">ON</a>\n";}

  if(led2stat)
    {ptr +="<p>LED2 Status: ON</p><a class=\"button button-off\" href=\"/led2off\">OFF</a>\n";}
  else
    {ptr +="<p>LED2 Status: OFF</p><a class=\"button button-on\" href=\"/led2on\">ON</a>\n";}

  ptr +="</body>\n";
  ptr +="</html>\n";

  return ptr;
}
#endif

/**
 * 扫描可用的wif并在串口和网页端分别显示
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
    ps += ": ";
    ps += i+1;
    ps += WiFi.SSID(i);
    ps += " (";
    ps += WiFi.RSSI(i);
    ps += "dBm)";
    ps += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*";
    ps += "</option>\n";
  }
	ps += "</select><br><label for=\"password\">Password:</label><input type=\"password\" id=\"password\" name=\"pass\"><br>\n";
	ps += "<input type=\"reset\" value=\"Reset\" >&nbsp;&nbsp;&nbsp;&nbsp;<input type=\"submit\" value=\"Submit\">\n";
  ps += "</form></div>\n";
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
