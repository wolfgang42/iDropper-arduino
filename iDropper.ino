#define PIN_IDROP_IN 3
#define PIN_IDROP_OUT 4
#define IDROP_MAX_WAIT 400000

#define MODE_OFF  0
#define MODE_EMIT 1
#define MODE_BIAS 2
#define MODE_READ 3

void setup() {
  Serial.begin(9600);
  pinMode(PIN_IDROP_OUT, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(11, OUTPUT);
}

boolean idrop_connected = false;

void loop() {
  idrop_loop();
  digitalWrite(13, idrop_connected?HIGH:LOW);
}

void idrop_next_byte(boolean &idle, byte &resp) {
  idle = true;
  resp = 0x00;
  //return;
  /*idle = false;
  static byte i = 0;
  static const char message[13] = "Hello world!";
  resp = message[i++];
  if (i > 11) {
    i = 0;
    idrop_emit(false);
    delay(1500);
  }*/
  /*if (Serial.available() > 0) {
    idle = false;
    resp = Serial.read();
  }*/
  static byte oldresp = '\0';
  resp = map(analogRead(1), 0, 1024, 0, 9) + '0';
  idle = resp == oldresp;
  oldresp = resp;
}

void idrop_recv_byte(byte b) {
  if (b >= '0' && b <= '9') {
    byte val = b - '0';
    analogWrite(11, map(val, 0, 9, 20, 200));
  } else {
    Serial.println(b);
  }
}
