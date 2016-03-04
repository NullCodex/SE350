void UART_iprocess(void) {
    msgbuf * message = NULL;
    msgbuf * message_to_send = NULL;
    int sender_id;
    char char_read;

    while(1){
        message = receive_message(sender_id);
        char_read = message->mtext[0];
        k_release_memory_block(message);
        create new message?
        message_to_send->mtext[0] = char_read;
        send_message(KCD_PID , message);

    }
}



void crt_proc(void) {
    msgbuf* message;
    char* str;

    while(1) {
        message = receive_message(NULL);
        __disable_irq();
        str = message->mtext;
        printf("\n\r");
        printf("%s\n\r", str);
        str = NULL;

        k_release_memory_block(message);
        __enable_irq();
    }
}


void kcd_proc(void) {
    int sender_id;
    int count;
    char* str;
    msgbuf* message = NULL;
    char commands[NUM_PROCS][max_command][max_lengthofcommand]; //process id, number of commands registered to a single id, max length of a command
    int num_commands[NUM_PROCS];



    while(1) {
        message = receive_message(&sender_id);
        if (message->mtype = DEFAULT)
        {

        } else if(message->mtype == KCD_REG) {
            count = num_commands[sender_id];

            for (i = 0; i < max_lengthofcommand; i++) {
                if (msg->mtext[i] != NULL) {
                    commands[sender_id][count][i] = messsage->mtext[i];
                }
                else {
                    commands[sender_id][count][i] = '\n';
                    break;
                }

            }
            if (i == max_lengthofcommand) {
                commands[sender_id][count][i] = '\n';
            }
            num_commands[sender_id] = count + 1;
            k_release_memory_block(message);
        }
    }
}