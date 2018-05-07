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
#include "minesweeper_sprites.h"
#include <stdlib.h>     /* srand, rand */
#include <time.h>

#define UP 0b01000000
#define DOWN 0b00000100
#define LEFT 0b00100000
#define RIGHT 0b00001000
#define CENTER 0b00010000
#define SW0 0b00000001
#define SW1 0b00000010

#define SIZEX 40
#define SIZEY 30
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
int res = 0;


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
	for (column = 0; column < SIZEX; column++) {
		for (row = 0; row < SIZEY; row++) {
			if (column == 15) map[column][row] = DIRT;
			else map[column][row] = GRASS;
		}
	}

	//postavlja random tornjeve
	while (numOfTowers > 0) {
		rnd = rand() % 2;
		int a_road = (SIZEX / 2) - 2;
		int b_road = (SIZEX / 2) + 1;
		if (rnd == 0) row = a_road;
		else row = b_road;

		map[column][row] = TOWERU;
		map[column][row + 1] = TOWERD;
		numOfTowers--;

	}

	while (numOfBushes > 0) {
		row = rand() % SIZEX;
		column = rand() % SIZEY;

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

void drawSprite(int in_x, int in_y, int out_x, int out_y, int width, int height) {
	int ox, oy, oi, iy, ix, ii;
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			ox = out_x + x;
			oy = out_y + y;
			oi = oy * 320 + ox;
			ix = in_x + x;
			iy = in_y + y;
			ii = iy * minesweeper_sprites.width + ix;
			R = minesweeper_sprites.pixel_data[ii
					* minesweeper_sprites.bytes_per_pixel] >> 5;
			G = minesweeper_sprites.pixel_data[ii
					* minesweeper_sprites.bytes_per_pixel + 1] >> 5;
			B = minesweeper_sprites.pixel_data[ii
					* minesweeper_sprites.bytes_per_pixel + 2] >> 5;
			R <<= 6;
			G <<= 3;
			RGB = R | G | B;

			VGA_PERIPH_MEM_mWriteMemory(
					XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF
							+ oi * 4, RGB);
		}
	}

}

int main() {

	init_platform();
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
				drawSprite(0, 0, red * 16, kolona * 16, 16,16);
				/*if (map[kolona][red] == GRASS){
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
				*/
			}
		}

	}
	cleanup_platform();

	return 0;
}
