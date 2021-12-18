/******************************************************************/
/*program to sort plaintext lists of assorted strings into a      */
/*plaintext 'database' of strings for password cracking           */
/*Features:                                                       */
/*Sexy, modern ncurses interface :P                               */
/*Sort mode:                                                      */
/*Strips strings which already exist in the database, strings     */
/*which are too long or short, and strings which don't match the  */
/*given charset.                                                  */
/*I/O files, string min/max sizes, and charset are user-defined.  */
/*Handles resizing and CTRL+C interruptions.                      */
/*Strip mode:                                                     */
/*Strips control chars and spaces from a file to make sorting     */
/*more accurate and prevent charset fails due to invisible chars. */
/*Other:                                                          */
/*Can print help file with fancy word-wrap for any size window    */
/*                                                                */
/*Author: Doug Yanez <Hacker.Deepspeed@gmail.com>                 */
/*License: GNU/GPL 2+                                             */
/*If you use/change this code, I'd love to see how it turns out :)*/
/******************************************************************/
/*
Compile with:
gcc Listerene-0.5.c -lncurses -Wall -faggressive-loop-optimizations -fwhole-program -o Listerene-0.5
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <curses.h>
#define MENUWID 10    /*menu window sizes*/
#define MENUHIT 8     /*it's a cute lil thing*/
FILE * newptr;        /*file pointers*/
FILE * dbptr;
bool boolRood=0;      /*flag for CTRL+C interrupts*/
bool boolFail=0;      /*prevents writing bad passes or wasting time*/
bool boolSetup=0;     /*for sanitizing Custom mode input*/
int intMenu=0;        /*flag for menu content*/
int intMin=0;         /*min str length holder*/
int intMax=0;         /*max str length holder*/
int intNewLen=0;      /*length of input string*/
int intSetLen=0;      /*length of charset string*/
int intMarker=0;      /*marks last line printed on*/
int derp=0;           /*derp prevention system at your service!*/
int herp=0;           /*derp's cousin, doesn't speak English.*/
int line,col;         /*ncurses stdscr max x,y values*/
int intPick=0;        /*menu choice holder*/
int centx = 0;        /*for centering*/
int centy = 0;
int num_choices;      /*ncurses manual copypasta :P */
float billiam=0;      /*percentage of accepted strings*/
double bobert=0;      /*percentage of stripped strings*/
double addedcount=0;  /*tally of unique strings added.*/
double stripcount=0;  /*total stripped strings*/
double totalcount=0;  /*total strings processed*/
long dupcount=0;      /*tally of duplicates*/
long shortcount=0;    /*tally of too-short strings*/
long longcount=0;     /*tally of too-long strings*/
long formatcount=0;   /*tally of bad-char strings*/
char strInLine[303];  /*string holder for input file*/
char strDbLine[302];  /*string holder for db file*/
char strCharSet[350]; /*charset holder*/
char strInPath[450];  /*input file location*/
char strDbPath[450];  /*db file location*/
char newLetter;       /*letter from strInLine*/
char setLetter;       /*letter from strCharSet*/
/*for ncurses menu thingy*/
char *modeChoices[] = { "Sort","Strip","Help","Exit",};
char *sortChoices[] = { "WPA","SSH","Custom","Exit",};
char *stripChoices[]= { "WPA","SSH","Help","Exit",};
/*Sometimes you just have to break down and prototype stuff*/
void sleep();
int isspace();
int iscntrl();
int toupper();//works on chars, returns ints
void gimme_files();
void brb();
void help_meh();
void wut_had_happend_wuz();
int main();//really, gcc?

/*Let's do this!*/

void draw_weapon(WINDOW *menu_win, int highlight)
{
	/*print menu, highlight selected choice*/
	int x,y,i;
	x=y=2;
	box(menu_win,0,0);//draw border
	for(i=0; i<num_choices; i++,++y)
	{
		if(highlight == i+1){//highlight selection
			wattron(menu_win, A_REVERSE);
			if(intMenu == 0)/*print varying choices*/
				mvwprintw(menu_win,y,x,"%s",modeChoices[i]);
			else if(intMenu == 1)
				mvwprintw(menu_win,y,x,"%s",sortChoices[i]);
			else if(intMenu == 2)
				mvwprintw(menu_win,y,x,"%s",stripChoices[i]);
			wattroff(menu_win, A_REVERSE);
		}
		else{//don't highlight unselected things
			if(intMenu == 0)
				mvwprintw(menu_win,y,x,"%s",modeChoices[i]);
			else if(intMenu == 1)
				mvwprintw(menu_win,y,x,"%s",sortChoices[i]);
			else if(intMenu == 2)
				mvwprintw(menu_win,y,x,"%s",stripChoices[i]);
		}
	}
	wrefresh(menu_win);
}

void fight_monsters()
{
	/*show menu, get choice.*/
	WINDOW *menu_win; /*give our menu window a name*/
	int highlight=1;  /*for highlighting menu selection*/
	int ch;   /*holder for getch.*/
	/*make individual num_choices values or highlighting breaks*/
	if(intMenu==0)
		num_choices = sizeof(modeChoices) / sizeof(char *);//waht?
	if(intMenu==1)
		num_choices = sizeof(sortChoices) / sizeof(char *);//waht?
	if(intMenu==2)
		num_choices = sizeof(stripChoices) / sizeof(char *);//waht?
	menu_win = newwin(MENUHIT, MENUWID, centy, centx);//create window
	keypad(menu_win, TRUE);//activate arrow keys
	draw_weapon(menu_win, highlight);//show menu
	while(1)//interact with keyboard
	{
		ch=wgetch(menu_win);
		switch(ch)/*Yay, object-oriented C!*/
		{	
			case KEY_UP:
				if(highlight == 1)//wrap selections
					highlight = num_choices;
				else
					--highlight;//selection up
				break;
			case KEY_DOWN:
				if(highlight == num_choices)
					highlight = 1;
				else
					++highlight;
				break;
			case 10://ascii 10 == "enter" key
				intPick = highlight;//selection made
				break;
			default://if not useful input
				mvwprintw(stdscr,line-1,0,"Use arrow keys and Enter.");
				refresh();//refresh in order!
				wrefresh(menu_win);
				break;
		}
		draw_weapon(menu_win, highlight);//redraw after button press
		refresh();
		if(intPick!=0){ //stop loop when choice made
			keypad(menu_win,FALSE);//maybe not needed?
			delwin(menu_win);//get menu off screen
			clear();//clear, start fresh
			break;
		}
	}
}

void get_nekkid()
{
	/*Strip whitespace and control chars from a file*/
	FILE * inptr;
	FILE * outptr;
	bool boolStrip=0;
	char filePath[400];
	char outPath[400];
	char letter;
	long lettercount=0;
	const char banner[]="Strip mode chosen!";
	const char ban2[]="Which type of stripping should we perform? *wink*wink*";

	clear();//fresh screen
	getmaxyx(stdscr,line,col);//get screen size
	centx = (col - MENUWID)/2;//center menu
	centy = (line - MENUHIT)/2;
	mvprintw(0,((col-strlen(banner))/2),"%s",banner);
	mvprintw(1,(col-strlen(ban2))/2,"%s",ban2);
	refresh();
	noecho();
	intMenu=2;//strip mode menu
	intPick=0;
	fight_monsters();//show menu, get choice
	if (intPick == 1)//WPA strip
		boolStrip=0;
	else if (intPick == 2)//SSH strip
		boolStrip=1;
	else if (intPick == 3){//show help and return
		help_meh();
		get_nekkid();}//failing to recurse in ncurses ends in cursing!
	else if (intPick == 4){//exit
		endwin();
		exit(0);
	}
	intPick=0;		
	clear();
	curs_set(1);//visible
	nocbreak();
	echo();
	/*Get file location, open file*/
	printw("What file am I stripping? (one at a time pl0x)\n");
	printw("Try CTRL+SHIFT+V, '/path/to/file.txt', or 'file.txt'.\n");
	fflush(stdin);
	scanw("%s", filePath);
	inptr = fopen(filePath, "r");//open file
	for (derp=0;derp<6 && inptr == 0;derp++,scanw("%s",filePath))//while !open
	{
		if (derp >= 4){//exit after 5 fails
			cbreak();
			noecho();//prevents segfaults
			curs_set(0);
			mvprintw(8,0,"Sorry.  I'm new at this.\n");
			printw("Let me go look at the rulebook.  Brb.");
			refresh();
			sleep(4);//a pause for the cause
			endwin();
			exit(1);//testing
		}
		mvprintw(4,0,"\nUmm... Error opening input file.\n");
		printw("Type it again.  Carefully.\n");
		fflush(stdin);
		clrtoeol();
		refresh();
	}
	noecho();
	cbreak();
	curs_set(0);//back to invisible
	strcpy(outPath,filePath);//use same filename for output file
	if(boolStrip==0)//if WPA strip
		strcat(outPath,".ws");//append '.w' to it
	if(boolStrip==1)//if SSH strip
		strcat(outPath,".ss");
	outptr = fopen(outPath,"a+");//append+edit mode
	if (outptr==0){
		printw("Failed to open output file!\n");
		fclose(inptr);//don't forget this!
		printw("Maybe you don't have permission to create files here?\n");
		refresh();
		sleep(3);
		endwin();
		main();
	}
	/*compare to acceptable charset and write new file*/
	for(letter=fgetc(inptr);!feof(inptr);letter=fgetc(inptr))//run til EOF
	{
		if (letter== '\n' || letter == '\0')//print ends before stripping
			fprintf(outptr,"%c",letter);
		if (boolStrip==0){//if WPA strip
			if (isspace(letter) || iscntrl(letter))//skip cntrls+spaces
				lettercount++;//keep a count
			else
				fprintf(outptr,"%c",letter);}//print remainder
		else if (boolStrip==1){//if SSH strip
			if (iscntrl(letter))//skip control chars
				lettercount++;
			else
				fprintf(outptr,"%c",letter);
		}
	}
	fclose(inptr);//close files to save the dolphins
	fclose(outptr);//dolphins are people too!
	printw("Stripped %li bad chars!  ",lettercount);
	printw("Take that, you malformed list!!\n");
	printw("Time taken:%2li seconds.\n",clock()/CLOCKS_PER_SEC);
	printw("<-Press Enter->");
	refresh();
	scanw("%s");
}

void help_meh()
{
	/*resizing during help display causes drop back to menu.  Not sure how/why.*/
	FILE * helpptr;//help file pointer
	/*must use absolute path here*/
	const char helpPath[] = "/usr/share/doc/listerene/HELP";
	char wrdBuf[500];//word buffer
	char chr;
	int y,x,i,len;
	helpptr = fopen(helpPath, "r");//open file
	if (helpptr == NULL){//if failed
		clear();
		mvprintw(0,0,"Error opening help file!");
		refresh();
		sleep(3);
	}
	else{/*Ye olde print-with-word-wrap-from-file loop.  This was hard.*/
/*Idea: get chars one at a time, use a buffer to hold a word. Use space and newlines 
as "buffer dump markers" then print that buffered word intelligently at space/newline.*/
		i=0;//advanced array positioning device (AAPD)
		getmaxyx(stdscr,line,col);
		getyx(stdscr,y,x);//get cursor position
		while((chr = fgetc(helpptr)))//next char each loop
		{ 
			i++;
			if (chr == EOF){
				move(y,0);
				clrtoeol();
				mvprintw(y+1,0,"End of Help File.  Press Enter To Continue.");
				refresh();
				scanw("%s");
				fflush(stdin);
				break;//<insert kit-kat joke here>
			}
			if (chr == ' ' || chr == '\n')//saves space to do this here
				i=0;
			if (chr != ' ' && chr != '\n')//make buffer
			{
				wrdBuf[i-1]=chr;
				len=strlen(wrdBuf);//var not needed, but handy!
			}
			/*print buffer sanely, flush, repeat*/
			else if ((chr==' '||chr=='\n')&& x<(col-len-1))//if buff fits in x
			{
				printw("%s%c",wrdBuf,chr);//print
				getyx(stdscr,y,x);//check position
				if (y >= line-1){//prevent runaway newlines
					move(line-1,0);
					clrtoeol();//clear unwanted words
					mvprintw(line-1,0,"<-Press Enter->");//pause for reading
					refresh();
					scanw("%s");
					clear();
					mvprintw(0,0,"%c",chr);//print on new page
				}		
				strncpy(wrdBuf," ",len);//flush buffer
			}
			else if ((chr==' '||chr=='\n')&& x>=col-len-1&& y<=line-2)
			{
				mvprintw(y+1,0,"%s",wrdBuf);//next line, print, but no \n's
				getyx(stdscr,y,x);//safety check
				if (y<=line-2)//if still in safe zone
					printw("%c",chr);//print possible newline
				getyx(stdscr,y,x);//get current cursor pos.
				if (y >= line-1){//if if at end of page
					move(line-1,0);
					clrtoeol();
					mvprintw(line-1,0,"<-Press Enter->");//pause to read
					refresh();
					scanw("%s");
					clear();//new screen
					mvprintw(0,0,"%s%c",wrdBuf,chr);//print on new page
				}
				strncpy(wrdBuf," ",len);
			}
			getyx(stdscr,y,x);//get cursor position
			getmaxyx(stdscr,line,col);
			refresh();
		}		
		fclose(helpptr);//always close files!
		clear();
	}
}

void discover_magical_powers()
{
	/*Sanitize custom mode input*/
	curs_set(1);//cursor visible again
	printw("Custom mode chosen!\n");
	while (boolSetup != 1 && derp < 5)
		{
		mvprintw(1,0,"What is the minimum acceptable string length?[0-300]: ");
		intMarker++;
		echo();//show typed keys
		nocbreak();//no cbreak for you!
		fflush(stdin);
		scanw("%d", &intMin);
		for(herp=0;intMin > 300 || intMin < 0;herp++)//sanitize intMin
		{
			if (herp >= 5){//if user fails to get it in 5 tries, we give up
				mvprintw(intMarker+3,0,"These things you keep typing...\n");
				printw("I'm not sure you understand what they mean.");
				refresh();
				sleep(4);//take a moment to regret your life choices
				endwin();				
				exit (1);
			}
			mvprintw(intMarker+1,0,"Hey, no funny business!\n");
			printw("Try a realistic number.(0-300): ");
			clrtoeol();//clears old number
			fflush(stdin);
			scanw("%d", &intMin);
		}
		clear();
		intMarker=0;
		mvprintw(0,0,"Thanks!\n");
		printw("What is the longest acceptable string length? [4-300]: ");
		intMarker++;//awkward thing to use, but it works!
		refresh();
		fflush(stdin);
		scanw("%d", &intMax);
		for(herp=0;intMax > 300 || intMax < 4;herp++)//sanitize intMax
		{
			if (herp >= 5){//if user fails to get it in 5 tries, we give up
				mvprintw(intMarker+3,0,"Grandma? Are you on ");
				printw("the computer again??");
				printw("\nGRANDMA THIS ISN'T SOLITAIRE!! >.>");
				refresh();
				sleep(4);
				endwin();
				exit (1);
			}
			mvprintw(intMarker+1,0,"U w0t m8?!  Have you been drinking?\n");
			printw("Try again.(4-300): ");
			clrtoeol();
			refresh();
			fflush(stdin);
			scanw("%d", &intMax);
		}
		if (intMin > intMax){//dumbass detector
			clear();
			mvprintw(0,0,"Hey, what kinda...\n");
			printw("Max size can't be smaller than Min	size!!\nTry again!\n");
			derp++;//preventing derps everyday!
			boolSetup=0;//not 1
		}
		else
			boolSetup=1;//ready to move on!
	}	
}
	
void leave_the_shire()
{
/*Set up sort mode*/
/*WPA allows alpha-numeric and punctuation chars, no whitespace.*/
const char WPAset[]="abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMN\
OPQRSTUVWXYZ><.,?\":\';}{][+=_-)(*&^%$#@!~`|\\\?/";
/*ssh/nix allows some non-printable chars, but not sure exactly which*/
const char SSHset[]="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWX\
YZ1234567890~!@#$%^&*()_+-=|\\}]{\v\"\?[\"\t\a\b\':`;/ >.<,";
	#define WPAMAX 68     //max legitWPA pass length
	#define WPAMIN 8      //min legit WPA pass length
	#define SSHMAX 300    //guessing here, no real max
	#define SSHMIN 0      //min legit SSH pass length
	const char sort[]="Sort mode chosen!";
	const char subsort[]="What rules should we use for sorting?";
	const char words[]="Which character set would you like to use?";
	intPick=0;
	intMenu = 1;//set up new menu contents
	clear();
	getmaxyx(stdscr,line,col);//get screen size
	mvwprintw(stdscr,0,(col-strlen(sort))/2,"%s",sort);
	mvprintw(1,(col-strlen(subsort))/2,"%s",subsort);
	refresh();
	fight_monsters();//display menu, get choice
	if (intPick == 1){//wpa
		intMax=WPAMAX;
		intMin=WPAMIN;
		strcpy(strCharSet, WPAset);
		intSetLen = strlen(strCharSet);
		mvprintw(0,0,"Standard WPA rules loaded!");
	}
	else if (intPick == 2){//ssh
		intMax=SSHMAX;
		intMin=SSHMIN;
		strcpy(strCharSet, SSHset);
		intSetLen = strlen(strCharSet);
		mvprintw(intMarker+1,0,"Standard SSH rules loaded!");
		intMarker++;
	}
	else if (intPick == 3)//You had to be particular, eh?
	{
		discover_magical_powers();//ugly block of input sanitizing
		cbreak();//back to old settings
		noecho();
		herp=0;
		clear();//clear for new menu
		intMarker=0;
		getmaxyx(stdscr,line,col);//get screen size
		mvprintw(1,(col-strlen(words))/2,"%s",words);
		refresh();
		intPick=0;//reset choice for menu loop
		curs_set(0);
		fight_monsters();//show menu, get choice
		if (intPick == 1){//WPA
			strcpy(strCharSet, WPAset);
			intSetLen = strlen(strCharSet);
			mvprintw(0,0,"WPA charset loaded!\n");
		}
		else if (intPick == 2){//SSH
			strcpy(strCharSet, SSHset);
			intSetLen = strlen(strCharSet);
			mvprintw(0,0,"SSH charset loaded!\n");
			intMarker++;
		}
		else if (intPick == 3){//The less-ugly-but-still-ugly bit
			curs_set(1);
			echo();
			nocbreak();//the cbreak nazi strikes again!
			mvprintw(0,0,"Carefully type all of the acceptable characters.");
			printw("\n(Case sensitive) Protip: Try CTRL+SHIFT+V");
			printw("\n: ");
			refresh();
			fflush(stdin);
			scanw("%s", strCharSet);
			/*should probably make fancier code to strip duplicate chars
			  from set but meh...*/
			clear();
			mvprintw(0,0,"Custom charset configured!\n");
			refresh();
			intSetLen = strlen(strCharSet);
		}
		else if (intPick == 4){
			endwin();
			exit (0);
		}
	}
	else if (intPick == 4){//so long, sucka
		endwin();
		exit (0);
	}
	gimme_files();
}

void gimme_files()
{
	 /*get sort mode file paths, open files. More sanitizing.*/
	//nocbreak();//broke custom mode for days with this line!
	echo();
	printw("Input list location? (one at a time pl0x)\n");
	printw("Try CTRL+SHIFT+V, '/path/to/file.txt', or 'file.txt'.\n");
	intMarker=2;
	fflush(stdin);
	scanw("%s", strInPath);
	for (derp=0; derp<5 && newptr == 0; derp++)/*open input file as read-only*/
	{
		newptr = fopen(strInPath, "r");
		if (newptr == 0){//if opening failed
			if (derp >= 5){//exit after 5 fails here
				mvprintw(8,0,"Well this is getting us nowhere.");
				printw("\nLet's take a breather.\n");
				sleep(4);
				endwin();
				exit (1);
			}
			mvprintw(4,0,"\nUmm... Error opening input file.\n");
			printw("Type it again.  Carefully.\n");
			clrtoeol();
			fflush(stdin);
			scanw("%s", strInPath);
		}
	}
	/*if file mounted, continue with setting up db location*/
	clear();
	mvprintw (0,0,"Awesome!  Now what is the database location?\n");
	refresh();
	fflush(stdin);
	scanw("%s", strDbPath);
	for (derp=0; derp <= 5 && dbptr == 0; derp++){
	/*a+ mode lets us read and write, but it opens
	     at end of file.  Will need to SEEK_SET.*/
		dbptr = fopen(strDbPath, "a+");
		if (dbptr != 0){//if opened
			clear();
			mvprintw(0,0,"Files opened!\n");
			noecho();//no more input needed!
			cbreak();
			break;
		}
		if (dbptr == 0){//if fail
			if (derp >= 5){ //exit after 5 fails
				mvprintw(5,0,"\nLook, this isn't working out. ");
				printw("It's not you, it's just...");	
				printw("\nBYE!!\n");
				fclose(newptr);//don't leave files open!
				refresh();
				sleep(4);
				endwin();
				exit (1);
			}
	        mvprintw(2,0,"Erm... Error opening database file.\n");
	        printw("Where did you say it was?\n");
			fflush(stdin);
	        scanw("%s", strDbPath);
			clrtoeol();
		}
	}
	brb();
}

void wait_wat(int sig)
{
	/*handle CTRL+C like a boss*/
	char sure='0';
	clear();
	curs_set(0);
	attron(A_REVERSE);
	for (herp=0; herp<line; herp++){//hitchhiker's reference for lulz
		printw("PLEASE DO NOT PRESS THIS BUTTON AGAIN!!!!!!\n");
	}
	attroff(A_REVERSE);
	refresh();
	sleep(2);
	clear();
	curs_set(1);
	mvprintw(0,0,"Are you absolutely sure you want to quit?\n");
	printw("0: No\n1: Yes\n");
	refresh();
	echo();
	nocbreak();
	fflush(stdin);  
	scanw("%c", &sure);
	for (derp=0; sure != '1' && sure != '0' && toupper(sure) != 'Y' && \
		toupper(sure) != 'N'; derp++)//The price of flexibility... sanitizing
	{
		if (derp >= 4){//derp prevention service!
			printw("\nGAME OVER\n\nINSERT 1 TOKEN TO CONTINUE\n");
			refresh();
			sure = '1';//force quit
			sleep(4);//a moment of silence for the stupid
			endwin();
			exit(1);
		}
		clear();
		mvprintw(0,0,"Improper input!  Do you really want to quit?\n");
		printw("0: No\n1: Yes\n");
		fflush(stdin);
		scanw("%c", &sure);
		
	}
	if (sure == '1' || toupper(sure) == 'Y'){//quitter
		boolRood = 1;//interrupted!
		wut_had_happend_wuz();//print results, close files, exit sanely
		endwin();
		exit(0);
	}
	else if (sure == '0' || toupper(sure) == 'N'){//false alarm
		clear();
		curs_set(0);
		noecho();
		cbreak();//back to work!  No talking!
	}
}
	
void become_hero_of_legend()
{
	/*Check easy things and clean up long string input*/
	intNewLen=strlen(strInLine)-1;
	if (intNewLen < intMin){//disqualify strings too short
		boolFail=1;
		shortcount++;
	}
	if (intNewLen > intMax){//disqualify strings too long
		boolFail=1;
		longcount++;
	}
	/*if no newline in fgets string, fgets stays on the same line*/
	/*This forces a skip to the next fgets string with a newline.*/
	intNewLen = (strlen(strInLine)-1);
	newLetter = strInLine[intNewLen];
	for (derp=0; newLetter != '\n' && !feof(newptr); derp++)//no newline at end
	{
		if (derp>10000){//block infinite looping but allow huge strings
			mvprintw(line-11,0,"Something is wrong.");
			refresh();
			break;
		}
		fgets(strInLine, (intMax+3), newptr);//get new input string
		intNewLen = (strlen(strInLine)-1);//reset
		newLetter = strInLine[intNewLen];
	}
}

void save_the_world()
{
	/*Check charset and check against the db*/
	for (derp=0; derp < intNewLen -1; derp++)//for new string letter
	{
		if (boolFail == 1)
			break;/*one char fails, whole string fails*/
		for (herp=0; herp<intSetLen+1; herp++)//for charset letter
		{
			setLetter = strCharSet[herp];
			if (setLetter == '\0'){//no charset match
				boolFail=1;
				formatcount++;
				break;
			}
			newLetter = strInLine[derp];
			if (newLetter == setLetter)//if match
				break;//not failed
		}
	}/*compare new string to all database strings.*/
	for (;!feof(dbptr); fgets(strDbLine,intMax+1,dbptr))
	{ 	/*Most demanding loop.*/
		if (boolFail == 1)//skip if already failed
			break;
		if (strInLine[0]==strDbLine[0]){//adds >10% efficiency!!
			if (strcmp(strInLine, strDbLine) == 0){//if duplicate
				boolFail=1;
				dupcount++;
				break;
			}
		}
	}
}

void brb()
{	
	int seconds;
	/*Set up vars with proper info before starting the roller-coaster*/
	if (!feof(newptr)){  /*seat belts on!*/
		fseek(newptr, 0, SEEK_SET);//start at beginning of file
		fgets(strInLine, (intMax+3), newptr); /*get string, allow strings*/
	}
	if (!feof(dbptr)){                   /*large enough to fail the check*/
		fseek(dbptr, 0, SEEK_SET); /*reset to start of DB*/
		fgets(strDbLine, intMax+1, dbptr); /*read first db string */
	}
	/*starting the main loop for string comparison.*/
	while (!feof(newptr))
	{
		//set up screen for the long-haul
		curs_set(0);//no need to see cursor any longer
		clear();//prevent artifacts from resizing
		printw("Reading from %s\nAdding to %s\n", strInPath, strDbPath);
		printw("String length limit: %i-%i chars.\n",intMin,intMax);
		printw("Performing a sexy dance in my stretchy pants.\n");
		printw("Please wait...\n\n");
		/*calculate and post stats.*/
		seconds=(clock()/CLOCKS_PER_SEC);//must update in loop
		if (addedcount==0)//prevent weird division derps
			bobert=100;
		else//do real division after words start being added to db
			bobert=((stripcount/totalcount) * 100);
		billiam = (100 - bobert);
		stripcount=dupcount+shortcount+formatcount+longcount;
		mvprintw(line-1,0,"Currently testing: %s",strInLine);
		mvprintw(line-11,0,"Short strings eliminated:        %i\n", shortcount);
		printw("Long strings eliminated:         %i\n", longcount);
		printw("Strings w/ invalid characters:   %i\n", formatcount);
		printw("Duplicate strings eliminated:    %i\n\n", dupcount);
		printw("Total processed strings:         %i\n",(long)totalcount);
		printw("Total strings eliminated:        %i",(long)stripcount);
		clrtoeol();
		printw(" or %.1f\%!\n",bobert);
		printw("Filtered strings added to db:    %i",(long)addedcount);
		clrtoeol();
		printw(" or %.1f\%!\n",billiam);
		printw("Runtime: %3li:%02li:%02li\n",(seconds/3600),\
			((seconds/60)%60),(seconds%60));
		refresh();
		become_hero_of_legend();//don't tell mom
		save_the_world();//*sigh* fiiine
		if (boolFail == 0){  /*if string passed checks*/
			fseek(dbptr, 0, SEEK_END);     //go to db end
			fprintf(dbptr, "%s", strInLine); /*append line to db*/
			addedcount++;
		}
		signal(SIGINT, wait_wat);//handle CTRL+C
		getmaxyx(stdscr,line,col);//check for screen resize
		totalcount++;
		fseek(dbptr, 0, SEEK_SET);/*resets DB file to beginning*/
		if (!feof(dbptr)) /*safety check*/
			fgets(strDbLine, intMax+1, dbptr); /*read first db string */
		boolFail=0;
		fgets(strInLine, (intMax+3), newptr); /*read next input string*/
	}
	wut_had_happend_wuz();
}
	
void wut_had_happend_wuz()
{
	int seconds=(clock()/CLOCKS_PER_SEC);
	/*Do a little math and then give the user an enema
	to help them feel accomplished.*/
	stripcount=dupcount+shortcount+formatcount+longcount;
	clear();//fresh screen!
	if (boolRood == 0)//if finished normally
		mvprintw(0,0,"Filtered from %s\nFiltered into %s.\n", strInPath, strDbPath);
	else if (boolRood == 1){//if interrupted.
		mvprintw(0,0,"You know I can't just finish on command...\n");
		printw("Anyway, here's what happened before you quit.\n");
		printw("Filtered from %s\nFiltered into %s\n", strInPath, strDbPath);
	}
	printw("Short strings eliminated:        %li\n", shortcount);
	printw("Long strings eliminated:         %li\n", longcount);
	printw("Strings w/ invalid characters:   %li\n", formatcount);
	printw("Duplicate strings eliminated:    %li\n\n", dupcount);
	printw("Total processed strings:         %li\n",(long)totalcount);
	printw("Total strings eliminated:        %li",(long)stripcount);
	clrtoeol();
	printw(" or %.1f\%!\n",bobert);
	printw("Filtered strings added to db:    %li",(long)addedcount);
	clrtoeol();
	printw(" or %.1f\%!\n",billiam);
	printw("Runtime: %3li:%02li:%02li\n",seconds/3600,(seconds/60)%60,seconds%60);
	printw("Press Enter to finish.");
	curs_set(1);//make cursor visible again
	refresh();
	nocbreak();//we don't need no stinking cbreak!
	noecho();
	fflush(stdin);
	scanw("%s");//get string, forces Enter press, dumps other chars.
	fclose(newptr); /*close files to prevent apocalypse*/
	fclose(dbptr);
	strncpy(strInPath," ",strlen(strInPath));
	strncpy(strDbPath," ",strlen(strDbPath));
	intMin=0;
	intMax=0;
	dupcount=0;
	addedcount=0;
	stripcount=0;
	totalcount=0;
	formatcount++;
	shortcount++;
	longcount++;
	bobert=0;
	billiam=0;
	boolSetup=0;
}

int main(int argc, char *argv[])
{
	/*set up ncurses*/
	const char greet[]="Welcome to Listerene 0.5!";//for centering
	const char tag[]="So, what are we gonna do today, Brain?";//anybody?..
	initscr();//ncurses activated!
	for(;;)//hooray for infinite loops!
	{	
		/*greet user*/
		intPick=0;
		intMenu=0;
		noecho();
		cbreak();
		curs_set(0);//the mysterious invisible cursor!
		clear();//fresh screen
		getmaxyx(stdscr,line,col);//get screen size
		centx = (col - MENUWID)/2;//centering for menu
		centy = (line - MENUHIT)/2;
		mvprintw(0,(col-strlen(greet))/2,"%s",greet);
		mvprintw(1,(col-strlen(tag))/2,"%s",tag);
		refresh();//the big reveal *drumroll*
		/*main menu of the program*/
		fight_monsters();//show menu, get choice
		if (intPick == 1){//sort mode
			leave_the_shire();//down the rabbit hole of user input!
		}
		else if (intPick == 2){//strip mode and return
			get_nekkid();/*wait.. it's not that kind of stripping?*/
		}
		else if (intPick == 3){//show help and return
			help_meh();
		}
		else if (intPick == 4){//exit
			clear();
			endwin();
			break;
		}
	}
	return (0);
}

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */
