/* Minimal readline/history compatibility layer for native Windows builds. */

#ifdef _WIN32

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#define GA_HISTORY_INITIAL_CAPACITY 64

int history_length = 0;
static int history_capacity = 0;
static HIST_ENTRY **history_entries = NULL;

static char *ga_strdup(const char *value) {
  size_t length = strlen(value) + 1;
  char *copy = (char *)malloc(length);
  if (copy != NULL) memcpy(copy, value, length);
  return copy;
}

static int ga_history_reserve(int required) {
  HIST_ENTRY **resized;
  int capacity = history_capacity == 0 ? GA_HISTORY_INITIAL_CAPACITY : history_capacity;
  while (capacity < required + 1) capacity *= 2;
  if (capacity == history_capacity) return 0;
  resized = (HIST_ENTRY **)realloc(history_entries, sizeof(HIST_ENTRY *) * capacity);
  if (resized == NULL) return -1;
  history_entries = resized;
  history_capacity = capacity;
  history_entries[history_length] = NULL;
  return 0;
}

char *readline(const char *prompt) {
  char buffer[4096];
  size_t length;
  if (prompt != NULL) {
    fputs(prompt, stdout);
    fflush(stdout);
  }
  if (fgets(buffer, sizeof(buffer), stdin) == NULL) return NULL;
  length = strlen(buffer);
  while (length > 0 && (buffer[length - 1] == '\n' || buffer[length - 1] == '\r')) {
    buffer[--length] = '\0';
  }
  return ga_strdup(buffer);
}

void add_history(const char *line) {
  HIST_ENTRY *entry;
  if (line == NULL || ga_history_reserve(history_length + 1) != 0) return;
  entry = (HIST_ENTRY *)calloc(1, sizeof(HIST_ENTRY));
  if (entry == NULL) return;
  entry->line = ga_strdup(line);
  if (entry->line == NULL) {
    free(entry);
    return;
  }
  history_entries[history_length++] = entry;
  history_entries[history_length] = NULL;
}

HIST_ENTRY **history_list(void) {
  return history_entries;
}

int history_search_pos(const char *string, int direction, int pos) {
  int index;
  if (string == NULL || history_length == 0) return -1;
  if (pos < 0 || pos >= history_length) pos = direction < 0 ? history_length - 1 : 0;
  for (index = pos; index >= 0 && index < history_length; index += direction < 0 ? -1 : 1) {
    if (strstr(history_entries[index]->line, string) != NULL) return index;
  }
  return -1;
}

int read_history(const char *filename) {
  FILE *file;
  char buffer[4096];
  size_t length;
  if (filename == NULL || (file = fopen(filename, "r")) == NULL) return -1;
  while (fgets(buffer, sizeof(buffer), file) != NULL) {
    length = strlen(buffer);
    while (length > 0 && (buffer[length - 1] == '\n' || buffer[length - 1] == '\r')) buffer[--length] = '\0';
    add_history(buffer);
  }
  fclose(file);
  return 0;
}

int write_history(const char *filename) {
  FILE *file;
  int index;
  if (filename == NULL || (file = fopen(filename, "w")) == NULL) return -1;
  for (index = 0; index < history_length; ++index) fprintf(file, "%s\n", history_entries[index]->line);
  fclose(file);
  return 0;
}

int history_truncate_file(const char *filename, int lines) {
  FILE *file;
  char **saved = NULL;
  char buffer[4096];
  int count = 0, capacity = 0, index, first;
  if (filename == NULL || lines < 0 || (file = fopen(filename, "r")) == NULL) return -1;
  while (fgets(buffer, sizeof(buffer), file) != NULL) {
    if (count == capacity) {
      int next = capacity == 0 ? 64 : capacity * 2;
      char **resized = (char **)realloc(saved, sizeof(char *) * next);
      if (resized == NULL) break;
      saved = resized;
      capacity = next;
    }
    saved[count++] = ga_strdup(buffer);
  }
  fclose(file);
  if ((file = fopen(filename, "w")) == NULL) return -1;
  first = count > lines ? count - lines : 0;
  for (index = first; index < count; ++index) fputs(saved[index], file);
  fclose(file);
  for (index = 0; index < count; ++index) free(saved[index]);
  free(saved);
  return 0;
}

int where_history(void) {
  return history_length;
}

HIST_ENTRY *remove_history(int which) {
  HIST_ENTRY *removed;
  int index;
  if (which < 0 || which >= history_length) return NULL;
  removed = history_entries[which];
  for (index = which; index < history_length - 1; ++index) history_entries[index] = history_entries[index + 1];
  history_entries[--history_length] = NULL;
  return removed;
}

#endif
