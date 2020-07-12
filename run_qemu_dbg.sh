#!/bin/bash

#запускает с отладчиком, далее в отладчике вводим
#target remote localhost:1234
#break main
#
#управление
#с - continue
#n - next
#s - into

qemu-system-i386 -s -S -hda hdd.img &
cgdb src/MiniOS.bin.dbg