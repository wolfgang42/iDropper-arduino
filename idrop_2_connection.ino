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

void idrop_recv_connected() {
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
          if (lightsSeen <= 6) { // space
            idrop_recv_bit(false);
            return;
          } else { // mark
            idrop_recv_bit(true);
            return;
          }
        }
      }
    }
  }
  idrop_connected = false;
}

