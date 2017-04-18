/********************************************************
* Author: Regina Imhoff                                 *
* Email: imhoffr@oregonstate.edu                        *
* Title: Program 2                                      *
* Due: May 2, 2016                                      *
* Description: Making a game                            *
*   like Colossal Cave                                  *
* https://en.wikipedia.org/wiki/Colossal_Cave_Adventure *
********************************************************/

/************
 * Includes *
 ************/
 #include <assert.h>
 #include <stdlib.h>
 #include <string.h>
 #include <stdio.h>
 #include <time.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <unistd.h>
 #include <dirent.h>

/******************
 * Defines & CONST*
 ******************/
#define ROOM_NAMES_DEFINED 10
#define STRING_LENGTH_MAX 31

/********
 * Enum *
 ********/
enum RoomType { START_ROOM, END_ROOM, MIDDLE_ROOM };

/***********
 * Structs *
 ***********/
struct room_node {
  int roomID;
  char *name;
  int *roomConnections;
  int numberOfConnections;
  enum RoomType roomType;
};

struct room_container {
  struct room_node **rooms;
  char **roomNames;
  int numberOfRooms;
};

/***********************
 * Function Prototypes *
 ***********************/
const char * runRoomToString(enum RoomType);
char * getRoomName(int);
void initializeRooms(struct room_container *);
void destroyRooms(struct room_container *);
struct room_container *initializeRoomNodes(const int);
void printRooms(struct room_container *);
void linkNodes(struct room_container *, int, int);
void randomConnections(struct room_container *);
void printOutputFile(struct room_container *, int, char *);
void outputFile(struct room_container *, char *);
int numberOfFilesInDirectory(char *);
void loadFiles(struct room_container *, char *);
int readFileLines(FILE *, char *, const int);
int getRoomIndex(struct room_container *, char *);
struct room_node *getStartingNode(struct room_container *);
void printConnections(struct room_container *, struct room_node *);
struct room_node *getNextRoomInNode(struct room_container *, struct room_node *currentRoomNode);

/****************
 * Main Program *
 ****************/
int main(){
  char DIRECTORY_NAME[30];
  sprintf(DIRECTORY_NAME, "imhoffr.rooms.%d", (int)getpid());

  // create new directory
  // mkdir in c http://pubs.opengroup.org/onlinepubs/009695399/functions/mkdir.html
  mkdir(DIRECTORY_NAME, 0777);

  //seed random
  srand(time(0));

  /* construct maze */
  struct room_container *containsRooms = initializeRoomNodes(9);

  // set room names
  initializeRooms(containsRooms);

  //build connections
  randomConnections(containsRooms);

  // create new file
  outputFile(containsRooms, DIRECTORY_NAME);

  // deallocate maze generation
  destroyRooms(containsRooms);
  containsRooms = NULL;

  /* Load Maze */
  containsRooms = initializeRoomNodes(numberOfFilesInDirectory(DIRECTORY_NAME));

  loadFiles(containsRooms, DIRECTORY_NAME);

  /* Play Game */
  const int MAXIMUM_GAME_MOVES = 100;
  struct room_node *currentLocation;
  struct room_node *userChoice;
  char *placementHistory[MAXIMUM_GAME_MOVES];
  int movesByPlayer;
  int inputValid;
  int i;

  currentLocation = getStartingNode(containsRooms);

  while(currentLocation->roomType != END_ROOM && movesByPlayer < MAXIMUM_GAME_MOVES){
    do{
      printf("CURRENT LOCATION: %s\r\n", currentLocation->name);
      printf("POSSIBLE ROUTES: ");
      printConnections(containsRooms, currentLocation);
      printf("\r\n");

      printf("WHICH WAY SHOULD WE GO >");
      
      userChoice = getNextRoomInNode(containsRooms, currentLocation);

      if(userChoice == NULL){
        inputValid = 0; //false
        printf("\r\n");
        printf("I DO NOT UNDERSTAND.  TRY AGAIN. \r\n");
      }else{
        inputValid = 1;//true
        currentLocation = userChoice;
      }
      printf("\n");
    } while(inputValid == 0);
    placementHistory[movesByPlayer] = currentLocation->name;
    movesByPlayer++;
  }

  // end room
  if(currentLocation->roomType == END_ROOM){
    printf("CONGRATULATIONS!!!! YOU HAVE FOUND THE END ROOM!\n");
    printf("YOU TOOK %d STEPS.  YOUR TOOK THE FOLLOWING PATH: \r\n", movesByPlayer);
    for(i = 0; i < movesByPlayer; i++){
      printf("%s \r\n", placementHistory[i]);
    }
  }else{
    printf("YOU RAN OUT OF MOVES AND DIED IN THE HOUSE.\r\n");
  }

  destroyRooms(containsRooms);
  containsRooms = NULL;
  
  return(0);
}

/*************
 * Functions *
 *************/
/*  runRoomToString
    switch cases function */
const char *runRoomToString(enum RoomType types) {
  switch(types){
    case START_ROOM:
      return "START_ROOM";
      break;
    case MIDDLE_ROOM:
      return "MIDDLE_ROOM";
      break;
    case END_ROOM:
      return "END_ROOM";
      break;
    // if none of the above cases, then do default:
    default:
      printf("ERROR: room type not defined");
      exit(1);
  }
}

/*  getRoomName
    rooms are assigned here */
char *getRoomName(int roomID) {
  char *roomName;
  roomName = malloc(sizeof(char) * (STRING_LENGTH_MAX + 1));

  switch(roomID){
    case 0:
      strncpy(roomName, "Music Room", STRING_LENGTH_MAX);
      break;
    case 1:
      strncpy(roomName, "Lady's Parlor", STRING_LENGTH_MAX);
      break;
    case 2:
      strncpy(roomName, "Dining Room", STRING_LENGTH_MAX);
      break;
    case 3:
      strncpy(roomName, "Kitchen", STRING_LENGTH_MAX);
      break;
    case 4:
      strncpy(roomName, "Scullery", STRING_LENGTH_MAX);
      break;
    case 5:
      strncpy(roomName, "Butler's Pantry", STRING_LENGTH_MAX);
      break;
    case 6:
      strncpy(roomName, "Library", STRING_LENGTH_MAX);
      break;
    case 7:
      strncpy(roomName, "Snooker Room", STRING_LENGTH_MAX);
      break;
    case 8:
      strncpy(roomName, "Nursery", STRING_LENGTH_MAX);
      break;
    case 9:
      strncpy(roomName, "Study", STRING_LENGTH_MAX);
      break;
    default:
      printf("ERROR: Could not assign room %d \n", roomID);
      exit(1);
      break;
  }
  // inserts null char at the end of strings
  roomName[STRING_LENGTH_MAX] = '\0';

  return roomName;
}

/*  initializeRooms
    create an array of strings w/ room names */
void initializeRooms(struct room_container *containsRooms){
  // allocate space for the room names
  containsRooms->roomNames = malloc(sizeof(char *) * containsRooms->numberOfRooms);

  int i;
  int *roomArray = malloc(sizeof(int *) * ROOM_NAMES_DEFINED);

  for(i = 0; i < ROOM_NAMES_DEFINED; i++){
    roomArray[i] = 0;
  }

  // give the rooms a name
  for(i = 0; i < containsRooms->numberOfRooms; i++){
    int randomRooms = rand() % ROOM_NAMES_DEFINED;
    while(roomArray[randomRooms] == 1){
      randomRooms = rand() % ROOM_NAMES_DEFINED;
    }
    roomArray[randomRooms] = 1;

    containsRooms->roomNames[i] = getRoomName(randomRooms);
    containsRooms->rooms[i]->name = containsRooms->roomNames[i];
  }

  free(roomArray);
}

struct room_container *initializeRoomNodes(const int CONST_NUMBER_OF_ROOMS){
  // allocate space for room struct
  struct room_container *containsRooms = malloc(sizeof(struct room_container));
  containsRooms->numberOfRooms = CONST_NUMBER_OF_ROOMS;

  // array of room nodes
  containsRooms->rooms = malloc(sizeof(struct room_node *) * CONST_NUMBER_OF_ROOMS);

  containsRooms->roomNames = NULL;

  int i;

  // create nodes
  for(i = 0; i < CONST_NUMBER_OF_ROOMS; i++){
    // make space for nodes
    containsRooms->rooms[i] = malloc(sizeof(struct room_node));

    // set values to all rooms
    containsRooms->rooms[i]->roomID = i;
    containsRooms->rooms[i]->name = NULL;
    containsRooms->rooms[i]->roomConnections = malloc(sizeof(int) * CONST_NUMBER_OF_ROOMS);
    containsRooms->rooms[i]->numberOfConnections = 0;
    containsRooms->rooms[i]->roomType = MIDDLE_ROOM;

    // connections array connection
    int j;
    for(j = 0; j < CONST_NUMBER_OF_ROOMS; j++){
      containsRooms->rooms[i]->roomConnections[j] = 0;
    }
  }

  // set start and end rooms
  containsRooms->rooms[0]->roomType = START_ROOM;
  containsRooms->rooms[rand() % (containsRooms->numberOfRooms - 1) + 1]->roomType = END_ROOM;

  return containsRooms;
}

/*  destroyRooms  */
void destroyRooms(struct room_container *containsRooms){
  // free up the memory in each room in the array
  int i;
  for(i = 0; i < containsRooms->numberOfRooms; i++){
    free(containsRooms->rooms[i]->name); // deallocate name pointer
    free(containsRooms->rooms[i]->roomConnections); // deallocate connections
    free(containsRooms->rooms[i]);
    containsRooms->rooms[i] = NULL;
  }

  // deallocate arrays
  free(containsRooms->roomNames);
  free(containsRooms->rooms);

  // reset struct values
  containsRooms->numberOfRooms = 0;
  containsRooms->roomNames = NULL;
  containsRooms->rooms = NULL;
}

void printRooms(struct room_container *containsRooms) {
  int i;

  for(i = 0; i < containsRooms->numberOfRooms; i++){
    printf("i = %d\r\n", i);
    printf("ID: %d\r\n", containsRooms->rooms[i]->roomID);
    printf("Name: %s\r\n", containsRooms->rooms[i]->name);
    printf("Connections: ");

    int j;

    for(j = 0; j < containsRooms->numberOfRooms; j++){
      printf("%d, \n", containsRooms->rooms[i]->roomConnections[j]);
    }

    printf("\r\n");
    printf("Number of Connections: %d\r\n", containsRooms->rooms[i]->numberOfConnections);
    printf("Room Type: ");
      switch(containsRooms->rooms[i]->roomType) {
           case START_ROOM:
                printf("START_ROOM\r\n");
                break;
           case END_ROOM:
                printf("END_ROOM\r\n");
                break;
           case MIDDLE_ROOM:
                printf("MIDDLE_ROOM\r\n");
                break;
           default:
                printf("Something bad happened\r\n");
                exit(1);
                break;
      }
      printf("\r\n");
  }
}

void linkNodes(struct room_container *containsRooms, int firstRoom, int secondRoom){
  // rooms have no more than the max number of connections
  assert(firstRoom != secondRoom);
  assert((containsRooms->rooms[firstRoom]->numberOfConnections) < (containsRooms->numberOfRooms -1));
  assert((containsRooms->rooms[secondRoom]->numberOfConnections) < (containsRooms->numberOfRooms - 1));

  // link firstRoom to secondRoom
  containsRooms->rooms[firstRoom]->roomConnections[secondRoom] = 1;

  // link secondRoom to firstRoom
  containsRooms->rooms[secondRoom]->roomConnections[firstRoom] = 1;

  // increase the # of connections
  containsRooms->rooms[firstRoom]->numberOfConnections++;
  containsRooms->rooms[secondRoom]->numberOfConnections++;
}

void randomConnections(struct room_container *containsRooms){
  // 7 rooms:  MAXIMUM_NUMBER_OF_CONNECTIONS = 6, MINIMUM_NUMBER_OF_CONNECTIONS = 3
  const int MAXIMUM_NUMBER_OF_CONNECTIONS = containsRooms->numberOfRooms - 1;
  const int MINIMUM_NUMBER_OF_CONNECTIONS = containsRooms->numberOfRooms / 2;

  int i;

  for(i = 0; i < MINIMUM_NUMBER_OF_CONNECTIONS + 1; i++){
    int k;

    for(k = 0; k < containsRooms->numberOfRooms; k++){
      // if MAXIMUM_NUMBER_OF_CONNECTIONS then skip.
      // the last connection has a 1 in 3 chance of additional connection
      if((i < MINIMUM_NUMBER_OF_CONNECTIONS && containsRooms->rooms[k]->numberOfConnections <= i) || (i >= MINIMUM_NUMBER_OF_CONNECTIONS && rand() % 3 == 0)){
        int randomRoomR = rand() % containsRooms->numberOfRooms;
        while((containsRooms->rooms[k]->roomConnections[randomRoomR] == 1) || randomRoomR == k){
          randomRoomR = rand() % containsRooms->numberOfRooms;
        }

        linkNodes(containsRooms, k, randomRoomR);
      }
    }
  }
}

void printOutputFile(struct room_container *containsRooms, int roomID, char *FILE_NAME){
  FILE *fileOut = fopen(FILE_NAME, "w");

  struct room_node *room = containsRooms->rooms[roomID];

  // error checking
  if(fileOut == NULL){
    printf("ERROR: COULD NOT WRITE TO FILE %s \n!", FILE_NAME);
    exit(1);
  }

  // print room name
  fprintf(fileOut, "ROOM NAME: %s\n", room->name);

  int i;
  int roomConnectionsCount = 0;

  // print connection
  for(i = 0; i < containsRooms->numberOfRooms; i++){
    if(room->roomConnections[i] == 1){ // if a connection exists...
        roomConnectionsCount++;
        fprintf(fileOut, "CONNECTION %d: %s\n", roomConnectionsCount, containsRooms->rooms[i]->name);
    }
  }

  // print room type
  fprintf(fileOut, "ROOM TYPE: %s\n", runRoomToString(room->roomType));

  // close the fileOut
  fclose(fileOut);
}

void outputFile(struct room_container *containsRooms, char *DIRECTORY_NAME){
  char FILE_NAME[50];
  char file_array[10];
  int i;

  // each room needs a file
  for(i = 0; i < containsRooms->numberOfRooms; i++){
    //build the file name
    FILE_NAME[0] = '\0'; // erase contents of file
    // strcat http://www.techonthenet.com/c_language/standard_library_functions/string_h/strcat.php
    strcat(FILE_NAME, DIRECTORY_NAME);
    strcat(FILE_NAME, "/");
    // sprintf http://www.techonthenet.com/c_language/standard_library_functions/stdio_h/sprintf.php
    sprintf(file_array, "%d", containsRooms->rooms[i]->roomID);
    strcat(FILE_NAME, file_array);

    printOutputFile(containsRooms, i, FILE_NAME);
  }
}

int numberOfFilesInDirectory(char *DIRECTORY_NAME){
  // https://www.gnu.org/software/libc/manual/html_node/Opening-a-Directory.html
  DIR *directoryD;
  struct dirent *newDirectory;
  int fileCount = 0;

  directoryD = opendir(DIRECTORY_NAME);
  assert(directoryD != NULL);

  while((newDirectory = readdir(directoryD)) != NULL){
    fileCount++;
  }

  closedir(directoryD); // close directory

  fileCount = fileCount - 2;

  return fileCount;
}

void loadFiles(struct room_container *containsRooms, char *DIRECTORY_NAME){
  const int ROOM_BUFFER = 100;
  DIR *directoryD;
  struct dirent *newDirectory;
  int fileCount = 0;
  char fileBuffer[ROOM_BUFFER];
  char filePathName[100];
  char *roomName = 0;
  char *bufferCommand = 0;
  char *name = 0;

  containsRooms->roomNames = malloc(sizeof(char *) * containsRooms->numberOfRooms);

  directoryD = opendir(DIRECTORY_NAME);
  assert(directoryD != NULL);

  // directory . and ..
  readdir(directoryD);
  readdir(directoryD);

  // load the name of the room in the file
  while((newDirectory = readdir(directoryD)) != NULL){
    fileBuffer[0] = 0;

    filePathName[0] = 0;
    strcpy(filePathName, DIRECTORY_NAME);
    strcat(filePathName, "/");
    strcat(filePathName, newDirectory->d_name);

    // open file
    // fopen need "r" to open file for reading http://www.tutorialspoint.com/c_standard_library/c_function_fopen.htm
    FILE *fin = fopen(filePathName, "r");

    // read room name and store
    readFileLines(fin, fileBuffer, ROOM_BUFFER);

    fclose(fin);

    // strstr : http://www.tutorialspoint.com/c_standard_library/c_function_strstr.htm
    // strstr(haystack, needle)
    roomName = strstr(fileBuffer, ": ") + 2;

    containsRooms->roomNames[fileCount] = malloc(sizeof(char) * STRING_LENGTH_MAX);

    strcpy(containsRooms->roomNames[fileCount], roomName);
    containsRooms->rooms[fileCount]->name = containsRooms->roomNames[fileCount];

    fileCount++;
  }

  closedir(directoryD);

  directoryD = opendir(DIRECTORY_NAME);
  assert(directoryD != NULL);

  // directory . and ..
  readdir(directoryD);
  readdir(directoryD);

  fileCount = 0;

  while((newDirectory = readdir(directoryD)) != NULL){
    filePathName[0] = 0;
    strcpy(filePathName, DIRECTORY_NAME);
    strcat(filePathName, "/");
    strcat(filePathName, newDirectory->d_name);

    FILE *fin = fopen(filePathName, "r");
    // rewind: http://www.tutorialspoint.com/c_standard_library/c_function_rewind.htm
    rewind(fin);

    while(readFileLines(fin, fileBuffer, ROOM_BUFFER)){
      name = strstr(fileBuffer, ": ") + 2;
      *strstr(fileBuffer, ": ") = 0;
      bufferCommand = fileBuffer;

      if(strcmp(bufferCommand, "ROOM NAME") == 0){
        assert(strcmp(name, containsRooms->rooms[fileCount]->name) == 0);
      }else if(strncmp(bufferCommand, "CONNECTION", 10) == 0){
        containsRooms->rooms[fileCount]->roomConnections[getRoomIndex(containsRooms, name)] = 1;
        containsRooms->rooms[fileCount]->numberOfConnections++;
      }else if(strcmp(bufferCommand, "ROOM TYPE") == 0){
        if(strcmp(name, "START_ROOM") == 0){
          containsRooms->rooms[fileCount]->roomType = START_ROOM;
        }else if(strcmp(name, "MIDDLE_ROOM") == 0){
          containsRooms->rooms[fileCount]->roomType = MIDDLE_ROOM;
        }else if(strcmp(name, "END_ROOM") == 0){
          containsRooms->rooms[fileCount]->roomType = END_ROOM;
        }else{
          printf("ERROR:  COULD NOT READ ROOM TYPE (%s) IN FILE (%s)\n", name, filePathName);
          exit(1);
        }
      }else{
        printf("ERROR:  COULD NOT PARSE COMMAND (%s) IN FILE (%s)\n", bufferCommand, filePathName);
        exit(1);
      }
    }
  fclose(fin);
  fileCount++;
  }
  closedir(directoryD);
}

int readFileLines(FILE *fin, char *fileBuffer, const int ROOM_BUFFER) {
     char fileChar;
     int i = 0;          //index
     int endFlag = 0;

     while (endFlag == 0) {
          //retrieve the next character
          fileChar = fgetc(fin);

          if (fileChar == '\n' || fileChar == '\r' || fileChar == 0 || fileChar == -1) {
               fileBuffer[i] = 0;
               endFlag = 1;
               return i;
          } else {
               fileBuffer[i] = fileChar;
          }

          //error check, prevent buffer overflows
          assert(i < ROOM_BUFFER);

          i++;
     }

     return i;
}

int getRoomIndex(struct room_container *containsRooms, char *stringStart) {
     int i;
     for (i = 0; i < containsRooms->numberOfRooms; i++) {
          if ( strcmp( containsRooms->roomNames[i], stringStart ) == 0 ) {
               return i;
          }
     }

     //index not found
     return -1;
}

struct room_node *getStartingNode(struct room_container *containsRooms) {
     int i;
     struct room_node *nodePointer;

     for(i = 0; i < containsRooms->numberOfRooms; i++) {
          nodePointer = containsRooms->rooms[i];
          if (nodePointer->roomType == START_ROOM) {
               return nodePointer;
          }
     }

     printf("ERROR: Could not find START_NODE\r\n");
     exit(1);
}

void printConnections(struct room_container *containsRooms, struct room_node *nodeForRoom) {
     int i;
     int count = 0;

     for (i = 0; i < containsRooms->numberOfRooms; i++) {
          if (nodeForRoom->roomConnections[i] == 1) {
               count++;

               if (count < nodeForRoom->numberOfConnections) {
                    printf("%s, ", containsRooms->roomNames[i]);
               } else {
                    //last connection to print
                    printf("%s.", containsRooms->roomNames[i]);
               }

          }
     }
}

struct room_node *getNextRoomInNode(struct room_container *containsRooms, struct room_node *currentRoomNode) {
     char roomNodeChar;
     int index;
     const int BUFFER_SIZE = 100;
     char nodeBuffer[BUFFER_SIZE];

     //get user input
     fgets (nodeBuffer, BUFFER_SIZE, stdin);
     *strchr(nodeBuffer, '\n') = 0;

     if ((index = getRoomIndex(containsRooms, nodeBuffer)) >= 0) {
          if (currentRoomNode->roomConnections[index] == 1)
               return containsRooms->rooms[index];
     }

     return NULL;

}
