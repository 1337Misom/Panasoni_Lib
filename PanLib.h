#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <curl/curl.h>

#define AUDIO_BUFFER_SIZE 4096 * 10

#define UDPPORT 49700
#define MAXBUFLEN 640*360*2 // size of a 640x360 rgb565 image

#define WIDTH 640
#define HEIGHT 360

#define APPROXIMATE_START 30
#define APPROXIMATE_END 10

#define AUDIO_ID 0x61
#define JPEG_ID 0x21

#define MAXURLLENGTH 125

#define CURL_TIMEOUT 1000

typedef void (JPEG_DRAW_CALLBACK)(uint8_t* jpeg_frame, ssize_t jpeg_size);
typedef void (AUDIO_PLAY_CALLBACK)(uint8_t* audio_frame, ssize_t audio_size);

const char host_ip[] = "0.0.0.0";

const char getinfopatt[] = "http://%s/cam.cgi?mode=getinfo&type=capability";
const char startstreampatt[] = "http://%s/cam.cgi?mode=startstream&value=%d";
const char sustainstreampatt[] = "http://%s/cam.cgi?mode=getstate";
const char setrecmodepatt[] = "http://%s/cam.cgi?mode=camcmd&value=recmode";

const char base_cmd_url_patt[] = "http://%s/cam.cgi?mode=camcmd&value=%s";

#define JPEG_HEADER_LENGTH 2
const uint8_t jpeg_header[JPEG_HEADER_LENGTH] = {0xff,0xd8};
const uint8_t jpeg_endheader[JPEG_HEADER_LENGTH] = {0xff,0xd9};

class PanLib {
  JPEG_DRAW_CALLBACK *jpeg_callback;
  JPEG_DRAW_CALLBACK *audio_callback;

  sockaddr_in server;
  hostent *host;

  int UDPSocket;
  unsigned int addr_len;

  int socket_return_length;

  uint8_t* audio_buffer;
  uint8_t* jpeg_buffer;
  uint8_t* jpeg_image_start;
  uint8_t* jpeg_image_end;
  uint8_t* recv_buffer;

  uint8_t counter = 0;
  uint8_t packet_id;
  uint16_t packet_length;

  char startstream[MAXURLLENGTH];
  char getinfo[MAXURLLENGTH];
  char sustainstream[MAXURLLENGTH];
  char setrecmode[MAXURLLENGTH];

  char temp[MAXURLLENGTH];

  const char * camera;

  CURL *curl;
  CURLcode curl_response;
private:
  int send_cmd(const char * command);
public:
  int init(JPEG_DRAW_CALLBACK jpeg_func,AUDIO_PLAY_CALLBACK audio_func, const char* camera_ip);
  int decode_frame();
  int connect();
  int zoom_fast() { return send_cmd("tele-fast"); };
  int zoom_normal() { return send_cmd("tele-normal"); };
  int wide_fast() { return send_cmd("wide-fast"); };
  int wide_normal() { return send_cmd("wide-normal"); };
  int zoom_stop() { return send_cmd("zoomstop"); };
  int pict_mode() { return send_cmd("pictmode"); };
  int rec_mode() { return send_cmd("recmode"); };
  int take_pic() { return send_cmd("capture"); };
  int start_rec() { return send_cmd("video_recstart"); };
  int stop_rec() { return send_cmd("video_recstop"); };

};
