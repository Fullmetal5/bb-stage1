#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <string.h>
#include <fat.h>
#include <ogc/machine/processor.h>

#include "test.h"

#include "elf_loader.h"
#include "sync.h"

extern void __exception_closeall(void);

void boot_elf(void *payload, size_t size) {
	printf("Shutting down IOS Subsystems... ");
	fflush(stdout);
	__IOS_ShutdownSubsystems();
	printf("OK\n");
	printf("Shutting down CPU ISR... ");
	fflush(stdout);
	u32 level;
	_CPU_ISR_Disable(level);
	printf("OK\n");
	printf("Shutting down exception vectors... ");
	fflush(stdout);
	__exception_closeall();
	printf("OK\n");
	
	printf("Copying ELF loading stub... ");
	fflush(stdout);
	memcpy((void*)0x81330000, loader_bin, loader_bin_len);
	sync_before_exec((void*)0x81330000, loader_bin_len);
	printf("OK\n");
	printf("Jumping to entry point!\n");
	fflush(stdout);
	
	void (*entry)(void*, u32) = (void*)0x81330000;
	entry(payload, size);
	
	printf("If you can see this then something has gone very wrong!\n");
	fflush(stdout);
	for(;;)
		;
}

int row_size;
int column_size;
int max_attribute_len = 30;

// I really wanted it semi-aligned to the right.
// Go ahead and avert your eyes now.
void print_credit(const char* name, const char* attribute) {
	int name_len = strlen(name);
	
	printf("%s", name);
	for (int j = 0; j < (row_size - name_len - max_attribute_len - 1); j++)
			printf(" ");
	
	for (int i = 0; attribute[i];) {
		char tmp[max_attribute_len + 1];
		for (int j = 0; j < max_attribute_len; j++) {
			if (attribute[i + j] != 0x00 && attribute[i + j] != '~') {
				tmp[j] = attribute[i + j];
			} else {
				tmp[j] = 0x00;
				break;
			}
		}
		tmp[max_attribute_len] = 0x00;
		
		printf("%s\n", tmp);
		
		i += strlen(tmp);
		if (attribute[i] == '~')
			i++;
		
		if (attribute[i])
			for (int j = 0; j < (row_size - max_attribute_len - 1); j++)
				printf(" ");
	}
}

void middle_align(const char* str) {
	for (int i = 0; i < ((row_size - strlen(str)) / 2); i++)
		printf(" ");
	printf("%s\n", str);
}

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

int main(int argc, char **argv) {
	VIDEO_Init();
	rmode = VIDEO_GetPreferredMode(NULL);
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
	row_size = rmode->fbWidth / 8;
	column_size = rmode->xfbHeight / 16;

	printf("\x1b[2;0H");

	middle_align("CREDITS");
	printf("\n");
	print_credit("Team Twiizers / Fail0verflow", "For the incredible things~"
												"you guys have done for the~"
												"community over the years."
					  );
	printf("\n");
	print_credit("The Dolphin devs", "For the amazing amount~"
									"of work that goes~"
									"into making the emulator.~"
									"This would be impossible~"
									"without you guys."
	);
	printf("\n");
	print_credit("The devkitPro devs", "For making a great~"
										"toolchain to help make~"
										"this community possible."
	);
	printf("\n");
	print_credit("metaconstruct", "For the ES vuln.");
	printf("\n");
	print_credit("byte[]", "For listening to my stupid~"
							"ideas.");
	
	sleep(10);
	for (int i = 0; i < column_size; i++) {
		usleep(15000);
		printf("\n");
	}
	printf("\x1b[%d;%dH", column_size / 2, (row_size / 2) + 2);
	printf("Thanks, from Fullmetal5");
	// ffmpeg -i small_1621606.jpg -f rawvideo -pix_fmt yuyv422 test.bin
#define IMG_WIDTH 202
#define IMG_HEIGHT 202
	int off_x = (row_size / 2) * 8 - IMG_WIDTH - 2;
	int off_y = (column_size / 2) * 16 - (IMG_HEIGHT / 2);
	for (int i = 0; i < IMG_HEIGHT; i++)
		memcpy(xfb + (off_y * rmode->fbWidth * 2) + (i * rmode->fbWidth * 2) + off_x, test_bin + (i * (IMG_WIDTH * 2)), IMG_WIDTH * 2);
	sleep(5);
	for (int i = 0; i < (column_size * 2); i++) {
		usleep(15000);
		printf("\n");
	}
	printf("\x1b[2;0H");
	
	printf("Trying to init usb device\n");
	
	__io_usbstorage.startup();
	
	if (__io_usbstorage.isInserted() == 0) {
		printf("No usb device detected!\n");
		goto let_hang;
	}
	
	printf("Trying to mount usb device\n");
	
	if (fatMountSimple ("fat", &__io_usbstorage) == 0) {
		printf("Couldn't mount usb device\n");
		goto let_hang;
	}
	
	FILE* f = fopen("fat:/boot.elf", "rb");
	if (f == NULL) {
		printf("boot.elf not found!\n");
		goto let_hang;
	}
	
	fseek(f, 0, SEEK_END);
	int len = ftell(f);
	rewind(f);
	
	void* boot_elf_bin = malloc(len);
	fread(boot_elf_bin, 1, len, f);
	fclose(f);
	
	printf("Ready to go!\n");
	
	boot_elf(boot_elf_bin, len);

let_hang:
	while(1) {
		VIDEO_WaitVSync();
	}

	return 0;
}
