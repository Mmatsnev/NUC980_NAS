cmd_arch/arm/boot/uImage := /bin/sh ./scripts/mkuboot.sh -A arm -O linux -C none  -T kernel -a 0x00008000 -e 0x00008000 -n 'Linux-4.4.179+' -d arch/arm/boot/zImage arch/arm/boot/uImage
