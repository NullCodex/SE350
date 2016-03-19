#include "sysproc.h"
#include "rtx.h"
#include "uart_polling.h"

REG_CMD sys_cmd[MAX_COMMANDS];
int cmd_index = 0;

PCB *clock_process;
PCB *priority_process;

char *strcpy(char *dest, const char *src)
 {
    char *save = dest;
    while(*dest++ = *src++);
    return save;
 }
 

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
			exists = - 1;
			message = (msgbuf*)receive_message(&sender_id);
			msg_sent = -1;
			i = 0;
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
    msgbuf* message = request_memory_block();
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
		str[8] = '\0';
    
    for (i = 0; i < 9; i ++){
        message->mtext[i] = str[i];
				//uart0_put_char(str[i]);
    }
		message->mtype = CRT_DISPLAY;
		//uart0_put_char('\n');
    send_message(PID_CRT, message);


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
    msg = (msgbuf*)request_memory_block();
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
                
                print_wall_clock(hour,minute,second);
                release_memory_block(message);
                send_wall_clock_message(message); //sends delayed message
                
                } else if (message->mtext[2] == 'R') { 
                    //resets clock
                    hour = 0;
                    minute = 0;
                    second = 0;
                    print_wall_clock(hour,minute,second);
                   
                    //deallocate then create a new one.
                    release_memory_block(message);

                } else if (message->mtext[2] == 'T') {
                    hour = 0;
                    minute = 0;
                    second = 0;
                    clock_on = FALSE;
                    release_memory_block(message);

                } else if (message->mtext[2] == 'S' && check_format(message->mtext)) {
                    for(i = 4; i < 11; i = i + 3) { 
                        temp = (message->mtext[i] - '0') * 10 + message->mtext[i + 1] - '0';
                        switch(i) {
                            case 4:
                                hour = temp % 24;
                                break;
                            case 7:
                                minute = temp;
                                break;
                            case 10:
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
                    print_wall_clock(hour,minute,second);
                   release_memory_block(message);
            } else{ 
                //else prints out the message
								msg->mtype = CRT_DISPLAY;
                send_message(PID_CRT, msg);
            }
        } else { 
            //if message is null or clock is off, deallocates the message
            release_memory_block(message);
        }
    }
}


void set_priority_process(void) {
    int sender_id;
    int process_id;
    int priority;
    int i = 3;
    int status = RTX_OK;
    BOOL isError = FALSE;
    msgbuf* msg;
		msgbuf* print_msg;

    // registers with the kcd
    msg = request_memory_block();
    msg->mtext[0] = '%';
    msg->mtext[1] = 'C';
		msg->mtext[2] = '\0';
    msg->mtype = KCD_REG;
    send_message(PID_KCD, msg);

    while(1) {
        msg = receive_message(&sender_id);

        if(msg->mtext[i] >= '0' && msg->mtext[i] <= '9') {
            if(msg->mtext[i+1] >= '0' && msg->mtext[i+1] <= '9') {
                process_id = (msg->mtext[i] - '0') * 10 + msg->mtext[i+1] - '0';
                i += 2;
            } else {
                process_id = (msg->mtext[i] - '0');
                i += 1;
            }
        } else {
            isError = TRUE;
        }

        if (!isError && msg->mtext[i] == ' ' && msg->mtext[i+1] >= '0' && msg->mtext[i+1] < (NUM_PRIORITIES + '0') && msg->mtext[i+2] == NULL) {
            priority = (msg->mtext[i+1] - '0');
        } else {
            isError = TRUE;
        }

        if(!isError) {
            status = set_process_priority(process_id, priority);
            if (status == RTX_ERR) {
                isError = TRUE;
            }
        }

        if (isError) {
						print_msg = request_memory_block();
						print_msg->mtype = CRT_DISPLAY;
						print_msg->mtext[0] = 'I';
						print_msg->mtext[1] = 'n';
						print_msg->mtext[2] = 'v';
						print_msg->mtext[3] = 'a';
						print_msg->mtext[4] = 'l';
						print_msg->mtext[5] = 'i';
						print_msg->mtext[6] = 'd';
						print_msg->mtext[7] = ' ';
						print_msg->mtext[8] = 'p';
						print_msg->mtext[9] = 'a';
						print_msg->mtext[10] = 'r';
						print_msg->mtext[11] = 'a';
						print_msg->mtext[12] = 'm';
						print_msg->mtext[13] = 'e';
						print_msg->mtext[14] = 't';
						print_msg->mtext[15] = 'e';
						print_msg->mtext[16] = 'r';
						print_msg->mtext[17] = 's';
						print_msg->mtext[18] = '\0';
						
						send_message(PID_CRT, print_msg);
        }

        isError = FALSE;
        i = 3;
        status = RTX_OK;
        release_memory_block(msg);
    }
}