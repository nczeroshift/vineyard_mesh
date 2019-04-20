
#include <LowPower.h>
#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include <EEPROM.h>

#define NRF24_Channel     0x70
#define NRF24_WakeUp_Pin  2
#define NRF24_NodeId      1

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

RF24 radio(9, 10);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

uint32_t displayTimer = 0;
long lastMillis = -1;
uint8_t firstSleep = 0;

void nrf24_handle_other_packets(RF24NetworkHeader * header){
  
  uint8_t channel = 0;
  network.read(*header,&channel,sizeof(uint8_t)); 
  
  char type = header->type;
  
  float value = 0.0;
  if(type == 'V')
    value = 1.0;
  else if(type == 'T')
    value = 24.0;
  else if(type == 'H')
    value = 60.0;
  else if(type == 'P')
    value = 10000.0;

  struct packet_float_t txPk;
  txPk.type = 0;
  txPk.value = value;

  delay(1); // Magic sleep time, just works!
  
  Serial.println(F("Returning Packet ..."));
  if(!mesh.write(&txPk, type, sizeof(txPk))) 
  {
      if(!mesh.checkConnection())
      {
        Serial.println(F("Renewing Address"));
        mesh.renewAddress();
      } 
      else{
        Serial.println(F("Send fail, Test OK"));
      }
  }
  else{
    Serial.println(F("Send OK!")); 
  }
}

void nrf24_handle_io_packet(struct packet_io_t * pk){
    
}

void nrf24_handle_time_packet(struct packet_time_t * pk){
    
}

void setup() {
  Serial.begin(9600);
  mesh.setNodeID(NRF24_NodeId);
  
  Serial.print(F("Connecting to the mesh..."));

  
    
  //mesh.begin();
  mesh.begin(NRF24_Channel, 
  RF24_1MBPS,//RF24_250KBPS,
  MESH_RENEWAL_TIMEOUT);

  radio.maskIRQ(1,1,0);
  
  Serial.println(F("connected!"));

  pinMode(NRF24_WakeUp_Pin, INPUT); 
}

void wakeUp()
{
  lastMillis = -1;
  Serial.println(F("Wakeup!"));
}

void loop() {

  if(lastMillis == -1){
    firstSleep= 1;
    lastMillis = millis();    
  }
  else if(millis() - lastMillis > 10000){
    
    //radio.powerDown();
    //radio.stopListening();
    //radio.setPALevel(RF24_PA_MIN);
    attachInterrupt(0, wakeUp, LOW);
    if(firstSleep == 1){
      Serial.println(F("Powering down!"));
      delay(100);
    }
    firstSleep = 0;
    
   /* LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, 
                  TIMER1_OFF, TIMER0_OFF, SPI_OFF,
                  USART0_OFF, TWI_OFF);*/
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    detachInterrupt(0);
    //radio.setPALevel(RF24_PA_MAX);
    //radio.powerUp();
    //radio.startListening();
    //lastMillis = -1;
    //Serial.begin(9600);
    //Serial.println(F("Power up!"));
  }
  
  mesh.update();

  
  if(network.available())
  {
    lastMillis = -1; // Reset sleep counter.
    
    RF24NetworkHeader header;
    network.peek(header);

    Serial.print(F("New packet, type: "));
    Serial.println((char)header.type);    
    
    switch(header.type)
    {
      // I/O Port
      case 'I':
      { 
        struct packet_io_t pk;
        network.read(header, &pk, sizeof(packet_io_t)); 
        nrf24_handle_io_packet(&pk);
        break;
      }

      // Time
      case 'E':
      { 
        struct packet_time_t pk;
        network.read(header, &pk, sizeof(packet_time_t)); 
        nrf24_handle_time_packet(&pk);
        break;
      }
      
      default: 
        nrf24_handle_other_packets(&header);
        break;
    }
  }
}
