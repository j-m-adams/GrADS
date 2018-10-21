/* Copyright (C) 1988-2018 by George Mason University. See file COPYRIGHT for more information. */

/*  written by Joe Wielgosz */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "config.h"
#include "gabufr.h"

#ifndef HAVE_FSEEKO
gaint fseeko(FILE *stream, off_t offset, gaint whence) {
  fseek(stream, (long)offset, whence);
}
off_t ftello(FILE *stream) {
  return (off_t)ftell(stream);
}
#endif

enum bufrscan_modes {
  HEADER_MODE = 0,
  DATA_MODE
};

void gabufr_print_dset(gabufr_dset * dset) {
  gabufr_msg * msg;
  gabufr_val * val;
  gabufr_varinf *varinf=NULL;
  gaint i;

  for (msg = dset->msgs; msg != NULL; msg = msg->next) {
    printf("\n\nmsg %d:\n", msg->fileindex);
    for (i = 0; i < msg->subcnt; i++) {
      printf("\nsubset %d:\n", i);
      for (val = msg->subs[i]; val != NULL; val = val->next) {
	printf("%d (%d) [%.3d]    0-%.2d-%.3d     ", msg->fileindex, 
	       i, val->z, val->x, val->y);
	if (val->sval) {
	  printf("[%s]", val->sval);
	} else {
	  if (val->undef) {
	    printf("undef (%g)", val->val);
	  } else {
	    printf("%g", val->val);
	  }	    
	}
	if ( gabufr_valid_varid(0, val->x, val->y)
	     && varinf == gabufr_get_varinf(val->x, val->y)) {
	  printf("\t\t(%s)", varinf->description);
	}
	printf("\n");
      }
    }
  }
}

void help() {
      printf("bufrscan [-h] [-d] tablepath filenames ...\n");
      printf("tablepath: directory containing BUFR decoding tables\n");
      printf("filenames: BUFR messages to be decoded\n");
      printf("-h, --header: print BUFR message headers (default)\n");
      printf("-d, --data: print BUFR message contents \n");
      printf("-?, --help: print this help message\n");
}

gaint main (gaint argc, char *argv[]) {
  gabufr_dset * dset;
  gaint i;
  const char * tablepath = NULL;
  gaint mode = HEADER_MODE;
  if (argc < 3) {
    help();
  }
  for (i = 1; i < argc; i++) {
    if (! strcmp(argv[i], "-d") || ! strcmp(argv[i], "--data")) {
      mode = DATA_MODE;
    } else if (! strcmp(argv[i], "-h") || ! strcmp(argv[i], "--header")) {
      mode = HEADER_MODE;
    } else if (! strcmp(argv[i], "-?") || ! strcmp(argv[i], "--help")) {
      help();
    } else if (!tablepath) {
	tablepath = argv[i];
	gabufr_set_tbl_base_path(tablepath);
    } else {
      if (mode == HEADER_MODE) {
	dset = gabufr_scan(argv[i]);
	if (! dset) {
	  return GABUFR_ERR;
	}
	gabufr_close(dset);
      } else {
	dset = gabufr_open(argv[i]);
	if (! dset) {
	  return GABUFR_ERR;
	}
	gabufr_print_dset(dset);
	
	gabufr_close(dset);
      }
    }	   
  }
  return GABUFR_OK;
}
