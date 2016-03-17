void A(void) //pid = 7
{
    msgbuf* msg;
    int num = 0;
    int sender_id;
    msg = request_memory_block();

    msg->mtype = KCD_REG;
    msg->mtext[0] = '%';
    msg->mtext[1] = 'Z';

    send_message(KCD_PID, msg);

    while(1){
        msg = receive_message(&sender_id);
        if(msg->mtext[0] == '%' && msg->mtext[1] == 'Z'){
            break;
        }
        release_memory_block(msg);
    }

    release_memory_block(msg);

    while(1) {
        msg = request_memory_block();
        msg->mtype = COUNT_REPORT;
        msg->mtext[0] = (char)num;
        send_message(8, msg);
        num = num + 1;
        release_processor();
    }

}

void B(void) //pid = 8
{
    msgbuf *msg;
    int sender_id;

    while(1){
        msg = receive_message(&sender_id);
        send_message(9, msg);
    }
}


void C(void) //pid == 9
{
    Queue q;
    int sender_id;
    msgbuf *msg;
    msgbuf *delay;
    msgbuf *receive;
    char print_msg[9] = {'P', 'r', 'o', 'c', 'e', 's', 's', ' ', 'C'};
    Element *element;
    int i = 0;

    q.first = NULL;
    q.last = NULL;

    while(1) {
        if(q.first == NULL){
            msg = receive_message(&sender_id);
        } else {
            element = pop(&q);
            msg = (msgbuf *)(element->data);
            element->data = NULL;
            release_memory_block(element);
        }
        if(msg->mtype == COUNT_REPORT && (int)(msg->mtext[0]) % 20 == 0){
            msg->mtype = DEFAULT;
            msg->mtext = print_msg;
            send_message(CRT_PID, msg);

            delay = request_memory_block();
            delay->mtype = wakeup10;
            delay->mtext[0] = NULL;
            delayed_send(9, delay, 10);
            while(1) {

                receive = receive_message(&sender_id);
                if(receive->mtype == wakeup10) {
                    release_memory_block(receive);
                    break;
                } else {
                        element = request_memory_block();
                        element->data = receive;
                        push(&q, element);
                }
            }


        } else {
            release_memory_block(msg);
        }
    }
}
