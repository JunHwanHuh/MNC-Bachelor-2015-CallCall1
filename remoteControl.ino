//150902 05:36
//new
#include <SPI.h>
#include <WiFi.h>
#include <IRremote.h>
#include <String.h>
#define led 9
#define MAX 250
#define nMAX 200
#define mMAX 20
#define XbeeDelay 1000

//unsigned long time_previous, time_current;

//#define XbeeMAX 50//for Xbee
char ssid[] = "AndroidHotspot9770"; //  your network SSID (name)
char pass[] = "65159770";    // your network password (use for WPA, or use as key for WEP)
//int keyIndex = 0;            // your network key Index number (needed only for WEP)
const int PORT = 3334;
char jsonComp[11] = ""; //from server company
char jsonBit[11] = ""; //from server company
char XbeeComp[11] = ""; //from server company
char XbeeBit[11] = ""; //from server company
int RECV_PIN = 2;
IRrecv irrecv(RECV_PIN);
IRsend irsend;
decode_results results;
long tempDash[3];//3 input
int dashCount = 0; //multi data index
int multi = 0;// 0 - single data 1- multi data
int flag = 0; // 0- wait detect 1- detect
char x;//Serial.read

int whereComp[9];
int whereBit[9];
int Count = 0;
char fromServer[MAX];
//char fromServer2[nMAX];
String input;
String input2;
char input3[mMAX];
String inputXbee;
  
//String singleComp;
//String singleBit;
int status = WL_IDLE_STATUS;// use the numeric IP instead of the name for the server:
IPAddress server(52, 68, 143, 227); // numeric IP
//char server[] = "ec2-52-68-143-227.ap-northeast-1.compute.amazonaws.com";
WiFiClient client;

void setup() {
    irrecv.enableIRIn();
    Serial.begin(9600);//check - always open
    connectWifi();
    pinMode(led, OUTPUT);   
  
    jsonBit[0] = '0';
    jsonBit[1] = 'x';
    XbeeBit[0] = '0';
    XbeeBit[1] = 'x';
  
    if (client.connect(server, PORT)) {
      //Serial.println("connected to server");
      // Make a HTTP request:
      client.print("GET /?0x");
      client.print(16, HEX);//garbage
      //client.print(results.value, HEX);//3772793023
      client.println("  HTTP/1.1");
      client.print("Host: 52,68,143,227:");
      client.println(PORT);
      client.println("Connection: close");
      client.println();
    }
}

void loop() {
  // if there are incoming bytes available from the server read them and print them:

    if (multi == 0) {//single input from server      
          while (client.available()) { 
              char c = client.read();
              fromServer[Count++] = c;//char *formServer = new char[MAX];
          }
          //server's disconnected, stop the client:
          if (!client.connected()) {
                for (int i = 0 ; i < 11; i++) jsonComp[i] = NULL;
                for (int i = 2 ; i < 11; i++) jsonBit[i] = NULL;
                  Count = 0;
                  input = fromServer;
                  whereComp[0] = input.indexOf("Comp")+7; // ~ whereBit-10
                  whereBit[0] = input.indexOf("0x");
                  
                  String singleComp = input.substring(whereComp[0], whereBit[0]-30);//-9
                  String singleBit = input.substring(whereBit[0], whereBit[0]+10);
                  
                  singleComp.toCharArray(jsonComp, 11);
                  singleBit.toCharArray(jsonBit, 11);
                  sendXbee(jsonComp, jsonBit);
                  sendSignal(jsonComp, jsonBit);
        
                for(int i= 100; i<MAX; i++) fromServer[i] = NULL;
                for (int i = 0 ; i < 11; i++) jsonComp[i] = NULL;
                for (int i = 2 ; i < 11; i++) jsonBit[i] = NULL;
                client.stop();
                flag = 0;
                connectServer();
            }
    }//single = multi X
    else { //multi O
    
          while (client.available()) { 
              char c = client.read();
                 fromServer[Count++] = c;
          }
          
          if (!client.connected()) {
                for (int i = 0 ; i < 11; i++) jsonComp[i] = NULL;
                for (int i = 2 ; i < 11; i++) jsonBit[i] = NULL;
                Count = 0;
                input = fromServer;
                whereComp[0] = input.indexOf("#");
                whereBit[0] = input.indexOf("0x");
                //input.remove(0, whereComp[0]-1);//문단
   //-Samsung0xE0E040BF0xE0E020DF0xE0E0A05F0xE0E0609F0xE0E0609F-Lg0x20DF10EF0x20DF48B70x20DF48B70x20DFC8370x20DFC837
   //01234567         
                String inputJsonComp = input.substring( whereComp[0]+1, whereBit[0]);
                inputJsonComp.toCharArray(jsonComp, 11);
                
                String inputJsonBit = input.substring( whereBit[0], whereBit[0]+10 );
                inputJsonBit.toCharArray(jsonBit, 11);
                sendXbee(jsonComp, jsonBit);
                  delay(XbeeDelay);
                sendSignal(jsonComp, jsonBit);//finish 1-1
                if(inputJsonBit == "0xE0E040BF" || inputJsonBit == "0x20DF10EF" ) delay(9000);
                
                
                whereComp[1] = input.indexOf("#",whereComp[0]+1);
                if(whereComp[1] != -1){// right? -1??? no more #
                 //if next OK
                        for(int k=1;  ; k++){
                            whereBit[k] = input.indexOf("0x", whereBit[k-1]+1);
                            if(whereBit[k] < whereComp[1]){
                                  String inputJsonBit = input.substring( whereBit[k], whereBit[k]+10 );
                                  inputJsonBit.toCharArray(jsonBit, 11);
                                  sendXbee(jsonComp, jsonBit);
                  delay(XbeeDelay);
                                  sendSignal(jsonComp, jsonBit);//finish 1
                            }
                            else if(whereBit[k] > whereComp[1]) break;
                            else if(whereBit[k] == -1 ) break;
                            
                        }//Multi Bit
                        
                        for(int j=0; j<11 ; j++) jsonComp[j] = NULL;
                        
                        whereBit[0] = input.indexOf("0x", whereComp[1]+1);
                        inputJsonComp = input.substring( whereComp[1]+1, whereBit[0]);
                        inputJsonComp.toCharArray(jsonComp, 11);
                        inputJsonBit = input.substring( whereBit[0], whereBit[0]+10 );
                        inputJsonBit.toCharArray(jsonBit, 11);
                        sendXbee(jsonComp, jsonBit);
                  delay(XbeeDelay);
                        sendSignal(jsonComp, jsonBit);//finish 2-1
                        if(inputJsonBit == "0xE0E040BF" || inputJsonBit == "0x20DF10EF" ) delay(9000);
                        
        //ifnext # XX
                                for(int k=1;  ; k++){
                                      whereBit[k] = input.indexOf("0x", whereBit[k-1]+1);
                                      inputJsonBit = input.substring( whereBit[k], whereBit[k]+10 );
                                      inputJsonBit.toCharArray(jsonBit, 11);
                                      sendXbee(jsonComp, jsonBit);
                  delay(XbeeDelay);
                                      sendSignal(jsonComp, jsonBit);
                                      if(whereBit[k] == -1 ) break;
                                  }//Multi Bit         
                }//next OK
         
                for(int i= 100; i<MAX; i++) fromServer[i] = NULL;
                client.stop();
                flag = 0;
                multi = 0;
                connectServer();      
                
          }//if(!client.connected())
     }//multi
}//loop


void connectServer() {
  //Serial.println("\n PUSH the button ");
  while (flag == 0) {
    //if Xbee Receive -> sendSignal 
    checkXbee();
    
    if ( irrecv.decode(&results) ) {
      if ( results.value == LGSharp || results.value == SSDash ) { //#
        multi = 1;
        irrecv.resume();//#
        while (dashCount < 3 ) {
          //Serial.print(dashCount);
          if ( irrecv.decode(&results)  ) {
            if (results.value == LGSharp || results.value == SSDash ) irrecv.resume();
            else {
              tempDash[dashCount++] = results.value;
              irrecv.resume();
            }
          }// under 3
        }//end 3
        
        
        dashCount = 0;
        for (int i = 0; i < MAX_SOCK_NUM; i++) WiFi._state[i] = -1;
        if (client.connect(server, PORT)) {
          client.print("GET /?first=0x");
          client.print(tempDash[0], HEX);
          client.print("&second=0x");
          client.print(tempDash[1], HEX);
          client.print("&third=0x");
          client.print(tempDash[2], HEX);
          client.println("  HTTP/1.1");
          client.print("Host: 52,68,143,227:");
          client.println(PORT);
          client.println("Connection: close");
          client.println();
        }
        flag = 1 ;
        irrecv.resume();
      }//case #
      else { //no # or *
        multi = 0;
        for (int i = 0; i < MAX_SOCK_NUM; i++) WiFi._state[i] = -1;
        if (client.connect(server, PORT)) {
          client.print("GET /?0x");
          client.print(results.value, HEX);
          client.println("  HTTP/1.1");
          client.print("Host: 52,68,143,227:");
          client.println(PORT);
          client.println("Connection: close");
          client.println();
        }
        flag = 1 ;
        irrecv.resume();//where?
      }
    }//detect
  }//flag - detect
}//

/*
void checkXbee(){
  //-Samsung!0xE0E0B04F 
  //-Lg!0xE0E0B04F
  //0xE0E030CF0xE0E050AF0xE0E0906F(X)
  //char* find할수 있지만 0x를 찾아야 되기 때문에 Stirng으로 처리
  for(int i=0 ; i<mMAX ; i++) input2[i] = NULL;
  int whereN = 0;
  int serialCount = 0;
  //String inputXbeeComp;
  //String inputXbeeBit;
  
  char x = Serial.read();// cut "-"
  //-Samsung!0xE0E040BF
  Serial.print(x);
  if(x =='-'){
        while (Serial.available()) {
          delay(40);
             x = Serial.read();
             input3[serialCount++] = x;
             Serial.print(x);
        }
Serial.println(input3);
        serialCount=0;
        inputXbee=input3;
Serial.println(inputXbee);
        
        for (int i = 0 ; i < 11; i++) XbeeComp[i] = NULL;
        for (int i = 2 ; i < 11; i++) XbeeBit[i] = NULL;

        whereN = inputXbee.indexOf("!");
        String inputXbeeComp = inputXbee.substring(0,whereN);//inputXbee.substr(0,whereN) -> comp
        String inputXbeeBit = inputXbee.substring(whereN+1, whereN+1);
  
        inputXbeeComp.toCharArray(XbeeComp, 11);
        inputXbeeBit.toCharArray(XbeeBit, 11);
        
        sendSignal(XbeeComp,XbeeBit);
        
        
  }// end x== '-'
}
*/

void sendXbee(char* jsonComp, char*jsonBit){
    //Xbee send
              Serial.print("-");
              Serial.print(jsonComp);
              Serial.print("!");
              Serial.println(jsonBit);  
}


// Original
void checkXbee(){
  //time_previous = millis(); 
          //digitalWrite(led, HIGH); 
          //Xbee receive
          x = Serial.read();
          delay(40);
          if(x =='-'){
                 x = Serial.read();   
          delay(40); 
                 for (int i = 0; x != '!'; i++) {
                      XbeeComp[i] = x;
                      x = Serial.read();
          delay(40);
                 }
          delay(40);
                 //x = ! out
                 for( int i= 0; i<10; i++) XbeeBit[i] = Serial.read();
                 
                   sendSignal(XbeeComp,XbeeBit);
                   
                   for (int i = 0 ; i < 11; i++) XbeeComp[i] = NULL;
                   for (int i = 2 ; i < 10; i++) XbeeBit[i] = NULL;//i=0 or 2 no matter
            }
          //digitalWrite(led, LOW);
          //time_current = millis(); 
          //Serial.print("ReceiveXbee Time : ");
          //Serial.println(  (time_current - time_previous) );
}


void sendSignal(char *jsonComp, char *jsonBit) {
  /*
  #define NEC 1
  #define SONY 2
  #define RC5 3
  #define RC6 4
  #define DISH 5
  #define SHARP 6
  #define SAMSUNG 7
  #define LG 8
  #define UNKNOWN -1
  */

  //char -> unsinged long
  unsigned long transData = strtoul(jsonBit, NULL, 16);
  //Serial.print("transData HEX : ");
  //Serial.println(transData,16);

  for (int i = 2; i < 10; i++) jsonBit[i] = 0;
  if ( strcmp(jsonComp, "Nec") == 0 ) {
      irsend.sendNEC(transData, 32);
      irrecv.enableIRIn();
      delay(40);
    Serial.print("send NEC : ");
    Serial.println(transData, 16);
  }
  else if (strcmp(jsonComp, "Sony") == 0) {
      irsend.sendSony(transData, 32);
      irrecv.enableIRIn();
      delay(40);
    Serial.print("send SONY : ");
    Serial.println(transData, 16);
  }
  else if ( strcmp(jsonComp, "Rc5") == 0 ) {
      irsend.sendRC5(transData, 32);
      irrecv.enableIRIn();
      delay(40);
    Serial.print("send RC5 : ");
    Serial.println(transData, 16);
  }
  else if ( strcmp(jsonComp, "Rc6") == 0 ) {
      irsend.sendRC6(transData, 32);
      irrecv.enableIRIn();
      delay(40);
    //Serial.print("send RC6 : ");
    Serial.println(transData, 16);
  }
  else if ( strcmp(jsonComp, "Dish") == 0 ) {
      irsend.sendDISH(transData, 32);
      irrecv.enableIRIn();
      delay(40);
    Serial.print("send DISH : ");
    Serial.println(transData, 16);
  }
  else if ( strcmp(jsonComp, "Sharp") == 0 ) {
      irsend.sendSharp(transData, 32);
      irrecv.enableIRIn();
      delay(40);
    Serial.print("send Sharp : ");
    Serial.println(transData, 16);
  }
  else if ( strcmp(jsonComp, "Samsung") == 0 ) {
      irsend.sendSAMSUNG(transData, 32);
      irrecv.enableIRIn();
      delay(40);
    Serial.print("send Samsung : ");
    Serial.println(transData, 16);
  }
  else if ( strcmp(jsonComp, "Lg") == 0 ) {
      irsend.sendLG(transData, 32);
      irrecv.enableIRIn();
      delay(40);
    Serial.print("send LG : ");
    Serial.println(transData, 16);
  }
  else {
      irsend.sendSAMSUNG(transData, 32);
      irrecv.enableIRIn();
      delay(40);
    Serial.print("send UN : ");
    Serial.println(transData, 16);
  }
}

void connectWifi() {

  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    //Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if ( fv != "1.1.0" ) Serial.println("Please upgrade the firmware");

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    //Serial.print("Attempting to connect to SSID: ");
    //Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  //Serial.println("Connected to wifi");
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
