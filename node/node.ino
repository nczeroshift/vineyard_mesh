
/**
 * Vineyard Mesh Node
 * https://github.com/nczeroshift/vineyard_mesh
 * MIT License
 * Luís F. Loureiro
 * 2019/04/21
 */ 
 
 
#include <LowPower.h>
#include <RF24.h>
#include <RF24Network.h>
#include <RF24Mesh.h>
#include <SPI.h>
#include <EEPROM.h>


/******************************************************************************
 * Setup constants
 */ 

 
#define SERIAL_BAUDRATE             9600        // Baudrate used

#define NRF24_NodeId                1           // Node Unique ID
#define NRF24_Channel               0x70        // Use scanner sketch to find a free channel
#define NRF24_Bitrate               RF24_1MBPS  // Network operational bitrate

#define ARDUINO_SLEEP                         // Enable to power down arduino and wake on received packet
#define ARDUINO_SLEEP_DELAY         10000       // Milliseconds before arduino enter "sleep"
#define ARDUINO_SLEEP_CHECK_WAN     4           // After X number of sleeps, it checks connectivity.
#define ARDUINO_SLEEP_TIME          SLEEP_8S    // Lowpower library setting.

// TODO
#define NRF24_RADIO_MUTE                        // Enable to power down nrf24 radio     
 
#define NRF24_CE_Pin                9           // arduino pro mini (9,10) 
#define NRF24_CS_Pin                10          // arduino pro mini (9,10) 
#define NRF24_WakeUp_Pin            2           // arduino pro mini (Int0 pin)                         


/******************************************************************************
 * Supported packet types
 */

 
// Time packet
struct packet_time_t {
    uint8_t type;
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

// IO operations packet
struct packet_io_t {
    uint8_t channel;
    uint8_t value; 
};

struct packet_int_t {
    uint8_t channel;
    int value;  
};

struct packet_float_t {
    uint8_t channel;
    float value;  
};


/******************************************************************************
 * Sketch global variables
 */

 
RF24 radio(NRF24_CE_Pin, NRF24_CS_Pin);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

long  lastMillis = -1;
int   sleepCount = 0;


/******************************************************************************
 * Prototypes
 */
 

bool nrf24_send_packet(char type, void * pk, uint8_t pk_size);
bool nrf24_handle_packet(RF24NetworkHeader * header);
void nrf24_rx_wake_up();
void sleep_managment();


/******************************************************************************
 * Arduino main functions
 */
 
void setup() 
{
    Serial.begin(SERIAL_BAUDRATE);  
    mesh.setNodeID(NRF24_NodeId);

    Serial.print(F("Connecting to the mesh..."));
    mesh.begin(NRF24_Channel, NRF24_Bitrate, MESH_RENEWAL_TIMEOUT);
    Serial.println(F("connected!"));

    radio.maskIRQ(1,1,0); // Enable RX interrupt

    pinMode(NRF24_WakeUp_Pin, INPUT); 
}

void loop() 
{
    sleep_managment();
    mesh.update();
  
    if(network.available()){
        lastMillis = -1; // Reset sleep counter.

        RF24NetworkHeader header;
        network.peek(header);

        Serial.print(F("New packet, type: "));
        Serial.println((char)header.type);    

        if(!nrf24_handle_packet(&header))
            network.read(header,0,0); // consume packet if not previously.
    }
}

/******************************************************************************
 * Implementations
 */
 
bool nrf24_handle_packet(RF24NetworkHeader * header)
{  
    char type = header->type;

    // This are examples ways to deal with received packets:
    if(type == 'V' || type == 'T' || type == 'H'){
        // Float packet to measure temperature, humidity or voltage.

        // Received packet may contain a channel id.
        uint8_t channel = 0;
        network.read(*header,&channel,sizeof(uint8_t)); 

        struct packet_float_t pk;
        pk.channel = channel;
        pk.value = 1.0;

        Serial.println(F("Returning Packet ..."));
        nrf24_send_packet(type,&pk,sizeof(pk));

        return true;
    }
    else if(type == 'I'){
        // IO packet, to enable/disable something.
        struct packet_io_t pk;
        network.read(header, &pk, sizeof(packet_io_t)); 

        return true;
    }

    return false;
}

void nrf24_rx_wake_up()
{
    lastMillis = -1;
    Serial.println(F("Wakeup!"));
}

void sleep_managment()
{
#ifdef ARDUINO_SLEEP
    if(lastMillis == -1){
        sleepCount = 0;
        lastMillis = millis();    
    }
    else if(millis() - lastMillis > ARDUINO_SLEEP_DELAY){

#ifdef NRF24_RADIO_MUTE
        radio.stopListening();
#endif 

        if(sleepCount == 0){
            Serial.println(F("Entering sleep!"));
            delay(100);
        }
        sleepCount++;

        attachInterrupt(0, nrf24_rx_wake_up, LOW);
        LowPower.powerDown(ARDUINO_SLEEP_TIME, ADC_OFF, BOD_OFF);
        detachInterrupt(0);

#ifdef NRF24_RADIO_MUTE
        radio.startListening();
#endif 

        if(sleepCount > ARDUINO_SLEEP_CHECK_WAN){
            if(!mesh.checkConnection()){
                Serial.println(F("Renewing Address"));
                mesh.renewAddress();
            }
            sleepCount = 0;
        }
    }
#endif
}


/******************************************************************************
 * Utils
 */
 
bool nrf24_send_packet(char type, void * pk, uint8_t pk_size)
{
    if(!mesh.write(pk, type, pk_size)) {
        if(!mesh.checkConnection()){
            Serial.println(F("Renewing Address"));
            mesh.renewAddress();
        }
        else{
            Serial.println(F("Send fail, Test OK"));
        }
    }
    else{
        Serial.println(F("Send OK!")); 
        return true;
    }
    return false;
}
