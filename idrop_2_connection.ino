void idrop_recv_connected() {
  boolean mark = idrop_read_connected();    
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

boolean idrop_read_connected() {
  byte lightsSeen = 0;
  boolean lit1 = false;
  boolean lit2 = false;
  byte darkCount = 0;
  boolean waitForDark = false;
  for (byte i = 0; i < 40; i++) {
    boolean nowLit = idrop_detect();
    if (nowLit) lightsSeen++;
    if (!waitForDark) { // Waiting for 2 lit, then dark
      if (lit1 && lit2 && !nowLit) { // We were lit, now darkness
        waitForDark = true;
        darkCount = 0;
      } else {
        // Advance everything along, I guess.
        lit1 = lit2;
        lit2 = nowLit;
      }
    } else { // waitForDark
      if (nowLit) { // We were expecting dark, but got light instead, reset
        waitForDark = false;
        lit1 = false;
        lit2 = false;
      } else {
        darkCount++;
        if (darkCount >= 10) {
         if (lightsSeen <= 6) {
            return false; // space
          } else {
            return true; //  mark
          }
        }
      }
    }
  }
  idrop_connected = false;
  return true;// TODO we seem to have disconnected.
}

