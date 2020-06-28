;
; boot.s -- Точка старта ядра. Также определяет multiboot заголовок.
;
;

;ассемблер использует синтаксис Intel
;Чтобы загрузчик Grub определил наше ядро, оно должно соотвествовать формату multiboot


;Создаем multiboot заголовок, указываем нужные флаги

MBOOT_PAGE_ALIGN    equ 1<<0    ; Load kernel and modules on a page boundary
MBOOT_MEM_INFO      equ 1<<1    ; Provide your kernel with memory info
MBOOT_HEADER_MAGIC  equ 0x1BADB002 ; Сигнатура формата Multiboot
; NOTE: We do not use MBOOT_AOUT_KLUDGE. It means that GRUB does not
; pass us a symbol table.
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO ;Флаги, которые содержат
; дополнительные требования к загрузке ядра и параметрам, передаваемым загрузчиком ядру
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS) ;Контрольная сумма


[BITS 32]                       ; All instructions should be 32-bit.

[GLOBAL mboot]                  ; Делаем 'mboot' доступным из Си.
[EXTERN code]                   ; Start of the '.text' section.
[EXTERN bss]                    ; Start of the .bss section.
[EXTERN end]                    ; End of the last loadable section.

mboot:
    dd  MBOOT_HEADER_MAGIC      ; GRUB will search for this value on each
                                ; 4-byte boundary in your kernel file
    dd  MBOOT_HEADER_FLAGS      ; How GRUB should load your file / settings
    dd  MBOOT_CHECKSUM          ; To ensure that the above values are correct
    
    dd  mboot                   ; Location of this descriptor
    dd  code                    ; Start of kernel '.text' (code) section.
    dd  bss                     ; End of kernel '.data' section.
    dd  end                     ; End of kernel.
    dd  start                   ; Kernel entry point (initial EIP).

[GLOBAL start]                  ; Точка входа ядра (делаем видимым для линковщика)
[EXTERN main]                   ; Точка входа нашего Си кода


;Grub через регистры %ebx и %eax передает указатель
; на структуру multiboot Information и значение 0x1BADB002
; соответственно. Структура multiboot Information содержит
; различную информацию, в том числе список загруженных модулей
; и их расположение, что может понадобиться для дальнейшей загрузки системы.

start:
    ; Загрузим multiboot информацию в стек, она будет первым параметром функции main на Си
    push    ebx

    ; Запускаем ядро:
    cli                         ; Отключаем прерывания
    call main                   ; Вызываем нашу функцию main()
    jmp $                       ; Войдем в бесконечный цикл, чтобы остановить
                                ; выполнением процессора мусора в памяти
                                ; после нашего ядра!