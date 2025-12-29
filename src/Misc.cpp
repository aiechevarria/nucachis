#include "Misc.h"

// Valid values for true/false statements
#define PARSER_TRUE_STRINGS 3
#define PARSER_FALSE_STRINGS 3
const char* strTrue[] = {"1", "yes", "true"};
const char* strFalse[] = {"0","no","false"};

// Valid values for replacement/write policies
#define PARSER_REPLACEMENT_STRINGS 4
#define PARSER_WRITE_STRINGS 2
const char* strReplacementPolicy[] = {"lru", "lfu", "rand", "fifo"};
const char* strWritePolicy[] = {"wt", "wb"};
const char* replacementPolicyStr(PolicyReplacement policy) { return strReplacementPolicy[policy]; }
const char* writePolicyStr(PolicyWrite policy) { return strWritePolicy[policy]; }

// Global variables
int debugLevel = 0;
uint32_t cycle = 0;

/**
 * Convert string into long. It can have a multiplier G, M or K. Any other char will result in error.
 * @param  String To be converted into long
 * @param  bool True if the multiplier should be taken as base 2 (G for 2^30, M for 2^20 o K for 2^10), false if it should be taken for base 10 (G for 10^9, M for 10^6 o K for 10^3) 
 * @return Long with converted value or error. -1 for wrong value error. -2 for null pointer error.
 */
long parseLong(const char* string, bool base2) {
    // Check for errors
    if (string == NULL) {
        return -2;
    }

    // Obtain the multiplier K, M, or G by checking for the last character in the string
    long len = strlen(string);

    long multiplier = 1;
    if (string[len - 1] == 'K' || string[len-1] == 'k') {
        if (base2) {
            multiplier = 1024;
        } else {
            multiplier = 1000;
        }
    } else if (string[len - 1] == 'M' || string[len - 1] == 'm') {
        if (base2) {
            multiplier = 1048576;
        } else {
            multiplier = 1000000;
        }
    } else if (string[len - 1] == 'G' || string[len - 1] == 'g') {
        if (base2) {
            multiplier = 1073741824;
        } else {
            multiplier = 1000000000;
        }
    } else if (string[len - 1] > '9' || string[len - 1] < '0') {
        return -1;
    }

    // If the string contains something that is not a number, error with -1
    for (long i = 0; i < len - 1; i++) {
        if (string[i] > '9' || string[i] < '0') {
            return -1;
        }
    }

    return atoi(string) * multiplier;
}

/**
 * Convert string into int. Does NOT accept a multiplier
 * @param  String to be converted into int
 * @return Int with converted value or error. -1 for wrong value error. -2 for null pointer error.
 */
int parseInt(const char* string) {
    if (string == NULL) {
        return -2;
    }

    int len = strlen(string);

    for (long i = 0; i < len; i++) {
        if (string[i] > '9' || string[i] < '0') {
            return -1;
        }
    }

    return atoi(string);
}

/**
 * Convert string into boolean.
 * @param  String to be converted into boolean. Possible strings: yes, no, true, false, 0, 1. Caps do not matter
 * @return int with converted value. 1 for true, 0 for false, -1 for wrong value error. -2 for null pointer error.
 */
int parseBoolean(const char* string) {
    if (string == NULL) {
        return -2;
    }

    // Turn into lower case
    char stringMin[10];
    int i;

    // Iterate until the null character (0) is reached
    for (i = 0; string[i]; i++) {
        stringMin[i] = tolower(string[i]);
    }

    stringMin[i] = '\0';

    // Check string content and retun equivalent boolean value if it matches any of the strings.
    for (int i = 0; i < PARSER_TRUE_STRINGS; i++) {
        if (strcmp(strTrue[i], stringMin) == 0) {
            return 1;
        }
    }

    for (int i = 0; i < PARSER_FALSE_STRINGS; i++) {
        if (strcmp(strFalse[i], stringMin) == 0) {
            return 0;
        }
    }

    return -1;
}

/**
 * Convert string into enum which represent replacement policy.
 * @param  String to be converted into enum. Possible strings defined in str_replacementPolicy
 * @return enum  value or error. -2 for null pointer. -1 for wrong value error
 */
int parseReplacementPolicy(const char* string) {
    if (string == NULL) {
        return -2;
    }

    for (int i = 0; i < NUM_POLICY_REPLACEMENT; i++) {
        if (strcmp(strReplacementPolicy[i], string) == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * Convert string into enum which represent write policy.
 * @param  String to be converted into enum. Possible strings defined in str_replacementPolicy
 * @return enum  value or error. -2 for null pointer error. -1 for wrong value error
 */
int parseWritePolicy(const char* string) {

    // if null pointer retun error -2
    if (string == NULL) {
        return -2;
    }

    for (int i = 0; i < NUM_POLICY_WRITE; i++) {
        if (strcmp(strWritePolicy[i], string) == 0) {
            return i;
        }
    }

    return -1;
}

/**
 * Convert string into double. It can have a multiplier p for 1e-12, n for 1e-9, u for 1e-6, m for 1e-3. Other char will result in error.
 * @param  String to be converted into double
 * @return long with converted value or error. -1 for wrong value error. -2 for null pointer error.
 */
double parseDouble(const char* string) {
    if (string == NULL) {
        return -2;
    }

    // Obtain the multiplier p, n, u, or m. Else error
    long len = strlen(string);
    double multiplier = 1;

    if (string[len - 1] == 'm') {
        multiplier = 1.0 / 1000.0;
    } else if (string[len - 1] == 'u') {
        multiplier = 1.0 / 1000000.0;
    } else if (string[len - 1] == 'n') {
        multiplier = 1.0 / 1000000000.0;
    } else if (string[len - 1] == 'p') {
        multiplier = 1.0 / 1000000000000.0;
    } else if (string[len - 1] > '9' || string[len - 1] < '0') {
        return -1;
    }

    // If something not numeric or multiplier, error, return -1
    for (long i = 0; i < len - 1; i++) {
        if (string[i] > '9' || string[i] < '0') {
            return -1;
        }
    }

    return atoi(string) * multiplier;
}

/**
 * Convert string hex address into long. It can have a the format 0x...
 * Other char will result in error.
 * @param  page_base_address to be converted into long
 * @return long with converted value or error. -1 for wrong value error. -2 for null pointer error.
 */
long parseAddress(const char* pageBaseAddress) {
    if (pageBaseAddress == NULL) {
		return -2;
	}

    // TODO some additional checks should be performed here !!
    long toReturn = strtol(pageBaseAddress, NULL, 16);
	return toReturn;
}

/**
 * Check if a number is power of 2.
 * @param number to check
 * @return boolean
 */
bool isPowerOf2(long number) {
    return number && !(number & (number - 1));
}

/**
 * Check if a number is a multiple of 8.
 * @param number to check
 * @return boolean
 */
bool isAMultipleOf8(long number) {
    return !number % 8;
}

/**
 * Check if a string has binary value inside.
 * @param  string String to be checked
 * @return 1 if it is binary. -1 if it is not binary. -2 if NULL char* param
 */
bool isCorrectBinary(const char* string) {
    if (string == NULL) {
        return -2;
    }
    
    for (int i = 0; string[i]; i++) {
        if (string[i] != '0' && string[i] != '1') {
        	return -1;
	    }
    }

    return 1;
}

/**
 * Checks if hex format is correct.
 * @param number hex number with string format to be checked
 */
bool isCorrectHexadecimal(char* number) {
   if (strlen(number) < 2 || 
        number[0] != '0' ||
        number[1] != 'x' && number[1] != 'X') {
      return 0;
   }

   for (int i = 2; i < strlen(number); i++) {
      if (number[i] < '0' || (number[i] > '9' && number[i] < 'A') || (number[i] > 'F' && number[i] < 'a') || number[i] > 'f') {
         return 0;
      }
   }
   return 1;
}

/**
 * Checks if decimal format is correct.
 * @param number dec number with string format to be checked
 */
bool isCorrectDecimal(char * number) {
   for (int i=0; number[i] != '\0'; i++) {
      if (number[i] < '0' || number[i] > '9') {
         return 0;
      }
   }

   return 1;
}


/**
 * Convert an array of integers to a string.
 * @param array 
 * @param content 
 * @param count 
 * @param width 
 */
void contentArrayToString(unsigned* array, char* content, int count, int width) {
   char num[50];
   content[0] = '\0';

   for (int i = 0; i < count; i++) {
      sprintf(num, "%s%0*x", i > 0 ? " " : "", width, array[i]);
      strcat(content, num);
   }
}

/**
 * Convert a space separated string of hex numbers into an array of longs.
 * @param array 
 * @param content 
 * @param count 
 */
void contentStringToArray(unsigned* array, char* content, int count) {
	if (content == NULL) {
		return;
	}

   // A copy of the original array gets created so that strtok does not modify the original array
   char* contentCopy = (char*) malloc(sizeof(char) * 9 * count);
   strcpy(contentCopy, content);

    char *pch = strtok(contentCopy, " ");
    for (int i = 0; pch != NULL && i < count; i++, pch = strtok(NULL, " ")) {
        array[i] = strtol(pch, NULL, 16);
    }

    free(contentCopy);
}


/**
 * Opens the specified DRAMSys file and returns the last memory access stored in the file
 * @param filename The name or path to the file.
 * @param f A pointer to the file that will get opened.
 * @return The number of the last memory access stored in the open file.
 */
int openDramsysFile(const char* filename, FILE** f) {
	// The file gets opened
	*f = fopen(filename, "r");

	// If the file could not be opened, return -1
    if (*f == NULL) {
        perror("Error opening file\n");
        return -1;
    }

    int number;
    char text[256];

    // While there is data on the file, iterate until the end is reached.
    while (fscanf(*f, "%d:%255[^\n]", &number, text) == 2) {}

    // If the file is empty, the timestamp should begin in 0 or 1, so -1 gets returned (It will get incremented afterwards)
    if (ftell(*f) == 0) {
		number = -1;
	}

    // The file is closed and reopened in append mode
    fclose(*f);
	*f = fopen(filename, "a");

	return number;
}


/**
 * Appends the specified memory address to the specified filename.
 * @param f Pointer to the end of the file to append the content (open_dramsys_file should be used).
 * @param lastNumber The number of the last memory access.
 * @param address The address to append to the file.
 * @param readOrWrite 0 If read, 1 if write.
 */
void writeToDramsysFile(FILE** f, int lastNumber, int readOrWrite, int address) {
	// The pointer gets checked before writing
    if (*f == NULL) {
        perror("File pointer is null\n");
        return;
    }

    // The line gets written to the file
    if (readOrWrite == 0) {
		fprintf(*f, "%d:\tread\t0x%x\n",lastNumber + 1, address);
	} else {
		fprintf(*f, "%d:\twrite\t0x%x\n",lastNumber + 1, address);
	}
}

/**
 * Count the number of lines in the file.
 */
int countLines(FILE* fp) {
   int count = 0;

   for (char c = getc(fp); c != EOF; c = getc(fp))
      if (c == '\n')
         count++;
   return count;
}

/**
 * Closes the DRAMSys trace file.
 * @param f The file to be closed.
 */
void closeDramsysFile(FILE **f) {
    fclose(*f);
}