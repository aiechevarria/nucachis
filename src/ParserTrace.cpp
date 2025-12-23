/**
 * @file traceparser.c
 * @brief Parses the trace (.vca) file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ParserTrace.h"

/**
 * Remove comments and other string operations on a line from a trace file.
 * Returns 1 if the line is not empty, after removing the comments.
 * @param currentLine Pointer to the text in the current line
 * @return int 1 if the line is not empty after removing the comments
 */
int preprocessTraceLine(char* currentLine) {
   bool notEmpty = false;

   for(int i = 0; currentLine[i] != '\0'; i++){
      // Replace tabs with spaces
      if (currentLine[i] == '\t') {
         currentLine[i] = ' ';
      }

      // Trim lines at comment characters
      if (currentLine[i] == '#' || currentLine[i] == '\n') {
         currentLine[i]='\0';
         break;
      }

      // Check if there are any non-space characters
      if (currentLine[i] != ' ') {
         notEmpty = true;
      }
   }

   return notEmpty;
}

/**
 * Parse a trace line. 
 * 
 * @param line Pointer to the sanitized, preprocessed string.
 * @param result Pointer that will store the operation 
 * @return int 0 if Ok, -1 if errors happened
 */
int parseLine(char* line, MemoryOperation *result){
   char message[1000];
   int fieldId = 0;        // Number of the field. All traces should have at least 2 fields
   char* pch;

   if (debugLevel == 2)
      fprintf(stderr,"Parsing trace line %s\n", line);

   // Set some defaults
   result->hasBreakPoint = false;

   // Check if it has a breakpoint
   if (line[0] == '!') {
      result->hasBreakPoint = true;

      // Skip the first character in a somewhat sketchy way
      line++;
   }

   // Fetch the next token with whitespace as a delimiter
   pch = strtok(line," ");

   // While there is something to process
   while (pch != NULL) {
      // Current field
      switch (fieldId) { 
         // Load/Fetch or Store (One character)
         case 0:
            if (strlen(pch) != 1 || ( *pch != 'L' && *pch != 'S')) {
               fprintf(stderr, "Memory operation must be Load (L) or Store (S).");
               return -1;
            }

            if (*pch == 'L') {
               result->operation = LOAD;
            } else {
               result->operation = STORE;
            }
            break;

         // Address (Must be in hexadecimal)
         case 1:
            if (!isCorrectHexadecimal(pch)) {
               fprintf(stderr, "Invalid or non hexadecimal address.");
               return -1;
            }

            result->address = strtol(pch, NULL, 16);
            break;

         // Instruction or Data (One character)
         case 2: 
            if (strlen(pch) != 1 || ( *pch != 'I' && *pch != 'D')) {
               fprintf(stderr, "Memory operation must be Intruction (I) or Data (D).");
               return -1;
            }

            if (*pch == 'I') {
               // Check that an instruction will be stored
               if (result->operation == STORE) {
                  fprintf(stderr, "You cannot Store (S) an Instruction (I).");
                  return -1;
               }

               result->isData = false;
            } else {
               result->isData = true;
            }
            break;

         // Data (Must be a number)
         case 3:
            if (!isCorrectDecimal(pch)) {
               fprintf(stderr, "Invalid data.");
               return -1;
            }

            result->data = atol(pch);

            if (result->operation == LOAD) {
               fprintf(stderr, "You cannot use the data field in load (L) operations.");
               return -1;
            }
            break;

         // Too many fields
         default:
            fprintf(stderr, "Too many fields.");
            return -1;
      }

      // Get the next field
      fieldId++;
      pch = strtok (NULL, " ");
   }

   // Check the minimum required fields are present
   if (fieldId < 3) {
      fprintf(stderr, "Too few fields.");
      return -1;
   }

   return 0;
}

/**
 * Parses the given trace file and stores all the operations in the memory operation pointer. 
 * The function allocates memory and the caller should free the pointer once finished.
 * 
 * @param traceFile A path to the trace file to parse
 * @param ops Pointer to a memory operation. The caller should NOT allocate memory, the function will do so dynamically depending on the travetrace's length
 * @param numOperations Pointer to an unsigned integer that represents the number of operations.
 * @return int The number of memory operations in the trace. -1 if error
 */
int parseTrace(const char* traceFile, MemoryOperation* ops, uint32_t* numOperations) {
   int errors = 0;

   // File related vars
   FILE *file;
   char *currentLine = NULL;
   size_t len = 0;
   size_t read;

   if (debugLevel == 1)
    printf("Loading trace file: %s\n", traceFile);

   // Open the file and check for errors 
   file = fopen(traceFile, "r");

   if (file == NULL){
      fprintf(stderr,"Error: Cannot open file %s.\n", traceFile);
      return -1;
   }

   // Count the lines and rewind the pointer to the start of the file
   int numberOfLines = countLines(file);
   rewind(file);

   // Allocate memory to store all operations
   if((ops = (MemoryOperation*) malloc(sizeof(MemoryOperation) * numberOfLines)) == NULL){
      fprintf(stderr,"Error: It was not possible to allocate memory in trace parsing.\n");
      return -1;
   }

   int currentLineNumber = 0;
   int numberOfOperations = 0;

   // Read all the lines in the file
   while ((read = getline(&currentLine, &len, file)) != -1) {
      currentLineNumber++;

      // Skip if the line is empty
      if (!preprocessTraceLine(currentLine)) {
         continue;
      }

      // Parse the line and store it in the pointer to the instruction
      if (parseLine(currentLine, &ops[numberOfOperations]) == -1) {
         errors++;
      }
   }

   if (errors == 0) {
      if (debugLevel == 1)
         fprintf(stderr,"\nTracefile was loaded correctly\n");

      *numOperations = numberOfOperations;
      return 0;
   }

   return -1;
}
