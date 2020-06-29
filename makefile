#обновление ядра в образе
update_image: 
	@losetup /dev/loop91 ./hdd.img
	@echo "Mounting partition to /dev/loop92..."
	@losetup /dev/loop92 ./hdd.img \
		--offset `echo \`fdisk -lu /dev/loop91 | sed -n 9p | awk '{print $$3}'\`*512 | bc` \
		--sizelimit `echo \`fdisk -lu /dev/loop91 | sed -n 9p | awk '{print $$4}'\`*512 | bc`
	@losetup -d /dev/loop91

	@echo "Write new MiniOS.bin to hdd.img..."
	@mkdir -p tempdir
	@mount /dev/loop92 tempdir/
	@cp src/MiniOS.bin tempdir/
	@umount /dev/loop92
	@rm -r tempdir
	@losetup -d /dev/loop92

#компиляция исходников. они в каталоге src, запускаем там свой Makefile
compile:
	make --directory=src

#создание образа
image:
	@echo "Creating hdd.img..."
	@dd if=/dev/zero of=./hdd.img bs=512 count=16065 1>/dev/null 2>&1

	@echo "Creating bootable first FAT32 partition..."
	@losetup /dev/loop91 ./hdd.img
	@(echo c; echo u; echo n; echo p; echo 1; echo ;  echo ; echo a; echo 1; echo t; echo c; echo w;) | fdisk /dev/loop91 1>/dev/null 2>&1 || true

	@echo "Mounting partition to /dev/loop92..."
	@losetup /dev/loop92 ./hdd.img \
		--offset `echo \`fdisk -lu /dev/loop91 | sed -n 9p | awk '{print $$3}'\`*512 | bc` \
		--sizelimit `echo \`fdisk -lu /dev/loop91 | sed -n 9p | awk '{print $$4}'\`*512 | bc`
	@losetup -d /dev/loop91

	@echo "Format partition..."
	@mkdosfs -I -n 'MiniOS' /dev/loop92

	@echo "Copy kernel and grub files on partition..."
	@mkdir -p tempdir
	@mount /dev/loop92 tempdir
	@mkdir tempdir/boot
	@cp -r grub tempdir/boot/
	@cp ./src/MiniOS.bin tempdir/
#	@cp ./initrd tempdir/
	@sleep 1
	@umount /dev/loop92
	@rm -r tempdir
	@losetup -d /dev/loop92

	@echo "Installing GRUB..."
	@echo "device (hd0) hdd.img \n \
		   root (hd0,0)		 \n \
		   setup (hd0)		  \n \
		   quit\n" | grub --batch 1>/dev/null
	@echo "Done!"

#очистка скомпилированных данных
clean:
	make clean --directory=src

#освобождение занятых ресурсов (если аварийно заверешены команды компиляции)
umount:
	@umount /dev/loop92
	@umount /dev/loop91
	@umount /dev/loop90
	@losetup -d /dev/loop91
	@losetup -d /dev/loop92
	@losetup -d /dev/loop90