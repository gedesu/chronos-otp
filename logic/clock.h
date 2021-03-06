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

#ifndef CLOCKTIMER_H_
#define CLOCKTIMER_H_

// *************************************************************************************************
// Defines section


// Converts between Truong Epoch and Unix Epoch :-)
// Truong Epoch starts at 01/01/2010 00:00:00 AM
#define SYSTEM_EPOCH_OFFSET					1262304000

// *************************************************************************************************
// Prototypes section
extern void reset_clock(void);
extern void sx_time(u8 line);
extern void mx_time(u8 line);
extern void clock_tick(void);
extern void clock_resync(void);
extern void display_time(u8 line, u8 update);


// *************************************************************************************************
// Global Variable section
struct time
{
	// System epoch time
	u32 	system_time;

	// THESE CALCULATED BASED ON system_time
	// DO NOT MANIPULATE THIS DIRECTLY
	u8		hour;
	u8		minute;
	u8 		second;
	
	// Flag to minimize display updates
	u8 		drawFlag;

	// Viewing style
	u8		line1ViewStyle;
	u8		line2ViewStyle;
		
	// Inactivity detection (exits set_value() function)
	u32 	last_activity;
	
	// offset of local time from UTC (=1: set time is UTC+1 =CET)
	s8		UTCoffset;
	
	// Force the resynchronization/recalculation of date and time
	// based on the system epoch time
	s8		force_resync;
};
extern struct time sTime;


#endif /*CLOCKTIMER_H_*/
