/******************************************************************************/
/*!
\file   filter.c
\author Devone Reynolds
\par    email: mtae000@gmail.com
\brief  
    This file contains the implementation of the following functions for filtering a file based on the command line parameters passed.
      
    Functions include:
      +dumpHelp
      +toLowerAlpha
      +toUpperAlpha
      +removeBlankLinesAlpha
      +deleteAlpha
      +replaceAlpha
      +countLinesAlpha
      +expandTabsAlpha
      +filter
      +main

  Hours spent on this assignment: 24.0

  Specific portions that gave you the most trouble:
     1. Figuring out how to count the files so that an array could be
        allocated to use them was a stretch for me. I thought that we
        would only have to use the get_opt function once in the program,
        but that was wrong.
     2. C-style comments.
     3. An additional read in from the file.
     4. Not freeing  memory from the allocated string in readFile causes 
        memleaks to resolve this a check was added in the readFile to 
        check for uninitialized memory.
*/
/******************************************************************************/
#include <getopt.h> /* optstring */
#include <stdio.h>  /* printf fopen fclose fgetc putc */
#include <stdlib.h> /*malloc free*/
#include <ctype.h>  /*toupper tolower */

/*!List of all possible long options*/
static struct option long_options[] =
    {
        {"help", no_argument, NULL, 'h'},
        {"remove-blank-lines", no_argument, NULL, 'b'},
        {"delete=X", required_argument, NULL, 'd'},
        {"replace=XY", required_argument, NULL, 'r'},
        {"tolower", no_argument, NULL, 'l'},
        {"toupper", no_argument, NULL, 'u'},
        {"line-numbers", no_argument, NULL, 'n'},
        {"expand-tabs[=X]", optional_argument, NULL, 'x'},
        {NULL, 0, NULL, 0}};

/*!Requested options stored in a bitfield as flags*/
static struct
{
  unsigned int help : 1;
  unsigned int blank_lines : 1;
  unsigned int delete : 1;
  unsigned int replace : 1;
  unsigned int lower : 1;
  unsigned int upper : 1;
  unsigned int line_numbers : 1;
  unsigned int tabs : 1;
} REQ_OPTIONS;

/*!Stores the data for delete option*/
static char *delete_args = 0;

/*!Stores the data for replace option*/
static char *replace_args = 0;

/*!Stores the data for tabs option*/
static char *tabs_args = 0;

/*!Flag for checking if at the beginning of a file*/
static int begin_of_file = 0;

/**************************************************************************/
/*!
  \brief
    Prints all available options to the screen.
*/
/**************************************************************************/
static void dumpHelp()
{
  /*Which line is being printed out starting from zero*/
  unsigned int line_num = 0;

  /*Help option text*/
  const char *string[] = {
      "Usage: filter [options] [file1 file2 ...] \n",
      "Options:",
      " -b  --remove-blank-lines  removes empty lines.",
      " -d  --delete=X            deletes all occurrences of char X.",
      " -h  --help                display this information.",
      " -l  --tolower             convert all characters to lower case.",
      " -n  --line-numbers        prepend line numbers to each line.",
      " -r  --replace=XY          replace all char X with char Y.",
      " -u  --toupper             convert all characters to upper case.",
      " -x  --expand-tabs[=X]     convert tabs to X spaces (default is 8).\n"};

  /*Print out each line of text*/
  for (; line_num < sizeof(string) / sizeof(string[0]); ++line_num)
  {
    printf("%s\n", string[line_num]);
  }
}

/**************************************************************************/
/*!
  \brief
    Converts an uppercase character to lowercase  character.
  
  \param c
    Current character read from file or stdin

  \return
    Return the modified character
*/
/**************************************************************************/
static char toLowerAlpha(char c)
{
  /*If letter is lowercase convert to uppercase*/
  if (c >= 'A' || c <= 'Z')
  {
    c = tolower(c);
  }

  /*Return the character*/
  return c;
}

/**************************************************************************/
/*!
  \brief
    Converts a lowercase character to uppercase.
  
  \param c
    Current character read from file or stdin

  \return
    Return the modified character
*/
/**************************************************************************/
static char toUpperAlpha(char c)
{
  /*If letter is lowercase convert to uppercase*/
  if (c >= 'a' || c <= 'z')
  {
    c = toupper(c);
  }

  /*Return the character*/
  return c;
}

/**************************************************************************/
/*!
  \brief
    Determines if a line is blank.
  
  \param c
    Current character read from file or stdin

  \param prev
    Previous character read from file or stdin

  \return
    Return the modified character
*/
/**************************************************************************/
static char removeBlankLinesAlpha(char c, char prev)
{
  /*A blankline line if defined as two repeating newlines.*/
  if (c == '\n' && prev == '\n')
  {
    /*Return nothing*/
    return 0;
  }

  /*Return the character*/
  return c;
}

/**************************************************************************/
/*!
  \brief
    Deletes the requested character.
  
  \param c
    Current character read from file or stdin

  \param letter
    Requested letter to be deleted

  \return
    Return the modified character
*/
/**************************************************************************/
static char deleteAlpha(char c, char *letter)
{
  /*Check if the letter needs to be removed*/
  if (c != letter[0])
  {
    /*Return the character*/
    return c;
  }

  /*Return nothing*/
  return 0;
}

/**************************************************************************/
/*!
  \brief
    Replace the requested character with chosen character.
  
  \param c
    Current character read from file or stdin

  \param letter
    Both the letter to be replaced and the letter replacing it

  \return
    Return the modified character
*/
/**************************************************************************/
static char replaceAlpha(char c, char *letter)
{

  /*Check if the letter needs to be removed*/
  if (c == letter[0])
  {
    /*Replace the letter*/
    c = letter[1];
  }

  /*Return the character*/
  return c;
}

/**************************************************************************/
/*!
  \brief
    Prints the current line number from a file or stdin.
  
  \param c
    Current character read from file or stdin

  \param prev
    Previous character read from file or stdin

  \return
    Return the modified character
*/
/**************************************************************************/
static char countLinesAlpha(char c, char prev)
{
  /*Number of lines read from the file*/
  static int line_count = 0;

  /*Obligatory first line*/
  if (c && (line_count == 0 || prev == '\n' || begin_of_file))
  {

    printf("%6i", ++line_count);
    printf("  ");

    /*No longer at the beginning of the file*/
    begin_of_file = 0;
  }

  /*Return the character*/
  return c;
}

/**************************************************************************/
/*!
  \brief
    Replaces an tabs with requested number of spaces or default 8 spaces.
  
  \param c
    Current character read from file or stdin

  \param spaces
    Number of spaces to replace tabs with

  \return
    Return the modified character
*/
/**************************************************************************/
static char expandTabsAlpha(char c, char *spaces)
{
  /*Number of spaces <default number of spaces is 8>*/
  int num_spaces = 8;

  /*Number of spaces printed*/
  int pc = 0;

  /*Check if a specific number of spaces was requested*/
  if (spaces)
  {
    num_spaces = atoi(spaces);
  }

  /*Check if the letter is a tab*/
  if (c == '\t')
  {
    /*Print all of the spaces*/
    for (; pc < num_spaces; ++pc)
    {
      /*Print a space*/
      putc(' ', stdout);
    }

    /*Return nothing*/
    return 0;
  }

  /*Return the character*/
  return c;
}

/**************************************************************************/
/*!
  \brief
    Modifies a file base on the requested filters and prints to stdout.
  
  \param fp
    Pointer to the file in memory

  \param file_name
    Name of the file
*/
/**************************************************************************/
static void filter(FILE *fp, const char *file_name)
{
  /*Stores the character read from the file*/
  char reader = 0;

  /*Previous character read from the file*/
  char previous = 0;

  /*Open the file*/
  fp = fopen(file_name, "rt");

  /*Set begginging of file flag to true*/
  begin_of_file = 1;

  /*Check if file opened correctly*/
  if (fp == NULL)
  {
    fp = stdin;
  }

  /*Check if line numbers flags was requested*/
  if (REQ_OPTIONS.line_numbers)
  {
    /*Print the first line*/
    reader = countLinesAlpha(reader, previous);
  }

  /*While not at th end of this file*/
  while (!feof(fp))
  {

    /*Check if reader has read EOF char*/
    if (reader == EOF)
    {
      /*Reached the end of the file*/
      break;
    }

    /*Filter that character based on flags that were selected*/
    if (REQ_OPTIONS.delete)
    {
      reader = deleteAlpha(reader, delete_args);
    }

    if (REQ_OPTIONS.replace)
    {
      reader = replaceAlpha(reader, replace_args);
    }

    if (REQ_OPTIONS.upper)
    {
      reader = toUpperAlpha(reader);
    }

    if (REQ_OPTIONS.lower)
    {
      reader = toLowerAlpha(reader);
    }

    if (REQ_OPTIONS.line_numbers)
    {
      reader = countLinesAlpha(reader, previous);
    }

    if (REQ_OPTIONS.blank_lines)
    {
      reader = removeBlankLinesAlpha(reader, previous);
    }

    if (REQ_OPTIONS.tabs)
    {
      reader = expandTabsAlpha(reader, tabs_args);

      /*If nothing was returned*/
      if (reader == 0)
      {
        /*Make sure the previous character reflects that*/
        previous = 0;
      }
    }

    /*Check if character */
    if (reader != 0)
    {
      /*Print out modified character*/
      fputc(reader, stdout);

      /*Update the previous char*/
      previous = reader;
    }

    /*Read the next character*/
    reader = fgetc(fp);
  }

  /*Close the file*/
  fclose(fp);
}

/**************************************************************************/
/*!
  \brief
    First entry point for the program
  
  \param argc
    Number or arguments read in from the command line

  \param argv
    Array of the arguments read in from the command line

  \return
    THe program exited cleanly
*/
/**************************************************************************/
int main(int argc, char **argv)
{
  /*Keycode for the options*/
  int key = 0;

  /*Index for the long options*/
  int long_option_index = 0;

  /*An array for file names*/
  char **file_name_list = NULL;

  /*Index into the array for file names*/
  int index = 0;

  /*Contents of the file in a string*/
  FILE *file_contents = 0;

  /*Determine which filters to use */
  /*get_opt will sort all of the non-option arguments to the end of the list*/
  while (key != -1)
  {
    /*Get the next option*/
    key = getopt_long(argc, argv, "-:hbd:lur:nx::", long_options, &long_option_index);

    switch (key)
    {
    case 'h':
      dumpHelp();
      return 0;
    case 'b':
      REQ_OPTIONS.blank_lines = 1;
      break;
    case 'd':
      REQ_OPTIONS.delete = 1;
      delete_args = optarg;
      break;
    case 'l':
      REQ_OPTIONS.lower = 1;
      break;
    case 'u':
      REQ_OPTIONS.upper = 1;
      break;
    case 'r':
      REQ_OPTIONS.replace = 1;
      replace_args = optarg;
      break;
    case 'n':
      REQ_OPTIONS.line_numbers = 1;
      break;
    case 'x':
      REQ_OPTIONS.tabs = 1;
      tabs_args = optarg;
      break;
    case '?':
      printf("Unknown option: %c\n", optopt);
      return 0;
    case ':':
      printf("Missing arg for %c\n", optopt);
      return 0;
    default:
      break;
    }
  }

  /*Allocate memory for the non option args*/
  file_name_list = calloc(sizeof(char *) * optind, 1);

  /*Get all of the non option arguments*/
  if (optind < argc)
  {
    while (optind < argc)
    {
      file_name_list[index++] = argv[optind++];
    }
  }

  /*Restart the option index and file list index*/
  optind = 1;
  index = 0;

  /*Parse the options while there are still files to parse*/
  do
  {
    /*Get the next option*/
    getopt_long(argc, argv, "-:hbd:lur:nx::", long_options, &long_option_index);

    /*Filter the file*/
    filter(file_contents, file_name_list[index++]);
  } while (file_name_list[index]);

  /*Give back the allocated memory*/
  free(file_name_list);
  return 0;
}
