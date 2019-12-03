cmd_util-linux/volume_id/ntfs.o := arm-nuvoton-linux-uclibceabi-gcc -Wp,-MD,util-linux/volume_id/.ntfs.o.d   -std=gnu99 -Iinclude -Ilibbb  -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D"BB_VER=KBUILD_STR(1.22.1)" -DBB_BT=AUTOCONF_TIMESTAMP  -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wunused -Wunused-parameter -Wunused-function -Wunused-value -Wmissing-prototypes -Wmissing-declarations -Wno-format-security -Wdeclaration-after-statement -Wold-style-definition -fno-builtin-strlen -finline-limit=0 -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-guess-branch-probability -funsigned-char -static-libgcc -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -fno-unwind-tables -fno-asynchronous-unwind-tables -Os     -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(ntfs)"  -D"KBUILD_MODNAME=KBUILD_STR(ntfs)" -c -o util-linux/volume_id/ntfs.o util-linux/volume_id/ntfs.c

deps_util-linux/volume_id/ntfs.o := \
  util-linux/volume_id/ntfs.c \
    $(wildcard include/config/feature/volumeid/ntfs.h) \
    $(wildcard include/config/feature/blkid/type.h) \
  util-linux/volume_id/volume_id_internal.h \
  include/libbb.h \
    $(wildcard include/config/feature/shadowpasswds.h) \
    $(wildcard include/config/use/bb/shadow.h) \
    $(wildcard include/config/selinux.h) \
    $(wildcard include/config/feature/utmp.h) \
    $(wildcard include/config/locale/support.h) \
    $(wildcard include/config/use/bb/pwd/grp.h) \
    $(wildcard include/config/lfs.h) \
    $(wildcard include/config/feature/buffers/go/on/stack.h) \
    $(wildcard include/config/feature/buffers/go/in/bss.h) \
    $(wildcard include/config/feature/ipv6.h) \
    $(wildcard include/config/feature/seamless/xz.h) \
    $(wildcard include/config/feature/seamless/lzma.h) \
    $(wildcard include/config/feature/seamless/bz2.h) \
    $(wildcard include/config/feature/seamless/gz.h) \
    $(wildcard include/config/feature/seamless/z.h) \
    $(wildcard include/config/feature/check/names.h) \
    $(wildcard include/config/feature/prefer/applets.h) \
    $(wildcard include/config/long/opts.h) \
    $(wildcard include/config/feature/getopt/long.h) \
    $(wildcard include/config/feature/pidfile.h) \
    $(wildcard include/config/feature/syslog.h) \
    $(wildcard include/config/feature/individual.h) \
    $(wildcard include/config/echo.h) \
    $(wildcard include/config/printf.h) \
    $(wildcard include/config/test.h) \
    $(wildcard include/config/kill.h) \
    $(wildcard include/config/chown.h) \
    $(wildcard include/config/ls.h) \
    $(wildcard include/config/xxx.h) \
    $(wildcard include/config/route.h) \
    $(wildcard include/config/feature/hwib.h) \
    $(wildcard include/config/desktop.h) \
    $(wildcard include/config/feature/crond/d.h) \
    $(wildcard include/config/use/bb/crypt.h) \
    $(wildcard include/config/feature/adduser/to/group.h) \
    $(wildcard include/config/feature/del/user/from/group.h) \
    $(wildcard include/config/ioctl/hex2str/error.h) \
    $(wildcard include/config/feature/editing.h) \
    $(wildcard include/config/feature/editing/history.h) \
    $(wildcard include/config/feature/editing/savehistory.h) \
    $(wildcard include/config/feature/tab/completion.h) \
    $(wildcard include/config/feature/username/completion.h) \
    $(wildcard include/config/feature/editing/vi.h) \
    $(wildcard include/config/feature/editing/save/on/exit.h) \
    $(wildcard include/config/pmap.h) \
    $(wildcard include/config/feature/show/threads.h) \
    $(wildcard include/config/feature/ps/additional/columns.h) \
    $(wildcard include/config/feature/topmem.h) \
    $(wildcard include/config/feature/top/smp/process.h) \
    $(wildcard include/config/killall.h) \
    $(wildcard include/config/pgrep.h) \
    $(wildcard include/config/pkill.h) \
    $(wildcard include/config/pidof.h) \
    $(wildcard include/config/sestatus.h) \
    $(wildcard include/config/unicode/support.h) \
    $(wildcard include/config/feature/mtab/support.h) \
    $(wildcard include/config/feature/clean/up.h) \
    $(wildcard include/config/feature/devfs.h) \
  include/platform.h \
    $(wildcard include/config/werror.h) \
    $(wildcard include/config/big/endian.h) \
    $(wildcard include/config/little/endian.h) \
    $(wildcard include/config/nommu.h) \
  /usr/local/arm_linux_4.8/lib/gcc/arm-nuvoton-linux-uclibceabi/4.8.4/include-fixed/limits.h \
  /usr/local/arm_linux_4.8/lib/gcc/arm-nuvoton-linux-uclibceabi/4.8.4/include-fixed/syslimits.h \
  /usr/local/arm_linux_4.8/usr/include/limits.h \
  /usr/local/arm_linux_4.8/usr/include/features.h \
  /usr/local/arm_linux_4.8/usr/include/bits/uClibc_config.h \
  /usr/local/arm_linux_4.8/usr/include/sys/cdefs.h \
  /usr/local/arm_linux_4.8/usr/include/bits/posix1_lim.h \
  /usr/local/arm_linux_4.8/usr/include/bits/local_lim.h \
  /usr/local/arm_linux_4.8/arm-nuvoton-linux-uclibceabi/include/linux/limits.h \
  /usr/local/arm_linux_4.8/usr/include/bits/uClibc_local_lim.h \
  /usr/local/arm_linux_4.8/usr/include/bits/posix2_lim.h \
  /usr/local/arm_linux_4.8/usr/include/bits/xopen_lim.h \
  /usr/local/arm_linux_4.8/usr/include/bits/stdio_lim.h \
  /usr/local/arm_linux_4.8/usr/include/byteswap.h \
  /usr/local/arm_linux_4.8/usr/include/bits/byteswap.h \
  /usr/local/arm_linux_4.8/usr/include/bits/byteswap-common.h \
  /usr/local/arm_linux_4.8/usr/include/endian.h \
  /usr/local/arm_linux_4.8/usr/include/bits/endian.h \
  /usr/local/arm_linux_4.8/lib/gcc/arm-nuvoton-linux-uclibceabi/4.8.4/include/stdint.h \
  /usr/local/arm_linux_4.8/usr/include/stdint.h \
  /usr/local/arm_linux_4.8/usr/include/bits/wchar.h \
  /usr/local/arm_linux_4.8/usr/include/bits/wordsize.h \
  /usr/local/arm_linux_4.8/lib/gcc/arm-nuvoton-linux-uclibceabi/4.8.4/include/stdbool.h \
  /usr/local/arm_linux_4.8/usr/include/unistd.h \
  /usr/local/arm_linux_4.8/usr/include/bits/posix_opt.h \
  /usr/local/arm_linux_4.8/usr/include/bits/environments.h \
  /usr/local/arm_linux_4.8/usr/include/bits/types.h \
  /usr/local/arm_linux_4.8/lib/gcc/arm-nuvoton-linux-uclibceabi/4.8.4/include/stddef.h \
  /usr/local/arm_linux_4.8/usr/include/bits/typesizes.h \
  /usr/local/arm_linux_4.8/usr/include/bits/pthreadtypes.h \
  /usr/local/arm_linux_4.8/usr/include/bits/confname.h \
  /usr/local/arm_linux_4.8/usr/include/bits/getopt.h \
  /usr/local/arm_linux_4.8/usr/include/ctype.h \
  /usr/local/arm_linux_4.8/usr/include/bits/uClibc_touplow.h \
  /usr/local/arm_linux_4.8/usr/include/xlocale.h \
  /usr/local/arm_linux_4.8/usr/include/locale.h \
  /usr/local/arm_linux_4.8/usr/include/bits/uClibc_locale.h \
  /usr/local/arm_linux_4.8/usr/include/dirent.h \
  /usr/local/arm_linux_4.8/usr/include/bits/dirent.h \
  /usr/local/arm_linux_4.8/usr/include/errno.h \
  /usr/local/arm_linux_4.8/usr/include/bits/errno.h \
  /usr/local/arm_linux_4.8/arm-nuvoton-linux-uclibceabi/include/linux/errno.h \
  /usr/local/arm_linux_4.8/arm-nuvoton-linux-uclibceabi/include/asm/errno.h \
  /usr/local/arm_linux_4.8/arm-nuvoton-linux-uclibceabi/include/asm-generic/errno.h \
  /usr/local/arm_linux_4.8/arm-nuvoton-linux-uclibceabi/include/asm-generic/errno-base.h \
  /usr/local/arm_linux_4.8/usr/include/fcntl.h \
  /usr/local/arm_linux_4.8/usr/include/bits/fcntl.h \
  /usr/local/arm_linux_4.8/usr/include/sys/types.h \
  /usr/local/arm_linux_4.8/usr/include/time.h \
  /usr/local/arm_linux_4.8/usr/include/sys/select.h \
  /usr/local/arm_linux_4.8/usr/include/bits/select.h \
  /usr/local/arm_linux_4.8/usr/include/bits/sigset.h \
  /usr/local/arm_linux_4.8/usr/include/bits/time.h \
  /usr/local/arm_linux_4.8/usr/include/sys/sysmacros.h \
  /usr/local/arm_linux_4.8/usr/include/bits/uio.h \
  /usr/local/arm_linux_4.8/usr/include/sys/stat.h \
  /usr/local/arm_linux_4.8/usr/include/bits/stat.h \
  /usr/local/arm_linux_4.8/usr/include/inttypes.h \
  /usr/local/arm_linux_4.8/usr/include/netdb.h \
  /usr/local/arm_linux_4.8/usr/include/netinet/in.h \
  /usr/local/arm_linux_4.8/usr/include/sys/socket.h \
  /usr/local/arm_linux_4.8/usr/include/sys/uio.h \
  /usr/local/arm_linux_4.8/usr/include/bits/socket.h \
  /usr/local/arm_linux_4.8/usr/include/bits/socket_type.h \
  /usr/local/arm_linux_4.8/usr/include/bits/sockaddr.h \
  /usr/local/arm_linux_4.8/arm-nuvoton-linux-uclibceabi/include/asm/socket.h \
  /usr/local/arm_linux_4.8/arm-nuvoton-linux-uclibceabi/include/asm-generic/socket.h \
  /usr/local/arm_linux_4.8/arm-nuvoton-linux-uclibceabi/include/asm/sockios.h \
  /usr/local/arm_linux_4.8/arm-nuvoton-linux-uclibceabi/include/asm-generic/sockios.h \
  /usr/local/arm_linux_4.8/usr/include/bits/in.h \
  /usr/local/arm_linux_4.8/usr/include/rpc/netdb.h \
  /usr/local/arm_linux_4.8/usr/include/bits/siginfo.h \
  /usr/local/arm_linux_4.8/usr/include/bits/netdb.h \
  /usr/local/arm_linux_4.8/usr/include/setjmp.h \
  /usr/local/arm_linux_4.8/usr/include/bits/setjmp.h \
  /usr/local/arm_linux_4.8/usr/include/signal.h \
  /usr/local/arm_linux_4.8/usr/include/bits/signum.h \
  /usr/local/arm_linux_4.8/usr/include/bits/sigaction.h \
  /usr/local/arm_linux_4.8/usr/include/bits/sigcontext.h \
  /usr/local/arm_linux_4.8/arm-nuvoton-linux-uclibceabi/include/asm/sigcontext.h \
  /usr/local/arm_linux_4.8/usr/include/bits/sigstack.h \
  /usr/local/arm_linux_4.8/usr/include/sys/ucontext.h \
  /usr/local/arm_linux_4.8/usr/include/sys/procfs.h \
  /usr/local/arm_linux_4.8/usr/include/sys/time.h \
  /usr/local/arm_linux_4.8/usr/include/sys/user.h \
  /usr/local/arm_linux_4.8/usr/include/bits/sigthread.h \
  /usr/local/arm_linux_4.8/usr/include/stdio.h \
  /usr/local/arm_linux_4.8/usr/include/bits/uClibc_stdio.h \
  /usr/local/arm_linux_4.8/usr/include/wchar.h \
  /usr/local/arm_linux_4.8/usr/include/bits/uClibc_mutex.h \
  /usr/local/arm_linux_4.8/usr/include/pthread.h \
  /usr/local/arm_linux_4.8/usr/include/sched.h \
  /usr/local/arm_linux_4.8/usr/include/bits/sched.h \
  /usr/local/arm_linux_4.8/usr/include/bits/uClibc_clk_tck.h \
  /usr/local/arm_linux_4.8/usr/include/bits/uClibc_pthread.h \
  /usr/local/arm_linux_4.8/lib/gcc/arm-nuvoton-linux-uclibceabi/4.8.4/include/stdarg.h \
  /usr/local/arm_linux_4.8/usr/include/stdlib.h \
  /usr/local/arm_linux_4.8/usr/include/bits/waitflags.h \
  /usr/local/arm_linux_4.8/usr/include/bits/waitstatus.h \
  /usr/local/arm_linux_4.8/usr/include/alloca.h \
  /usr/local/arm_linux_4.8/usr/include/string.h \
  /usr/local/arm_linux_4.8/usr/include/libgen.h \
  /usr/local/arm_linux_4.8/usr/include/poll.h \
  /usr/local/arm_linux_4.8/usr/include/sys/poll.h \
  /usr/local/arm_linux_4.8/usr/include/bits/poll.h \
  /usr/local/arm_linux_4.8/usr/include/sys/ioctl.h \
  /usr/local/arm_linux_4.8/usr/include/bits/ioctls.h \
  /usr/local/arm_linux_4.8/arm-nuvoton-linux-uclibceabi/include/asm/ioctls.h \
  /usr/local/arm_linux_4.8/arm-nuvoton-linux-uclibceabi/include/asm-generic/ioctls.h \
  /usr/local/arm_linux_4.8/arm-nuvoton-linux-uclibceabi/include/linux/ioctl.h \
  /usr/local/arm_linux_4.8/arm-nuvoton-linux-uclibceabi/include/asm/ioctl.h \
  /usr/local/arm_linux_4.8/arm-nuvoton-linux-uclibceabi/include/asm-generic/ioctl.h \
  /usr/local/arm_linux_4.8/usr/include/bits/ioctl-types.h \
  /usr/local/arm_linux_4.8/usr/include/sys/ttydefaults.h \
  /usr/local/arm_linux_4.8/usr/include/sys/mman.h \
  /usr/local/arm_linux_4.8/usr/include/bits/mman.h \
  /usr/local/arm_linux_4.8/usr/include/bits/mman-common.h \
  /usr/local/arm_linux_4.8/usr/include/sys/wait.h \
  /usr/local/arm_linux_4.8/usr/include/sys/resource.h \
  /usr/local/arm_linux_4.8/usr/include/bits/resource.h \
  /usr/local/arm_linux_4.8/usr/include/termios.h \
  /usr/local/arm_linux_4.8/usr/include/bits/termios.h \
  /usr/local/arm_linux_4.8/usr/include/sys/param.h \
  /usr/local/arm_linux_4.8/arm-nuvoton-linux-uclibceabi/include/linux/param.h \
  /usr/local/arm_linux_4.8/arm-nuvoton-linux-uclibceabi/include/asm/param.h \
  /usr/local/arm_linux_4.8/arm-nuvoton-linux-uclibceabi/include/asm-generic/param.h \
  /usr/local/arm_linux_4.8/usr/include/pwd.h \
  /usr/local/arm_linux_4.8/usr/include/grp.h \
  /usr/local/arm_linux_4.8/usr/include/mntent.h \
  /usr/local/arm_linux_4.8/usr/include/paths.h \
  /usr/local/arm_linux_4.8/usr/include/sys/statfs.h \
  /usr/local/arm_linux_4.8/usr/include/bits/statfs.h \
  /usr/local/arm_linux_4.8/usr/include/utmp.h \
  /usr/local/arm_linux_4.8/usr/include/bits/utmp.h \
  /usr/local/arm_linux_4.8/usr/include/arpa/inet.h \
  include/pwd_.h \
  include/grp_.h \
  include/shadow_.h \
  include/xatonum.h \
  include/volume_id.h \

util-linux/volume_id/ntfs.o: $(deps_util-linux/volume_id/ntfs.o)

$(deps_util-linux/volume_id/ntfs.o):
