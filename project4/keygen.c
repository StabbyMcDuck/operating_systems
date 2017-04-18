/*****************************
  Author: Regina Imhoff
  Email:  imhoffr@oregonstate.edu
  Due:    06/06/2016
  Desc:   This program creates a key file of specified length.
******************************/

/* INCLUDES */
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>

/* MAIN FUNCTION */
int main(int argc, char *argv[]){
  int i;
  int randomNumber;
  int randomCharacter;

  // seed random number generator with time
  srand(time(NULL));

  if(argc != 2){
    printf("Usage: %s LENGTH\n", argv[0]);
    exit(1);
  }

  // argv[1] gets assigned to the key_length
  int key_length = atoi(argv[1]);

  // create the key
  for(i = 0; i < key_length; i++){
    randomNumber = rand() % 27;

    if(randomNumber < 26){
      randomCharacter = 65 + randomNumber;
      printf("%c", randomCharacter);
    }
    else{
        // print space
        printf(" ");
    }
  }

  // add a new line
  printf("\n");
}
