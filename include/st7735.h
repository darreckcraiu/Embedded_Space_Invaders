#ifndef ST7735_H
#define ST7735_H

#include <avr/io.h>
#include <util/delay.h>
#include "helper.h"
#include "spiAVR.h"

#define CS                        PORTB2
#define RESET                     PORTC1
#define A0                        PORTC0

//for some reason, the screen does not start at 0,0 for the pixels,
//it starts at 2,1. These offsets are added to the x and y values in the funtions 
//so you don't have to worry about them when using the functions
#define X_OFFSET                  2
#define Y_OFFSET                  1
#define SPRITE_WIDTH              16
#define BULLET_WIDTH              4


//Sends a command to the ST7735 
void SendCmd(char cmd){
  PORTB = SetBit(PORTB,CS,0);
  PORTC = SetBit(PORTC,A0,0);
  SPI_SEND(cmd);
  PORTB = SetBit(PORTB,CS,1); //ignore receiving
}

//TODO 1: Complete the SendData function
//Hint: it is similar to the SendCmd function(but not the exact same)
void SendData(char data){
  //Set the correct bits before the send
  PORTB = SetBit(PORTB,CS,0);
  PORTC = SetBit(PORTC,A0,1);
  SPI_SEND(data);
  //Set the correct bits after the send
  PORTB = SetBit(PORTB,CS,1);
}

//Performs a hardware reset on the ST7735
void HardwareResetST7735(){
  PORTC = SetBit(PORTC, RESET, 0);//Set reset low
  _delay_ms(200);
  PORTC = SetBit(PORTC, RESET, 1);//Set reset high
  _delay_ms(200);
}


//TODO 2: complete the InitST7735LCD function by looking for the hex values the commands/parameters in the comments corespond to.
// You will need to look at the command table in the datasheet for the st7735
void InitST7735LCD(){
  HardwareResetST7735();
  SendCmd(0x01);//SWRESET
  _delay_ms(150);
  SendCmd(0x11);//SLPOUT
  _delay_ms(200);
  SendCmd(0x3A);//COLMOD
  SendData(0x06);//18bit mode
  _delay_ms(10);
  _delay_ms(10);
  SendCmd(0x29);//DISPON
  _delay_ms(200);
  
}

//These are RGB values of the colors that will be needed for the game
//colors(in order): BLACK, LIGHTGREY, DARKGREY, LIGHTRED, DARKRED, GREEN, ORANGE, YELLOW
unsigned char colors[8][3]{
    {0,0,0}, //BLACK 
    {200,200,200},//LIGHTGREY
    {82,82,82}, //DARKGREY 
    {255,0,0},//LIGHTRED
    {137,4,4}, //DARKRED 
    {141,216,54},//GREEN 
    {228,142,42},//ORANGE 
    {255,255,0}//YELLOW
};

//These are the sprites for the game. The alien and the ship are 16x16 pixels, the bullet is 4x4.
//The numbers in the 2 dimensional arrays corresponded to the values in the colors[][] array.
//For example, 0 is Black since it is the first set of numbers in colors[][]. 5 is green since it is the 
//6th value in the colors[] array.
const unsigned char alienSprite[16][16]{
    {0,0,0,0,0,0,0,5,5,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,5,5,5,5,0,0,0,0,0,0},
    {0,0,0,0,5,0,5,5,5,5,0,5,0,0,0,0},
    {0,0,0,5,0,0,5,5,5,5,0,0,5,0,0,0},
    {0,5,0,5,0,0,5,5,5,5,0,0,5,0,5,0},
    {5,0,0,5,0,0,5,5,5,5,0,0,5,0,0,5},
    {5,0,0,5,0,0,5,5,5,5,0,0,5,0,0,5},
    {5,5,0,5,5,0,3,5,5,3,0,5,5,0,5,5},
    {0,5,5,5,5,6,5,5,5,5,6,5,5,5,5,0},
    {0,0,5,5,5,6,6,5,5,6,6,5,5,5,0,0},
    {0,0,0,5,5,6,6,6,6,6,6,5,5,0,0,0},
    {0,0,5,5,0,0,6,6,6,6,0,0,5,5,0,0},
    {0,0,5,0,0,0,0,5,5,0,0,0,0,5,0,0},
    {0,5,0,0,0,0,0,5,5,0,0,0,0,0,5,0},
    {0,5,0,0,5,0,0,0,0,0,0,5,0,0,5,0},
    {0,0,5,5,0,0,0,0,0,0,0,0,5,5,0,0},

};


const unsigned char shipSprite[16][16]{
    {0,0,0,0,0,0,2,1,2,2,0,0,0,0,0,0},
    {0,0,0,0,0,0,2,1,2,2,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,2,0,0,0,0,3,0,0},
    {0,0,0,0,0,0,1,1,1,1,0,0,0,2,0,0},
    {0,0,0,0,0,1,2,2,2,2,2,0,0,2,0,0},
    {0,0,4,4,4,2,2,2,2,2,2,2,0,2,0,0},
    {0,0,4,4,4,4,4,4,4,4,4,4,4,4,0,0},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,2,0,0,4,4,0,4,4,0,4,4,0,0,2,3},
    {0,0,1,0,3,3,0,3,3,0,3,3,0,1,0,0},
    {0,0,0,1,2,1,2,1,2,1,2,1,2,0,0,0}

};


const unsigned char bulletSprite[4][4]{
    {0,3,6,3},
    {0,6,7,6},
    {0,3,6,3},
    {0,0,3,0}
};



//Draws a rectangle on the screen. Can be used to turn entire screen a single color
//TODO 3: Complete the drawBox fucntion. You will need to find the 8 bit values for the missing commands
void drawBox(int xs, int xe, int ys, int ye, int r, int g, int b){
  //sets the x borders
  SendCmd(0x2A);//CASET
  SendData(0x00);
  SendData(xs+X_OFFSET);
  SendData(0x00);
  SendData(xe+X_OFFSET);
  
  //sets the y borders
  SendCmd(0x2B);//RASET
  SendData(0x00);
  SendData(ys+Y_OFFSET);
  SendData(0x00);
  SendData(ye+Y_OFFSET);

  //sets the colors for each pixel
  SendCmd(0x2c);//RAMWR

  /*
  int area = (xe-xs+1)*(ye-ys+1);//+1 since start and end values are included. Ex: start column being 20 and end column being 40 gives us a width of 21 pixels, not 20
  for(int i =0; i < area; i++){
    //By default the send order is B,G,R instead of R,G,B. This can be changed with the correct command and parameter in the init function
    SendData(b);
    SendData(g);
    SendData(r);
  }*/
 // Hold CS low for the entire pixel burst
  PORTC = SetBit(PORTC, A0, 1);   // data mode
  PORTB = SetBit(PORTB, CS, 0);
  
  
  int area = (xe-xs+1)*(ye-ys+1);
  for(int i = 0; i < area; i++){
    SPI_SEND(b);
    SPI_SEND(g);
    SPI_SEND(r);
  }
  
  PORTB = SetBit(PORTB, CS, 1);   // release CS only at the end
}


//This function is used to draw an alien sprite on the screen.
//Since the screen is 128x128 and the aliens are 16x16, this function
//essentially divides the screen into a grid of 8x8 spots. The parameters,
//xPos and yPos should be between 0 and 7 to draw the sprites correctly.
//TODO 4: Complete the missing parts of the function
void drawAlien(int xPos, int yPos){
  //TODO 4.1: convert xPos and yPos from a value between 0-7 into a value between 0-127 to be sent to the ST7735 as data
  //Hint: xPos = 0 should result in x = 0, xPos = 1 should result in x = 16, xPos = 2 should result in x = 32, etc.
  int x = xPos * 16;
  int y = yPos * 16;
  //TODO 4.2: Find the commands needed for the following missing commands
  SendCmd(0x2a);//CASET
  SendData(0x00);
  SendData(x+X_OFFSET);
  SendData(0x00);
  SendData(x+15+X_OFFSET);

  SendCmd(0x2B);//RASET
  SendData(0x00);
  SendData(y+Y_OFFSET);
  SendData(0x00);
  SendData(y+15+Y_OFFSET);

  SendCmd(0x2c);//RAMWR
  for(int i = 0; i < 16; i++){
    for(int j = 0; j < 16; j++){
      //TODO 4.3: you will need to use the i and j values along with the alienSprite 2d array and the colors 2d array
      //to send the RGB values.
      //Reminder: the order for the screen by default is BGR
      SendData(colors[alienSprite[i][j]][2]);//B
      SendData(colors[alienSprite[i][j]][1]);//G
      SendData(colors[alienSprite[i][j]][0]);//R
    }
  }
}

//This function is used to draw the ship sprite on the screen.
//Since the screen is 128x128 and the aliens are 16x16, this functions
//essentially divides the screen into a grid of 8x8 spots. Since the ship
//should always be at the bottom, yPos is always 7 so only xPos is a 
//parameter.
//TODO 5: Complete the missing parts of the function
void drawShip(int xPos){
  int yPos = 7;
  //TODO 5.1: convert xPos and yPos from a value between 0-7 into a value between 0-127 to be sent to the ST7735 as data
  //Hint: look at the drawAlien function.
  int x = xPos * 16;
  int y = yPos * 16;
  //TODO 5.2: Find the commands needed for the following missing commands
  SendCmd(0x2a);//CASET
  SendData(0x00);
  SendData(x+X_OFFSET);
  SendData(0x00);
  SendData(x+15+X_OFFSET);

  SendCmd(0x2B);//RASET
  SendData(0x00);
  SendData(y+Y_OFFSET);
  SendData(0x00);
  SendData(y+15+Y_OFFSET);

  SendCmd(0x2c);//RAMWR

  /*
  for(int i = 0; i < 16; i++){
    for(int j = 0; j<16; j++){
      //TODO 5.3: you will need to use the i and j values along with the shipSprite 2d array and the colors 2d array
      //to send the RGB values.
      //Reminder: the order for the screen by default is BGR
      SendData(colors[shipSprite[i][j]][2]);//B
      SendData(colors[shipSprite[i][j]][1]);//G
      SendData(colors[shipSprite[i][j]][0]);//R
    }
  }*/

  // Hold CS low for the entire pixel burst
  PORTC = SetBit(PORTC, A0, 1);   // data mode
  PORTB = SetBit(PORTB, CS, 0);
  
  
  for(int i = 0; i < 16; i++){
    for(int j = 0; j < 16; j++){
      SPI_SEND(colors[shipSprite[i][j]][2]);//B
      SPI_SEND(colors[shipSprite[i][j]][1]);//G
      SPI_SEND(colors[shipSprite[i][j]][0]);//R
    }
  }
  
  PORTB = SetBit(PORTB, CS, 1);   // release CS at the end
}


//This function is used to draw a bullet sprite on the screen.
//Since the screen is 128x128 and the bullets are 4x4, this functions
//essentially divides the screen into a grid of 8x32 spots(8 since there are 8 
//lanes the ship can shoot the bullet from and 32 since the bullet has a height of 4 pixels and 
//128/4=32).The parameters, xPos should be between 0 and 7 and yPos should be between 0 and 32
//to draw the sprites correctly.
//TODO 6: Complete the missing parts of the function
void drawBullet(int xPos, int yPos){
  //TODO 6.1: convert xPos and yPos from a value between 0-7 into a value between 0-127 to be sent to the ST7735 as data
  //Hint: the missing part for x will be the same as what you have in the drawAlien and drawShip functions since there
  //is only suppose to be 8 lanes. The +6 is just there to center the bullet in that lane
  //For y, the idea is the same as the previous functions but there are 32 possible values not 8
  //Ex: yPos = 0 should result in y = 0, yPos = 1 should result in y = 4, yPos = 2 should result in y = 8, etc.
  int x = (xPos * 16) +6;
  int y = yPos * 4;
  //TODO 6.2: Find the commands needed for the following missing commands
  SendCmd(0x2a);//CASET
  SendData(0x00);
  SendData(x+X_OFFSET);
  SendData(0x00);
  SendData(x+3+X_OFFSET);

  SendCmd(0x2B);//RASET
  SendData(0x00);
  SendData(y+Y_OFFSET);
  SendData(0x00);
  SendData(y+3+Y_OFFSET);

  SendCmd(0x2c);//RAMWR
  for(int i = 0; i < 4; i++){
    for(int j = 0; j<4; j++){
      //TODO 6.3: you will need to use the i and j values along with the bulletSprite 2d array and the colors 2d array
      //to send the RGB values.
      //Reminder: the order for the screen by default is BGR
      SendData(colors[bulletSprite[i][j]][2]);//B
      SendData(colors[bulletSprite[i][j]][1]);//G
      SendData(colors[bulletSprite[i][j]][0]);//R
    }
  }
}

//The following erase functions will be useful for when sprites move
//Before moving a sprite, you will need to erase it from its current spot
//and then redraw it at its new position

//draws black box over 16x16 sprite
void eraseLargeSprite(int xPos, int yPos){
  //TODO 7: Set the correct values for x and y
  //Hint: it will be the exact same as TODO 4.1
  int x = xPos * 16;
  int y = yPos * 16;
  drawBox(x, x+15, y, y+15,0,0,0);
}

//draws black box over 4x4 sprite
void eraseSmallSprite(int xPos, int yPos){
  //TODO 8:Set the correct values for x and y
  //Hint: it will be the exact same as TODO 6.1
  int x = (xPos * 16) +6;
  int y = yPos * 4;
  drawBox(x, x+3, y, y+3,0,0,0);
}

#endif /* ST7735_H */
