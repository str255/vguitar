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

  options.h file
*/
#ifndef __VGUITR_OPTIONS__
#define __VGUITR_OPTIONS__

#define true 1
#define false 0
#define SIXSTRING  6

typedef struct file_seq {
  char *filename;
  struct file_seq *prev;
  struct file_seq *next;
} file_seq;

file_seq *file_seq_new(char *filename);

typedef enum datafile_format {
  format_vguitar,
  format_classicTrax,
  format_gp5,
} datafile_format;

typedef enum sharp_flat_string {
  STRING_TRUE,
  STRING_FLAT,
  STRING_SHARP
} sharp_flat_string;

void print_options();
void default_options();
void parse_options(int argc, char **argv);
void read_input_file(char *input_filename, datafile_format input_format);
void read_input_file_classicTrax(FILE*fp);
void read_input_file_gp5(FILE*fp);
void write_input_file();
void write_output_file();
void write_output_file_classicTrax(FILE *fp);
void parse_chord(char *arg, chord *achord);

typedef enum run_mode {
  do_tablature,
  do_strum,
  do_strum2,
  do_box,
} run_mode;

#if 0
typedef enum tuning_mode {
  TUNING_EADGBE,
  TUNING_OPEND,
  TUNING_FREQ,
  TUNING_MIDI,
} tuning_mode;
#endif

typedef struct options_vguitar {
  int     headless;          // headless, if true 
  int     runningMode;
  tuning_mode tuning;
  int     do_single_digit_fret;
  int     do_clip;
  int     is_allput;
  int     has_input, has_output;
  int     verbose;
  //  int     alsa_client_addr;
  //  int     alsa_client_port;
  int     alsa_server_addr;
  int     alsa_server_port;
  int     bpm;
  float   roll_factor;     // 0.1
  int     clip_start, clip_end;
  int     loop;
  sstrum *strum_sequence;
  sstrum *current_strum_sequence;
  //  char   *strum_sequence;
  chord  *chords;
  chord  *chord_stack;
  int     has_dir;
  char    dirpath[255];
  file_seq *input_file;
  char    output_file[255];
  FILE    *input, *output;
  //  float   strings_tuning_frequency[SIXSTRING];
  int     strings_tuning_midi[SIXSTRING];
  datafile_format input_format, output_format;
} options_vguitar;
#endif
