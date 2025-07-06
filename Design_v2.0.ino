
  #include "arduinoFFT.h"
  
  #define SAMPLES 256                //Number of samples, must be in base 2 
  #define SAMPLING_FREQUENCY 30000   //Sampliling frequency, Nyquist freq, 2 times the highest expected frequency.
  #define A0 26
  #define A1 27
  
  arduinoFFT FFT = arduinoFFT();
  
  unsigned int samplingPeriod; 
  unsigned long microSeconds;
  
  double vReal1[SAMPLES];                                             //create vector of size SAMPLES to hold real values
  double vImag1[SAMPLES];                                             //create vector of size SAMPLES to hold imaginary values
  double vReal2[SAMPLES];                                             //create vector of size SAMPLES to hold real values
  double vImag2[SAMPLES];                                             //create vector of size SAMPLES to hold imaginary values
  double Trigger[12];
  int check = 0;                                                      //Bool to trigger the start of opperation 

  // For printing the vector
  #define SCL_INDEX 0x00
  #define SCL_TIME 0x01
  #define SCL_FREQUENCY 0x02
  #define SCL_PLOT 0x03


  
  void setup() 
  { // Debug 
    Serial.begin(115200);                                           //Baud rate for the Serial Monitor - serial debugging 
      samplingPeriod = round(1000000*(1.0/SAMPLING_FREQUENCY));     //Period in microseconds 

      //Pin setup for hbridge
      pinMode(10, OUTPUT);//ENB
      pinMode(11, OUTPUT);//IN4
      pinMode(12, OUTPUT);//IN3
      pinMode(13, OUTPUT);//IN2
      pinMode(14, OUTPUT);//IN1
      pinMode(15, OUTPUT);//ENA

      digitalWrite(14, HIGH);
      digitalWrite(13, LOW);
      digitalWrite(12, LOW);
      digitalWrite(11, HIGH);

  }
  
  void loop() 
  {  
    /*Sample SAMPLES times*/
    for(int i=0; i<SAMPLES; i++)
    {
        microSeconds = micros();                                       //Returns the number of microseconds since the Arduino board began running the current script. 
      
        vReal1[i] = analogRead(A0);                                     //Reads the value from analog pin 26 (A0), quantize it and save it as a real term.
        vImag1[i] = 0;                                                  //Makes imaginary term 0 always

        vReal2[i] = analogRead(A1);                                     //Reads the value from analog pin 27 (A1), quantize it and save it as a real term.
        vImag2[i] = 0;                                                  //Makes imaginary term 0 always        

        /*remaining wait time between samples if necessary*/
        while(micros() < (microSeconds + samplingPeriod))
        {
          //do nothing
        }
    }



    /*Perform FFT on mic 1 samples*/
    FFT.Windowing(vReal1, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal1, vImag1, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal1, vImag1, SAMPLES);

    /*Perform FFT on mic 2 samples*/
    FFT.Windowing(vReal2, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal2, vImag2, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal2, vImag2, SAMPLES);

  /* for self refference of used frequencies 
  index   9 = 1 054.6875 Hz  - trigger freq
  index 100 = 11 718.75
  index  75 = 8 789.0625
  index  50 = 5 859.375
  */

  if (vReal1 [9] < vReal2 [9] ){
    vReal1[9] = vReal2[9];
  }
    if (vReal1 [40] < vReal2 [40] ){
    vReal1[40] = vReal2[40];
  }
    if (vReal1 [35] < vReal2 [35] ){
    vReal1[35] = vReal2[35];
  }
    if (vReal1 [30] < vReal2 [30] ){    
    vReal1[30] = vReal2[30];
  }
    if (vReal1 [25] < vReal2 [25] ){    
    vReal1[25] = vReal2[25];
  }

  

    double avgTrigger = 0;
    for ( int i = 0; i<10 ; i++){
        Trigger [i] = Trigger [i + 1];
    }

    Trigger [10] = vReal1[9];
    for ( int i = 0; i<11 ; i++){
        avgTrigger = avgTrigger + Trigger [i];
    }

    if ( avgTrigger > 550 ){

      check = 10;
      
    }


    // Debugging and monitoring 
    Serial.print("-------DETECT :  ");      
    Serial.print(avgTrigger);     
    Serial.print("-------RIGHT magnetude:  ");      
    Serial.print(vReal1[100]);      
    Serial.print("-------STRAIGHT magnetude:  ");      
    Serial.print(vReal1[75]);      
    Serial.print("-------LEFT magnetude:  ");      
    Serial.print(vReal1[50]);      
    Serial.println("  ");      

    // start of opperation 
    if ( check  > 1){

    
      // Go Straight
      if (vReal1[30] > vReal1[40] && vReal1[30] > vReal1[35] && vReal1[30] > vReal1[25]){
        /*
        analogWrite(15, 255);
        analogWrite(10, 255);
        delay(15);
        analogWrite(15, 185);
        analogWrite(10, 185);
        */
                analogWrite(10, 255);
        delay(35);
        analogWrite(15, 0);
        analogWrite(10, 100);
      
      // Go left    
      } else if(vReal1[35] > vReal1[40] && vReal1[35] > vReal1[30] && vReal1[35] > vReal1[25]) {
        analogWrite(15, 255);
        delay(35);
        analogWrite(15, 100);
        analogWrite(10, 0);
      

      // Go Right 
      } else if(vReal1[25] > vReal1[40] && vReal1[25] > vReal1[35] && vReal1[25] > vReal1[30] ) {
        /*
        analogWrite(10, 255);
        delay(25);
        analogWrite(15, 0);
        analogWrite(10, 100);
        */
              analogWrite(15, 255);
        analogWrite(10, 255);
        delay(25);
        analogWrite(15, 185);
        analogWrite(10, 185);
      // Do Nothing
      } else if ( vReal1[40] > vReal1[35] && vReal1[40] > vReal1[30] && vReal1[40] > vReal1[25] ){
        analogWrite(15, 0);
        analogWrite(10, 0);

      }
    // Remain stationary
    else{
        analogWrite(15, 0);
        analogWrite(10, 0);
      }
    }
    else{
      analogWrite(15, 0);
      analogWrite(10, 0);
    }

    // Debugging and Monitoring 
    //Serial.println("Computed magnitudes:");
    //PrintVector(vReal1, (SAMPLES >> 1), SCL_FREQUENCY);


    /*Find peak frequency and print peak
    double peak = FFT.MajorPeak(vReal1, SAMPLES, SAMPLING_FREQUENCY);
    Serial.println(peak);     //Print out the most dominant frequency.*/

  
/*
 
      /*Script stops here. Hardware reset required.*/
      /*while (1); //do one time*/
  }














  void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
  {
    for  (uint16_t i = bufferSize; i > 0 ; i--)     //for (uint16_t i = 0; i < bufferSize; i++) (uint16_t i = 885; i > 875 ; i--)    // 
    {
      double abscissa;
      /* Print abscissa value */
      switch (scaleType)
      {
        case SCL_INDEX:
          abscissa = (i * 1.0);
    break;
        case SCL_TIME:
          abscissa = ((i * 1.0) / SAMPLING_FREQUENCY);
    break;
        case SCL_FREQUENCY:
          abscissa = ((i * 1.0 * SAMPLING_FREQUENCY) / SAMPLES);
    break;
      }
      Serial.print(" @");
      Serial.print(i);
      Serial.print(" ");
      Serial.print(abscissa, 2);
      if(scaleType==SCL_FREQUENCY)
        Serial.print("Hz");
      Serial.print(" ");
      Serial.print((vData[i]), 2);  //ln
      Serial.print(" || ");
    }
    Serial.println();
  }