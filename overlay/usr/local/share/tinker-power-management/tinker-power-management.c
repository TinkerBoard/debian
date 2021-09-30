/*Tinker-Power-Management Version 1.02*/

#include <ncurses.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#define SQA_HEIGHT 21
#define SQC_HEIGHT 19
#define SQ_WIDTH (COLS/9)
#define LEFT (COLS%9)

#define A_EXTEND 3
#define SUB_SHIFT_R 4
#define SUBB_SHIFT_D 5
#define SUBC_SHIFT_D 4

#define MAX_CPU_SQ 4
#define MAX_A17_FREQ 13

#define MAX_GPU_SQ 4
#define MAX_T76X_FREQ 4

#define SQFORM_WIDTH ((SQ_WIDTH * 5 + LEFT - A_EXTEND - 3) / 5)

WINDOW *BOARDA;
WINDOW *BOARDB;
WINDOW *BOARDC;

WINDOW *FORMA[6];
WINDOW *FORMB[6];
WINDOW *FORMC[6];
WINDOW *FORMD[6];

WINDOW *CPUGOVER[4];
WINDOW *A17FREQ[13];

WINDOW *GPUGOVER[4];
WINDOW *T76XFREQ[4];

FILE *fp;
char buffer[50];
int curr_cpu_menu = 0,curr_gpu_menu = 0, cpu_mode = 0, gpu_mode = 0;
int curr_cpu_governor, cpu_dirty;
int curr_gpu_governor, gpu_dirty;
int switch_tag, curr_tag, priority = 0;

int a17_min, a17_max;
int t76x_min, t76x_max;

int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
	if(ch != EOF) {
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
}

void draw_square_A(void) {
	mvwprintw(BOARDA, 1, 2, "Device Info");

	fp = popen("cat /proc/boardinfo", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	mvwprintw(BOARDA, 3, 3,"Name\t\t= %s", buffer);

	fp = popen("cat /proc/boardver", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	mvwprintw(BOARDA, 4, 3,"Version\t= %s", buffer);

	fp = popen("cat /proc/device-tree/serial-number", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	mvwprintw(BOARDA, 5, 3,"SN/PPID\t= %s", buffer);

	mvwprintw(BOARDA, 7, 3,"SoC/CPU\t= RK3399");

	fp = popen("cat /proc/ddr", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	mvwprintw(BOARDA, 8, 3,"Memory\t= %s", buffer);

	fp = popen("lsblk | grep -w 'mmcblk1'; echo $?", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	if (atoi(buffer) == 0) {
		fp = popen("lsblk | grep -w 'mmcblk1' | grep 'disk' | rev | cut -d ' ' -f5 | rev", "r");
		memset(buffer, '\0', sizeof(buffer));
		fgets(buffer, sizeof(buffer), fp);
		pclose(fp);
		mvwprintw(BOARDA, 9, 3,"Storage\t= eMMC(mmcblk1)\t\t%s", buffer);
	} else
		mvwprintw(BOARDA, 9, 3,"Storage\t= No eMMC(mmcblk1)");

	fp = popen("lsblk | grep -w 'mmcblk0'; echo $?", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	if (atoi(buffer) == 0) {
		fp = popen("lsblk | grep -w 'mmcblk0' | grep 'disk' | rev | cut -d ' ' -f5 | rev", "r");
		memset(buffer, '\0', sizeof(buffer));
		fgets(buffer, sizeof(buffer), fp);
		pclose(fp);
		mvwprintw(BOARDA, 10, 3,"\t\t  SD card(mmcblk0)\t%s", buffer);
	} else
		mvwprintw(BOARDA, 10, 3,"\t\t  No SD card(mmcblk0)\t     ");

	fp = popen("cat /etc/os-release | grep -w 'NAME=' | cut -d '\"' -f2", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	mvwprintw(BOARDA, 12, 3,"OS\t\t= %s", buffer);

	fp = popen("cat /etc/os-release | grep 'VERSION=' | cut -d '\"' -f2", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	mvwprintw(BOARDA, 13, 3,"Version\t= %s", buffer);

	fp = popen("lscpu | grep 'Architecture' | rev | cut -d ' ' -f1 | rev", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	mvwprintw(BOARDA, 14, 3,"Platform\t= %s", buffer);

	fp = popen("cat /etc/version", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	mvwprintw(BOARDA, 16, 3,"Build\t= %s", buffer);

	mvwprintw(BOARDA, 17, 3,"Kernel:");

	fp = popen("cat /proc/version | awk {print'$1\" \"$2\" \"$3'}", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	mvwprintw(BOARDA, 18, 5,"Release\t= %s", buffer);

	fp = popen("cat /proc/version | awk {print'$13\" \"$14\" \"$15\" \"$16\" \"$17\" \"$18\" \"$19\" \"$20'}", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	mvwprintw(BOARDA, 19, 5,"Version\t= %s", buffer);

	box(BOARDA, 0, 0);
}

/* mode: 1 for the page to select governor, so need to flash original governor.
 * caller: 0 from cpu governor, 1 from gpu governor.				*/
void draw_square_B(int mode, int caller) {
	mvwprintw(BOARDB, 1, 2, "System Config");

	mvwprintw(BOARDB, 11, 3,"GPU:");
	mvwprintw(BOARDB, 12, 5,"Governor\t= ");

	fp = popen("cat /sys/class/devfreq/ffa30000.gpu/governor", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);

	if (t76x_min == 200 && t76x_max == 600)
		gpu_dirty = 0;
	else
		gpu_dirty = 1;

	if (!strncmp(buffer, "simple_ondemand", strlen("simple_ondemand"))) {
		if (curr_gpu_menu)
			curr_gpu_governor = 1;
		else if (gpu_dirty == 0)
			curr_gpu_governor = 0;
		else {
			curr_gpu_governor = 1;
			curr_gpu_menu = 1;
			priority = 1;
		}
	} else if (!strncmp(buffer, "powersave", strlen("powersave")))
		curr_gpu_governor = 2;
	else if (!strncmp(buffer, "performance", strlen("performance")))
		curr_gpu_governor = 3;

	if (mode == 1 && caller == 1)
		mvwprintw(BOARDB, 12, 18,"            ");
	else {
		wattron(BOARDB, A_BOLD);

		switch(curr_gpu_governor) {
			case 0:
				mvwprintw(BOARDB, 12, 18,"auto");
				break;
			case 1:
				mvwprintw(BOARDB, 12, 18,"manual");
				break;
			default:
				mvwprintw(BOARDB, 12, 18,"%s", buffer);
		}

		wattroff(BOARDB, A_BOLD);
	}

	mvwprintw(BOARDB, 3, 3,"CPU:");

	mvwprintw(BOARDB, 4, 5,"Governor\t= ");

	fp = popen("cat /sys/devices/system/cpu/cpufreq/policy0/scaling_governor", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);

	if (a17_min == 126 && a17_max == 1800)
		cpu_dirty = 0;
	else
		cpu_dirty = 1;

	if (!strncmp(buffer, "ondemand", strlen("ondemand"))) {
		if (curr_cpu_menu)
			curr_cpu_governor = 1;
		else if (cpu_dirty == 0)
			curr_cpu_governor = 0;
		else {
			curr_cpu_governor = 1;
			curr_cpu_menu = 1;
			priority = 0;
		}
	} else if (!strncmp(buffer, "powersave", strlen("powersave")))
		curr_cpu_governor = 2;
	else if (!strncmp(buffer, "performance", strlen("performance")))
		curr_cpu_governor = 3;

	if (mode == 1 && caller == 0)
		mvwprintw(BOARDB, 4, 18,"            ");
	else {
		wattron(BOARDB, A_BOLD);

		switch(curr_cpu_governor) {
			case 0:
				mvwprintw(BOARDB, 4, 18,"auto");
				break;
			case 1:
				mvwprintw(BOARDB, 4, 18,"manual");
				break;
			default:
				mvwprintw(BOARDB, 4, 18,"%s", buffer);
		}

		wattroff(BOARDB, A_BOLD);
	}

	box(BOARDB, 0, 0);
}

void draw_square_C(void) {
	mvwprintw(BOARDC, 1, 2,"Monitor");
	mvwprintw(BOARDC, 3, 3,"CPU:");
	mvwprintw(BOARDC, 10, 3,"GPU:");

	mvwprintw(BOARDC, 3, SQFORM_WIDTH * 5 + SUB_SHIFT_R * 2 - 2, "CPU usage:");
	mvwprintw(BOARDC, 5, SQFORM_WIDTH * 5 + SUB_SHIFT_R * 2 - 2, "[");
	mvwprintw(BOARDC, 5, SQFORM_WIDTH * 5 + SUB_SHIFT_R * 2 - 2 + 51, "]");

	mvwprintw(BOARDC, 7, SQFORM_WIDTH * 5 + SUB_SHIFT_R * 2 - 2, "GPU usage:");
	mvwprintw(BOARDC, 9, SQFORM_WIDTH * 5 + SUB_SHIFT_R * 2 - 2, "[");
	mvwprintw(BOARDC, 9, SQFORM_WIDTH * 5 + SUB_SHIFT_R * 2 - 2 + 51, "]");

	mvwprintw(BOARDC, 11, SQFORM_WIDTH * 5 + SUB_SHIFT_R * 2 - 2, "Memory usage:");
	mvwprintw(BOARDC, 13, SQFORM_WIDTH * 5 + SUB_SHIFT_R * 2 - 2, "[");
	mvwprintw(BOARDC, 13, SQFORM_WIDTH * 5 + SUB_SHIFT_R * 2 - 2 + 51, "]");

	box(BOARDC, 0, 0);
}

void draw_square_FORMAC(int sq) {
	if (sq == 0) {
		wborder(FORMA[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_ULCORNER, ACS_TTEE, ACS_LTEE, ACS_PLUS);
		wborder(FORMC[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_ULCORNER, ACS_TTEE, ACS_LTEE, ACS_PLUS);
	} else if (sq == 5) {
		wborder(FORMA[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_LTEE, ACS_PLUS, ACS_LLCORNER, ACS_BTEE);
		wborder(FORMC[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_LTEE, ACS_PLUS, ACS_LLCORNER, ACS_BTEE);

		fp = popen("cat /proc/cpuinfo | grep 'CPU part' | grep -c '0xc0d'", "r");
		memset(buffer, '\0', sizeof(buffer));
		fgets(buffer, sizeof(buffer), fp);
		pclose(fp);
		mvwprintw(FORMA[sq], 1, 2,"%d x Arm Cortex-A17", atoi(buffer));
		mvwprintw(FORMC[sq], 1, 2,"%d x Arm Cortex-A17", atoi(buffer));
	} else if (sq == 3) {
		wborder(FORMA[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_TTEE, ACS_TTEE, ACS_PLUS, ACS_PLUS);
		wborder(FORMC[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_TTEE, ACS_TTEE, ACS_PLUS, ACS_PLUS);

		mvwprintw(FORMA[sq], 1, (SQFORM_WIDTH - 10) / 2,"Min. freq.");
		mvwprintw(FORMC[sq], 1, (SQFORM_WIDTH - 11) / 2,"Curr. freq.");
	} else if (sq == 1) {
		wborder(FORMA[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_PLUS, ACS_PLUS, ACS_BTEE, ACS_BTEE);
		wborder(FORMC[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_PLUS, ACS_PLUS, ACS_BTEE, ACS_BTEE);

		fp = popen("cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq", "r");
		memset(buffer, '\0', sizeof(buffer));
		fgets(buffer, sizeof(buffer), fp);
		pclose(fp);

		a17_min = atoi(buffer) / 1000;
		mvwprintw(FORMA[sq], 1, (SQFORM_WIDTH - 3) / 2,"%d ", a17_min);

		switch (a17_min) {
			case 126:
				curr_tag = 0;
				break;
			case 216:
				curr_tag = 1;
				break;
			case 408:
				curr_tag = 2;
				break;
			case 600:
				curr_tag = 3;
				break;
			case 696:
				curr_tag = 4;
				break;
			case 816:
				curr_tag = 5;
				break;
			case 1008:
				curr_tag = 6;
				break;
			case 1200:
				curr_tag = 7;
				break;
			case 1416:
				curr_tag = 8;
				break;
			case 1512:
				curr_tag = 9;
				break;
			case 1608:
				curr_tag = 10;
				break;
			case 1704:
				curr_tag = 11;
				break;
			case 1800:
				curr_tag = 12;
				break;
                }
	} else if (sq == 4) {
		wborder(FORMA[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_TTEE, ACS_URCORNER, ACS_PLUS, ACS_RTEE);
		wborder(FORMC[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_TTEE, ACS_URCORNER, ACS_PLUS, ACS_RTEE);

		mvwprintw(FORMA[sq], 1, (SQFORM_WIDTH - 10) / 2,"Max. freq.");
		mvwprintw(FORMC[sq], 1, (SQFORM_WIDTH - 11) / 2,"Temperature");
	} else if (sq == 2) {
		wborder(FORMA[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_PLUS, ACS_RTEE, ACS_BTEE, ACS_LRCORNER);
		wborder(FORMC[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_PLUS, ACS_RTEE, ACS_BTEE, ACS_LRCORNER);

		fp = popen("cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", "r");
		memset(buffer, '\0', sizeof(buffer));
		fgets(buffer, sizeof(buffer), fp);
		pclose(fp);

		a17_max = atoi(buffer) / 1000;
		mvwprintw(FORMA[sq], 1, (SQFORM_WIDTH - 4) / 2,"%d ", a17_max);

		switch (a17_max) {
			case 126:
				curr_tag = 0;
				break;
			case 216:
				curr_tag = 1;
				break;
			case 408:
				curr_tag = 2;
				break;
			case 600:
				curr_tag = 3;
				break;
			case 696:
				curr_tag = 4;
				break;
			case 816:
				curr_tag = 5;
				break;
			case 1008:
				curr_tag = 6;
				break;
			case 1200:
				curr_tag = 7;
				break;
			case 1416:
				curr_tag = 8;
				break;
			case 1512:
				curr_tag = 9;
				break;
			case 1608:
				curr_tag = 10;
				break;
			case 1704:
				curr_tag = 11;
				break;
			case 1800:
				curr_tag = 12;
				break;
		}
	}
}

void draw_square_FORMBD(int sq) {
	if (sq == 0) {
		wborder(FORMB[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_ULCORNER, ACS_TTEE, ACS_LTEE, ACS_PLUS);
		wborder(FORMD[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_ULCORNER, ACS_TTEE, ACS_LTEE, ACS_PLUS);
	} else if (sq == 5) {
		wborder(FORMB[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_LTEE, ACS_PLUS, ACS_LLCORNER, ACS_BTEE);
		wborder(FORMD[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_LTEE, ACS_PLUS, ACS_LLCORNER, ACS_BTEE);

		fp = popen("cat /sys/class/devfreq/ffa30000.gpu/device/gpuinfo | cut -d ' ' -f1", "r");
		memset(buffer, '\0', sizeof(buffer));
		fgets(buffer, sizeof(buffer), fp);
		pclose(fp);

		mvwprintw(FORMB[sq], 1, 2,"Arm %s", buffer);
		mvwprintw(FORMD[sq], 1, 2,"Arm %s", buffer);
	} else if (sq == 3) {
		wborder(FORMB[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_TTEE, ACS_TTEE, ACS_PLUS, ACS_PLUS);
		wborder(FORMD[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_TTEE, ACS_TTEE, ACS_PLUS, ACS_PLUS);

		mvwprintw(FORMB[sq], 1, (SQFORM_WIDTH - 10) / 2,"Min. freq.");
		mvwprintw(FORMD[sq], 1, (SQFORM_WIDTH - 11) / 2,"Curr. freq.");
	} else if (sq == 1) {
		wborder(FORMB[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_PLUS, ACS_PLUS, ACS_BTEE, ACS_BTEE);
		wborder(FORMD[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_PLUS, ACS_PLUS, ACS_BTEE, ACS_BTEE);

		fp = popen("cat /sys/class/devfreq/ffa30000.gpu/min_freq", "r");
		memset(buffer, '\0', sizeof(buffer));
		fgets(buffer, sizeof(buffer), fp);
		pclose(fp);

		t76x_min = atoi(buffer) / 1000000;
		mvwprintw(FORMB[sq], 1, (SQFORM_WIDTH - 3) / 2,"%d", t76x_min);

		switch (t76x_min) {
			case 200:
				curr_tag = 0;
				break;
			case 300:
				curr_tag = 1;
				break;
			case 420:
				curr_tag = 2;
				break;
			case 600:
				curr_tag = 3;
				break;
		}
	} else if (sq == 4) {
		wborder(FORMB[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_TTEE, ACS_URCORNER, ACS_PLUS, ACS_RTEE);
		wborder(FORMD[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_TTEE, ACS_URCORNER, ACS_PLUS, ACS_RTEE);

		mvwprintw(FORMB[sq], 1, (SQFORM_WIDTH - 10) / 2,"Max. freq.");
		mvwprintw(FORMD[sq], 1, (SQFORM_WIDTH - 11) / 2,"Temperature");
	} else if (sq == 2) {
		wborder(FORMB[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_PLUS, ACS_RTEE, ACS_BTEE, ACS_LRCORNER);
		wborder(FORMD[sq], ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_PLUS, ACS_RTEE, ACS_BTEE, ACS_LRCORNER);

		fp = popen("cat /sys/class/devfreq/ffa30000.gpu/max_freq", "r");
		memset(buffer, '\0', sizeof(buffer));
		fgets(buffer, sizeof(buffer), fp);
		pclose(fp);

		t76x_max = atoi(buffer) / 1000000;
		mvwprintw(FORMB[sq], 1, (SQFORM_WIDTH - 3) / 2,"%d", t76x_max);

		switch (t76x_max) {
			case 200:
				curr_tag = 0;
				break;
			case 300:
				curr_tag = 1;
				break;
			case 420:
				curr_tag = 2;
				break;
			case 600:
				curr_tag = 3;
				break;
		}
	}
}

void draw_square_CPUGOVER(int sq, int mode) {
	if (sq == 0) {
		if (mode)
			mvwprintw(CPUGOVER[sq], 1, 1, "auto");
		else
			mvwprintw(CPUGOVER[sq], 1, 1, "    ");
	} else if (sq == 1) {
		if (mode)
			mvwprintw(CPUGOVER[sq], 1, 1, "manual");
		else
			mvwprintw(CPUGOVER[sq], 1, 1, "      ");
	} else if (sq == 2) {
		if (mode)
			mvwprintw(CPUGOVER[sq], 1, 1, "powersave");
		else
			mvwprintw(CPUGOVER[sq], 1, 1, "         ");
	} else if (sq == 3) {
		if (mode)
			mvwprintw(CPUGOVER[sq], 1, 1, "performance");
		else
			mvwprintw(CPUGOVER[sq], 1, 1, "           ");
	}
}

void draw_square_GPUGOVER(int sq, int mode) {
	if (sq == 0) {
		if (mode)
			mvwprintw(GPUGOVER[sq], 1, 1, "auto");
		else
			mvwprintw(GPUGOVER[sq], 1, 1, "    ");
	} else if (sq == 1) {
		if (mode)
			mvwprintw(GPUGOVER[sq], 1, 1, "manual");
		else
			mvwprintw(GPUGOVER[sq], 1, 1, "      ");
	} else if (sq == 2) {
		if (mode)
			mvwprintw(GPUGOVER[sq], 1, 1, "powersave");
		else
			mvwprintw(GPUGOVER[sq], 1, 1, "         ");
	} else if (sq == 3) {
		if (mode)
			mvwprintw(GPUGOVER[sq], 1, 1, "performance");
		else
			mvwprintw(GPUGOVER[sq], 1, 1, "           ");
	}
}

void draw_square_A17FREQ(int sq, int mode) {
	if (sq == 0) {
		if (mode)
			mvwprintw(A17FREQ[sq], 1, 1, " 126");
		else
			mvwprintw(A17FREQ[sq], 1, 1, "    ");
	} else if (sq == 1) {
		if (mode)
			mvwprintw(A17FREQ[sq], 1, 1, " 216");
		else
			mvwprintw(A17FREQ[sq], 1, 1, "    ");
	} else if (sq == 2) {
		if (mode)
			mvwprintw(A17FREQ[sq], 1, 1, " 408");
		else
			mvwprintw(A17FREQ[sq], 1, 1, "    ");
	} else if (sq == 3) {
		if (mode)
			mvwprintw(A17FREQ[sq], 1, 1, " 600");
		else
			mvwprintw(A17FREQ[sq], 1, 1, "    ");
	} else if (sq == 4) {
		if (mode)
			mvwprintw(A17FREQ[sq], 1, 1, " 696");
		else
			mvwprintw(A17FREQ[sq], 1, 1, "    ");
	} else if (sq == 5) {
		if (mode)
			mvwprintw(A17FREQ[sq], 1, 1, " 816");
		else
			mvwprintw(A17FREQ[sq], 1, 1, "    ");
	} else if (sq == 6) {
		if (mode)
			mvwprintw(A17FREQ[sq], 1, 1, "1008");
		else
			mvwprintw(A17FREQ[sq], 1, 1, "    ");
	} else if (sq == 7) {
		if (mode)
			mvwprintw(A17FREQ[sq], 1, 1, "1200");
		else
			mvwprintw(A17FREQ[sq], 1, 1, "    ");
	} else if (sq == 8) {
		if (mode)
			mvwprintw(A17FREQ[sq], 1, 1, "1416");
		else
			mvwprintw(A17FREQ[sq], 1, 1, "    ");
	} else if (sq == 9) {
		if (mode)
			mvwprintw(A17FREQ[sq], 1, 1, "1512");
		else
			mvwprintw(A17FREQ[sq], 1, 1, "    ");
	} else if (sq == 10) {
		if (mode)
			mvwprintw(A17FREQ[sq], 1, 1, "1608");
		else
			mvwprintw(A17FREQ[sq], 1, 1, "    ");
	} else if (sq == 11) {
		if (mode)
			mvwprintw(A17FREQ[sq], 1, 1, "1704");
		else
			mvwprintw(A17FREQ[sq], 1, 1, "    ");
	} else if (sq == 12) {
		if (mode)
			mvwprintw(A17FREQ[sq], 1, 1, "1800");
		else
			mvwprintw(A17FREQ[sq], 1, 1, "    ");
	}
}

void draw_square_T76XFREQ(int sq, int mode) {
	if (sq == 0) {
		if (mode)
			mvwprintw(T76XFREQ[sq], 1, 1, "200");
		else
			mvwprintw(T76XFREQ[sq], 1, 1, "   ");
	} else if (sq == 1) {
		if (mode)
			mvwprintw(T76XFREQ[sq], 1, 1, "300");
		else
			mvwprintw(T76XFREQ[sq], 1, 1, "   ");
	} else if (sq == 2) {
		if (mode)
			mvwprintw(T76XFREQ[sq], 1, 1, "420");
		else
			mvwprintw(T76XFREQ[sq], 1, 1, "   ");
	} else if (sq == 3) {
		if (mode)
			mvwprintw(T76XFREQ[sq], 1, 1, "600");
		else
			mvwprintw(T76XFREQ[sq], 1, 1, "   ");
	}
}

void highlight_a17_freq(int sq) {
        wattron(A17FREQ[sq], A_BOLD);
        draw_square_A17FREQ(sq, cpu_mode);
        wattroff(A17FREQ[sq], A_BOLD);
}

void highlight_t76x_freq(int sq) {
	wattron(T76XFREQ[sq], A_BOLD);
	draw_square_T76XFREQ(sq, gpu_mode);
	wattroff(T76XFREQ[sq], A_BOLD);
}

/* caller: 0 for cpu, 1 for gpu */
void highlight_square(int sq, int caller) {
	if (caller) {
		wattron(GPUGOVER[sq], A_BOLD);
		draw_square_GPUGOVER(sq, gpu_mode);
		wattroff(GPUGOVER[sq], A_BOLD);
	} else {
		wattron(CPUGOVER[sq], A_BOLD);
		draw_square_CPUGOVER(sq, cpu_mode);
		wattroff(CPUGOVER[sq], A_BOLD);
	}
}

/* caller: 0 for cpu, 1 for gpu */
void highlight_menu(int menu, int caller) {
	if (menu) {
		if (caller) {
			wattron(FORMB[menu], A_BOLD);
			draw_square_FORMBD(menu);
			wattroff(FORMB[menu], A_BOLD);
		} else {
			wattron(FORMA[menu], A_BOLD);
			draw_square_FORMAC(menu);
			wattroff(FORMA[menu], A_BOLD);
		}
	}
}

void create_board(void) {
	int i;
	int startx = 0, starty = 0;

	BOARDA = newwin(SQA_HEIGHT, SQ_WIDTH * 4 + A_EXTEND, starty, startx);
	BOARDB = newwin(SQA_HEIGHT, SQ_WIDTH * 5 + LEFT - A_EXTEND, starty, startx + SQ_WIDTH * 4 + A_EXTEND);
	BOARDC = newwin(SQC_HEIGHT, COLS, starty + SQA_HEIGHT, startx);

	FORMA[0] = derwin(BOARDB, 3, SQFORM_WIDTH * 3, SUBB_SHIFT_D, SUB_SHIFT_R);
	FORMA[5] = derwin(BOARDB, 3, SQFORM_WIDTH * 3, SUBB_SHIFT_D + 2, SUB_SHIFT_R);
	FORMA[3] = derwin(BOARDB, 3, SQFORM_WIDTH, SUBB_SHIFT_D, SUB_SHIFT_R + SQFORM_WIDTH * 3 - 1);
	FORMA[1] = derwin(BOARDB, 3, SQFORM_WIDTH, SUBB_SHIFT_D + 2, SUB_SHIFT_R + SQFORM_WIDTH * 3 - 1);
	FORMA[4] = derwin(BOARDB, 3, SQFORM_WIDTH, SUBB_SHIFT_D, SUB_SHIFT_R + SQFORM_WIDTH * 4 - 2);
	FORMA[2] = derwin(BOARDB, 3, SQFORM_WIDTH, SUBB_SHIFT_D + 2, SUB_SHIFT_R + SQFORM_WIDTH * 4 - 2);

	FORMC[0] = derwin(BOARDC, 3, SQFORM_WIDTH * 3, SUBC_SHIFT_D, SUB_SHIFT_R);
	FORMC[5] = derwin(BOARDC, 3, SQFORM_WIDTH * 3, SUBC_SHIFT_D + 2, SUB_SHIFT_R);
	FORMC[3] = derwin(BOARDC, 3, SQFORM_WIDTH, SUBC_SHIFT_D, SUB_SHIFT_R + SQFORM_WIDTH * 3 - 1);
	FORMC[1] = derwin(BOARDC, 3, SQFORM_WIDTH, SUBC_SHIFT_D + 2, SUB_SHIFT_R + SQFORM_WIDTH * 3 - 1);
	FORMC[4] = derwin(BOARDC, 3, SQFORM_WIDTH, SUBC_SHIFT_D, SUB_SHIFT_R + SQFORM_WIDTH * 4 - 2);
	FORMC[2] = derwin(BOARDC, 3, SQFORM_WIDTH, SUBC_SHIFT_D + 2, SUB_SHIFT_R + SQFORM_WIDTH * 4 - 2);

	FORMB[0] = derwin(BOARDB, 3, SQFORM_WIDTH * 3, SUBB_SHIFT_D + 8, SUB_SHIFT_R);
	FORMB[5] = derwin(BOARDB, 3, SQFORM_WIDTH * 3, SUBB_SHIFT_D + 8 + 2, SUB_SHIFT_R);
	FORMB[3] = derwin(BOARDB, 3, SQFORM_WIDTH, SUBB_SHIFT_D + 8, SUB_SHIFT_R + SQFORM_WIDTH * 3 - 1);
	FORMB[1] = derwin(BOARDB, 3, SQFORM_WIDTH, SUBB_SHIFT_D + 8 + 2, SUB_SHIFT_R + SQFORM_WIDTH * 3 - 1);
	FORMB[4] = derwin(BOARDB, 3, SQFORM_WIDTH, SUBB_SHIFT_D + 8, SUB_SHIFT_R + SQFORM_WIDTH * 4 - 2);
	FORMB[2] = derwin(BOARDB, 3, SQFORM_WIDTH, SUBB_SHIFT_D + 8 + 2, SUB_SHIFT_R + SQFORM_WIDTH * 4 - 2);

	FORMD[0] = derwin(BOARDC, 3, SQFORM_WIDTH * 3, SUBC_SHIFT_D + 7, SUB_SHIFT_R);
	FORMD[5] = derwin(BOARDC, 3, SQFORM_WIDTH * 3, SUBC_SHIFT_D + 7 + 2, SUB_SHIFT_R);
	FORMD[3] = derwin(BOARDC, 3, SQFORM_WIDTH, SUBC_SHIFT_D + 7, SUB_SHIFT_R + SQFORM_WIDTH * 3 - 1);
	FORMD[1] = derwin(BOARDC, 3, SQFORM_WIDTH, SUBC_SHIFT_D + 7 + 2, SUB_SHIFT_R + SQFORM_WIDTH * 3 - 1);
	FORMD[4] = derwin(BOARDC, 3, SQFORM_WIDTH, SUBC_SHIFT_D + 7, SUB_SHIFT_R + SQFORM_WIDTH * 4 - 2);
	FORMD[2] = derwin(BOARDC, 3, SQFORM_WIDTH, SUBC_SHIFT_D + 7 + 2, SUB_SHIFT_R + SQFORM_WIDTH * 4 - 2);

	CPUGOVER[0] = derwin(BOARDB, 3, sizeof(" auto "), SUBB_SHIFT_D - 2, 17);
	CPUGOVER[1] = derwin(BOARDB, 3, sizeof(" manual "), SUBB_SHIFT_D - 2, 17 + sizeof(" auto ") - 1);
	CPUGOVER[2] = derwin(BOARDB, 3, sizeof(" powersave "), SUBB_SHIFT_D - 2, 17 + sizeof(" auto ") + sizeof(" manual ") - 2);
	CPUGOVER[3] = derwin(BOARDB, 3, sizeof(" performance "), SUBB_SHIFT_D - 2, 17 + sizeof(" auto ") + sizeof(" manual ") + sizeof(" powersave ") - 3);

	GPUGOVER[0] = derwin(BOARDB, 3, sizeof(" auto "), SUBB_SHIFT_D + 6, 17);
	GPUGOVER[1] = derwin(BOARDB, 3, sizeof(" manual "), SUBB_SHIFT_D + 6, 17 + sizeof(" auto ") - 1);
	GPUGOVER[2] = derwin(BOARDB, 3, sizeof(" powersave "), SUBB_SHIFT_D + 6, 17 + sizeof(" auto ") + sizeof(" manual ") - 2);
	GPUGOVER[3] = derwin(BOARDB, 3, sizeof(" performance "), SUBB_SHIFT_D + 6, 17 + sizeof(" auto ") + sizeof(" manual ") + sizeof(" powersave ") - 3);

	for (i = 0; i < 13; i++)		// sizeof(" xxxx ")
		A17FREQ[i] = derwin(BOARDB, 3, 6, SUBB_SHIFT_D + 4, SUB_SHIFT_R + i * 6 - i);

	for (i = 0; i < 4; i++)			// sizeof(" xxx ")
		T76XFREQ[i] = derwin(BOARDB, 3, 5, SUBB_SHIFT_D + 12, SUB_SHIFT_R + i * 5 - i);

	draw_square_A();
	draw_square_C();

	for (i = 5; i >= 0; i--)
		draw_square_FORMAC(i);
	for (i = 5; i >= 0; i--)
		draw_square_FORMBD(i);

	draw_square_B(0, 2);

	if (priority)
		highlight_menu(curr_gpu_menu, priority);
	else
		highlight_menu(curr_cpu_menu, priority);
}

void cpu_info(void) {
	int reg;
	double temp;

	fp = popen("sudo cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	mvwprintw(FORMC[1], 1, (SQFORM_WIDTH - 3) / 2, "%d ", atoi(buffer) / 1000);

	fp = popen("cat /sys/class/thermal/thermal_zone0/temp", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	temp = (float)(atoi(buffer)) / 1000;
	mvwprintw(FORMC[2], 1, (SQFORM_WIDTH - 7) / 2, "%.2f°C", temp);

	fp = popen("top -b -n2 -d0.1 | grep 'Cpu(s)' | awk '{print $2+$4+$6+$14}' | tail -n1", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	mvwprintw(BOARDC, 3, SQFORM_WIDTH * 5 + SUB_SHIFT_R * 2 + 11, "%d%%  ", atoi(buffer));

	for (reg = 1; reg <= 50; reg++) {
		if (reg > atoi(buffer) / 2)
			mvwprintw(BOARDC, 5, SQFORM_WIDTH * 5 + SUB_SHIFT_R * 2 - 2 + reg, " ");
		else
			mvwprintw(BOARDC, 5, SQFORM_WIDTH * 5 + SUB_SHIFT_R * 2 - 2 + reg, "#");
	}
}

void gpu_info(void) {
	int reg;
	double temp;

	fp = popen("cat /sys/class/devfreq/ffa30000.gpu/cur_freq", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	mvwprintw(FORMD[1], 1, (SQFORM_WIDTH - 3) / 2, "%d ", atoi(buffer) / 1000000);

	fp = popen("cat /sys/class/thermal/thermal_zone1/temp", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	temp = (float)(atoi(buffer)) / 1000;
	mvwprintw(FORMD[2], 1, (SQFORM_WIDTH - 7) / 2, "%.2f°C", temp);

	fp = popen("cat /sys/devices/platform/ffa30000.gpu/utilisation", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	mvwprintw(BOARDC, 7, SQFORM_WIDTH * 5 + SUB_SHIFT_R * 2 + 11, "%d%%  ", atoi(buffer));

	for (reg = 1; reg <= 50; reg++) {
		if (reg > atoi(buffer) / 2)
			mvwprintw(BOARDC, 9, SQFORM_WIDTH * 5 + SUB_SHIFT_R * 2 - 2 + reg, " ");
		else
			mvwprintw(BOARDC, 9, SQFORM_WIDTH * 5 + SUB_SHIFT_R * 2 - 2 + reg, "#");
	}
}

void ddr_info(void) {
	int reg;
	int mem_total, mem_used, mem_usage;

	fp = popen("lsblk | grep -w 'mmcblk1'; echo $?", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	if (atoi(buffer) == 0) {
		fp = popen("lsblk | grep -w 'mmcblk1' | grep 'disk' | rev | cut -d ' ' -f5 | rev", "r");
		memset(buffer, '\0', sizeof(buffer));
		fgets(buffer, sizeof(buffer), fp);
		pclose(fp);
		mvwprintw(BOARDA, 9, 10,"\t= eMMC(mmcblk1)\t\t%s", buffer);
	} else
		mvwprintw(BOARDA, 9, 10,"\t= No eMMC(mmcblk1)");

	fp = popen("lsblk | grep -w 'mmcblk0'; echo $?", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	if (atoi(buffer) == 0) {
		fp = popen("lsblk | grep -w 'mmcblk0' | grep 'disk' | rev | cut -d ' ' -f5 | rev", "r");
		memset(buffer, '\0', sizeof(buffer));
		fgets(buffer, sizeof(buffer), fp);
		pclose(fp);
		mvwprintw(BOARDA, 10, 3,"\t\t  SD card(mmcblk0)\t%s", buffer);
	} else
		mvwprintw(BOARDA, 10, 3,"\t\t  No SD card(mmcblk0)\t     ");

	box(BOARDA, 0, 0);

	fp = popen("cat /proc/meminfo | grep 'MemTotal' | rev | cut -d ' ' -f2 | rev", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	mem_total = atoi(buffer) / 1000;

	fp = popen("cat /proc/meminfo | grep 'MemAvailable' | rev | cut -d ' ' -f2 | rev", "r");
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	mem_used = mem_total - atoi(buffer) / 1000;

	mem_usage = mem_used * 100 / mem_total;

	mvwprintw(BOARDC, 11, SQFORM_WIDTH * 5 + SUB_SHIFT_R * 2 + 14, "                     ");	//MAX:21

	mvwprintw(BOARDC, 11, SQFORM_WIDTH * 5 + SUB_SHIFT_R * 2 + 14, "%d / %d MB (%d%%)", mem_used, mem_total, mem_usage);

	for (reg = 1; reg <= 50; reg++) {
		if (reg > mem_usage / 2)
			mvwprintw(BOARDC, 13, SQFORM_WIDTH * 5 + SUB_SHIFT_R * 2 - 2 + reg, " ");
		else
			mvwprintw(BOARDC, 13, SQFORM_WIDTH * 5 + SUB_SHIFT_R * 2 - 2 + reg, "#");
	}
}

int main() {
	int i;
	char key;

	setlocale(LC_ALL,"");

	initscr();
	noecho();

	create_board();

	refresh();
	for (i = 0; i < 6; i++) {
		wrefresh(FORMA[i]);
		wrefresh(FORMC[i]);
	}
	for (i = 0; i < 6; i++) {
		wrefresh(FORMB[i]);
		wrefresh(FORMD[i]);
	}
	wrefresh(BOARDA);
	wrefresh(BOARDB);
	wrefresh(BOARDC);

	while (1) {
		if (!kbhit()) {
			if (cpu_mode == 1 || gpu_mode == 1) {
				if (LINES > 40)
					mvprintw(LINES - 1, 0, "Press Left or Right keys to select the governor and Space key to save. Press Q to quit and go back.                       ");
				else if (LINES == 40) {
					box(BOARDC, 0, 0);
					mvprintw(LINES - 1, 0, "Press Left or Right keys to select the governor and Space key to save. Press Q to quit and go back.");
					wrefresh(BOARDC);
				}
			} else if (cpu_mode == 0 && gpu_mode == 0) {
				if (curr_cpu_governor == 1 || curr_gpu_governor == 1) {
					if (LINES > 40)
						mvprintw(LINES - 1, 0, "Press (C)PU or (G)PU to change governor, Ctrl + C to exit. Use Arrow keys and Space key to select the frequency to change.");
					else if (LINES == 40) {
						box(BOARDC, 0, 0);
						mvprintw(LINES - 1, 0, "Press (C)PU or (G)PU to change governor, Ctrl + C to exit. Use Arrow keys and Space key to select the frequency to change.");
						wrefresh(BOARDC);
					}
				} else {
					if (LINES > 40)
						mvprintw(LINES - 1, 0, "Press (C)PU or (G)PU to change governor, Ctrl + C to exit.                                                                ");
					else if (LINES == 40) {
						box(BOARDC, 0, 0);
						mvprintw(LINES - 1, 0, "Press (C)PU or (G)PU to change governor, Ctrl + C to exit.");
						wrefresh(BOARDC);
					}
				}
			} else {
				if (LINES > 40)
					mvprintw(LINES - 1, 0, "Press Left or Right keys to select the frequency and Space key to save. Press Q to quit and go back.                      ");
				else if (LINES == 40) {
					box(BOARDC, 0, 0);
					mvprintw(LINES - 1, 0, "Press Left or Right keys to select the frequency and Space key to save. Press Q to quit and go back.");
					wrefresh(BOARDC);
				}
			}

			refresh();

			cpu_info();
			gpu_info();
			ddr_info();
		} else {
			key = getchar();

			if (cpu_mode == 1) {
				if (key == ' ') {
					if (switch_tag != 1 && cpu_dirty == 1) {
						fp = popen("sudo su -c \"echo 126000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq\"", "r");
						pclose(fp);

						fp = popen("sudo sed -i 's/\\(a17_minfreq=\\).*/\\1126000/g' /boot/config.txt", "r");
						pclose(fp);

						fp = popen("sudo su -c \"echo 1800000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq\"", "r");
						pclose(fp);

						fp = popen("sudo sed -i 's/\\(a17_maxfreq=\\).*/\\11800000/g' /boot/config.txt", "r");
						pclose(fp);

						for (i = 1; i <= 2; i++)
							draw_square_FORMAC(i);
					}

					switch(switch_tag) {
						case 0:
							curr_cpu_menu = 0;

							fp = popen("sudo su -c \"echo ondemand > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor\"", "r");
							pclose(fp);

							fp = popen("sudo sed -i 's/\\(cpu_governor=\\).*/\\1ondemand/g' /boot/config.txt", "r");
							pclose(fp);

							fp = popen("sudo su -c \"echo enabled > /sys/class/thermal/thermal_zone0/mode\"", "r");
							pclose(fp);
							break;
						case 1:
							curr_cpu_menu = 1;
							priority = 0;
							highlight_menu(curr_cpu_menu, priority);

							fp = popen("sudo su -c \"echo ondemand > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor\"", "r");
							pclose(fp);

							fp = popen("sudo sed -i 's/\\(cpu_governor=\\).*/\\1ondemand/g' /boot/config.txt", "r");
							pclose(fp);

							fp = popen("sudo su -c \"echo enabled > /sys/class/thermal/thermal_zone0/mode\"", "r");
							pclose(fp);
							break;
						case 2:
							curr_cpu_menu = 0;

							fp = popen("sudo su -c \"echo powersave > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor\"", "r");
							pclose(fp);

							fp = popen("sudo sed -i 's/\\(cpu_governor=\\).*/\\1powersave/g' /boot/config.txt", "r");
							pclose(fp);

							fp = popen("sudo su -c \"echo enabled > /sys/class/thermal/thermal_zone0/mode\"", "r");
							pclose(fp);
							break;
						case 3:
							curr_cpu_menu = 0;

							fp = popen("sudo su -c \"echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor\"", "r");
							pclose(fp);

							fp = popen("sudo sed -i 's/\\(cpu_governor=\\).*/\\1performance/g' /boot/config.txt", "r");
							pclose(fp);

							fp = popen("sudo su -c \"echo disabled > /sys/class/thermal/thermal_zone0/mode\"", "r");
							pclose(fp);
							fp = popen("sudo su -c \"echo 0 > /sys/class/thermal/thermal_zone0/cdev0/cur_state\"", "r");
							pclose(fp);
							fp = popen("sudo su -c \"echo 0 > /sys/class/thermal/thermal_zone0/cdev1/cur_state\"", "r");
							pclose(fp);
							break;
					}
				}

				if (key == 'q' || key == ' ') {
					cpu_mode = 0;

					for (i = 0; i < 4; i++)
						draw_square_CPUGOVER(i, cpu_mode);
					draw_square_B(cpu_mode, 0);
					highlight_menu(curr_cpu_menu, 0);

					if (curr_cpu_governor != 1 && curr_gpu_governor == 1) {
						priority = 1;
						highlight_menu(curr_gpu_menu, priority);
					}
				} else if (key == 'C') {	//right
					draw_square_CPUGOVER(switch_tag, cpu_mode);

					if (switch_tag < MAX_CPU_SQ - 1)
						highlight_square(++switch_tag, 0);
					else {
						switch_tag = 0;
						highlight_square(switch_tag, 0);
					}
				} else if (key == 'D') {	//left
					draw_square_CPUGOVER(switch_tag, cpu_mode);

					if (switch_tag > 0)
						highlight_square(--switch_tag, 0);
					else {
						switch_tag = MAX_CPU_SQ - 1;
						highlight_square(switch_tag, 0);
					}
				}
			} else if (cpu_mode == 2 || cpu_mode == 3) {
				if (key == ' ') {
					cpu_dirty = 1;
					switch(switch_tag) {
						case 0:
							if (cpu_mode == 2 && a17_max >= 126 ) {
								fp = popen("sudo su -c \"echo 126000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_minfreq=\\).*/\\1126000/g' /boot/config.txt", "r");
								pclose(fp);

								cpu_dirty = 0;
							} else if (cpu_mode == 3 && a17_min <= 126) {
								fp = popen("sudo su -c \"echo 126000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_maxfreq=\\).*/\\1126000/g' /boot/config.txt", "r");
								pclose(fp);
							}
							break;
						case 1:
							if (cpu_mode == 2 && a17_max >= 216 ) {
								fp = popen("sudo su -c \"echo 216000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_minfreq=\\).*/\\1216000/g' /boot/config.txt", "r");
								pclose(fp);
							} else if (cpu_mode == 3 && a17_min <= 216) {
								fp = popen("sudo su -c \"echo 216000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_maxfreq=\\).*/\\1216000/g' /boot/config.txt", "r");
								pclose(fp);
							}
							break;
						case 2:
							if (cpu_mode == 2 && a17_max >= 408 ) {
								fp = popen("sudo su -c \"echo 408000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_minfreq=\\).*/\\1408000/g' /boot/config.txt", "r");
								pclose(fp);
							} else if (cpu_mode == 3 && a17_min <= 408) {
								fp = popen("sudo su -c \"echo 408000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_maxfreq=\\).*/\\1408000/g' /boot/config.txt", "r");
								pclose(fp);
							}
							break;
						case 3:
							if (cpu_mode == 2 && a17_max >= 600 ) {
								fp = popen("sudo su -c \"echo 600000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_minfreq=\\).*/\\1600000/g' /boot/config.txt", "r");
								pclose(fp);
							} else if (cpu_mode == 3 && a17_min <= 600) {
								fp = popen("sudo su -c \"echo 600000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_maxfreq=\\).*/\\1600000/g' /boot/config.txt", "r");
								pclose(fp);
							}
							break;
						case 4:
							if (cpu_mode == 2 && a17_max >= 696 ) {
								fp = popen("sudo su -c \"echo 696000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_minfreq=\\).*/\\1696000/g' /boot/config.txt", "r");
								pclose(fp);
							} else if (cpu_mode == 3 && a17_min <= 696) {
								fp = popen("sudo su -c \"echo 696000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_maxfreq=\\).*/\\1696000/g' /boot/config.txt", "r");
								pclose(fp);
							}
							break;
						case 5:
							if (cpu_mode == 2 && a17_max >= 816 ) {
								fp = popen("sudo su -c \"echo 816000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_minfreq=\\).*/\\1816000/g' /boot/config.txt", "r");
								pclose(fp);
							} else if (cpu_mode == 3 && a17_min <= 816) {
								fp = popen("sudo su -c \"echo 816000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_maxfreq=\\).*/\\1816000/g' /boot/config.txt", "r");
								pclose(fp);
							}
							break;
						case 6:
							if (cpu_mode == 2 && a17_max >= 1008 ) {
								fp = popen("sudo su -c \"echo 1008000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_minfreq=\\).*/\\11008000/g' /boot/config.txt", "r");
								pclose(fp);
							} else if (cpu_mode == 3 && a17_min <= 1008) {
								fp = popen("sudo su -c \"echo 1008000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_maxfreq=\\).*/\\11008000/g' /boot/config.txt", "r");
								pclose(fp);
							}
							break;
						case 7:
							if (cpu_mode == 2 && a17_max >= 1200 ) {
								fp = popen("sudo su -c \"echo 1200000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_minfreq=\\).*/\\11200000/g' /boot/config.txt", "r");
								pclose(fp);
							} else if (cpu_mode == 3 && a17_min <= 1200) {
								fp = popen("sudo su -c \"echo 1200000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_maxfreq=\\).*/\\11200000/g' /boot/config.txt", "r");
								pclose(fp);
							}
							break;
						case 8:
							if (cpu_mode == 2 && a17_max >= 1416 ) {
								fp = popen("sudo su -c \"echo 1416000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_minfreq=\\).*/\\11416000/g' /boot/config.txt", "r");
								pclose(fp);
							} else if (cpu_mode == 3 && a17_min <= 1416) {
								fp = popen("sudo su -c \"echo 1416000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_maxfreq=\\).*/\\11416000/g' /boot/config.txt", "r");
								pclose(fp);
							}
							break;
						case 9:
							if (cpu_mode == 2 && a17_max >= 1512 ) {
								fp = popen("sudo su -c \"echo 1512000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_minfreq=\\).*/\\11512000/g' /boot/config.txt", "r");
								pclose(fp);
							} else if (cpu_mode == 3 && a17_min <= 1512) {
								fp = popen("sudo su -c \"echo 1512000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_maxfreq=\\).*/\\11512000/g' /boot/config.txt", "r");
								pclose(fp);
							}
							break;
						case 10:
							if (cpu_mode == 2 && a17_max >= 1608 ) {
								fp = popen("sudo su -c \"echo 1608000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_minfreq=\\).*/\\11608000/g' /boot/config.txt", "r");
								pclose(fp);
							} else if (cpu_mode == 3 && a17_min <= 1608) {
								fp = popen("sudo su -c \"echo 1608000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_maxfreq=\\).*/\\11608000/g' /boot/config.txt", "r");
								pclose(fp);
							}
							break;
						case 11:
							if (cpu_mode == 2 && a17_max >= 1704 ) {
								fp = popen("sudo su -c \"echo 1704000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_minfreq=\\).*/\\11704000/g' /boot/config.txt", "r");
								pclose(fp);
							} else if (cpu_mode == 3 && a17_min <= 1704) {
								fp = popen("sudo su -c \"echo 1704000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_maxfreq=\\).*/\\11704000/g' /boot/config.txt", "r");
								pclose(fp);
							}
							break;
						case 12:
							if (cpu_mode == 2 && a17_max >= 1800 ) {
								fp = popen("sudo su -c \"echo 1800000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_minfreq=\\).*/\\11800000/g' /boot/config.txt", "r");
								pclose(fp);
							} else if (cpu_mode == 3 && a17_min <= 1800) {
								fp = popen("sudo su -c \"echo 1800000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(a17_maxfreq=\\).*/\\11800000/g' /boot/config.txt", "r");
								pclose(fp);

								cpu_dirty = 0;
							}
							break;
					}
				}

				if (key == 'q' || key == ' ') {
					cpu_mode = 0;

					for (i = 0; i < MAX_A17_FREQ; i++)
						draw_square_A17FREQ(i, cpu_mode);
					highlight_menu(curr_cpu_menu, 0);
				} else if (key == 'C') {        //right
					draw_square_A17FREQ(switch_tag, cpu_mode);

					if (switch_tag < MAX_A17_FREQ - 1)
						highlight_a17_freq(++switch_tag);
					else {
						switch_tag = 0;
						highlight_a17_freq(switch_tag);
					}
				} else if (key == 'D') {        //left
					draw_square_A17FREQ(switch_tag, cpu_mode);

					if (switch_tag > 0)
						highlight_a17_freq(--switch_tag);
					else {
						switch_tag = MAX_A17_FREQ - 1;
						highlight_a17_freq(switch_tag);
					}
				}
			} else if (cpu_mode == 0 && gpu_mode == 0) {
				if (key == 'c') {
					cpu_mode = 1;
					priority = 0;

					draw_square_B(cpu_mode, 0);

					for (i = 0; i < 4; i++)
						draw_square_CPUGOVER(i, cpu_mode);

					for (i = 1; i <= 2; i++)
						draw_square_FORMAC(i);

					for (i = 1; i <= 2; i++)
						draw_square_FORMBD(i);

					highlight_square(curr_cpu_governor, priority);

					switch_tag = curr_cpu_governor;
				} else if (key == 'g') {
					gpu_mode = 1;
					priority = 1;

					draw_square_B(gpu_mode, 1);

					for (i = 0; i < 4; i++)
						draw_square_GPUGOVER(i, gpu_mode);

					for (i = 1; i <= 2; i++)
						draw_square_FORMBD(i);

					for (i = 1; i <= 2; i++)
						draw_square_FORMAC(i);

					highlight_square(curr_gpu_governor, priority);

					switch_tag = curr_gpu_governor;
				} else if (key == ' ') {
					if (curr_cpu_governor == 1 || curr_gpu_governor == 1) {
						if (priority) {
							gpu_mode = curr_gpu_menu + 1;

							switch(curr_gpu_menu) {
								case 1:
								case 2:
									for (i = 0; i < MAX_T76X_FREQ; i++)
										draw_square_T76XFREQ(i, gpu_mode);

									highlight_t76x_freq(curr_tag);
									break;
							}

							switch_tag = curr_tag;
						} else {
							cpu_mode = curr_cpu_menu + 1;

							switch(curr_cpu_menu) {
								case 1:
								case 2:
									for (i = 0; i < MAX_A17_FREQ; i++)
										draw_square_A17FREQ(i, cpu_mode);

									highlight_a17_freq(curr_tag);
									break;
							}

							switch_tag = curr_tag;
						}
					}
				} else if (curr_cpu_governor == 1 && curr_gpu_governor == 1) {

					if (key == 'A' || key == 'B') {	//up, down
						if (priority) {
							draw_square_FORMBD(curr_gpu_menu);

							if (curr_gpu_menu == 1) {
								curr_cpu_menu = 1;
								priority = 0;
							} else if (curr_gpu_menu == 2) {
								curr_cpu_menu = 2;
								priority = 0;
							}
						} else {
							draw_square_FORMAC(curr_cpu_menu);

							if (curr_cpu_menu == 1) {
								curr_gpu_menu = 1;
								priority = 1;
							} else if (curr_cpu_menu == 2) {
								curr_gpu_menu = 2;
								priority = 1;
							}
						}

						if (priority)
							highlight_menu(curr_gpu_menu, priority);
						else
							highlight_menu(curr_cpu_menu, priority);

					} else if (key == 'C' || key == 'D') {
						if (priority) {
							draw_square_FORMBD(curr_gpu_menu);

							if (curr_gpu_menu == 1)
								curr_gpu_menu = 2;
							else if (curr_gpu_menu == 2)
								curr_gpu_menu = 1;

							highlight_menu(curr_gpu_menu, priority);
						} else {
							draw_square_FORMAC(curr_cpu_menu);

							if (curr_cpu_menu == 1)
								curr_cpu_menu = 2;
							else if (curr_cpu_menu == 2)
								curr_cpu_menu = 1;

							highlight_menu(curr_cpu_menu, priority);
						}
					}
				} else if (curr_cpu_governor == 1) {		//left, right
					if (key == 'C' || key == 'D') {
						draw_square_FORMAC(curr_cpu_menu);

						if (curr_cpu_menu == 1)
							curr_cpu_menu = 2;
						else if (curr_cpu_menu == 2)
							curr_cpu_menu = 1;

						highlight_menu(curr_cpu_menu, 0);
					}
				} else if (curr_gpu_governor == 1) {
					if (key == 'C' || key == 'D') {		//left, right
						draw_square_FORMBD(curr_gpu_menu);

						if (curr_gpu_menu == 1)
							curr_gpu_menu = 2;
						else if (curr_gpu_menu == 2)
							curr_gpu_menu = 1;

						highlight_menu(curr_gpu_menu, 1);
					}
				}
			} else if (gpu_mode == 1) {
				if (key == ' ') {
					if (switch_tag != 1 && gpu_dirty == 1) {
						fp = popen("sudo su -c \"echo 200000000 > /sys/class/devfreq/ffa30000.gpu/min_freq\"", "r");
						pclose(fp);

						fp = popen("sudo sed -i 's/\\(t76x_minfreq=\\).*/\\1200000000/g' /boot/config.txt", "r");
						pclose(fp);

						fp = popen("sudo su -c \"echo 600000000 > /sys/class/devfreq/ffa30000.gpu/max_freq\"", "r");
						pclose(fp);

						fp = popen("sudo sed -i 's/\\(t76x_maxfreq=\\).*/\\1600000000/g' /boot/config.txt", "r");
						pclose(fp);

						for (i = 1; i <= 2; i++)
							draw_square_FORMBD(i);
					}

					switch(switch_tag) {
						case 0:
							curr_gpu_menu = 0;

							fp = popen("sudo su -c \"echo simple_ondemand > /sys/class/devfreq/ffa30000.gpu/governor\"", "r");
							pclose(fp);

							fp = popen("sudo sed -i 's/\\(gpu_governor=\\).*/\\1simple_ondemand/g' /boot/config.txt", "r");
							pclose(fp);

							fp = popen("sudo su -c \"echo enabled > /sys/class/thermal/thermal_zone1/mode\"", "r");
							pclose(fp);
							break;
						case 1:
							curr_gpu_menu = 1;
							priority = 1;
							highlight_menu(curr_gpu_menu, priority);

							fp = popen("sudo su -c \"echo simple_ondemand > /sys/class/devfreq/ffa30000.gpu/governor\"", "r");
							pclose(fp);

							fp = popen("sudo sed -i 's/\\(gpu_governor=\\).*/\\1simple_ondemand/g' /boot/config.txt", "r");
							pclose(fp);

							fp = popen("sudo su -c \"echo enabled > /sys/class/thermal/thermal_zone1/mode\"", "r");
							pclose(fp);
							break;
						case 2:
							curr_gpu_menu = 0;

							fp = popen("sudo su -c \"echo powersave > /sys/class/devfreq/ffa30000.gpu/governor\"", "r");
							pclose(fp);

							fp = popen("sudo sed -i 's/\\(gpu_governor=\\).*/\\1powersave/g' /boot/config.txt", "r");
							pclose(fp);

							fp = popen("sudo su -c \"echo enabled > /sys/class/thermal/thermal_zone1/mode\"", "r");
							pclose(fp);
							break;
						case 3:
							curr_gpu_menu = 0;

							fp = popen("sudo su -c \"echo performance > /sys/class/devfreq/ffa30000.gpu/governor\"", "r");
							pclose(fp);

							fp = popen("sudo sed -i 's/\\(gpu_governor=\\).*/\\1performance/g' /boot/config.txt", "r");
							pclose(fp);

							fp = popen("sudo su -c \"echo disabled > /sys/class/thermal/thermal_zone1/mode\"", "r");
							pclose(fp);
							break;
					}
				}

				if (key == 'q' || key == ' ') {
					gpu_mode = 0;

					for (i = 0; i < 4; i++)
						draw_square_GPUGOVER(i, gpu_mode);
					draw_square_B(gpu_mode, 1);
					highlight_menu(curr_gpu_menu, 1);

					if (curr_gpu_governor != 1 && curr_cpu_governor == 1) {
						priority = 0;
						highlight_menu(curr_cpu_menu, priority);
					}
				} else if (key == 'C') {	//right
					draw_square_GPUGOVER(switch_tag, gpu_mode);

					if (switch_tag < MAX_GPU_SQ - 1)
						highlight_square(++switch_tag, 1);
					else {
						switch_tag = 0;
						highlight_square(switch_tag, 1);
					}
				} else if (key == 'D') {	//left
					draw_square_GPUGOVER(switch_tag, gpu_mode);

					if (switch_tag > 0)
						highlight_square(--switch_tag, 1);
					else {
						switch_tag = MAX_GPU_SQ - 1;
						highlight_square(switch_tag, 1);
					}
				}
			} else if (gpu_mode == 2 || gpu_mode == 3) {
				if (key == ' ') {
					gpu_dirty = 1;
					switch(switch_tag) {
						case 0:
							if (gpu_mode == 2 && t76x_max >= 200 ) {
								fp = popen("sudo su -c \"echo 200000000 > /sys/class/devfreq/ffa30000.gpu/min_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(t76x_minfreq=\\).*/\\1200000000/g' /boot/config.txt", "r");
								pclose(fp);

								gpu_dirty = 0;
							} else if (gpu_mode == 3 && t76x_min <= 200) {
								fp = popen("sudo su -c \"echo 200000000 > /sys/class/devfreq/ffa30000.gpu/max_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(t76x_maxfreq=\\).*/\\1200000000/g' /boot/config.txt", "r");
								pclose(fp);
							}
							break;
						case 1:
							if (gpu_mode == 2 && t76x_max >= 300 ) {
								fp = popen("sudo su -c \"echo 300000000 > /sys/class/devfreq/ffa30000.gpu/min_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(t76x_minfreq=\\).*/\\1300000000/g' /boot/config.txt", "r");
								pclose(fp);
							} else if (gpu_mode == 3 && t76x_min <= 300) {
								fp = popen("sudo su -c \"echo 300000000 > /sys/class/devfreq/ffa30000.gpu/max_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(t76x_maxfreq=\\).*/\\1300000000/g' /boot/config.txt", "r");
								pclose(fp);
							}
							break;
						case 2:
							if (gpu_mode == 2 && t76x_max >= 420 ) {
								fp = popen("sudo su -c \"echo 420000000 > /sys/class/devfreq/ffa30000.gpu/min_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(t76x_minfreq=\\).*/\\1420000000/g' /boot/config.txt", "r");
								pclose(fp);
							} else if (gpu_mode == 3 && t76x_min <= 420) {
								fp = popen("sudo su -c \"echo 420000000 > /sys/class/devfreq/ffa30000.gpu/max_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(t76x_maxfreq=\\).*/\\1420000000/g' /boot/config.txt", "r");
								pclose(fp);
							}
							break;
						case 3:
							if (gpu_mode == 2 && t76x_max >= 600 ) {
								fp = popen("sudo su -c \"echo 600000000 > /sys/class/devfreq/ffa30000.gpu/min_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(t76x_minfreq=\\).*/\\1600000000/g' /boot/config.txt", "r");
								pclose(fp);
							} else if (gpu_mode == 3 && t76x_min <= 600) {
								fp = popen("sudo su -c \"echo 600000000 > /sys/class/devfreq/ffa30000.gpu/max_freq\"", "r");
								pclose(fp);

								fp = popen("sudo sed -i 's/\\(t76x_maxfreq=\\).*/\\1600000000/g' /boot/config.txt", "r");
								pclose(fp);

								gpu_dirty = 0;
							}
							break;
					}
				}

				if (key == 'q' || key == ' ') {
					gpu_mode = 0;

					for (i = 0; i < MAX_T76X_FREQ; i++)
						draw_square_T76XFREQ(i, gpu_mode);
					highlight_menu(curr_gpu_menu, 1);
				} else if (key == 'C') {        //right
					draw_square_T76XFREQ(switch_tag, gpu_mode);

					if (switch_tag < MAX_T76X_FREQ - 1)
						highlight_t76x_freq(++switch_tag);
					else {
						switch_tag = 0;
						highlight_t76x_freq(switch_tag);
					}
				} else if (key == 'D') {        //left
					draw_square_T76XFREQ(switch_tag, gpu_mode);

					if (switch_tag > 0)
						highlight_t76x_freq(--switch_tag);
					else {
						switch_tag = MAX_T76X_FREQ - 1;
						highlight_t76x_freq(switch_tag);
					}
				}
			}
		}

		refresh();

		for (i = 0; i < 6; i++) {
			wrefresh(FORMA[i]);
			wrefresh(FORMC[i]);
		}

		for (i = 0; i < 6; i++) {
			wrefresh(FORMB[i]);
			wrefresh(FORMD[i]);
		}

		for (i = 0; i < MAX_CPU_SQ; i++)
			wrefresh(CPUGOVER[i]);

		for (i = 0; i < MAX_GPU_SQ; i++)
			wrefresh(GPUGOVER[i]);

		for (i = 0; i < MAX_A17_FREQ; i++)
			wrefresh(A17FREQ[i]);

		for (i = 0; i < MAX_T76X_FREQ; i++)
			wrefresh(T76XFREQ[i]);

		wrefresh(BOARDA);
		wrefresh(BOARDB);
		wrefresh(BOARDC);
	}

	delwin(BOARDA);
	delwin(BOARDB);
	delwin(BOARDC);

	endwin();
	return 0;
}
