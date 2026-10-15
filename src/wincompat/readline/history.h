#ifndef GRADS_WINCOMPAT_HISTORY_H
#define GRADS_WINCOMPAT_HISTORY_H

typedef struct _hist_entry {
  char *line;
  char *timestamp;
  void *data;
} HIST_ENTRY;

extern int history_length;

void add_history(const char *line);
HIST_ENTRY **history_list(void);
int history_search_pos(const char *string, int direction, int pos);
int read_history(const char *filename);
int write_history(const char *filename);
int history_truncate_file(const char *filename, int lines);
int where_history(void);
HIST_ENTRY *remove_history(int which);

#endif
