/*
 Name: Trenton Tidwell
 BlazerId: tidwellt
 Course Section: CS 432
 Homework #: 1
*/

#include <iostream>
#include <ctime>      //Clock
#include <thread>
#include <chrono>
#include <stdlib.h>   //Rand
#include <stdio.h>
#include <omp.h>
using namespace std;

bool** InitializeBoard(bool** board, int Rows, int Cols, bool fill);
void copy_array_2d(bool** read_arr, bool** write_arr, int rows, int cols);
void fill_ghost(bool** board, int rows, int cols);
void delete_arr(bool** arr, int rows);
void game_of_life(bool* R0, bool* R1, bool* R2, bool* next, int Cols, int cyc);
void run_game(int ROWS, int COLS, int MAX_CYCLES, int THREADS);
void print_board(bool** board, int Rows, int Cols);


int main(int argc, char* argv[]){
  //COMMAND LINE ARGUMENT

  int ROWS = 10;
  int COLS = 10;
  int MAX_CYCLES = 16;
  int THREADS = 8;

  if(argc == 4){
    ROWS = stoi(argv[1]);
    COLS = stoi(argv[1]);
    MAX_CYCLES = stoi(argv[2]);
    THREADS = stoi(argv[3]);
  } else if(argc == 5){
    ROWS = stoi(argv[1]);
    COLS = stoi(argv[2]);
    MAX_CYCLES = stoi(argv[3]);
    THREADS = stoi(argv[4]);
  }

  clock_t begin = clock();

  run_game(ROWS, COLS, MAX_CYCLES, THREADS);

  //Clock and print time required
  clock_t end = clock();
  double elapsed_secs = ((double)end - (double)begin) / CLOCKS_PER_SEC;
  printf("\nTime required for program to run: %10.2f seconds\n\n", elapsed_secs);
  system ("pause");
  return 0;
}


void run_game(int ROWS, int COLS, int MAX_CYCLES, int THREADS){
  bool **GameBoard = new bool*[ROWS+2];
  bool **NextBoard = new bool*[ROWS+2];
  GameBoard = InitializeBoard(GameBoard, ROWS, COLS, true);
  NextBoard = InitializeBoard(NextBoard, ROWS, COLS, false);
  copy_array_2d(GameBoard, NextBoard, ROWS+2, COLS+2);


  //runs one iteration of the game, swaps the next/current board pointers and loops
  int fitted_max_cycles = MAX_CYCLES;
  int cycle_count = 0;
  int r; int n;

  # pragma omp parallel num_threads(THREADS) \
    default(none) shared(GameBoard, NextBoard, ROWS, COLS, fitted_max_cycles, cycle_count) private(r, n)
  while(true){
    if(cycle_count > fitted_max_cycles){break;}
    // game_of_life(GameBoard, NextBoard, ROWS, COLS);



    # pragma omp for
    for(r=1; r < ROWS; r++){
      printf("T:%d, R:%d\n", omp_get_thread_num(), r);
      n = omp_get_thread_num();
      // printf("\nmy r: %d my n %d  ", r, n);
      game_of_life(GameBoard[r-1], GameBoard[r], GameBoard[r+1], NextBoard[r], COLS, cycle_count);

    }

    fill_ghost(NextBoard, ROWS+2, COLS+2);
    // print_board(NextBoard, ROWS, COLS);
    copy_array_2d(NextBoard, GameBoard, ROWS+2, COLS+2);
    // printf("CYCLE: %d\n", cycle_count);
    # pragma omp critical
    cycle_count = cycle_count+1;
    printf("CYCLE: %d\n\n", cycle_count);
  }

  //Delete gameboards from the Heap
  delete_arr(GameBoard, ROWS+2);
  delete_arr(NextBoard, ROWS+2);
  return;
}

void game_of_life(bool* R0, bool* R1, bool* R2, bool* next, int Cols, int cyc){
  //Do not include GhostCells in array diminsions passed to this function
  //this method takes in 3 row parameters to decide what to write to 1 row in the next board

  // iterate through board GhostCells
  int live_count;
  for(int c = 1; c<=Cols; c++){
    live_count = 0;

    for(int j = -1; j <= 1; j++){
      if(R0[c+j]==true){live_count+=1;}
      if(R2[c+j]==true){live_count+=1;}
    }
    if(R1[c-1]==true){live_count+=1;}
    if(R1[c+1]==true){live_count+=1;}

    //Conway's algorithm
    if(live_count < 2 || live_count > 3 ){next[c] = false;}
    else {next[c] = true;}
  }

  return;
}

void fill_ghost(bool** board, int rows, int cols){
  //Sizes including GhostCells should be passed to this function
  //Coppies full array including ghost cells
  for(int i = 1; i < rows; i++){                       //Rows
    board[i][0] = board[i][cols-2];
    board[i][cols-1] = board[i][1];
  }

  for(int j = 1; j < cols; j++){                        //Cols
    board[0][j] = board[rows-2][j];
    board[rows-1][j] = board[1][j];
  }

  /***** copy board corner data to ghost cells******/
  board[0][0] = board[rows-2][cols-2];                       //Corners
  board[0][cols-1] = board[rows-2][1];
  board[rows-1][0] = board[1][cols-2];
  board[rows-1][cols-1] = board[1][1];

}

bool** InitializeBoard(bool** board, int Rows, int Cols, bool fill){
  //Do not include GhostCells in array diminsions passed to this function

  int max_row = Rows+1;
  int max_col = Cols+1;

  //Create collumn array to hold pointers to each row array
  for(int row = 0; row < Rows+2; row++){
    board[row] = new bool[Cols+2];
  }

  //During init, one board is filled and the other is copied
  //This is for the case that this object should be filled
  if(fill == true) {
    srand(time(NULL));

    // Iterate through Board
    for(int row = 1; row < Rows; row++){
      for(int col = 1; col <= Cols; col++){
        board[row][col] = (0==(rand()%5)); // 1 of 5 cells will be alive
      }
    }

    fill_ghost(board, Rows+2, Cols+2);

  }


  else{     //if fill variable is false, fill array with all false
    for(int row = 0; row <= Rows; row++){
      for(int col = 0; col <= Cols; col++){
        board[row][col] = false; // all cells are dead
      }
    }
  }

  return board;
}

void copy_array_2d(bool** read_arr, bool** write_arr, int rows, int cols){
  //Sizes including GhostCells should be passed to this function
  //Coppies full array including ghost cells

  for(int row = 0; row < rows; row++){
    for(int col = 0; col < cols; col++){
      write_arr[row][col] = read_arr[row][col];
    }
  }
}

void print_board(bool** board, int Rows, int Cols){
  //Sizes of dimensions excluding ghost cells should be passed here
  for(int i = 1; i<=Rows; i++){
    for(int j = 1; j<=Cols; j++){
      if (board[i][j] == true){cout << "O ";}
      else{cout << ". ";}
    }
    cout << "\n";
  }
  cout << "\n\n";
}

void delete_arr(bool** arr, int rows){
  //Full array dimensions (including Ghost Cells) Should be passed here

  int i;
  for(i = 0; i<rows; i++){
    delete[] arr[i];        //Delete each row pointer
    arr[i] = 0;
  }
  delete[] arr;            //Delete the pointer to the collumn which
  arr = 0;                //Holds the row pointers
}
