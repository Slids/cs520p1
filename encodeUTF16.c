/**********************************************
** This program will turn a UTF 32 encoded file
** with filename (first entry)
** into a UTF 16 encoded file with file name
** second entry.
**********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//This global variable will be set early and referenced often
//as to whether this is big or small endian
char _ISBIGENDIAN;

/*********************************************
** This function will throw an error on a EOF
** Input:
** 	A charecter that may be an EOF
** Output:
**	void
** Effect:
** 	Throw an error EOF
*/
void errorIfEOF(unsigned int newChar)
{
	if(newChar == EOF)
	{
		printf("Error: Found a EOF when not suppose to");
		exit(-1);
	}
}

/*********************************************
** This function will decide whether the input
** file is big or little endian. It should only
** be used on a newly opened file
** Input:
** 	Pointer to a FILE
** Output:
**	void
** Effect:
**	Sets the global _ISBIGENDIAN flag.
*/
void checkBigEndian(FILE *inputFile)
{
	//Fe must get all of the charecters of the bom and check validity
	unsigned int tempChar0 = getc(inputFile);
	errorIfEOF(tempChar0);
	unsigned int tempChar1 = getc(inputFile);
	errorIfEOF(tempChar1);
	unsigned int tempChar2 = getc(inputFile);
	errorIfEOF(tempChar2);
	unsigned int tempChar3 = getc(inputFile);
	errorIfEOF(tempChar3);

	if((tempChar0 == 0x00u) & (tempChar1 == 0x00u) & (tempChar2 == 0xFEu)
		& (tempChar3 == 0xFFu))
	{
		_ISBIGENDIAN = 1;
		return;
	}
	if((tempChar0 == 0xFFu) & (tempChar1 == 0xFEu) & (tempChar2 == 0x00u)
		& (tempChar3 == 0x00u))
	{
		_ISBIGENDIAN = 0;
		return;
	}
	printf("Error: No BOM");
	exit(-1);
}

/*********************************************
** This function will print a 16 bit entry
** to the output file;
** Input:
** 	The pointer to the file to print to
**	The unsigned int to print
** Output:
**	void
*/
void print16BitCharecterToFile(FILE *outputFile, unsigned int toPrint)
{
	if(_ISBIGENDIAN)
	{
		putc(toPrint >> 8, outputFile);
		putc(toPrint & 0xFFu, outputFile);
	}
	else
	{
		putc(toPrint & 0xFFu, outputFile);
		putc(toPrint >> 8, outputFile);
	}
}

/*********************************************
** This function will print a 32 bit entry
** to the output file;
** Input:
** 	The pointer to the file to print to
**	The unsigned long to print
** Output:
**	void
*/
void print32BitCharecterToFile(FILE *outputFile, unsigned long toPrint)
{
	print16BitCharecterToFile(outputFile, (unsigned int) (toPrint >> 16));
	print16BitCharecterToFile(outputFile, (unsigned int) (toPrint & 0xFFFF));
}

/*********************************************
** This function will get the next word in the input
** file throwing an error if it finds an EOL
** Input:
** 	The pointer to the file to get from
** 	The first unsigned long of the word
** Output:
** 	The UTF-32 charecter
*/
unsigned long getNextLong(FILE *inputFile, unsigned int firstChar)
{
  unsigned int secondChar = getc(inputFile);
  errorIfEOF(secondChar);
  unsigned int thirdChar = getc(inputFile);
  errorIfEOF(thirdChar);
  unsigned int fourthChar = getc(inputFile);
  errorIfEOF(fourthChar);
  if(_ISBIGENDIAN)
    {
      return ((((unsigned long)firstChar) << 24) +
	      (((unsigned long)secondChar) << 16) +
	      (((unsigned long)thirdChar) << 8) +
	      ((unsigned long)fourthChar));
    }
  {
    return (((unsigned long)firstChar) +
	    (((unsigned long)secondChar) << 8) +
	    (((unsigned long)thirdChar) << 16) +
	    (((unsigned long)fourthChar) << 24));
  }
}


/*********************************************
 ** This function will check if a long is a valid
 ** unicode charecter
 ** Input:
 **	The unsigned long
 ** Output:
 **	void
 ** Effect:
 ** 	Error if invalid
 */
void checkIfValidWord(unsigned long word)
{
  if(word > 0x10FFFFu)
    {
      printf("Error: Found a word to large to be unicode");
      exit(-1);
    }
  if(((word == 0xFFFEu) |( word == 0xFFFF)) |
     ((word == 0x1FFFEu) | (word == 0x1FFFF)) |
     ((word == 0x2FFFEu) | (word == 0x2FFFF)) |
     ((word == 0x3FFFEu) | (word == 0x3FFFF)) |
     ((word == 0x4FFFEu) | (word == 0x4FFFF)) |
     ((word == 0x5FFFEu) | (word == 0x5FFFF)) |
     ((word == 0x6FFFEu) | (word == 0x6FFFF)) |
     ((word == 0x7FFFEu) | (word == 0x7FFFF)) |
     ((word == 0x8FFFEu) | (word == 0x8FFFF)) |
     ((word == 0x9FFFEu) | (word == 0x9FFFF)) |
     ((word == 0xAFFFEu) | (word == 0xAFFFF)) |
     ((word == 0xBFFFEu) | (word == 0xBFFFF)) |
     ((word == 0xCFFFEu) | (word == 0xCFFFF)) |
     ((word == 0xDFFFEu) | (word == 0xDFFFF)) |
     ((word == 0xEFFFEu) | (word == 0xEFFFF)) |
     ((word == 0xFFFFEu) | (word == 0xFFFFF)) |
     ((word == 0x10FFEu) | (word == 0x10FFFF)))
    {
      printf("Error: Found a word ending in FFFE or FFFF");
      exit(-1);
    }
  if((word >= 0xFDD0) & (word <= 0xFDDF))
    {
      printf("Error: Found a word between 0xFDD0 and 0xFDDF");
      exit(-1);
    }
}

/*********************************************
** This function will check if a long is in the BMP
** Input:
**	The unsigned long
** Output:
**	Wether or not the word will be a 16 bit or 32 bit word
*/
int checkIf16BitUTF16(unsigned long word)
{
	if(word < 0xFFFFu)
	{
		return 1;
	}
	else
	{
	  return 0;
	}
}

/*********************************************
** This function will transform a Unicode charecter
** to a long representing a UTF-16 2 part charecter
** Input:
**	The unsigned long
** Output:
**	Wether or not the word will be a 16 bit or 32 bit word
*/
unsigned long transformWordToUnicode(unsigned long word)
{
	word -= 0x10000;
	unsigned int firstChar = word >> 10;
	unsigned int secondChar = word & 0x3FF;

	firstChar += 0xD800u;
	secondChar += 0xDC00u;
	return ((unsigned long)secondChar) + (((unsigned long) firstChar) << 16);
}

/*********************************************
** This function prints a BOM to the underlying file
** Input:
**	The unsigned long
** Output:
**      Void
*/
void printBOMToFile(FILE *outputFile)
{
  print16BitCharecterToFile(outputFile, 0xFEFFu);
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

        //Lets open the files for reading and writing
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
        checkBigEndian(inputFile);
        printBOMToFile(outputFile);

        //We process the rest in a loop
        unsigned int firstChar = getc(inputFile);
        while(firstChar != EOF)
	{
		//Get the next word (we already have the first part
		unsigned long nextWord = getNextLong(inputFile, firstChar);
		//check if its valid
		checkIfValidWord(nextWord);
		if(checkIf16BitUTF16(nextWord))
		{
			print16BitCharecterToFile(outputFile, (unsigned int)nextWord);
		}
		else
		{
			unsigned long utf16Char = transformWordToUnicode(nextWord);
			print32BitCharecterToFile(outputFile, utf16Char);
		}
	        firstChar = getc(inputFile);
	}
	return 1;
}
