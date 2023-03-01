// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : eport_demo.c
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef __EPORT_DEMO_H__
#define __EPORT_DEMO_H__

enum {
	gpio_intput_test = 1,
	gpio_output_test, 
	low_level_test, 
	high_level_test,
	raise_edge_test,
	fall_edge_test, 
	all_edge_test, 
	Register_test
};

extern void Eport_Demo(void);
#endif
