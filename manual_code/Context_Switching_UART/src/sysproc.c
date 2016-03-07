#include "sysproc.h"
#include "rtx.h"
#include "uart_polling.h"

REG_CMD sys_cmd[MAX_COMMANDS];
int cmd_index = 0;

PCB *clock_process;
int is_prefix (char str1[], char str2[]) {
	int i = 0;
	char txt = str1[i];
	while (i < COMMAND_SIZE && txt != '\0') {
		if (txt != str2[i]) {
			return -1;
		}
		i++;
		txt = str1[i];
	}
	return 0;
}

int check_cmd (char str[], int sender_id) {
	int exists = -1;
	int i;
	for (i = 0; i < MAX_COMMANDS; i++) {
		if (is_prefix(sys_cmd[i].cmd_str, str) == 0) {
			return sys_cmd[i].sender_id;
		}
	}
	return -1;
}

void insert_cmd (char str[], int sender_id) {
	
	if( cmd_index < MAX_COMMANDS ) {
		strcpy(sys_cmd[cmd_index].cmd_str, str);
		sys_cmd[cmd_index].sender_id = sender_id;
		cmd_index++;
	}
}

void kcd_proc(void) {
		int msg_sent = -1;
		int i;
		int exists = -1;
		int sender_id;
		char txt;
    msgbuf* message = NULL;
		char cmd[COMMAND_SIZE];
		
    while(1) {
			message = (msgbuf*)receive_message(&sender_id);
			for (i = 0; i < COMMAND_SIZE; i++) {
				txt = message->mtext[i];
				if (txt == '\0') {
					break;
				}
				cmd[i] = txt;
			}
			 
			if (message->mtype == DEFAULT)
			{
				// Check if the command is registered
				if (cmd_index > 0) {
					exists = check_cmd(cmd, sender_id);
					if (exists != -1) {
						message->mtype = DEFAULT;
						msg_sent = send_message(exists, (void *)message);
					}
					
				}
			} else if(message->mtype == KCD_REG) {
				insert_cmd(cmd, sender_id);
			}
			if (msg_sent == -1) {
				release_memory_block(message);
			}
		}
}

void print_wall_clock(int hour, int minute, int second){
    Envelope* envelope;
    msgbuf* message;
		int i;
    char str[9];

    str[0] = hour /10 + '0';
    str[1] = hour %10 + '0';
    str[2] = ':';
    str[3] = minute /10 + '0';
    str[4] = minute %10 + '0';
    str[5] = ':';
    str[6] = second /10 + '0';
    str[7] = second %10 + '0';
    
    for (i = 0; i < 8; i ++){
        message->mtext[i] = str[i];
				//printf("%c", str[i]);
				uart0_put_char(str[i]);
    }
		uart0_put_char('\n');
    //send_message(PID_CRT, envelope);


}

//checks if string is in WS hh:mm:ss format. 
BOOL check_format(char *str) {
    int i;
    //for (i = 3; i < 10; i = i + 3) {
    //    if (str[i] == NULL || (str[i] < '0' || str[i] > '9' )||( str[i+1] < '0' || str[i+1] > '9') || (str[i+2] != ':' && i != 9))
    //        return FALSE;
    //}
    return TRUE;
} 

void send_wall_clock_message(msgbuf *msg){ 
    //sends a delayed message to wall_clock
    msg = request_memory_block();
    msg->mtype = DEFAULT;
    msg->mtext[0] = ' ';
    delayed_send(PID_CLOCK, msg, 15); 

}


void wall_clock(void){
		char firstC;
    int sender_id;
    msgbuf* message;
    int hour = 0;
    int minute = 0;
    int second = 0;

    int temp = 0;
    int i = 0;
    BOOL clock_on = FALSE;
    msgbuf* msg;

    //registering to KCD
    message = request_memory_block();
    message->mtext[0] = '%';
    message->mtext[1] = 'W';
    message->mtype = KCD_REG;
    send_message(PID_KCD, message);

    message = request_memory_block();

    message->mtext[0] = '%';
    message->mtext[1] = 'W';
    message->mtext[2] = 'R';
    message->mtype = KCD_REG;
    send_message(PID_KCD, message);
    
    message = request_memory_block();

    message->mtext[0] = '%';
    message->mtext[1] = 'W';
    message->mtext[2] = 'T';
    message->mtype = KCD_REG;
    send_message(PID_KCD, message);
		
    message = request_memory_block();

    message->mtext[0] = '%';
    message->mtext[1] = 'W';
    message->mtext[2] = 'S';
    message->mtype = KCD_REG;
    send_message(PID_KCD, message); 

    while(1){
        message = (msgbuf*)receive_message(&sender_id);
        
        //start the clock
			
				printf("Clocklol: %c\n", message->mtext[0]);
				printf("Clocklol: %c\n", message->mtext[1]);
        if (message->mtext[1] == 'W' && message->mtext[2] == '\0') {
            message->mtext[1] = ' ';
            clock_on = TRUE;
        }

        if (message != NULL && clock_on) { 
            //checks if msg has text and clock is on
            if (message->mtext[1] == ' ' || message->mtext[2] == '\0' ) {
                second++;
                if (second >= 60){
                    minute ++;
                    second = second % 60;
                }
                if (minute >= 60){
                    hour = (hour + 1) % 24;
                    minute = minute % 60;
                }
                
                __disable_irq();
                print_wall_clock(hour,minute,second);
                __enable_irq();
        //        release_memory_block((void*)message);
                send_wall_clock_message(message); //sends delayed message
                
                } else if (message->mtext[2] == 'R') { 
                    //resets clock
                    hour = 0;
                    minute = 0;
                    second = 0;
                    __disable_irq();
                    print_wall_clock(hour,minute,second);
                    __enable_irq();

                    //deallocate then create a new one.
       ///             release_memory_block((void *)message);

                } else if (message->mtext[2] == 'T') {
                    hour = 0;
                    minute = 0;
                    second = 0;
                    clock_on = FALSE;
      //              release_memory_block((void*)message);

                } else if (message->mtext[2] == 'S' && check_format(message->mtext)) {
                    for(i = 3; i < 10; i = i + 3) { 
                        temp = (message->mtext[i] - '0') * 10 + message->mtext[i + 1] - '0';
                        switch(i) {
                            case 3:
                                hour = temp % 24;
                                break;
                            case 6:
                                minute = temp;
                                break;
                            case 9:
                                second = temp;
                                break;
                        }
                    }
                    if (second >= 60) {
                         minute ++;
                         second = second % 60;
                    }
                    if (minute >= 60){
                                hour = (hour +1 ) % 24;
                                minute = minute % 60;
                    }
                    __disable_irq();
                    print_wall_clock(hour,minute,second);
                    __enable_irq();
       //             release_memory_block((void*)message);
            } else{ 
                //else prints out the message
                send_message(PID_CRT, msg);
            }
        } else { 
            //if message is null or clock is off, deallocates the message
      //      release_memory_block((void*)message);
        }
    }
}


