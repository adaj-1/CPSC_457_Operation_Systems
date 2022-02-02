/*
 * mastermind.c
 *
 *  Created on: Jun. 1, 2021
 *      Author: jadal
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>

#define MY_POS_INFINITY 999999
#define MY_NEG_INFINITY -999999

char startUpOutput[100][100];  // buffer to  hold game start up outputs
int startUpRow = 0;            // counts rows for buffer
char screenOutput[40][80];     // buffer to  hold in game screen outputs
int screenCurrentRow = 0;      // counts rows for buffer
char gameOverOutput[100][100]; // buffer to  hold end game outputs
int gameOverRow = 0;           // counts rows for buffer
bool startGame;
bool checkScore;

/*
 * Description: struct to hold all user input parameters
 */
struct UserSetup
{
    char playerName[20]; // command line input player name
    int rows;
    int columns;
    int numOfColours;
    int maxTrials;
    int maxTime;
    int mode;
};

/*
 * Description: struct to hold all time variables for game
 */
struct GameTime
{
    time_t startTime;
    double timeRemaining;
    int hours;
    int minutes;
    int seconds;
};

/*
 * Description: struct to hold all scores for game
 */
struct AllScores
{
    int B;
    int W;
    int numOfTrials;
    double cumScore;
    double finalScore;
};

/*
 * Description: Random number generator
 *
 * @param	lowerBound 	for the random number generator
 * 			upperBound 	for the random number generator
 * 			neg			did not do as Prof said this was unnecessary in lecture
 * @return random integer within the bounds
 */
int randomNum(int lowerBound, int upperBound)
{
    return (rand() % (upperBound + 1 - lowerBound) + lowerBound); // returns a random number within lower & upper bounds
}

/*
 * Description:  returns a letter (colour) based on an integer value n
 *
 * @param 	n			to randomize colour select
 *
 * @return	newColour	randomized colour
 */
char colour(int n)
{
    char newColour = (char)n + 'A'; // convert # to A-Z ASCII to represent color

    return newColour;
}

/*
 * Description: to determine time elapsed
 *
 * @param	maxTimeSec	time specified by user in seconds
 * 			gameTime	struct to store changes to time
 *
 */
void findTime(int maxTimeSec, struct GameTime *gameTime)
{
    time_t currentTime;
    time(&currentTime); // gets current time

    time_t temp = difftime(currentTime, gameTime->startTime); // calculates time difference

    gameTime->timeRemaining = maxTimeSec - temp; // calculates time remaining
}

/*
 * Description: converts timeRemaining into a mm:ss format to be used for log and transcripe functions
 *
 * @param	timeInSec	timeRemaining in seconds
 * 			timeStr		formatted time in string
 */
void timeToString(double timeInSec, char *timeStr)
{
    if (timeInSec == MY_POS_INFINITY)
    {
        strcpy(timeStr, "INFINITY");
    }
    else if (timeInSec > 0) // if maxTime is not exceeded
    {
        int minutes = 0;
        int seconds = 0;

        minutes = timeInSec / 60;
        seconds = timeInSec - (minutes * 60);

        sprintf(timeStr, "%02d:%02d", minutes, seconds); // string together minutes and seconds
    }
    else
    {
        strcpy(timeStr, "00:00"); // placeholder in case time is negative
    }
}

/*
 * Description: initializes code, time, scores, and mode
 *
 * @param	setup		prints starting messages and used to initialize code
 * 			code		creates randomized code
 * 			*gameTime	resets gameTime if game is restarted
 * 			*scores		resets scores if game is restarted
 */
void initializeGame(struct UserSetup setup,
                    char code[setup.rows][setup.columns],
                    struct GameTime *gameTime,
                    struct AllScores *scores)
{
    time(&gameTime->startTime); // time stamps start of game

    /* initializing struct gameTime */
    gameTime->timeRemaining = 0;
    gameTime->hours = 0;
    gameTime->minutes = 0;
    gameTime->seconds = 0;

    /* initializing struct scores */
    scores->B = 0;
    scores->W = 0;
    scores->numOfTrials = 0;
    scores->cumScore = 0;
    scores->finalScore = 0;

    /* generating random seed for game*/
    time_t t;
    srand((unsigned)time(&t));

    /* resets display buffers */
    screenCurrentRow = 1;
    startUpRow = 0;
    gameOverRow = 0;

    if (setup.mode == 1)
    {
        sprintf(startUpOutput[startUpRow++], "Hello %s!\nRunning Mastermind in test mode\n", setup.playerName);
        printf("Hello %s!\nRunning Mastermind in test mode\n", setup.playerName);
    }
    else
    {
        sprintf(startUpOutput[startUpRow++], "Hello %s!\nRunning Mastermind in play mode\n", setup.playerName);
        printf("Hello %s!\nRunning Mastermind in play mode\n", setup.playerName);
    }

    /* generates randomized colour code and fills code array */
    for (int i = 0; i < setup.rows; i++)
    {
        for (int j = 0; j < setup.columns; j++)
        {
            int randNum = randomNum(0, setup.numOfColours - 1); // upper/lower bound based on user input
            code[i][j] = colour(randNum);                       // filling code with randomized colours
        }
    }

    if (setup.mode == 1)
    {
        sprintf(startUpOutput[startUpRow++], "Hidden code is: "); // for transcripe game
        printf("Hidden code is: ");

        /* displays hidden code for test mode */
        for (int i = 0; i < setup.rows; i++)
        {
            for (int j = 0; j < setup.columns; j++)
            {
                sprintf(startUpOutput[startUpRow++], " %c ", code[i][j]);
                printf(" %c ", code[i][j]);
            }
            sprintf(startUpOutput[startUpRow++], "\n");
            printf("\n");
        }
    }

    sprintf(startUpOutput[startUpRow++], "Start cracking...\n");
    printf("Start cracking...\n");

    /* prints initial user interface */
    for (int i = 0; i < setup.rows; i++)
    {
        for (int j = 0; j < setup.columns; j++)
        {
            printf("- ");
        }

        if (i == 0)
        {
            printf("  B   W   R   S   T\n");
        }

        printf("\n");
    }
}

/*
 * Description: prints the hints
 *
 * @param	B is the num of colours that are correct and in the right slot
 * 			W is  the num of colours that are correct but in the wrong slot
 * 			R is the num of trials
 * 			S is the cumulative score
 * 			T is the remaining time
 */
void displayHints(struct UserSetup setup,
                  struct GameTime gameTime,
                  struct AllScores score,
                  char userGuess[setup.rows][setup.columns])
{
    char userInputSpaceStr[80]; // to hold cracked code dashes
    char userInputStr[80];      // to hold userGuess

    userInputSpaceStr[0] = '\0'; // initialize to null
    userInputStr[0] = '\0';      // initialize to null

    /* to reprint user guess with hints */ // TODO if time fix for multi rows
    for (int i = 0; i < setup.rows * setup.columns; i++)
    {
        strcat(userInputSpaceStr, "- ");
        userInputStr[2 * i] = userGuess[0][i];
        userInputStr[2 * i + 1] = ' ';
        userInputStr[2 * i + 2] = '\0';
    }

    /* convert time in sec to mm:ss format  */
    char timeStr[20];
    timeToString(gameTime.timeRemaining, timeStr);

    /* generate screenOuput for the last results */
    sprintf(screenOutput[0], "%s  B  W  R  S    T\n", userInputSpaceStr);
    sprintf(screenOutput[screenCurrentRow++], "%s %2d %2d %2d  %4.2f %s\n", userInputStr,
            score.B,
            score.W,
            score.numOfTrials,
            score.cumScore,
            timeStr);

    system("clear");

    for (int i = 0; i < screenCurrentRow; i++) // print all hints for this game
    {
        printf("%s", screenOutput[i]);
    }
}

/*
 * Description: finding B and W scores
 *
 * @param	setup		provides user input parameters
 * 			code		holds code
 * 			userGuess	holds user guess
 * 			*score		updates score
 */
void findBW(struct UserSetup setup,
            char code[setup.rows][setup.columns],
            char userGuess[setup.rows][setup.columns],
            struct AllScores *score)
{
    char tmpCode[setup.rows][setup.columns];      // tmp code to prevent override of code
    char tmpUserGuess[setup.rows][setup.columns]; // tmp userInput to prevent override of userGuess

    score->B = 0;
    score->W = 0;

    /* copying code to tmpcode */
    for (int i = 0; i < setup.rows; i++)
    {
        for (int j = 0; j < setup.columns; j++)
        {
            tmpCode[i][j] = code[i][j];
        }
    }

    /* copying userGuess to tmpUserGuess*/
    for (int i = 0; i < setup.rows; i++)
    {
        for (int j = 0; j < setup.columns; j++)
        {
            tmpUserGuess[i][j] = userGuess[i][j];
        }
    }

    /* calculating B score */
    for (int i = 0; i < setup.rows; i++)
    {
        for (int j = 0; j < setup.columns; j++)
        {
            if (tmpUserGuess[i][j] == tmpCode[i][j])
            {
                score->B++;
                tmpUserGuess[i][j] = '0'; // set to 0 to avoid future match
                tmpCode[i][j] = '1';      // set to 1 to avoid future match
            }
        }
    }

    /* calculating W score */
    for (int i = 0; i < setup.rows; i++)
    {
        for (int j = 0; j < setup.columns; j++)
        {
            char colour = tmpUserGuess[i][j]; // checks for one colour at a time

            for (int x = 0; x < setup.rows; x++)
            {
                for (int y = 0; y < setup.columns; y++)
                {
                    if (colour == tmpCode[x][y])
                    {
                        score->W++;
                        tmpUserGuess[i][j] = '0'; // set to 0 to avoid future match
                        tmpCode[x][y] = '1';      // set to 1 to avoid future match
                    }
                }
            }
        }
    }
}

/*
 * Description: returns overall score
 *
 * @param	setup		used to calculate scores
 * 			code		used to check user guess
 * 			userGuess	checks user guess
 * 			*gameTime	updates remaining time
 * 			*score		to update scores
 *
 * @return	false if	(1) code is cracked
 * 						(2) time exceeded
 * 						(3) trials exceeded
 */
bool calculateScore(struct UserSetup setup,
                    char code[setup.rows][setup.columns],
                    char userGuess[setup.rows][setup.columns],
                    struct GameTime *gameTime,
                    struct AllScores *score)
{
    findBW(setup, code, userGuess, score); // calculates B and W

    // S- calculate cumulative score
    double stepScore = (score->B + (score->W / 2)) / score->numOfTrials; // calculate stepscore

    score->cumScore += stepScore; // calculates cumulative score

    findTime(setup.maxTime * 60, gameTime); // *60 to convert from min to sec

    if (gameTime->timeRemaining < 0) // checks if time exceeded
    {
        gameTime->timeRemaining = 0;
    }

    score->finalScore = (score->cumScore / score->numOfTrials) * gameTime->timeRemaining * 1000; // calculates final score

    if (score->B == setup.rows * setup.columns || gameTime->timeRemaining <= 0) // checks for game over conditions
    {
        return true;
    }

    if (score->numOfTrials > setup.maxTrials - 1) // checks for game over conditions
    {
        score->finalScore = score->finalScore * -1;
        return true;
    }

    return false;
}

/*
 * Description: Records player names and score in log file
 *
 * @param	setup		hold player information
 * 			score 		holds all scores
 * 			gameTime	holds game time information
 */
void logScore(struct UserSetup setup,
              struct AllScores score,
              struct GameTime gameTime)
{
    FILE *fp;
    fp = fopen("mastermind.log", "a+"); // creates/ adds to log file

    char timeStr[80];
    timeToString(gameTime.timeRemaining, timeStr);

    if (score.finalScore == MY_NEG_INFINITY) // checks for player quit
    {
        fprintf(fp, "%s -INFINITY INFINITY\n", setup.playerName); // records player name and score in log file
    }
    else
    {
        fprintf(fp, "%s %f %s\n", setup.playerName,
                score.finalScore,
                timeStr); // records player name and score in log file
    }
}

/*
 * Description: records the game played in a txt file
 *
 * @param	setup		player information
 * 			code		not used
 * 			gameTime	used to build file name
 */
void transcripeGame(struct UserSetup setup,
                    char code[setup.rows][setup.columns],
                    struct GameTime gameTime)
{
    char timeStr[40];

    /* CITATION: https://en.wikibooks.org/wiki/C_Programming/time.h/time_t */
    time_t now;
    struct tm *ts;
    char testBuf[80];

    /* Get the current time */
    now = time(NULL);

    /* Format and print the time, "hh:mm:ss " */
    ts = localtime(&now);
    strftime(testBuf, sizeof(testBuf), "%H-%M-%S", ts);

    sprintf(timeStr, "%s_%s", setup.playerName, testBuf);

    char buf[0x100];
    snprintf(buf, sizeof(buf), "%s.txt", timeStr);

    FILE *fp;
    fp = fopen(buf, "a+");
    for (int i = 0; i < startUpRow; i++) // print start game messages
    {
        fprintf(fp, "%s", startUpOutput[i]);
    }

    for (int i = 0; i < screenCurrentRow; i++) // print all hints for this game
    {
        fprintf(fp, "%s", screenOutput[i]);
    }

    for (int i = 0; i < gameOverRow; i++) // print game over messages
    {
        fprintf(fp, "%s", gameOverOutput[i]);
    }
}

/*
 * Description: exits the game
 *
 * @param	setup		passes user input parameters
 * 			score		passes final score information
 * 			gameTime	passes final time
 */
void exitGame(struct UserSetup setup,
              char code[setup.rows][setup.columns],
              struct AllScores score,
              struct GameTime gameTime)
{

    logScore(setup, score, gameTime);      // adds player score to logfile
    transcripeGame(setup, code, gameTime); // transcribes the game
}

/*
 * Description: user can also ask to display the top n scores before or after any game, including player names and duration.
 *
 * @param	numOfTop	number of top scores
 */
void displayTop(int numOfTop)
{
    FILE *fptr;

    /* tmp arrays to check for top score */
    char tmpName[20];
    char tmpScore[80];
    char tmpDuration[10];

    /* arrays for printing top scores*/
    char name[numOfTop][20]; // highest score in slot 0
    double score[numOfTop];
    char duration[numOfTop][10];

    /* initializes top scores arrays*/
    for (int i = 0; i < numOfTop; i++)
    {
        name[i][0] = 0; // null string
        score[i] = -INFINITY;
        duration[i][0] = 0;
    }

    fptr = fopen("mastermind.log", "r"); // read log file

    if (fptr == NULL)
    {
        printf("Error! File is Empty.\n");
        return;
    }

    /* scans file for top scores */
    while (fscanf(fptr, "%s %s %s", tmpName, tmpScore, tmpDuration) == 3)
    {
        /* does not check -INFINITY scores */
        if (strcmp(tmpScore, "-INFINITY") != 0)
        {
            float logScore = atof(tmpScore); // converts string to float
            for (int i = 0; i < numOfTop; i++)
            {
                if (logScore > score[i]) // add tmpScore to right position
                {
                    for (int j = numOfTop - 1; j > (i - 1); j--)
                    {
                        score[j] = score[j - 1]; // shift the top scores down by 1
                        strcpy(name[j], name[j - 1]);
                        strcpy(duration[j], duration[j - 1]);
                    }

                    score[i] = logScore; // add the tmpScore to the right position
                    strcpy(name[i], tmpName);
                    strcpy(duration[i], tmpDuration);
                    break; // break out of for loop
                }
            }
        }
    }

    /* print top scores*/
    for (int i = 0; i < numOfTop; i++)
    {
        printf("%s %f %s\n", name[i], score[i], duration[i]);
    }
}

/*
 * Description: user can also ask to display the bottom n scores before or after any game, including player names and duration.
 *
 * @param	numOfBottom	number of bottom scores
 */
void displayBottom(int numOfBottom)
{
    FILE *fptr;

    /* tmp arrays to check for bottom score */
    char tmpName[20];
    char tmpScore[80];
    char tmpDuration[10];

    /* arrays for printing bottom scores*/
    char name[numOfBottom][20]; // highest score in slot 0
    double score[numOfBottom];
    char duration[numOfBottom][10];

    /* initializes bottom scores arrays*/
    for (int i = 0; i < numOfBottom; i++)
    {
        name[i][0] = 0; // null string
        score[i] = INFINITY;
        duration[i][0] = 0;
    }

    fptr = fopen("mastermind.log", "r"); // read log file

    if (fptr == NULL)
    {
        printf("Error! File is Empty.\n");
        return;
    }

    /* scans file for bottom scores */
    while (fscanf(fptr, "%s %s %s", tmpName, tmpScore, tmpDuration) == 3)
    {
        /* does not check -INFINITY scores */
        if (strcmp(tmpScore, "-INFINITY") != 0)
        {
            float logScore = atof(tmpScore); // converts string to float
            for (int i = 0; i < numOfBottom; i++)
            {
                if (logScore < score[i]) // add tmpScore to right position
                {
                    for (int j = numOfBottom - 1; j > (i - 1); j--)
                    {
                        score[j] = score[j - 1]; // shift the top scores down by 1
                        strcpy(name[j], name[j - 1]);
                        strcpy(duration[j], duration[j - 1]);
                    }
                    score[i] = logScore; // add the tmpScore to the right position
                    strcpy(name[i], tmpName);
                    strcpy(duration[i], tmpDuration);
                    break; // break out of for loop
                }
            }
        }
    }

    /* print bottom scores*/
    for (int i = 0; i < numOfBottom; i++)
    {
        printf("%s %f %s\n", name[i], score[i], duration[i]);
    }
}

/*
 * Description: asks user for how many top or bottom scores
 */
bool checkDisplayTopBottom()
{
    int TopOrBot;
    int n;

    char checkPlayAgain[50];
    char *string1;

    while (1)
    {
        printf("Top Scores (0) or Bottom Scores (1)\n");
        fgets(checkPlayAgain, 50, stdin); // takes user input

        /* Citation: https://www.geeksforgeeks.org/strtok-strtok_r-functions-c-examples/ */
        string1 = strtok(checkPlayAgain, " "); // removes spaces from string

        if (string1 != NULL) // ensures string is not NULL
        {
            if (string1[0] == '$') // checking for user quit
            {
                return true;
            }
            else
            {
                TopOrBot = atoi(string1);
                /* top scores */
                if (TopOrBot == 0)
                {
                    printf("Enter number of Top Scores to display:\n");
                    scanf("%d", &n);
                    displayTop(n);
                }

                /* bottom scores */
                else if (TopOrBot == 1)
                {
                    printf("Enter number of Bottom Scores to display:\n");
                    scanf("%d", &n);
                    displayBottom(n);
                }
                return false;
            }
        }
    }
}

/*
 * Description: checks user input for guess or quit
 *
 * @param	setup		user input parameters
 * 			userGuess	user inputed guess
 *
 * @return	true	quit command
 * 			false	user guess
 */
bool getGuessOrCommands(struct UserSetup setup,
                        char userGuess[setup.rows][setup.columns])
{
    char inputString[100] = {0};
    char tmpString[100] = {0};

    do
    {
        fgets(inputString, 100, stdin);
    } while (inputString[0] == '\n'); // ignore NULL input

    /* CITATION: https://stackoverflow.com/questions/13084236/function-to-remove-spaces-from-string-char-array-in-c */
    for (int i = 0, j = 0; i < strlen(inputString); i++, j++) // Evaluate each character in the input
    {
        if (inputString[i] != ' ')
        {
            tmpString[j] = inputString[i]; // If the character is not a space
        }                                  // Copy that character to the output char[]
        else
        {
            j--; // If it is a space then do not increment the output index (j)
        }
    }

    /* fill userGuess array */
    for (int i = 0; i < setup.rows; i++)
    {
        for (int j = 0; j < setup.columns; j++)
        {
            userGuess[i][j] = tmpString[i * setup.columns + j];
        }
    }

    startGame = false;
    checkScore = false;

    /* check if user Quit */
    if (userGuess[0][0] == '$')
    {
        return true;
    }

    if (userGuess[0][0] == '#') // Start
    {
        startGame = true;
        return false;
    }

    if (userGuess[0][0] == '!') // SCORES
    {
        checkScore = true;
        return false;
    }
    else
    {
        return false;
    }
}

/*
 * Description: checks for start game, quit game and top/bottom score commands
 *
 * @param	setup		to check input
 * 			userGuess	to check input
 */
bool checkStartOrExitOrScore(struct UserSetup setup, char userGuess[setup.rows][setup.columns])
{
    bool gameQuit = false;
    do
    {
        printf("Start Game (#) or Quit Game ($) or Check Top/Bottom Scores (!)\n");
        gameQuit = getGuessOrCommands(setup, userGuess);

        if (gameQuit) // check if game was quit
        {
            return true;
        }

        if (checkScore)
        {
            gameQuit = checkDisplayTopBottom(); // check if user would like to see top/ bottom scores

            if (gameQuit) // check if game was quit
            {
                return true;
            }
            checkScore = false;
        }
    } while (!startGame); // loop until game starts

    return false;
}

/*
 * Description: user input validation
 *
 * @return true	if all input is valid
 */
bool checkUserSettings(struct UserSetup setup)
{
    bool inputStatus = true;

    if (setup.rows < 1)
    {
        printf("Input Error: N must be greater than or equal to 1.\n");
        inputStatus = false;
    }

    if (setup.columns <= 4)
    {
        printf("Input Error: C must be greater than or equal to 5.\n");
        inputStatus = false;
    }

    if (setup.columns > setup.numOfColours)
    {
        printf("Input Error: M must be greater than or equal to C.\n");
        inputStatus = false;
    }

    if (setup.maxTrials < 1)
    {
        printf("Input Error: R must be greater than or equal to 1.\n");
        inputStatus = false;
    }

    if (setup.maxTime <= 0)
    {
        printf("Input Error: T must be greater than 0 minutes.\n");
        inputStatus = false;
    }

    return inputStatus;
}

/*
 * Description: Prompts user input, initializes, maintains, and terminates the game
 *
 * @param	argc argument count that holds the number of strings pointed
 * 			*argv argument vector
 *
 */
int main(int argc, char *argv[])
{
    struct UserSetup mySetup;
    struct AllScores myScores;
    struct GameTime myGameTime;

    bool userInputValid = false;

    if (argc == 7) // validate number of arguments
    {
        strcpy(mySetup.playerName, argv[1]); // copying program argument to playerName
        mySetup.rows = atoi(argv[2]);        // converting string argument to integer
        mySetup.columns = atoi(argv[3]);
        mySetup.numOfColours = atoi(argv[4]);
        mySetup.maxTrials = atoi(argv[5]);
        mySetup.maxTime = atoi(argv[6]);

        userInputValid = checkUserSettings(mySetup); // check for valid input
    }
    else
    {
        printf("Input Error: ");
    }
    /* ask for new input if previous was invalid */
    while (!userInputValid)
    {
        printf("Please enter Player Name, N, M, C, R, and T values\n");
        scanf("%s %d %d %d %d %d", mySetup.playerName,
              &mySetup.rows,
              &mySetup.columns,
              &mySetup.numOfColours,
              &mySetup.maxTrials,
              &mySetup.maxTime);

        userInputValid = checkUserSettings(mySetup); // check for valid input

        if (!userInputValid)
        {
            printf("Input Error: ");
        }
    }

    /* request game mode from user*/
    bool modeSelect = false;
    do
    {
        printf("Please select a mode: Play (0) or Test (1)\n");
        scanf("%d", &mySetup.mode);

        if (mySetup.mode == 1 || mySetup.mode == 0)
        {
            modeSelect = true;
        }
        else
        {
            printf("Input Error: Please enter 1 or 0.\n");
        }
    } while (!modeSelect);

    char(*code)[mySetup.columns] = malloc(mySetup.rows * mySetup.columns * sizeof(code[0][0]));      // allocate memory for code
    char(*userGuess)[mySetup.columns] = malloc(mySetup.rows * mySetup.columns * sizeof(code[0][0])); // allocate memory for userGuess

    /* game begins */
    bool gameQuit = false;

    gameQuit = checkStartOrExitOrScore(mySetup, userGuess);

    while (!gameQuit) // run until game is Quit
    {
        initializeGame(mySetup, code, &myGameTime, &myScores);

        bool gameOver = false;
        while (!gameOver && !gameQuit) // run until game is over
        {
            myScores.numOfTrials++; // keep track of trials

            printf("Enter your guess below:\n");
            gameQuit = getGuessOrCommands(mySetup, userGuess); // get user input

            if (gameQuit)
            {
                myScores.finalScore = MY_NEG_INFINITY;
                myGameTime.timeRemaining = MY_POS_INFINITY;
            }
            else
            {
                gameOver = calculateScore(mySetup, code, userGuess, &myGameTime, &myScores); // calculate score and check if game over
                displayHints(mySetup, myGameTime, myScores, userGuess);                      // display hints
                if (gameOver)
                {
                    if (myScores.B == mySetup.rows * mySetup.columns) // code cracked
                    {
                        sprintf(gameOverOutput[gameOverRow++], "Cracked!\n");
                        printf("Cracked!\n");
                    }

                    if (myScores.numOfTrials > mySetup.maxTrials - 1) // exceeded max trials
                    {
                        sprintf(gameOverOutput[gameOverRow++], "Trials exceeded.\n");
                        printf("Trials exceeded.\n");
                    }

                    if (myGameTime.timeRemaining < 0) // exceeded time
                    {
                        sprintf(gameOverOutput[gameOverRow++], "Trials exceeded.\n");
                        printf("Time exceeded.\n");
                    }

                    /* prints final score */
                    sprintf(gameOverOutput[gameOverRow++], "Final Score: %.2f\n", myScores.finalScore);
                    printf("Final Score: %.2f\n", myScores.finalScore);

                    gameQuit = checkStartOrExitOrScore(mySetup, userGuess);
                }
            }
        }                                              // not gameOver
        exitGame(mySetup, code, myScores, myGameTime); // exit game
    }                                                  // not gameQuit

    free(code);
    free(userGuess);
}
