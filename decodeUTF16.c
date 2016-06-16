/*
**This program will  take a file containing utf-16
**charecters and translate them into utf-32 charecters
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************
** This is a helper function to throw an error if
** we hit EOL early
** input:
** 	Received charecter
** Output:
** 	Void
*/
void errorIfEOLChar(unsigned int nextChar)
{
	if(nextChar == EOF)
	{
		printf("Error: Met an EOL charecter early while processing");
		exit(-1);
	}
}

/***************************
** This function will check the BOM of a UTF file
** If it is big endian will return true, else false.
** Input:
** 	Pointer to a file
** Output:
** 	Whether the file is big endian
** Error:
** 	No BOM will report an error and exit
*/
int checkBigEndian(FILE *inputFile)
{
	unsigned int firstChar = getc(inputFile);
	if(firstChar == EOF)
	{
		printf("Error, no BOM\n");
	}
	unsigned int secondChar = getc(inputFile);
	if(secondChar == EOF)
	{
		printf("Error, no BOM\n");
	}
	if((firstChar == 0xFEu) & (secondChar == 0xFFu))
	{
		return 1;
	}
	if((firstChar == 0xFFu) & (secondChar == 0xFEu))
	{
		return 0;
	}
	printf("Error: No BOM\n");
	exit(-1);
}

/*******************************
** This will print the BOM to the output file
** Input:
** 	Int declaring isBigEndian?
**	The path to the file
** Output
** 	void
*/
void printBOMToFile(int isBigEndian, FILE *outputFile)
{
	if(isBigEndian)
	{
		putc(0x00,outputFile);
		putc(0x00,outputFile);
		putc(0xFE,outputFile);
		putc(0xFF,outputFile);
	}
	else
	{
		putc(0xFF,outputFile);
		putc(0xFE,outputFile);
		putc(0x00,outputFile);
		putc(0x00,outputFile);
	}
}

/********************************
 ** This will print a long (32 bits) to file
 ** Input:
 ** 	One long (32 bits)
 ** 	File to write to
 ** Output:
 ** 	void
 ** Effect:
 ** 	Write int to file
 */
void writeLongToFile(int isBigEndian, unsigned long toWrite, FILE *outputFile)
{
  if(isBigEndian)
    {
      printf("%lx\n", toWrite);
      printf("%lx\n", toWrite >> 24);
      printf("%lx\n", (toWrite & 0xFFFFFFu) >> 16);
      printf("%x\n", ((unsigned int) ((toWrite & 0xFFFFu) >> 8)));
      printf("%x\n", ((unsigned int) ((toWrite & 0xFFu))));
      putc( ((unsigned int)(toWrite >> 24)), outputFile);
      putc( ((unsigned int) ((toWrite & 0xFFFFFFu) >> 16))  ,outputFile);
      putc( ((unsigned int) ((toWrite & 0xFFFFu) >> 8))  ,outputFile);
      putc( ((unsigned int) ((toWrite & 0xFFu))),outputFile);
    }
  else
    {
      putc( (unsigned int) ((toWrite & 0xFFu))  ,outputFile);
      putc( (unsigned int) ((toWrite & 0xFFFFu) >> 8)  ,outputFile);
      putc( (unsigned int) ((toWrite & 0xFFFFFFu) >> 16)  ,outputFile);
      putc( (unsigned int)(toWrite >> 24), outputFile);
    }
}


/*********************************
** We need to get the full integer corresponding to a two block
** Input:
** 	The first charecter
**	The second charecter
**	wether it is big endian or little
** Output
**	A UTF-16 two block
*/
unsigned int fullInteger(unsigned int firstChar, unsigned int secondChar, int isBigEndian)
{
	//If its big endian then the first char is most significant, else second
	if(isBigEndian)
	{
		return (firstChar << 8) + secondChar;
	}
	return (secondChar << 8) + firstChar;
}

/*****************************************
** This will determine if a 32 bit long is
** a noncharecter
** Input:
** 	A long
** Output:
** 	void
** Effect:
** 	Error if not a charecter
*/
void isLongOk(unsigned long possibleLong)
{
	unsigned int intPart = (unsigned long) possibleLong; 
	//check all planes, really we need only look at the int part
	if( (intPart == 0xFFFE) | (intPart == 0xFFFF))
	{
		printf("Error: Non-charecter found");
		exit(-1);
	}
	//If in the BMP check invalid region
	if(((unsigned long) intPart) == possibleLong)
	{
		if( (intPart >= 0xFDD0u) & (intPart <= 0xFDEF))
		{
			printf("Error: Non-charecter found");
			exit(-1);
		}
	}
}

int main(int argc, char *argv[])
{
	//We first need to check that we have the right number of input elements
	if(argc != 3)
	{
		printf("Incorrect number of arguments");
		exit(-1);
	}

	//Correct number of files, where they different?
	if(!strcmp(argv[1],argv[2]))
	{
		printf("The input and output files cannot be the same");
		exit(-1);
	}

	//Let open the files for reading and writing
	//then check that the files were valid
	FILE *inputFile = fopen(argv[1],"r");
	FILE *outputFile = fopen(argv[2],"w");

	if(inputFile == NULL)
	{
		printf("The input file could not be opened");
		exit(-1);
	}

	if(outputFile == NULL)
	{
		printf("The output file could not be opened");
		exit(-1);
	}

	//Test the BOM and if correct write the BOM to the output file
	int isBigEndian = checkBigEndian(inputFile);
       	printBOMToFile(isBigEndian, outputFile);
	//We process the rest in a loop
	unsigned int firstChar = getc(inputFile);
	while(firstChar != EOF)
	{
		//We will need at least two charecters (8 bits a piece) for a valid utf element
		unsigned int secondChar = getc(inputFile);
		errorIfEOLChar(secondChar);
		unsigned int firstInt = fullInteger(firstChar, secondChar, isBigEndian);
		//We should check if we are done getting a utf element
		if( (firstInt < 0xD800u) | (firstInt > 0xDFFFu))
		{
			unsigned long fullChar = (((unsigned long) secondChar) << 8) + (unsigned long)firstChar;
			//check if this is a valid charecter
			isLongOk(fullChar);
			//This is BMP charecter, we need to write as utf 8
			writeLongToFile(isBigEndian, fullChar, outputFile);
		} //If this if didn't go then we have determined that this is not in the BMP
		else
		{
			//Get the second integer
			unsigned int thirdChar = getc(inputFile);
			errorIfEOLChar(thirdChar);
			unsigned int fourthChar = getc(inputFile);
			errorIfEOLChar(fourthChar);
			unsigned int secondInt = fullInteger(thirdChar, fourthChar, isBigEndian);
			if( (secondInt < 0xDC00u) | (secondInt > 0xDFFFu))
			{
				printf("Error: The input file had an invalid trail surrogate");
				exit(-1);
			}
			//Next transform
			unsigned int tempFirst = firstInt - 0xD800u;
			unsigned int tempSecond = secondInt - 0xDC00u;
			unsigned int firstTenBitsOfF = tempFirst & 0x3FFu;
			unsigned int firstTenBitsOfS = tempSecond & 0x3FFu;
			unsigned long total = (((unsigned long) firstTenBitsOfF) << 10)
						+ ((unsigned long) firstTenBitsOfS);
			unsigned long fullChar = total + 0x10000u;
			//We now have the real value, we need to check validity and write it to the output file
			isLongOk(fullChar);
			writeLongToFile(isBigEndian, fullChar, outputFile);
		}
		firstChar = getc(inputFile);
	}
}
