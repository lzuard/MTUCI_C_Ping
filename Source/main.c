//Главный файл проекта, содержит функцию main

//Подключение обязательного файла с декларациями переменных
#include "../Headers/declarations.h"

// Процедура main
void main(int argc, char *argv[])
{
    //Инициализация переменных
    params_address = NULL;  // Адрес назначения, полученный от пользователя
    params_log_path = NULL; // Полный путь к файлу лога, полученный от пользователя
    program_error_code = 0; // Код ошибки программы
    log_error_code = 0;     // Код ошибки лога
    log_file = NULL;        // Указатель на файл лога
    ttl = 128;              // TTL отправляемых пакетов
    ping_socket = 0;        // Cокет для отправки и приема пакетов
    packets_sent = 0;       // Количество отправленных пакетов
    max_packets_sent = 4;   // Максимальное количество пакетов для отправки
    start_time_ms = 0;      // Время отправки эхо-запроса
    packet_size = 32;       // Стандартный размер ICMP пакета в байтах
    bytes_sent = 0;         // Колличество отправленных байт
    receive_result = 0;     // Результат расшифровки полученного ICMP пакета

    //Инициализация структуры
    recv_buf = malloc(sizeof(struct IPHeader));  // Буфер для поулчаемого заголовка IP

    //Проверка входных параметров и запись в переменные
    switch(u_check_params(argc, argv, &params_address, &params_log_path))
    {
        case 1: //Введены неверные параметры
            program_error_code=-1;
            u_stop_program(program_error_code, log_error_code, &log_file);
            break;
        case 0:
            //Открываем файл лога
            switch(log_open_file(&log_file,params_log_path, params_address, &program_error_code,&log_error_code))
            {
                case 1: //Ошибка открытия файла
                    log_diagnostics(log_error_code);
                    u_stop_program(program_error_code, log_error_code, &log_file);
                    break;
                case 0:
                    //Проверяем тип адреса хоста
                    switch(nw_check_host(params_address,ttl,&dest_addr,&wsaData,&ping_socket, &program_error_code))
                    {
                        case 2: //Введен неверный хост
                            if(log_write_error(log_file,program_error_code,&log_error_code)!=0)
                            {
                                log_diagnostics(log_error_code);
                            }
                            u_stop_program(program_error_code, log_error_code, &log_file);
                            break;
                        case 1: //Адрес хоста - доменное имя
                            //Пытаемся получить Ip адрес из доменного имени
                            switch(nw_get_ip(params_address,&dest_addr,&program_error_code))
                            {
                                case 1: //Произошла ошибка
                                    if(log_write_error(log_file,program_error_code,&log_error_code)!=0)
                                    {
                                        log_diagnostics(log_error_code);
                                    }
                                    u_stop_program(program_error_code, log_error_code, &log_file);
                                    break;
                                case 0: //Ip Получен
                                    //Цикл для отправки нескольких пакетов
                                    while (packets_sent<max_packets_sent)
                                    {
                                        //Фиксируем время отправки пакета
                                        start_time_ms=u_get_cur_time_ms();
                                        //Отправляем пакет
                                        switch (nw_send_request(ping_socket, dest_addr, send_buf, packet_size,&program_error_code, &bytes_sent))
                                        {
                                            case 1: //Ошибка отправки
                                                if(log_write_error(log_file,program_error_code,&log_error_code)!=0)
                                                {
                                                    log_diagnostics(log_error_code);
                                                }
                                                u_stop_program(program_error_code, log_error_code, &log_file);
                                                break;
                                            case 0: //Пакет отправлен
                                                //Увеличиваем счетчик отправленных пакетов
                                                packets_sent++;
                                                //Пытаемся получить ответ
                                                switch(nw_get_reply(ping_socket,dest_addr,recv_buf,packet_size,&program_error_code, &receive_result))
                                                {
                                                    case 0: //Ответ получен
                                                        //Выводим результат получения в консоль
                                                        u_show_result(receive_result, inet_ntoa(dest_addr.sin_addr), u_get_cur_time_ms() - start_time_ms, packet_size, recv_buf->ttl);
                                                        //Пытаемся вывести результат получения в лог
                                                        if(log_write_result(log_file,&log_error_code, receive_result, inet_ntoa(dest_addr.sin_addr), u_get_cur_time_ms() - start_time_ms, packet_size, recv_buf->ttl) != 0)
                                                        {
                                                            log_diagnostics(log_error_code);
                                                            u_stop_program(program_error_code, log_error_code, &log_file);
                                                        }
                                                        break;
                                                    default://Ошибка получения пакета
                                                        if(log_write_error(log_file,program_error_code,&log_error_code)!=0)
                                                        {
                                                            log_diagnostics(log_error_code);
                                                        }
                                                        u_stop_program(program_error_code, log_error_code, &log_file);
                                                        break;
                                                }
                                                break;
                                        }
                                    }
                            }
                            break;
                        case 0: //Адрес хоста - Ip адрес
                            //Цикл для отправки нескольких пакетов
                            while (packets_sent<max_packets_sent)
                            {
                                //Фиксируем время отправки
                                start_time_ms=u_get_cur_time_ms();
                                //Отправляем пакет
                                switch (nw_send_request(ping_socket, dest_addr, send_buf, packet_size,&program_error_code, &bytes_sent)) //send
                                {
                                    case 1: //Ошибка отправки
                                        if(log_write_error(log_file,program_error_code,&log_error_code)!=0)
                                        {
                                            log_diagnostics(log_error_code);
                                        }
                                        u_stop_program(program_error_code, log_error_code, &log_file);
                                        break;
                                    case 0: //Пакет отправлен
                                        //Увеличиваем счетчик отправленных пакетов
                                        packets_sent++;
                                        //Пытаемся получить ответ
                                        switch(nw_get_reply(ping_socket,dest_addr,recv_buf,packet_size,&program_error_code, &receive_result))
                                        {
                                            case 0: //Ответ получен
                                                u_show_result(receive_result, inet_ntoa(dest_addr.sin_addr), u_get_cur_time_ms() - start_time_ms, packet_size, recv_buf->ttl);
                                                if(log_write_result(log_file,&log_error_code, receive_result, inet_ntoa(dest_addr.sin_addr), u_get_cur_time_ms() - start_time_ms, packet_size, recv_buf->ttl) != 0)
                                                {
                                                    log_diagnostics(log_error_code);
                                                    u_stop_program(program_error_code, log_error_code, &log_file);
                                                }
                                                break;
                                            default: //Ошибка получения пакета
                                                if(log_write_error(log_file,program_error_code,&log_error_code)!=0)
                                                {
                                                    log_diagnostics(log_error_code);
                                                }
                                                u_stop_program(program_error_code, log_error_code, &log_file);
                                                break;
                                        }
                                        break;
                                }
                            }
                            break;
                    }
            }
    }
    //Завершение работы
    u_stop_program(program_error_code, log_error_code, &log_file);
}
