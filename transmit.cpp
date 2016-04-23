int outPin = 13;

String user_input;
String encMessage;
String received;
char system_state;

int state,system_rec_state;
boolean receiving;
String input;

int error1=-1,error2=-1;
void reset_rec(){
  state = 0;
  input = "";
  receiving = false;
  system_rec_state = 0;
}


void setup() {
	// put your setup code here, to run once:
	system_state = '0';
	String received = "";
	user_input = "";
	encMessage = "";
	Serial.begin(9600);
	Serial.println("Enter your first message");
	reset_rec();
	pinMode(A2, INPUT);
	pinMode(outPin, OUTPUT);
}

String bitstuff(String payload) {

	int counter = 0;
	int n = payload.length();
	int numones = 0;
	String stuffed = "";
	while (counter != n) {
		if (payload[counter] == '1') {
			numones += 1;
		}
		else{
			numones = 0;			
		}
		stuffed += payload[counter];
		if (numones == 5) {
			stuffed += '0';
			numones = 0;
		}
		counter += 1;
	}
	return stuffed;
}

String redundant_bits(String payload) {
	int n = payload.length();
	String downbits = "";
	String sidebits = "";

	// payload is already a multiple of 8

	int a[] = {0, 0, 0, 0, 0, 0, 0, 0};
	int numrows = n / 8;
	int parity = 0;
	for (int i = 0; i < numrows; i++) {
		parity = 0;
		for (int j = 0; j < 8; j++) {
			if (payload[i * 8 + j] == '1') {
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
			downbits += '0';
		}
		else {
			downbits += '1';
		}
	}

	return downbits + sidebits;
}

String length_bits(int n) {
	String len = "";
	while (n != 0) {
		if (n % 2 == 0) {
			len = '0' + len;
		}
		else {
			len = '1' + len;
		}
		n = n / 2;
	}
	int l = len.length();
	String temp = "";
	while (l < 6) {
		temp += '0';
		l++;
	}
	len = temp + len;
	return len;
}

String encode(String payload) {
	int counter = 0;
	int numones = 0;
	int n = payload.length();

	String dummy = payload;

	int k = n % 8;
	if (k != 0) {
		k = 8 - k;
	}
	while (k > 0) {
		dummy += '0';
		k -= 1;
	}


	String encMessage = "";
	encMessage += "01111110";
	String len_bits = length_bits(n);
	String red_bits = redundant_bits(dummy);
	String bitstuffed = bitstuff(len_bits + payload + red_bits);
	encMessage = encMessage + bitstuff(length_bits(n) + payload + redundant_bits(dummy));
	Serial.println("len_bits: " + len_bits);
	Serial.println("payload: " + payload);
	Serial.println("red_bits: " + red_bits);
	Serial.println("bitstuffed: " + bitstuffed);
	encMessage += "01111110";

	return encMessage;
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

void receiveMsg(String&receive_string){
	reset_rec();
	while(loop_receive()){
		delay(100);
	}
	receive_string = input;
	return;
}

boolean loop_receive(){
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
    	if(receiving){
    		system_rec_state = 1;
    	}
    	receiving=!receiving;
    }
    return true;
  }
  else if(system_rec_state == 1){
  	input = input.substring(0,input.length() - 8);
  	return false;
  }
}
void send_msg(String msg){
	Serial.println("Started Transmission");
	for (int i = 0; i < msg.length(); ++i)
	{
		if(msg[i] == '1'){
			digitalWrite(outPin,HIGH);
		}
		else{
			digitalWrite(outPin,LOW);
		}
		delay(100);
	}
	Serial.println("Ended Transmission");
}


void loop() {
	// transmit to device #8
	// put your main code here, to run repeatedly:
	if (system_state == '0') {
		String received = "";
		user_input = "";
		encMessage = "";
		system_state = 'a';
	}
	else if (system_state == 'a') { //Input

		if (Serial.available() > 0) {
			char c = Serial.read();
			if (c == 'a') {
				Serial.println("You entered : " + user_input);
				system_state = 'b';
			}
			else if(c == 'e'){
				if(error1==-1){
					error1 = user_input.length()-1;
				}
				else if(error2 == -1){
					error2 = user_input.length()-1;
				} else{
					Serial.println("Cannot handle more than 2 errors");
				}
			}
			else {
				user_input += c;
			}
		}
	}
	else if (system_state == 'b') { //Encode

		encMessage = encode(user_input);
		Serial.println("Encoded message: " + encMessage);	
		system_state = 'c';
	}
	else if (system_state == 'c') { //Transmit
		String errorMessage = encMessage;
		if(error1 != -1){
			if(errorMessage[8+6+error1] == '1'){
				errorMessage[8+6+error1] = '0';
			}
			else{
				errorMessage[8+6+error1] = '1';
			}
		}
		if(error2 != -1){
			if(errorMessage[8+6+error2] == '1'){
				errorMessage[8+6+error2] = '0';
			}
			else{
				errorMessage[8+6+error2] = '1';
			}
		}

		Serial.println("error message: " + errorMessage);
		send_msg(errorMessage);
		error1=error2=-1;
		system_state = 'd';
	}
	else if (system_state == 'd') { //Listen
		String rec_msg="";
		receiveMsg(rec_msg);
		if(rec_msg == "1"){//ACK
			system_state = '0';
			Serial.println("Message Delivered Succcessfully!\n\n Enter next message");
		}
		else if(rec_msg == "0"){//NACK
			system_state = 'e';
			Serial.println("Message Delivered Unsucccessfully!\nRetransmitting");
			delay(35);	
		}
		else{
			Serial.println("Unexpected ACK/NACK error!");
			Serial.println("Unexpected ACK/NACK error!");
		}
	}
	else if (system_state == 'e') { //Retransmit
		send_msg(encMessage);
		system_state = 'd';
	}

	delay(10);
}


// Input
// encode
// Transmit
// listen/recieve
// re-transmit
// listen/recieve
// back to Input
