#ifndef F_CPU
#define F_CPU 3333333
#endif

#include <avr/io.h>

/* FeeRTOS include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stream_buffer.h"
#include "message_buffer.h"
#include "semphr.h"
#define LED1 PIN4_bm
#define LED2 PIN5_bm
#define LED3 PIN6_bm
#define BUTTON_UP    PIN2_bm  // Up button connected to Pin A2
#define BUTTON_DOWN  PIN3_bm  // Down button connected to Pin A3
// TODO Declare data types
//make sure to use vDelay instead of normal delay
// TODO Declare global inter-task communication primitives
// e.g., Semaphores, Queues, etc.
QueueHandle_t         led_queue;
//SemaphoreHandle_t     semaphore;
// TODO Define Task Functions
typedef enum {
    BUTTON_NONE,
    BUTTON_UP_PRESSED,
    BUTTON_DOWN_PRESSED
} ButtonPress_t;
typedef enum {
    LED_COLOR_RED,
    LED_COLOR_YELLOW,
    LED_COLOR_GREEN
} LEDColor_t;
// Task 1: Polls the Up button
void TskUpButton(void *pvParameters)
{

    int buttonState;
    int lastButtonState = BUTTON_UP;
    ButtonPress_t buttonPress;
    //basically everytime it detects a  button press, we send it to the queue 
    //where the LED control will receive it, and switch case it.
    while(1)
    {
        // checks whether button up has been pressed
        buttonState = PORTA.IN & BUTTON_UP;

        // basically if button down is pressed...
        if ((buttonState == 0) && (lastButtonState != 0))
        {
            // if Button is pressed send it back to the queue
            buttonPress = BUTTON_UP_PRESSED;
            xQueueSend(led_queue, &buttonPress, portMAX_DELAY);
        }

        lastButtonState = buttonState;
        //debouncing
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Task 2: Polls the Down button
void TskDownButton(void *pvParameters)
{

    int buttonState;
    int lastButtonState = BUTTON_DOWN;
    ButtonPress_t buttonPress;
    //basically everytime it detects a down button press, we send it to the queue 
    //where the LED control will receive it, and switch case it.
    while(1)
    {
        // checks whether button down has been pressed
        
        buttonState = PORTA.IN & BUTTON_DOWN;

        // basically if button down is pressed...
        if ((buttonState == 0) && (lastButtonState != 0))
        {
            // if Button is pressed send it back to the queue
            buttonPress = BUTTON_DOWN_PRESSED;
            xQueueSend(led_queue, &buttonPress, portMAX_DELAY);
        }
        
        lastButtonState = buttonState;

        //add a delay for debouncing
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Task 3: Controls the LEDs
void TskLEDControl(void *pvParameters)
{

    LEDColor_t currentLED = LED_COLOR_RED;  // Start with red LED
    ButtonPress_t buttonPress;

    while(1)
    {
        xQueueReceive(led_queue, &buttonPress, portMAX_DELAY);
        // Wait for a button press message
            // Update the current LED based on the button press
            switch(buttonPress){
                case BUTTON_UP_PRESSED:
                    if (currentLED == LED_COLOR_RED)
                    {
                        currentLED = LED_COLOR_YELLOW;
                    }
                    else if (currentLED == LED_COLOR_YELLOW)
                    {
                        currentLED = LED_COLOR_GREEN;
                    }
                    else if (currentLED == LED_COLOR_GREEN)
                    {
                        currentLED = LED_COLOR_RED;
                    }
                    break;

                case BUTTON_DOWN_PRESSED:
                    if (currentLED == LED_COLOR_RED)
                    {
                        currentLED = LED_COLOR_GREEN;
                    }
                    else if (currentLED == LED_COLOR_YELLOW)
                    {
                        currentLED = LED_COLOR_RED;
                    }
                    else if (currentLED == LED_COLOR_GREEN)
                    {
                        currentLED = LED_COLOR_YELLOW;
                    }
                    break;

                default:
                    break;
            }

//update LEDS
            switch(currentLED)
            {
                case LED_COLOR_RED:
                    PORTA.OUT |= LED1;
                    PORTA.OUTCLR = LED2 | LED3;
                    break;
                case LED_COLOR_YELLOW:
                    PORTA.OUT |= LED2;
                    PORTA.OUTCLR = LED1 | LED3;
                    break;
                case LED_COLOR_GREEN:
                    PORTA.OUT |= LED3;
                    PORTA.OUTCLR = LED1 | LED2;
                    break;
            }
        }
    }
int main(void) {
	// TODO Initialize GPIO Pins
    PORTA.DIR |= LED1 | LED2 | LED3;
    PORTA.DIRCLR = PIN2_bm | PIN3_bm;
    PORTA.DIRCLR = BUTTON_UP | BUTTON_DOWN;
    PORTA.PIN2CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN3CTRL = PORT_PULLUPEN_bm;
    PORTA.OUT |= LED1;
    PORTA.OUTCLR = LED2 | LED3;
    // TODO Initialize inter-task communication primitives
    led_queue = xQueueCreate(10, sizeof(ButtonPress_t));
    // TODO Create tasks with xTaskCreate()
    int led1Pin = LED1;
    int led2Pin = LED2;
    int led3Pin = LED3;
    //specifies with LED the task should control
    //pass the address to know which led to control
    xTaskCreate(TskUpButton, "UpButton", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(TskDownButton, "DownButton", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(TskLEDControl, "LEDControl", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	vTaskStartScheduler();
	return 0;
}
