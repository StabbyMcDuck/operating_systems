/*****************************
  Author: Regina Imhoff
  Email:  imhoffr@oregonstate.edu
  Due:    06/06/2016
  OTP Decode - Client
  Resources:  http://linux.die.net/man/2/send
 *****************************/

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
    printf("Usage: %s CIPHER_TEXT_PATH KEY_PATH DECRYPTION_PORT\n", argv[0]);
    exit(1);
  }

  FILE *text = fopen(argv[1], "r");

  if (text == NULL) {
    perror("Cannot open CIPHER_TEXT_PATH: ");
    exit(2);
  }

  FILE *key = fopen(argv[2], "r");

  if (key == NULL) {
    perror("Cannot open KEY_PATH");
    exit(3);
  }

  // http://www.tutorialspoint.com/c_standard_library/c_function_fseek.htm
  // fseek(FILE *stream, int offset, int whence)
  fseek(text, 0, SEEK_END);
  fseek(key, 0, SEEK_END);

  int bytes_in_text = ftell(text);
  int bytes_in_key = ftell(key);

  fclose(text);
  fclose(key);

  // key must be longer than the text file
  if(bytes_in_key < bytes_in_text){
    printf("ERROR:  Key %s is too short\n", argv[2]);
    exit(1);
  }

  // create a new socket
  int socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);

  // set the port number from the command line arguments
  int port_number = atoi(argv[3]);

  struct sockaddr_in server_address;
  struct hostent *server = gethostbyname("localhost");

  // set up server_address
  bzero((char *) &server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;

  bcopy((char*) server->h_addr, (char *) &server_address.sin_addr.s_addr, server->h_length);
  server_address.sin_port = htons(port_number);

  // connect to socket
  connect(socket_file_descriptor, (struct sockaddr *) &server_address, sizeof(server_address));

  char socket_type[2];
  bzero(socket_type, 2);
  int n = recv(socket_file_descriptor, socket_type, 2, 0);

  if(socket_type[0] == 'd'){ // d is the decode server
    // send the text...
    sendText(socket_file_descriptor, argv[1]);

    char confirm[2];
    bzero(confirm, 2);

    // wait for the confirmation
    n = recv(socket_file_descriptor, confirm, 2, 0);

    // if the confirmation is received then the key gets sent
    if(confirm[0] == '1'){
      sendKey(socket_file_descriptor, argv[2]);
    }

    // decoded message gets sent back
    char decodedText[bytes_in_text];
    bzero(decodedText, bytes_in_text);

    int m = recv(socket_file_descriptor, decodedText, (bytes_in_text - 1), 0);

    sprintf(decodedText, "%s\n", decodedText);
    printf("%s\n", decodedText);
  }
  else{
    printf("ERROR: Could not connect to otp_dec_d\n");
    exit(2);
  }

  close(socket_file_descriptor);

  return 0;
}

/* FUNCTIONS */
// sendText sends the text file to the server
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
