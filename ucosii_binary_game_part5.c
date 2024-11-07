#include <stdio.h>
#include <stdlib.h>
#include "includes.h"

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK	timer_task_stk[TASK_STACKSIZE];
OS_STK	io_poll_task_stk[TASK_STACKSIZE];
OS_STK	display_task_stk[TASK_STACKSIZE];
OS_STK	fsm_task_stk[TASK_STACKSIZE];
OS_STK	answer_task_stk[TASK_STACKSIZE];

/* Definition of States */
#define IDLE 0
#define PLAY 1
#define PAUSE 2
#define OFF 3
#define ON 4

/* Definition of Priorities */
#define TIMER_TASK_PRIORITY 2
#define DISPLAY_TASK_PRIORITY 5
#define IO_POLL_TASK_PRIORITY 1
#define FSM_TASK_PRIORITY 3
#define ANSWER_TASK_PRIORITY 4

/* Definition of Semaphores */
OS_EVENT *SemTimerState;
OS_EVENT *SemKeySW;
OS_EVENT *SemDisp;

/* Definition of Message Queues */
#define MSG_Q_SIZE 16
OS_EVENT *QuestionMsgQ;
OS_EVENT *ScoreMsgQ;
OS_EVENT *ElapsedSecondsMsgQ;
OS_EVENT *ElapsedMinutesMsgQ;
OS_EVENT *TimerMsgQ;
void *MsgStorage1[MSG_Q_SIZE];
void *MsgStorage2[MSG_Q_SIZE];
void *MsgStorage3[MSG_Q_SIZE];
void *MsgStorage4[MSG_Q_SIZE];
void *MsgStorage5[MSG_Q_SIZE];

/* Definition of Event Flags */
#define KEY0_PRESS 0x01
#define KEY1_PRESS 0x02
#define KEY2_PRESS 0x04
OS_FLAG_GRP *KeyFlags;

#define SW0_ON 0x01
#define SW1_ON 0x02
#define SW2_ON 0x04
#define SW3_ON 0x08
#define SW17_ON 0x10
OS_FLAG_GRP *SWFlags;

volatile int * KEY_ptr = (int*) PUSHBUTTONS_BASE;
volatile int * SW_switch_ptr = (int*) SLIDER_SWITCHES_BASE;
volatile int * LEDR_ptr = (int*) RED_LEDS_BASE;
volatile int * LEDG_ptr = (int*) GREEN_LEDS_BASE;
volatile char * LCD_display_ptr = (char*) CHAR_LCD_16X2_BASE;
volatile int * HEX_7_HEX_4_ptr = (int*) HEX7_HEX4_BASE;
volatile int * HEX_3_HEX_0_ptr = (int*) HEX3_HEX0_BASE;

int state = IDLE; // state of the timer
int power;
int KEY_value;
int SW_value;
int timer;
int time_elapsed_minutes;
int time_elapsed_seconds;
int key0, key1, key2 = 0;

unsigned char seven_seg_decode_table[] = {
        0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
        0x7F, 0x67, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};
char* binary_decode_table[] = {"0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111",
							   "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111"};
char* binary_operations[6] = {"AND", "NAND", "OR", "NOR", "XOR", "XNOR"};

int score;
unsigned int binary_num1;
unsigned int binary_num2;
unsigned int answer;
unsigned int question;
unsigned int binary_operation;


/* IO Polling Functions */

void read_key(void) {
	KEY_value = *(KEY_ptr) & 0x7;

	if (KEY_value == 1) {
		if (!key0) {
			OSFlagPost(KeyFlags, KEY0_PRESS, OS_FLAG_SET, &err);
			key0 = 1;
			*(LEDG_ptr) |= 0x1;
		}
	}
	else {
		key0 = 0;
		*(LEDG_ptr) &= ~0x1;
	}

	if (KEY_value == 2) {
		if(!key1) {
				OSFlagPost(KeyFlags, KEY1_PRESS, OS_FLAG_SET, &err);
				key1 = 1;
				*(LEDG_ptr) |= 0x2;
			}
	}
	else {
		key1 = 0;
		*(LEDG_ptr) &= ~0x2;
	}

	if (KEY_value == 4) {
		if(!key2) {
				OSFlagPost(KeyFlags, KEY2_PRESS, OS_FLAG_SET, &err);
				key2 = 1;
				*(LEDG_ptr) |= 0x4;
			}
	}
	else {
		key2 = 0;
		*(LEDG_ptr) &= ~0x4;
	}
}

void read_switch(void) {
	SW_value = *(SW_switch_ptr) & 0x2000F;
	*(LEDR_ptr) = SW_value;

	if(SW_value & 0x20000) { // SW17
			OSFlagPost(SWFlags, SW17_ON, OS_FLAG_SET, &err);
		} else {
			OSFlagPost(SWFlags, SW17_ON, OS_FLAG_CLR, &err);
		}

		if(SW_value & 0x1) { // SW0
			OSFlagPost(SWFlags, SW0_ON, OS_FLAG_SET, &err);
		} else {
			OSFlagPost(SWFlags, SW0_ON, OS_FLAG_CLR, &err);
		}

		if(SW_value & 0x2) { // SW1
			OSFlagPost(SWFlags, SW1_ON, OS_FLAG_SET, &err);
		} else {
			OSFlagPost(SWFlags, SW1_ON, OS_FLAG_CLR, &err);
		}

		if(SW_value & 0x4) { // SW2
			OSFlagPost(SWFlags, SW2_ON, OS_FLAG_SET, &err);
		} else {
			OSFlagPost(SWFlags, SW2_ON, OS_FLAG_CLR, &err);
		}

		if(SW_value & 0x8) { // SW3
			OSFlagPost(SWFlags, SW3_ON, OS_FLAG_SET, &err);
		} else {
			OSFlagPost(SWFlags, SW3_ON, OS_FLAG_CLR, &err);
		}
}


/* Display Functions */

void display_text(char * text_ptr) {
    while (*(text_ptr)) {
        *(LCD_display_ptr + 0x1) = *(text_ptr);
        text_ptr++;
    }
}

void cursor(int x, int y) {
    int move = x;
    if (y != 0) {
    	move |= 0x40;
    }
    move |= 0x80;
    *(LCD_display_ptr) = move;
    *(LCD_display_ptr) = 0x0C;
}

void display_time_and_score(int* local_score, int* local_timer) {
	if (local_timer != NULL) {
		int timer_d10 = *local_timer % 100 / 10;
		int timer_d1 = *local_timer % 10;
	}
	if (local_score != NULL) {
		int score_d10 = *local_score % 100 / 10;
		int score_d1 = *local_score % 10;
	}
	*(HEX_7_HEX_4_ptr) = (seven_seg_decode_table[score_d10] << 24) | (seven_seg_decode_table[score_d1] << 16) | (seven_seg_decode_table[timer_d10] << 8) | (seven_seg_decode_table[timer_d1]);
}

void display_time_elapsed(int* local_elapsed_seconds, int* local_elapsed_minutes) {
	if (local_elapsed_minutes != NULL) {
		int minutes_elapsed_d10 = time_elapsed_minutes % 100 / 10;
		int minutes_elapsed_d1 = time_elapsed_minutes % 10;
	}
	if (local_elapsed_seconds != NULL) {
		int seconds_elapsed_d10 = time_elapsed_seconds % 100 / 10;
		int seconds_elapsed_d1 = time_elapsed_seconds % 10;
	}
	*(HEX_3_HEX_0_ptr) = (seven_seg_decode_table[minutes_elapsed_d10] << 24) | (seven_seg_decode_table[minutes_elapsed_d1] << 16) | (seven_seg_decode_table[seconds_elapsed_d10] << 8) | (seven_seg_decode_table[seconds_elapsed_d1]);
}

void display_question(int* local_num1, int* local_num2, int* local_operation) {
	*(LCD_display_ptr) = 1;
	cursor(0,0);
	if (local_num1 != NULL) {
		display_text(binary_decode_table[*local_num1]);
	}
	cursor(0,1);
	if (local_num2 != NULL) {
		display_text(binary_decode_table[*local_num2]);
	}
	cursor(12,1);
	if (local_operation != NULL) {
		display_text(binary_operations[*local_operation]);
	}
}

void generate_question(void) {
	binary_num1 = rand() % 0xF;
	binary_num2 = rand() % 0xF;

	binary_operation = rand() % 0x6;

	OSQPost(QuestionMsgQ, (void *)&binary_num1);
	OSQPost(QuestionMsgQ, (void *)&binary_num2);
	OSQPost(QuestionMsgQ, (void *)&binary_operation);

	if (binary_operation == 0) {
		answer = binary_num1 & binary_num2;
	}
	else if (binary_operation == 1) {
		answer = (~(binary_num1 & binary_num2)) & 0xF;
	}
	else if (binary_operation == 2) {
		answer = binary_num1 | binary_num2;
	}
	else if (binary_operation == 3) {
		answer = (~(binary_num1 | binary_num2)) & 0xF;
	}
	else if (binary_operation == 4) {
		answer = binary_num1 ^ binary_num2;
	}
	else if (binary_operation == 5) {
		answer = (~(binary_num1 ^ binary_num2)) & 0xF;
	}
}


/* Answer Functions */

void check_answer(void) {
	OS_FLAGS sw_value;
	sw_value = OSFlagAccept(SWFlags, SW0_ON + SW1_ON + SW2_ON + SW3_ON, OS_FLAG_WAIT_SET_ALL, &err);
	if (sw_value == answer) {
		score++;
	}
}


/* Task Functions */

void fsm_task(void) {
	INT8U err;
	INT16U value;

	while (1) {
		value = OSSemAccept(SemKeySW);
		if (value > 0) {
			OSSemPend(SemTimerState, 0, &err);
		}

		if (OSFlagAccept(SWFlags, SW17_ON, OS_FLAG_WAIT_CLR_ALL, &err)) {
			power = ON;
		}
		else {
			power = OFF;
		}

		if (power == ON) {
			if (question == 10) {
				state = PAUSE;
			}

			if((state == IDLE) && (OSFlagAccept(KeyFlags, KEY1_PRESS, OS_FLAG_WAIT_SET_ALL + OS_FLAG_CONSUME, &err))) {
				state = PLAY;
			} else if (state == PLAY && (OSFlagAccept(KeyFlags, KEY1_PRESS, OS_FLAG_WAIT_SET_ALL + OS_FLAG_CONSUME, &err))) {
				state = PAUSE;
			} else if (state == PAUSE && (OSFlagAccept(KeyFlags, KEY1_PRESS, OS_FLAG_WAIT_SET_ALL + OS_FLAG_CONSUME, &err)) && (question < 10)) {
				state = PLAY;
			} else if (state == PAUSE && (OSFlagAccept(KeyFlags, KEY0_PRESS, OS_FLAG_WAIT_SET_ALL + OS_FLAG_CONSUME, &err))) {
				state = IDLE;
			}
		} else { // If power is off, state is idle
			state = IDLE;
		}

		OSSemPost(SemTimerState);
		OSTimeDly(1); // Delay by 1 ticks (125 ms)
	}
}

void io_poll_task(void) {
	while (1) {
		read_switch();
		read_key();

		OSSemPost(SemKeySW);
		OSTimeDly(1); // Delay by 1 tick (125 ms)
	}
}

void display_task(void) {
	INT8U err;

	while (1) {
		OSSemPend(SemDisp, 0, &err);

		if (power == OFF) {
			*(HEX_7_HEX_4_ptr) = 0;
			*(HEX_3_HEX_0_ptr) = 0;
		}
		else if (question == 10) {
			int* local_score = (int *)OSQAccept(ScoreMsgQ, &err);
			int* local_timer = (int *)OSQAccept(TimerMsgQ, &err);
			int* local_elapsed_seconds = (int *)OSQAccept(ElapsedSecondsMsgQ, &err);
			int* local_elapsed_minutes = (int *)OSQAccept(ElapsedMinutesMsgQ, &err);
			display_time_and_score(local_score, local_timer);
			display_time_elapsed(local_elapsed_seconds, local_elapsed_minutes);
		}
		else {
			int* local_score = (int *)OSQAccept(ScoreMsgQ, &err);
			int* local_timer = (int *)OSQAccept(TimerMsgQ, &err);
			display_time_and_score(local_score, local_timer);
			*(HEX_3_HEX_0_ptr) = 0;
		}

		if ((power == OFF) || (state == PAUSE) || (state == IDLE)) {
			*(LCD_display_ptr) = 1;
		}
		else {
			int* num1;
			int* num2;
			int* operation;

			int* local_num1 = (int *)OSQAccept(QuestionMsgQ, &err);
			if(local_num1 != NULL){
				num1 = local_num1;
			}

			int* local_num2 = (int *)OSQAccept(QuestionMsgQ, &err);
			if(local_num2 != NULL) {
				num2 = local_num2;
			}

			int* local_operation = (int *)OSQAccept(QuestionMsgQ, &err);
			if(local_operation != NULL) {
				operation = local_operation;
			}

			display_question(num1, num2, operation);
		}

		OSSemPost(SemDisp);
		OSTimeDly(2); // Delay by 2 ticks (250 ms)
	}
}

void answer_task(void) {
	INT8U err;
	INT16U value;

	while (1) {
		value = OSSemAccept(SemKeySW);
		if (value > 0) {
			OSSemPend(SemTimerState, 0, &err);
		}

		if (state == IDLE) {
			question = 0;
			score = 0;
		}

		if (OSFlagAccept(KeyFlags, KEY2_PRESS, OS_FLAG_WAIT_SET_ALL + OS_FLAG_CONSUME, &err)) {
			if (state == PLAY) {
				check_answer();
				question++;
				timer = 10;
				OSQPost(TimerMsgQ, (void *)&timer);
				generate_question();
			}
			key2 = 0;
		}

		OSQPost(TimerMsgQ, (void *)&timer);
		OSQPost(ScoreMsgQ, (void *)&score);
		OSSemPost(SemTimerState);
		OSTimeDly(1); // Delay by 1 tick (125 ms)
	}
}

void timer_task(void) {
	INT8U err;

	while (1) {
		OSSemPend(SemTimerState, 0, &err);

		if (state == IDLE || power == OFF) {
			timer = 10;
			time_elapsed_minutes = 0;
			time_elapsed_seconds = 0;
		}
		else if (state == PLAY) {
			if (timer <= 1) {
				timer = 10;
				question++;
				generate_question();
			}
			else {
				timer--;
			}

			if (time_elapsed_seconds >= 59) {
				if (time_elapsed_minutes >= 59) {
					time_elapsed_minutes = 0;
				}
				else {
					time_elapsed_minutes++;
				}
				time_elapsed_seconds = 0;
			}
			else {
				time_elapsed_seconds++;
			}
		}

		OSQPost(TimerMsgQ, (void *)&timer);
		OSQPost(ElapsedSecondsMsgQ, (void *)&time_elapsed_seconds);
		OSQPost(ElapsedMinutesMsgQ, (void *)&time_elapsed_minutes);

		OSSemPost(SemTimerState);
		OSTimeDly(8); // Delay by 8 ticks (1s)
	}
}

/* The main function creates two task and starts multi-tasking */
int main(void)
{

  OSTaskCreateExt(timer_task,
                      NULL,
                      (void *)&timer_task_stk[TASK_STACKSIZE-1],
                      TIMER_TASK_PRIORITY,
                      TIMER_TASK_PRIORITY,
                      timer_task_stk,
                      TASK_STACKSIZE,
                      NULL,
                      0);

  OSTaskCreateExt(io_poll_task,
                      NULL,
                      (void *)&io_poll_task_stk[TASK_STACKSIZE-1],
                      IO_POLL_TASK_PRIORITY,
                      IO_POLL_TASK_PRIORITY,
                      io_poll_task_stk,
                      TASK_STACKSIZE,
                      NULL,
                      0);

  OSTaskCreateExt(display_task,
                      NULL,
                      (void *)&display_task_stk[TASK_STACKSIZE-1],
                      DISPLAY_TASK_PRIORITY,
                      DISPLAY_TASK_PRIORITY,
                      display_task_stk,
                      TASK_STACKSIZE,
                      NULL,
                      0);

  OSTaskCreateExt(fsm_task,
                      NULL,
                      (void *)&fsm_task_stk[TASK_STACKSIZE-1],
                      FSM_TASK_PRIORITY,
                      FSM_TASK_PRIORITY,
                      fsm_task_stk,
                      TASK_STACKSIZE,
                      NULL,
                      0);

  OSTaskCreateExt(answer_task,
                      NULL,
                      (void *)&answer_task_stk[TASK_STACKSIZE-1],
                      ANSWER_TASK_PRIORITY,
                      ANSWER_TASK_PRIORITY,
                      answer_task_stk,
                      TASK_STACKSIZE,
                      NULL,
                      0);

  SemTimerState = OSSemCreate(1);
  SemKeySW = OSSemCreate(0);
  SemDisp = OSSemCreate(1);

  ElapsedSecondsMsgQ = OSQCreate(&MsgStorage1[0], MSG_Q_SIZE);
  ElapsedMinutesMsgQ = OSQCreate(&MsgStorage2[0], MSG_Q_SIZE);
  TimerMsgQ = OSQCreate(&MsgStorage3[0], MSG_Q_SIZE);
  ScoreMsgQ = OSQCreate(&MsgStorage4[0], MSG_Q_SIZE);
  QuestionMsgQ = OSQCreate(&MsgStorage5[0], MSG_Q_SIZE);

  KeyFlags = OSFlagCreate(0x00, &err);
  SWFlags = OSFlagCreate(0x00, &err);

  OSStart();
  return 0;
}
