#include <reg51.h>

sbit RS = P0^1;
sbit RW = P0^2;
sbit EN = P0^3;
sbit START_PAUSE = P3^2; 
sbit RESET = P3^3;        

#define LCD P2

unsigned char sec = 0;
unsigned char min = 0;
unsigned char hour = 0;
unsigned char count = 0;
bit timer_running = 0;    

void delay(unsigned char t) {
    int i, j;
    for(i = 0; i < t; i++)
        for(j = 0; j < 1275; j++);
}

void cmd(unsigned char a) {
    LCD = a;
    RS = 0;
    RW = 0;
    EN = 1;
    delay(2);
    EN = 0;
}

void dat(unsigned char b) {
    LCD = b;
    RS = 1;
    RW = 0;
    EN = 1;
    delay(2);
    EN = 0;    
}

void init() {
    cmd(0x38);
    cmd(0x0C);
    cmd(0x06);
    cmd(0x01);
}

void strn(unsigned char *str) {
    while(*str) {
        dat(*str++);
    }
}

void display(unsigned char num) {
    dat((num / 10) + '0');
    dat((num % 10) + '0');
}

void display_time() {
    cmd(0x80);
    display(hour);
    dat(':');
    display(min);
    dat(':');
    display(sec);
}

void timer_init() {
    TMOD = 0x01;
    TH0 = 0x3C;
    TL0 = 0xB0;
    IE = 0x82;
    TR0 = 0;  
}

void isr() interrupt 1 {
    TH0 = 0x3C;
    TL0 = 0xB0;
    
    if(timer_running) {
        count++;
        if(count >= 20) {
            count = 0;
            sec++;
            
            if(sec >= 60) {
                sec = 0;
                min++;
                
                if(min >= 60) {
                    min = 0;
                    hour++;
                    
                    if(hour >= 24) {
                        hour = 0;
                    }
                }
            }
            display_time();
        }
    }
}

void main() {
    bit start_prev = 1, reset_prev = 1;  
    
    init();
    strn("00:00:00");
    delay(100);
    timer_init();
    
    while(1) {
        
        if(START_PAUSE == 0 && start_prev == 1) {  
            delay(20);  
            if(START_PAUSE == 0) {
                timer_running = !timer_running;
                
                if(timer_running) {
                    TR0 = 1; 
                } else {
                    TR0 = 0; 
                }
                
                while(START_PAUSE == 0);  
                delay(20);  
            }
        }
        start_prev = START_PAUSE;
        
        
        if(RESET == 0 && reset_prev == 1) {  
            delay(20); 
            if(RESET == 0) {
                TR0 = 0; 
                timer_running = 0;
                sec = 0;
                min = 0;
                hour = 0;
                count = 0;
                display_time();
                
                while(RESET == 0);  
                delay(20);  
            }
        }
        reset_prev = RESET;
    }
}