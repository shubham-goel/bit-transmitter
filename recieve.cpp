
String received = "";
int state;
boolean receiving;
String input, message;

int system_rec_state = 0;

void setup() {
  pinMode(A2, INPUT);
  pinMode(13, OUTPUT);
  Serial.begin(9600);           // start serial for output
  reset();
}

void reset(){
  state = 0;
  input = "";
  receiving = false;
  system_rec_state = 0;
}

boolean frame_check(boolean a){
  boolean res = false;
  switch(state){
    case 0: if(!a){
              state++;
            }
            else{
              state=0;
            }
            break;
    case 1: if(a){
              state++;
            }
            else{
              state=1;
            }
            break;
    case 2:if(a){
              state++;
            }
            else{
              state=1;
            }
            break;
    case 3:if(a){
              state++;
            }
            else{
              state=1;
            }
            break;
    case 4:if(a){
              state++;
            }
            else{
              state=1;
            }
            break;
    case 5:if(a){
              state++;
            }
            else{
              state=1;
            }
            break;
    case 6:if(a){
              state++;
            }
            else{
              state=1;
            }
            break;
    case 7:if(!a){
              res = true;
            }
            state=0;
  }
  return res;
}


void sendFlag() {
  digitalWrite(13, LOW);
  delay(100);
  int i = 0;
  while(i < 6) {
    digitalWrite(13, HIGH);
    delay(100);
    i++;
  }
  digitalWrite(13, LOW);
  delay(100);
}

void sendNACK() {
  sendFlag();
  digitalWrite(13, LOW);
  delay(100);
  sendFlag();   
  Serial.println("NACK sent");
}

void sendACK() {
  sendFlag();
  digitalWrite(13, HIGH);
  delay(100);
  sendFlag();  
  Serial.println("ACK sent");
}


void removeBitStuff(String &received) {
  String temp = "";
  int temp2 = 0;
  for (int i = 0; i < received.length(); i++) {
    temp += received[i];
    if (received[i] == '1') {
      temp2 += 1;
      if (temp2 == 5) {
        i++;
        temp2 = 0;
      }
    } else {
      temp2 = 0;
    }
  }
  received = temp;
}

int strToInt(String x) {
  int res=0;
  int pow=1;
  for(int i=x.length()-1;i>=0;i-=1) {
    if(x[i] == '1') {
      res += pow;
    }

    pow*=2;
  }
  return res;
}
boolean errorCheck(String &received) {
  Serial.println(received);
  int len = strToInt(received.substring(0, 6));
  Serial.println("Len:");
  Serial.println(len);
  String dummy = received.substring(6,6+len);
  String d2 = dummy;
  int k = len % 8;
  if (k != 0) {
    k = 8 - k;
  }
  while (k > 0) {
    dummy += '0';
    k -= 1;
  }
  int a[] = {0, 0, 0, 0, 0, 0, 0, 0};
  int dl = dummy.length();
  int numrows = dl / 8;
  int parity = 0;
  String down = "";
  String sidebits = "";
  for (int i = 0; i < numrows; i++) {
    parity = 0;
    for (int j = 0; j < 8; j++) {
      if (dummy[i * 8 + j] == '1') {
        parity = 1 - parity;
        a[j] = 1 - a[j];
      }
    }
    if (parity == 0) {
      sidebits += '0';
    }
    else {
      sidebits += '1';
    }
  }
  for (int i = 0; i < 8; i++) {
    if (a[i] == 0) {
      down += '0';
    }
    else {
      down += '1';
    }
  }
  Serial.println("Down : " + down);
  Serial.println("sidebits : " + sidebits);

  if (down == received.substring(6 + len, 14 + len) && sidebits == received.substring(14 + len, 14 + len + numrows)) {
    message = d2;
    return true;
  } else {
    return false;
  }
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
// void receiveEvent(int howMany) {
//   Serial.println("Started Receiving!");
//   while (Wire.available()) { // loop through all but the last
//     char c =  (char)Wire.read(); // receive byte as a character
//     received += c;         // print the character
//   }

//   if (checkStartEnd(received)) {
//     removeBitStuff(received);
//     if (errorCheck(received)) {
//       Serial.print(received);
//       sendACK();
//     } else {
//       sendNACK();
//       secondTime = true;
//       Wire.onReceive(receiveEvent); // register event
//     }
//   } else {
//     sendNACK();
//     secondTime = true;
//     Wire.onReceive(receiveEvent); // register event
//   }

//   // int x = Wire.read();    // receive byte as an integer
//   // Serial.println(x);         // print the integer
// }


void loop(){
  if(system_rec_state == 0){
    boolean a = digitalRead(A2);

    if(receiving){
      if(a){
        input += '1';
      }
      else{
        input += '0';
      }
    }
    else{
      input="";
    }
    if(frame_check(a)){
      Serial.println("Frame detected");
      if(receiving){
        system_rec_state = 1;
      }
      receiving=!receiving;
      Serial.println("receiving : ");
      Serial.println(receiving);
    }
  }
  else if(system_rec_state == 1){
    Serial.println("Received String : " + input);

    input = input.substring(0,input.length() - 8);
    removeBitStuff(input);
    if(errorCheck(input)){
      Serial.println("Message Received successfully!");
      delay(2000);
      Serial.println("Message : " + message);
      system_rec_state = 2;
    }
    else{
      Serial.println("Message Received unsuccessfully!");
      delay(2000);
      system_rec_state = 3;
    }
  }
  else if(system_rec_state == 2){
    sendACK();
    reset();
    system_rec_state = 0;
  }
  else if(system_rec_state == 3){
    sendNACK();
    reset();
    system_rec_state = 0;
  }

  delay(100);
}
