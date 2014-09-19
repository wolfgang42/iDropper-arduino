void idrop_loop() {
  idrop_send();
  idrop_recv();
}

void idrop_send() {
  idrop_emit(true);
  boolean mark = idrop_connected?idrop_next_bit():true;
  delayMicroseconds(mark?1000:500);
  idrop_emit(false);
}

void idrop_recv() {
  if (idrop_connected) {
    idrop_recv_connected();
  } else {
    idrop_recv_idle();
  }
}

void idrop_recv_idle() {
  // Look for light 4ms
  boolean wasLight = false;
  for (byte i = 0; i < 40; i++) {
    boolean nowLit = idrop_detect();
    if (wasLight && nowLit) {
      idrop_connected = true;
      return;
    }
    wasLight = nowLit;
  }
}

boolean idrop_next_bit() {  
  static boolean sending = false;
  static char sendingChar;
  static byte sendingCharPos = 0;
  static char bufBegin = 0;
  if (!sending) {
    boolean idle;
    byte next;
    idrop_next_byte(idle, next);
    if (!idle) {
      sendingChar = next;
      //Serial.print("Send: ");
      //Serial.println(sendingChar);
      sendingCharPos = 0;
      bufBegin=true;
      sending = true;
    }
    return true; // Send marks to pad the byte
  } else {
    static boolean start = true;
    if (bufBegin < 2) {
      bufBegin++;
      start = true;
      return true;
    }
    if (start) {
      start = false;
      return false; // space at beginning of byte
    }
    if (sendingCharPos == 8) {
      sending = false;
      return true; // Send a mark at the end of a byte
    } else {
      boolean isMark = sendingChar&1;
      sendingChar = sendingChar >> 1;
      sendingCharPos++;
      return isMark;
    }
  }
}

