// ***** 0. Documentation Section *****
// TableTrafficLight.c for Lab 10
// Runs on LM4F120/TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Daniel Valvano, Jonathan Valvano
// January 15, 2016


// Written By: Hossam Eid

// east red light connected to PB5
// east yellow light connected to PB4
// east green light connected to PB3
// north facing red light connected to PB2
// north facing yellow light connected to PB1
// north facing green light connected to PB0
// pedestrian detector connected to PE2 (1=pedestrian present)
// north car detector connected to PE1 (1=car present)
// east car detector connected to PE0 (1=car present)
// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"

// ***** 2. Global Declarations Section *****

struct State{
	unsigned long TrafficLeds;	//Traffic Leds output Pattern
	unsigned long WalkLeds;			//walk Led output pattern
	unsigned long Time;					//Delay time for this state
	unsigned long Next[8];}; 		//Index of next state
typedef const struct State sTyp;

#define GoEast 		0
#define WaitEast 	1
#define	GoNorth 	2
#define WaitNorth 3
#define Walk 			4
#define FlashOff1 5
#define FlashOn1  6
#define FlashOff2 7
#define FlashOn2	8

//			FSM Model
sTyp FSM[9]= {
{0xC, 0x02, 80,{GoEast, GoEast, WaitEast,WaitEast,WaitEast,WaitEast,WaitEast,WaitEast}},
{0x14, 0x02,50, {GoNorth, GoNorth, GoNorth, GoNorth, Walk, Walk, Walk, Walk}},
{0x21, 0x02, 80, {GoNorth, WaitNorth, GoNorth, WaitNorth, WaitNorth, WaitNorth, WaitNorth, WaitNorth}},
{0x22, 0x02, 50, {GoEast, GoEast, GoEast, GoEast, Walk, Walk, Walk, GoEast}},
{0x24, 0x08, 80, {Walk, FlashOff1, FlashOff1, FlashOff1, Walk, FlashOff1, FlashOff1, FlashOff1}},
{0x24, 0x00, 50, {FlashOn1, FlashOn1, FlashOn1, FlashOn1, FlashOn1, FlashOn1, FlashOn1, FlashOn1}},
{0x24, 0x02, 50, {FlashOff2, FlashOff2, FlashOff2, FlashOff2, FlashOff2, FlashOff2, FlashOff2, FlashOff2}},
{0x24, 0x00, 50, {FlashOn2, FlashOn2, FlashOn2, FlashOn2, FlashOn2, FlashOn2, FlashOn2, FlashOn2}},
{0x24, 0x02, 50, {GoEast, GoEast, GoNorth, GoEast, GoEast, GoEast, GoNorth, GoNorth}}};

unsigned long currState;
unsigned long input;

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void PortF_Init(void);
void PortE_Init(void);
void PortB_Init(void);
void SysTick_wait(unsigned long delay);
void SysTick_wait10ms(unsigned long delay);
// ***** 3. Subroutines Section *****

int main(void){ unsigned long volatile delay;
  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210,ScopeOff); // activate grader and set system clock to 80 MHz
	
	SYSCTL_RCGC2_R = 0x32;					//Enable the clock for port B, E and F
	delay = SYSCTL_RCGC2_R;					//delay to wait for the clock to start
  PortF_Init();
	PortE_Init();
	PortB_Init();
	
	currState = GoEast;							//Initialzing the states
  EnableInterrupts();
  while(1){
		GPIO_PORTB_DATA_R = FSM[currState].TrafficLeds;     //Sets the Traffic Light LEDS
		GPIO_PORTF_DATA_R = FSM[currState].WalkLeds;				//Sets the Walk LED
    input = GPIO_PORTE_DATA_R & 0x07;										//Gets the input from the 3 sensors
		SysTick_wait10ms(FSM[currState].Time);							//Wait
		currState = FSM[currState].Next[input];							//Update the state
  }
}



//  Subroutines defenitions
void PortF_Init(void){
	
	GPIO_PORTF_AMSEL_R  &= ~0x0A;						//1- Disable the analog function for bits PF3 and PF1 
	GPIO_PORTF_PCTL_R &= 0x0;								//2- Disable The special digital function
	GPIO_PORTF_DIR_R |= 0x0A;								//3- Setting PF3 and PF1 as outputs
	GPIO_PORTF_AFSEL_R &= ~0x0A;						//4- Disable the alternative analog function
	GPIO_PORTF_DEN_R |= 0x0A;								//5- enable Digital
 	
}

void PortE_Init(void){
	
	GPIO_PORTE_AMSEL_R  &= ~0x07;						//1- Disable the analog function for bits PE0 ,PE1 and PE2 
	GPIO_PORTE_PCTL_R &= 0x0;								//2- Disable The special digital function
	GPIO_PORTE_DIR_R &= ~0x07;							//3- Setting PE2 , PE1 and PE0 as inputs
	GPIO_PORTE_AFSEL_R &= ~0x07;						//4- Disable the alternative analog function
	GPIO_PORTE_DEN_R |= 0x07;								//5- enable Digital
	
}

void PortB_Init(void){
	
	GPIO_PORTB_AMSEL_R  &= ~0x3F;
	GPIO_PORTB_PCTL_R &= 0x0;
	GPIO_PORTB_DIR_R |= 0x3F;								//Setting PB5 to PB0 as outputs
	GPIO_PORTB_AFSEL_R &= ~0x3F;
	GPIO_PORTB_DEN_R |= 0x3F;
	
}

void SysTick_wait(unsigned long delay){

	NVIC_ST_CTRL_R = 0;
	NVIC_ST_RELOAD_R = delay;
	NVIC_ST_CURRENT_R = 0;
	NVIC_ST_CTRL_R = 0x5;
	while((NVIC_ST_CTRL_R & 0x10000) == 00){}

}

void SysTick_wait10ms(unsigned long delay){
	
	while(delay--)
	{
		SysTick_wait(800000);
	}
}