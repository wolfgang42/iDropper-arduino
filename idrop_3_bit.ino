void idrop_recv_bit(boolean mark) {
  static byte state = 0;
  static char rchar;
  static byte ri;
  if (state == 0) { // Skip the first bit
    state = 1;
  } else if (state == 1) { // Idle
    if (!mark) {
      state = 2; // A space was received, the byte begins
      rchar = 0x00;
      ri=0;
    }
  } else if (state == 2) {
    if (mark) {
      rchar |= (1 << ri);
    }
    if (ri == 8) {
      state = 3;
    }
    ri++;
  } else if (state == 3) {
    if (mark) {
      idrop_recv_byte(rchar);
      state=1;
    } else {
      Serial.print("Recv error, out of sync!");
      state = 0;
    }
  } else {
    Serial.println("FATAL: unknown error!");
  }
  if (!idrop_connected) {
    state = 0;
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

