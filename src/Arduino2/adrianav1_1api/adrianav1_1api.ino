
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Dns.h>
#include <sha1.h>
#include <Adafruit_Thermal.h>
#include <SoftwareSerial.h>


#define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr



const char PROGMEM
  
  consumer_key[]  = "bfiH2CjAc1AumGT2hyow",
  access_token[]  = "449142514-jLLRqfsnrUeIqFDe2GRPh32jQjrmLiPC5huUZj9m",
  signingKey[]    = "R2AwoSXuJ1lF3gGNh7PUly1nfr5Ahf1MAL1asFsdsGI"     
    "&"             "PB6dBvF9gibWZH475Me7sKYvnJqllfuHeHLHXNnSM", 
  
  queryString[]   = "amor",


  endpoint[]      = "/1.1/search/tweets.json",
  agent[]         = "Gutenbird v1.0";
const char
  host[]          = "api.twitter.com";
const int
  led_pin         = 3,           
  
  printer_RX_Pin  = 5,           
  printer_TX_Pin  = 6,           
  printer_Ground  = 7;           
const unsigned long
  pollingInterval = 60L * 1000L, 
  searchesPerDay  = 86400000L / pollingInterval,
  connectTimeout  = 15L * 1000L, 
  responseTimeout = 15L * 1000L; 
Adafruit_Thermal
  printer(printer_RX_Pin, printer_TX_Pin);
byte
  maxTweets = 1, 
  sleepPos  = 0, 
  resultsDepth,  
  
  mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x76, 0x09 };
IPAddress
  ip(192,168,0,118); 
char
  lastId[21],    
  timeStamp[32], 
  fromUser[16],  
  msgText[141],  
  name[12],     
  value[141];    
int
  searchCount = 0;
unsigned long
  currentTime = 0L;
EthernetClient
  client;
PROGMEM byte
 const sleepTab[] = { 
      0,   0,   0,   0,   0,   0,   0,   0,   0,   1,
      1,   1,   2,   3,   4,   5,   6,   8,  10,  13,
     15,  19,  22,  26,  31,  36,  41,  47,  54,  61,
     68,  76,  84,  92, 101, 110, 120, 129, 139, 148,
    158, 167, 177, 186, 194, 203, 211, 218, 225, 232,
    237, 242, 246, 250, 252, 254, 255 };

// --------------------------------------------------------------------------

void setup() {

  
  TCCR1A  = _BV(WGM11); 
  TCCR1B  = _BV(WGM13) | _BV(WGM12) | _BV(CS11) | _BV(CS10);
  ICR1    = 8333;       
  TIMSK1 |= _BV(TOIE1); 
  sei();                

  randomSeed(analogRead(0));
  Serial.begin(57600);
  pinMode(printer_Ground, OUTPUT);
  digitalWrite(printer_Ground, LOW); 
  printer.begin();
  printer.sleep();

  
  Serial.print(F("Initializing Ethernet..."));
  if(Ethernet.begin(mac)) {
    Serial.print(F("OK\r\n"));
  } else {
    Serial.print(F("\r\nno DHCP response, using static IP address."));
    Ethernet.begin(mac, ip);
  }

  
  for(uint8_t i=0; (i<5) && !(currentTime = getTime()); delay(15000L), i++);

  
  strcpy_P(lastId, PSTR("1"));
  timeStamp[0] = fromUser[0] = msgText[0] = name[0] = value[0] = 0;
}

// Search occurs in loop. ---------------------------------------------------

void loop() {
  uint8_t                  *in, out, i;
  char                      nonce[9],       // 8 random digits + NUL
                            searchTime[11], // 32-bit int + NUL
                            b64[29];
  unsigned long             startTime, t;
  static const char PROGMEM b64chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  startTime = millis();

 
  TIMSK1 &= ~_BV(TOIE1);
  analogWrite(led_pin, 255);


  sprintf(nonce, "%04x%04x", random() ^ currentTime, startTime ^ currentTime);
  sprintf(searchTime, "%ld", currentTime);

 
  Serial.print(F("  Current time: "));
  Serial.println(currentTime);
  Serial.print(F("  Last ID: "));
  Serial.println(lastId);

  Sha1.initHmac_P((uint8_t *)signingKey, sizeof(signingKey) - 1);


  Sha1.print(F("GET&http%3A%2F%2F"));
  Sha1.print(host);
  urlEncode(Sha1, endpoint, true, false);
  Sha1.print(F("&count%3D"));
  Sha1.print(maxTweets);
  Sha1.print(F("%26include_entities%3D0%26oauth_consumer_key%3D"));
  Sha1.print(F2(consumer_key));
  Sha1.print(F("%26oauth_nonce%3D"));
  Sha1.print(nonce);
  Sha1.print(F("%26oauth_signature_method%3DHMAC-SHA1%26oauth_timestamp%3D"));
  Sha1.print(searchTime);
  Sha1.print(F("%26oauth_token%3D"));
  Sha1.print(F2(access_token));
  Sha1.print(F("%26oauth_version%3D1.0%26q%3D"));
  urlEncode(Sha1, queryString, true, true);
  Sha1.print(F("%26since_id%3D"));
  Sha1.print(lastId);

  for(in = Sha1.resultHmac(), out=0; ; in += 3) { // octets to sextets
    b64[out++] =   in[0] >> 2;
    b64[out++] = ((in[0] & 0x03) << 4) | (in[1] >> 4);
    if(out >= 26) break;
    b64[out++] = ((in[1] & 0x0f) << 2) | (in[2] >> 6);
    b64[out++] =   in[2] & 0x3f;
  }
  b64[out] = (in[1] & 0x0f) << 2;
  
  for(i=0; i<=out; i++) b64[i] = pgm_read_byte(&b64chars[b64[i]]);
  b64[i++] = '=';
  b64[i++] = 0;

  Serial.print(F("Connecting to server..."));
  t = millis();
  while((client.connect(host, 80) == false) &&
    ((millis() - t) < connectTimeout));

  if(client.connected()) { // Success!
    Serial.print(F("OK\r\nIssuing HTTP request..."));

  
    client.print(F("GET "));
    client.print(F2(endpoint));
    client.print(F("?count="));
    client.print(maxTweets);
    client.print(F("&since_id="));
    client.print(lastId);
    client.print(F("&include_entities=0&q="));
    urlEncode(client, queryString, true, false);
    client.print(F(" HTTP/1.1\r\nHost: "));
    client.print(host);
    client.print(F("\r\nUser-Agent: "));
    client.print(F2(agent));
    client.print(F("\r\nConnection: close\r\n"
                       "Content-Type: application/x-www-form-urlencoded;charset=UTF-8\r\n"
                       "Authorization: Oauth oauth_consumer_key=\""));
    client.print(F2(consumer_key));
    client.print(F("\", oauth_nonce=\""));
    client.print(nonce);
    client.print(F("\", oauth_signature=\""));
    urlEncode(client, b64, false, false);
    client.print(F("\", oauth_signature_method=\"HMAC-SHA1\", oauth_timestamp=\""));
    client.print(searchTime);
    client.print(F("\", oauth_token=\""));
    client.print(F2(access_token));
    client.print(F("\", oauth_version=\"1.0\"\r\n\r\n"));

    Serial.print(F("OK\r\nAwaiting results (if any)..."));
    t = millis();
    while((!client.available()) && ((millis() - t) < responseTimeout));
    if(client.available()) { // Response received?
      
      if(client.find("\r\n\r\n")) { 
        Serial.print(F("OK\r\nProcessing results...\r\n"));
        resultsDepth = 0;
        jsonParse(0, 0);
      } else Serial.print(F("response not recognized.\r\n"));
    } else   Serial.print(F("connection timed out.\r\n"));
    Serial.print(F("Done.\r\n"));

    client.stop();
  } else { 
    Serial.print(F("failed\r\n"));
  }

  
  currentTime += pollingInterval / 1000L;
  if((++searchCount >= searchesPerDay) && (t = getTime())) {
    currentTime = t;
    searchCount = 0;
  }

  while((millis() - startTime) < 4000UL);

  if((millis() - startTime) < pollingInterval) {
    Serial.print(F("Pausing..."));
    sleepPos = sizeof(sleepTab); 
    TIMSK1 |= _BV(TOIE1); 
    while((millis() - startTime) < pollingInterval);
    Serial.print(F("done\r\n"));
  }
}



boolean jsonParse(int depth, byte endChar) {
  int     c, i;
  boolean readName = true;
  for(;;) {
    while(isspace(c = timedRead())); 
    if(c < 0)        return false;   
    if(c == endChar) return true;    

    if(c == '{') { 
      if(!jsonParse(depth + 1, '}')) return false;
      if(!depth)                     return true; 
      if(depth == resultsDepth) { 
        
        printer.wake();
        printer.inverseOn();
        printer.write(' ');
        printer.print(fromUser);
        for(i=strlen(fromUser); i<31; i++) printer.write(' ');
        printer.inverseOff();
        printer.underlineOn();
        printer.print(timeStamp);
        for(i=strlen(timeStamp); i<32; i++) printer.write(' ');
        printer.underlineOff();
        printer.println(msgText);
        printer.feed(3);
        printer.sleep();

        
        Serial.print(F("  User: "));
        Serial.println(fromUser);
        Serial.print(F("  Text: "));
        Serial.println(msgText);
        Serial.print(F("  Time: "));
        Serial.println(timeStamp);

        
        timeStamp[0] = fromUser[0] = msgText[0] = 0;

        maxTweets = 1; // si se desea aumentar los twitts por minuto
      }
    } else if(c == '[') { 
      if((!resultsDepth) && (!strcasecmp(name, "statuses")))
        resultsDepth = depth + 1;
      if(!jsonParse(depth + 1,']')) return false;
    } else if((c == '"') || (c == '\'')) { // String follows
      if(readName) { // Name-reading mode
        if(!readString(name, sizeof(name)-1, c)) return false;
      } else { // Value-reading mode
        if(!readString(value, sizeof(value)-1, c)) return false;
        // Process name and value strings:
        if       (!strcasecmp(name, "max_id_str")) {
          strncpy(lastId, value, sizeof(lastId)-1);
        } else if(!strcasecmp(name, "created_at")) {
          // Use created_at value for tweet, not user
          if(depth == (resultsDepth + 1)) {
            strncpy(timeStamp, value, sizeof(timeStamp)-1);
          }
        } else if(!strcasecmp(name, "screen_name")) {
          strncpy(fromUser, value, sizeof(fromUser)-1);
        } else if(!strcasecmp(name, "text")) {
          strncpy(msgText, value, sizeof(msgText)-1);
        }
      }
    } else if(c == ':') { 
      readName = false; 
      value[0] = 0;     
    } else if(c == ',') {
     
      readName = true; 
      name[0]  = 0;    
    } 
      
  }
}

boolean readString(char *dest, int maxLen, char quote) {
  int c, len = 0;

  while((c = timedRead()) != quote) { 
    if(c == '\\') {    
      c = timedRead(); 
     
      if     (c == 'b') c = '\b'; 
      else if(c == 'f') c = '\f'; 
      else if(c == 'n') c = '\n'; 
      else if(c == 'r') c = '\r'; 
      else if(c == 't') c = '\t'; 
      else if(c == 'u') c = unidecode(4);
      else if(c == 'U') c = unidecode(8);
      
    } 

    if(c < 0) return false; 

    if(len < maxLen) dest[len++] = c;
  }

  dest[len] = 0;
  return true; 
}

int unidecode(byte len) {
  int c, v, result = 0;
  while(len--) {
    if((c = timedRead()) < 0) return -1; // Stream timeout
    if     ((c >= '0') && (c <= '9')) v =      c - '0';
    else if((c >= 'A') && (c <= 'F')) v = 10 + c - 'A';
    else if((c >= 'a') && (c <= 'f')) v = 10 + c - 'a';
    else return '-'; 
    result = (result << 4) | v;
  }



  return '-';
}


int timedRead(void) {
  unsigned long start = millis();
  while((!client.available()) && ((millis() - start) < 5000L));
  return client.read();  
}


void urlEncode(
  Print      &p,       
  const char *src,     
  boolean     progmem, 
  boolean     x2)      
{
  static const char PROGMEM hexChar[] = "0123456789ABCDEF";
  uint8_t c;

  while((c = (progmem ? pgm_read_byte(src) : *src))) {
    if(((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) ||
       ((c >= '0') && (c <= '9')) || strchr_P(PSTR("-_.~"), c)) {
      p.write(c);
    } else {
      if(x2) p.print("%25");
      else   p.write('%');
      p.write(pgm_read_byte(&hexChar[c >> 4]));
      p.write(pgm_read_byte(&hexChar[c & 15]));
    }
    src++;
  }
}

unsigned long getTime(void) {
  EthernetUDP   udp;
  DNSClient     dns;
  IPAddress     addr;
  byte          buf[48];
  unsigned long t = 0L;

  Serial.print(F("Polling time server..."));

  udp.begin(8888);
  dns.begin(Ethernet.dnsServerIP());

  if(dns.getHostByName("pool.ntp.org", addr)) {
    static const char PROGMEM
      timeReqA[] = { 227,  0,  6, 236 },
      timeReqB[] = {  49, 78, 49,  52 };

   
    memset(buf, 0, sizeof(buf));
    memcpy_P( buf    , timeReqA, sizeof(timeReqA));
    memcpy_P(&buf[12], timeReqB, sizeof(timeReqB));

    udp.beginPacket(addr, 123);
    udp.write(buf, sizeof(buf));
    udp.endPacket();

    delay(1000); 
    if(udp.parsePacket()) {
      
      udp.read(buf, sizeof(buf));
      t = (((unsigned long)buf[40] << 24) |
           ((unsigned long)buf[41] << 16) |
           ((unsigned long)buf[42] <<  8) |
            (unsigned long)buf[43]) - 2208988800UL;
      Serial.print(F("OK\r\n"));
    }
  }
  udp.stop();
  if(!t) Serial.print(F("error\r\n"));

  return t;
}


ISR(TIMER1_OVF_vect, ISR_NOBLOCK) {
  
  analogWrite(led_pin, pgm_read_byte(&sleepTab[
    (sleepPos >= sizeof(sleepTab)) ?
    (sizeof(sleepTab) * 2 - 1 - sleepPos) : sleepPos]));
  if(++sleepPos >= (sizeof(sleepTab) * 2)) sleepPos = 0; 
  TIFR1 |= TOV1; 
}

