#include <cstddef>
#include <array>

#include "types.hpp"
#include "keyboard.hpp"
#include "kernel_utils.hpp"
#include "console.hpp"
#include "shell.hpp"
#include "timer.hpp"
#include "utils.hpp"
#include "memory.hpp"
#include "ata.hpp"
 
namespace {

void reboot_command(const char* params);
void help_command(const char* params);
void uptime_command(const char* params);
void clear_command(const char* params);
void date_command(const char* params);
void sleep_command(const char* params);
void echo_command(const char* params);
void mmap_command(const char* params);
void memory_command(const char* params);
void disks_command(const char* params);

struct command_definition {
    const char* name;
    void (*function)(const char*);
};

std::array<command_definition, 10> commands = {{
    {"reboot", reboot_command},
    {"help", help_command},
    {"uptime", uptime_command},
    {"clear", clear_command},
    {"date", date_command},
    {"sleep", sleep_command},
    {"echo", echo_command},
    {"mmap", mmap_command},
    {"memory", memory_command},
    {"disks", disks_command},
}};

std::size_t current_input_length = 0;
char current_input[50];

void exec_command();

#define KEY_ENTER 0x1C
#define KEY_BACKSPACE 0x0E

void keyboard_handler(){
    uint8_t key = in_byte(0x60);

    if(key & 0x80){
    } else {
        if(key == KEY_ENTER){
            current_input[current_input_length] = '\0';

            k_print_line();

            exec_command();
            if(get_column() != 0){
                set_column(0);
                set_line(get_line() + 1);
                } 
            current_input_length = 0;

            k_print("root@balecok # ");
        } else if(key == KEY_BACKSPACE){
            if(current_input_length > 0){
                set_column(get_column() - 1);
                k_print(' ');
                set_column(get_column() - 1);

                --current_input_length;
            }
        } else {
           auto qwerty_key = key_to_ascii(key);

           if(qwerty_key > 0){
               current_input[current_input_length++] = qwerty_key;
               k_print(qwerty_key);
           }
        }
    }
}

void exec_command(){
    char buffer[50];

    for(auto& command : commands){
        const char* input_command = current_input;
        if(str_contains(current_input, ' ')){
            str_copy(current_input, buffer);
            input_command = str_until(buffer, ' ');
        }

        if(str_equals(input_command, command.name)){
            command.function(current_input);

            return;
        }
    }

    k_printf("The command \"%s\" does not exist\n", current_input);
}

void clear_command(const char*){
    wipeout();
}

void reboot_command(const char*){
    interrupt<60>();
}

void help_command(const char*){
    k_print("Available commands:\n");

    for(auto& command : commands){
        k_print('\t');
        k_print_line(command.name);
    }
}

void uptime_command(const char*){
    k_printf("Uptime: %ds\n", timer_seconds());
}

#define CURRENT_YEAR        2013
#define century_register    0x00
#define cmos_address        0x70
#define cmos_data           0x71

int get_update_in_progress_flag() {
      out_byte(cmos_address, 0x0A);
      return (in_byte(cmos_data) & 0x80);
}

uint8_t get_RTC_register(int reg) {
      out_byte(cmos_address, reg);
      return in_byte(cmos_data);
}

void date_command(const char*){
    std::size_t second;
    std::size_t minute;
    std::size_t hour;
    std::size_t day;
    std::size_t month;
    std::size_t year;

    std::size_t last_second;
    std::size_t last_minute;
    std::size_t last_hour;
    std::size_t last_day;
    std::size_t last_month;
    std::size_t last_year;
    std::size_t registerB;

    while (get_update_in_progress_flag()){};                

    second = get_RTC_register(0x00);
    minute = get_RTC_register(0x02);
    hour = get_RTC_register(0x04);
    day = get_RTC_register(0x07);
    month = get_RTC_register(0x08);
    year = get_RTC_register(0x09);

    do {
        last_second = second;
        last_minute = minute;
        last_hour = hour;
        last_day = day;
        last_month = month;
        last_year = year;

        while (get_update_in_progress_flag()){};         

        second = get_RTC_register(0x00);
        minute = get_RTC_register(0x02);
        hour = get_RTC_register(0x04);
        day = get_RTC_register(0x07);
        month = get_RTC_register(0x08);
        year = get_RTC_register(0x09);
    } while( (last_second != second) || (last_minute != minute) || (last_hour != hour) ||
        (last_day != day) || (last_month != month) || (last_year != year) );

    registerB = get_RTC_register(0x0B);


    if (!(registerB & 0x04)) {
        second = (second & 0x0F) + ((second / 16) * 10);
        minute = (minute & 0x0F) + ((minute / 16) * 10);
        hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
        day = (day & 0x0F) + ((day / 16) * 10);
        month = (month & 0x0F) + ((month / 16) * 10);
        year = (year & 0x0F) + ((year / 16) * 10);

    }

    if (!(registerB & 0x02) && (hour & 0x80)) {
        hour = ((hour & 0x7F) + 12) % 24;
    }
    year += (CURRENT_YEAR / 100) * 100;
    if(year < CURRENT_YEAR){
        year += 100;
    }

    k_printf("%d.%d.%d %d:%.2d:%.2d\n", day, month, year, hour, minute, second);
}


void sleep_command(const char* params){
    const char* delay_str = params + 6;
    sleep_ms(parse(delay_str) * 1000);
}

void echo_command(const char* params){
    k_print_line(params + 5);
}

void mmap_command(const char*){
    if(mmap_failed()){
        k_print_line("The mmap was not correctly loaded from e820");
    } else {
        k_printf("There are %d mmap entry\n", mmap_entry_count());
        k_print_line("Base         End          Size                  Type");
        for(std::size_t i = 0; i < mmap_entry_count(); ++i){
            auto& entry = mmap_entry(i);

            k_printf("%.10h %.10h %.10h %8m %s\n",
                entry.base, entry.base + entry.size, entry.size, entry.size, str_e820_type(entry.type));
        }
    }
}


void memory_command(const char*){
    if(mmap_failed()){
        k_print_line("The mmap was not correctly loaded from e820");
    } else {
        k_printf("Total available memory: %m\n", available_memory());
        k_printf("Total used memory: %m\n", used_memory());
        k_printf("Total free memory: %m\n", free_memory());
    }
    uint16_t* buffer = reinterpret_cast<uint16_t*>(k_malloc(512));

    if(!ata_read_sectors(drive(0), 2048, 1, buffer)){
        k_print_line("Read failed");
    } else {
        for(int i = 0; i < 80; i += 8){
            k_printf("%h %h %h %h %h %h %h %h\n",
                buffer[i+0], buffer[i+1], buffer[i+2], buffer[i+3],
                buffer[i+4], buffer[i+5], buffer[i+6], buffer[i+7]);
        }

        k_free(reinterpret_cast<std::size_t*>(buffer));
    }
}

void disks_command(const char*){
    k_print_line("Controller   Drive    Present");

    for(std::size_t i = 0; i < number_of_disks(); ++i){
        auto& descriptor = drive(i);

        k_printf("%12h %8h %s\n", descriptor.controller, descriptor.drive, descriptor.present ? "Yes" : "No");
    }
}

}
void init_shell(){
    current_input_length = 0;

    clear_command(0);

    k_print("root@balecok # ");

    register_irq_handler<1>(keyboard_handler);
}
