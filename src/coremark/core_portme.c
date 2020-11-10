/*
	File : core_portme.c
*/
/*
	Author : Shay Gal-On, EEMBC
	Legal : TODO!
*/
#include <stdio.h>
#include <stdlib.h>
#include "coremark.h"
#include "core_portme.h"
//#include "sc_test.h"

#if VALIDATION_RUN
	volatile ee_s32 seed1_volatile=0x3415;
	volatile ee_s32 seed2_volatile=0x3415;
	volatile ee_s32 seed3_volatile=0x66;
#endif
#if PERFORMANCE_RUN
	volatile ee_s32 seed1_volatile=0x0;
	volatile ee_s32 seed2_volatile=0x0;
	volatile ee_s32 seed3_volatile=0x66;
#endif
#if PROFILE_RUN
	volatile ee_s32 seed1_volatile=0x8;
	volatile ee_s32 seed2_volatile=0x8;
	volatile ee_s32 seed3_volatile=0x8;
#endif

	volatile ee_s32 seed4_volatile=ITERATIONS;
	volatile ee_s32 seed5_volatile=0;

/* Porting : Timing functions
	How to capture time and convert to seconds must be ported to whatever is supported by the platform.
	e.g. Read value from on board RTC, read value from cpu clock cycles performance counter etc.
	Sample implementation for standard time.h and windows.h definitions included.
*/
#if 1

#define CLOCKS_PER_SEC 10000000
#define MHZ	CLOCKS_PER_SEC/1000000
#define TIMER ((int volatile *)0x20000040)
#define INSNC ((unsigned int volatile *)0x20000060) //trace and debug unit
#define UART	 ((int volatile *)0x20000000)
CORETIMETYPE barebones_clock() {
	int cycles;
	cycles=TIMER[3];
	return cycles;
}
/* Define : TIMER_RES_DIVIDER
	Divider to trade off timer resolution and total time that can be measured.

	Use lower values to increase resolution, but make sure that overflow does not occur.
	If there are issues with the return value overflowing, increase this value.
	*/
/* #define NSECS_PER_SEC CLOCKS_PER_SEC */
/* #define CORETIMETYPE clock_t */
#define GETMYTIME(_t) (*_t=barebones_clock())
#define MYTIMEDIFF(fin,ini) ((fin)-(ini))
#define TIMER_RES_DIVIDER 1
#define SAMPLE_TIME_IMPLEMENTATION 1
#define EE_TICKS_PER_SEC (CLOCKS_PER_SEC / TIMER_RES_DIVIDER)
#else

#endif
/* Function : insn
	This function starts the instruction counter
*/
void start_insn()
{
	INSNC[1]=3;//reset
	INSNC[1]=2;//enable
}
/* Function : insn
	This function stops the instruction counter
*/
void stop_insn()
{
	INSNC[1]=0;//disable
}
/** Define Host specific (POSIX), or target specific global time variables. */
static CORETIMETYPE start_time_val, stop_time_val;

/* Function : start_time
	This function will be called right before starting the timed portion of the benchmark.

	Implementation may be capturing a system timer (as implemented in the example code)
	or zeroing some system parameters - e.g. setting the cpu clocks cycles to 0.
*/
void start_time(void) {
	start_insn();//start instruction counter
	
	TIMER[0]=0xFFFFFFFF;
	TIMER[1]=3;//reset
	TIMER[1]=1;//enable
	start_time_val= (clock_t) TIMER[3];
	//GETMYTIME(&start_time_val );
}
/* Function : stop_time
	This function will be called right after ending the timed portion of the benchmark.

	Implementation may be capturing a system timer (as implemented in the example code)
	or other system parameters - e.g. reading the current value of cpu cycles counter.
*/
void stop_time(void) {
	stop_insn();//stope instruction counter
	TIMER[1]=0;//disable
	stop_time_val=(clock_t) TIMER[3];
	//GETMYTIME(&stop_time_val );
	INSNC[1]=0;//disable
}

/* Function : insn
	This function will return the number of instructions retired by the processor 
*/
int insn()
{
	int cycles;
	cycles=INSNC[2];
	return cycles;
}
/* Function : get_time
	Return an abstract "ticks" number that signifies time on the system.

	Actual value returned may be cpu cycles, milliseconds or any other value,
	as long as it can be converted to seconds by <time_in_secs>.
	This methodology is taken to accomodate any hardware or simulated platform.
	The sample implementation returns millisecs by default,
	and the resolution is controlled by <TIMER_RES_DIVIDER>
*/
CORE_TICKS get_time(void) {
	CORE_TICKS elapsed=(CORE_TICKS)(MYTIMEDIFF(stop_time_val, start_time_val));
	return elapsed;
}
/* Function : time_in_secs
	Convert the value returned by get_time to seconds.

	The <secs_ret> type is used to accomodate systems with no support for floating point.
	Default implementation implemented by the EE_TICKS_PER_SEC macro above.
*/
secs_ret time_in_secs(CORE_TICKS ticks) {
	secs_ret retval=((secs_ret)ticks) / (secs_ret)EE_TICKS_PER_SEC;
	return retval;
}

ee_u32 default_num_contexts=1;

/* Function : portable_init
	Target specific initialization code
	Test for some common mistakes.
*/
void delay_ms( int ms ){ 
	TIMER[0]=(CLOCKS_PER_SEC/1000);//set timer count
	for(int cnt=0;cnt<ms;cnt++){
		TIMER[1]=3;//reset
		TIMER[1]=1;//enable
		while(!(TIMER[2]&0x1));//check for end
	}
}
void UART_INIT(const int baud){
	UART[0]=0x0888;//Line control register
	UART[1]=CLOCKS_PER_SEC/baud;//set clock ticks baudrate to measured
	UART[3]=0x10;//baudrate selector set to manual and keep dma
}
void portable_init(core_portable *p, int *argc, char *argv[])
{
	UART_INIT(9600);
	delay_ms(100);  
  ee_printf("CoreMark 1.0\n");
  
	if (sizeof(ee_ptr_int) != sizeof(ee_u8 *)) {
		ee_printf("ERROR! Please define ee_ptr_int to a type that holds a pointer! (%u != %u)\n", sizeof(ee_ptr_int), sizeof(ee_u8 *));
	}
	if (sizeof(ee_u32) != 4) {
		ee_printf("ERROR! Please define ee_u32 to a 32b unsigned type! (%u)\n", sizeof(ee_u32));
	}
	p->portable_id=1;
}
/* Function : portable_fini
	Target specific final code
*/


void portable_fini(core_portable *p)
{
	 //
	CORE_TICKS total_time=get_time();
		
	float Cycles_Per_Instruction = ((float) total_time)/((float) INSNC[2]);	
	float CoreMark = ((float)(default_num_contexts * ITERATIONS)) /((float) time_in_secs(total_time));	
	float CoreMark_Per_MHZ= CoreMark/((float) MHZ);
	ee_printf("Cycles_Per_Instruction: %f\n",Cycles_Per_Instruction);		
	ee_printf("CoreMark 1.0 : %f\n",CoreMark);
	ee_printf("CoreMark/MHz : %f\n",CoreMark_Per_MHZ);
    
	ee_printf("FINISH\n");

}
