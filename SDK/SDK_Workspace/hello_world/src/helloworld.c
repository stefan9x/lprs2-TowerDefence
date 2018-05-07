/*
 * Copyright (c) 2009-2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 *
 *
 *
 */

#include <stdio.h>
#include "platform.h"
#include "xparameters.h"
#include "xio.h"
#include "xil_exception.h"
#include "vga_periph_mem.h"
#include "towerdefence_sprites.h"
#include <stdlib.h>     /* srand, rand */
#include <time.h>

#define UP 0b01000000
#define DOWN 0b00000100
#define LEFT 0b00100000
#define RIGHT 0b00001000
#define CENTER 0b00010000
#define SW0 0b00000001
#define SW1 0b00000010

#define SIZEX 30
#define SIZEY 40
#define TOWERU '1'
#define TOWERD '2'
#define GRASS 'G'
#define DIRT 'D'
#define BUSH 'B'
#define NUMOFTOWERS 8
#define NUMOFBUSHES 20

int i, x, y, ii, oi, R, G, B, RGB, kolona, red, RGBgray;
int randomCounter = 50;

char map[SIZEX][SIZEY];

void init(){
	VGA_PERIPH_MEM_mWriteMemory(
				XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x00, 0x0); // direct mode   0
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x04, 0x3); // display_mode  1
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x08, 0x0); // show frame      2
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x0C, 0xff); // font size       3
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x10, 0xFFFFFF); // foreground 4
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x14, 0x0000FF); // background color 5
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x18, 0xFF0000); // frame color      6
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x20, 1);
}

//function that generates random game map
void makeMap() {
	int numOfTowers = NUMOFTOWERS, numOfBushes = NUMOFBUSHES;
	int row, column, i, j, rnd;

	srand(randomCounter);

	//generise teren
	for (i = 0; i < SIZEX; i++) {
		for (j = 0; j < SIZEY; j++) {
			if (i == 15) map[i][j] = DIRT;
			else map[i][j] = GRASS;
		}
	}

	//postavlja random tornjeve
	while (numOfTowers > 0) {
		rnd = rand() % 2;
		if (rnd == 0) row = 13;
		else row = 16;

		if (row == 13) {
			map[column][row] = TOWERU;
			map[column][row + 1] = TOWERD;
			numOfTowers--;
		}

		if (row == 16) {
			map[column][row] = TOWERU;
			map[column][row + 1] = TOWERD;
			numOfTowers--;
		}
	}

	while (numOfBushes > 0) {
		row = rand() % 30;
		column = rand() % 40;

		if (map[column][row] == GRASS) {
			map[column][row] = BUSH;
			numOfBushes--;
		}
	}

	//for testing

	for (i = 0; i < SIZEX; i++) {
		for (j = 0; j < SIZEY; j++) {
			xil_printf("%c", map[i][j]);
		}
		xil_printf("\n");
	}

}

//extracting pixel data from a picture for printing out on the display

void drawSprite(int sprite_pos_x, int sprite_pos_y, int display_out_x, int display_out_y, int width, int height) {
	int ox, oy, oi, iy, ix, ii;
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			ox = display_out_x + x;
			oy = display_out_y + y;
			oi = oy * 640 + ox;
			ix = sprite_pos_x + x;
			iy = sprite_pos_y + y;
			ii = iy * towerdefence_sprites.width + ix;
			R = towerdefence_sprites.pixel_data[ii
					* towerdefence_sprites.bytes_per_pixel] >> 5;
			G = towerdefence_sprites.pixel_data[ii
					* towerdefence_sprites.bytes_per_pixel + 1] >> 5;
			B = towerdefence_sprites.pixel_data[ii
					* towerdefence_sprites.bytes_per_pixel + 2] >> 5;
			R <<= 6;
			G <<= 3;
			RGB = R | G | B;

			VGA_PERIPH_MEM_mWriteMemory(
					XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF
							+ oi, RGB);
		}
	}

}

int main() {

	init_platform();
	init();

	//black background
	/*for (x = 0; x < 640; x++) {
		for (y = 0; y < 480; y++) {
			i = y * 640 + x;
			VGA_PERIPH_MEM_mWriteMemory(
					XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF
							+ i * 4, 0x000000);
		}
	}*/

	makeMap();

	//drawing a map
	while(1){
		for (kolona = 0; kolona < SIZEX; kolona++) {
			for (red = 0; red < SIZEY; red++) {
				if (map[kolona][red] == GRASS){
					drawSprite(16, 0, red * 16, kolona * 16, 16, 16);
				}
				if (map[kolona][red] == DIRT) {
					drawSprite(0, 0, red * 16, kolona * 16, 16, 16);
				}
				if (map[kolona][red] == TOWERU) {
					drawSprite(32, 0, red * 16, kolona * 16, 16, 16);
				}
				if (map[kolona][red] == TOWERD) {
					drawSprite(32, 16, red * 16, kolona * 16, 16, 16);
				}
				if (map[kolona][red] == BUSH){
					drawSprite(64, 0, red * 16, kolona * 16, 16, 16);
				}
			}
		}
		cleanup_platform();
	}


	return 0;
}
