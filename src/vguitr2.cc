/*  Copyright (C) 2017 Nicholas C. Strauss (strauss@positive-internet.com)
    This file is part of VGUITAR -- the ncurses virtual guitar

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

  vguitr2.c file
*/
#include "config.h"
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <alsa/asoundlib.h>
#include <ncurses.h>
#include <ctype.h>
#include <getopt.h>
#define __VGUITAR_GLOBAL__
#include "vguitr2.h"
#include "options.h"

options_vguitar Options;
post           *song=NULL, *gnos=NULL;
finger_press    kp;
//int             global_time = 0;
vguitar         guitar;
//int             basetime = 0;
int             fret;

// neck
void
vguitar::init()
{
  number_strings = 6;
  number_vtimes   = 10;
  string_size = 1;
  vtime_size = 5;

  /* what term dim's? */
  getmaxyx(stdscr, MAX_ROWS, MAX_COLS);

  int neck_size = MAX_COLS/2;   //  half-size neck.
      neck_size = MAX_COLS;     // full size neck

  nut = MAX_COLS-neck_size;
  MAX_COLS = neck_size;
  number_vtimes = (MAX_COLS -5)/vtime_size;
  zero_boxlet();
}

// this must be called whenever entering box mode or changing clip
void
vguitar::zero_boxlet()
{
  for (int i=0;i<SIXSTRING;i++)
    for (int j=0;j<TWENTYONEFRET;j++) boxlet[i][j]=0;
}

void
vguitar::update_refresh()
{
  refresh();
}

void
vguitar::update_cursor()
{	/* draw cursor */
	move(kp.row*string_size,nut+(kp.col+1)*vtime_size); //addch(kp.fret);
	//refresh();
}

void
vguitar::drawing()
{
	// redraw basetime and frets
	guitar.draw_basetime();
	
	if (Options.do_clip && Options.runningMode == do_tablature) guitar.draw_clip();
	else guitar.draw_neck();

	guitar.draw_clear_chords();

	guitar.draw_strings();

	//        guitar.update_tablature();

#if 0 /*to ::drawing */
	/* draw cursor */
	move(kp.row*string_size,nut+(kp.col+1)*vtime_size); //addch(kp.fret);
	refresh();
#endif      

	//  draw_remembered_chords();
	//  update_strum2();

#if 0
  // redraw basetime and frets

  draw_basetime();

  draw_strings();

  
  draw_bridge(strings_picked);
	
  draw_clear_clip();

  draw_remembered_chords();

  update_strum2();

  /* draw cursor */
  move(kp.row*string_size,nut+(kp.col+1)*vtime_size); //addch(kp.fret);
  refresh();
#endif
}
#if 0
int open_strings[NUM_GSTRINGS]={1,1,1,1,1,1};
int strings_used[NUM_GSTRINGS]={0, 0, 0, 0, 0, 0};
int strings_col[NUM_GSTRINGS]={0, 0, 0, 0, 0, 0};
int strings_picked[NUM_GSTRINGS]={1, 0, 0, 2, 3, 0};
#endif

const char client_name[]="Virtual Guitar";
static snd_seq_t *seq_handle = NULL;
static int my_client, my_port;
//static int to_seq_client, to_seq_port;
//static int chan_no = 1;
static snd_seq_event_t event;

int
main(int argc, char **argv)
{
  int  running = true;
  int  play = false, iloop;
  int  piano_roll;
  unsigned int  ch;
  post     *apost = NULL;

  //kp.row = 1;
  //  kp.col = 0;

  default_options();
  if (argc > 1){
    parse_options(argc, argv);
  }
  //  print_options();
  // Read input, format=vguitar, trax, gp5
  if (Options.has_input){
    file_seq *fsn = Options.input_file;
    //    global_time = 0;
    while (fsn){
      read_input_file(fsn->filename, Options.input_format);
      fsn = (file_seq*)fsn->next;
    }
  }
  //  if (Options.runningMode == do_tablature &&
  //      Options.has_input) read_input_file();
  //  exit(0);

  if (Options.headless){
    Options.clip_end = calc_max_time();
    goto Headless;
  }

  initscr();

  guitar.init();

  cbreak();
  keypad(stdscr, TRUE);
  noecho();
  
  /* Draw neck */
  guitar.draw_neck();

  /* Draw strings */
  guitar.draw_strings();

  Options.clip_end = calc_max_time();

  if (Options.do_clip) guitar.draw_clip();

  guitar.draw_current_mode();

  guitar.update_tablature();
  kp.row = 1;
  kp.col = 0;

  while (running){
    // if (Options.runningMode == do_strum)
    //   while (key_strum(&kp,&play,&piano_roll)){
    // 	// redraw basetime and frets
    // 	draw_basetime();
    //      
    // 	draw_strings();
    //
    // 	update_tablature();
    //
    // 	/* draw cursor */
    // 	move(kp.row*string_size,(kp.col+1)*vtime_size); //addch(kp.fret);
    //
    // 	refresh();
    // 	if (play) strum_voices(gnos, piano_roll);
    //   }
    // else 
    if (Options.runningMode == do_strum2) {
      guitar.drawing();
      guitar.draw_remembered_chords();
      guitar.update_strum2();
      guitar.draw_current_mode();
      guitar.update_cursor();
      guitar.update_refresh();
      if (play){
	for (iloop=0;iloop<Options.loop;iloop++){
	  if (!Options.current_strum_sequence){
	    // strum_voices(gnos, piano_roll);
	    //	  post *chord = setchord(3, 2, 3, 0, 0, -1);
	    //	  strum_voices(chord, piano_roll);
	    strum_voices2(strings_used, strings_col, piano_roll, strings_picked);
	  }
	  else {
	    int beats_per_minute = Options.bpm, start, end;
	    double beat_duration = 60./(double)beats_per_minute;
	    int msec = (int)floor(beat_duration * 1e6);
	    char *ssq = Options.current_strum_sequence->sequence, cseq = *ssq;
	    chord *achord;
	    int    i;

	    while (cseq){
	      switch (cseq){
		//	    case 'E':
		//	      strum_voices2(strings_used, strings_col, piano_roll);
		//	      usleep(msec*0.5);
		//	      break;
	      case 'u':
		//case 'U':
		strum_voices2(strings_used, strings_col, PIANO_UP, strings_picked);
		//strum_voices(gnos, PIANO_UP);
		usleep(msec*0.5);
		break;
	      case 'd':
		//	    case 'D':
		strum_voices2(strings_used, strings_col, PIANO_DOWN, strings_picked);
		//	      strum_voices(chord, PIANO_DOWN);
		usleep(msec*0.5);
		break;
	      case 'p':
		//	    case 'D':
		strum_voices2(strings_used, strings_col, PIANO_PICK, strings_picked);
		//	      strum_voices(chord, PIANO_DOWN);
		usleep(msec*0.5);
		break;
	      default:
		break;
	      }
	      if (isdigit(cseq)) usleep(msec*(cseq-'0'));
	      // pop/play stack chord
	      else if (cseq=='P'){
		cseq =*++ssq;                         // read next char
		chord *achord=chord_keybound(cseq);
		if (achord) chord_push(achord);
	      }
	      else if (cseq=='M'){    
		achord=chord_pop();
		if (achord)
		  for (i=0;i<NUM_GSTRINGS;i++){
		    open_strings[i] = achord->open_strings[i];
		    strings_used[i] = achord->strings_used[i];
		    strings_col[i] = achord->strings_col[i];
		  }
	      }
	      else if (achord=chord_keybound(cseq)){
		//    printf("keybinding %c found. chord\n", ch);
		for (i=0;i<NUM_GSTRINGS;i++){
		  open_strings[i] = achord->open_strings[i];
		  strings_used[i] = achord->strings_used[i];
		  strings_col[i] = achord->strings_col[i];
		}
	      }

	      cseq =*++ssq;
	    }
	  }
	}
      }
      guitar.key_strum2(&kp,&play,&piano_roll, &running);
    } 
    else if (Options.runningMode == do_box) {
	guitar.drawing();
	guitar.update_box();
	guitar.draw_current_mode();
	guitar.update_cursor();
	guitar.update_refresh();
	if (play){
	  for (iloop=0;iloop<Options.loop;iloop++)
	    output_voices_alltuning(gnos, false);
	}
	guitar.key_box(&kp, &play,&piano_roll, &running);
    }
    else {
      guitar.drawing();
      guitar.update_tablature();
      guitar.draw_current_mode();
      guitar.update_cursor();
      guitar.update_refresh();
      if (play){
	for (iloop=0;iloop<Options.loop;iloop++)
	  output_voices_alltuning(gnos, false);
      }
      guitar.key_tablature(&kp, &play, &running);
    }
  }

  //while (guitar.key_tablature(&kp, &play, &running));
  //    guitar.draw_current_mode();
  // while running 

  endwin();

 Headless:
  if (Options.runningMode == do_strum2){
    chord_dump();
    play_strum2(piano_roll);
  }
  else for (iloop=0;iloop<Options.loop;iloop++) output_voices_alltuning(gnos, true);

  //  print_options();

  if (Options.has_output) write_output_file();

  return 0;
}

void vguitar::draw_clear_chords()
{
  int   j;
  for (j=0;j<80;j++){
    move(number_strings+2,nut+j); addch(32);
    move(number_strings+3,nut+j); addch(32);
  }
}

/* neck position 0 and 7 */
void vguitar::draw_remembered_chords()
{
  char ch, *sequence;
  chord *remembered = Options.chords;
  int vtime, i = 1, j, k;

  // clear the row
  for (j=0;j<80;j++){
    move(number_strings+2,nut+j); addch(32);
    move(number_strings+3,nut+j); addch(32);
  }

  while (remembered){
    move(number_strings+2,nut+(i)*vtime_size+1); addch(remembered->key_binding); 
    //    if (remembered == Options.current)
    i += 1;
    remembered = remembered->next;
  }

  sstrum *strum = Options.strum_sequence;

  while (strum){
    sequence = strum->sequence;
    for (j=0;j<strlen(sequence);j++){
      ch = sequence[j];
      move(number_strings+2,nut+(i+1)*vtime_size+j); addch(ch);
    }
    if (Options.current_strum_sequence == strum){
      move(number_strings+3,nut+(i+1)*vtime_size); addch('-');
    }
    k = j/vtime_size+1;
    i+=k;
    strum = strum->next;
  }
}

void vguitar::draw_neck()
{
  if (Options.runningMode == do_box) draw_box_neck();
  else draw_plain_neck();
}

void vguitar::draw_plain_neck()
{
  int vtime, i, ch;
  for (vtime=0;vtime<number_vtimes;vtime++){
      ch = '=';
      for (i=0;i<vtime_size;i++){ move(number_strings+1,nut+(vtime+1)*vtime_size+i); addch(ch); }
      for (i=0;i<vtime_size;i++){ move(0,nut+(vtime+1)*vtime_size+i); addch(ch); }
  }
}

void vguitar::draw_box_neck()
{
  int vtime, i, ch;
  for (vtime=0;vtime<number_vtimes;vtime++){
    if (vtime==3||vtime==5||vtime==7||vtime==9||vtime==12||vtime==15||vtime==17||vtime==19||vtime==21){
      ch = '.';
      for (i=0;i<1;i++){ move(number_strings+1,nut+(vtime+1)*vtime_size+i); addch(ch); }
      for (i=0;i<1;i++){ move(0,nut+(vtime+1)*vtime_size+i); addch(ch); }
      for (i=1;i<vtime_size;i++){ move(number_strings+1,nut+(vtime+1)*vtime_size+i); addch('='); }
      for (i=1;i<vtime_size;i++){ move(0,nut+(vtime+1)*vtime_size+i); addch('='); }
    } else {
      ch = '=';
      for (i=0;i<vtime_size;i++){ move(number_strings+1,nut+(vtime+1)*vtime_size+i); addch(ch); }
      for (i=0;i<vtime_size;i++){ move(0,nut+(vtime+1)*vtime_size+i); addch(ch); }
    }
  }
}

/* strings are 1-indexed */
void vguitar::draw_bridge(int strings_picked[NUM_GSTRINGS])
{
  int  string, vtime;
  if (Options.runningMode == do_tablature) return;

  for (string=1;string<number_strings+1;string++){
    move((NUM_GSTRINGS+1-string)*string_size,nut+number_vtimes*vtime_size);
    addch(strings_picked[string-1]+'0');
  }
}

/* strings are 1-indexed */
void vguitar::draw_strings()
{
  int  i, string, vtime;
  if (Options.runningMode == do_strum2){
    for (string=1;string<number_strings+1;string++){
      move((NUM_GSTRINGS+1-string)*string_size,nut+0);
      // open or closed strings
      if (open_strings[string-1])
	addch('O');
      else
	addch('X');
    }
  } else {
#if 0
    //    for (string=1;string<number_strings+1;string++){
    // standard EADGBE
    // openD (from lowest to highest): D A D F♯ A D. 
    string = 1;
    move(string*string_size,nut+0);
    if (Options.tuning == TUNING_EADGBE) addch('E');
    else addch('D');
    string = 2;
    move(string*string_size,nut+0);
    if (Options.tuning == TUNING_EADGBE) addch('B');
    else addch('A');
    string = 3;
    move(string*string_size,nut+0);
    if (Options.tuning == TUNING_EADGBE)    addch('G');
    else { addch('F');  addch('#'); }
    string = 4;
    move(string*string_size,nut+0);
    if (Options.tuning == TUNING_EADGBE)    addch('D');
    else addch('D');
    string = 5;
    move(string*string_size,nut+0);
    if (Options.tuning == TUNING_EADGBE)    addch('A');
    else addch('A');
    string = 6;
    move(string*string_size,nut+0);
    if (Options.tuning == TUNING_EADGBE)    addch('E');                  
    else addch('D');
    //    }
#endif
    int i;
    char standard_strings[]="EBGDAE";
    char general_strings[]="654321";
    char *draws;
    if (Options.tuning == TUNING_EADGBE) draws = standard_strings;
    else draws = general_strings;

    for (i=0;i<6;i++){
      // EBGDAE
      string = i+1;
      move(string*string_size,nut+0); addch(draws[i]);
    }
#if 0
    string = 2;
    move(string*string_size,nut+0);addch('B');
    string = 3;
    move(string*string_size,nut+0);addch('G');
    string = 4;
    move(string*string_size,nut+0);  addch('D');
    string = 5;
    move(string*string_size,nut+0);   addch('A');
    string = 6;
    move(string*string_size,nut+0);addch('E');                  
#endif
  }
  /* Draw six strings, for visible vtime */
  for (string=1;string<number_strings+1;string++){
    for (vtime=0;vtime<number_vtimes;vtime++){
      //      move(string*string_size,nut+(vtime+1)*vtime_size); 
      //      for (i=0;i<1;i++){ addch('-'); addch(' '); } // lengthening
      move(string*string_size,nut+(vtime+1)*vtime_size-1); 
      for (i=0;i<1;i++){ addch(' '); addch('-'); addch(' '); } // lengthening

    }
  }
}

void vguitar::draw_clear_clip()
{
  int   vtime, i;
  for (vtime=0;vtime<number_vtimes;vtime++)
    for (i=0;i<vtime_size;i++){ move(neck_low,nut+(vtime+1)*vtime_size+i); addch('='); }
}

void vguitar::draw_current_mode()
{
  
  move(neck_high, nut+vtime_size);
  if (Options.runningMode == do_tablature){
    addch('T');  addch('A');  addch('B');   // TAB
  }
  if (Options.runningMode == do_strum ||
      Options.runningMode == do_strum2){    // STM
    addch('S');  addch('T');  addch('M');
  }
  if (Options.runningMode == do_box){       // MAP or BOX
    addch('B');  addch('O');  addch('X');
  }
}

void vguitar::draw_clip()
{
  int   clip_start, clip_end;
  int   vtime, i;
  for (vtime=0;vtime<number_vtimes;vtime++)
    for (i=0;i<vtime_size;i++){ move(neck_low,nut+(vtime+1)*vtime_size+i); addch('='); }

  clip_start = (Options.clip_start-basetime);
  clip_end = (Options.clip_end-basetime);

  if (clip_start >= 0 && clip_start < number_vtimes){
    move(neck_low, nut+(clip_start+1)*vtime_size);
    addch('[');
  }
  if (clip_end >= 0 && clip_end < number_vtimes){
    move(neck_low, nut+(clip_end+1)*vtime_size);
    addch(']');
  }
  for (i=0;i<vtime_size;i++){ move(neck_low,nut+(vtime+1)*vtime_size+i); addch('='); }
}

void vguitar::draw_basetime()
{
  int    places[2];    // can extend
  if (basetime >= 10){
    places[ONEPLACE] = basetime%10;
    places[TENPLACE] = (basetime-places[ONEPLACE])/10;
  }
  else {
    places[ONEPLACE] = basetime;
    places[TENPLACE] = 0;
  }
  move(neck_low, nut+0);
  addch(places[TENPLACE]+'0');
  addch(places[ONEPLACE]+'0');
  move(neck_high, nut+0);
  addch(places[TENPLACE]+'0');
  addch(places[ONEPLACE]+'0');
}

void vguitar::cycle_runningmodes(int sense)
{      // cycle thru runningmode
  if (sense > 0){
    Options.runningMode++;
    if (Options.runningMode == do_strum) Options.runningMode++;
    if (Options.runningMode > do_box) Options.runningMode = do_tablature;
    Options.do_clip = true;
  }
  else {
    if (Options.runningMode == do_tablature) Options.runningMode = do_box;
    else Options.runningMode--;
    if (Options.runningMode == do_strum) Options.runningMode--;
    Options.do_clip = false;
  }
  // on entering tablature mode
  if (Options.runningMode == do_tablature){
    //    kp.row = 6; // low E string
    kp.col = (Options.clip_start-basetime);
    guitar.draw_neck();
  }
  // on entering boxmode
  if (Options.runningMode == do_box){
    //    kp.row = 6;  // low E string
    kp.col = 0;
    zero_boxlet();  
  }
}

/* in key_box mode the time is the entry
   and the row and columns are the string and fret position.

   string == row
   fret == col
   time == entry
*/
int vguitar::key_box(finger_press *kp, int *play, int *piano_roll, int *running)
{
  unsigned int ch;
  post *apost;
  int a_rest = false;

  *play = false;

  /* Process keyboards */
  //  while (1){
  ch = getch();
  if (ch=='q'){ *running=0; return 0; }
  else if (ch==KEY_HOME) cycle_runningmodes(+1);
  else if (ch==KEY_END) cycle_runningmodes(-1); 
  // add note to the song
  else if (isdigit(ch)){
    int time, string, fret;
    if (ch >= 'a') time = (ch - 'a');
    else if (ch >= 'A') time = (ch - 'A');
    else time = (ch - '0');
    time += Options.clip_start;
    string = NUM_GSTRINGS+1-kp->row;
    fret = kp->col; // +basetime);
    /*    fprintf(stderr, "tsf %d %d %d", time, string, fret); */
    add_to_song(time, string, fret);
  }
  else if (ch=='c'||ch==32) *play=true;
  else
    switch (ch){
    case 'n':
    case KEY_DOWN:
      kp->row += 1;
      break;
    case 'p':
    case KEY_UP:
      kp->row -= 1;
      break;
    case 'l':
    case KEY_RIGHT:
    case 9://tab
      kp->col += 1;
      break;
    case 'j':
    case KEY_LEFT:
      kp->col -= 1;
      break;
    default:
      break;
    }
  
  // Row bounds. virtual basetime, wraparound strings
  if (kp->row < string_EH) kp->row = string_E; 
  else if (kp->row > string_E ) kp->row = string_EH;

  // Column bounds.
  if (kp->col < 0){
    kp->col = 0;
  }
  else if (kp->col >= number_vtimes){
    kp->col = number_vtimes -1; 
  }
  return 1;
}

void
vguitar::add_to_song(int time, int string, int fret)
{
  post *apost;
  int a_rest = false;

  apost = post_fret(string, time);
  if (apost) post_add_digit(apost, fret); 
  else {
    if (song){
      song->next = post_new(string, time, fret, 1.0, a_rest, TABLATURE);
      song = song->next;
    }
    else gnos = song = post_new(string, time, fret, 1.0, a_rest, TABLATURE);
  }
}

int vguitar::key_tablature(finger_press *kp, int *play, int *running)
{
  unsigned int ch;
  post *apost;
  int a_rest = false;

  *play = false;
  /* Process keyboards */
  //  while (1){
    ch = getch();
    // quit loop
    if (ch=='q'){ *running=0; return 0; }
    else if (ch==KEY_HOME) cycle_runningmodes(+1); 
    else if (ch==KEY_END) cycle_runningmodes(-1);
    else
    if (ch=='c'||ch==32) *play=true;
    else if (ch=='e'||ch=='E'){
      kp->row = string_E;
    }
    else if (ch=='a'||ch=='A'){
      kp->row = string_A;
    }
    else if (ch=='d'||ch=='D'){
      kp->row = string_D;
    }
    else if (ch=='g'||ch=='G'){
      kp->row = string_G;
    }
    else if (ch=='b'||ch=='B'){
      kp->row = string_B;
    }
    else if (ch=='h'||ch=='H'){
      kp->row = string_EH;
    }
    // '[' start clip interval
    else if (ch=='[') Options.clip_start = kp->col+basetime;
    // '[' end clip interval
    else if (ch==']') Options.clip_end = kp->col+basetime;
    // '-' lengthen the note:  eighth (0.5), quarter (1.0), half (1.5)
    else if (ch=='-'){
      apost = post_fret((NUM_GSTRINGS+1-kp->row),kp->col+basetime);
      if (apost && apost->duration < 1.5) apost->duration += 0.5;
    }
    // '.' shorten the note:  eighth (0.5), quarter (1.0), half (1.5)
    else if (ch=='.'){
      apost = post_fret((NUM_GSTRINGS+1-kp->row),kp->col+basetime);
      if (apost && apost->duration > 0.5) apost->duration -= 0.5;
    }
    else if (ch=='<'){ move_times(kp->col+basetime,-1); kp->col--;}
    else if (ch=='>'){ move_times(kp->col+basetime,+1); kp->col++;}
    // bending
    else if (ch=='^'){
      apost = post_fret((NUM_GSTRINGS+1-kp->row),kp->col+basetime);
      post_bend(apost); 
    }
    // octothorpe rest
    else if (ch=='#'){ 
      fret = OCTOTHORPE;  
      apost = post_fret((NUM_GSTRINGS+1-kp->row),kp->col+basetime);
      if (apost) post_add_digit(apost, fret); 
      else {
	if (song){
	  song->next = post_new((NUM_GSTRINGS+1-kp->row), kp->col+basetime, fret, 1.0, a_rest, TABLATURE);
	  song = song->next;
	}
	else gnos = song = post_new((NUM_GSTRINGS+1-kp->row), kp->col+basetime, fret, 1.0, a_rest, TABLATURE);
      }
    }
    // add note to the song
    else
    if (isdigit(ch)){
      fret = (ch - '0');
      apost = post_fret((NUM_GSTRINGS+1-kp->row),kp->col+basetime);
      if (apost) post_add_digit(apost, fret); 
      else {
	if (song){
	  song->next = post_new((NUM_GSTRINGS+1-kp->row), kp->col+basetime, fret, 1.0, a_rest, TABLATURE);
	  song = song->next;
	}
	else gnos = song = post_new((NUM_GSTRINGS+1-kp->row), kp->col+basetime, fret, 1.0, a_rest, TABLATURE);
      }
    }
    else
    // correct typno
      if (ch == 'x' || ch==KEY_DC || ch==KEY_CLEAR){
      //      move(kp->row*string_size, kp->col*vtime_size);
      //      addch('*');
      apost = post_fret((NUM_GSTRINGS+1-kp->row),kp->col+basetime);
      if (apost) post_zero_digits(apost); 
    }
    else
    switch (ch){
    case 'n':
    case KEY_DOWN:
      kp->row += 1;
      break;
    case 'p':
    case KEY_UP:
      kp->row -= 1;
      break;
    case 'l':
    case KEY_RIGHT:
    case 9://tab
      kp->col += 1;
      break;
    case 'j':
    case KEY_LEFT:
      kp->col -= 1;
      break;
    default:
      break;
    }

    // Row bounds. virtual basetime, wraparound strings
    if (kp->row < string_EH) kp->row = string_E; 
    else if (kp->row > string_E ) kp->row = string_EH;

    // Column bounds.
    if (kp->col < 0){
      kp->col = 0;
      basetime -= 1;
      if (basetime < 0) basetime = 0;
    }
    else if (kp->col >= number_vtimes){
      kp->col = number_vtimes -1; 
      basetime += 1;
    }
    return 1;
}


int vguitar::key_strum(finger_press *kp, int *play, int *piano_roll)
{
  unsigned int ch;
  post *apost;
  int a_rest = false;

  /* Process keyboards */
  //  while (1){
  *play=false;
  ch = getch();
  // quit loop
  if (ch=='q') return 0;       // quit
  else if (isdigit(ch)) kp->col = ch -'0';
  // add note to the song
  else if (ch=='e'||ch=='E'){
    kp->row = string_E;
  }
  else if (ch=='a'||ch=='A'){
    kp->row = string_A;
  }
  else if (ch=='d'||ch=='D'){
    kp->row = string_D;
  }
  else if (ch=='g'||ch=='G'){
    kp->row = string_G;
  }
  else if (ch=='b'||ch=='B'){
    kp->row = string_B;
  }
  else if (ch=='h'||ch=='H'){
    kp->row = string_EH;
  }
  else if (ch==32){ *play=true; return 1; }
  else if (ch=='k') {   // space key-end
    apost = post_fret((NUM_GSTRINGS+1-kp->row),kp->col+basetime);
    if (apost){
      if (apost->unused) apost->unused = 0;
      else apost->unused = 1;        //post_add_digit(apost, 0); 
    }
    else {
      if (song){
	song->next = post_new((NUM_GSTRINGS+1-kp->row), kp->col+basetime, 0, 1.0, a_rest, TABLATURE);
	song = song->next;
      }
      else gnos = song = post_new((NUM_GSTRINGS+1-kp->row), kp->col+basetime, 0, 1.0, a_rest, TABLATURE);
    }
  }
  // mute
  else if (ch == 'm') *play = !*play;
  // piano_roll
  else if (ch == 'u') (*piano_roll)>0?*piano_roll=0:*piano_roll=1;
  // roll factor
  else if (ch == '.') Options.roll_factor *= 0.5;
  else if (ch == '-') Options.roll_factor *= 2.0;
  //    else if (ch==32){ *play=true; return 1; } // play
  // correct typno
  else if (ch==KEY_DC || ch==KEY_CLEAR){
    apost = post_fret((NUM_GSTRINGS+1-kp->row),kp->col+basetime);
    if (apost) post_zero_digits(apost); 
  }
  else
    switch (ch){
    case 'n':
    case KEY_DOWN:
      kp->row += 1;
      break;
    case 'p':
    case KEY_UP:
      kp->row -= 1;
      break;
    case 'l':
    case KEY_RIGHT:
      //      case 32:
      //      case 9://tab
      kp->col += 1;
      break;
    case 'j':
    case KEY_LEFT:
      kp->col -= 1;
      break;
    default:
      break;
    }

  // Row bounds. virtual basetime, wraparound strings
  if (kp->row < string_EH) kp->row = string_E; 
  else if (kp->row > string_E ) kp->row = string_EH;

  // Column bounds.
  if (kp->col < 0){
    kp->col = 0;
    basetime -= 1;
    if (basetime < 0) basetime = 0;
  }
  else if (kp->col >= number_vtimes){
    kp->col = number_vtimes -1; 
    basetime += 1;
  }
  return 1;
}

int vguitar::key_strum2(finger_press *kp, int *play, int *piano_roll, int *running)
{
  unsigned int ch;
  post *apost;
  chord *achord;
  int i;

  /* Process keyboards */
  //  while (1){
  *play=false;
  ch = getch();
  // quit loop
  if (ch=='q'){ *running=0; return 0; }     // quit
  // Left Hand fretz
  // qwert
  //  else if (isdigit(ch)) kp->col = ch -'0';
  // add note to the song
  //  if (ch=='q'||ch=='Q'){
  //    kp->col = 0;  
  //    kp->row--;
  //  }
  // push chord onto stack ch=P (plus)
  else if (ch=='P'){
    chord_push(chord_set());
  }
  // pop chord from stack ch=M (minus)
  else if (ch=='M'){
    chord *achord = chord_pop();
    if (achord)
      for (i=0;i<NUM_GSTRINGS;i++){
	open_strings[i] = achord->open_strings[i];
	strings_used[i] = achord->strings_used[i];
	strings_col[i] = achord->strings_col[i];
      }
  }
  // keybound, remember chord
  else if (ch=='R'){
    char key = getch();
    chord *achord;
    achord = chord_keybound(key);
    if (!achord){
      achord = chord_new(key);
      if (!Options.chords) Options.chords = achord;
      else {
	achord->next = Options.chords;
	Options.chords = achord;
      }
    }
    chord_remember(achord);    
  }
  // keybound, lookup chord
  else if (achord=chord_keybound(ch)){
    //    printf("keybinding %c found. chord\n", ch);
    for (i=0;i<NUM_GSTRINGS;i++){
      open_strings[i] = achord->open_strings[i];
      strings_used[i] = achord->strings_used[i];
      strings_col[i] = achord->strings_col[i];
    }
  }
  // Right Hand
  else if (ch==32){ *play=true; return 1; }
  else if (ch=='o'){
    if (kp->row >= 1 && kp->row <= 6)
      open_strings[NUM_GSTRINGS-kp->row] = !open_strings[NUM_GSTRINGS-kp->row];
  }
  else if (ch=='k') {   // space key-end
    strings_used[NUM_GSTRINGS-kp->row] = !strings_used[NUM_GSTRINGS-kp->row];
    strings_col[NUM_GSTRINGS-kp->row] = kp->col+basetime+1;
  }
  // mute
  else if (ch == 'm') *play = !*play;
  // piano_roll
  else if (ch == 'u') (*piano_roll)>0?*piano_roll=0:*piano_roll=1;
  // roll factor
  else if (ch == '.') Options.roll_factor *= 0.5;
  else if (ch == '-') Options.roll_factor *= 2.0;
  //    else if (ch==32){ *play=true; return 1; } // play
  // can call this cycle_modes();
  // correct typno
  else if (ch==KEY_HOME){ cycle_runningmodes(+1); return 0; }
  else if (ch==KEY_END){ cycle_runningmodes(-1); return 0; }
  else if (ch==KEY_F0){
    fprintf(stderr, "key F0\n");
  }
  else if (ch==KEY_F0+1){
    fprintf(stderr, "key F1\n");
  }
  else if (ch==KEY_NPAGE){
    if (Options.current_strum_sequence && Options.current_strum_sequence->next) 
      Options.current_strum_sequence = Options.current_strum_sequence->next;
  }
  else if (ch==KEY_PPAGE){
    if (Options.current_strum_sequence && Options.current_strum_sequence->prev) 
      Options.current_strum_sequence = Options.current_strum_sequence->prev;
  }
  else
    switch (ch){
    case 'n':
    case KEY_DOWN:
      kp->row += 1;
      break;
    case 'p':
    case KEY_UP:
      kp->row -= 1;
      break;
    case 'l':
    case KEY_RIGHT:
      //      case 32:
      //      case 9://tab
      kp->col += 1;
      break;
    case 'j':
    case KEY_LEFT:
      kp->col -= 1;
      break;
    default:
      break;
    }

  // Row bounds. virtual basetime, wraparound strings
  if (kp->row < string_EH) kp->row = string_E; 
  else if (kp->row > string_E ) kp->row = string_EH;

  // Column bounds.
  if (kp->col < 0){
    kp->col = 0;
    basetime -= 1;
    if (basetime < 0) basetime = 0;
  }
  else if (kp->col >= number_vtimes){
    kp->col = number_vtimes -1; 
    basetime += 1;
  }
  return 1;
}

void vguitar::update_box()
{
  int time=1, string=1, fret=1;
  int start = Options.clip_start, end = Options.clip_end, current_time;
  post   *apost;
  short shim;

  zero_boxlet();  // on entering boxmode
  for (current_time = start; current_time <= end; current_time++){
    //    fprintf(stderr, "time %d\n", current_time);
    // noteon
    apost = gnos;  
    while (apost){

      if (apost->unused){ apost = apost->next; continue; }
      if (apost->entry == OCTOTHORPE){ apost = apost->next; continue; }
      if (apost->col != current_time){ apost = apost->next; continue; }

      // play current_time posts
      //printf(_("play time %d (%f) string %d fret %d\n"), apost->col, apost->duration, apost->row, apost->entry);

      time   = apost->col - start;
      string = apost->row;
      fret   = apost->entry;

      shim = compute_shim(string, fret);
      move((NUM_GSTRINGS+1-string)*string_size, nut+(fret+1)*vtime_size+shim);
      if (0){
	addch('-');
	move((NUM_GSTRINGS+1-string)*string_size, nut+(fret+1)*vtime_size+1);	
	addch(' ');
      }
      else {
	//	move((NUM_GSTRINGS+1-string)*string_size, nut+(fret+1)*vtime_size); 
	if (time<10) addch(time+'0');
	else addch(time+'a'-10);
      }
    apost = apost->next;
    }
  }
}

short vguitar::compute_shim(int string, int fret)
{
  short *twink, shim, parity;
  twink = &(boxlet[string][fret]);
  parity=(*twink)%3;
  switch (parity) {
  case 0:
    shim = -1;
    break;
  case 1: 
    shim = +1;
    break;
  case 2: 
    shim = 0;
    break;
  }
  *twink = *twink+1;
  return shim;
}

void vguitar::update_tablature()
{
  int     tenplace, i;
  post   *apost = gnos;
  while (apost){

    // Column bounds.
    int col = apost->col-basetime;
    if (col < 0 || col >= number_vtimes){
      apost=apost->next;
      continue;
    }
				  
    // draw tabulature
    move((NUM_GSTRINGS+1-apost->row)*string_size, nut+(apost->col-basetime+1)*vtime_size);
    if (apost->unused){
      addch('-');
      move((NUM_GSTRINGS+1-apost->row)*string_size, nut+(apost->col-basetime+1)*vtime_size+1);	
      addch(' ');
    }
    /* else if (apost->fret[TENPLACE] > 0){ */
    /*   addch(apost->fret[TENPLACE]+'0'); */
    /*   move((NUM_GSTRINGS+1-apost->row)*string_size, (apost->col-basetime+1)*vtime_size+1); */
    /*   addch(apost->fret[ONEPLACE]+'0'); */
    /* } */
    /* else addch(apost->fret[ONEPLACE]+'0'); */
    else {
      if (apost->entry == OCTOTHORPE) addch('#');        // octothorpe 
      else {
	tenplace = (apost->entry/10);
	if (tenplace){
	  addch(tenplace+'0');
	  move((NUM_GSTRINGS+1-apost->row)*string_size, nut+(apost->col-basetime+1)*vtime_size+1); 
	  addch((apost->entry%10)+'0');
	} 
	else addch((apost->entry%10)+'0');
	if (apost->bend){
	  if (tenplace) move((NUM_GSTRINGS+1-apost->row)*string_size, nut+(apost->col-basetime+1)*vtime_size+(vtime_size/2));
	  else move((NUM_GSTRINGS+1-apost->row)*string_size, nut+(apost->col-basetime+1)*vtime_size+(vtime_size/3));
	  addch('^');
	} 
	else {
	  if (tenplace) move((NUM_GSTRINGS+1-apost->row)*string_size, nut+(apost->col-basetime+1)*vtime_size+(vtime_size/2));
	  else move((NUM_GSTRINGS+1-apost->row)*string_size, nut+(apost->col-basetime+1)*vtime_size+(vtime_size/3));
	  addch(' ');
	}
      }
    }

    // duration half and eighth notes
    if (apost->duration == 1.5) addch('-');
    else if (apost->duration == 0.5) addch('.');
    apost = apost->next;
  }
}

void vguitar::update_strum2()
{
  int string;
  /* neck position 0 and 7 */
  /* end at 0 */
  for (string=1;string<number_strings+1;string++){
    move((NUM_GSTRINGS+1-string)*string_size, nut+(strings_col[string-1])*vtime_size);
    if (strings_used[string-1])  addch('0');
#if 0
    if (!strings_used[string-1]){
      addch('-');
      move((NUM_GSTRINGS+1-string)*string_size, nut+(strings_col[string-1]+1)*vtime_size+1);	
      addch(' ');
    }
    else addch('0');
#endif
  }
}

post *post_new(int row, int col, int fret, float duration, int a_rest, post_type a_boxed)
{
  struct post *p;

  p = (struct post*)malloc(sizeof(struct post));
  p->row = row;
  p->col = col;
  //  p->digit = 0;
  p->unused = false;
  //  p->fret[TENPLACE] = 0;
  //  p->fret[ONEPLACE] = fret;
  p->boxed = a_boxed;
  p->entry = fret;
  p->duration = duration;
  p->next = NULL;
  p->rest = a_rest;
  p->bend = false;
  return p;
}

post *post_fret(int row, int col)
{
  struct post *p = gnos;
  while (p){
    if (p->row == row && p->col == col) return p;
    p=p->next;
  }
  return NULL; 
}

post *post_bend(post *apost)
{
  if (apost){ 
    if (apost->bend) apost->bend = false;
    else apost->bend = true; 
    return apost;
  }
  return NULL; 
}

void post_zero_digits(post *p)
{
  p->unused = true;
  p->col = -1;
  p->entry = 0;
  //  p->fret[TENPLACE] = 0;
  //  p->fret[ONEPLACE] = 0;
}
void post_add_digit(post *p, int fret)
{
  if (fret == OCTOTHORPE){ p->entry = OCTOTHORPE; return; } // octothorpe
  p->entry = 10*p->entry + fret;
  if (p->entry >= 100) p->entry = 0;
  return;
#if 0
  if (Options.do_single_digit_fret == true){
  }
#endif
}

post *
song_addpost_at_index(post *song, post *apost, int sequence_index)
{
#if 0
  post *a_song = song, *b_song = NULL;
  if (!song){
    if (sequence_index == 0) return apost;
    printf("Error: song_addpost_at_index: index=%d song NULL\n", sequence_index);
    return song;
  }
  while (sequence_index > 0){
    b_song = a_song;
    a_song = a_song->next;
    sequence_index--;
  }
  //  insert apost between b_song and a_song;
  if (b_song) b_song->next = apost;
  apost->next = a_song;
  broke needs work.
#endif
  return song;
}

sstrum *
sstrum_new(char *seq)
{
  sstrum *s = (sstrum*)malloc(sizeof(struct sstrum));
  if (!s){
    printf("can't allocate strum\n");
    return NULL;
  }
  s->sequence = strdup(seq);
  s->next = NULL;
  s->prev = NULL;
  return s;
}

chord *
chord_new(unsigned char key_binding)
{
  chord *p = (struct chord*)malloc(sizeof(struct chord));
  if (!p){
    printf("can't allocate chord\n");
  }
  p->key_binding = key_binding;
  p->next = NULL;
  p->stack = NULL;
  //  printf("key_binding %c\n", p->key_binding);
  return p;
}

chord *
chord_keybound(char key)
{
  chord *achord = Options.chords;
  while (achord){
    if (achord->key_binding == key) return achord;
    achord = (chord*)achord->next;
  }
  return (chord*)NULL;
}

chord *
chord_pop()
{
  chord *achord = Options.chord_stack;
  if (Options.chord_stack)
    Options.chord_stack = Options.chord_stack->stack;
  //  if (!achord) Options.chord_stack = Options.chords;
  return (chord*)achord;
}

chord *
chord_push(chord *achord)
{
  achord->stack = Options.chord_stack;
  Options.chord_stack = achord;
  return achord;
}

chord *chord_set()
{
  int i;
  chord   *achord = chord_new(NULL);
  return chord_remember(achord);
  /*
  for (i=0;i<NUM_GSTRINGS;i++){
    achord->open_strings[i] = open_strings[i];
    achord->strings_used[i] = strings_used[i];
    achord->strings_col[i]  = strings_col[i];
  }
  return achord;
  */
}

chord *chord_remember(chord *achord)
{
  int i;
  if (!achord) return NULL;
  for (i=0;i<NUM_GSTRINGS;i++){
    achord->open_strings[i] = open_strings[i];
    achord->strings_used[i] = strings_used[i];
    achord->strings_col[i]  = strings_col[i];
  }
  return achord;
}

// dump all chords to a file
void
chord_dump()
{
  int i;
  chord *achord = Options.chords;
  char string_letters[]="eadgbh";

  while (achord){
    printf("--chord=\"");
    printf("%c", achord->key_binding);
    for (i=0;i<NUM_GSTRINGS;i++){
      if (!achord->open_strings[i]) printf("%cx", string_letters[i]);
      else if (achord->strings_used[i]) 
	printf("%c%d", string_letters[i], achord->strings_col[i]);
    }
    printf("\"\n");
    achord = achord->next;
  }
}

/*
pitch bend
#define snd_seq_ev_set_pitchbend(ev,ch,val) \
* \brief set pitch-bend event                                                  
* \param ev event record                                                       
* \param ch channel number                                                     
* \param val pitch bend; zero centered from -8192 to 8191                      

*/

void output_voices(post *gnos, int verbose)
{
  post     *apost = NULL;
  int beats_per_minute = Options.bpm, start, end;
  double beat_duration = 60./(double)beats_per_minute;
  //  int strings[NUM_GSTRINGS]={EHI_STRING, B_STRING, G_STRING, D_STRING, A_STRING, E_STRING};
  int strings[NUM_GSTRINGS]={E_STRING, A_STRING, D_STRING, G_STRING, B_STRING, EHI_STRING};
  int to_seq_client = Options.alsa_server_addr; //129; // sequencer client no. (fluid synth)
  int to_seq_port = Options.alsa_server_port;    // sequencer port no.
  unsigned int caps;
  int vel  = 127;        // voice volume
  int success = 0;
  int msec = (int)floor(beat_duration * 1e6);
  int current_time;
  int max_time;
  float duration = 1.0;
  //  int foo = 0;

  if (verbose)
    printf("standard strings tuning E(%d),A(%d),D(%d),G(%d),B(%d),E(%d)\n",
	   strings[0],
	   strings[1],
	   strings[2],
	   strings[3],
	   strings[4],
	   strings[5]);

  // Open an Output voice (or Input listener) Sequence, 
  // Get assigned id, and set name. 
  snd_seq_open(&seq_handle, "hw", SND_SEQ_OPEN_OUTPUT, 0);
  my_client = snd_seq_client_id(seq_handle);
  if (verbose) printf(_("sequence client id %d\n"), my_client);
  snd_seq_set_client_name(seq_handle, client_name);

  // Create the output port
  //  snd_seq_set_client_group(seq_handle, "input");
  //  caps = SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_WRITE;
  caps = SND_SEQ_PORT_CAP_WRITE;  
  //  if (to_seq_client == SND_SEQ_ADDRESS_SUBSCRIBERS)
  //    caps |= SND_SEQ_PORT_CAP_SUBS_READ;
  my_port = snd_seq_create_simple_port(seq_handle, client_name, caps,
				       SND_SEQ_PORT_TYPE_MIDI_GENERIC |
				       SND_SEQ_PORT_TYPE_APPLICATION);
  if (verbose) printf(_("from addr: %d:%d\n"), my_client, my_port);
  if (verbose) printf(_("to addr %d:%d\n"), to_seq_client, to_seq_port);

  // connect to to_seq
  success = snd_seq_connect_to(seq_handle, my_port, to_seq_client, to_seq_port);
  if (verbose) printf(_("success %d\n"), success);

  // event processing
  snd_seq_ev_set_dest(&event, to_seq_client, to_seq_port);
  snd_seq_ev_set_direct(&event);
  snd_seq_ev_set_source(&event, my_port);
  //  snd_seq_ev_set_dest(&event, to_seq_client, to_seq_port);

  /* or  snd_seq_ev_set_subs(&event);        */
  /* terribly innefficient */
  max_time = 0;
  apost = gnos;  
  while (apost){
    if (apost->unused){ apost = apost->next; continue; }
    max_time = MAX(max_time, apost->col);
    apost = apost->next;
  }
  if (Options.do_clip == true){
    start = Options.clip_start;
    end = Options.clip_end;
  } else {
    start = 0;
    end = max_time;
  }
  for (current_time = start; current_time <= end; current_time++){

    /* if (foo){ */
    /* apost = gnos; */
    /* while (apost){ */
    /*   if (apost->unused){ apost = apost->next; continue; } */
    /*   if (apost->col != current_time){ apost = apost->next; continue; } */
    /*   // play current_time posts */
    /*   //      printf("time %d string %d fret %d\n", apost->col, apost->row, apost->fret[ONEPLACE]);     */
    /*   //      snd_seq_ev_set_noteon(&event, 0, strings[apost->row-1]+apost->entry+1, vel);       */
    /*   snd_seq_ev_set_pitchbend(&event,0, 16383); */
    /*   snd_seq_event_output(seq_handle, &event); */
    /*   snd_seq_drain_output(seq_handle); */
      
    /*   apost = apost->next; */
    /* } */
    /* } */

    // noteon
    apost = gnos;  
    while (apost){
      int pit;

      //      printf("peek time %d string %d fret %d\n", apost->col, apost->row, apost->fret[ONEPLACE]);    
      if (apost->unused){ apost = apost->next; continue; }
      if (apost->col != current_time){ apost = apost->next; continue; }
      // play current_time posts
      if (verbose) printf(_("play time %d (%f) string %d fret %d\n"), apost->col, apost->duration, apost->row, apost->entry); // apost->fret[ONEPLACE]);    
      //      snd_seq_ev_set_noteon(&event, 0, strings[apost->row-1]+apost->fret[ONEPLACE], vel);

#if 0
      event.type = MIDI_CTL_PORTAMENTO_CONTROL;
      snd_seq_ev_set_fixed(&event);
      event.data.note.channel = 0;
      event.data.note.note =  strings[apost->row-1]+apost->entry - 2;
      snd_seq_event_output(seq_handle, &event);
      snd_seq_drain_output(seq_handle);
#endif
      
      // OCTOTHORPE is a rest
      if (apost->entry != OCTOTHORPE)
	snd_seq_ev_set_noteon(&event, 0, strings[apost->row-1]+apost->entry, vel);
      //      snd_seq_ev_set_noteon(&event, 0, strings[apost->row-1], vel);
      duration = apost->duration;
      snd_seq_event_output(seq_handle, &event);
      snd_seq_drain_output(seq_handle);

      //-8192..8191
      if (apost->bend)
	for (pit=0;pit<8191;pit++){	
	  snd_seq_ev_set_pitchbend(&event, 0, pit);
	  snd_seq_event_output(seq_handle, &event);
	  snd_seq_drain_output(seq_handle);
	  //	  usleep(msec*0.0001);
	  usleep(msec*0.00001);
	}


      apost = apost->next;
    }
    //    if (current_time == 2) duration = 1.5;
    //    else duration = 1.0;
    usleep(msec*duration);
    //    printf("noteoff\n");
    // noteoff
    apost = gnos;  
    while (apost){
      if (apost->unused){ apost = apost->next; continue; }
      if (apost->col != current_time){ apost = apost->next; continue; }
      // play current_time posts
      //      printf("time %d string %d fret %d\n", apost->col, apost->row, apost->fret[ONEPLACE]);    
      //      snd_seq_ev_set_noteoff(&event, 0, strings[apost->row-1]+apost->fret[ONEPLACE], 0);
      //snd_seq_ev_set_pitchbend(&event,0, 8191);
      snd_seq_ev_set_noteoff(&event, 0, strings[apost->row-1]+apost->entry, 0);      
      snd_seq_event_output(seq_handle, &event);
      snd_seq_drain_output(seq_handle);
      
      apost = apost->next;
    }
  }
  /*
    usleep(msec);
    snd_seq_ev_set_noteoff(&event, 0, strings[apost->row]+apost->col, vel);
    snd_seq_event_output(seq_handle, &event);
    snd_seq_drain_output(seq_handle);
    sleep(beat_duration);*/

}

void strum_voices(post *gnos, int piano_roll)
{
  post     *apost = NULL;
  int beats_per_minute = Options.bpm;
  double beat_duration = 60./(double)beats_per_minute;
  //  int strings[NUM_GSTRINGS]={EHI_STRING, B_STRING, G_STRING, D_STRING, A_STRING, E_STRING};
  int strings[NUM_GSTRINGS]={E_STRING, A_STRING, D_STRING, G_STRING, B_STRING, EHI_STRING};
  int strings_used[NUM_GSTRINGS]={0,0,0,0,0,0};
  int strings_col[NUM_GSTRINGS]={0,0,0,0,0,0};
  int to_seq_client = Options.alsa_server_addr; //129; // sequencer client no. (fluid synth)
  int to_seq_port = Options.alsa_server_port;    // sequencer port no.
  unsigned int caps;
  int vel  = 127;        // voice volume
  int success = 0;
  int msec = (int)floor(beat_duration * 1e6);
  int current_time;
  int max_time;
  float duration = 1.0;
  int i;
  
  // Open an Output voice (or Input listener) Sequence, 
  // Get assigned id, and set name. 
  snd_seq_open(&seq_handle, "hw", SND_SEQ_OPEN_OUTPUT, 0);
  my_client = snd_seq_client_id(seq_handle);
  //  printf("sequence client id %d\n", my_client);
  snd_seq_set_client_name(seq_handle, client_name);

  // Create the output port
  //  snd_seq_set_client_group(seq_handle, "input");
  //  caps = SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_WRITE;
  caps = SND_SEQ_PORT_CAP_WRITE;  
  //  if (to_seq_client == SND_SEQ_ADDRESS_SUBSCRIBERS)
  //    caps |= SND_SEQ_PORT_CAP_SUBS_READ;
  my_port = snd_seq_create_simple_port(seq_handle, client_name, caps,
				       SND_SEQ_PORT_TYPE_MIDI_GENERIC |
				       SND_SEQ_PORT_TYPE_APPLICATION);
  //  printf("from addr: %d:%d\n", my_client, my_port);
  //  printf("to addr %d:%d\n", to_seq_client, to_seq_port);

  // connect to to_seq
  success = snd_seq_connect_to(seq_handle, my_port, to_seq_client, to_seq_port);
  //  printf("success %d\n", success);

  // event processing
  snd_seq_ev_set_dest(&event, to_seq_client, to_seq_port);
  snd_seq_ev_set_direct(&event);
  snd_seq_ev_set_source(&event, my_port);
  //  snd_seq_ev_set_dest(&event, to_seq_client, to_seq_port);

  // noteon

  apost = gnos;  
#if 1
  while (apost){
    if (apost->unused){ apost = apost->next; continue; }
    strings_used[apost->row-1] = 1; // string has been sounded
    strings_col[apost->row-1] = apost->col;
    // duration = apost->duration;
    apost = apost->next;
  }
#endif
  if (piano_roll==PIANO_UP){
    // play open strings scale piano roll up 
    for (i=0;i<NUM_GSTRINGS;i++){
      if (open_strings[i]==0) continue;   

      if (strings_used[i]==0){
	snd_seq_ev_set_noteon(&event, 0, strings[i], vel);
	snd_seq_event_output(seq_handle, &event);
	snd_seq_drain_output(seq_handle);
	usleep(msec*Options.roll_factor);  // .1 sec
	/*
	  snd_seq_ev_set_noteoff(&event, 0, strings[i], 0);
	  snd_seq_event_output(seq_handle, &event);
	  snd_seq_drain_output(seq_handle);
	*/
      }
      else {
	snd_seq_ev_set_noteon(&event, 0, strings[i]+strings_col[i], vel);
	snd_seq_event_output(seq_handle, &event);
	snd_seq_drain_output(seq_handle);
	usleep(msec*Options.roll_factor);  // .1 sec
      }
    }
  }
  // play open strings scale piano roll down
  else {
    for (i=NUM_GSTRINGS-1;i>=0;i--){
      if (open_strings[i]==0) continue;   
      if (strings_used[i]==0){
	snd_seq_ev_set_noteon(&event, 0, strings[i], vel);
	snd_seq_event_output(seq_handle, &event);
	snd_seq_drain_output(seq_handle);
	usleep(msec*Options.roll_factor);  // .1 sec
	/*
	  snd_seq_ev_set_noteoff(&event, 0, strings[i], 0);
	  snd_seq_event_output(seq_handle, &event);
	  snd_seq_drain_output(seq_handle);
	*/
      }
      else {
	snd_seq_ev_set_noteon(&event, 0, strings[i]+strings_col[i], vel);
	snd_seq_event_output(seq_handle, &event);
	snd_seq_drain_output(seq_handle);
	usleep(msec*Options.roll_factor);  // .1 sec
      }

    }
  }
#if 0
  //  usleep(msec);
  // noteoff
  apost = gnos;  
  while (apost){
    if (apost->unused){ apost = apost->next; continue; }
    //      snd_seq_ev_set_noteoff(&event, 0, strings[apost->row-1]+apost->fret[ONEPLACE], 0);
      snd_seq_ev_set_noteoff(&event, 0, strings[apost->row-1]+apost->entry, 0);      
      //      snd_seq_ev_set_noteoff(&event, 0, strings[apost->row-1], 0);
      snd_seq_event_output(seq_handle, &event);
      snd_seq_drain_output(seq_handle);

      apost = apost->next;
    }
  }
#endif
}

void strum_voices2(int strings_used[NUM_GSTRINGS], int strings_col[NUM_GSTRINGS], int piano_roll, 
		   int strings_picked[NUM_GSTRINGS])
{
  post     *apost = NULL;
  int beats_per_minute = Options.bpm;
  double beat_duration = 60./(double)beats_per_minute;
  //  int strings[NUM_GSTRINGS]={EHI_STRING, B_STRING, G_STRING, D_STRING, A_STRING, E_STRING};
  int strings[NUM_GSTRINGS]={E_STRING, A_STRING, D_STRING, G_STRING, B_STRING, EHI_STRING};
  int to_seq_client = Options.alsa_server_addr; //129; // sequencer client no. (fluid synth)
  int to_seq_port = Options.alsa_server_port;    // sequencer port no.
  unsigned int caps;
  int vel  = 127;        // voice volume
  int success = 0;
  int msec = (int)floor(beat_duration * 1e6);
  int current_time;
  int max_time;
  float duration = 1.0;
  int i;

  if (Options.tuning == TUNING_MIDI){
    if (Options.verbose) 
      printf("MIDI number tuning %d,%d,%d,%d,%d,%d\n",
	   Options.strings_tuning_midi[0],
	   Options.strings_tuning_midi[1],
	   Options.strings_tuning_midi[2],
	   Options.strings_tuning_midi[3],
	   Options.strings_tuning_midi[4],
	   Options.strings_tuning_midi[5]);

    for (i=0;i<NUM_GSTRINGS;i++)
      strings[i]=Options.strings_tuning_midi[i];
  }
  else if (Options.tuning == TUNING_OPEND){
    strings[0]=OPEND_1_STRING;
    strings[1]=OPEND_2_STRING;
    strings[2]=OPEND_3_STRING;
    strings[3]=OPEND_4_STRING;
    strings[4]=OPEND_5_STRING;
    strings[5]=OPEND_6_STRING;    
  }

  // Open an Output voice (or Input listener) Sequence, 
  // Get assigned id, and set name. 
  snd_seq_open(&seq_handle, "hw", SND_SEQ_OPEN_OUTPUT, 0);
  my_client = snd_seq_client_id(seq_handle);
  //  printf("sequence client id %d\n", my_client);
  snd_seq_set_client_name(seq_handle, client_name);

  // Create the output port
  //  snd_seq_set_client_group(seq_handle, "input");
  //  caps = SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_WRITE;
  caps = SND_SEQ_PORT_CAP_WRITE;  
  //  if (to_seq_client == SND_SEQ_ADDRESS_SUBSCRIBERS)
  //    caps |= SND_SEQ_PORT_CAP_SUBS_READ;
  my_port = snd_seq_create_simple_port(seq_handle, client_name, caps,
				       SND_SEQ_PORT_TYPE_MIDI_GENERIC |
				       SND_SEQ_PORT_TYPE_APPLICATION);
  //  printf("from addr: %d:%d\n", my_client, my_port);
  //  printf("to addr %d:%d\n", to_seq_client, to_seq_port);

  // connect to to_seq
  success = snd_seq_connect_to(seq_handle, my_port, to_seq_client, to_seq_port);
  //  printf("success %d\n", success);

  // event processing
  snd_seq_ev_set_dest(&event, to_seq_client, to_seq_port);
  snd_seq_ev_set_direct(&event);
  snd_seq_ev_set_source(&event, my_port);
  //  snd_seq_ev_set_dest(&event, to_seq_client, to_seq_port);

  // noteon
  if (piano_roll==PIANO_UP){
    // play open strings scale piano roll up 
    for (i=0;i<NUM_GSTRINGS;i++){
      if (open_strings[i]==0) continue;   
      if (strings_used[i]==0){
	snd_seq_ev_set_noteon(&event, 0, strings[i], vel);
	snd_seq_event_output(seq_handle, &event);
	snd_seq_drain_output(seq_handle);
	usleep(msec*Options.roll_factor);  // .1 sec
      }
      else {
	snd_seq_ev_set_noteon(&event, 0, strings[i]+strings_col[i], vel);
	snd_seq_event_output(seq_handle, &event);
	snd_seq_drain_output(seq_handle);
	usleep(msec*Options.roll_factor);  // .1 sec
      }
    }
  }
  // play open strings scale piano roll down
  else if (piano_roll==PIANO_DOWN) {
    for (i=NUM_GSTRINGS-1;i>=0;i--){
      if (open_strings[i]==0) continue;   
      if (strings_used[i]==0){
	snd_seq_ev_set_noteon(&event, 0, strings[i], vel);
	snd_seq_event_output(seq_handle, &event);
	snd_seq_drain_output(seq_handle);
	usleep(msec*Options.roll_factor);  // .1 sec
      }
      else {
	snd_seq_ev_set_noteon(&event, 0, strings[i]+strings_col[i], vel);
	snd_seq_event_output(seq_handle, &event);
	snd_seq_drain_output(seq_handle);
	usleep(msec*Options.roll_factor);  // .1 sec
      }
    }
  }
  else if (piano_roll==PIANO_PICK) {
    int       j = 0;
    for (i=0;i<=NUM_GSTRINGS-1;i++){
      if (open_strings[i]==0) continue;   
      if (strings_picked[i]>j){
	snd_seq_ev_set_noteon(&event, 0, strings[i]+strings_col[i], vel);
	snd_seq_event_output(seq_handle, &event);
	snd_seq_drain_output(seq_handle);
	usleep(msec*Options.roll_factor);  // .1 sec
	j = strings_picked[i]; 
      }
    }
  }
  else printf("this can't happen\n");
  snd_seq_close(seq_handle);
}

void move_times(int col, int increment)
{
  post *anote = gnos;
  while (anote){
    if (anote->col >= col) anote->col += increment;
    anote = anote->next;
  }
}

int calc_max_time()
{
  post *anote = gnos;
  int  t = 0;
  while (anote){
    t = MAX(t, anote->col);
    anote = anote->next;
    //    fprintf(stderr, "t %d %d\n", t, anote->col);
  }
  return t;
}


void
play_strum2(int piano_roll){
  int iloop;
  for (iloop=0;iloop<Options.loop;iloop++){
    if (!Options.current_strum_sequence){
      // strum_voices(gnos, piano_roll);
      //	  post *chord = setchord(3, 2, 3, 0, 0, -1);
      //	  strum_voices(chord, piano_roll);
      strum_voices2(strings_used, strings_col, piano_roll, strings_picked);
    }
    else {
      int beats_per_minute = Options.bpm, start, end;
      double beat_duration = 60./(double)beats_per_minute;
      int msec = (int)floor(beat_duration * 1e6);
      char *ssq = Options.current_strum_sequence->sequence, cseq = *ssq;
      chord *achord;
      int    i;

      while (cseq){
	switch (cseq){
	  //	    case 'E':
	  //	      strum_voices2(strings_used, strings_col, piano_roll);
	  //	      usleep(msec*0.5);
	  //	      break;
	case 'u':
	  //case 'U':
	  strum_voices2(strings_used, strings_col, PIANO_UP, strings_picked);
	  //strum_voices(gnos, PIANO_UP);
	  usleep(msec*0.5);
	  break;
	case 'd':
	  //	    case 'D':
	  strum_voices2(strings_used, strings_col, PIANO_DOWN, strings_picked);
	  //	      strum_voices(chord, PIANO_DOWN);
	  usleep(msec*0.5);
	  break;
	case 'p':
	  //	    case 'D':
	  strum_voices2(strings_used, strings_col, PIANO_PICK, strings_picked);
	  //	      strum_voices(chord, PIANO_DOWN);
	  usleep(msec*0.5);
	  break;
	default:
	  break;
	}
	if (isdigit(cseq)) usleep(msec*(cseq-'0'));
	// pop/play stack chord
	else if (cseq=='P'){
	  cseq =*++ssq;                         // read next char
	  chord *achord=chord_keybound(cseq);
	  if (achord) chord_push(achord);
	}
	else if (cseq=='M'){    
	  achord=chord_pop();
	  if (achord)
	    for (i=0;i<NUM_GSTRINGS;i++){
	      open_strings[i] = achord->open_strings[i];
	      strings_used[i] = achord->strings_used[i];
	      strings_col[i] = achord->strings_col[i];
	    }
	}
	else if (achord=chord_keybound(cseq)){
	  //    printf("keybinding %c found. chord\n", ch);
	  for (i=0;i<NUM_GSTRINGS;i++){
	    open_strings[i] = achord->open_strings[i];
	    strings_used[i] = achord->strings_used[i];
	    strings_col[i] = achord->strings_col[i];
	  }
	}

	cseq =*++ssq;
      }
    }
  }
}

post *
setchord(int fret_eh, int fret_b, int fret_g, int fret_d, int fret_a, int fret_e)
{
  int 
    frets[6] = {fret_eh, fret_b, fret_g, fret_d, fret_a, fret_e},
    strings[6] = {string_EH, string_B, string_G, string_D, string_A, string_E },
      fret, string,
	i;
  int a_rest = false;
  post  *gnos = NULL, *asong = NULL, *song = NULL;

  for (i=0;i<6;i++){
    string = strings[i];
    fret = frets[i];
    asong = post_new(7-string, fret, 0, 1.0, a_rest, TABLATURE);
    if (i==0) gnos = song;
    else song->next = asong;
    song = asong;
  }
  return gnos;
}

void output_voices_openD(post *gnos, int verbose)
{
  output_voices_openD_and_midi(gnos, verbose, false);
}

void output_voices_midi(post *gnos, int verbose)
{
  output_voices_openD_and_midi(gnos, verbose, true);
}

void output_voices_openD_and_midi(post *gnos, int verbose, bool is_midi)
{
  int    i;
  post     *apost = NULL;
  int beats_per_minute = Options.bpm, start, end;
  double beat_duration = 60./(double)beats_per_minute;
  int strings[NUM_GSTRINGS]={OPEND_1_STRING, OPEND_2_STRING, OPEND_3_STRING, 
			     OPEND_4_STRING, OPEND_5_STRING, OPEND_6_STRING};
  int to_seq_client = Options.alsa_server_addr;  // sequencer client no. (fluid synth)
  int to_seq_port = Options.alsa_server_port;    // sequencer port no.
  unsigned int caps;
  int vel  = 127;        // voice volume
  int success = 0;
  int msec = (int)floor(beat_duration * 1e6);
  int current_time;
  int max_time;
  float duration = 1.0;
  //  int foo = 0;

  if (is_midi){

    if (verbose) 
      printf("MIDI number tuning %d,%d,%d,%d,%d,%d\n",
	   Options.strings_tuning_midi[0],
	   Options.strings_tuning_midi[1],
	   Options.strings_tuning_midi[2],
	   Options.strings_tuning_midi[3],
	   Options.strings_tuning_midi[4],
	   Options.strings_tuning_midi[5]);

    for (i=0;i<NUM_GSTRINGS;i++)
      strings[i]=Options.strings_tuning_midi[i];
  }
  else {
    if (verbose)
      printf("strings tuning %d,%d,%d,%d,%d,%d\n",
	   strings[0],
	   strings[1],
	   strings[2],
	   strings[3],
	   strings[4],
	   strings[5]);
  }

  // Open an Output voice (or Input listener) Sequence, 
  // Get assigned id, and set name. 
  snd_seq_open(&seq_handle, "hw", SND_SEQ_OPEN_OUTPUT, 0);
  my_client = snd_seq_client_id(seq_handle);
  if (verbose) printf(_("sequence client id %d\n"), my_client);
  snd_seq_set_client_name(seq_handle, client_name);

  // Create the output port
  //  snd_seq_set_client_group(seq_handle, "input");
  //  caps = SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_WRITE;
  caps = SND_SEQ_PORT_CAP_WRITE;  
  //  if (to_seq_client == SND_SEQ_ADDRESS_SUBSCRIBERS)
  //    caps |= SND_SEQ_PORT_CAP_SUBS_READ;
  my_port = snd_seq_create_simple_port(seq_handle, client_name, caps,
				       SND_SEQ_PORT_TYPE_MIDI_GENERIC |
				       SND_SEQ_PORT_TYPE_APPLICATION);
  if (verbose) printf(_("from addr: %d:%d\n"), my_client, my_port);
  if (verbose) printf(_("to addr %d:%d\n"), to_seq_client, to_seq_port);

  // connect to to_seq
  success = snd_seq_connect_to(seq_handle, my_port, to_seq_client, to_seq_port);
  if (verbose) printf(_("success %d\n"), success);

  // event processing
  snd_seq_ev_set_dest(&event, to_seq_client, to_seq_port);
  snd_seq_ev_set_direct(&event);
  snd_seq_ev_set_source(&event, my_port);
  //  snd_seq_ev_set_dest(&event, to_seq_client, to_seq_port);

  /* or  snd_seq_ev_set_subs(&event);        */
  /* terribly innefficient */
  max_time = 0;
  apost = gnos;  
  while (apost){
    if (apost->unused){ apost = apost->next; continue; }
    max_time = MAX(max_time, apost->col);
    apost = apost->next;
  }
  if (Options.do_clip == true){
    start = Options.clip_start;
    end = Options.clip_end;
  } else {
    start = 0;
    end = max_time;
  }
  for (current_time = start; current_time <= end; current_time++){

    // noteon
    apost = gnos;  
    while (apost){
      //      printf("peek time %d string %d fret %d\n", apost->col, apost->row, apost->fret[ONEPLACE]);    
      if (apost->unused){ apost = apost->next; continue; }
      if (apost->col != current_time){ apost = apost->next; continue; }
      // play current_time posts
      if (verbose) printf(_("play time %d (%f) string %d fret %d\n"), apost->col, apost->duration, apost->row, apost->entry); // apost->fret[ONEPLACE]);    
      //      snd_seq_ev_set_noteon(&event, 0, strings[apost->row-1]+apost->fret[ONEPLACE], vel);

      //      snd_seq_ev_set_pitchbend(&event,0, 8000);
      // OCTOTHORPE is a rest
      if (apost->entry != OCTOTHORPE)
	snd_seq_ev_set_noteon(&event, 0, strings[apost->row-1]+apost->entry, vel);
      //      snd_seq_ev_set_noteon(&event, 0, strings[apost->row-1], vel);
      duration = apost->duration;
      snd_seq_event_output(seq_handle, &event);
      snd_seq_drain_output(seq_handle);
#if 1
      int pit;
     for (pit=0;pit<8191;pit++){
      snd_seq_ev_set_pitchbend(&event, 0, pit);
      snd_seq_event_output(seq_handle, &event);
      snd_seq_drain_output(seq_handle);
      usleep(msec*0.000001);
      }
#endif

      apost = apost->next;
    }
    //    if (current_time == 2) duration = 1.5;
    //    else duration = 1.0;
    usleep(msec*duration);
    //    printf("noteoff\n");
    // noteoff
    apost = gnos;  
    while (apost){
      if (apost->unused){ apost = apost->next; continue; }
      if (apost->col != current_time){ apost = apost->next; continue; }
      // play current_time posts
      //      printf("time %d string %d fret %d\n", apost->col, apost->row, apost->fret[ONEPLACE]);    
      //      snd_seq_ev_set_noteoff(&event, 0, strings[apost->row-1]+apost->fret[ONEPLACE], 0);
      //snd_seq_ev_set_pitchbend(&event,0, 8191);
      snd_seq_ev_set_noteoff(&event, 0, strings[apost->row-1]+apost->entry, 0);      
      snd_seq_event_output(seq_handle, &event);
      snd_seq_drain_output(seq_handle);
      
      apost = apost->next;
    }
  }
}

void output_voices_alltuning(post *gnos, int verbose)
{
  if (Options.tuning == TUNING_OPEND)
    output_voices_openD(gnos, verbose);
  else if (Options.tuning == TUNING_EADGBE)
    output_voices(gnos, verbose);
  else if (Options.tuning == TUNING_MIDI)
    output_voices_midi(gnos, verbose);
  else {
    printf("tuning not defined\n");
  }
}

/* MIDI to/from frequency conversion 
   used for defining scales for strings 
*/
double midi2freq(int m)
{
  return ( 440.0*pow((m-69)/12.0, 2) );
}

int freq2midi(double f)
{
  return ( 69.0 + 12 * log2(f/440.) );
}

