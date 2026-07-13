# esp8266_btop
btop like system monitor for oled 0.92 display

Для запуска необходимо:

1. Собрать программу для PC
2. Записать скетч
3. Подключить по кабелю
4. Запустить собранную программу на PC

Сбор программы для PC
   g++ cpu_monitor_PC.cpp -o cpu_monitor

Программа, по сути, отправляет данные в порт /dev/ttyUSB$NUM вида
    $HOSTNAME:$CPU:$MEM:$GPU
    где $NUM - перебор порядковых номеров
