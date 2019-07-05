#define SVC_UART_PUTS		0x0001
#define SVC_UART_HEX		0x0002
#define SVC_UART_HEX_LONG	0x0003
#define SVC_UART_PUTC		0x0004

#define SVC_CLEAR_SCREEN	0x0010
#define SVC_DRAW_WALLPAPER  0x0011
#define SVC_DRAW_PIXEL		0x0012

.global sys_uart_puts
.balign 8
.type    sys_uart_puts,%function
sys_uart_puts:
    mov     x12, x0
    svc     SVC_UART_PUTS
    ret

.global sys_uart_hex
.balign 8
.type    sys_uart_hex,%function
sys_uart_hex:
    mov     x12, x0
    svc     SVC_UART_HEX
    ret

.global sys_uart_hex_long
.balign 8
.type    sys_uart_hex_long,%function
sys_uart_hex_long:
    mov     x12, x0
    svc     SVC_UART_HEX_LONG
    ret

.global sys_draw_wallpaper
.balign 8
.type    sys_draw_wallpaper,%function
sys_draw_wallpaper:
    svc     SVC_DRAW_WALLPAPER
    ret

.global sys_draw_pixel
.balign 8
.type    sys_draw_pixel,%function
sys_draw_pixel:
    mov     x12, x0
    mov     x11, x1
    svc     SVC_DRAW_PIXEL
    ret
