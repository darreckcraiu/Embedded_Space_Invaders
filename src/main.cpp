/* Darreck Craiu, dcrai011@ucr.edu

 *         Discussion Section: B21

 *         Assignment: Lab #4 Exercise #1
 
 *         Exercise Description: "In this lab, you will implement a simplified version of the game Space Invaders. It will have a single level with 5 aliens. If you destroy all 5 aliens before they reach the bottom, you win, if not you lose. The 16x2 LCD will display messages to the user(such as "Press button to start" and "You win") while the ST7735 color LCD will display the actual game. The only controls will be a joystick.

The following exercises are only there as a guide for the order to approach parts of the project. You will not need to submit separate code files and videos for each exercise. You will only submit your completed st7735.h, your final version of main.cpp, and a demo video showcasing everything that you were able to complete. This means that you do not need to complete things in the order outlined below if you do not want to.

 

Exercise 1
In this exercise, you will implement a basic menu and the user messages using the 16x2 LCD.  

When you start the system, the 16x2 will display a message saying "Press Joystick To Start"
When the joystick button is pressed, the message will change, letting the user know that the game is "active"
You can pick whatever message you would like for when the game is "active" as long as it makes sense. The sample demo video just says "Playing"
When you press the joystick button again, The 16x2 LCD will go back to the first message
 

Exercise 2
In this exercise, you will complete the st7735.h file and use it display the sprites on the st7735 LCD

Go through the st7735.h given and complete all the parts missing. The comments and information in the lab manual should be helpful to do so.
 You will be able to find all the missing commands in this datasheetLinks to an external site.. The command table starts on page 77 of the pdf.
You can comment out the TimerOn() function in your main(to stop any previously implemented SMs from running) and then use the main to test the functions you complete.
Don't forget to remove any test code and uncomment the TimerOn() when you are done testing the st7735.h file.
Now that you can display sprites, you will update the system so when it is "active" it will display some sprites on the ST7735 LCD
The overall system will still behave as before, but after pressing the button the first time, The ship will be displayed in the bottom of the screen and 5 aliens will be displayed at the top
since the sprites for the ship and aliens are both 16x16 pixels we will pretend that the screen is a grid of 8x8. 
the ship should be in the bottom row
the 5 aliens should be in the top 2 rows. You can pick any pattern you want when displaying the aliens as long as they are in 5 continuous columns
the demo video has the aliens alternating rows
When you press the button again, the system will go back to the menu and the sprites will be erased (the st7735 will just display a black screen)
 

Exercise 3
In this exercise, you will implement the movement of the ship and aliens and the "loser" message

When the game is "active", the joystick will now move the ship left or right.
since we are dividing the ST7735 into a grid of 8x8 for the ship, there will only be 8 possible x positions of the ship.
When the joystick is pushed right, the ship will move right a spot. If it is held right, it will keep moving
When the joystick is pushed left, the ship will move left a spot. If it is held left it will keep moving.
Hint: The period of the SM that controls the ship will determine the speed of the ship moving.
When the system goes into "active", the aliens will start moving.
They will start moving right
once the right most alien reaches the right most spot, they will move down one spot
after moving down once spot, they will start moving left
when the left most alien reaches the left most spot, they will move down one spot
these 4 steps will repeat until they reach the bottom
Hint: Just like the ship, the period of the SM that controls the aliens will determine the speed of the aliens moving.
Since the aliens are now moving, this is a good time to implement the "loser" message
While the game is "active" the button will no longer send the game back to the menu
Once the aliens reach the bottom, everything stops and the 16x2 LCD will display a new message letting the user know they have lost
the message can be anything you want as long as it makes sense 
When the joystick is pressed during the "loser" state, it will send the system back to the menu essentially resetting the system
 

Exercise 4
In this exercise, you will implement the bullets and the "winner" message.

When the system is "active", pressing the button will shoot a bullet from the ship
Holding the button should not shoot multiple bullets. Each press is a single shot
when the bullet is shot, it will start from right above the ship and continue upwards in a straight line
To prevent users from "spamming" the button, the user will only be allowed to have 3 bullets active at a time
a bullet will no longer be active once it either goes of the screen or it collides with an alien
if a bullet collides with an alien, the alien will be destroyed
the alien should then be erased and no longer drawn for the remainder of the round
If all aliens are destroyed before they reach the bottom, the system should let the user know they have won
the message can be anything you want as long as it makes sense 
 When the joystick is pressed during the "winner" state, it will send the system back to the menu essentially resetting the system
 

Exercise 5
In this exercise, you will implement the music.

While the game is not "active", the music should be played.
An array with the notes to play and their duration is given in the main.cpp template. 
To play the music, you will need to iterate through the music array and play each note for the specified time before moving to the next note.
When you reach the end of the music array, you will just loop back to the beginning and play it again
To make the notes played distinct, between each note in the array, you should turn the buzzer off for 30ms
When the game is active, the buzzer should be off"

 *        

 *         I acknowledge all content contained herein, excluding template or example code, is my own original work.

 *

 *         Demo Link: https://youtu.be/eGb0SGQPO30

 */

#include "spiAVR.h"
#include "helper.h"
#include "periph.h"
#include "timerISR.h"
#include "LCD.h"
#include "st7735.h"
#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#include "serialatmega.h"

#define NUM_TASKS 8 //TODO: Change to the number of tasks being used


//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;

task tasks[NUM_TASKS]; // declared task array with NUM_TASKS amount of tasks

//The following are structs and arrays that could be useful
//when keeping track of the aliens and bullets
struct Alien {
  bool isAlive = true;
  int xPos = 0;
  int yPos = 0;
};

struct Bullet
{
  bool isActive = false;
  int xPos = 0;
  int yPos = 28;
};


//The frequencies for the notes used
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784

//a struct to make accessing a note easier to read
struct Note {
  int frequency;    
  int duration;     // duration in milliseconds
};

//an array of the Note struct
//To access the duration of the first note, you would do Music[0].duration
Note Music[] = {
  {NOTE_G4, 250}, {NOTE_G4, 250}, {NOTE_G4, 250}, {NOTE_C5, 500},
  {NOTE_G4, 250}, {NOTE_G4, 250}, {NOTE_G4, 250}, {NOTE_E5, 500},
  {NOTE_G5, 250}, {NOTE_F5, 250}, {NOTE_E5, 250}, {NOTE_C5, 250},
  {NOTE_D5, 250}, {NOTE_C5, 250}, {NOTE_G4, 250}, {NOTE_G4, 250},
  {NOTE_G4, 250}, {NOTE_G4, 250}, {NOTE_G5, 250}, {NOTE_F5, 250}, 
  {NOTE_E5, 250}, {NOTE_C5, 250}, {NOTE_D5, 250}, {NOTE_E5, 250}, 
  {NOTE_F5, 500}, {NOTE_E5, 250}, {NOTE_D5, 250}, {NOTE_C5, 250}, 
  {NOTE_D5, 250}, {NOTE_E5, 250}, {NOTE_F5, 250}, {NOTE_G5, 500},
  {NOTE_E5, 250}, {NOTE_D5, 250}, {NOTE_C5, 250}, {NOTE_G4, 750}
};

//a function to help play a specific note.
void setTone(int freq){
  TCCR1A |= (1 << COM1A1);
  ICR1 = (16000000 / (8 * freq)) - 1;
  OCR1A = ICR1 / 2;
}

//a function to turn the buzzer off
void offTone(){
   TCCR1A &= ~(1 << COM1A1);
  //OCR1A = 0;
}

const int themeLength = sizeof(Music) / sizeof(Note); //The array length of Music[]. NOT the total duration in time

//TODO: Define Periods for each task
// e.g. const unsined long TASK1_PERIOD = <PERIOD>
const unsigned long GCD_PERIOD = 50;
const unsigned long JOYSTICK_PERIOD = 50;
const unsigned long JOYSTICK_BUTTON_PERIOD = 50;
const unsigned long LCD_PERIOD = 500;
const unsigned long CONTROLLER_PERIOD = 50;
const unsigned long ALIENS_PERIOD = 750;
const unsigned long SHIP_PERIOD = 300;
const unsigned long BULLETS_PERIOD = 100;
const unsigned long SONG_PERIOD = 50;


// TODO: declare your global variables here
bool joystick_left, joystick_right, joystick_press;
enum Modes {START, IN_PLAY, WINNER, LOSER} mode;
unsigned long JOYSTICK_BUTTON_HIGH_TIME;
unsigned char ship_x;
Alien aliens[5];
Bullet bullets[3];

void eraseAliens() { for (int i = 0; i < 5; i++) eraseLargeSprite(aliens[i].xPos, aliens[i].yPos); }
void drawAliens() { for (int i = 0; i < 5; i++) if (aliens[i].isAlive) {drawAlien(aliens[i].xPos, aliens[i].yPos);} }
void eraseBullets() { for (int i = 0; i < 3; i++) eraseSmallSprite(bullets[i].xPos, bullets[i].yPos); }
void drawBullets() { for (int i = 0; i < 3; i++) if (bullets[i].isActive) {drawBullet(bullets[i].xPos, bullets[i].yPos);} }
void destroyOutOfBoundsBullets() {
  for (int i = 0; i < 3; i++) {
    Bullet& b = bullets[i];
    if (b.isActive && b.yPos < 0) {
      b.isActive = false;
    }
  }
}
Alien* alienHit(Bullet& b) {
  for (int i = 0; i < 5; i++) {
    if (aliens[i].isAlive && b.isActive && ((b.xPos == aliens[i].xPos) && (b.yPos / 4 == aliens[i].yPos))) {
      return &aliens[i];
    }
  }
  return nullptr;
}

enum JOYSTICK_States {JOYSTICK_init, JOYSTICK_s0};
int TickFct_JOYSTICK(int state);
enum JOYSTICK_BUTTON_States {JOYSTICK_BUTTON_init, JOYSTICK_BUTTON_release_low, JOYSTICK_BUTTON_press_high, JOYSTICK_BUTTON_press_low};
int TickFct_JOYSTICK_BUTTON(int state);
enum LCD_States {LCD_init, LCD_start, LCD_in_play, LCD_end};
int TickFct_LCD(int state);
enum CONTROLLER_States {CONTROLLER_init, CONTROLLER_begin, CONTROLLER_begin_delay, CONTROLLER_in_play, CONTROLLER_end, CONTROLLER_end_delay};
int TickFct_CONTROLLER(int state);
enum ALIENS_States {ALIENS_init, ALIENS_off, ALIENS_move_right, ALIENS_right_move_down, ALIENS_move_left, ALIENS_left_move_down};
int TickFct_ALIENS(int state);
enum SHIP_States {SHIP_init, SHIP_off, SHIP_idle, SHIP_move_right, SHIP_move_left};
int TickFct_SHIP(int state);
enum BULLETS_States {BULLETS_init, BULLETS_off, BULLETS_s0};
int TickFct_BULLETS(int state);
enum SONG_States {SONG_init, SONG_play_song, SONG_off, SONG_delay_1, SONG_delay_2, SONG_delay_3};
int TickFct_SONG(int state);

void TimerISR() {
    for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {                   // Iterate through each task in the task array
		if ( tasks[i].elapsedTime == tasks[i].period ) {           // Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
			tasks[i].elapsedTime = 0;                          // Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;                        // Increment the elapsed time by GCD_PERIOD
	}
}


int main(void)
{
  //TODO: Initialize all your inputs and outputs
  DDRC =    0b0000011;
  PORTC =   0b0001010;
  DDRB =    0b0101110;
  PORTB =   0b0000000;
  DDRD =    0b1111100;
  PORTD =   0b0000000;

  //Other initilizations
  SPI_INIT();
  InitST7735LCD();
  ADC_init();
  lcd_init();
  lcd_clear();

  serial_init(9600);

  //Buzzer initialization
  TCCR1A |= (1 << WGM11) | (1 << COM1A1); //COM1A1 sets it to channel A
  TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11); //CS11 sets the prescaler to be 8
  //WGM11, WGM12, WGM13 set timer to fast pwm mode
  // To silence:
  //TCCR1B = (TCCR1B & 0xF8);   // clear prescaler bits → timer stopped, no PWM output

  // To play:
  //TCCR1B = (TCCR1B & 0xF8) | 0x02;   // restart with prescaler

  // TODO: Assign your tasks into the tasks[] array
  // example:
  tasks[0].period = JOYSTICK_PERIOD;
  tasks[0].state = JOYSTICK_init;
  tasks[0].elapsedTime = JOYSTICK_PERIOD;
  tasks[0].TickFct = &TickFct_JOYSTICK;

  tasks[1].period = JOYSTICK_BUTTON_PERIOD;
  tasks[1].state = JOYSTICK_BUTTON_init;
  tasks[1].elapsedTime = JOYSTICK_BUTTON_PERIOD;
  tasks[1].TickFct = &TickFct_JOYSTICK_BUTTON;

  tasks[2].period = LCD_PERIOD;
  tasks[2].state = LCD_init;
  tasks[2].elapsedTime = LCD_PERIOD;
  tasks[2].TickFct = &TickFct_LCD;

  tasks[3].period = CONTROLLER_PERIOD;
  tasks[3].state = CONTROLLER_init;
  tasks[3].elapsedTime = CONTROLLER_PERIOD;
  tasks[3].TickFct = &TickFct_CONTROLLER;

  tasks[4].period = SONG_PERIOD;
  tasks[4].state = SONG_init;
  tasks[4].elapsedTime = SONG_PERIOD;
  tasks[4].TickFct = &TickFct_SONG;

  tasks[5].period = ALIENS_PERIOD;
  tasks[5].state = ALIENS_init;
  tasks[5].elapsedTime = ALIENS_PERIOD;
  tasks[5].TickFct = &TickFct_ALIENS;

  tasks[6].period = SHIP_PERIOD;
  tasks[6].state = SHIP_init;
  tasks[6].elapsedTime = SHIP_PERIOD;
  tasks[6].TickFct = &TickFct_SHIP;
  
  tasks[7].period = BULLETS_PERIOD;
  tasks[7].state = BULLETS_init;
  tasks[7].elapsedTime = BULLETS_PERIOD;
  tasks[7].TickFct = &TickFct_BULLETS;
  
  TimerSet(GCD_PERIOD);
  TimerOn();

  while(1)
  {
    
  }
}

/*
int TickFct_{x}(int state) {
  // transitions
  switch(state) {
    case {x}_init:
      state = ;
      break;
    default:
      state = {x}_init;
      break;
  }

  // state actions
  switch(state) {
    
    default: break;
  }

  return state;
}
*/


int TickFct_SONG(int state) {
  static unsigned char i, j;
  static unsigned long curr_freq;
  
  // transitions
  switch(state) {
    case SONG_init:
      offTone();
      i = j = 0;
      state = SONG_play_song;
      break;
    case SONG_play_song:
      if (mode == IN_PLAY || i >= 37) {
        state = SONG_off;
      } else if (j == 0) {
        state = SONG_delay_1;
      }
      break;
    case SONG_off:
      if (mode != IN_PLAY) {
        i = 0;
        state = SONG_play_song;
      }
      break;
    case SONG_delay_1:
      state = SONG_delay_2;
      break;
    case SONG_delay_2:
      state = SONG_delay_3;
      break;
    case SONG_delay_3:
      state = SONG_play_song;
      break;
    default:
      state = SONG_init;
      break;
  }

  // state actions
  switch(state) {
    case SONG_play_song:
      if (j == 0) {
        curr_freq = Music[i].frequency;
        j = Music[i].duration / SONG_PERIOD;
        setTone(curr_freq);
        i++;
      } else {
        j--;
      }
      break;
    case SONG_off:  
      offTone();    
      break;
    default: break;
  }

  //serial_print(i); serial_print("    ");

  return state;
}

int TickFct_BULLETS(int state) {
  // transitions
  switch(state) {
    case BULLETS_init:
      state = BULLETS_off;
      break;
    case BULLETS_off:
      if (mode == IN_PLAY) {
        state = BULLETS_s0;
      }
      break;
    case BULLETS_s0:
      if (mode == IN_PLAY) {
        eraseBullets();
      } else {
        for (int i = 0; i < 3; i++) {
          bullets[i].isActive = false;
        }
        state = BULLETS_off;
      }      
      break;
    default:
      state = BULLETS_init;
      break;
  }

  // state actions
  switch(state) {
    case BULLETS_off:    
      break;
    case BULLETS_s0:
      //spawn a new bullet on joystick press
      if (joystick_press) {
        for (int i = 0; i < 3; i++) {
          if (!bullets[i].isActive) {
            bullets[i].isActive = true;
            bullets[i].xPos = ship_x;
            bullets[i].yPos = 28;
            break;
          }          
        }
      }
      
      //each active bullet moves up by 1
      for (int i = 0; i < 3; i++) {
        if (bullets[i].isActive) bullets[i].yPos--;
      }
      drawBullets();
      break;
    default: break;
  }

  return state;
}

int TickFct_SHIP(int state) {
  // transitions
  switch(state) {
    case SHIP_init:
      ship_x = 3;
      state = SHIP_off;
      break;
    case SHIP_off:
      if (mode == IN_PLAY) {
        ship_x = 3;
        drawShip(ship_x);
        state = SHIP_idle;
      }
      break;
    case SHIP_idle:
      if (joystick_right && !joystick_left && mode == IN_PLAY) {
        eraseLargeSprite(ship_x, 7);
        state = SHIP_move_right;
      } else if (!joystick_right && joystick_left && mode == IN_PLAY) {
        eraseLargeSprite(ship_x, 7);
        state = SHIP_move_left;
      } else if (mode != IN_PLAY) {
        state = SHIP_off;
      }
      break;
    case SHIP_move_right:
      eraseLargeSprite(ship_x, 7);
      if (!joystick_right && joystick_left && mode == IN_PLAY) {
        state = SHIP_move_left;
      } else if (!joystick_right && !joystick_left && mode == IN_PLAY) {
        drawShip(ship_x);
        state = SHIP_idle;
      } else if (mode != IN_PLAY) {
        state = SHIP_off;
      }
      break;
    case SHIP_move_left:
      eraseLargeSprite(ship_x, 7);
      if (joystick_right && !joystick_left && mode == IN_PLAY) {
        state = SHIP_move_right;
      } else if (!joystick_right && !joystick_left && mode == IN_PLAY) {
        drawShip(ship_x);
        state = SHIP_idle;
      } else if (mode != IN_PLAY) {
        state = SHIP_off;
      }
      break;    
    default:
      state = SHIP_init;
      break;
  }

  // state actions
  switch(state) {
    case SHIP_off:      
      break;
    case SHIP_idle:      
      break;
    case SHIP_move_right:
      ship_x = ship_x < 7 ? ship_x + 1 : ship_x;
      drawShip(ship_x);
      break;
    case SHIP_move_left:
      ship_x = ship_x > 0 ? ship_x - 1 : ship_x;
      drawShip(ship_x);
      break;
    default: break;
  }

  return state;
}

int TickFct_ALIENS(int state) {
  static unsigned char i;
  
  // transitions
  switch(state) {
    case ALIENS_init:
      aliens[0].xPos = 1; aliens[0].yPos = 1; aliens[0].isAlive = true;
      aliens[1].xPos = 3; aliens[1].yPos = 1; aliens[1].isAlive = true;
      aliens[2].xPos = 0; aliens[2].yPos = 2; aliens[2].isAlive = true;
      aliens[3].xPos = 2; aliens[3].yPos = 2; aliens[3].isAlive = true;
      aliens[4].xPos = 4; aliens[4].yPos = 2; aliens[4].isAlive = true;
      state = ALIENS_off;
      break;
    case ALIENS_off:
      if (mode == IN_PLAY) {
        eraseAliens();        
        i = 0;
        state = ALIENS_move_right;
      }
      break;
    case ALIENS_move_right:
      eraseAliens();
      if (i >= 3 && mode == IN_PLAY) {
        state = ALIENS_right_move_down;
      } else if (mode != IN_PLAY) {
        state = ALIENS_init;
      }
      break;
    case ALIENS_right_move_down:
      if (mode == IN_PLAY) {
        eraseAliens();
        i = 0;
        state = ALIENS_move_left;
      } else {
        state = ALIENS_init;
      }     
      break;
    case ALIENS_move_left:
      eraseAliens();
      if (i >= 3 && mode == IN_PLAY) {
        state = ALIENS_left_move_down;
      } else if (mode != IN_PLAY) {
        state = ALIENS_init;
      }
      break;
    case ALIENS_left_move_down:
      if (mode == IN_PLAY) {
        eraseAliens();
        i = 0;
        state = ALIENS_move_right;
      } else {
        state = ALIENS_init;
      }  
      break;
    default:
      state = ALIENS_init;
      break;
  }

  // state actions
  switch(state) {
    case ALIENS_off:    
      break;
    case ALIENS_move_right:
      i++;
      for (int i = 0; i < 5; i++) {
        aliens[i].xPos++;
      }
      drawAliens();
      break;
    case ALIENS_right_move_down:
      for (int i = 0; i < 5; i++) {
        aliens[i].yPos++;
      }
      drawAliens();
      break;
    case ALIENS_move_left:
      i++;
      for (int i = 0; i < 5; i++) {
        aliens[i].xPos--;
      }
      drawAliens();
      break;
    case ALIENS_left_move_down:
      for (int i = 0; i < 5; i++) {
        aliens[i].yPos++;
      }
      drawAliens();
      break;
    default: break;
  }

  //serial_print(aliens[0].yPos);
  
  return state;
}

int TickFct_CONTROLLER(int state) {  
  static unsigned char i;
  
  // transitions
  switch(state) {
    case CONTROLLER_init:
      drawBox(0,127, 0, 127, 0, 0,0);//should turn the screen black
      JOYSTICK_BUTTON_HIGH_TIME = 1000;
      mode = START;
      state = CONTROLLER_begin;
      break;
    case CONTROLLER_begin:
      if (joystick_press) {        
        JOYSTICK_BUTTON_HIGH_TIME = 100;
        i = 0;
        state = CONTROLLER_begin_delay;
      }
      break;
    case CONTROLLER_begin_delay:
      if (i >= 20) {
        mode = IN_PLAY;
        state = CONTROLLER_in_play;
      }      
      break;
    case CONTROLLER_in_play:
      if (mode != IN_PLAY) {
        JOYSTICK_BUTTON_HIGH_TIME = 1000;
        state = CONTROLLER_end;
      }
      break;
    case CONTROLLER_end:
      if (joystick_press) {
        i = 0;
        drawBox(0,127, 0, 127, 0, 0,0);//should turn the screen black
        state = CONTROLLER_end_delay;
      }
      break;
    case CONTROLLER_end_delay:
      if (i >= 20) {
        mode = START;
        state = CONTROLLER_begin;
      }      
      break;
    default:
      state = CONTROLLER_init;
      break;
  }

  // state actions
  switch(state) {
    case CONTROLLER_begin:
      break;
    case CONTROLLER_begin_delay:
      i++;
      break;
    case CONTROLLER_in_play: {
      //kill bullet and alien pair if any touch
      for (int i = 0; i < 3; i++) {
        Bullet& b = bullets[i];
        if (Alien* a = alienHit(b)) {
          a->isAlive = false;
          eraseLargeSprite(a->xPos, a->yPos);
          b.isActive = false;
          eraseSmallSprite(b.xPos, b.yPos);
        }
      }

      //end game if any living alien makes it to the ship's row
      for (int i = 0; i < 5; i++) {
        Alien& a = aliens[i];
        if (a.isAlive && a.yPos == 7) {
          mode = LOSER;
          break;
        }
      }

      //end game if all aliens are dead
      bool allDead = true;
      for (int i = 0; i < 5; i++) {
        if (aliens[i].isAlive) {
          allDead = false;
          break;
        }
      }
      if (allDead) {
        mode = WINNER;
      }

      //kill any bullet that reaches out of bounds
      destroyOutOfBoundsBullets();      
      break;
    }
    case CONTROLLER_end:
      break;
    case CONTROLLER_end_delay:
      i++;
      //serial_print(i);
      break;
    default: break;
  }

  //serial_print(i);
  /*
  static unsigned char printspeed = 0;
  printspeed = (printspeed + 1) % 3;
  if (printspeed == 0) {
    serial_print("mode: "); serial_println(mode); 
    serial_print("controller_state: "); 
    switch(state) {
      case 0:                       serial_println("init"); break;
      case CONTROLLER_begin:        serial_println("begin"); break;
      case CONTROLLER_begin_delay:  serial_println("begin_delay"); break;
      case CONTROLLER_in_play:      serial_println("in_play"); break;
      case CONTROLLER_end:          serial_println("end"); break;
      case CONTROLLER_end_delay:    serial_println("end_delay"); break;
    }
  }*/

  //serial_print(joystick_press);
  

  //_delay_ms(1000);
  
  
  return state;
}

int TickFct_LCD(int state) {
  // transitions
  switch(state) {
    case LCD_init:
      lcd_clear();
      lcd_goto_xy(0, 0);
      lcd_write_str("Press Joystick");
      lcd_goto_xy(1, 0);
      lcd_write_str("to Start");
      state = LCD_start;
      break;
    case LCD_start:
      if (mode == IN_PLAY) {
        lcd_clear();
        lcd_goto_xy(0, 0);
        lcd_write_str("Playing...");
        state = LCD_in_play;
      }
      break;
    case LCD_in_play:
      if (mode == LOSER) {
        lcd_clear();
        lcd_goto_xy(0, 0);
        lcd_write_str("YOU DIED");
        lcd_goto_xy(1, 0);
        lcd_write_str("Press Joystick");
        state = LCD_end;
      } else if (mode == WINNER) {
        lcd_clear();
        lcd_goto_xy(0, 0);
        lcd_write_str("WINNER!");
        lcd_goto_xy(1, 0);
        lcd_write_str("Press Joystick");
        state = LCD_end;
      }
      break;
    case LCD_end:
      if (joystick_press) {
        lcd_clear();
        lcd_goto_xy(0, 0);
        lcd_write_str("Press Joystick");
        lcd_goto_xy(1, 0);
        lcd_write_str("to Start");
        state = LCD_start;
      }
      break;
    default:
      state = LCD_init;
      break;
  }

  // state actions
  switch(state) {
    case LCD_start:
      break;
    case LCD_in_play:
      break;
    case LCD_end:
      break;
    default: break;
  }

  return state;
}

int TickFct_JOYSTICK(int state) {
  // transitions
  switch(state) {
    case JOYSTICK_init:
      state = JOYSTICK_s0;
      break;
    case JOYSTICK_s0:     
      break;
    default:
      state = JOYSTICK_init;
      break;
  }

  // state actions
  switch(state) {
    case JOYSTICK_s0:
      joystick_right = ADC_read(2) > 550 ? 1 : 0;
      joystick_left = ADC_read(2) < 500 ? 1 : 0;  
      break;
    default: break;
  }

  return state;
}

int TickFct_JOYSTICK_BUTTON(int state) {
  static unsigned char i;
  
  // transitions
  switch(state) {
    case JOYSTICK_BUTTON_init:
      i = 0;
      joystick_press = 0;
      state = JOYSTICK_BUTTON_release_low;
      break;
    case JOYSTICK_BUTTON_release_low:   
      if (!GetBit(PINC, PINC3)) {
        joystick_press = 1;
        i = 0;
        state = JOYSTICK_BUTTON_press_high;
      }
      break;
    case JOYSTICK_BUTTON_press_high:
      if (i >= JOYSTICK_BUTTON_HIGH_TIME / JOYSTICK_BUTTON_PERIOD) {
        joystick_press = 0;
        state = JOYSTICK_BUTTON_press_low;
      }
      break;
    case JOYSTICK_BUTTON_press_low:
      if (GetBit(PINC, PINC3)) {
        state = JOYSTICK_BUTTON_release_low;
      }
      break;
    default:
      state = JOYSTICK_BUTTON_init;
      break;
  }

  // state actions
  switch(state) {
    case JOYSTICK_BUTTON_release_low:
      break;
    case JOYSTICK_BUTTON_press_high:
      i++;
      joystick_press = 1;
      break;
    case JOYSTICK_BUTTON_press_low:
      break;    
    default: break;
  }

  //serial_print(joystick_press);

  return state;
}

