SRCS = $(wildcard *.c kernel/*.c drivers/*.c gui/*.c lib/*.c fs/*.c)
OBJS = $(SRCS:.c=.o)
CFLAGS = -g -gdwarf-4 -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles
INCLUDE = -I lib -I drivers -I gui -I kernel -I . -I fs

all: clean kernel8.img

start.o: start.S
	aarch64-none-elf-gcc $(CFLAGS) -c start.S -o start.o

entry.o: entry.S
	aarch64-none-elf-gcc $(CFLAGS) -c entry.S -o entry.o

%.o: %.c
	aarch64-none-elf-gcc $(CFLAGS) $(INCLUDE) -c $< -o $@

kernel8.img: start.o entry.o $(OBJS)
	aarch64-none-elf-ld -nostdlib -nostartfiles start.o entry.o $(OBJS) -T link.ld -o kernel8.elf
	aarch64-none-elf-objcopy -O binary kernel8.elf kernel8.img

clean:
	rm kernel8.elf *.o >/dev/null 2>/dev/null || true
	rm kernel/*.o >/dev/null 2>/dev/null || true
	rm drivers/*.o >/dev/null 2>/dev/null || true
	rm lib/*.o >/dev/null 2>/dev/null || true
	rm gui/*.o >/dev/null 2>/dev/null || true
	rm fs/*.o >/dev/null 2>/dev/null || true

run:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -drive file=disk.img,if=sd,format=raw -serial stdio

deploy:
	cp kernel8.img /Volumes/boot/
