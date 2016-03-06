#include "sysproc.h"

char commands[NUM_TEST_PROCS][NUM_COMMANDS];

void kcd_proc(void) {
		int msg_sent = -1;
		int i;
		int j;
    msgbuf* message = NULL;
    //char commands[NUM_PROCS][max_command][max_lengthofcommand]; //process id, number of commands registered to a single id, max length of a command
		int num_commands = ARRAYSIZE(commands);
		char* toExecute;
		
    while(1) {
			for (i = 0; i < ARRAYSIZE(message->mtext); i++) {
				toExecute += message->mtext[i];
			}
			if (message->mtype == DEFAULT)
			{
				
				// Check if the command is registered
				if (num_commands > 0) {
					for (i = 0; i < NUM_TEST_PROCS; i++) {
						for (j = 0; j < NUM_COMMANDS; j++) {
							if (commands[i][j] == *toExecute) {
								printf("To Execute Command: %s\n", commands[i][j]);
								//TODO: IDK what to do here...how to execute this proc?
								message->mtype = DEFAULT;
								msg_sent = k_send_message(i, (void *)message);
								break;
							}
						}
					}
				}
			} else if(message->mtype == KCD_REG) {
				
				for (i = 0; i < NUM_COMMANDS; i++) {
					if (commands[i] == NULL) {
						commands[message->sender_id][i] = *toExecute;
					}
				}
		}
		if (msg_sent == -1) {
			k_release_memory_block((void*)message);
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
    }
    
    k_send_message(PID_CRT, envelope);


}

//checks if string is in WS hh:mm:ss format. 
BOOL check_format(char *str) {
    int i;
    for (i = 3; i < 10; i = i + 3) {
        if (str[i] == NULL || (str[i] < '0' || str[i] > '9' )||( str[i+1] < '0' || str[i+1] > '9') || (str[i+2] != ':' && i != 9))
            return FALSE;
    }
    return TRUE;
} 

void send_wall_clock_message(msgbuf *msg){ 
    //sends a delayed message to wall_clock
    msg = k_request_memory_block();
    msg->mtype = DEFAULT;
    msg->mtext[0] = ' ';
    k_delayed_send(PID_CLOCK, msg, 1); 

}


void wall_clock(void){
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
    message = k_request_memory_block();
    message->mtext[0] = '%';
    message->mtext[1] = 'W';
    message->mtype = KCD_REG;
    k_send_message(PID_KCD, message);

    message = k_request_memory_block();

    message->mtext[0] = '%';
    message->mtext[1] = 'W';
    message->mtext[2] = 'R';
    message->mtype = KCD_REG;
    k_send_message(PID_KCD, message);
    
    message = k_request_memory_block();

    message->mtext[0] = '%';
    message->mtext[1] = 'W';
    message->mtext[2] = 'T';
    message->mtype = KCD_REG;
    k_send_message(PID_KCD, message);
		
    message = k_request_memory_block();

    message->mtext[0] = '%';
    message->mtext[1] = 'W';
    message->mtext[2] = 'S';
    message->mtype = KCD_REG;
    k_send_message(PID_KCD, message);

    while(1){
        message = k_receive_message(&sender_id);
        
        //start the clock
        if (message->mtext[0] == 'W' && message->mtext[1] == NULL) {
            message->mtext[0] = ' ';
            clock_on = TRUE;
        }

        if (message != NULL && clock_on) { 
            //checks if msg has text and clock is on
            if (message->mtext[0] == ' ' || message->mtext[1] == NULL ) {
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
                k_release_memory_block((void*)message);
                send_wall_clock_message(message); //sends delayed message
                
                } else if (message->mtext[1] == 'R') { 
                    //resets clock
                    hour = 0;
                    minute = 0;
                    second = 0;
                    __disable_irq();
                    print_wall_clock(hour,minute,second);
                    __enable_irq();

                    //deallocate then create a new one.
                    k_release_memory_block((void *)message);

                } else if (message->mtext[1] == 'T') {
                    hour = 0;
                    minute = 0;
                    second = 0;
                    clock_on = FALSE;
                    k_release_memory_block((void*)message);

                } else if (message->mtext[1] == 'S' && check_format(message->mtext)) {
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
                    k_release_memory_block((void*)message);
            } else{ 
                //else prints out the message
                k_send_message(PID_CRT, msg);
            }
        } else { 
            //if message is null or clock is off, deallocates the message
            k_release_memory_block((void*)message);
        }
    }
}


