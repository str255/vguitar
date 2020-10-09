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

  vguitr2.h file
*/
#ifndef __VGUITR2__
#define __VGUITR2__

#define SIXSTRING  6
#define TWENTYONEFRET 20
#define NUM_GSTRINGS 6   // six string guitar 

#ifdef __VGUITAR_GLOBAL__
int open_strings[NUM_GSTRINGS]={1,1,1,1,1,1};
int strings_used[NUM_GSTRINGS]={0, 0, 0, 0, 0, 0};
int strings_col[NUM_GSTRINGS]={0, 0, 0, 0, 0, 0};
int strings_picked[NUM_GSTRINGS]={1, 0, 0, 2, 3, 0};
#else
extern int open_strings[];
extern int strings_used[];
extern int strings_col[];
extern int strings_picked[];
#endif

typedef enum post_type {
  TABLATURE,
  BOXPING,
} post_type;

class finger_press {
 public:
  int row;
  int col;
};

class vguitar {
 public:

  void init();
  void drawing();
  void draw_neck();
  void draw_plain_neck();
  void draw_box_neck();
  void draw_strings();
  void draw_bridge(int strings_picked[NUM_GSTRINGS]);
  void draw_basetime();
  void draw_remembered_chords();
  void draw_clear_chords();
  void draw_current_mode();
  void draw_clip();
  void draw_clear_clip();
  void update_cursor();
  void update_refresh();
  void update_box();
  void update_tablature();
  void update_strum2();
  int key_tablature(finger_press *kp, int *play, int *running);
  int key_strum(finger_press *kp, int *play, int *piano_roll);
  int key_strum2(finger_press *kp, int *play, int *piano_roll, int *running);
  int key_box(finger_press *kp, int *play, int *piano_roll, int *running);
  void add_to_song(int time, int string, int fret);
  void cycle_runningmodes(int sense);
  int basetime;
// neck
  int number_strings;    
  int number_vtimes;
  int string_size;
  int vtime_size;
  int MAX_ROWS; 
  int MAX_COLS;
  int nut;
  int bridge;

  void zero_boxlet();
  short compute_shim(int string, int fret);
 private:
  short boxlet[SIXSTRING][TWENTYONEFRET];
};

/* Sparse */
class post {
public:
  post_type  boxed;   // entry boxed or freted
  int unused;
  //  int fret[2];
  int rest;        // musical rest/pause
  int bend;        // is this note bended
  // int ifret;
  int entry;
  int row;
  double duration;   // percent quarter notes
  struct post *next;
  int gettime();
  post *find_post(finger_press *press);
public:
  int col;
};

typedef enum piano_roll_type {
  PIANO_UP,
  PIANO_DOWN,
  PIANO_PICK,
} piano_roll_type;


typedef struct sstrum {
  char *sequence;
  struct sstrum *prev;
  struct sstrum *next;
} sstrum;

sstrum *sstrum_new(char *seq);

typedef struct chord {
  unsigned char key_binding;
  int open_strings[NUM_GSTRINGS];
  int strings_used[NUM_GSTRINGS];
  int strings_col[NUM_GSTRINGS];
  struct chord *next;
  struct chord *stack;
} chord;

chord *chord_new(unsigned char key_binding);
void   chord_dump();
chord *chord_remember(chord *achord);
chord *chord_keybound(char key);
chord *chord_pop();
chord *chord_push(chord *achord);
chord *chord_set();   // create a chord, from strings

void output_voices_alltuning(post *gnos, int verbose);
void output_voices(post *gnos, int verbose);
void output_voices_openD(post *gnos, int verbose);
void output_voices_midi(post *gnos, int verbose);
void output_voices_openD_and_midi(post *gnos, int verbose, bool is_midi);
void strum_voices(post *gnos,  int piano_roll);
void strum_voices2(int strings_used[NUM_GSTRINGS], int strings_col[NUM_GSTRINGS], int piano_roll,
		   int strings_picked[NUM_GSTRINGS]);
void play_strum2(int piano_roll);

post *post_new(int row, int col, int fret, float duration, int a_rest, post_type boxed);
post *post_fret(int row, int col);
post *post_bend(post *apost);

void post_add_digit(post *p, int digit);
void post_zero_digits(post *p);
void move_times(int col, int increment);
int calc_max_time();
post *setchord(int fret_eh, int fret_b, int fret_g, int fret_d, int fret_a, int fret_e);
post *song_addpost_at_index(post *song, post *apost, int sequence_index);

#define MAX(a,b) ((a)>(b)?(a):(b))

#define OCTOTHORPE 99
#define ONEPLACE 1
#define TENPLACE 0
#define DEFAULT_BPM 120
#define DEFAULT_ROLL 0.1
/* note high to low */
typedef enum guitar_strings {
  neck_high,
  string_EH,
  string_B,
  string_G,
  string_D,
  string_A,
  string_E,
  neck_low,
} guitar_strings;

typedef enum guitar_open_strings {
  OPEN_E,
  OPEN_A,
  OPEN_D,
  OPEN_G,
  OPEN_B,
  OPEN_EH,
} guitar_open_strings;

/* MIDI offsets to middle-C 
standard tuning -- E A D G B E
open  D tuning  -- D A D F♯ A D
*/
#define C_NOTE 60
#define C_STRING (C_NOTE)
#define E_STRING (C_NOTE-20)
#define F_STRING (E_STRING+1)
#define A_STRING (C_NOTE-15)
#define D_STRING (C_NOTE-10)
#define G_STRING (C_NOTE-5)
#define B_STRING (C_NOTE-1)
#define EHI_STRING (C_NOTE+4)

/* Relative tuning. 
   These are MIDI values 
   for tuning the sixth string */
#define MIDI_E4 64//E4
#define MIDI_C4 60 //C4
#define MIDI_B3 59 //B3
#define MIDI_G3 55 //G3
#define MIDI_D3 50 //D3
#define MIDI_A2 45 //A2
#define MIDI_F2 41 //F2
#define MIDI_E2 40 //E2

#define EHI_TUNE 64//E4
#define C_TUNE 60 //C4
#define F_TUNE 41 //F2
#define B_TUNE 59 //B3
#define G_TUNE 55 //G3
#define D_TUNE 50 //D3
#define A_TUNE 45 //A2
#define E_TUNE 40 //E2


#define C_NOTE 60
#define OPEND_6_STRING (EHI_STRING-2)
#define OPEND_5_STRING (B_STRING-2)
#define OPEND_4_STRING (G_STRING-1)
#define OPEND_3_STRING (C_NOTE-10)
#define OPEND_2_STRING (C_NOTE-15)
#define OPEND_1_STRING (E_STRING-2)

double midi2freq(int m);
int freq2midi(double f);

/* Relative tuning. 
   map from reference note to 
   the actual midi numbering
   for tuning the sixth string 
   e.g. 'E' maps to 40 (E2)
*/
typedef struct midi_table {
  char note;
  int  midi_number;
} midi_table;

#ifdef __VGUITAR_GLOBAL__
midi_table midi_map[7]={
  { 'A', A_TUNE},
  { 'B', B_TUNE},
  { 'C', C_TUNE},
  { 'D', D_TUNE},
  { 'E', E_TUNE},
  { 'F', F_TUNE},
  { 'G', G_TUNE},
};

#else
extern midi_table midi_map[7];
#endif

/* for internationalization */
#define _(x) (x)
#endif
