/*
****************************************************************************************
****************************************************************************************
20231121
Files Required to make a complete program - 
  CLI_V1.0, Quest_CLI.h, Quest_Flight.h, Quest_flight.cpp
      Important Functions and their use:
          cmd_takeSphot
              This function will use the serial camera (the one with the cable) to take a photo (jpg)
              on the transfer of this photo to the Host controller it will create a text file (txt)
              with the same file name as the jpg file of the conditions of when the jpg was taken.
              NOTE: add2text can be used with this command
          cmd_takeSpiphoto
              This function is for the SPI camera (the one that plugges directly on the microlab board)
              it will take a jpg the same as the serial camera, also creating a txt file.  Also NOTE: the
              add2text function can be used on the photo text file.
          nophotophoto
              if a camera is not being used, then this will simulate the camera photo operation. The text file 
              is still created, so the add2text function can be used the get data downloaded in the text file
          nophoto30K
              this dois not use a camera, instead it uses the file space for the photo the containe ascii data
              using dataappend function defined below.  The data will still have a jpg etension, but the file
              will contain ascii of the data put in buy the dataappend unction. Plus a text file is generated,
              and the add2text function will add more data.
          -----
          add2text(int value1,int value2,int value3)
              this function will take the three varables and place them at the end of the text file (the file is
              limited the 1024 charators),  if you look at the function the text output can be formatted is almost
              any way.  Look at it and change it to acccomadate your data...
          dataappend(int counts,int ampli,int SiPM,int Deadtime)
              this function will add ascii data to the 30K byte data buffer that was used a a jpg photo.  look at
              the data formating and change it necessary for you project information.  To view the data, you can
              use a hex/ascii file viewer or change the ext to a txt.  then it should be viewable with a text exitor.
.
******************************************************************************************                                   
******************************************************************************************                  
*/

#include "Quest_Flight.h"
#include "Quest_CLI.h"

//////////////////////////////////////////////////////////////////////////
//    This defines the timers used to control flight operations
//////////////////////////////////////////////////////////////////////////
//====== To Do Before Flight =============
//
// 1. initfram
// 2. enterTeamID
// 3. format
// 4. Speedfacot to 1
// 5. Set TimeEvent to the time to download the text file
// 6. Set Sensor1time to the number of min between readings
// 7. Set Event2time to the perion of time to check for flight operations
// 8. Set Wait24hour to 0 to wait 24 hours

#define IO7 (13)              // Input/Output to payload plus 
#define IO6 (12)              // Input/Output to payload plus 
#define IO5 (11)              // Input/Output to payload plus
#define IO4 (10)              // Input/Output to payload plus
#define IO3 (9)               // Input/Output to payload plus
#define IO2 (4)               // Input/Output to payload plus
#define IO1 (3)               // Input/Output to payload plus
#define IO0 (A6)              // Input/Output to payload plus 
//
//
uint16_t Wait24hour = 1;         // 0 = wait 24hour   1=Do not Wair 24 hour.
#define SpeedFactor 60          // = times faster
uint32_t  secperday =  86400;  //number of seconds in a day for wait time
//
//
//////////////////////////////////////////////////////////////////////////
//
#define one_sec   1000                       //one second = 1000 millis
#define one_min   60*one_sec                 // one minute of time
#define one_hour  60*one_min                 // one hour of time
//
//
#define TimeEvent1_time     ((one_min * 60) / SpeedFactor)      //take a photo time
#define Sensor1time         ((one_min * 5) / SpeedFactor)       //Time to make Sensor1 readings 
#define Event2time          ((one_min * 15) / SpeedFactor)      // 24 hour wait time
//
  int sensor1count = 0;     //counter of times the sensor has been accessed
  int Event2count = 0;     //counter of times the sensor has been accessed

//
//
    //
    //============  routins to support Ph value   ============
    //
    //
      // float mid_cal = 1704;
      // float low_cal = 1783;
      // float high_cal = 1624;
      // float voltage_mV = 0;
      float mid_cal = 1705;
      float low_cal = 1785;
      float high_cal = 1624;
      float voltage_mV = 0;
    //  
    //  
    // 
float read_voltage() {
  float voltage_mV = 0;
  uint8_t volt_avg_len =5;
  for (int i = 0; i < volt_avg_len; ++i) {
  voltage_mV += analogRead(A0) / 1024.0 * 3300.0;
  }
  voltage_mV /= volt_avg_len;
  return voltage_mV;
}
float ph(float voltage_mV) {
  if (voltage_mV > low_cal) {                //high voltage = low ph
    return 7.0 - 3.0 / (low_cal - mid_cal) * (voltage_mV - mid_cal);
  } else {
    return 7.0 - 3.0 / (mid_cal - high_cal) * (voltage_mV -mid_cal);
  }
}
// ===============  end of support of Ph  ==================
//
//
///////////////////////////////////////////////////////////////////////////
/**
   @brief Flying function is used to capture all logic of an experiment during flight.
*/
//************************************************************************
//   Beginning of the flight program setup
//
void Flying() {
  pinMode(IO7, OUTPUT);      //
  digitalWrite(IO7, LOW);   //
  pinMode(IO6, OUTPUT);      //
  digitalWrite(IO6, LOW);   //
  pinMode(IO5, OUTPUT);      //
  digitalWrite(IO5, LOW);   //
  pinMode(IO4, OUTPUT);      //
  digitalWrite(IO4, LOW);   //
  pinMode(IO3, OUTPUT);      //
  digitalWrite(IO3, LOW);   //
  pinMode(IO2, OUTPUT);      //
  digitalWrite(IO2, LOW);   //
  pinMode(IO1, OUTPUT);      //
  digitalWrite(IO1, LOW);   //
  pinMode(IO0, OUTPUT);      //
  digitalWrite(IO0, HIGH);   //
  //
  Serial.println("\n\rRun flight program\n\r");
  //
  uint32_t TimeEvent1 = millis();               //set TimeEvent1 to effective 0
  uint32_t Sensor1Timer = millis();             //clear sensor1Timer to effective 0
  uint32_t Event2Timer = millis();             //clear sensor1Timer to effective 0
  uint32_t Sensor2Deadmillis = millis();        //clear mills for difference
  uint32_t one_secTimer = millis();             //set happens every second
  uint32_t sec60Timer = millis();               //set minute timer

  //*****************************************************************
  //   Here to set up flight conditions i/o pins, atod, and other special condition
  //   of your program
  //
  //
  //
  //******************************************************************



  //------------ flying -----------------------

  Serial.println("Flying NOW  -  x=abort");                 //terminal output for abort back to test
  Serial.println("Terminal must be reset after abort");     //terminal reset requirement upon soft reset
  //
  missionMillis = millis();     //Set mission clock millis, you just entered flight conditions
  //
  /////////////////////////////////////////////////////
  //----- Here to start a flight from a reset ---------
  /////////////////////////////////////////////////////
  //
  DateTime now = rtc.now();                   //get time now
  currentunix = (now.unixtime());             //get current unix time, don't count time not flying
  writelongfram(currentunix, PreviousUnix);   //set fram Mission time start now counting seconds unix time
  //
  //***********************************************************************
  //***********************************************************************
  //  All Flight conditions are now set up,  NOW to enter flight operations
  //***********************************************************************
  //***********************************************************************
  //
  while (1) {
    //
    //----------- Test for terminal abort command (x) from flying ----------------------
    //
    while (Serial.available()) {      //Abort flight program progress
      byte x = Serial.read();         //get the input byte
      if (x == 'x') {                 //check the byte for an abort x
        return  ;                     //return back to poeration sellection
      }                               //end check
    }                                 //end abort check
//------------------------------------------------------------------
//
//*********** Timed Event 1 test ***************************************
// time to output a photo file only containing the text information
//  No need to delay 24 hours file operation output ok
//
    //  this test if TimeEvent1 time has come
    //  See above for TimeEvent1_time settings between this event
    //
    if ((millis() - TimeEvent1) > TimeEvent1_time) {
      TimeEvent1 = millis();                    //yes is time now reset TimeEvent1
    //   
    //  no camera - send just text appended with data to the output Queue
    //
    nophotophoto();               //photo event with no photo just to transfer data  
              
    }  //----------- end of TimeEvent1_time
    //
    //------------------------------------------------------------------
    //
//*******************************************************************************
//*********** One second counter timer will trigger every second ****************
//*******************************************************************************
    //  Here one sec timer - every second
    //
    if ((millis() - one_secTimer) > one_sec) {      //one sec counter
      one_secTimer = millis();                      //reset one second timer
      DotStarYellow();                              //turn on Yellow DotStar to Blink for running
      //
//****************** NO_NO_NO_NO_NO_NO_NO_NO_NO_NO_NO_ *************************
// DO NOT TOUCH THIS CODE IT IS NECESARY FOR PROPER MISSION CLOCK OPERATIONS
//    Mission clock timer
//    FRAM keep track of cunlitive power on time
//    and RTC with unix seconds
//------------------------------------------------------------------------------
      DateTime now = rtc.now();                           //get the time time,don't know how long away
      currentunix = (now.unixtime());                     //get current unix time
      Serial.print(currentunix); Serial.print(" ");      //testing print unix clock
      uint32_t framdeltaunix = (currentunix - readlongFromfram(PreviousUnix)); //get delta sec of unix time
      uint32_t cumunix = readlongFromfram(CumUnix);       //Get cumulative unix mission clock
      writelongfram((cumunix + framdeltaunix), CumUnix);  //add and Save cumulative unix time Mission
      writelongfram(currentunix, PreviousUnix);           //reset PreviousUnix to current for next time
//
//********* END_NO_END_NO_END_NO_END_NO_END_NO_END_NO_ **************************
      //
      //  This part prints out every second
      //
      Serial.print(": Mission Clock = ");             //testing print mission clock
      Serial.print(readlongFromfram(CumUnix));        //mission clock
      Serial.print(" is ");                           //spacer
      //
      //------Output to the terminal  days hours min sec
      //
      getmissionclk();
      Serial.print(xd); Serial.print(" Days  ");
      Serial.print(xh); Serial.print(" Hours  ");
      Serial.print(xm); Serial.print(" Min  ");
      Serial.print(xs); Serial.println(" Sec");
      //
      //
       DotStarOff();
    }  // end of one second routine
//
//**********************************************************************
//*********** Read Sensor1 Event read and add to text buffer************
//**********************************************************************
      //
    if ((millis() - Sensor1Timer) > Sensor1time) {    //Is it time to read?
      Sensor1Timer = millis();                        //Yes, lets read the sensor1
      sensor1count++;
      float ph_value = 0;
      voltage_mV = read_voltage();
      Serial.print(voltage_mV);Serial.println(" mV");
      
      ph_value = ph(voltage_mV);
      Serial.print("Ph value = ");Serial.println(ph_value);      
      uint16_t intPh = ph_value*100;
      int Ph  = intPh;
      uint16_t intMV = int(round(voltage_mV*100));
      Serial.println(voltage_mV*100);
      Serial.println(int(round(voltage_mV*100)));
      Serial.println(intMV);

      int mmV = int(round(voltage_mV*100));
      Serial.println(mmV);
      add2text(sensor1count, Ph,mmV);       //add the values to the text buffer
      //    
    }     // End of Sensor1 time event

//**********************************************************************
//*********** event 2 Event read will wait 24 hours befor doing this************
//*********** Will test for > than 24hours befor doing the event *****
//********************************************************************** 
    //  If it is event driven then remove the Event2Timer 
    //  here to get the  data for the event 
    //
    if ((millis() - Event2Timer) > Event2time) {    //Is it time to do an veent 2
      Event2Timer = millis();                       //Yes, lets read the sensor1
      //
      Serial.println(">>>>>>>>>>>  Event2 time >>>>>>>>>>>>>>>>>>>>>>>>");
      //
      if (Wait24hour == 1){                             //check for enable wait 24hour clock
        if ((readlongFromfram(CumUnix)> secperday)){          //mission clock for over 24 hours
           Serial.println("<<<<<<<<<<< Time is over 24 hours, can start experiment>>>>>>>>>>>>");
           //===================================================================================
           //========Experiment operations ie pump water etc Here ==============================
           //===================================================================================
           //
           
           //
           //===================================================================================
           //=========End of Experiment operations =============================================
           //===================================================================================
        }     
        //
        //  ****  place your pump operations here --  The program will get here after 24 hours
        //   ****  if the secper day 
      }   // End of Enable Wait24hours
    }     // End of Sensor2Timer          
  }       // End of while 
}         //End nof Flighting
//
//
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//    This is a function to adds three values to the user_text_buffer
//    Written specificy for 2023-2024 Team F, Team B,
//    Enter the function with "add2text(1st interger value, 2nd intergre value, 3rd intergervalue);
//    the " - value1 " text can be changed to lable the value or removed to same space
//    ", value2 " and ", value 3 " masy also be removed or changed to a lable.
//    Space availiable is 1024 bytes, Note- - each Data line has a ncarrage return and a line feed
//
//********************************** this is the one that is modified for team F  *******************************
//
//example of calling routine:
//       //
//      int count = 55;
//      float Ph = 55000;
//
//      add2text(value1, count, Ph);
//      //
//EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE  
//
char text_buffer[10]; // Adjust the size based on the expected length of the string         
//
void add2text(int value1,int value2,int mV){                 //Add value to text file
      Serial.print(value1);Serial.print("  ");Serial.println(value2);
        if (strlen(user_text_buf0) >= (sizeof(user_text_buf0)-100)){    //Check for full
          Serial.println("text buffer full");                           //yes, say so
          return;                                                       //back to calling
        }
        char temp[11];                  // Maximum number of digits for a 32-bit integer 
        int index = 10;                 //Start from the end of the temperary buffer  
        char str[12];                   //digits + null terminator   
//--------- get time and convert to str for entry into text buffer ----
        DateTime now = rtc.now();                   //get time of entry
        uint32_t value = now.unixtime();            //get unix time from entry
        Serial.println(value); 
     //            
    /*    do {
            temp[index--] = '0' + (value % 10);     // Convert the least significant digit to ASCII
            value /= 10;                            // Move to the next digit      
        } 
        while (value != 0);
            strcpy(str, temp + index +1);               // Copy the result to the output string
        //
    */
         sprintf(temp, "%d", value);
         strcpy(str, temp);               // Copy the result to the output string
        //
//---------- end of time conversion uni time is now in str -------------       
        strcat(user_text_buf0, (str));              //write unix time
        Serial.println(user_text_buf0);
        //
        //
        // unit time finish entry into this data line
        //
        //
        // Serial.println(text_buffer);
        //
        //
        strcat(user_text_buf0, ("-cnt="));            // seperator
        strcat(user_text_buf0, (itoa(value1, ascii, 10)));
        strcat(user_text_buf0, ("Ph="));
        strcat(user_text_buf0, (itoa(value2, ascii, 10)));       
        strcat(user_text_buf0, ("mV="));
        strcat(user_text_buf0, (itoa(mV, ascii, 10)));      
        strcat(user_text_buf0, ("\r\n"));

        //Serial.println(strlen(user_text_buf0));  //for testing
}        

//------end of Function to add to user text buffer ------ 
//

//=============================================================================
//
////FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//  Function to write into a 30K databuffer
//    char databuffer[30000];         // Create a character buffer with a size of 2KB
//    int databufferLength = 0;       // Initialize the buffer length
//  Append data to the large data buffer buffer always enter unit time of data added
//  enter: void dataappend(int counts, int ampli, int SiPM, int Deadtime) (4 values)
//
void dataappend(int counts,int ampli,int SiPM,int Deadtime) {          //entry, add line with values to databuffer
  //----- get and set time to entry -----
  DateTime now = rtc.now();                                               //get time of entry
  String stringValue = String(now.unixtime());                            //convert unix time to string
  const char* charValue = stringValue.c_str();                            //convert to a C string value
  appendToBuffer(charValue);                                              //Sent unix time to databuffer
  //----- add formated string to buffer -----
  String results = " - " + String(counts) + " " + String(ampli) + " " + String(SiPM) + " " + String (Deadtime) + "\r\n";  //format databuffer entry
  const char* charValue1 = results.c_str();                               //convert to a C string value
  appendToBuffer(charValue1);                                             //Send formated string to databuff
  //
  //  Serial.println(databufferLength);                                   //print buffer length for testing only
}
//-----------------------                                               //end dataappend
//----- sub part od dataappend -- append to Buffer -----
//-----------------------
void  appendToBuffer(const char* data) {                                   //enter with charator string to append
  int dataLength = strlen(data);                                          //define the length of data to append
      // ----- Check if there is enough space in the buffer                           //enough space?
  if (databufferLength + dataLength < sizeof(databuffer)) {               //enouth space left in buffer
      // ----- Append the data to the buffer
    strcat(databuffer, data);                                             //yes enough space, add data to end of buffer
    databufferLength += dataLength;                                       //change to length of the buffer
  } else {
    Serial.println("Buffer is full. Data not appended.");                 //Not enough space, say so on terminal
  }       //end not enough space
}         //end appendToBuffer
//

//=================================================================================================================
//=================================================================================================================
