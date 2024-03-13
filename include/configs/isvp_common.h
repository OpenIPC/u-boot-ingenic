#undef CONFIG_BOOTARGS
#undef CONFIG_BOOTCOMMAND

#undef CONFIG_ENV_OFFSET
#undef CONFIG_ENV_SECT_SIZE
#undef CONFIG_ENV_SIZE
#undef CONFIG_SFC_MIN_ALIGN

#undef CONFIG_SYS_BOOTPARAMS_LEN
#undef CONFIG_SYS_CBSIZE
#undef CONFIG_SYS_LOAD_ADDR
#undef CONFIG_SYS_MONITOR_LEN
#undef CONFIG_SYS_PROMPT

#undef CONFIG_IPADDR
#undef CONFIG_GATEWAYIP
#undef CONFIG_SERVERIP
#undef CONFIG_ETHADDR

#ifdef __CONFIG_ISVP_H__
#define CONFIG_SOC "t10"
#elif defined(__CONFIG_ISVP_T20_H__)
#define CONFIG_SOC "t20"
#elif defined(__CONFIG_ISVP_T21_H__)
#define CONFIG_SOC "t21"
#elif defined(__CONFIG_ISVP_T30_H__)
#define CONFIG_SOC "t30"
#else
#define CONFIG_SOC "t31"
#endif

#if (defined(CONFIG_DDR2_128M) || defined(CONFIG_DDR3_128M))
#define CONFIG_EXTRA_SETTINGS \
	"totalmem=128M\0" \
	"osmem=64M\0" \
	"rmem=64M@0x4000000\0"
#else
#define CONFIG_EXTRA_SETTINGS \
	"totalmem=64M\0" \
	"osmem=40M\0" \
	"rmem=24M@0x2800000\0"
#endif

#define CONFIG_ENV_KERNADDR 0x50000
#define CONFIG_ENV_KERNSIZE 0x200000
#define CONFIG_ENV_ROOTADDR 0x250000
#define CONFIG_ENV_ROOTSIZE 0x500000

#define CONFIG_BOOTARGS "console=ttyS1,115200 panic=10 root=/dev/mtdblock3 init=/init mtdparts=jz_sfc:256k(boot),64k(env),2048k(kernel),\\${rootmtd}(rootfs),-(rootfs_data) mem=\\${osmem} rmem=\\${rmem}"
#define CONFIG_BOOTCOMMAND "setenv bootcmd ${cmdnor}; sf probe 0; saveenv; run bootcmd"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"baseaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"kernaddr=" __stringify(CONFIG_ENV_KERNADDR) "\0" \
	"kernsize=" __stringify(CONFIG_ENV_KERNSIZE) "\0" \
	"rootaddr=" __stringify(CONFIG_ENV_ROOTADDR) "\0" \
	"rootsize=" __stringify(CONFIG_ENV_ROOTSIZE) "\0" \
	"cmdnor=sf probe 0; setenv setargs setenv bootargs ${bootargs}; run setargs; sf read ${baseaddr} ${kernaddr} ${kernsize}; bootm ${baseaddr}\0" \
	"ubnor=sf probe 0; sf erase 0x0 ${kernaddr}; sf write ${baseaddr} 0x0 ${kernaddr}\0" \
	"uknor=${updatetool} ${baseaddr} uImage.${soc}; sf probe 0; sf erase ${kernaddr} ${kernsize}; sf write ${baseaddr} ${kernaddr} ${filesize}\0" \
	"urnor=${updatetool} ${baseaddr} rootfs.squashfs.${soc}; sf probe 0; sf erase ${rootaddr} ${rootsize}; sf write ${baseaddr} ${rootaddr} ${filesize}\0" \
	"rootmtd=5120k\0" \
	"updatetool=tftpboot\0" \
	"sdcard=mmc rescan; setenv updatetool fatload mmc 0\0" \
	"soc="CONFIG_SOC"\0" \
	CONFIG_EXTRA_SETTINGS

#define CONFIG_ENV_OFFSET 0x40000
#define CONFIG_ENV_SIZE 0x10000
#define CONFIG_ENV_SECT_SIZE 0x10000
#define CONFIG_SFC_MIN_ALIGN 0x10000

#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_PROMPT "\nPress ENTER to interrupt boot in %d...\n"
#define CONFIG_AUTOBOOT_STOP_STR "\x0D"

#define CONFIG_CMDLINE_EDITING
#define CONFIG_CMDLINE_TAG
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_MAXARGS 64
#define CONFIG_SYS_NO_FLASH

#define CONFIG_DECMP_BUFFER_ADRS 0x80200000
#define CONFIG_SPL_LZOP
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_LOAD_ADDR 0x80600000
#define CONFIG_SYS_PROMPT "OpenIPC # "

#define CONFIG_SYS_CBSIZE 2048
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MALLOC_LEN (32 * 1024 * 1024)
#define CONFIG_SYS_BOOTPARAMS_LEN (256 * 1024)
#define CONFIG_SYS_MONITOR_LEN (230 * 1024)

#define CONFIG_BOOTDELAY 1
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_FAT
#define CONFIG_CMD_FS_GENERIC
#define CONFIG_CMD_GPIO
#define CONFIG_CMD_SOURCE
#define CONFIG_CMD_TFTPPUT
#define CONFIG_VERSION_VARIABLE

#define LOAD_SCRIPT0 "fatload mmc 0 ${baseaddr} boot.scr"

#define CONFIG_IPADDR 192.168.1.10
#define CONFIG_GATEWAYIP 192.168.1.1
#define CONFIG_SERVERIP 192.168.1.254
#define CONFIG_NETMASK 255.255.255.0
#define CONFIG_ETHADDR 00:00:23:34:45:66
