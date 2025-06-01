#include "esp_camera.h"
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>

#define CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// TaskHandle_t Task1;

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
TFT_eSprite spr = TFT_eSprite(&tft);

camera_config_t config;

uint16_t *scr;
long initalTime = 0;
long frameTime = 1;
volatile bool screenRefreshFlag = true;

int log_line = 30;
const int LOG_X_START = 40;
const int LOG_Y_INCREMENT = 10;

void debugLog(String text) {
  tft.drawString(text, LOG_X_START, log_line, 1);
  log_line += LOG_Y_INCREMENT;
}

void setupCamera() {
  //core0 setup
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_240X240;//FRAMESIZE_UXGA;
  // config.pixel_format = PIXFORMAT_JPEG;  // for streaming
  config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  if (psramFound()) {
    Serial.println("PSRAM found!");
    config.fb_location = CAMERA_FB_IN_PSRAM;
  } else {
    Serial.println("PSRAM NOT found!");
    config.fb_location = CAMERA_FB_IN_DRAM;
  }
  config.jpeg_quality = 12;
  config.fb_count = 1;
  // config.frame_size = FRAMESIZE_240X240;
  // config.pixel_format = PIXFORMAT_RGB565;
  // config.grab_mode = CAMERA_GRAB_LATEST;    //option CAMERA_GRAB_WHEN_EMPTY
  // // config.fb_location = CAMERA_FB_IN_PSRAM;
  // config.jpeg_quality = 12;
  // config.fb_count = 2;                          //need more than 1 for latest grab

    // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
//   if (config.pixel_format == PIXFORMAT_JPEG) {
//     if (psramFound()) {
//       config.jpeg_quality = 10;
//       config.fb_count = 2;
//       config.grab_mode = CAMERA_GRAB_LATEST;
//     } else {
//       // Limit the frame size when PSRAM is not available
//       config.frame_size = FRAMESIZE_SVGA;
//       config.fb_location = CAMERA_FB_IN_DRAM;
//     }
//   } else {
//     // Best option for face detection/recognition
//     config.frame_size = FRAMESIZE_240X240;
// #if CONFIG_IDF_TARGET_ESP32S3
//     config.fb_count = 2;
// #endif
//   }

  Serial.println("Init camera");

  esp_err_t err = esp_camera_init(&config);
  if (err != 0) {
    // tft.drawString("Error!!!", 105, 105, 1);
    debugLog("Cam init Error!!!");
  } else {
    // tft.drawString("Init done...", 105, 105, 1);
    debugLog("Cam init OK");
  }

  Serial.println("Config camera sensor");
  sensor_t * s = esp_camera_sensor_get();
  s->set_brightness(s, 0);     // -2 to 2
  s->set_contrast(s, 0);       // -2 to 2
  s->set_saturation(s, 0);     // -2 to 2
  s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
  s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
  s->set_aec2(s, 0);           // 0 = disable , 1 = enable
  s->set_ae_level(s, 0);       // -2 to 2
  s->set_aec_value(s, 300);    // 0 to 1200
  s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  s->set_agc_gain(s, 0);       // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
  s->set_bpc(s, 0);            // 0 = disable , 1 = enable
  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
  s->set_vflip(s, 0);          // 0 = disable , 1 = enable
  s->set_dcw(s, 1);            // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);       // 0 = disable , 1 = enable

  Serial.println("Camera setup finished");
  // tft.drawString("Done :3", 105, 105, 1);
  debugLog("Cam setup done");
}

/////////////////////////////////
// void Task1code( void * pvParameters ) {

//   Serial.println("Start core loop");
//   //core0 loop
//   for (;;) {
//     //take picture
//     camera_fb_t  * fb = NULL;
//     fb = esp_camera_fb_get();
//     //transfer frame buffer data to pointer
//     for (size_t i = 0; i < 57600; i++) {    //240x240px = 57600
//       byte first_byte = fb->buf[i * 2];
//       byte second_byte = fb->buf[i * 2 + 1];
//       scr[i] = (second_byte << 8) + first_byte;
//     }
//     screenRefreshFlag = true;
//     esp_camera_fb_return(fb);   //return the frame buffer back to the driver for reuse
//   }
// }

//////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  Serial.println("Begin");

  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  scr = (uint16_t*)spr.createSprite(240, 240);
  // tft.drawString("Loading...", 105, 105, 1);
  debugLog("Starting...");
  
  setupCamera();

  // xTaskCreatePinnedToCore(
  //   Task1code,   // Task function.
  //   "Task1",     // name of task.
  //   100000,      // Stack size of task
  //   NULL,        // parameter of the task
  //   1,           // priority of the task
  //   &Task1,      // Task handle to keep track of created task
  //   0);          // pin task to core 0

  delay(1000); 
  Serial.println("Setup done, entering loop");
}

//////////////////////////////////
void loop() {

    //take picture
    camera_fb_t  * fb = NULL;
    fb = esp_camera_fb_get();
    //transfer frame buffer data to pointer
    for (size_t i = 0; i < 57600; i++) {    //240x240px = 57600
      byte first_byte = fb->buf[i * 2];
      byte second_byte = fb->buf[i * 2 + 1];
      scr[i] = (second_byte << 8) + first_byte;
    }
    // screenRefreshFlag = true;
    esp_camera_fb_return(fb);   //return the frame buffer back to the driver for reuse

    initalTime = millis();
    spr.drawString(String(frameTime), 105, 220, 1); //print frame time in milliseconds
    spr.drawString("ms", 125, 220, 1);
    spr.drawString(String(1000.0/frameTime), 85, 210, 1); //print frame time in milliseconds
    spr.drawString("FPS", 125, 210, 1);
    spr.pushSprite(0, 0);
    // screenRefreshFlag = false;
    frameTime = millis() - initalTime;

  //refresh display if there is a new image from the camera
  // if (screenRefreshFlag) {
  //   initalTime = millis();
  //   spr.drawString(String(frameTime), 100, 220, 2); //print frame time in milliseconds
  //   spr.drawString("ms", 125, 220, 2);
  //   spr.pushSprite(0, 0);
  //   screenRefreshFlag = false;
  //   frameTime = millis() - initalTime;
  // }
}