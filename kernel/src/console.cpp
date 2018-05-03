#include <stdarg.h>

#include "console.hpp"
#include "types.hpp"

namespace {

long current_line = 0;
long current_column = 0;

enum vga_color {
    BLACK = 0,
    BLUE = 1,
    GREEN = 2,
    CYAN = 3,
    RED = 4,
    MAGENTA = 5,
    BROWN = 6,
    LIGHT_GREY = 7,
    DARK_GREY = 8,
    LIGHT_BLUE = 9,
    LIGHT_GREEN = 10,
    LIGHT_CYAN = 11,
    LIGHT_RED = 12,
    LIGHT_MAGENTA = 13,
    LIGHT_BROWN = 14,
    WHITE = 15,
};

uint8_t make_color(vga_color fg, vga_color bg){
    return fg | bg << 4;
}

uint16_t make_vga_entry(char c, uint8_t color){
    uint16_t c16 = c;
    uint16_t color16 = color;
    return c16 | color16 << 8;
}

}

void set_column(long column){
    current_column = column;
}

long get_column(){
    return current_column;
}

void set_line(long line){ 
    current_line = line; 
}

long get_line(){ 
    return current_line; 
}

void k_print_line(){
    current_column = 0;
    ++current_line;
}

void k_print_line(const char* string){
    k_print(string);

    current_column = 0;
    ++current_line;
}

uint64_t digits(uint64_t number){
    if(number < 10){
        return 1;
    }

    int i = 0;

    while(number != 0){
        number /= 10;
        ++i;
    }

    return i;
}

void k_print(uint64_t number){
    if(number == 0){
        k_print('0');
        return;
    }
    char buffer[20];
    int i = 0;

    while(number != 0){
        buffer[i++] = '0' + number % 10;
        number /= 10;
    }

    --i;

    for(; i >= 0; --i){
        k_print(buffer[i]);
    }
}

void k_print(char key){
    if(key == '\n'){
        ++current_line;
        current_column = 0;
    } else if(key == '\t'){
        k_print("    ");
    } else {
        uint16_t* vga_buffer = reinterpret_cast<uint16_t*>(0x0B8000);

        vga_buffer[current_line * 80 + current_column] = make_vga_entry(key, make_color(WHITE, BLACK));

        ++current_column;
    }
}


void k_print(const char* string){
    for(int i = 0; string[i] != 0; ++i){
        k_print(string[i]);
    }

}

void wipeout(){
    current_line = 0;
    current_column = 0;

    for(int line = 0; line < 25; ++line){
        for(uint64_t column = 0; column < 80; ++column){
            k_print(' ');
        }
    }

    current_line = 0;
    current_column = 0;
}

void k_printf(const char* fmt, ...){
    va_list va;
    va_start(va, fmt);

    char ch;

    while ((ch=*(fmt++))) {
        if(ch != '%'){
            k_print(ch);
        } else {
            ch = *(fmt++);
            
            uint64_t min_width = 0;
            while(ch >= '0' && ch <= '9'){
                min_width = 10 * min_width + (ch - '0');
                ch = *(fmt++);
            }
            uint64_t min_digits = 0;
            if(ch == '.'){
                ch = *(fmt++);

                while(ch >= '0' && ch <= '9'){
                    min_digits = 10 * min_digits + (ch - '0');
                    ch = *(fmt++);
                }
            }
            auto prev  = current_column;
            
            if(ch == 'd'){
                auto arg = va_arg(va, uint64_t);
                if(min_digits > 0){
                    auto d = digits(arg);
                    if(min_digits > d){
                        min_digits -= d;
                        while(min_digits > 0){
                            while(min_digits > 0){
                                k_print('0');
                                --min_digits;
                            }
                        }
                    }
                }
                k_print(arg);
            } else if(ch == 'h'){
                k_print("0x");

                uint8_t buffer[20];

                auto arg = va_arg(va, uint64_t);
                int i = 0;

                while(arg / 16 != 0){
                    buffer[i++] = arg % 16;
                    arg /= 16;
                }

                buffer[i] = arg;

                if(min_digits > 0 && min_digits > i){
                    min_digits -= i + 1;
                    while(min_digits > 0){
                        k_print('0');
                        --min_digits;
                    }
                }
                
                while(i >= 0){
                    uint8_t digit = buffer[i];

                    if(digit < 10){
                         k_print(static_cast<char>('0' + digit));
                    } else {
                        switch(digit){
                            case 10:
                                k_print('A');
                                break;
                            case 11:
                                k_print('B');
                                break;
                            case 12:
                                k_print('C');
                                break;
                            case 13:
                                k_print('D');
                                break;
                            case 14:
                                k_print('E');
                                break;
                            case 15:
                                k_print('F');
                                break;
                        }
                    }

                    --i;
                }
            } 
            else if(ch == 'm'){
                auto memory= va_arg(va, uint64_t);

                if(memory > 1024 * 1024 * 1024){
                    k_print(memory / (1024 * 1024 * 1024));
                    k_print("GiB");
                } else if(memory > 1024 * 1024){
                    k_print(memory / (1024 * 1024));
                    k_print("MiB");
                } else if(memory > 1024){
                    k_print(memory / 1024);
                    k_print("KiB");
                } else {
                    k_print(memory);
                    k_print("B");
                }
            }
            else if(ch == 's'){
                auto arg = va_arg(va, const char*);
                k_print(arg);
            }
            if(min_width > 0){
                auto width = current_column - prev;

                if(min_width > width){
                    min_width -= width;

                    while(min_width > 0){
                        k_print(' ');
                        --min_width;
                    }
                }
            }
        }
    }

    va_end(va);
}
