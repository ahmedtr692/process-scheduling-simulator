#include "headers/basic_sched.h"

int main(int argv, char** argc){
  File* file ;
  char* path ;
  path = argc[1];
  file = fopen(path, 'r');
  if(file == NULL) return -1 ;
  return 0 ;
}
