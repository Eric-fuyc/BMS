#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <stdio.h>

#define DATABASE "database"

/*
 * Copy from CSDN
 *
 *    https://blog.csdn.net/ky_heart/article/details/53783479
 *
 */
int getch() {
  int c = 0;
  struct termios org_opts, new_opts;
  int res = 0;
  //-----  store old settings -----------
  res = tcgetattr(STDIN_FILENO, &org_opts);
  assert(res == 0);
  //---- set new terminal parms --------
  memcpy(&new_opts, &org_opts, sizeof(new_opts));
  new_opts.c_lflag &=
      ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
  tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
  c = getchar();
  //------  restore old settings ---------
  res = tcsetattr(STDIN_FILENO, TCSANOW, &org_opts);
  assert(res == 0);
  return c;
}

typedef struct {
  long id;
  char username[30];
  char name[15];
  char password[40];
} user, *userp;

typedef struct {
  userp source;
  userp target;
  int sum;
} record, *recordp;

typedef struct {
  long user_num;
  userp users;
  long record_num;
  recordp records;
} bankdata;

bankdata data;
user SYSTEM_ACCOUNT = {-1, "System", "System"};

userp bankdata_get_user_by_id(bankdata* data, long id) {
  if (id == -1) {
    return &SYSTEM_ACCOUNT;
  }
  for (long i = 0; i < data->record_num; i++) {
    userp current = (data->users + i);
    if (current->id == id) {
      return current;
    }
  }
  return NULL;
}

int bankdata_init(bankdata* data, char* filename) {
  FILE* f = fopen(filename, "r");

  int co = fscanf(f, "%ld%ld", &data->user_num, &data->record_num);
  if (co != 2)
    return -1;

  userp users = malloc(data->user_num * sizeof(user));
  if (!users) {
    return -1;
  }
  data->users = users;

  recordp records = malloc(data->record_num * sizeof(record));
  if (!records) {
    return -1;
  }
  data->records = records;

  for (long i = 0; i < data->user_num; i++) {
    userp current = (data->users + i);
    int co = fscanf(f, "%ld%s%s%s", &current->id, current->username,
                    current->name, current->password);
    if (co != 4)
      return -1;
  }

  for (long i = 0; i < data->record_num; i++) {
    recordp current = (data->records + i);
    long source, target;
    int co = fscanf(f, "%ld\t%ld\t%d\n", &source, &target, &current->sum);
    if (co != 3)
      return -1;

    current->source = bankdata_get_user_by_id(data, source);
    current->target = bankdata_get_user_by_id(data, target);
    if (current->source == NULL || current->target == NULL)
      return -1;
  }

  return fclose(f);
}

void bankdata_save(bankdata* data, char* filename) {
  FILE* f = fopen(filename, "w");

  fprintf(f, "%ld\t%ld\n", data->user_num, data->record_num);

  for (long i = 0; i < data->user_num; i++) {
    userp current = data->users + i;
    fprintf(f, "%ld\t%s\t%s\t%s\n", current->id, current->username,
            current->name, current->password);
  }

  for (long i = 0; i < data->record_num; i++) {
    recordp current = data->records + i;
    fprintf(f, "%ld\t%ld\t%d\n", current->source->id, current->target->id,
            current->sum);
  }

  fclose(f);
}
