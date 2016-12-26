#ifndef __VIS_DB_H
#define __VIS_DB_H

#define DB_PATH "/etc/vispace.db"
#define FIELD_MAX_SIZE 32
#define FIELD_MAX_COUNT 120
typedef struct
{
  int fields;
  uchar_t max_row;
  uchar_t row;
  uchar_t col;
  char (*col_name)[16];
  char (*results) [FIELD_MAX_SIZE];
}result_t;

typedef struct
{
  uchar_t max_row;
  uchar_t row;
  char *col_name;
  uchar_t col_size;
  char *cols;
}column_t;
#endif //__VIS_DB_H
