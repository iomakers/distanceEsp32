/**
 * Sensor de distância usando ESP32
 * Autor: João Campos
 *
 * Tutoriais Utilizados
 * https://github.com/Ebiroll/esp32_ultra/blob/master/main/main.cpp
 * https://github.com/EngineeringLibrary/embeddedSystemControl/blob/master/main/ultrassonic.cpp
 * https://techtutorialsx.com/2017/05/09/esp32-running-code-on-a-specific-core/
 */



#include <sys/time.h>



static void ultraDistanceTask(void *inpar);
uint32_t get_usec();


float distance_1 = 0;

// hardware timer
hw_timer_t * timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;


void IRAM_ATTR onTimer(){
  // Increment the counter and set the time of ISR
  //portENTER_CRITICAL_ISR(&timerMux);
  //portEXIT_CRITICAL_ISR(&timerMux);

  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timerSemaphore, NULL);

  // It is safe to use digitalRead/Write here if you want to toggle an output
}



#define ECHO_PIN GPIO_NUM_4
#define TRIG_PIN GPIO_NUM_15

void setup() {
  Serial.begin(115200);
  delay(10);

  xTaskCreatePinnedToCore(&ultraDistanceTask, "ultra", 4096, NULL, 20, NULL, 0);

  timerSemaphore = xSemaphoreCreateBinary();
  timer = timerBegin(0, 80, true);

  // Attach onTimer function to our timer.
  timerAttachInterrupt(timer, &onTimer, true);

  // Colocar alarme para a função onTimer ser chamada a cada 2 s (valor em microsegundos).
  // Repetir alarme (=true)
  timerAlarmWrite(timer, 2000000, true);

}

void loop(){

  if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE){
          Serial.println(distance_1);
  }

}


////
//// Toggle trig pin and wait for input on echo pin
////
static void ultraDistanceTask(void *inpar) {

    gpio_pad_select_gpio(TRIG_PIN);
    gpio_pad_select_gpio(ECHO_PIN);

    gpio_set_direction(TRIG_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);

    while(1) {
        // HC-SR04P
        gpio_set_level(TRIG_PIN, 1);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gpio_set_level(TRIG_PIN, 0);
        uint32_t startTime=get_usec();

        while (gpio_get_level(ECHO_PIN)==0 && get_usec()-startTime < 500*1000)
        {
            // Wait until echo goes high
        }

        startTime=get_usec();

        while (gpio_get_level(ECHO_PIN)==1 && get_usec()-startTime < 500*1000)
        {
            // Wait until echo goes low again
        }

        if (gpio_get_level(ECHO_PIN) == 0)
        {
            uint32_t diff = get_usec() - startTime; // Diff time in uSecs
            // Distance is TimeEchoInSeconds * SpeeOfSound / 2
            distance_1 = 340.29 * diff / (1000 * 1000 * 2); // Distance in meters
           // printf("Distance is %f cm\n", distance * 100);
        }
        else
        {
            // No value
            printf("Did not receive a response!\n");
        }
        // Delay and re run.
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}


// Similar to uint32_t system_get_time(void)
uint32_t get_usec() {

 struct timeval tv;

 //              struct timeval {
 //                time_t      tv_sec;     // seconds
 //                suseconds_t tv_usec;    // microseconds
 //              };

 gettimeofday(&tv,NULL);
 return (tv.tv_sec*1000000 + tv.tv_usec);

}
