#!/bin/bash

#Подготавливаем окружение для компиляции
make umount -i 2> /dev/null

#Компилируем ядро
make compile

#Компилируем образ
make image

#Обновляем образ
make update_image