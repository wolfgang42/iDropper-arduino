boolean idrop_detect(){
  digitalWrite(PIN_IDROP_OUT, LOW);
  pinMode(PIN_IDROP_IN, OUTPUT);
  digitalWrite(PIN_IDROP_IN, HIGH);
  delayMicroseconds(10);
  pinMode(PIN_IDROP_IN, INPUT);
  digitalWrite(PIN_IDROP_IN, LOW);
  delayMicroseconds(90);
  return digitalRead(PIN_IDROP_IN) == LOW;
}

unsigned long idrop_emit(boolean on) {
  pinMode(PIN_IDROP_IN, OUTPUT);
  digitalWrite(PIN_IDROP_IN, LOW);
  if (on) {
    digitalWrite(PIN_IDROP_OUT, HIGH);
  } else {
    digitalWrite(PIN_IDROP_OUT, LOW);
  }
}
