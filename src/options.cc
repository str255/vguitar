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

  options.c file
*/
#include "config.h"
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include "vguitr2.h"
#include "options.h"

extern options_vguitar Options;

extern post           *song, *gnos;
//extern int                    global_time;

void
parse_options (int argc, char **argv)
{
  int       c;
  int       i, j, tuning;

  while (1)
    {
      static struct option long_options[] =
	{
	  {"help",      no_argument,       0, 'h'},		
	  {"tablature", no_argument,       0, 't'},
	  //{"strum",     no_argument,       0, 's'},
	  {"strum",    no_argument,          0, 'S'},
	  {"box",    no_argument,          0, 'M'},	  	  
	  {"chord",    required_argument, 0, 'C'},
	  {"clip",     no_argument,       0, 'c'},
	  {"multi_digit_fret",  no_argument, 0, '1'},
	  {"openD_tuning",  no_argument, 0, 'D'},
	  {"general_tuning",  required_argument, 0, 'Z'},			  
	  {"dir",       required_argument, 0, 'd'},		
	  {"input",     required_argument, 0, 'i'},		
	  {"output",    required_argument, 0, 'o'},
	  {"allput",    required_argument, 0, 'u'},
	  {"addr",      required_argument, 0, 'A'},
	  {"port",      required_argument, 0, 'P'},
	  {"bpm",       required_argument, 0, 'b'},
	  {"loop",       required_argument, 0, 'l'},	  
	  {"roll",      required_argument, 0, 'r'},
	  {"strum_sequence",    required_argument, 0, 'q'},	  	  
	  {"input_format",  required_argument, 0, 'f'},
	  {"output_format", required_argument, 0, 'e'},
	  {"allput_format", required_argument, 0, '2'},	  
	  {"headless", no_argument, 0, 'g'},	  
	  /*	  {"verbose",   no_argument,       0, 'v'},*/
	  {"version",   no_argument,       0, 'v'},	  
	  {0, 0, 0, 0}
	};
      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long (argc, argv, "12:A:b:cC:Dd:e:f:ghi:l:MP:q:o:tSu:vZ:",
		       long_options, &option_index);
      
      /* Detect the end of the options. */
      if (c == -1)
	break;
      
      switch (c){
      case 'v':
	//	puts ("option -v\n");
	//	Options.verbose = true;
	printf("%s %s\n", argv[0], VERSION);
	printf("Copyright (C) 2017,2018,2020 Nicholas C. Strauss\n"
	"This program is free software: you can redistribute it and/or modify\n"
	"it under the terms of the GNU General Public License as published by\n"
	"the Free Software Foundation, either version 3 of the License, or\n"
	"(at your option) any later version.\n\n"

        "This program is distributed in the hope that it will be useful,\n"
	"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	"GNU General Public License for more details.\n\n"

	"You should have received a copy of the GNU General Public License\n"
	"along with this program.  If not, see <https://www.gnu.org/licenses/>.\n"
	"Contact for help or donations: strauss@positive-internet.com\n"
	"Donations are much appreciated. Thank you for your support.\n");
	exit(0);
	break;

	// tablature
      case 't':
	//	printf (_("tablature mode `%s'\n"), optarg);
	printf (_("tablature mode\n"));
	Options.runningMode = do_tablature;
	break;
	// strum
      case 's':
	printf (_("strumming mode not supported. Use -S instead.\n"));
	//	Options.runningMode = do_strum;
	break;
	// clip mode
      case 'S':
	printf (_("strumming mode 2\n"));
	Options.runningMode = do_strum2;
	break;
      case 'M':
	printf (_("Box mode\n"));
	Options.runningMode = do_box;
	break;
      case 'q':
	{
	printf("strum sequence %s\n", optarg);
	sstrum *astrum = sstrum_new(optarg);
	if (Options.strum_sequence){
	  astrum->next = Options.strum_sequence;
	  Options.strum_sequence->prev = astrum;
	}
	Options.current_strum_sequence = astrum;
	Options.strum_sequence = astrum;
	// (char*)malloc(255*sizeof(char));
	// strncpy(Options.strum_sequence, optarg, 255);
	break;
	// chords
	}
      case 'C':
	{
	printf("chord %s\n", optarg);
        char *chord_line = strdup(optarg);
	chord *achord = chord_new(chord_line[0]);
	printf("parse cord |%s|\n", chord_line);
	parse_chord(chord_line, achord);
	if (!Options.chords) Options.chords = achord;
	else {
	  achord->next = Options.chords;
	  Options.chords = achord;
	}

	/*
	while (token){
	  printf("token %s %c\n", token, token[0]);
	  printf("token |%s| |%c|\n", token, achord->key_binding);

	  achord->next = Options.chords;
	  Options.chords = achord;
	}
	*/
	}
	break;
	// single digit frets
      case '1':
	printf (_("multi_digit_fret\n"));
	Options.do_single_digit_fret = false;
	break;

      case '2':	
	printf (_("allput file format -f with value `%s'\n"), optarg);
	if (strcmp(optarg,"classicTrax")==0)
	  Options.output_format = (Options.input_format = format_classicTrax);
	else if (strcmp(optarg,"gp5")==0)
	  Options.output_format = (Options.input_format = format_gp5);
	else
	  Options.output_format = (Options.input_format = format_vguitar);
	break;

	// clip mode
      case 'c':
	printf (_("clip mode\n"));
	Options.do_clip = true;
	break;

	// alsa server port
      case 'P':
	printf (_("ALSA server port `%s'\n"), optarg);
	Options.alsa_server_port = (int)floor(atof(optarg));
	break;

	// alsa server addr
      case 'A':
	{
	  char  *port_str;
	  printf (_("Alsa client addr `%s'\n"), optarg);
	  if ((port_str=strchr(optarg,':'))){
	    int    addr, port;
	    *port_str++ = 0;
	    port = (int)floor(atof(port_str));
	    addr = (int)floor(atof(optarg));
	    printf("addr %d port %d\n", addr, port);
	    Options.alsa_server_addr = addr;
	    Options.alsa_server_port = port;
	    break;
	  }
	  Options.alsa_server_addr = (int)floor(atof(optarg));
	  break;
	}

	// beats per minute (bpm)
      case 'b':
	printf (_("beats per minute with value `%s'\n"), optarg);
	Options.bpm = (int)floor(atof(optarg));
	break;

	// loop
      case 'l':
	printf (_("loop (number of repetitions) `%s'\n"), optarg);
	Options.loop = (int)floor(atof(optarg));
	break;

	// roll factor 0.1 
      case 'r':
	printf (_("roll factor `%s'\n"), optarg);
	Options.roll_factor = atof(optarg);
	break;
	// input file
      case 'i':
	{
	  printf (_("input file -i with value `%s'\n"), optarg);
	  Options.has_input = true;
	  //	  strcpy(Options.input_file, optarg);
	  file_seq *fsn = file_seq_new(optarg);
	  if (Options.input_file){
	    fsn->next = Options.input_file;
	    Options.input_file->prev = fsn;
	  }
	  Options.input_file = fsn;
	//	read_input_file(Options.input_file, Options.input_format);
	}
	break;
      case 'D':             // alternate openD tuning
	{
	  printf (_("openD tuning\n"));
	  Options.tuning = TUNING_OPEND;
	}
	break;
      case 'd':
	{
	  printf (_("input directory -d with value `%s'\n"), optarg);
	  Options.has_dir = true;
	  strcpy(Options.dirpath, optarg);
	}
	break;
      case 'e':	
	printf (_("output file format -e with value `%s'\n"), optarg);
	if (strcmp(optarg,"classicTrax")==0)
	  Options.output_format = format_classicTrax;
	else if (strcmp(optarg,"gp5")==0)
	  Options.output_format = format_gp5;
	else
	  Options.output_format = format_vguitar;	  
	break;
      case 'f':	
	printf (_("input file format -f with value `%s'\n"), optarg);
	if (strcmp(optarg,"classicTrax")==0)
	  Options.input_format = format_classicTrax;
	else if (strcmp(optarg,"gp5")==0)
	  Options.input_format = format_gp5;
	else
	  Options.input_format = format_vguitar;	  
	//	Options.has_input = true;
	//	strcpy(Options.input_file, optarg);
	break;
	// output file
      case 'o':
	printf (_("output file -o with value `%s'\n"), optarg);
	Options.has_output = true;
	strcpy(Options.output_file, optarg);
	break;
      case 'u':
	{
	// allput is both input and output
	printf (_("allput (input/output) files -o with value `%s'\n"), optarg);
	Options.is_allput = true;
	// output
	Options.has_output = true;
	strcpy(Options.output_file, optarg);
	// input
	Options.has_input = true;
	file_seq *fsn = file_seq_new(optarg);
	if (Options.input_file){
	  fsn->next = Options.input_file;
	  Options.input_file->prev = fsn;
	}
	Options.input_file = fsn;
      }
	break;
      case '?':
      case 'h':
	print_options();
	exit(0);
	break;
      case 'g':
	printf (_("headless\n"));
	Options.headless = true;
	break;
      case 'Z':     // MIDI tuning
	{
	  // int strings[NUM_GSTRINGS]={E_STRING, A_STRING, D_STRING, G_STRING, B_STRING, EHI_STRING};
	  printf (_("general tuning\n"));
	  Options.tuning = TUNING_MIDI;
	  // six string array of midi numbers *OR* array of frequencies
	  char *tuning_strings = strdup(optarg);
	  if (tuning_strings[0]=='M'){
	    sscanf(tuning_strings,"M%d,%d,%d,%d,%d,%d", 
		   &Options.strings_tuning_midi[0],
		   &Options.strings_tuning_midi[1],
		   &Options.strings_tuning_midi[2],
		   &Options.strings_tuning_midi[3],
		   &Options.strings_tuning_midi[4],
		   &Options.strings_tuning_midi[5]);
	    printf("midi number tuning %d,%d,%d,%d,%d,%d\n", 
		   Options.strings_tuning_midi[0],
		   Options.strings_tuning_midi[1],
		   Options.strings_tuning_midi[2],
		   Options.strings_tuning_midi[3],
		   Options.strings_tuning_midi[4],
		   Options.strings_tuning_midi[5]);
	  }
	  else if (tuning_strings[0]=='A'||tuning_strings[0]=='B'||tuning_strings[0]=='C'||
		   tuning_strings[0]=='D'||tuning_strings[0]=='E'||tuning_strings[0]=='F'||
		   tuning_strings[0]=='G'){
	    // E55545
	    int       i, j, tuning;
	    char      string_one;    // in standard tuning this is the low E.
	    int       relative[5];
	    sharp_flat_string  sharp_flat_true; // 0==none, 1=flat, 2=sharp.

	    string_one = tuning_strings[0];
	    if (tuning_strings[1]=='b') sharp_flat_true = STRING_FLAT;
	    else if (tuning_strings[1]=='#') sharp_flat_true = STRING_SHARP;
	    else sharp_flat_true = STRING_TRUE;
	    if (sharp_flat_true == STRING_TRUE){
	      relative[0] = tuning_strings[1]-'0';
	      relative[1] = tuning_strings[2]-'0';
	      relative[2] = tuning_strings[3]-'0';
	      relative[3] = tuning_strings[4]-'0';
	      relative[4] = tuning_strings[5]-'0';
	    }
	    else {
	      relative[0] = tuning_strings[2]-'0';
	      relative[1] = tuning_strings[3]-'0';
	      relative[2] = tuning_strings[4]-'0';
	      relative[3] = tuning_strings[5]-'0';
	      relative[4] = tuning_strings[6]-'0';
	    }

	    fprintf(stderr,"tuning |%s| recognized.\n",
		   tuning_strings);
	    for (i=0;i<7;i++){
	      fprintf(stderr,"midi_map[%d] %c (%d)\n", i, midi_map[i].note, midi_map[i].midi_number);
	      if (string_one == midi_map[i].note) j = i;
	    }

	    if (sharp_flat_true==STRING_TRUE){
	      fprintf(stderr,"relative tunings %c %d,%d,%d,%d,%d|\n", 
		     string_one, relative[0], relative[1], relative[2], relative[3], relative[4]);
	      fprintf(stderr,"tuning string six to absolute note %c (%d)\n", midi_map[j].note, midi_map[j].midi_number);
	      Options.strings_tuning_midi[0] = midi_map[j].midi_number;
	    }
	    else if (sharp_flat_true==STRING_FLAT){
	      fprintf(stderr,"relative tunings %cb %d,%d,%d,%d,%d|\n", 
		     string_one, relative[0], relative[1], relative[2], relative[3], relative[4]);
	      fprintf(stderr,"tuning string six to absolute note %cb (%d)\n", midi_map[j].note, midi_map[j].midi_number-1);
	      Options.strings_tuning_midi[0] = midi_map[j].midi_number-1;
	    }
	    else if (sharp_flat_true==STRING_SHARP){
	      fprintf(stderr,"relative tunings %c# %d,%d,%d,%d,%d|\n", 
		     string_one, relative[0], relative[1], relative[2], relative[3], relative[4]);
	      fprintf(stderr,"tuning string six to absolute note %c# (%d)\n", midi_map[j].note, midi_map[j].midi_number+1);
	      Options.strings_tuning_midi[0] = midi_map[j].midi_number+1;
	    }

	    for (i=1;i<6;i++)
	      Options.strings_tuning_midi[i] = Options.strings_tuning_midi[i-1] + relative[i-1];

	    printf("midi number tuning %d,%d,%d,%d,%d,%d\n", 
		   Options.strings_tuning_midi[0],
		   Options.strings_tuning_midi[1],
		   Options.strings_tuning_midi[2],
		   Options.strings_tuning_midi[3],
		   Options.strings_tuning_midi[4],
		   Options.strings_tuning_midi[5]);

	  }
	  else fprintf(stderr,"tuning |%s| not recognized.\nRecognized are MIDI numbers, and relative tunings.\n",
		      tuning_strings);
	}
	//	exit(0);
	break;
      default:
	//	exit(0);
	break;
      }
    }

  /* Print any remaining command line arguments (not options). */
  if (optind < argc) {
    printf ("non-option ARGV-elements: ");
    while (optind < argc)
      printf ("%s ", argv[optind++]);
    putchar ('\n');
  }
  return;
}

void
default_options()
{
  Options.headless = false;
  Options.runningMode = do_tablature;
  Options.tuning = TUNING_EADGBE;
  Options.do_clip = true;
  Options.do_single_digit_fret = true;  
  Options.is_allput = false;
  Options.has_input = false;
  Options.has_output = false;
  Options.alsa_server_port = 0;
  Options.alsa_server_addr = 129;  
  Options.bpm = DEFAULT_BPM;
  Options.loop = 1;
  Options.roll_factor = DEFAULT_ROLL;
  Options.clip_start = 0;
  Options.clip_end = 10;
  Options.chords = NULL;
  Options.chord_stack = NULL;
  Options.strum_sequence = NULL;
  Options.current_strum_sequence = NULL;  
  //  strcpy(Options.input_file, "/usr/share/vguitar/share/virtual_guitar_input.txt");
  Options.has_dir = false;
  strcpy(Options.dirpath,"~/");
  Options.input_file = NULL;
  strcpy(Options.output_file, "virtual_guitar_output.txt");
  //  Options.input_format = format_vguitar;
  //  Options.output_format = format_vguitar;
  Options.input_format = format_classicTrax; // Fri Sep 27 23:07:30 CDT 2019
  Options.output_format = format_classicTrax;
}

void
print_options()
{
  printf(_("Virtual Guitar - Play guitar in any term window.\n"));
  printf(_("Usage: vguitar -A port [--allput=infile.trax -allput_format=classicTrax]\n"));
  printf(_("Play  guitar  in a term window. Use with a MIDI synthesizer (qsynth) by specifying the ALSA synthesizer address and port. These can be listed using aconnect -l. Tablature, Box, and Strum modes are supported. Alternative  tunings  of  OpenD,  MIDI, and relative are supported. File format is classicTrax which is easily tablature human readable.\n"));
  printf("\n\n");
  printf(_("*OPTIONS*\n"));
  printf(_(" -[t|S|M]                  tablature, strum mode 2, or box mode\n"));
  printf(_(" -c --clip                   use clip interval (default).\n"));
  printf(_(" -d --dir                    <directory>\n"));
  printf(_(" -A --addr                   (eg 128:0) -P --port\n"));
  printf(_(" -u --allput[=FILE]\n"));  
  printf(_(" -2 --allput_format[=classicTrax]\n"));    
  printf(_(" -D --openD_tuning           alternate tuning\n"));
  printf(_(" -Z --general_tuning         alternate relative or MIDI tunings\n"));  
  printf(_(" -i --input[=FILE]\n"));
  printf(_(" -o --output[=FILE]\n"));
  printf(_(" -b --bpm                    beats per minute\n"));
  printf(_(" -l --loop                   number of repetitions\n")); 
  printf(_(" -r --roll                   roll factor\n"));
  printf(_(" -C --chord[=CHORD]\n"));
  printf(_(" -q --strum_sequence=[strum_sequence]\n"));
  printf(_(" -1 --multi_digit_fret      default false\n"));
  printf(_(" -e --output_format[=classicTrax]\n"));  
  printf(_(" -f --input_format[=classicTrax]\n"));
  printf(_(" -g --headless              run without term\n")); 

  printf("\n");
  printf(_("*Tablature Keyboard*\n"));
  printf(_(" space - play selected clip\n"));
  printf(_(" Key # - a rest (or pause) note\n"));
  printf(_(" Keys[0...9] - set fret number\n"));
  printf(_(" Keys '<' and '>' move the time left / right\n"));
  printf(_(" Keys '[' and ']' set the clip interval\n"));
  printf(_(" Key 'c' play selected clip\n"));
  printf(_(" Keys [j|l] [n|p] - move cursor\n"));
  printf(_(" Keys [eadgbh] - move cursor to string (h is hi-E)\n"));
  printf(_(" Key 'x' delete the selected note\n"));  
  printf(_(" Keys [.|-] shorten/lengthen the note\n"));  
  printf(_(" HOME / END - cycle running modes TAB/STM/BOX\n"));
  printf("\n");
  printf(_("*Strum Keyboard*\n"));  
  printf(_(" space - strum\n"));
  printf(_(" m - mute on/off\n"));
  printf(_(" u - roll up/down\n"));  
  printf(_(" k - mark key for chord\n"));
  printf(_(" o - open string play/muted\n"));
  printf(_(" q - quit\n"));
  printf(_(" M - pop chord from stack, set fret board\n"));
  printf(_(" P - push chord onto stack\n"));      
  printf(_(" R - remember chord\n"));      
  printf(_(" HOME / END - cycle running modes TAB/STM/BOX\n"));
  printf(_(" END  - switch to strum2 mode\n"));
  printf(_(" PAGE UP/DOWN - select next/previous strum sequence\n"));  
  printf(_(" Keys [j|l] [n|p] - move cursor\n"));
  printf(_(" Arrow Keys - move cursor\n"));
  printf(_(" Keys [.|-] shorten/lengthen the roll\n"));
  printf("\n");
  printf(_("Examples:\n"));
  printf(_("vguitar -A 128:0 --allput auld-lang-syne.txt\n\n"));  
  printf(_("vguitar -A 128:0 --allput_format classicTrax --allput open1.trax -Z\"E55545\"   (relative tuning from E)\n\n"));
  printf(_("vguitar -A 128:0 --allput_format classicTrax --input bwv996-a.trax --input bwv996-b.trax --output=\"combined.trax\"\n\n"));
  printf(_("vguitar -A 128:0 -S --roll=0.1    (strum mode)\n\n"));  
  printf(_("Example of interactive strum mode, two strum sequences, and three chords and a stack pop (M):\n\n"));
  printf(_("vguitar -A 129:0 -S --chord=\"Aexhx\" --chord=\"Eexaxdxgxbxh1\" --chord=\"Dexaxdxg3b2h3\" -q \"dd1Muuuu\" -q \"dudu\" \n\n"));
  printf(_("Example of interactive strum mode, two strum sequences, and four chords at 200 beats per minute\n\n"));
  printf(_("vguitar -A 128:0 -S --chord=\"Aexd2g2b2\" --chord=\"Fe2a4d4g2b2h2\" --chord=\"Dexaxg2b3h2\" --chord=\"Ea2d2g1\" -q \"d2du2udu\" -q \"d2du2ududd\" --bpm=200\n\n"));
  printf(_("Example of strum mode,  two chords (A and D) in one strum sequences, at 200 beats per minute:\n\n"));
  printf(_("vguitar -A 128:0 -S --chord=\"Aexd2g2b2\" --chord=\"Dexaxg2b3h2\" -q \"Ad2Ddu2udu\" --bpm=200 --headless\n\n"));  
      /*
  printf(_("Options.headless %d\n"), Options.headless);
  printf(_("Options.runningMode %d\n"), Options.runningMode);
  printf(_("Options.do_clip %d\n"), Options.do_clip);
  printf(_("Options.alsa_server_port %d\n"), Options.alsa_server_port);
  printf(_("Options.alsa_server_addr %d\n"), Options.alsa_server_addr);
  printf(_("Options.bpm %d\n"), Options.bpm);
  printf(_("Options.clip_start %d clip_end %d\n"), Options.clip_start, Options.clip_end);
  printf(_("Input (%d) %s\n"), Options.has_input, Options.input_file);
  printf(_("Output (%d) %s\n"), Options.has_output, Options.output_file);*/
  /*  chord *acord = Options.chords;
  while (acord){
    printf("key %ch\n", acord->key_binding);
    acord = acord->next;
    }*/
}

void
read_input_file(char *input_filename, datafile_format aformat)
{
  char             buffer[132];
  int              recordid=0;
  float            duration;
  int              string = 0, fret = 0, time = -1, velocity = 0;
  int              max_time = 0;
  int              a_rest = false;
  static           int global_time = 0;
  char             read_create[5] = "r";
  FILE *fp = NULL;

  // Under allput, if the file isnt found, 
  // create it. Only then, if it can't be opened punt.
  if (Options.is_allput) strcpy(read_create, "a+");
  else strcpy(read_create, "r");

  //  if (!Options.has_input) return;
  if (Options.has_dir){
    char    *path = (char*)malloc(1024);
    strcpy(path, Options.dirpath);
    int pathlen = strlen(path);
    if (path[pathlen-1]!='/') strcat(path,"/");
    strcat(path, input_filename);
    fp = fopen(path, read_create);
    printf("path %s\n", path);
  }
  else fp = fopen(input_filename, read_create);
  if (!fp){
    printf(_("virtual guitar: can't open input file %s\n"), input_filename);
    return;
  }
  if (Options.input_format == format_vguitar){
    while (fgets(buffer, 80, fp)){
      if (buffer[0]=='#') continue;
      sscanf(buffer, "%d %f %d %d %d\n", &time, &duration, &string, &fret, &velocity);
      /* filter out noise */
      if (time > 100000 || time < -100000) continue;
      time += global_time;
      max_time = MAX(time, max_time);
      printf(_("record %d: time=%d duration=%f string=%d fret=%d velocity=%d\n"),
	     recordid,
	     time, duration, string, fret, velocity);
      if (song){
	song->next = post_new(string, time, fret, duration, a_rest, TABLATURE);
	song = song->next;
      }
      else gnos = song = post_new(string, time, fret, duration, a_rest, TABLATURE);
    
      recordid++;
    }
    global_time = MAX(global_time, max_time);
  }
  else if (Options.input_format == format_classicTrax) read_input_file_classicTrax(fp);
  else if (Options.input_format == format_gp5) read_input_file_gp5(fp);

  Options.clip_end = global_time;
  if (fp) fclose(fp);
}

// #define CHECK_THREE(a, b, c)  ((a)==buffer[0]&&(b)==buffer[1]&&(c)==buffer[2])

void
read_input_file_classicTrax(FILE *fp)
{
  char  buffer[132];
  int   ch, lookahead, skip, bend;
  static int   measure = 0, global_time = 0;
  int  istring = 0;
  int  highE = false, time_measure, i, string, fret, duration, a_rest;
  float time;

  printf("read_input_file_classicTrax tuning=%d\n", Options.tuning);
  
  /* not supporting time (4/4) rudimentry knowledge of measures. */
  /* read first measure */
  /*  global_time = 0; */
  while (fgets(buffer, 80, fp)){
    fret = -1;
    string = neck_high;
    // standard tuning EADGBE
    // for now MIDI read in as standard tuning
    if (Options.tuning == TUNING_EADGBE ){
      if (buffer[0]=='E' && !highE){ string = string_EH; highE = true; }
      else if (buffer[0]=='B') string = string_B;
      else if (buffer[0]=='G') string = string_G;
      else if (buffer[0]=='D') string = string_D;
      else if (buffer[0]=='A') string = string_A;
      else if (buffer[0]=='E'){ string = string_E; highE = false; }
      else continue; // only parse real strings
    }
    // openD tuning
    else if (Options.tuning == TUNING_OPEND || Options.tuning == TUNING_MIDI){
      if (buffer[0]=='1' && !highE){ string = string_EH; highE = true; }
      else if (buffer[0]=='2') string = string_B;
      else if (buffer[0]=='3') string = string_G;
      else if (buffer[0]=='4') string = string_D;
      else if (buffer[0]=='5') string = string_A;
      else if (buffer[0]=='6'){ string = string_E; highE = false; }
      else continue; // only parse real strings
    }
    time_measure = strlen(buffer);
    for (i=1; i<time_measure && (ch=buffer[i])!=32; i++){
      if (ch != '-' && ch != '|' && ch != 32 
	  && ch != '\n' && ch != '*'){
	// 8x3x3=24x3=72
	// int dashes_per_bar = 8;
	int dashes_per_bar = 72;
	//	time = (i-2)/8.;
	lookahead = buffer[i+1];
	time = (i-3);
	fret = ch-'0';
	if (ch == '#'){ a_rest = true; fret = OCTOTHORPE; }          // octothorpe
	else a_rest = false;
	duration = 1;
	skip = false;
	bend = false;
	// bended frets start at F (cant bend an open string)
	if (ch > 'E'){  
	  bend = true;
	  fret = ch - 'E';
	}
	//	// check for bends
	//	if (lookahead=='^'){
	//	  bend = true;
	//	}
	// check for ten-place frets
	if (isdigit(lookahead)){
	  fret = lookahead-'0';
	  skip = true;
	}

	if (song){
	  song->next = post_new(7-string, measure*dashes_per_bar+time, fret, duration, a_rest, TABLATURE);
	  if (skip) song->next->entry += 10*(ch-'0'); //song->next->fret[TENPLACE]=1;
	  if (bend) song->next->bend = true;
	  song = song->next;
	}
	else {
	  gnos = song = post_new(7-string, measure*dashes_per_bar+time, fret, duration, a_rest, TABLATURE);
	  if (skip) song->entry += 10*(ch-'0'); // song->fret[TENPLACE]=1;
	  if (bend) song->next->bend = true;
	}
      }
      if (skip) i++;
    }
    if (!highE){ 
      measure++;
      global_time += 8;    // assume 8 beats per measure
      //      break;
    }
  }
  // over acol, filter out dead times from song
  post *anote;
  int  iclean, acol, dead_col;
  for (iclean=0;iclean<10;iclean++){
    for (acol=0;acol<10000;acol++){
      dead_col = true;
      anote = gnos;
      while (anote){
	if (anote->col == acol){ dead_col = false; break; }
	anote = anote->next;
      }
      if (dead_col == true){
	move_times(acol, -1);
      }
    }
  }
}

void
read_input_file_gp5(FILE *fp)
{
  printf("format gp5 not currently supported\n");
}

void
write_output_file()
{
  post            *apost;
  float            duration;
  int              string, fret, time, velocity;

  FILE *fp;
  if (!Options.has_output) return;
  if (Options.runningMode != do_tablature){
    printf(_("output only supported in tablature mode\n"));
    return;
  }
  if (Options.has_dir){
    char    *path = (char*)malloc(1024);
    strcpy(path, Options.dirpath);
    int pathlen = strlen(path);
    if (path[pathlen-1]!='/') strcat(path,"/");
    strcat(path, Options.output_file);
    fp = fopen(path, "w");
    printf("path %s\n", path);
  }
  else fp = fopen(Options.output_file, "w");
  if (!fp){
    printf(_("virtual guitar: can't open output file %s\n"), Options.output_file);
    exit(0);
  }

  if (Options.output_format == format_classicTrax){
    write_output_file_classicTrax(fp);
    return;
  }
  else if (Options.output_format == format_gp5){
    printf("write gp5 format not supported\n");
    return;
  }
  apost = gnos;
  while (apost){
    time = apost->col;
    duration = apost->duration;
    string = apost->row;
    fret = apost->entry; // apost->fret[ONEPLACE]+10*apost->fret[TENPLACE];
    velocity = 127;
    fprintf(fp, "%d %f %d %d %d\n", time, duration, string, fret, velocity);
    apost = apost->next;
  }
}


void write_output_file_classicTrax(FILE *fp)
{
  int              max_time = 0, current_time;
  post            *apost;
  float            duration;
  //  int              string, fret, time, velocity;
  //  int strings[NUM_GSTRINGS]={E_STRING, A_STRING, D_STRING, G_STRING, B_STRING, EHI_STRING};
  int              istring, ifret, strings[6];
  char             measure[6][80];
  int              imeasure, imeasure_time, number_measures, measure_size = 30;

  apost = gnos;  
  while (apost){
    if (apost->unused){ apost = apost->next; continue; }
    max_time = MAX(max_time, apost->col);
    apost = apost->next;
  }

  number_measures = max_time/measure_size;
  for (imeasure=0;imeasure<=number_measures;imeasure++){
    for (imeasure_time = 0; imeasure_time < measure_size; imeasure_time++){
      current_time = imeasure*measure_size + imeasure_time;
      for (istring=0;istring<6;istring++) strings[istring] = -1;  // unused
      // noteon
      apost = gnos;  
      /* terribly innefficient */
      while (apost){
	if (apost->unused){ apost = apost->next; continue; }
	if (apost->col != current_time){ apost = apost->next; continue; }
	istring = apost->row-1;
	ifret = apost->entry; // apost->fret[ONEPLACE];
	if (apost->bend) ifret = ifret-'0'+'E';
	if (istring>=0 && istring < 6) strings[istring] = ifret;
	apost = apost->next;
      }
      // output current time
      for (istring=0;istring<6;istring++){
	if (strings[istring] == -1) measure[istring][imeasure_time]='-';
	else if (strings[istring] == OCTOTHORPE) measure[istring][imeasure_time]='#';
	else measure[istring][imeasure_time] = strings[istring]+'0';
      }
    }

    for (istring=5;istring>=0;istring--){
      for (imeasure_time = 0; imeasure_time < measure_size; imeasure_time++){
	if (imeasure_time == 0){
	  switch (istring){
	  case  5:
	    if (Options.tuning==TUNING_EADGBE) fprintf(fp,"E|");
	    else fprintf(fp,"1|");
	    break;
	  case  4:
	    if (Options.tuning==TUNING_EADGBE) fprintf(fp,"B|");
	    else fprintf(fp,"2|");	    
	    break;
	  case  3:
	    if (Options.tuning==TUNING_EADGBE) fprintf(fp,"G|");
	    else fprintf(fp,"3|");	    
	    break;
	  case  2:
	    if (Options.tuning==TUNING_EADGBE) fprintf(fp,"D|");
	    else fprintf(fp,"4|");	    	    
	    break;
	  case  1:
	    if (Options.tuning==TUNING_EADGBE) fprintf(fp,"A|");
	    else fprintf(fp,"5|");	    	    	    
	    break;
	  case  0:
	    if (Options.tuning==TUNING_EADGBE) fprintf(fp,"E|");
	    else fprintf(fp,"6|");	    	    	    	    
	    break;
	  }
	}
	fprintf(fp,"-%c", measure[istring][imeasure_time]);
      }
      fprintf(fp,"\n");
    }
    fprintf(fp,"\n\n");
  }
}

/* chord specified by fret 
   position on six strings */
void
parse_chord(char *arg, chord *achord)
{
  char *carg=arg+1;
  char ch, val;
  int  i, fret;
  int  open;
  int *open_strings = achord->open_strings;
  int *strings_used = achord->strings_used;
  int *strings_col = achord->strings_col;

  for (i=0;i<NUM_GSTRINGS;i++){
    open_strings[i] = true;
    strings_used[i] = false;
    strings_col[i] = -1;
  }
  //  printf("carg |%s|\n", carg);

  while (ch=*carg){
    val = *(carg+1);
    printf("string=%c val=%c\n", ch, val);
    if (!val){ printf("vguitar: error (1) in chord %s\n", arg); return; }
    if (val=='x'||val=='X'){ open = false; }
    else {
      open = true;
      if (!isdigit(val)){ printf("vguitar: error (2) in chord %s val %c\n", arg, val); return; }
      fret = val - '0';
    }
    switch (ch){
    case 'e':
    case 'E':
      open_strings[OPEN_E] = open;
      if (open){
	strings_used[OPEN_E] = 1;
	strings_col[OPEN_E] = fret;
      }
      break;
    case 'a':
    case 'A':
      open_strings[OPEN_A] = open;
      if (open){
	strings_used[OPEN_A] = 1;
	strings_col[OPEN_A] = fret;
      }
      break;
    case 'd':
    case 'D':
      open_strings[OPEN_D] = open;
      if (open){
	strings_used[OPEN_D] = 1;
	strings_col[OPEN_D] = fret;
      }
      break;
    case 'G':
    case 'g':      
      open_strings[OPEN_G] = open;
      if (open){
	strings_used[OPEN_G] = 1;
	strings_col[OPEN_G] = fret;
      }
      break;
    case 'B':
    case 'b':      
      open_strings[OPEN_B] = open;
      if (open){
	strings_used[OPEN_B] = 1;
	strings_col[OPEN_B] = fret;
      }
      break;
    case 'H':
    case 'h':      
      open_strings[OPEN_EH] = open;
      if (open){
	strings_used[OPEN_EH] = 1;
	strings_col[OPEN_EH] = fret;
      }
      break;
    }
    carg += 2;
  }
#if 0
  for (i=0;i<NUM_GSTRINGS;i++){
    printf("strings %d: %d %d %d\n", i,
	   open_strings[i],
	   strings_used[i],
	   strings_col[i]);
  }
  //  exit(0);
#endif
}

file_seq *
file_seq_new(char *seq)
{
  file_seq *s = (file_seq*)malloc(sizeof(struct file_seq));
  if (!s){
    printf("can't allocate file_seq\n");
    return NULL;
  }
  s->filename = strdup(seq);
  s->next = NULL;
  s->prev = NULL;
  return s;
}
