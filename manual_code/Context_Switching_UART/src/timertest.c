void timer_test_proc(void) {
    const uint32_t sample_size = 100;
    uint32_t start_time, end_time;
    uint32_t elapsed_time_send[sample_size];
    uint32_t elapsed_time_receive[sample_size];
    uint32_t elapsed_time_request[sample_size];
    uint32_t i = 0;
    uint32_t sender_id;
    msgbuf* msg;

    for (i = 0; i < sample_size; i++) {
        // Test request_memory_block
        start_time = *tc_count;
        msg = request_memory_block();
        end_time = *tc_count;
        elapsed_time_request[i] = (end_time - start_time);

        msg->mtype = DEFAULT;
        msg->mtext[0] = "A";

        // Test send message
        start_time = *tc_count;
        send_message(PID_TIMER_TEST_PROC, msg);
        end_time = *tc_count;
        elapsed_time_send[i] = (end_time - start_time);
        release_memory_block(envelope);

        // Test receive message
        start_time = *tc_count;
        envelope = (msg_buf_t*)receive_message(sender_id);
        end_time = *tc_count;
        elapsed_time_receive[i] = (end_time - start_time);
        release_memory_block(envelope);
    }

    // print everything :D
    printf("Request Memory Block Timings:\r\n");
    for (i = 0; i < sample_size; i++) {
        printf("%d\r\n", elapsed_time_request[i]);
    }
    printf("Send Message Timings:\r\n");
    for (i = 0; i < sample_size; i++) {
        printf("%d\r\n", elapsed_time_send[i]);
    }
    printf("Receive Message Timings: \r\n");
    for (i = 0; i < sample_size; i++) {
        printf("%d\r\n", elapsed_time_receive[i]);
    }
}