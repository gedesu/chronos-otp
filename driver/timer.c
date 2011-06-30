// *************************************************************************************************
//
//	Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/ 
//	 
//	 
//	  Redistribution and use in source and binary forms, with or without 
//	  modification, are permitted provided that the following conditions 
//	  are met:
//	
//	    Redistributions of source code must retain the above copyright 
//	    notice, this list of conditions and the following disclaimer.
//	 
//	    Redistributions in binary form must reproduce the above copyright
//	    notice, this list of conditions and the following disclaimer in the 
//	    documentation and/or other materials provided with the   
//	    distribution.
//	 
//	    Neither the name of Texas Instruments Incorporated nor the names of
//	    its contributors may be used to endorse or promote products derived
//	    from this software without specific prior written permission.
//	
//	  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//	  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//	  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//	  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//	  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//	  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//	  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//	  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//	  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//	  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//	  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// *************************************************************************************************
// Timer service routines.
// *************************************************************************************************


// *************************************************************************************************
// Include section

// system
#include "project.h"

// driver
#include "timer.h"
#include "ports.h"
#include "buzzer.h"
#include "vti_ps.h"
#ifdef FEATURE_PROVIDE_ACCEL
#include "vti_as.h"
#endif
#include "display.h"

// logic
#include "clock.h"
#include "battery.h"
#include "stopwatch.h"
#include "alarm.h"
#include "display.h"
#include "rfsimpliciti.h"
#include "simpliciti.h"


#ifdef CONFIG_EGGTIMER
#include "eggtimer.h"
#endif

#ifdef CONFIG_OTP
#include "otp.h"
#endif

// *************************************************************************************************
// Prototypes section
void Timer0_Init(void);
void Timer0_Stop(void);
void Timer0_A1_Start(u16 ticks);
void Timer0_A1_Stop(void);
void Timer0_A3_Start(u16 ticks);
void Timer0_A3_Stop(void);
void Timer0_A4_Delay(u16 ticks);
void (*fptr_Timer0_A3_function)(void);
#ifdef CONFIG_USE_GPS
void (*fptr_Timer0_A1_function)(void);
#endif

// *************************************************************************************************
// Defines section


// *************************************************************************************************
// Global Variable section
struct timer sTimer;

// *************************************************************************************************
// Extern section
extern void BRRX_TimerTask_v(void);
extern void to_lpm(void);


// *************************************************************************************************
// @fn          Timer0_Init
// @brief       Set Timer0 to a period of 1 or 2 sec. IRQ TACCR0 is asserted when timer overflows.
// @param       none
// @return      none
// *************************************************************************************************
void Timer0_Init(void)
{
	// Set interrupt frequency to 1Hz
	TA0CCR0   = 32768 - 1;                

	// Enable timer interrupt    
	TA0CCTL0 |= CCIE;                     

	// Clear and start timer now   
	// Continuous mode: Count to 0xFFFF and restart from 0 again - 1sec timing will be generated by ISR
	TA0CTL   |= TASSEL0 + MC1 + TACLR;                       
}


// *************************************************************************************************
// @fn          Timer0_Start
// @brief       Start Timer0.
// @param       none
// @return      none
// *************************************************************************************************
void Timer0_Start(void)
{ 
	// Start Timer0 in continuous mode	 
	TA0CTL |= MC_2;          
}


// *************************************************************************************************
// @fn          Timer0_Stop
// @brief       Stop and reset Timer0.
// @param       none
// @return      none
// *************************************************************************************************
void Timer0_Stop(void)
{ 
	// Stop Timer0	 
	TA0CTL &= ~MC_2;

	// Set Timer0 count register to 0x0000
	TA0R = 0;                             
}


void Timer0_A1_Start(u16 ticks)
{
	/*old version
	// Set interrupt frequency to 1Hz
	TA0CCR1   = TA0R + 32678 ;

	// Enable timer interrupt
	TA0CCTL1 |= CCIE; */

	u16 value;

		// Store timer ticks in global variable
		sTimer.timer0_A1_ticks = ticks;

		// Delay based on current counter value
		value = TA0R + ticks;

		// Update CCR
		TA0CCR1 = value;

		// Reset IRQ flag
		TA0CCTL1 &= ~CCIFG;

		// Enable timer interrupt
		TA0CCTL1 |= CCIE;
	
}

void Timer0_A1_Stop(void)
{
	// Clear timer interrupt    
	TA0CCTL1 &= ~CCIE; 
}



// *************************************************************************************************
// @fn          Timer0_A3_Start
// @brief       Trigger IRQ every "ticks" microseconds
// @param       ticks (1 tick = 1/32768 sec)
// @return      none
// *************************************************************************************************
void Timer0_A3_Start(u16 ticks)
{
	u16 value;
	
	// Store timer ticks in global variable
	sTimer.timer0_A3_ticks = ticks;
	
	// Delay based on current counter value
	value = TA0R + ticks;
	
	// Update CCR
	TA0CCR3 = value;   

	// Reset IRQ flag    
	TA0CCTL3 &= ~CCIFG; 
	          
	// Enable timer interrupt    
	TA0CCTL3 |= CCIE; 
}



// *************************************************************************************************
// @fn          Timer0_A3_Stop
// @brief       Stop Timer0_A3.
// @param       none
// @return      none
// *************************************************************************************************
void Timer0_A3_Stop(void)
{
	// Clear timer interrupt    
	TA0CCTL3 &= ~CCIE; 
}


// *************************************************************************************************
// @fn          Timer0_A4_Delay
// @brief       Wait for some microseconds
// @param       ticks (1 tick = 1/32768 sec)
// @return      none
// *************************************************************************************************
void Timer0_A4_Delay(u16 ticks)
{
	u16 value;
	
	// Exit immediately if Timer0 not running - otherwise we'll get stuck here
	if ((TA0CTL & (BIT4 | BIT5)) == 0) return;    

	// Disable timer interrupt    
	TA0CCTL4 &= ~CCIE; 	

	// Clear delay_over flag
	sys.flag.delay_over = 0;
	
	// Add delay to current timer value
	value = TA0R + ticks;
	
	// Update CCR
	TA0CCR4 = value;   

	// Reset IRQ flag    
	TA0CCTL4 &= ~CCIFG; 
	          
	// Enable timer interrupt    
	TA0CCTL4 |= CCIE; 
	
	// Wait for timer IRQ
	while (1)
	{
		// Delay in LPM
		to_lpm();

#ifdef USE_WATCHDOG		
		// Service watchdog
		WDTCTL = WDTPW + WDTIS__512K + WDTSSEL__ACLK + WDTCNTCL;
#endif
#ifdef CONFIG_STOP_WATCH
		// Redraw stopwatch display
		if (is_stopwatch_run()) display_stopwatch(LINE2, DISPLAY_LINE_UPDATE_PARTIAL);
#endif

		// Check stop condition
		if (sys.flag.delay_over) break;
	}
}



// *************************************************************************************************
// @fn          TIMER0_A0_ISR
// @brief       IRQ handler for TIMER0_A0 IRQ
//				Timer0_A0	1/1sec clock tick 			(serviced by function TIMER0_A0_ISR)
//				Timer0_A1	 							(serviced by function TIMER0_A1_5_ISR)
//				Timer0_A2	1/100 sec Stopwatch			(serviced by function TIMER0_A1_5_ISR)
//				Timer0_A3	Configurable periodic IRQ	(serviced by function TIMER0_A1_5_ISR)
//				Timer0_A4	One-time delay				(serviced by function TIMER0_A1_5_ISR)
// @param       none
// @return      none
// *************************************************************************************************
//pfs 
#ifdef __GNUC__  
#include <signal.h>
interrupt (TIMER0_A0_VECTOR) TIMER0_A0_ISR(void)
#else
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
#endif
{
	static u8 button_lock_counter = 0;
	static u8 button_beep_counter = 0;
	
	// Disable IE 
	TA0CCTL0 &= ~CCIE;
	// Reset IRQ flag  
	TA0CCTL0 &= ~CCIFG;  
	// Add 1 sec to TACCR0 register (IRQ will be asserted at 0x7FFF and 0xFFFF = 1 sec intervals)
	TA0CCR0 += 32768;
	// Enable IE 
	TA0CCTL0 |= CCIE;
	
	// Add 1 second to global time
	clock_tick();
	
	#ifdef CONFIG_OTP
	// Rechecks 
	if (!sOtp_cache.inactive) {
	  if (sTime.second == 0 || sTime.second == 30) {
	    recalculate_otp();
	  }
	}
	#endif
	
	// Set clock update flag
	display.flag.update_time = 1;
	
	// While SimpliciTI stack operates or BlueRobin searches, freeze system state
	if (is_rf())
	{
		// SimpliciTI automatic timeout
		if (sRFsmpl.timeout == 0) 
		{
			simpliciti_flag |= SIMPLICITI_TRIGGER_STOP;
		}
		else
		{
			sRFsmpl.timeout--;
		}
		
		// Exit from LPM3 on RETI
		_BIC_SR_IRQ(LPM3_bits);     
		return;
	}
	
	// -------------------------------------------------------------------
	// Service modules that require 1/min processing
	if (sTime.drawFlag >= 2) 
	{
		#ifdef CONFIG_BATTERY
		// Measure battery voltage to keep track of remaining battery life
		request.flag.voltage_measurement = 1;
		#endif
		
		#ifdef CONFIG_ALARM
		// If the chime is enabled, we beep here
		if (sTime.minute == 0) {
			if (sAlarm.hourly == ALARM_ENABLED) {
				request.flag.buzzer = 1;
			}
		}
		// Check if alarm needs to be turned on
		check_alarm();
		#endif
	}

	// -------------------------------------------------------------------
	// Service active modules that require 1/s processing
	
	#ifdef CONFIG_ALARM  // N8VI NOTE eventually, eggtimer should use this code too
	// Generate alarm signal
	if (sAlarm.state == ALARM_ON) 
	{
		// Decrement alarm duration counter
		if (sAlarm.duration-- > 0)
		{
			request.flag.buzzer = 1;
		}
		else
		{
			sAlarm.duration = ALARM_ON_DURATION;
			stop_alarm();
		}
	}
	#endif

	#ifdef CONFIG_BATTERY
	// If battery is low, decrement display counter
	if (sys.flag.low_battery)
	{
		if (sBatt.lobatt_display-- == 0) 
		{
			message.flag.prepare = 1;
			message.flag.type_lobatt = 1;
			sBatt.lobatt_display = BATTERY_LOW_MESSAGE_CYCLE;
		}
	}
	#endif
	
	// If a message has to be displayed, set display flag
	if (message.all_flags)
	{
		if (message.flag.prepare)
		{
			message.flag.prepare = 0;
			message.flag.show    = 1;
		}
		else if (message.flag.erase) // message cycle is over, so erase it
		{
			message.flag.erase       = 0;
			message.flag.block_line1 = 0;
			message.flag.block_line2 = 0;
			display.flag.full_update = 1;
		}	
	}
	
	// -------------------------------------------------------------------
	// Check idle timeout, set timeout flag
	if (sys.flag.idle_timeout_enabled)
	{
		if (sTime.system_time - sTime.last_activity > INACTIVITY_TIME) sys.flag.idle_timeout = 1; //setFlag(sysFlag_g, SYS_TIMEOUT_IDLE);
	}
	
	// -------------------------------------------------------------------
	// Detect continuous button high states

	if (BUTTON_STAR_IS_PRESSED && BUTTON_UP_IS_PRESSED)
	{
		if (button_beep_counter++ > LEFT_BUTTON_LONG_TIME)
		{
			// Toggle no_beep buttons flag
			sys.flag.no_beep = ~sys.flag.no_beep;
	
			// Show "beep / nobeep" message synchronously with next second tick
			message.flag.prepare = 1;
			if (sys.flag.no_beep)	message.flag.type_no_beep_on   = 1;
			else					message.flag.type_no_beep_off  = 1;
			
			// Reset button beep counter
			button_beep_counter = 0;
		}
	} else if (BUTTON_NUM_IS_PRESSED && BUTTON_DOWN_IS_PRESSED) // Trying to lock/unlock buttons?
	{
		if (button_lock_counter++ > LEFT_BUTTON_LONG_TIME)
		{
			// Toggle lock / unlock buttons flag
			sys.flag.lock_buttons = ~sys.flag.lock_buttons;
	
			// Show "buttons are locked/unlocked" message synchronously with next second tick
			message.flag.prepare = 1;
			if (sys.flag.lock_buttons)	message.flag.type_locked   = 1;
			else						message.flag.type_unlocked = 1;
			
			// Reset button lock counter
			button_lock_counter = 0;
		}
	}
	else // Trying to create a long button press?
	{
		// Reset button lock counter
		button_lock_counter = 0;
		
		if (BUTTON_STAR_IS_PRESSED) 	
		{
			sButton.star_timeout++;
			
			// Check if button was held low for some seconds
			if (sButton.star_timeout > LEFT_BUTTON_LONG_TIME) 
			{
				button.flag.star_long = 1;
				sButton.star_timeout = 0;
			}
		}
		else
		{
			sButton.star_timeout = 0;
		}
	
		if (BUTTON_NUM_IS_PRESSED) 	
		{
			sButton.num_timeout++;
		
			// Check if button was held low for some seconds
			if (sButton.num_timeout > LEFT_BUTTON_LONG_TIME) 
			{
				button.flag.num_long = 1;
				sButton.num_timeout = 0;
			}
		}
		else
		{
			sButton.num_timeout = 0;
		}
	}
	
	// Exit from LPM3 on RETI
	_BIC_SR_IRQ(LPM3_bits);               
}


// *************************************************************************************************
// @fn          Timer0_A1_5_ISR
// @brief       IRQ handler for timer IRQ.
//				Timer0_A0	1/1sec clock tick (serviced by function TIMER0_A0_ISR)
//				Timer0_A1	BlueRobin timer / doorlock
//				Timer0_A2	1/100 sec Stopwatch
//				Timer0_A3	Configurable periodic IRQ (used by button_repeat and buzzer)
//				Timer0_A4	One-time delay
// @param       none
// @return      none
// *************************************************************************************************
//pfs 
#ifdef __GNUC__
#include <signal.h>
interrupt (TIMER0_A1_VECTOR) TIMER0_A1_5_ISR(void)
#else
#pragma vector = TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_5_ISR(void)
#endif
{
	u16 value;
		
	switch (TA0IV)
	{
		// Timer0_A2	1/1 or 1/100 sec Stopwatch				
		case 0x04:	// Timer0_A2 handler
					// Disable IE 
					TA0CCTL2 &= ~CCIE;
					// Reset IRQ flag  
					TA0CCTL2 &= ~CCIFG;  
					// Load CCR register with next capture point
#ifdef CONFIG_STOP_WATCH
					update_stopwatch_timer();
#endif
#ifdef CONFIG_EGGTIMER
					update_eggtimer_timer();
#endif
					// Enable timer interrupt    
					TA0CCTL2 |= CCIE; 	
					// Increase stopwatch counter
#ifdef CONFIG_STOP_WATCH
					stopwatch_tick();
#endif
#ifdef CONFIG_EGGTIMER
					eggtimer_tick();
#endif
					break;
					
		// Timer0_A3	Configurable periodic IRQ (used by button_repeat and buzzer)			
		case 0x06:	// Disable IE 
					TA0CCTL3 &= ~CCIE;
					// Reset IRQ flag  
					TA0CCTL3 &= ~CCIFG;  
					// Store new value in CCR
					value = TA0R + sTimer.timer0_A3_ticks; //timer0_A3_ticks_g;
					// Load CCR register with next capture point
					TA0CCR3 = value;   
					// Enable timer interrupt    
					TA0CCTL3 |= CCIE; 	
					// Call function handler
					fptr_Timer0_A3_function();
					break;
		
		// Timer0_A4	One-time delay			
		case 0x08:	// Disable IE 
					TA0CCTL4 &= ~CCIE;
					// Reset IRQ flag  
					TA0CCTL4 &= ~CCIFG;  
					// Set delay over flag
					sys.flag.delay_over = 1;
					break;
	}
	
	// Exit from LPM3 on RETI
	_BIC_SR_IRQ(LPM3_bits);               
}

