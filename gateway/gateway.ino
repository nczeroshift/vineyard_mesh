  
#include <EtherCard.h>
#include "RF24Network.h"
#include "RF24.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include <EEPROM.h>

enum StatusType{
  STATUS_200,
  STATUS_401,
};

enum MimeType{
  MIME_HTML,
  MIME_TEXT,
};

void http_serve_header(enum StatusType statusType, enum MimeType mimeType);
void http_serve_footer(void);

int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

struct packet_time_t {
  uint8_t type;
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
};

struct packet_io_t {
  uint8_t type;
  uint8_t channel;
  uint8_t value; 
};

struct packet_int_t {
  uint8_t type;
  int value;  
};

struct packet_float_t {
  uint8_t type;
  float value;  
};


static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
//static byte myip[] = { 192,168,137,1 };

byte Ethernet::buffer[400];
BufferFiller bfill;

/***** Configure the chosen CE,CS pins *****/

#define NRF24_Channel   0x70
#define NRF24_MAX_RX_SIZE 32

RF24 radio(7,8);
RF24Network network(radio);
RF24Mesh mesh(radio,network);

uint16_t nrf24_rx_address = 0;
uint8_t nrf24_rx_type     = 0;
uint8_t nrf24_rx_size     = 0;
byte nrf24_rx_packet[NRF24_MAX_RX_SIZE];

static void ethernet_setup(void){
  Serial.print(F("Setting up ethernet .."));
  
  if (ether.begin(sizeof Ethernet::buffer, mymac, 9) == 0){
    Serial.println(F("error"));
    return;
  }
  else 
    Serial.println(F("ok"));
  
  Serial.print(F("Setting up DHCP .."));
  //ether.staticSetup(myip);
  if (!ether.dhcpSetup()){
    Serial.println(F("error"));
    return;
  }
  else 
    Serial.println(F("ok"));
   
  ether.printIp("IP: ", ether.myip);
  ether.printIp("Netmask: ", ether.netmask);
  ether.printIp("GW IP: ", ether.gwip);
  ether.printIp("DNS IP: ", ether.dnsip);
}

/**
 * Setup NRF24 mesh network.
 */
static void nrf24_setup(void){
  Serial.print(F("Setup mesh.."));
  mesh.setNodeID(0); // Set 0 for master node

  //mesh.begin();
  mesh.begin(NRF24_Channel,
  RF24_1MBPS,//RF24_250KBPS,
  MESH_RENEWAL_TIMEOUT);
  
  //Serial.print(F("NodeId: "));
  //Serial.println(mesh.getNodeID());
  
  Serial.println(F("ok"));
}

/**
 * Send a packet to a registered node in the NRF24 network.
 */
static bool nrf24_send_packet(uint16_t nodeId, uint8_t type, uint8_t * data, uint8_t len){
  RF24NetworkHeader header(nodeId, type);
  return network.write(header, data, len) == 1;
}

const char http_status_401 [] PROGMEM = 
  "HTTP/1.0 401 Unauthorized\r\n";

const char http_status_200 [] PROGMEM = 
  "HTTP/1.0 200 OK\r\n";

const char http_message_401 [] PROGMEM =
  "<h1>401 Unauthorized</h1>"; 
  
const char http_headers_cors [] PROGMEM = 
  "Access-Control-Allow-Headers:X-Requested-With,Content-Type,Authorization\r\n"
  "Access-Control-Allow-Methods:GET,POST\r\n"
  "Access-Control-Allow-Origin:*\r\n";

const char http_headers_no_cache [] PROGMEM = 
  "Pragma: no-cache\r\n";
 
const char http_headers_type_html [] PROGMEM = 
  "Content-Type: text/html\r\n";

const char http_headers_type_plain [] PROGMEM = 
  "Content-Type: text/plain\r\n";

const char http_line_break [] PROGMEM = 
  "\r\n";

const char http_homepage[] PROGMEM = 
    "<title>NRF24 Ethernet Adapter</title>"
    "<h1>About</h1>"
    "<p>NRF24L01+ Mesh Network to Ethernet Adapter</p>"
    "<h1>Version</h1>"
    "<p>1.0 under MIT Library (2019/03/16)</p>"
    "<a href=\"https:\\github.com/nczeroshift/nrf24_ethernet\">github</a>";

/**
 * Send HTTP Error(401) response.
 */
static void http_send_error(){
  http_serve_header(STATUS_401,MIME_HTML);
  http_serve_footer();
}


/**
 * Send HTTP OK(200) response.
 */
void http_respond_ok(void){
  http_serve_header(STATUS_200, MIME_HTML);
  http_serve_footer();
}


/**
 * Send home page Response
 */
void http_send_homepage() {
  http_serve_header(STATUS_200, MIME_HTML);
  
  memcpy_P(ether.tcpOffset(), http_homepage, sizeof http_homepage);
  ether.httpServerReply_with_flags(sizeof http_homepage-1,TCP_FLAGS_ACK_V);

  http_serve_footer();
}

/**
 * Send list of NRF24 nodes formated in a TSV way, first line is the count of nodes.
 */
void http_serve_header(enum StatusType statusType, enum MimeType mimeType){
  ether.httpServerReplyAck();

  if(statusType == STATUS_200){
    memcpy_P(ether.tcpOffset(), http_status_200, sizeof http_status_200);
    ether.httpServerReply_with_flags(sizeof http_status_200-1,TCP_FLAGS_ACK_V);
  }if(statusType == STATUS_401){
    memcpy_P(ether.tcpOffset(), http_status_401, sizeof http_status_401);
    ether.httpServerReply_with_flags(sizeof http_status_401-1,TCP_FLAGS_ACK_V);
  }
  
  memcpy_P(ether.tcpOffset(), http_headers_cors, sizeof http_headers_cors);
  ether.httpServerReply_with_flags(sizeof http_headers_cors-1,TCP_FLAGS_ACK_V);

  if(mimeType == MIME_TEXT){
    memcpy_P(ether.tcpOffset(), http_headers_type_plain, sizeof http_headers_type_plain);
    ether.httpServerReply_with_flags(sizeof http_headers_type_plain-1,TCP_FLAGS_ACK_V);
  } 
  else if(mimeType == MIME_HTML){
    memcpy_P(ether.tcpOffset(), http_headers_type_html, sizeof http_headers_type_html);
    ether.httpServerReply_with_flags(sizeof http_headers_type_html-1,TCP_FLAGS_ACK_V);
  }
  
  memcpy_P(ether.tcpOffset(), http_headers_no_cache, sizeof http_headers_no_cache);
  ether.httpServerReply_with_flags(sizeof http_headers_no_cache-1,TCP_FLAGS_ACK_V);

  memcpy_P(ether.tcpOffset(), http_line_break, sizeof http_line_break);
  ether.httpServerReply_with_flags(sizeof http_line_break-1,TCP_FLAGS_ACK_V);
}

void http_serve_footer(void){
  const char eof_line[] = "\n";
  memcpy(ether.tcpOffset(), eof_line, sizeof eof_line); 
  ether.httpServerReply_with_flags(sizeof eof_line-1,TCP_FLAGS_ACK_V|TCP_FLAGS_FIN_V);
}

static void http_send_ram(void){
  http_serve_header(STATUS_200,MIME_TEXT);
  String line = String(freeRam(), DEC) + "\n";
  memcpy(ether.tcpOffset(), line.c_str(), line.length()); 
  ether.httpServerReply_with_flags(line.length(),TCP_FLAGS_ACK_V);
  http_serve_footer();
}


static void http_send_rx(void){
  http_serve_header(STATUS_200,MIME_TEXT);

  String line = String(nrf24_rx_size, DEC) + "\n";
  memcpy(ether.tcpOffset(), line.c_str(), line.length()); 
  ether.httpServerReply_with_flags(line.length(),TCP_FLAGS_ACK_V);

  if(nrf24_rx_size > 0){
    line = String(nrf24_rx_type, DEC) + "\n";
    memcpy(ether.tcpOffset(), line.c_str(), line.length()); 
    ether.httpServerReply_with_flags(line.length(),TCP_FLAGS_ACK_V);
    
    line = String(nrf24_rx_address, DEC) + "\n";
    memcpy(ether.tcpOffset(), line.c_str(), line.length()); 
    ether.httpServerReply_with_flags(line.length(),TCP_FLAGS_ACK_V);

    for(uint8_t i = 0; i < nrf24_rx_size; i ++){
      line = String(nrf24_rx_packet[i], HEX);
      if(line.length() == 1)
        line = String("0")+line;
      memcpy(ether.tcpOffset(), line.c_str(), line.length()); 
      ether.httpServerReply_with_flags(line.length(),TCP_FLAGS_ACK_V);
    }
  }

  nrf24_rx_size = 0;
  nrf24_rx_address = 0;
  nrf24_rx_type = 0;
  
  http_serve_footer();
}


static void http_send_node_list(void){
  http_serve_header(STATUS_200,MIME_TEXT);
 
  String line = "count," + String(mesh.addrListTop, DEC) + "\n";
  memcpy(ether.tcpOffset(), line.c_str(), line.length()); 
  ether.httpServerReply_with_flags(line.length(),TCP_FLAGS_ACK_V);
  
  for(uint8_t i = 0; i < mesh.addrListTop; i++){
    line = String(mesh.addrList[i].nodeID,DEC) + ","+ String(mesh.addrList[i].address,DEC)+ "\n";
    memcpy(ether.tcpOffset(), line.c_str(), line.length()); 
    ether.httpServerReply_with_flags(line.length(),TCP_FLAGS_ACK_V);
  }
  
  http_serve_footer();
}


/**
 * Handle POST network to send a packet to NRF24 network.
 */
bool ethernet_handle_post_send(char * data, word len){
  uint8_t state = 0;
  char param[4] = "";
  uint8_t param_i = 0;
  uint8_t value_i = 0;

  char * node_id = NULL;
  char * packet_type = NULL;
  char * packet_data = NULL;
  char * reply_address = NULL;
  
  for(int i = 0;i<len;i++){
    bool eof_flag = false;
    
    if(data[i] == '\r' && data[i+1] == '\r'){
      eof_flag = true;
    }
    
    if(state == 1){
      if(data[i] == '='){
        state = 2;
        value_i = i+1;
      }
      else{
        param[param_i++] = data[i];
        param[param_i] = '\0'; 
      }
    }
    else if(state == 2){
      if(data[i] == '&' || eof_flag){
        state = 1;
        param_i = 0;
        data[i] = '\0';
        
        if(strncmp(param,"id",2) == 0){
          // node id
          node_id = &data[value_i];
        } 
        else if(strncmp(param,"tp",2) == 0){
          // packet type
          packet_type = &data[value_i];
        }
        else if(strncmp(param,"dt",2) == 0){
          // packet data
          packet_data = &data[value_i];
        }
        else if(strncmp(param,"ip",2) == 0){
          // ip adress
          reply_address = &data[value_i];
        }
      }
    }
  
    if(eof_flag)
      break;
      
    if(data[i] == '\r' && data[i+1] == '\n'){
      if(param_i == 0)
        state = 1; 
      param_i = 0;
      i++;
    }
    else if(state == 0 && data[i] != '\n')
      param_i ++;                   
  }

  if(node_id && packet_type && packet_data)
  { 
    const uint16_t id = String(node_id).toInt();
    const char type = packet_type[0]; 
    const char data_len = strlen(packet_data);
    uint8_t j_len = 0;
    
    Serial.print(F("Sending packet("));
    Serial.print(id);Serial.print(",");Serial.print(type);Serial.print(")... ");

    for(uint8_t i = 0; i < data_len; i += 2){
      char tmp[3] = {0,0,0};
      tmp[0] = packet_data[i];
      tmp[1] = packet_data[i+1];
      // reuse packet_data buffer to write the bytes.
      packet_data[j_len++] = strtol(tmp, NULL, 16); 
    }

    delay(1); // magic sleep, just works!
    
    if(nrf24_send_packet(id, packet_type[0], packet_data, j_len)){
      Serial.println(F("success!"));
      return true;
    }
   
    Serial.println(F("failure!"));
  }
  
  return false;
}

/**
 * Handle ethernet packets.
 * @returns true if it needs to send 
 */
void ethernet_handle()
{
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);
  if (pos)
  {
    bfill = ether.tcpOffset();
    char *data = (char *) Ethernet::buffer + pos;

    if (strncmp("GET /", data, 5) == 0)
    {
      data += 5; len -= 5;
      
      if (data[0] == ' '){
        http_send_homepage();
      }
      else if (strncmp("?list", data, 5) == 0)     
        http_send_node_list();
      else if (strncmp("?ram", data, 4) == 0)     
        http_send_ram();
      else if (strncmp("?read", data, 5) == 0){  
        http_send_rx();
      }
      else
        http_send_error();
    }
    else if (strncmp("POST /", data, 6) == 0)
    {
      data += 6; len -= 6;
      
      if (strncmp("?send", data, 5) == 0) {  
        if(ethernet_handle_post_send(data, len))
          http_respond_ok();
        else
          http_send_error();
      }
      else
        http_send_error();
    }
    else
      http_send_error();
  }
  
  return false;
}




void nrf24_update(){
  mesh.update();
  mesh.DHCP();
  
  if(network.available())
  {
    RF24NetworkHeader header;
    network.peek(header);

    Serial.print("New packet: ");
    Serial.println((char)header.type);
    
    switch(header.type)
    {
      case 'V': // Voltage
      case 'T': // Temperature
      case 'H': // Humidity
      case 'P': // Pressure
      {
          struct packet_float_t pk;
          network.read(header, &pk, sizeof(packet_float_t)); 

          memcpy(nrf24_rx_packet,&pk, (nrf24_rx_size = sizeof(packet_float_t)));
          nrf24_rx_type = header.type;
          nrf24_rx_address = header.from_node;        
          break; 
      }
      
      // I/O Port
      case 'I':
      { 
        struct packet_io_t pk;
        network.read(header, &pk, sizeof(packet_io_t)); 

        memcpy(nrf24_rx_packet,&pk, (nrf24_rx_size = sizeof(packet_float_t)));
        nrf24_rx_type = header.type;
        nrf24_rx_address = header.from_node;
        break;
      }

      // Time
      case 'E':
      { 
        struct packet_time_t pk;
        network.read(header, &pk, sizeof(packet_time_t)); 

        memcpy(nrf24_rx_packet,&pk, (nrf24_rx_size = sizeof(packet_float_t)));
        nrf24_rx_type = header.type;
        nrf24_rx_address = header.from_node;
        break;
      }
      
      default: 
        network.read(header,0,0); 
        Serial.print(F("  Type: "));
        Serial.println(header.type);
        break;
    }

  }
}


void setup() {
  Serial.begin(9600);
  ethernet_setup();
  nrf24_setup();
  //pinMode(5, INPUT); 
}

long displayTimer = 0;

void loop() 
{    
  ethernet_handle();
  nrf24_update();
  /* 
  if(millis() - displayTimer > 5000){
    displayTimer = millis();
    Serial.println(" ");
    Serial.println(F("********Assigned Addresses********"));
     for(int i=0; i<mesh.addrListTop; i++){
       Serial.print("NodeID: ");
       Serial.print(mesh.addrList[i].nodeID);
       Serial.print(" RF24Network Address: 0");
       Serial.println(mesh.addrList[i].address,OCT);
     }
    Serial.println(F("**********************************"));
    Serial.print(F("Free mem"));
    Serial.println(freeRam());
  }
  */
}
  
