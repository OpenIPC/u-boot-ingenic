#include <common.h>
#include <command.h>
#include <environment.h>
#include <image.h>
#include <asm/byteorder.h>
#include <fat.h>
#include <mmc.h>

// Enumerated error codes for various error scenarios
typedef enum {
	ERR_NONE = 0,
	ERR_INVALID_IMAGE_SIZE,
	ERR_INVALID_DATA_CKSUM,
	ERR_INVALID_HEADER_SIZE,
	ERR_INVALID_LOAD_ADDR,
	ERR_INVALID_MAGIC_ARCH,
	ERR_INVALID_HEADER_CKSUM,
	ERR_INVALID_KERNEL_TYPE,
	ERR_FILE_NOT_FOUND,
	ERR_MMC_NOT_PRESENT,
	ERR_FAT_REGISTRATION_FAIL,
	ERR_FAT_DETECT_FAIL
} ErrorCode;

// Memory address definition
static unsigned char *LOAD_ADDR = NULL;

// Format strings for boot arguments and commands

// Maximum load size
#define SDSTART_MAX_LOADSZ	0xA00000 // 10MB

// Delay before loading the kernel, in seconds.
// The countdown will only start if a kernel image is found.
#define SDSTART_DEFAULT_DELAY	1

// Macro to generate kernel filenames based on SoC
#define SDSTART_KERNEL_FILENAMES(soc) \
    "factory_" #soc "_ZMC6tiIDQN", \
    NULL  // sentinel value

// Kernel filenames based on SoC
#ifdef CONFIG_T10
static const char* kernel_filenames[] = { SDSTART_KERNEL_FILENAMES(t10) };
#elif defined(CONFIG_T15)
static const char* kernel_filenames[] = { SDSTART_KERNEL_FILENAMES(t15) };
#elif defined(CONFIG_T20)
// Special case for T20 to match factory
static const char* kernel_filenames[] = {
	"factory_ZMC6tiIDQN",
	NULL
};
#elif defined(CONFIG_T21)
static const char* kernel_filenames[] = { SDSTART_KERNEL_FILENAMES(t21) };
#elif defined(CONFIG_T30)
static const char* kernel_filenames[] = { SDSTART_KERNEL_FILENAMES(t30) };
#elif defined(CONFIG_T31)
static const char* kernel_filenames[] = { SDSTART_KERNEL_FILENAMES(t31) };
#elif defined(CONFIG_T40)
static const char* kernel_filenames[] = { SDSTART_KERNEL_FILENAMES(t40) };
#else
static const char* kernel_filenames[] = { NULL };  // Default case, do not load anything
#endif

// This function prompts the user to press Ctrl-C to cancel the kernel loading,
// lasting for the specified delay (in seconds).
static bool prompt_and_wait_for_interrupt(int delay) {
	printf("MMC:   To cancel loading the kernel, press Ctrl-C within the next %d second(s)...\n", delay);
	unsigned long start = get_timer(0);

	while (get_timer(0) - start < delay * 1000) {
		if (ctrlc()) {
			printf("Kernel loading cancel by user.\n");
			return true;
		}
		udelay(10000);  // Check every 10ms
	}
	return false;
}

// Function to handle and display errors based on ErrorCode
static int handle_error(ErrorCode err, const char* context) {
	switch (err) {
		case ERR_INVALID_IMAGE_SIZE:
			printf("Error: Invalid image size for %s.\n", context);
			break;
		case ERR_INVALID_DATA_CKSUM:
			printf("Error: Invalid data checksum for %s.\n", context);
			break;
		case ERR_INVALID_HEADER_SIZE:
			printf("Error: Invalid header size for %s.\n", context);
			break;
		case ERR_INVALID_MAGIC_ARCH:
			printf("Error: Invalid MAGIC or ARCH for %s.\n", context);
			break;
		case ERR_INVALID_HEADER_CKSUM:
			printf("Error: Invalid header checksum for %s.\n", context);
			break;
		case ERR_INVALID_KERNEL_TYPE:
			printf("Error: Invalid kernel type for %s.\n", context);
			break;
		case ERR_FILE_NOT_FOUND:
			printf("MMC:   Kernel file not found.\n");
			break;
		case ERR_MMC_NOT_PRESENT:
			printf("Error: MMC card is not present.\n");
			break;
		case ERR_FAT_REGISTRATION_FAIL:
			printf("Error: Failed to register FAT device.\n");
			break;
		case ERR_FAT_DETECT_FAIL:
			printf("Error: Failed to detect FAT filesystem.\n");
			break;
		case ERR_INVALID_LOAD_ADDR:
			printf("Error: Load ADDR is not initialized.\n");
			break;
		default:
			break;
	}
	return 0; // Return success so u-Boot doesn't display usage
}

// Function to validate the header and checksum of the loaded image
static ErrorCode check_header_and_checksum_validity(long nbytes) {
	// Load address initialization
	if (!LOAD_ADDR) {
		return ERR_INVALID_LOAD_ADDR;
	}

	image_header_t *hdr = (image_header_t *)LOAD_ADDR;
	unsigned long checksum;

	if (nbytes < sizeof(*hdr)) {
		return ERR_INVALID_HEADER_SIZE;
	}

	image_header_t local_hdr = *hdr;  // Create a local copy of the header
	if (ntohl(local_hdr.ih_magic) != IH_MAGIC || local_hdr.ih_arch != IH_ARCH_MIPS) {
		return ERR_INVALID_MAGIC_ARCH;
	}

	checksum = ntohl(local_hdr.ih_hcrc);
	local_hdr.ih_hcrc = 0;

	if (crc32(0, (unsigned char const *)&local_hdr, sizeof(local_hdr)) != checksum) {
		return ERR_INVALID_HEADER_CKSUM;
	}

	if (local_hdr.ih_type != IH_TYPE_KERNEL) {
		return ERR_INVALID_KERNEL_TYPE;
	}

	if (nbytes != (sizeof(local_hdr) + ntohl(local_hdr.ih_size))) {
		return ERR_INVALID_IMAGE_SIZE;
	}

	checksum = ntohl(local_hdr.ih_dcrc);
	if (crc32(0, (unsigned char const *)(LOAD_ADDR + sizeof(local_hdr)), ntohl(local_hdr.ih_size)) != checksum) {
		return ERR_INVALID_DATA_CKSUM;
	}

	return ERR_NONE;
}

// Function to load the kernel from MMC into RAM and validate its contents
static int load_kernel_and_validate(void) {
	int i;
	for (i = 0; kernel_filenames[i] != NULL; i++) {
		long file_size = file_fat_read(kernel_filenames[i], LOAD_ADDR, SDSTART_MAX_LOADSZ);
		if (file_size > 0) {
			ErrorCode err = check_header_and_checksum_validity(file_size);
			if (err == ERR_NONE) {
				return 0; // Kernel loaded and validated
			}
			// Handle errors other than ERR_FILE_NOT_FOUND, and exit
			if (err != ERR_FILE_NOT_FOUND) {
				return handle_error(err, kernel_filenames[i]);
			}
		}
	}
	return handle_error(ERR_FILE_NOT_FOUND, kernel_filenames[0]);
}

/*
// Function to check if the SD card is present and contains a valid FAT filesystem
static int validate_sd_card(void) {
	block_dev_desc_t *stor_dev = get_dev("mmc", 0);
	if (NULL == stor_dev) {
		handle_error(ERR_MMC_NOT_PRESENT, "");
		return false;
	}
	int ret = fat_register_device(stor_dev, 1);
	if (ret != 0) {
		handle_error(ERR_FAT_REGISTRATION_FAIL, "");
		return false;
	}
	ret = file_fat_detectfs();
	if (ret != 0) {
		handle_error(ERR_FAT_DETECT_FAIL, "");
		return false;
	}
	printf("FAT filesystem detected successfully.\n");
	return true;
}
*/

// Function to set the environment variables for booting the kernel
static void configure_boot_environment(void) {
	char bootcmd[512];

	// Construct the bootcmd using the LOAD_ADDR
	snprintf(bootcmd, sizeof(bootcmd),
			"sf probe; setenv setargs setenv bootargs ${bootargs}; run setargs; bootm %p", (void*)LOAD_ADDR);

	// Update the bootcmd environment variable
	setenv("bootcmd", bootcmd);

	// Run the boot command
	run_command("boot", 0);
}

// Main function when the "sdstart" command is run in U-Boot
int sdstart(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[]) {
	// Fetch 'baseaddr' from the U-Boot environment
	const char *baseaddr_str = getenv("baseaddr");
	if (!baseaddr_str) {
		printf("MMC:   Error: 'baseaddr' not found in the environment.\n");
		return -1;
	}

	// Convert string to pointer (address)
	LOAD_ADDR = (unsigned char *)simple_strtoul(baseaddr_str, NULL, 16);

	/* Lets disable this for now, if we are running, assume it's valid already
	// Validate the SD card
	if (!validate_sd_card() != 0) {
		return 0; // Return early if SD card is not valid, silently
	}
	*/

	// Detect the kernel's presence without fully loading it
        printf("MMC:   Checking for kernel image from MMC... \n");
	int i;
	int found_kernel = 0;
	for (i = 0; kernel_filenames[i] != NULL; i++) {
		// Reading with size 0 to just detect
		long file_size = file_fat_read(kernel_filenames[i], LOAD_ADDR, 0);
		if (file_size > 0) {
			found_kernel = 1;
			break;
		}
	}

	if (!found_kernel) {
		return handle_error(ERR_FILE_NOT_FOUND, kernel_filenames[0]);
	}

	// If kernel is detected, then show the Ctrl-C prompt
	int delay = (argc > 1) ? simple_strtol(argv[1], NULL, 10) : SDSTART_DEFAULT_DELAY;
	if (prompt_and_wait_for_interrupt(delay)) {
		printf("MMC:   SDStart loading was canceled during the prompt.\n");
		return 0; // Return early if user interrupted
	}

        printf("MMC:   Loading external kernel image... \n");

	// Load and validate the kernel
	int old_ctrlc = disable_ctrlc(0);
	if (load_kernel_and_validate() == 0) {
		configure_boot_environment();
		return 0;
	}
	disable_ctrlc(old_ctrlc);

	return 0;
}

U_BOOT_CMD(
	sdstart,	2,	1,	sdstart,
	"Load a kernel from the MMC card with an interruptible delay.",
	"[delay_in_seconds]"
);
