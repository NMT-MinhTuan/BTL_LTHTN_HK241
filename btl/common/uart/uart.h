#ifndef UART_H
#define UART_H

typedef void (*func_ptr)(char *);
void module_sim(void);
void call_back_uart(void* f);
void make_call(const char *phone_number);
void send_sms(const char *phone_number, const char *message);
#endif