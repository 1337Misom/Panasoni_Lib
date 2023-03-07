# Panasonic Lib
This is a simple C++ Library for controlling and playing video of certain Panasonic camcorders.

## Dependencies
Install on ubuntu:
`sudo apt install libcurl-dev`

## Compilling your programm
PanLib first has to be compiled into a object file: `g++ PanLib.cpp -c -lcurl -o PanLib.o`
Now you can compile your programm: `g++ example.c++ PanLib.o -o example`

## How to use
1. You first have to create a video and audio callback function e.g.
```
 void jpeg_callback(uint8_t* jpeg_data,ssize_t jpeg_size){
  // Do something with the jpeg
 }

 void audio_callback(uint8_t* audio_data,ssize_t audio_size){
   // Do something with the audio
  }
```
#### !!!! Be careful the audio is g711 encoded.
2. In you main function you have to create a PanLib object and initalize it e.g.
  ```
    PanLib cam;
    cam.init(jpeg_callback,audio_callback,<The ip of the camera>);
  ```
3. You also have to call `connect()` and a short time later `decode_frame()` repeatedly. Keep in mind that `decode_frame()` has to be called at least every 10 seconds or the camera will stop streaming and you will have to call `connect()` again.
4. Enjoy

## The functions
- `init(JPEG_DRAW_CALLBACK jpeg_func,AUDIO_PLAY_CALLBACK audio_func, const char* camera_ip)` creates all the necessary buffers etc.
- `connect()` tells the camera to start providing a stream.
- `decode_frame()` waits until the next frame and calls the appropriate callback.
- `zoom_fast()` zooms the camera in fast
- `zoom_normal()` zooms the camera in 'normal'
- `wide_fast()` goes fast wide again
- `wide_normal()` goes normal wide again
- `zoom_stop()` stops zooming in or out
- `take_pic()` takes a picture
- `rec_mode()` goes into recording mode
- `pict_mode()` goes into picture mode
- `start_rec()` starts a recording
- `stop_rec()` stops a recording
