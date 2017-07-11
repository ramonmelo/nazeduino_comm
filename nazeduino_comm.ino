
#include <SoftwareSerial.h>

#define IDLE 1
#define START 2
#define START_M 4
#define START_A 8
#define START_CMD 16
#define START_LOAD 32
#define START_CHK 64

#define CMD_RC 105


#define AUX_SERIAL_RX 10
#define AUX_SERIAL_TX 11


SoftwareSerial auxSerial(AUX_SERIAL_RX, AUX_SERIAL_TX); // RX TX

void setup()
{
  Serial.begin(115200);

  auxSerial.begin(9600);
  auxSerial.println("Hello Naze!");
}

void loop()
{
  uint8_t datad = 0;
  uint8_t *data = &datad;

  auxSerial.println("asking for data!");

  send_msp( CMD_RC, data, 0);
  readData();

  delay(100);
}

void send_msp(uint8_t opcode, uint8_t *data, uint8_t n_bytes) {

  uint8_t checksum = 0;

  Serial.write((byte *)"$M<", 3);
  Serial.write(n_bytes);
  checksum ^= n_bytes;

  Serial.write(opcode);
  checksum ^= opcode;

  Serial.write(checksum);
}

void readData() {
  delay(100);

  byte state = IDLE;
  byte size = 0;
  byte temp_size = 0;
  byte cmd = 0;

  String value;
  byte count = 0;

  while (Serial.available()) {
    byte c = Serial.read();

    if ( state == IDLE && c == '$' ) {
        state = START;
    } else if ( state == START && c == 'M') {
        state = START_M;
    } else if ( state == START_M && c == '>' ) {
        state = START_A;
    } else if ( state == START_A ) {
        size = c;
        temp_size = size;
        state = START_CMD;
    } else if ( state == START_CMD ) {
        cmd = c;
        state = START_LOAD;
    } else if ( state == START_LOAD ) {

        switch(cmd) {
            case CMD_RC: {
                if (temp_size == 0)
                {
                    state == START_CHK;
                    break;
                }

                value.concat(String(c, BIN));

                count += 1;
                count %= 2;

                if (count == 0)
                {
                    auxSerial.println(binToInt(value));
                    value = String();
                }

                temp_size -= 1;
                break;
            }
        }
    } else if ( state == START_CHK ) {
        // save checksum
        state = IDLE;
    }
  }

  auxSerial.println();
}

int binToInt(String value) {
    byte size = value.length();
    int output = 0;

    for (int i = size - 1; i >=0; --i)
    {
        int base = pow(2, i);

        if ( value.charAt(i) == '1' )
        {
            output += base;
        }
    }

    return output;
}
