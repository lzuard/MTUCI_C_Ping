//Декларация фунций работы с логом

#include <stdio.h>      // Стандартные функции ввода/вывода
#include <errno.h>      // Стандартные функции работы с ошибками
#include <windows.h>    // Функции Win32 api
#include <time.h>       // Функции получения времени


//Декларация фунций
// Функция открытия файла лога
int log_open_file(FILE* *log_file, char* params_log_path,char* params_address, int *program_error_code, int *log_error_code);
// Функция записи ошибки в файл логов
int log_write_error(FILE* log_file, int program_error_code, int* log_error_code);
// Функция записи результата работы программы в файл логов
int log_write_result(FILE* logfile,int *log_error_code, int result, char* host, ULONG time_ms, int packet_size, byte ttl);
// Процеура диагностики файла логов
void log_diagnostics(int log_error_code);
