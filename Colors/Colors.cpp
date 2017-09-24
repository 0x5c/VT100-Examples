#include "targetver.h"
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <wchar.h>
#include <Windows.h>


#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#define ESC "\x1b"
#define CSI "\x1b["

bool EnableVTMode() {
	// Set output mode to handle virtual terminal sequences
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	DWORD dwMode = 0;
	if (!GetConsoleMode(hOut, &dwMode))
	{
		return false;
	}

	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	if (!SetConsoleMode(hOut, dwMode))
	{
		return false;
	}
	return true;
}

void changeBuffer(int const buffer) {
	switch (buffer) {
	case 0:
		printf(CSI "?1049l");							//Switch to the main buffer
		break;
	case 1:
		printf(CSI "?1049h");							//Switch to the alternate buffer
		break;
	default:
		printf(CSI "?1049l");							//Defaults to the main buffer
		break;
	}
}

void clearBuffer() {
	printf(CSI "2J");
}

void moveto(int const x, int const y) {
	printf(CSI "%d;%dH", y, x);							//Move to x,y
}

void setScrollMargins(int const topMargin, int const botomMargin, COORD const Size) {
	printf(CSI "%d;%dr", topMargin, Size.Y - botomMargin);
}

void printTitleBar(char* const titleBarMessage, int const foreground, int const background, COORD const Size) {
	printf(CSI "0;0H");									//Move to 0,0
	printf(CSI "%d;%dm", foreground, background);		//Set background and foreground
	for (int i = 1; i < Size.X - 1; i++) printf(" ");	//Fill line with spaces
	printf(CSI "0;0H");									//Move to 0;0
	printf(titleBarMessage);							//Title bar text
	printf(CSI "0m");									//Reset color
}

void PrintVerticalBorder()
{
	printf(ESC "(0");       // Enter Line drawing mode
	printf(CSI "104;93m");   // bright yellow on bright blue
	printf("x");            // in line drawing mode, \x78 -> \u2502 "Vertical Bar"
	printf(CSI "0m");       // restore color
	printf(ESC "(B");       // exit line drawing mode
}

void PrintHorizontalBorder(COORD const Size, bool fIsTop)
{
	printf(ESC "(0");       // Enter Line drawing mode
	printf(CSI "104;93m");  // Make the border bright yellow on bright blue
	printf(fIsTop ? "l" : "m"); // print left corner 

	for (int i = 1; i < Size.X - 1; i++)
		printf("q"); // in line drawing mode, \x71 -> \u2500 "HORIZONTAL SCAN LINE-5"

	printf(fIsTop ? "k" : "j"); // print right corner
	printf(CSI "0m");
	printf(ESC "(B");       // exit line drawing mode
}

void PrintStatusLine(char* const pszMessage, COORD const Size)
{
	printf(CSI "%d;1H", Size.Y);
	printf(CSI "K"); // clear the line
	printf(pszMessage);
}

int main() {

	// Set output mode to handle virtual terminal sequences
	#pragma region VT_en 
	bool fSuccess = EnableVTMode();
	if (!fSuccess)
	{
		printf("Unable to enter VT processing mode. Quitting.\n");
		return -1;
	}
	#pragma endregion 

	//Get buffer info
	#pragma region buff_info 
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE) {
		printf("Couldn't get the console handle. Quitting.\n");
		return -1;
	}
	CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo;
	GetConsoleScreenBufferInfo(hOut, &ScreenBufferInfo);
	COORD Size;
	Size.X = ScreenBufferInfo.srWindow.Right - ScreenBufferInfo.srWindow.Left + 1;
	Size.Y = ScreenBufferInfo.srWindow.Bottom - ScreenBufferInfo.srWindow.Top + 1;
	#pragma endregion

	//"Initialize" the console
	SetConsoleTitle(TEXT("VT100 Colors Demo"));			//Set window title
	changeBuffer(1);									//Opens the alternate buffer (1)
	clearBuffer();										//Clears the buffer

	setScrollMargins(2, 0, Size);
	printTitleBar(" VT100 Colors\t\tAll sequences are CSI and end with \'M\'", 30, 42, Size);		//Print title bar (foreground Green/background Black)

	moveto(0, 2);										//Move to second line

	//Display color palette
	printf("Modifiers:\n");
	printf("Default: All:0 Foreground:39 Background:49\n");
	printf("Bold/Bright: " CSI "1m1" CSI "0m\n");
	printf("Negative/Positive: " CSI "7m 7 " CSI "27m/ 27" CSI "0m\n");
	printf("Colors:\n");
	
	//Print the color table
	{
		int foreground[16] = { 30, 31, 32, 33, 34, 35, 36, 37, 90, 91, 92, 93, 94, 95, 96, 97 };
		int background[16] = { 40, 41, 42, 43, 44, 45, 46, 47, 100, 101, 102, 103, 104, 105, 106, 107 };

		//Print the frist line (foreground numbers)
		printf("\t");									//Padding
		for each (int i in foreground) {	
			if (i == 30) { 
				printf(CSI "47m");						//Illisibility mitigation
				printf(CSI "%dm %d ", i, i);
				printf(CSI "0m");
			}
			else {
				printf(CSI "%dm %d ", i, i);
			}
		}
		printf(CSI "0m\n");								//Reset color and move to next line

		//Print the actual color table
		for each (int i in background)
		{
			//Padding
			if (i < 100) {
				printf("    ");
			} 
			else {
				printf("   ");
			}

			//Illisibility mitigation (again)
			if (i > 45 || i == 42) {
				printf(CSI "30m");
			}

			printf(CSI "%dm %d ", i, i);				//Background number

			//Print each foreground color in the line
			for each (int i in foreground) {
				printf(CSI "%dm ** ", i);
			}

			printf(CSI "0m \n");						//Reset color and move to next line
		}
	}

	//Wait for user
	moveto(0, Size.Y);									//Move to status line
	system("pause");

	//Reset buffer and exit
	changeBuffer(0);									//Switch back to the main buffer
	return 0;
}