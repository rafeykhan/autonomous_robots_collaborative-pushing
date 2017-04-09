/* helloworld.c for TOPPERS/ATK(OSEK) */
#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"
#include <stdlib.h>

DeclareTask(OSEK_Task_Background);
DeclareCounter(SysTimerCnt);
DeclareTask(TaskSonar);
//static U8 bt_receive_buf[32];
#define RUNTIME_CONNECTION
unsigned char btFlag = 0;
int counter = 0;
int counter2 = 0;
int goFlag=0;

static U8 bt_send_buf[32] ;
//const U8 bd_addr[7] = {0x00, 0x16, 0x53, 0x18, 0x84, 0xC1, 0x00};

/* LEJOS OSEK hooks */
void ecrobot_device_initialize(){

	#ifndef RUNTIME_CONNECTION
		ecrobot_init_bt_slave("LEJOS-OSEK");
	#endif
		ecrobot_init_sonar_sensor(NXT_PORT_S1);
		ecrobot_init_sonar_sensor(NXT_PORT_S2);
		ecrobot_init_sonar_sensor(NXT_PORT_S3);
		ecrobot_init_sonar_sensor(NXT_PORT_S4);
	}
void ecrobot_device_terminate(){

		ecrobot_term_bt_connection();
		ecrobot_term_sonar_sensor(NXT_PORT_S1);
		ecrobot_init_sonar_sensor(NXT_PORT_S2);
		ecrobot_init_sonar_sensor(NXT_PORT_S3);
		ecrobot_term_sonar_sensor(NXT_PORT_S4);
	}
/* LEJOS OSEK hook to be invoked from an ISR in category 2 */

/* nxtOSEK hook to be invoked from an ISR in category 2 */

void disp(int row, char *str, int val)
{
#define DISPLAY_ON
#ifdef DISPLAY_ON
	//display_clear(0);
	display_goto_xy(0, row);
	display_string(str);
	display_int(val, 0);
	display_update();
#endif
}

/* LEJOS OSEK hook to be invoked from an ISR in category 2 */
void user_1ms_isr_type2(void)
{
  StatusType ercd;

  ercd = SignalCounter(SysTimerCnt); /* Increment OSEK Alarm Counter */
  if(ercd != E_OK)
  {
    ShutdownOS(ercd);
  }
}


void slowTurnLeft(void){
	nxt_motor_set_speed(NXT_PORT_B, -20, 1);
	nxt_motor_set_speed(NXT_PORT_A, 20, 1);
}

void slowTurnRight(void){
	nxt_motor_set_speed(NXT_PORT_B, 20, 1);
	nxt_motor_set_speed(NXT_PORT_A, -20, 1);
}


void motorReset(void){
	nxt_motor_set_count(NXT_PORT_A, 0);
	nxt_motor_set_count(NXT_PORT_B, 0);
}

void stopMotors(){
	nxt_motor_set_speed(NXT_PORT_A, 0, 1);
	nxt_motor_set_speed(NXT_PORT_B, 0, 1);
}


int revA = 0;
int revB = 0;
int RNa = 40;
int RNb = 40;
int dist = 0;

void forwardMotion(double tile){
RNa = 40;
RNb = 40;
dist += 650 * tile;

do {
	nxt_motor_set_speed(NXT_PORT_A, RNa, 1);
	nxt_motor_set_speed(NXT_PORT_B, RNb, 1);
	revA = nxt_motor_get_count(NXT_PORT_A);
	revB = nxt_motor_get_count(NXT_PORT_B);

//	disp(1, " REV_A: ", revA);	// Display Rotation Sensor
///	disp(2, " REV_B: ", revB);	// Display random number
//	disp(3, " Dist: ", dist);
//	disp(6, " RNa: ", RNa);
//	disp(7, " RNb: ", RNb);


	if ( RNb <25 || RNa <25)//if it become stupid, then reset it
	{
		RNa = 40;
		RNb = 40;

		nxt_motor_set_speed(NXT_PORT_B, RNb, 1);
		nxt_motor_set_speed(NXT_PORT_A, RNa, 1);
	}


	else if (revB  > revA){
			RNb--;
			nxt_motor_set_speed(NXT_PORT_B, RNb, 1);
			nxt_motor_set_speed(NXT_PORT_A, RNa, 1);
	}
	else if (revA  > revB){
			RNa--;
			nxt_motor_set_speed(NXT_PORT_B, RNb, 1);
			nxt_motor_set_speed(NXT_PORT_A, RNa, 1);

	}
//	systick_wait_ms(100);
} while( revB < dist ); //* dist);
stopMotors();

return;
}


int turn_value = 190;
void Turn(int left, int right){
	int RN = 25;
	int revAl =  0;
	int revBr =  0;
	revAl =  revA + turn_value;
	revBr =  revB + turn_value;

	if(right==1){
	while(revB <= revBr){//turn right
		nxt_motor_set_speed(NXT_PORT_B, RN, 1);
		nxt_motor_set_speed(NXT_PORT_A, -RN, 1);
		revB = nxt_motor_get_count(NXT_PORT_B);
		revA = nxt_motor_get_count(NXT_PORT_A);
		}
		//since we don't count the dist when turning,
		//we need to substract the wheel count after turning to keep it straight
		nxt_motor_set_count(NXT_PORT_B, revB - turn_value);
		nxt_motor_set_count(NXT_PORT_A,  revA + turn_value);
	}

	else if(left==1){//turn left
		while(revA <= revAl){
		nxt_motor_set_speed(NXT_PORT_A, RN, 1);
		nxt_motor_set_speed(NXT_PORT_B, -RN, 1);
		revB = nxt_motor_get_count(NXT_PORT_B);
		revA = nxt_motor_get_count(NXT_PORT_A);
		}
		//since we don't count the dist when turning,
		//we need to substract the wheel count after turning to keep it straight
		nxt_motor_set_count(NXT_PORT_B, revB + turn_value);
		nxt_motor_set_count(NXT_PORT_A,  revA - turn_value);
	}


	stopMotors();

	//systick_wait_ms(500);
}



/* Task1 executed every 50msec */
int sonar1 = 255;
int sonar2 = 0;
int sonar3 = 0;
int sonar4 = 255;

TASK(TaskSonar)
{
	sonar1 = ecrobot_get_sonar_sensor(NXT_PORT_S1);
	sonar2 = ecrobot_get_sonar_sensor(NXT_PORT_S2);
	sonar3 = ecrobot_get_sonar_sensor(NXT_PORT_S3);
	sonar4 = ecrobot_get_sonar_sensor(NXT_PORT_S4);
 	TerminateTask();
}



// void posCorrect(void){
//
// 	while(1000){
// 		//debugging sonar display
// 		rightCluster = (sonar1+sonar2)/2;
// 		leftCluster = (sonar3+sonar4)/2;
// 		disp(3,"[Sonar1]=               ",sonar1);
// 		disp(4,"[Sonar2]=               ",sonar2);
// 		disp(5,"[Sonar3]=               ",sonar3);
// 		disp(6,"[Sonar3]=               ",sonar3);
// 		disp(3,"[Sonar1]= ",sonar1);
// 		disp(4,"[Sonar2]= ",sonar2);
// 		disp(5,"[Sonar3]= ",sonar3);
// 		disp(6,"[Sonar3]= ",sonar4);
// 		disp(1,"[RghtClstr]=               ",0);
// 		disp(2,"[LftClstr]=                ",0);
// 		disp(1,"[RghtClstr]= ",rightCluster);
// 		disp(2,"[LftClstr]= ",leftCluster);
//
// 		if(rightCluster > leftCluster){
// 			slowTurnLeft();
// 		}
// 		else if (leftCluster > rightCluster ){
// 			slowTurnRight();
// 		}
// 		if((rightCluster == leftCluster+1 || rightCluster == leftCluster-1 ) && (rightCluster < 30) && (leftCluster < 30) ){
// 			stopMotors();
// 			//break;
// 		}
// 		counter++;
// 		}
// }


TASK(OSEK_Task_Background)
{
	systick_wait_ms(500);
	revA = nxt_motor_get_count(NXT_PORT_A);
	revB = nxt_motor_get_count(NXT_PORT_B);
	int tileLen = 650;

	int repeatDist = tileLen * 5;
	int stopDist = tileLen * 4;//4 tiles length
	int direction = 0;//which way to robot is heading toward or away the exit
	int rightCluster = 0;
	int leftCluster = 0;
	int counter = 0;
	int flag1=1;
	int flag2=1;
	int verticalDist = 650 * 10;
	int horizontalDist = 650 * 6;
	int hdist=0;
	motorReset();
//
//
//


			static SINT bt_status = BT_NO_INIT;
			static U8 bt_receive_buf[32];

		  // int counter1 = 0;
		  // int counter2 = 0;
		  int first, second,third,fourth;
			while(btFlag < 3){
				#ifdef RUNTIME_CONNECTION
				ecrobot_init_bt_slave("LEJOS-OSEK");
				#endif
				if (ecrobot_get_bt_status() == BT_STREAM && bt_status != BT_STREAM){
					disp(1,"BT[on]          ", 0);

					//bt_status = ecrobot_get_bt_status();
				}
		    bt_status = ecrobot_get_bt_status();

		    if((bt_status == BT_STREAM) && (btFlag == 0)){
		      ecrobot_read_bt_packet(bt_receive_buf, 32);
		      if(bt_receive_buf[0]  == 1){
		        first = (int)bt_receive_buf[1];
		        second = (int)bt_receive_buf[2];
						third = (int)bt_receive_buf[3];
		        fourth = (int)bt_receive_buf[4];
						disp(4,"",first);
						disp(5,"",second);
						disp(6,"",third);
						disp(7,"",fourth);
						hdist=(first*1000)+(second*100)+(third*10)+fourth;
						//disp(4,"hdist ",hdist);

						break;
		        btFlag++;
		      }
		    }
		    //review packet buffer
		    if((bt_status == BT_STREAM) && (btFlag == 1)){

		        bt_send_buf[0] = 2;
		        ecrobot_send_bt_packet(bt_send_buf, 32);
		        btFlag++;
		    }

		    if((bt_status == BT_STREAM) && (btFlag == 2)){
		      ecrobot_read_bt_packet(bt_receive_buf, 32);
		      if(bt_receive_buf[0]  == 2){
						disp(5, "PKG RECVD ", 0);
						btFlag++;
		      }
		      else
		        btFlag--;
		    }
				//disp(4, "In While ", 0);
			}
		disp(2, "hdist ", hdist);
		//disp(3, "hdist+ ", hdist+1100);
		hdist=hdist+1300;
		systick_wait_ms(500);
		while(nxt_motor_get_count(NXT_PORT_A) < hdist){
				forwardMotion(0.1);
		}
		stopMotors();
		systick_wait_ms(500);
		Turn(0,1);//turn right
		systick_wait_ms(500);
		while(true){
			if (sonar1<20 || sonar2<20){
				stopMotors();
				break;
			}
			else{
				forwardMotion(0.1);
			}
		}


			systick_wait_ms(500);
			while(counter < 1000){

				if(sonar1 > sonar2){
					slowTurnRight();
				}
				else if (sonar2 > sonar1){
					slowTurnLeft();
				}
				if((sonar1 == sonar2+1 || sonar1 == sonar2-1 ) && (sonar1< 30) && (sonar2 < 30) ){
					stopMotors();
					counter++;
				}
			}

			while(sonar1 > 10  || sonar2 > 10){
				nxt_motor_set_speed(NXT_PORT_A, 19, 1);
				nxt_motor_set_speed(NXT_PORT_B, 19, 1);
				counter=0;
			}
			stopMotors();
			while(counter < 1000){

				if(sonar1 > sonar2){
					slowTurnRight();
				}
				else if (sonar2 > sonar1){
					slowTurnLeft();
				}
				if((sonar1 == sonar2+1 || sonar1 == sonar2-1 ) && (sonar1< 30) && (sonar2 < 30) ){
					stopMotors();
					counter++;
					btFlag=0;
				}
			}
			systick_wait_ms(500);

			while(btFlag < 3){
				motorReset();
				#ifdef RUNTIME_CONNECTION
				ecrobot_init_bt_slave("LEJOS-OSEK");
				#endif
				if (ecrobot_get_bt_status() == BT_STREAM && bt_status != BT_STREAM){
					disp(1,"BT[on]          ", 0);
				}
		    bt_status = ecrobot_get_bt_status();
				if(goFlag==0){
		 		 while(	nxt_motor_get_count(NXT_PORT_A) < 100){
		 			 nxt_motor_set_speed(NXT_PORT_A, 18, 1);
		 			 nxt_motor_set_speed(NXT_PORT_B, 16, 1);
		 		 }
		 		 goFlag++;
		 		 btFlag=0;
		 	 }

		 	 if((bt_status == BT_STREAM) && (btFlag == 0)){
		 			 //disp(7,"Sent: 		",1);
		 			 bt_send_buf[0] = 1;
		 			 ecrobot_send_bt_packet(bt_send_buf, 32);
		 			 btFlag++;
		 		 }
		 	 if((bt_status == BT_STREAM) && (btFlag == 1)){
		 		 ecrobot_read_bt_packet(bt_receive_buf, 32);
		 		 if(bt_receive_buf[0]  == 1){
		 			 goFlag = 0;
		 		 }
		 		 else
		 			 btFlag--;
		 	 }
/*
		    if((bt_status == BT_STREAM) && (btFlag == 0)){
		      ecrobot_read_bt_packet(bt_receive_buf, 32);
		      if(bt_receive_buf[0]  == 1){
						while(	nxt_motor_get_count(NXT_PORT_A) < 100){
							nxt_motor_set_speed(NXT_PORT_A, 19, 1);
							nxt_motor_set_speed(NXT_PORT_B, 19, 1);
						}
					}
					else{
		        	btFlag++;
		      }
		    }
		    //review packet buffer
		    if((bt_status == BT_STREAM) && (btFlag == 1)){

		        bt_send_buf[0] = 1;
		        ecrobot_send_bt_packet(bt_send_buf, 32);
		        btFlag--;
		    }
*/
				//disp(4, "In While ", 0);
			}




	TerminateTask();
}
