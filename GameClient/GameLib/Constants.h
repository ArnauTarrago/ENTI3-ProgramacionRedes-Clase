#pragma once

const int TIMER_RESEND_CRITICAL_PACKETS_DURING_LOW_TRAFFIC_IN_MILLISECONDS = 500;
const int TIMER_RESEND_CRITICAL_PACKETS_DURING_HIGH_TRAFFIC_IN_MILLISECONDS = 1000;
const int TIMER_SERVER_CHECK_FOR_CLIENT_INACTIVITY_WHILE_CONNECTED_IN_SECONDS = 60;
const int TIMER_SERVER_CHECK_FOR_CLIENT_INACTIVITY_DURING_CONNECTION_IN_SECONDS = 30;
const int TIMER_SERVER_SAVE_RTT_TO_FILE_IN_SECONDS = 10;
const int TIMER_SERVER_VALIDATE_PLAYER_MOVES_IN_MILLISECONDS = 100;
const int TIMER_CLIENT_CHECK_FOR_SERVER_INACTIVITY_WHILE_CONNECTED_IN_SECONDS = 60;
const int TIMER_CLIENT_CHECK_FOR_SERVER_INACTIVITY_DURING_CONNECTION_IN_SECONDS = 30;
const int TIMER_CLIENT_RESEND_HELLO_WHILE_CONNECTING_IN_MILLISECONDS = 5000;
const int TIMER_CLIENT_SEND_ACCUMULATED_MOVES_IN_MILLISECONDS = 100;
