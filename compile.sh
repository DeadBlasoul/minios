#!/bin/bash

#Подготавливаем окружение для компиляции
make umount -i

#Компилируем ядро
make compile

#Компилируем образ
make image

#Обновляем образ
make update_image