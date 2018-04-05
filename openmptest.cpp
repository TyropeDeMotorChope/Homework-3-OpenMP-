#include <iostream>
#include <ctime>      //Clock
#include <stdlib.h>   //Rand
#include <stdio.h>
#include <omp.h>

int main()
{
  #pragma omp parallel
  {
    int ID = omp_get_thread_num();
    printf("hello(%d)", ID);
    printf("world(%d)\n", ID);
  }
  return 0;
}
