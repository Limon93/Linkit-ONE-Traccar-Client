// --------------TRACCAR LINKIT ONE CLIENT------------------
// USAGE: SET PARAMETERS, PROGRAM THE BOARD, INSERT GPRS WEB ENABLED SIM, CONNECT GSM & GPS ANTENNAS
// LOCATION UPDATE IS PROVIDED VIA GPRS CONNECTION, USING OSMAND PROTOCOL,INFO AT: https://www.traccar.org/osmand/
// GPRS MOBILE WEB MUST BE INITIALIZED WITH CARRIER APN, CARRIER APN USERNAME & CARRIER APN PASSWORD, LAST TWO CAN BE OMITTED.
// REMEBER TO REGISTER USERID ON YOUR TRACCAR SERVER TO MATCH CLIENT USERID DEFINED IN PARAMETERS
// HAVE FUN
// 
// AUTHOR: LIMON93


#include <LTask.h>
#include <vmthread.h>
#include <stddef.h>
#include <LGPS.h>
#include <LGSM.h>
#include <math.h>
#include <LDateTime.h>
#include <LStorage.h>
#include <LFlash.h>
#include <LGPRSClient.h>
#include <LGPRSServer.h>
#include <LGPRS.h>
#include "struct.h"
#define LEDGPS   13


//PARAMETERS

#define  UPDATE_FREQ     10    // INSERT CLIENT POSITION UPDATE INTERVAL IN SECONDS
char USERID[20] =  "123456789"; // INSERT YOUR TRACCAR SERVER USER ID
char server[] = "demo.traccar.org"; //INSERT YOUR SERVER ADDRESS (aaa.com or xxx.xxx.xxx.xxx)
int port = 5055; // INSERT YOUR SERVER PORT
char APN[]="internet.com"; //INSERT YOUR MOBILE GPRS INTERNET APN
char APNUSER[]=""; //INSERT YOUR MOBILE GPRS INTERNET APN USERNAME, LEAVE BLANK IF NONE
char APNPASS[]="";//INSERT YOUR MOBILE GPRS INTERNET APN PASSWORD, LEAVE BLANK IF NONE



gpsSentenceInfoStruct info;
LGPRSClient client;
char buff[512];


static double getDoubleNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atof(buf);
  return rev; 
}
static unsigned char getComma(unsigned char num,const char *str){
	unsigned char i,j = 0;
	int len=strlen(str);
	for(i = 0;i < len;i ++){
		if(str[i] == ',')
			j++;
		if(j == num)
			return i + 1; 
		}
	return 0; 
}

static float getFloatNumber(const char *s){
	char buf[10];
	unsigned char i;
	float rev;

	i=getComma(1, s);
	i = i - 1;
	strncpy(buf, s, i);
	buf[i] = 0;
	rev=atof(buf);
	return rev; 
}

static float getIntNumber(const char *s){
	char buf[10];
	unsigned char i;
	float rev;

	i=getComma(1, s);
	i = i - 1;
	strncpy(buf, s, i);
	buf[i] = 0;
	rev=atoi(buf);
	return rev; 
}

void parseGPGGA(const char* GPGGAstr){
	if(GPGGAstr[0] == '$'){
		int tmp;
		tmp = getComma(1, GPGGAstr);
		MyGPSPos.hour     = (GPGGAstr[tmp + 0] - '0') * 10 + (GPGGAstr[tmp + 1] - '0');
		MyGPSPos.minute   = (GPGGAstr[tmp + 2] - '0') * 10 + (GPGGAstr[tmp + 3] - '0');
		MyGPSPos.second    = (GPGGAstr[tmp + 4] - '0') * 10 + (GPGGAstr[tmp + 5] - '0');

		//get time
		sprintf(buff, "UTC time %02d:%02d:%02d", MyGPSPos.hour, MyGPSPos.minute, MyGPSPos.second);
		Serial.print(buff);
		//get lat/lon coordinates
		float latitudetmp;
		float longitudetmp;
		tmp = getComma(2, GPGGAstr);
		latitudetmp = getFloatNumber(&GPGGAstr[tmp]);
		tmp = getComma(4, GPGGAstr);
		longitudetmp = getFloatNumber(&GPGGAstr[tmp]);
		// need to convert format
		convertCoords(latitudetmp, longitudetmp, MyGPSPos.latitude, MyGPSPos.longitude);
		//get lat/lon direction
		tmp = getComma(3, GPGGAstr);
    if(GPGGAstr[tmp] == 'N')
    { // do nothing
    }
    else if(GPGGAstr[tmp] == 'S')
    {
       MyGPSPos.latitude = -MyGPSPos.latitude;
    }
    else
    {
      sprintf(buff, "Error: found %i(%c). Expected N or S.", GPGGAstr[tmp], GPGGAstr[tmp]);
      Serial.println(buff);
    }
		MyGPSPos.latitude_dir = (GPGGAstr[tmp]);
		tmp = getComma(5, GPGGAstr);
    if(GPGGAstr[tmp] == 'E')
    { // do nothing
     }
     else if(GPGGAstr[tmp] == 'W')
    {
      MyGPSPos.longitude = -MyGPSPos.longitude;
    }
    else
    {
      sprintf(buff, "Error: found %i(%c). Expected E or W.", GPGGAstr[tmp], GPGGAstr[tmp]);
      Serial.println(buff);
    }
		MyGPSPos.longitude_dir = (GPGGAstr[tmp]);
		
		tmp = getComma(6, GPGGAstr);
		MyGPSPos.fix = getIntNumber(&GPGGAstr[tmp]);    
		sprintf(buff, "  -  GPS fix quality = %d", MyGPSPos.fix);
		Serial.print(buff);   
		//get satellites in view
		tmp = getComma(7, GPGGAstr);
		MyGPSPos.num = getIntNumber(&GPGGAstr[tmp]);    
		sprintf(buff, "  -  %d satellites", MyGPSPos.num);
		Serial.println(buff); 
    tmp = getComma(9, GPGGAstr);
    MyGPSPos.alt = getIntNumber(&GPGGAstr[tmp]); 
	}
	else{
		Serial.println("No GPS data"); 
	}
}


void convertCoords(float latitude, float longitude, float &lat_return, float &lon_return){
	int lat_deg_int = int(latitude/100);		//extract the first 2 chars to get the latitudinal degrees
	int lon_deg_int = int(longitude/100);		//extract first 3 chars to get the longitudinal degrees
    // must now take remainder/60
    // this is to convert from degrees-mins-secs to decimal degrees
    // so the coordinates are "google mappable"
    float latitude_float = latitude - lat_deg_int * 100;		//remove the degrees part of the coordinates - so we are left with only minutes-seconds part of the coordinates
    float longitude_float = longitude - lon_deg_int * 100;     
    lat_return = lat_deg_int + latitude_float / 60 ;			//add back on the degrees part, so it is decimal degrees
    lon_return = lon_deg_int + longitude_float / 60 ;
}


void GetGPSPos(void){
		Serial.println("--- LGPS loop ---"); 
		LGPS.getData(&info);
		parseGPRMC((const char*)info.GPRMC);
    LGPS.getData(&info);
    parseGPGGA((const char*)info.GPGGA);
				
		//check fix 
		//if GPS fix is OK
		if ( MyGPSPos.fix == GPS || MyGPSPos.fix == DGPS || MyGPSPos.fix == PPS ){
			//set a flag
			MyFlag.fix3D = true;
		}
		else{
			//reset flag 
			MyFlag.fix3D = false;
		}
		sprintf(buff, "Current position is : https://www.google.com/maps?q=%2.6f,%3.6f Alititude=%d", MyGPSPos.latitude,MyGPSPos.longitude,MyGPSPos.alt);
		Serial.println(buff);
		Serial.println();
}
void parseGPRMC(const char* GPRMCstr)
{
  double spd;
  int tmp, hour, minute, second, date, num ;
  if(GPRMCstr[0] == '$')
  {
    tmp = getComma(7, GPRMCstr);
    spd = getDoubleNumber(&GPRMCstr[tmp]);    
    sprintf(buff, "Speed(knots) = %5.2f", spd);
    Serial.println(buff); 
    MyGPSPos.spd=spd;
  }
  else
  {
    Serial.println("Unable to get speed data."); 
  }
}



void setup() {
	// put your setup code here, to run once:
	Serial.begin(115200);
	
	// set I/O direction
	pinMode(LEDGPS, OUTPUT);

	
	delay(5000);
	Serial.println("Limon93Studio Traccar client."); 
	// GPS power on
	LGPS.powerOn();
	Serial.println("GPS powered ON.");
	// set default value for GPS (needed for default led status)
	MyGPSPos.fix = Error;

  LTask.remoteCall(createThread1, NULL);
	
	// GSM setup
	while(!LSMS.ready()){
		delay(1000);
		Serial.println("Insert SIM");
	}
  	Serial.println("SIM ready.");	
	pinMode(LEDGPS, OUTPUT);
	
	Serial.println("Setup done.");	
    while (!LGPRS.attachGPRS(APN,APNUSER,APNPASS))
     {
        delay(500);
     }
     GetGPSPos();
     delay(30000);
}

void loop() {
    GetGPSPos();
	  if (MyFlag.fix3D){
    Serial.println("Periodic update."); 
    char path[200]; 
    sprintf(path,"/?id=%s&lat=%2.6f&lon=%3.6f&altitude=%d&speed=%d",USERID,MyGPSPos.latitude, MyGPSPos.longitude,MyGPSPos.alt,MyGPSPos.spd);
    if (client.connect(server, port))
    {
    Serial.println("Connected to network and server.");
    // Make a HTTP request:
    client.print("GET ");
    client.print(path);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();
    Serial.println("Done.\n");
      }
    else
     {
    // if you didn't get a connection to the server:
    Serial.println("Connection failed.");
     }
     delay(UPDATE_FREQ*1000);
} 
else {Serial.println("Waiting for 3D GPS fix..."); delay(1000);}
}
boolean createThread1(void* userdata) {
	// The priority can be 1 - 255 and default priority are 0
	// the arduino priority are 245
	vm_thread_create(thread_ledgps, NULL, 255);
    return true;
}

//----------------------------------------------------------------------
//!\brief           THREAD LED GPS
//---------------------------------------------------------------------- 
VMINT32 thread_ledgps(VM_THREAD_HANDLE thread_handle, void* user_data){
    for (;;){
		switch(MyGPSPos.fix){
			case Invalid:
				// blink led as pulse
				digitalWrite(LEDGPS, HIGH);
				delay(500);
				digitalWrite(LEDGPS, LOW);
				delay(500);
				break;
			case GPS:
			case DGPS:
			case PPS:
			case RTK:
			case FloatRTK:
			case DR:
			case Manual:
			case Simulation:
				// blink led as slow pulse
				digitalWrite(LEDGPS, HIGH);
				delay(150);
				digitalWrite(LEDGPS, LOW);
				delay(2850);
				break;
			case Error:
				// Fast blinking led
				digitalWrite(LEDGPS, HIGH);
				delay(100);
				digitalWrite(LEDGPS, LOW);
				delay(100);
				break;
		}
	}
    return 0;
}

