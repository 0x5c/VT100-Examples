#include "targetver.h"

//Headers
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <wchar.h>
#include <Windows.h>

#include <iostream>
using namespace std;

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#define ESC "\x1b"
#define CSI "\x1b["


bool EnableVTMode()
{
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

	//Get buffer info (Size)
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
	SetConsoleTitle(TEXT("Title bar test"));			//Set window title
	changeBuffer(1);									//Opens the alternate buffer (1)
	clearBuffer();										//Clears the buffer
	
	//"Set the table"
	setScrollMargins(2, 1, Size);						//Set scroll margins
	printTitleBar(" Title bar (scroll margins demo)", 30, 42, Size);			//Print the title bar (background green, foreground black)
	PrintStatusLine("Here is a status line", Size);		//Print the status line

	//Count to 99 (and print it)
	moveto(0, Size.Y - 1);								//Move to last line
	{
		int count;
		count = 0;
		do {
			cout << "\n" << count;
			count++;
			Sleep(50);
		} while (count < 100);
	}
	
	//Wait for user
	moveto(0, Size.Y);									//Move to status line
	system("pause");

	//Reset buffer and exit
	changeBuffer(0);									//Switch back to the main buffer
	return 0;
}

