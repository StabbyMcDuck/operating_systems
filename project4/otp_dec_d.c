/*****************************
  Author: Regina Imhoff
  Email:  imhoffr@oregonstate.edu
  Due:    06/06/2016
  OTP decode - client
******************************/

/* INCLUDES */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

/* FUNCTION PROTOTYPES */
int getFile(int socket, int process_id);
void getKey(int socket, int process_id);
void thread(int socket, int process_id);
void decode(int process_id, int number_of_characters, int socket);

/* MAIN FUNCTION */
int main(int argc, char *argv[]){
  if (argc != 2) {
    printf("Usage: %s DECRYPTION_PORT\n", argv[0]);
    exit(1);
  }

  int socket_file_descriptor;
  int new_socket_file_descriptor;
  int port_number;
  int process_id;

  socklen_t client_length; // stores the size of client address
  struct sockaddr_in server_address;
  struct sockaddr_in client_address;

  // create new socket!
  socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);

  // initialize server_address to all zeroes
  bzero((char *) &server_address, sizeof(server_address));

  // change command line command from char to int
  port_number = atoi(argv[1]);

  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port_number);
  server_address.sin_addr.s_addr = INADDR_ANY; // ip address of the machine!

  // bind program to the address
  bind(socket_file_descriptor, (struct sockaddr *) &server_address, sizeof(server_address));

  // listen to the
  listen(socket_file_descriptor, 5);

  client_length = sizeof(client_address);

  while(1){
    new_socket_file_descriptor = accept(socket_file_descriptor, (struct sockaddr *) &client_address, &client_length);
    process_id = fork();  // New thread created!

    if(process_id == 0){
        close(socket_file_descriptor);
        int child_process = getpid();
        thread(new_socket_file_descriptor, child_process);
        exit(0);
    }
  }

  return 0;
}

/* FUNCTIONS */
// getFile fetches the file using process_id
int getFile(int socket, int process_id){
  char file_name[10];
  bzero(file_name, 10);

  char receivedMessage[10];
  sprintf(file_name, "%d", process_id);

  // create a temp file to store the text
  FILE *temp_file = fopen(file_name, "a");
  int temp_file_size = 0;

  // get the chunks
  while((temp_file_size = recv(socket, receivedMessage, 10, 0)) > 0){
    // write to the file
    int write_size = fwrite(receivedMessage, sizeof(char), temp_file_size, temp_file);
    bzero(receivedMessage, 10);

    // Anything less than a full size chunk is the last chunk!
    if(temp_file_size == 0 || temp_file_size != 10){
      break;
    }
  }
  int number_of_characters = ftell(temp_file);

  fclose(temp_file);

  /*FILE *text = fopen(file_name, "r");
  fseek(text, 0, SEEK_END);
  fclose(text);*/

  return number_of_characters;
}

void getKey(int socket, int process_id){
  char file_name[10];
  bzero(file_name, 10);

  char receivedMessage[10];
  sprintf(file_name, "%d", process_id);

  FILE *temp_file = fopen(file_name, "a");
  int temp_file_size = 0;

  while((temp_file_size = recv(socket, receivedMessage, 10, 0)) > 0){
    int write_size = fwrite(receivedMessage, sizeof(char), temp_file_size, temp_file);

    bzero(receivedMessage, 10);

    // Anything less than a full size chunk is the last chunk!
    if(temp_file_size == 0 || temp_file_size != 10){
      break;
    }
  }
  fclose(temp_file);
}

void thread(int socket, int process_id){
  char type[2];
  bzero(type, 2);
  sprintf(type, "d");
  // authenticate decode server
  int n = write(socket, type, strlen(type));

  int number_of_characters = getFile(socket, process_id); // get the file and store the number of characters
  char confirm[2];
  bzero(confirm, 1);
  sprintf(confirm, "1");
  n = write(socket, confirm, strlen(confirm)); // client moves on

  getKey(socket, process_id); // key file acquired
  decode(process_id, number_of_characters, socket);

  close(socket);
}

const char letters[28] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

int offset(char letter){
  int i;
  int letter_offset = -1;

  for(i = 0; i < sizeof(letters); i++){
    if(letters[i] == letter){
      letter_offset = i;
      break;
    }
  }
  if(letter_offset == -1){
    fprintf(stderr, "ERROR: bad letter `%d`", letter);
    exit(1);
  }
  return letter_offset;
}

void decode(int process_id, int number_of_characters, int socket){
  int fill_array = number_of_characters - 1; // -1 for the new line char
  char file_array[fill_array];
  char key_array[fill_array];
  char encrypted_array[fill_array];
  int i = 0;
  int c;

  char file_name[10];
  bzero(file_name, 10);
  sprintf(file_name, "%d", process_id);

  FILE *temp_file = fopen(file_name, "r");
  while((c = fgetc(temp_file)) != EOF && i < fill_array){
    file_array[i] = c;
    i++;
  }

  char key_name[10];
  bzero(key_name, 10);
  sprintf(key_name, "%d", process_id);

  int j = 0;
  int d;

  while((d = fgetc(temp_file)) != EOF && j < fill_array){
    key_array[j] = d;
    j++;
  }
  fclose(temp_file);

  for(i = 0; i < fill_array; i++){
    int sum = 0;
    
    int message_offset = offset(file_array[i]);

    int key_offset = offset(key_array[i]);

    sum = offset(file_array[i]) + offset(key_array[i]);
    /*for(j = 0; j < 28; j++){
      if(file_array[i] == letters[j]){
        sum = sum + j;
      }
      if(key_array[i] == letters[j]){
        sum = sum + j;
      }
    }*/

    int remainder = sum % 27; // mod27 not 28
    encrypted_array[i] = letters[remainder];
  }

  fprintf(stderr, "%s\n", encrypted_array);

  char encrypted_message[number_of_characters];
  bzero(encrypted_message, number_of_characters);
  sprintf(encrypted_message, "%s", encrypted_array);

  int z = write(socket, encrypted_array, fill_array);
  unlink(file_name);
  unlink(key_name);
}
