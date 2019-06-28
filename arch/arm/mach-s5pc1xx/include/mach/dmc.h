#ifndef __DMC_H__
#define __DMC_H__

struct s5pc110_dmc {
	unsigned int concontrol;
	unsigned int memcontrol;
	unsigned int memconfig0;
	unsigned int memconfig1;
	unsigned int directcmd;
	unsigned int prechconfig;
	unsigned int phycontrol0;
	unsigned int phycontrol1;
	unsigned char res0[0x8];
	unsigned int pwrdnconfig;
	unsigned char res1[0x4];
	unsigned int timingaref;
	unsigned int timingrow;
	unsigned int timingdata;
	unsigned int timingpower;
	unsigned int phystatus;
	unsigned char res2[0x4];
	unsigned int chip0status;
	unsigned int chip1status;
	unsigned int arefstatus;
	unsigned int mrstatus;
	unsigned int phytest0;
	unsigned int phytest1;
	unsigned int qoscontrol0;
	unsigned int qosconfig0;
	unsigned int qoscontrol1;
	unsigned int qosconfig1;
	unsigned int qoscontrol2;
	unsigned int qosconfig2;
	unsigned int qoscontrol3;
	unsigned int qosconfig3;
	unsigned int qoscontrol4;
	unsigned int qosconfig4;
	unsigned int qoscontrol5;
	unsigned int qosconfig5;
	unsigned int qoscontrol6;
	unsigned int qosconfig6;
	unsigned int qoscontrol7;
	unsigned int qosconfig7;
	unsigned int qoscontrol8;
	unsigned int qosconfig8;
	unsigned int qoscontrol9;
	unsigned int qosconfig9;
	unsigned int qoscontrol10;
	unsigned int qosconfig10;
	unsigned int qoscontrol11;
	unsigned int qosconfig11;
	unsigned int qoscontrol12;
	unsigned int qosconfig12;
	unsigned int qoscontrol13;
	unsigned int qosconfig13;
	unsigned int qoscontrol14;
	unsigned int qosconfig14;
	unsigned int qoscontrol15;
	unsigned int qosconfig15;
};

void mem_ctrl_init(int reset);

struct dmc_memconfigs {
	unsigned int dmc0memconfig0;
	unsigned int dmc0memconfig1;
	unsigned int dmc1memconfig0;
	unsigned int dmc1memconfig1;
};

#endif
