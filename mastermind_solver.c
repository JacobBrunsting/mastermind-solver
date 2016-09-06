#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "code_list.h"

#define CODE_LENGTH 4
#define COLOR_COUNT 8

/*
 * Colors:
 *   Black   (a)
 *   Blue    (b)
 *   Brown   (c)
 *   Green   (d)
 *   Orange  (e)
 *   Red     (f)
 *   White   (g)
 *   Yellow  (h)
 */
 
// CODE_LENGTH must be less than or equal to COLOR_COUNT and less than 10 (to allow for 1 char answers)
static const char COLORS[] = {0, 1, 2, 3, 4, 5, 6, 7};
static const char * const COLORS_IN_WORDS[] = {"Black", "Blue", "Brown", "Green", "Orange", "Red", "White", "Yellow"};

int guessCount = 0;
int colorsGuessed[] = {0, 0, 0, 0, 0, 0, 0, 0};
int scoreForAverageMatchAtGuessLength = 8;
// TODO: These values are all just set based on guessing, eventually I
//   will write something to run this program continuously with random
//   codes, and modify the values to minimize the number of guesses
double scoreForAverageMatchAtGuess[] = {2, 2, 2, 2, 2, 2, 2, 2};
int scoreReductionForDuplicateAtGuessLength = 8;
double scoreReductionForDuplicateAtGuess[] = {5, 5, 5, 5, 5, 5, 5, 5};
int scoreForUnusedColorAtGuessLength = 8;
double scoreForUnusedColorAtGuess[] = {3, 3, 3, 3, 3, 3, 3, 3};

static const int SCORE_FOR_AVERAGE_MATCH = 2;
static const int SCORE_REDUCTION_FOR_DUPLICATE = 5;

struct GuessInfo {
	char *code;
	int numRightColor;
	int numRightSpace;
};
// blue brown green orange
struct CodeList *codeList;

// assumes the number is in base 10
int changeBase(int base, int num) {
	int newNum = 0;

	while (num > 0) {
		newNum *= 10;
		newNum += num % base;
		num /= base;
	}

	return newNum;
}

int power(int base, int pow) {
	int result = 1;

	for (int i = 0; i < pow; ++i) {
		result *= base;
	}

	return result;
}

int randomNumber(int max) {
	int divisor = RAND_MAX / (max + 1);
	int randVal;

	do {
		randVal = rand() / divisor;
	} while (randVal > max);
	return randVal;
}

// user must free
char * generateFirstGuess() {
	char *code = malloc(CODE_LENGTH * sizeof(char));
	int numPossibleColors = COLOR_COUNT;
	char *possibleColors = malloc(COLOR_COUNT * sizeof(char));

	for (int i = 0; i < COLOR_COUNT; ++i) {
		possibleColors[i] = COLORS[i];
	}

	for (int i = 0; i < CODE_LENGTH; ++i) {
		const int colorIndex = randomNumber(numPossibleColors - 1);
		code[i] = possibleColors[colorIndex];		
		--numPossibleColors;
		possibleColors[colorIndex] = possibleColors[numPossibleColors];
	}
	free(possibleColors);

	return code;
} // blue blue yellow white

void getGuessResults(struct GuessInfo *info) {
	printf("Is your code");
	for (int i = 0; i < CODE_LENGTH; ++i) {
		printf(" %s", COLORS_IN_WORDS[(int) info->code[i]]);
	}
	char input;
	printf("?\n");
	// TODO: make all these strings constants
	printf("Enter the number of colors that are in the right place: ");
	scanf(" %c", &input);
	if (input >= '0') {
		info->numRightSpace = input - '0';
		if (info->numRightSpace == 4) {
			info->numRightColor = 0;
			return;
		}
	} else {
		info->numRightSpace = 0;
	}
	printf("Enter the number of colors that are in the wrong place, but are one of the colors ");
	printf("in your code: ");
	scanf(" %c", &input);
	if (input >= '0') {
		info->numRightColor = input - '0';
	} else {
		info->numRightColor = 0;
	}
}

int codeConsistentWithGuessInfo(struct GuessInfo *info, char *code) {
	int numRightColor = 0;
	int numRightSpace = 0;
	char *guessCode = malloc(CODE_LENGTH * sizeof(char));
	char *codeCopy = malloc(CODE_LENGTH * sizeof(char));

	// determine the number in the correct space
	for (int i = 0; i < CODE_LENGTH; ++i) {
		if (info->code[i] == code[i]) {
			++numRightSpace;
			guessCode[i] = -1;
			codeCopy[i] = -1;
		} else {
			guessCode[i] = info->code[i];
			codeCopy[i] = code[i];
		}
	}

	// determine the number with the correct color
	for (int i = 0; i < CODE_LENGTH; ++i) {
		if (guessCode[i] != -1) {
			for (int c = 0; c < CODE_LENGTH; ++c) {
				if (codeCopy[c] != -1 && codeCopy[c] == guessCode[i]) {
					codeCopy[c] = -1;
					++numRightColor;
					break;
				}
			}
		}
	}// r g g blu
	
	// printf("Num right place is %i, num right color is %i for code %i %i %i %i, guess %i %i %i %i\n", numRightSpace, numRightColor, info->code[0], info->code[1], info->code[2], info->code[3], code[0], code[1], code[2], code[3]);

	return numRightColor == info->numRightColor && numRightSpace == info->numRightSpace;
}

void removeInvalidAnswers(struct GuessInfo *info) {
	struct CodeHolder *possibleCode = getFirstItem(codeList);

	while (possibleCode) {
		struct CodeHolder *nextPossibleCode = getNextItem(possibleCode);
		char *code = getCodeFromListItem(possibleCode);
		
		if (!codeConsistentWithGuessInfo(info, code)) {
			// printf("Removed\n");
			removeListItem(codeList, possibleCode);
		}
		
		possibleCode = nextPossibleCode;
	}
}

// user must free
char *generateNextGuess() {
	char *nextGuess = malloc(CODE_LENGTH * sizeof(char));
	char *mostCommonColors = malloc(CODE_LENGTH * sizeof(char));
	int characterCounts[CODE_LENGTH][COLOR_COUNT] = {{}};
	struct CodeHolder *possibleCode = getFirstItem(codeList);
	while (possibleCode) {
		char *code = getCodeFromListItem(possibleCode);
		if (code != NULL) {
			for (int i = 0; i < CODE_LENGTH; ++i) {
				characterCounts[i][(int) code[i]] = characterCounts[i][(int) code[i]] + 1;
			}
		}
		
		possibleCode = getNextItem(possibleCode);
	}

	for (int i = 0; i < CODE_LENGTH; ++i) {
		int maxCount = 0;
		int winningChar;
		for (int c = 0; c < COLOR_COUNT; ++c) {
			if (characterCounts[i][c] > maxCount) {
				winningChar = c;
				maxCount = characterCounts[i][c];
			}
		}
		mostCommonColors[i] = winningChar;
	}
	// printf("most common colors are %i %i %i %i\n", mostCommonColors[0], mostCommonColors[1], mostCommonColors[2], mostCommonColors[3]);
	
	possibleCode = getFirstItem(codeList);
	int currentMaxScore = 0;
	int scoreSet = 0;
	
	int scoreForAverageMatch;
	int scoreReductionForDuplicate;
	int scoreForUnusedColor;
	
	if (guessCount < scoreForAverageMatchAtGuessLength) {
		scoreForAverageMatch = scoreForAverageMatchAtGuess[guessCount];
	} else {
		scoreForAverageMatch = scoreForAverageMatchAtGuess[scoreForAverageMatchAtGuessLength - 1];
	}
	
	if (guessCount < scoreReductionForDuplicateAtGuessLength) {
		scoreReductionForDuplicate = scoreReductionForDuplicateAtGuess[guessCount];
	} else {
		scoreReductionForDuplicate = scoreReductionForDuplicateAtGuess[scoreReductionForDuplicateAtGuessLength - 1];
	}
	
	if (guessCount < scoreForUnusedColorAtGuessLength) {
		scoreForUnusedColor = scoreForUnusedColorAtGuess[guessCount];
	} else {
		scoreForUnusedColor = scoreForUnusedColorAtGuess[scoreForUnusedColorAtGuessLength - 1];
	}
	
	while (possibleCode) {
		char *code = getCodeFromListItem(possibleCode);
		int score = 0;
		if (code != NULL) {
			// printf("code is %i %i %i %i\n", code[0], code[1], code[2], code[3]);
			for (int i = 0; i < CODE_LENGTH; ++i) {
				if (code[i] == mostCommonColors[i]) {
					score = score + scoreForAverageMatch;
					// printf("|   match   |");
				}
				int colorDuplicate = 0;
				for (int c = 0; c < i; ++c) {
					if (code[c] == code[i]) {
						score = score - scoreReductionForDuplicate;
						// printf("| duplicate |");
						colorDuplicate = 1;
						break;
					}
				}
				if (!colorsGuessed[(int) code[i]] && !colorDuplicate) {
					score = score + scoreForUnusedColor;
				}
			}
			// printf("\ncode has a score of %i\n", score);
			if (score > currentMaxScore || !scoreSet) {
				// printf("====================max score overtaken======================\n");
				currentMaxScore = score;
				scoreSet = 1;
				for (int i = 0; i < CODE_LENGTH; ++i) {
					nextGuess[i] = code[i];
				}
			}
		}
		possibleCode = getNextItem(possibleCode);
	}
	
	free(possibleCode);
	
	return nextGuess;
}

int main() {
	srand(time(NULL));
	
	// TODO: Make these string constants!
	printf("Welcome to Mastermind. Please choose a 4 color code, made up of black, blue, brown,");
	printf("green, orange, red, white, and yellow colors.\n");
	printf("Press 'y' when ready, or 'q' to quit: ");

	char input;
	scanf("%c", &input);

	if (input == 'q') {
		return 0;
	}

	codeList = createCodeList();

	// generate the list of possible codes
	for (int i = 0; i < power(COLOR_COUNT, CODE_LENGTH); ++i) {
		char *code = malloc(CODE_LENGTH * sizeof(char));
		// each digit of the colorIndices represents the index of a color
		int colorIndices = changeBase(COLOR_COUNT, i);

		for (int m = 0; m < CODE_LENGTH; ++m) {
			code[m] = COLORS[colorIndices % 10];
			colorIndices /= 10;
		}

		addListItem(codeList, code);
	}

	struct GuessInfo *guess = malloc(sizeof(struct GuessInfo));
	guess->code = generateFirstGuess();
	guess->numRightColor = 0;
	guess->numRightSpace = 0;

	int guessFreed = 0;

	do {
		getGuessResults(guess);
		if (guess->numRightSpace == CODE_LENGTH) {
			break;
		}
		
		for (int i = 0; i < CODE_LENGTH; ++i) {
			colorsGuessed[(int)guess->code[i]] = 1;
		}
		
		removeInvalidAnswers(guess);
		guess->numRightColor = 0;
		guess->numRightSpace = 0;
		guessFreed = 1;
		free(guess->code);
		if (getListLength(codeList) > 0) {
			guessFreed = 0;
			guessCount++;
			guess->code = generateNextGuess();
		}
	} while (getListLength(codeList) > 0);
	
	guessCount++;

	if (!guessFreed) {
		free(guess->code);
	}
	
	if (getListLength(codeList) > 0) {
		printf("Got the answer in %i %s", guessCount, guessCount == 1 ? "guess" : "guesses");
	} else {
		printf("Couldn't get the answer, you must have messed up :D\n");
	}

	return 0;
}
