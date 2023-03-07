#include "PanLib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <curl/curl.h>
#include <string.h>

// Just for removing the print of libcurl
size_t write_data (void *buffer, size_t size, size_t nmemb, void *userp) {
        return size * nmemb;
}

int PanLib::init(JPEG_DRAW_CALLBACK jpeg_func,AUDIO_PLAY_CALLBACK audio_func, const char* camera_ip){
  jpeg_callback = jpeg_func;
  audio_callback = audio_func;

  // Set cgi-bin sites
  snprintf(sustainstream, MAXURLLENGTH, sustainstreampatt, camera_ip);
  snprintf(getinfo, MAXURLLENGTH, getinfopatt, camera_ip);
  snprintf(setrecmode, MAXURLLENGTH, setrecmodepatt, camera_ip);
  snprintf(startstream, MAXURLLENGTH, startstreampatt, camera_ip, UDPPORT);

  curl = curl_easy_init();

  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, write_data);
  curl_easy_setopt (curl, CURLOPT_TIMEOUT, CURL_TIMEOUT);
  curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt (curl, CURLOPT_VERBOSE, 0L);

  recv_buffer = (uint8_t*) malloc(MAXBUFLEN);

  camera = camera_ip;

  return 0;
}

int PanLib::connect(){

  if ((UDPSocket = socket (AF_INET, SOCK_DGRAM, 0)) == -1){
    perror ("socket");
    return 1;
  }

  server.sin_family = AF_INET;
  server.sin_port = htons(UDPPORT);
  server.sin_addr.s_addr = inet_addr(host_ip);

  bzero (&(server.sin_zero), 8);

  addr_len = sizeof (server);
  curl_response = curl_easy_setopt (curl, CURLOPT_URL, getinfo);
  curl_response = curl_easy_perform (curl);
  if (curl_response != CURLE_OK)
  {
          printf ("curl_easy_perform() failed: %s\n", curl_easy_strerror (curl_response));
          return 1;
  }

  curl_response = curl_easy_setopt (curl, CURLOPT_URL, startstream);
  curl_response = curl_easy_perform (curl);

  if (curl_response != CURLE_OK)
  {
          printf ("curl_easy_perform() failed: %s\n", curl_easy_strerror (curl_response));
          return 1;
  }

  if (bind(UDPSocket, (struct sockaddr *) &server, addr_len) == -1){
    perror ("Bind");
    return 1;
  }

  curl_easy_setopt (curl, CURLOPT_URL, getinfo);

  curl_response = curl_easy_perform(curl);
  if (curl_response != CURLE_OK) {
    printf("curl_easy_perform() failed: %s\n", curl_easy_strerror (curl_response));
    return 1;
  }

  return 0;
}

int PanLib::decode_frame(){
  if ((socket_return_length = recvfrom (UDPSocket, recv_buffer, MAXBUFLEN, MSG_WAITALL,(struct sockaddr *) &server, &addr_len)) == -1) {
    perror("recvfrom");
    return 1;
  }
  if(counter == 0){
    curl_easy_setopt (curl, CURLOPT_URL, sustainstream);
    curl_response = curl_easy_perform (curl);
    counter = 0;
    if (curl_response != CURLE_OK){
      printf ("curl_easy_perform() failed: %s\n",curl_easy_strerror (curl_response));
      return 1;
    }
  }

  counter++;
  packet_length = recv_buffer[0] << 8 | recv_buffer[1];
  packet_id = recv_buffer[16];

  if (packet_id == AUDIO_ID){
    audio_buffer = recv_buffer + 54;
    audio_callback(audio_buffer,socket_return_length - 54);
  } else if (packet_id == JPEG_ID) {
    // Scan for 0xffd8 (Jpeg start)
    jpeg_image_start = (uint8_t*) memmem(recv_buffer+APPROXIMATE_START, socket_return_length-APPROXIMATE_START,jpeg_header, JPEG_HEADER_LENGTH);
    jpeg_image_end = (uint8_t*) memmem(recv_buffer+(socket_return_length-APPROXIMATE_END), socket_return_length-(socket_return_length-APPROXIMATE_END),jpeg_endheader, JPEG_HEADER_LENGTH);
    jpeg_callback(jpeg_image_start,jpeg_image_end-(jpeg_image_start-4));
  }

  return 0;
}

int PanLib::send_cmd(const char* command){

  sprintf(temp,base_cmd_url_patt,camera,command);

  curl_easy_setopt (curl, CURLOPT_URL, sustainstream);
  curl_response = curl_easy_perform (curl);
  if (curl_response != CURLE_OK){
    printf ("curl_easy_perform() failed: %s\n",curl_easy_strerror (curl_response));
    return 1;
  }
  return 0;
}
