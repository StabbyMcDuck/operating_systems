/*****************************
  Author: Regina Imhoff
  Email:  imhoffr@oregonstate.edu
  Due:    06/06/2016
  OTP encode
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
#include <fcntl.h>

/* FUNCTION PROTOTYPES */
void sendText(int socket_file_descriptor, char file_name[]);
void sendKey(int socket_file_descriptor, char file_name[]);

/* MAIN FUNCTION */
int main(int argc, char *argv[]){
  if (argc != 4) {
    printf("Usage: %s PLAIN_TEXT_PATH KEY_PATH ENCRYPTION_PORT\n", argv[0]);
    exit(1);
  }

  // key must be longer than the input file!!
  FILE *text = fopen(argv[1], "r");

  if (text == NULL) {
    perror("Cannot open PLAIN_TEXT_PATH");
    exit(2);
  }

  FILE *key = fopen(argv[2], "r");

  if (key == NULL) {
    perror("Cannot open KEY_PATH");
    exit(3);
  }
  
  fseek(text, 0, SEEK_END);
  fseek(key, 0, SEEK_END);

  int bytes_in_text = ftell(text);
  int bytes_in_key = ftell(key);

  // close
  fclose(text);
  fclose(key);

  if(bytes_in_key < bytes_in_text){
    fprintf(stderr, "ERROR:  key %s is too short!\n", argv[2]);
    exit(1);
  }

  // back character check
  char character;
  FILE *text_file = fopen(argv[1], "r");

  while((character = fgetc(text_file)) != EOF){
    if(character == '$'){
      fprintf(stderr, "ERROR: %s has 1 or more bad characters\n", argv[1]);
      exit(1);
      return 0;
    }
  }

  fclose(text_file);

  // Socket setup!
  int socket_file_descriptor; // listen to this
  int port_number;
  int number_of_characters;
  struct sockaddr_in server_address;
  struct hostent *server;

  // creat new socket
  socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
  // set the port number from command line arguments
  port_number = atoi(argv[3]);
  server = gethostbyname("localhost");
  // set server_address fields
  bzero((char *) &server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;

  // bcopy used to move the char
  bcopy((char *) server->h_addr, (char *) &server_address.sin_addr.s_addr, server->h_length);
  server_address.sin_port = htons(port_number);

  // connect completely
  connect(socket_file_descriptor, (struct sockaddr *) &server_address, sizeof(server_address));

  char type[2];
  bzero(type, 2);

  number_of_characters = recv(socket_file_descriptor, type, 2, 0);

  // encode server
  if(type[0] == 'e'){
    sendText(socket_file_descriptor, argv[1]);
    char confirm[2];
    bzero(confirm, 2);

    // wait to get confirmation
    number_of_characters = recv(socket_file_descriptor, confirm, 2, 0);

    if(confirm[0] == '1'){
      sendKey(socket_file_descriptor, argv[2]);

    char encrypted_message[bytes_in_text];
    bzero(encrypted_message, bytes_in_text);

    int receive_encrypted_message = recv(socket_file_descriptor, encrypted_message, bytes_in_text - 1, 0);
    sprintf(encrypted_message, "%s", encrypted_message);

    printf("%s", encrypted_message);
    printf("\n");

    }
    else{
      printf("ERROR: Failed to connect to otp_enc_d\n");
      exit(2);
    }
  }

  close(socket_file_descriptor);
  return 0;
}


/* FUNCTIONS */
void sendText(int socket_file_descriptor, char file_name[]){
  char buffer[10];
  // open the file
  FILE *text_file = fopen(file_name, "r");
  bzero(buffer, 10);
  int file_size;
  int bytes_sent;

  // read the file
  while((file_size = fread(buffer, sizeof(char), 1, text_file)) > 0){
    // send the file
    if((bytes_sent = send(socket_file_descriptor, buffer, file_size, 0)) < 0){
      break;
    }

    bzero(buffer, 10);
  }

  // chunks are 10 long, stop when the chunks are no longer 10 long
  if(bytes_sent == 10){
    send(socket_file_descriptor, "0", 1, 0);
  }

  fclose(text_file);
}

void sendKey(int socket_file_descriptor, char file_name[]){
  char buffer[10];
  FILE *text_file = fopen(file_name, "r");
  bzero(buffer, 10);
  int file_size;
  int bytes_sent;

  // read the file
  while((file_size = fread(buffer, sizeof(char), 10, text_file)) > 0){
    // file has been sent
    if((bytes_sent = send(socket_file_descriptor, buffer, file_size, 0)) < 0){
      break;
    }

    bzero(buffer, 10);
  }

  // chunks are 10 long, stop when the chunks are no longer 10 long
  if(bytes_sent == 10){
    send(socket_file_descriptor, "0", 1, 0);
  }

  fclose(text_file);
}
