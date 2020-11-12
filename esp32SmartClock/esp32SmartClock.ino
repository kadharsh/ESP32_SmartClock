#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>

/***pin assignment for display***/
 #define TFT_CS         25
 #define TFT_RST        26
 #define TFT_DC         27
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
/***pin assignment for rotory encoder***/
 #define clk    04
 #define dt     16
 #define button 05
  
uint8_t State;
uint8_t LastState; 
uint8_t buttonTrig;

uint8_t selected_itm;
const String mainMenu[] = {
                      "Settings",
                      "Music",
                      "Media",
                      "Notification",
                      "Alarm",
                      "Do List",
                      "opt7",
                      "opt8",
                      "opt9",
                      "opt10"};
                      
/***for timer intrupts***/
volatile int interrupts;
hw_timer_t * timer = NULL;

/***Count time***/
uint8_t prevM;
uint8_t sec, m, hr, date;
bool isAM = true;
char Time [10];

/* this function will be called every second and will count
 * seconds, minut, hour and date */
void tic(){
   sec++;
   if(sec >= 60){
     sec = 0;
     m++;
     if(m >=60){
       m = 0;
       hr++;
       if(hr >= 13){
         hr=1;
         isAM = !isAM;   
         if(isAM){
          date++;
         }
       }
     }
    }
  }
/*void IRAM_ATTR onTimer(){   //count time for each sec
  tic();
}*/

void setup() {
  Serial.begin(115200);
  /*rotory encoder setup*/
  pinMode (clk,INPUT);             //rotory encoder pin
  pinMode (dt,INPUT); 
  LastState = digitalRead(clk);    //encoder last state
  attachInterrupt(clk, updateEncoder, CHANGE );
  attachInterrupt(button, buttonClk, FALLING ); 

  /* Timer and interrupt set up for calling "tic()" every second */
  timer = timerBegin(0,80,true);   //start timer
  timerAttachInterrupt(timer, tic, true);
  timerAlarmWrite(timer, 1000000, true);
  timerAlarmEnable(timer);
  
  tft.initR(INITR_BLACKTAB);       // Init ST7735S chip, black tab if using a 1.8" TFT screen

  tft.setRotation(3);              // rotate display to portrate
  tft.fillScreen(ST77XX_BLACK);    // Background
  sec=0; m=30; hr=9;               // set time
  
}
/*Encoder intrupt
 * This function is called when encoder input is triggerd.
 * Inrupt is ditached while function runs and attached at last.
 * determines the dirrection of movement
 */
 void updateEncoder(){
    detachInterrupt(clk);
    delay(10);
    State = digitalRead(clk);
     if (State != LastState){     
       if (digitalRead(dt) != State) { 
        buttonTrig = 1;
       } else buttonTrig = 2;
     } 
     LastState = State;   
     attachInterrupt(clk, updateEncoder, CHANGE );
 }
/*Rotory encoder button click*/
void buttonClk(){         //called when an input is triggerd
  Serial.println("Ok button interrupted");
  detachInterrupt(button);
  delay(10);
  buttonTrig = 3;
  attachInterrupt(button, buttonClk, FALLING ); 
}
/***********************************************************/


void loop() {
  if(m != prevM){          //check if minute changed
  screen_1();              //if changed screen is updated
  prevM = m;
  }
  if(buttonTrig !=0){      //if button has triggerd
   selected_itm = (printMenu("Menu", mainMenu,10);
   if(selected_itm == 0) ){   //pressed back, so print main screen
     screen_1();
   }else if(selected_itm ==1){
           //Run setings page <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<(create a function for other items too)?@@@@@@@@@@@@@@
         }
 }
}

/* Main Screen
 * Main screen is printed on to the display when this function is called
 */
void screen_1(){
  int16_t  x2, y2;
  uint16_t w, h;
  tft.fillScreen(ST77XX_BLACK);          //set Background
  
  tft.fillCircle(0,0,36,0x214478);       //side circle as icon bg
  tft.fillCircle(160,0,36,0x214478);

  tft.drawRect(4,6,23,12,ST77XX_WHITE);   //for battery icon
  tft.fillRect(28,9,3,6,ST77XX_WHITE);    //" battery icon
  tft.fillRect(6,8,13,8,ST77XX_GREEN);    //" battery icon
 
  tft.fillTriangle(134,6,154,6,144,23,ST77XX_WHITE); //For wiFi function

  sprintf(Time, "%02u:%02u",hr,m);
  tft.setFont(&FreeSansBold12pt7b);

 /*Printing temp*/
  tft.setCursor(56, 40);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE);
  tft.getTextBounds("30 C", 56, 40, &x2, &y2, &w, &h);
  tft.fillRect((x2-6),y2,w+12,h,ST77XX_BLACK);
  tft.print("30 C");
  
 /*Printing time*/
  tft.setCursor(20, 85);
  tft.setTextWrap(false);
  tft.setFont(&FreeSansBold24pt7b);
  tft.setTextColor(ST77XX_YELLOW);
  tft.getTextBounds(Time, 20, 85, &x2, &y2, &w, &h);
  tft.fillRect((x2-6),y2,(w+14),h,ST77XX_BLACK);
  tft.print(Time);

  /*Printing date*/
  tft.setCursor(9, 116);
  tft.setTextWrap(false);
  tft.setFont(&FreeSansBold12pt7b);
  tft.setTextColor(0xED00);
  tft.getTextBounds("SUN 04 OCT", 9, 116, &x2, &y2, &w, &h);
  tft.fillRect((x2-6),y2,(w+12),h,ST77XX_BLACK);
  tft.print("THU 12 NOV");
}

/*To find bgColor of the menu box
 *used to determine slection box's bg
 */
uint16_t findBgColor(byte curBx){
  if(curBx ==0){
    return(ST77XX_BLACK);
  }else {
    if(curBx%2){
      return(0x100D);
    }else return( 0x0808);
  }
}
/////////////////////////////

/* Print menu page
 * The function prints the menu page and navigates acording to user input
 * This function returns the selected item index
 */
uint8_t printMenu(String menuName, String items[], byte arSize){
 byte curPage=0,curBox=0,itemSize=0,pg=0;
 bool pageChanged = true;
 while(true){  //loop runs continuously until something is returned

   if(pageChanged){                       //if current page is changed print new page
    
    tft.fillRect(0,0,160,23,ST77XX_BLUE); //bg1 for heading (blue)
    tft.fillRect(3,03,154,17,0x06FF);     //bg2 for heading (cyan)
    tft.fillRect(0,23,160,15,0x0000);     //bg for 'back' button
    tft.fillRect(0,38,160,90,0x100D);     //dark stripes
    
    tft.fillRect(0,53,160,15,0x0808);     //light stripes  //160px wide and 12px tall
    tft.fillRect(0,83,160,15,0x0808);     //  "  "
    tft.fillRect(0,113,160,15,0x0808);    //  "  "

    tft.setFont();
    tft.setTextWrap(false);
    tft.setTextColor(ST77XX_RED);         //To print menu Name
    tft.setCursor(5,5);                   // "  "
    tft.print(menuName);                  // "  "
    tft.setTextColor(ST77XX_BLUE);        //To print 'back'
    tft.setCursor(5,26);                  // "  "
    tft.print("< Back");                  // "  "
    
    /*calculate no of items in current page and determine if it fits
     *6 items are printed in 1 page
     */
    pg = (arSize/6);                      // round offs to the lowest int value and stores in 'pg'
    if(curPage == pg){                    // true means page can fit inside
      itemSize = (arSize%6);              // calculate how many items needs to print in current page
    }else{
      if(curPage < pg) itemSize = 6;
    }
    
    //Print all the elements  
    for(byte i=0;i<itemSize;i++){ 
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(5,41+(i*15));           //set the cursor position
      tft.print( items[(i+(curPage*6))] );  //printing elements in array
    }
    tft.drawRect(1,23+(curBox*15),120,13,ST77XX_GREEN);
    pageChanged = false; 
  }   

 if(buttonTrig == 0){              //if no button presed do nothing
  //do nothing
 }else if(buttonTrig == 1){        //'left' button triggerd
     buttonTrig = 0;
     if(curBox>0){                 //move the selection box up 
       tft.drawRect(1,23+(curBox*15),120,13,findBgColor(curBox)); 
       curBox--;
       tft.drawRect(1,23+(curBox*15),120,13,ST77XX_GREEN);           //move the box;
     }
 }else if(buttonTrig == 2){        //'right' button triggerd
     buttonTrig = 0;
     if(curBox < itemSize){        //move the selection box down
       tft.drawRect(1,23+(curBox*15),120,13,findBgColor(curBox));
       curBox++;//move the box;
       tft.drawRect(1,23+(curBox*15),120,13,ST77XX_GREEN);
     }else{ 
        if(curPage < pg){          //check if max page reached
          curPage++;               //if end of the page print page 
          pageChanged = true; 
          curBox =0;
        }
      }
 }else if(buttonTrig == 3){        //'ok' button triggerd
     Serial.println(buttonTrig);
     buttonTrig = 0;
     if(curBox ==0){               //check if its back if yes check if last page if yes return '0'
       if(curPage==0){                
       return(0);                    
       }else{
          if(curPage !=0){
            curPage--;             // else move to previous page
            pageChanged = true;
            curBox =0;
          }
       }
     }else{
       return(curBox+(curPage*8)); // else reurn the selected string
     }
 }

}}
