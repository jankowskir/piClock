#include <iostream>
#include <string>
#include <ctime>
#include <unistd.h>
#include <chrono>
#include <wiringPi.h>
#include <mcp23017.h>
#include <softPwm.h>
#include <stdio.h>
#include <fstream>
#include <thread>

using namespace std;

int minBrightness = 1; //PWM Display Minimum Duty Cycle
int midBrightness = 50; //PWM Display Half Duty Cycle
int maxBrightness = 100; //PWM Display Maximum Duty Cycle
int brightnessCount = 0;
int Min = 0; //Minute
int PriorMin = 99; //Default minute to force update on start
int Hour = 0; // Hour
int PriorHour = 99; //Default hour to force update on start
int PWM_pin = 1; //PWM Display pin number
int Separator = 0; //Separator state
int SeparatorCount = 0; //Count to blink the separator
string brightness;
//string newBrightness;
string oldBrightness;


// Inverse 7-segment display digits
int num_array[10][7] = {  { 0,0,0,0,0,0,1 },    // 0
                          { 1,0,0,1,1,1,1 },    // 1
                          { 0,0,1,0,0,1,0 },    // 2
                          { 0,0,0,0,1,1,0 },    // 3
                          { 1,0,0,1,1,0,0 },    // 4
                          { 0,1,0,0,1,0,0 },    // 5
                          { 0,1,0,0,0,0,0 },    // 6
                          { 0,0,0,1,1,1,1 },    // 7
                          { 0,0,0,0,0,0,0 },    // 8
                          { 0,0,0,1,1,0,0 }};   // 9


//7-Segment display digits of the clock from left to right
int digitOne[7] = {100,126,106,104,103,107,105};
int digitTwo[7] = {102,129,138,127,128,107,136};
int digitThree[7] = {143,140,151,137,139,142,141};
int digitFour[7] = {154,148,155,152,153,149,150};

//Use chrono to get the time
tm chronoTPtoTM(const chrono::system_clock::time_point& tp) {
    time_t aux = std::chrono::system_clock::to_time_t(tp);
    return *localtime(&aux);
}

void blinkSeparator(){
	if (SeparatorCount >= 8){
		if (Separator == 0){
			Separator = 1;
			SeparatorCount = 0;
		} else {
			Separator = 0;
			SeparatorCount = 0;
		}
	digitalWrite(101, Separator);
	}
SeparatorCount++;
}

//Get 7-segment settings for the given digit
string getDigital(int digit){
	string result = "";
		for (int d=0; d<7; d++) {
			result = result.append(to_string(num_array[digit][d]));
		}
	return result;
}

//Set the display to the correct numbers
void setPins(int digit, int value){
    string digitNumber;
    if (digit == 1){
        if (value == 0000001){ //If the first number in the hour is zero, turn off the segments
            for(int i=0; i < 7; i++){
                digitalWrite(digitOne[i], 1);
            } 
        } else { //Set the first digit of the hour
        digitNumber = "digitOne";
        digitalWrite(digitOne[0], value / 1000000 % 10);
        digitalWrite(digitOne[1], value / 100000 % 10);
        digitalWrite(digitOne[2], value / 10000 % 10);
        digitalWrite(digitOne[3], value / 1000 % 10);
        digitalWrite(digitOne[4], value / 100 % 10);
        digitalWrite(digitOne[5], value / 10 % 10);
        digitalWrite(digitOne[6], value % 10);
        }
    } else if (digit == 2){ //Set the second digit of the hour
        digitNumber = "digitTwo";
        digitalWrite(digitTwo[0], value / 1000000 % 10);
        digitalWrite(digitTwo[1], value / 100000 % 10);
        digitalWrite(digitTwo[2], value / 10000 % 10);
        digitalWrite(digitTwo[3], value / 1000 % 10);
        digitalWrite(digitTwo[4], value / 100 % 10);
        digitalWrite(digitTwo[5], value / 10 % 10);
        digitalWrite(digitTwo[6], value % 10);
    } else if (digit == 3){//Set the first digit of the minute
        digitNumber = "digitThree"; 
        digitalWrite(digitThree[0], value / 1000000 % 10);
        digitalWrite(digitThree[1], value / 100000 % 10);
        digitalWrite(digitThree[2], value / 10000 % 10);
        digitalWrite(digitThree[3], value / 1000 % 10);
        digitalWrite(digitThree[4], value / 100 % 10);
        digitalWrite(digitThree[5], value / 10 % 10);
        digitalWrite(digitThree[6], value % 10);
    } else if (digit == 4){//Set the second digit of the minute
        digitNumber = "digitFour";
        digitalWrite(digitFour[0], value / 1000000 % 10);
        digitalWrite(digitFour[1], value / 100000 % 10);
        digitalWrite(digitFour[2], value / 10000 % 10);
        digitalWrite(digitFour[3], value / 1000 % 10);
        digitalWrite(digitFour[4], value / 100 % 10);
        digitalWrite(digitFour[5], value / 10 % 10);
        digitalWrite(digitFour[6], value % 10);
    }
}

void setTimeHour(int h){
//    PriorHour = h;
	/*
    if (h >= 21 || h < 6){ //Dim the display between 10pm and 6am
		softPwmWrite(PWM_pin, minBrightness);
	} else { //Normal brightness
		softPwmWrite(PWM_pin, maxBrightness);
	}
    */

    if (h > 12){ //Convert to 12 hour
      h = h - 12;
	digitalWrite(125, 0); //turn on the PM indicator
    } else if (h == 0) {
        digitalWrite(125, 1); //turn off the PM indicator
        h = 12;
    } else {
	digitalWrite(125, 1); //turn off the PM indicator
    }

    int h1 = h / 10 % 10;
    int h2 = h % 10;
    setPins(1,stoi(getDigital(h1))); //Set 1st hour digit on the display
    setPins(2,stoi(getDigital(h2))); //Set 2nd hour digit on the display
}

void setTimeMin(int m){
    int m1 = m / 10 % 10;
    int m2 = m % 10;
    setPins(3, stoi(getDigital(m1))); //Set 1st minute digit on the display
    setPins(4, stoi(getDigital(m2))); //Set 2nd minute digit on the display
}

void setBrightness(){
    while (1) {
//    system("python3 clockbrightness.py");
    ifstream infile("/tmp/brightness");
    if (infile.good())
        {
            string sLine;
            getline(infile, sLine);
    	    brightness=sLine;
        }
    infile.close();
    if (brightness != oldBrightness){
        if (brightness == "High")
        {
            softPwmWrite(PWM_pin, maxBrightness);
        } else if (brightness == "Med")
        {
           softPwmWrite(PWM_pin, midBrightness); 
        } else if (brightness == "Low")
        {
            softPwmWrite(PWM_pin, minBrightness);
        } else {
            softPwmWrite(PWM_pin, maxBrightness);
        }
        oldBrightness = brightness;
        }
//    brightnessCount = 0;
    sleep(10);
    }
//brightnessCount++;
}

int main() { //Main program execution
    wiringPiSetup(); //Initialize the GPIO and MCP23017

    //Set up the MCP23017's
    mcp23017Setup(100, 0x23);
    mcp23017Setup(116, 0x25);
    mcp23017Setup(132, 0x26);
    mcp23017Setup(148, 0x27);

    //Set all the pin modes to output (can probably convert this to only use the utilized pins but I am lazy)
    for (int i = 0 ; i < 64 ; ++i){
        pinMode (100 + i, OUTPUT) ;
        digitalWrite(100+i, 1);
    }
    //Set up the PWM display
    softPwmCreate(PWM_pin, maxBrightness, maxBrightness);
    digitalWrite(101, 0); // turn the : on
    thread brightness(setBrightness);
    while (1) {
        chrono::system_clock::time_point t = chrono::system_clock::now();
        tm local_time = chronoTPtoTM(t);
        Min = local_time.tm_min;
        Hour = local_time.tm_hour;
        if (Min != PriorMin) { //If the minute changed, update the time
            if (Hour != PriorHour){ //If the hour changed too, update that as well
                setTimeHour(local_time.tm_hour);
                PriorHour = Hour;
            }
            setTimeMin(local_time.tm_min);
            PriorMin = Min;
	}
	usleep(125000); //250000 uSec = 1/8 second
//    setBrightness();
    blinkSeparator();
    }
    return 0;
}
