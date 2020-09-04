#include "main.h"

WebServer server(80);

// ===== rtos task handles =========================
// Streaming is implemented with 3 tasks:
TaskHandle_t tMjpeg;   // handles client connections to the webserver
TaskHandle_t tCam;     // handles getting picture frames from the camera and storing them locally

uint8_t       noActiveClients;       // number of active clients

// frameSync semaphore is used to prevent streaming buffer as it is replaced with the next frame
SemaphoreHandle_t frameSync = NULL;

class BlinkerTask : public Task {
public:
  BlinkerTask(uint8_t pin, bool level) : Task("BlinkerTask", 1024), _blinker(NULL), _pin(pin), _level(level) {}
  void Demo();

protected:
  void setup();
  void loop();
  void cleanup();

  struct __attribute__((__packed__)) {
    Blinker *_blinker;
    uint8_t _pin : 7;
    bool _level : 1;
  };
};

void BlinkerTask::setup() {
  if (_task) {
    _blinker = new Blinker(_pin, _level);
    if ((! _blinker) || (! *_blinker)) {
      if (_blinker) {
        delete _blinker;
        _blinker= NULL;
      }
      destroy();
    }
  }
  *_blinker = Blinker::BLINK_PWM; // фиксированный уровень
  *_blinker<<16;
}

void BlinkerTask::Demo()
{
  const char *BLINKS[] = { "OFF", "ON", "TOGGLE", "0.5 Hz", "1 Hz", "2 Hz", "4 Hz", "FADE IN", "FADE OUT", "FADE IN/OUT", "PWM" };

  Blinker::blinkmode_t mode = _blinker->getMode();

  if (mode < Blinker::BLINK_PWM)
    mode = (Blinker::blinkmode_t)((uint8_t)mode + 1);
  else
    mode = Blinker::BLINK_OFF;
  *_blinker = mode;
  lock();
  Serial.print("Blinker switch to ");
  Serial.println(BLINKS[mode]);
  unlock();
}

void BlinkerTask::loop() {
  // *_blinker = Blinker::BLINK_PWM; // фиксированный уровень
  // Demo();
  vTaskDelay(pdMS_TO_TICKS(5000));
}

void BlinkerTask::cleanup() {
  if (_blinker) {
    delete _blinker;
    _blinker = NULL;
  }
}

Task *task = NULL;

static void halt(const char *msg) {
  if (task)
    delete task;
  Serial.println(msg);
  Serial.flush();
  esp_deep_sleep_start();
}

// ==== SETUP method ==================================================================
void setup()
{

  // Setup Serial connection:
  Serial.begin(115200);
  delay(1000); // wait for a second to let Serial connect
  Serial.printf("setup: free heap  : %d\n", ESP.getFreeHeap());

#if defined(CAMERA_MODEL_AI_THINKER)
  // // включу светодиод
  // pinMode(GPIO_NUM_4, OUTPUT);
  // digitalWrite(GPIO_NUM_4, HIGH);
  task = new BlinkerTask(LED_PIN, LED_LEVEL);
  if ((! task) || (! *task))
    halt("Error initializing blinker task!");
#endif

  // Configure the camera
  //  camera_config_t config;
  //  config.ledc_channel = LEDC_CHANNEL_0;
  //  config.ledc_timer = LEDC_TIMER_0;
  //  config.pin_d0 = Y2_GPIO_NUM;
  //  config.pin_d1 = Y3_GPIO_NUM;
  //  config.pin_d2 = Y4_GPIO_NUM;
  //  config.pin_d3 = Y5_GPIO_NUM;
  //  config.pin_d4 = Y6_GPIO_NUM;
  //  config.pin_d5 = Y7_GPIO_NUM;
  //  config.pin_d6 = Y8_GPIO_NUM;
  //  config.pin_d7 = Y9_GPIO_NUM;
  //  config.pin_xclk = XCLK_GPIO_NUM;
  //  config.pin_pclk = PCLK_GPIO_NUM;
  //  config.pin_vsync = VSYNC_GPIO_NUM;
  //  config.pin_href = HREF_GPIO_NUM;
  //  config.pin_sscb_sda = SIOD_GPIO_NUM;
  //  config.pin_sscb_scl = SIOC_GPIO_NUM;
  //  config.pin_pwdn = PWDN_GPIO_NUM;
  //  config.pin_reset = RESET_GPIO_NUM;
  //  config.xclk_freq_hz = 20000000;
  //  config.pixel_format = PIXFORMAT_JPEG;
  //
  //  // Frame parameters: pick one
  //  //  config.frame_size = FRAMESIZE_UXGA;
  //  //  config.frame_size = FRAMESIZE_SVGA;
  //  //  config.frame_size = FRAMESIZE_QVGA;
  //  config.frame_size = FRAMESIZE_VGA;
  //  config.jpeg_quality = 12;
  //  config.fb_count = 2;

  static camera_config_t camera_config = {
    .pin_pwdn       = PWDN_GPIO_NUM,
    .pin_reset      = RESET_GPIO_NUM,
    .pin_xclk       = XCLK_GPIO_NUM,
    .pin_sscb_sda   = SIOD_GPIO_NUM,
    .pin_sscb_scl   = SIOC_GPIO_NUM,
    .pin_d7         = Y9_GPIO_NUM,
    .pin_d6         = Y8_GPIO_NUM,
    .pin_d5         = Y7_GPIO_NUM,
    .pin_d4         = Y6_GPIO_NUM,
    .pin_d3         = Y5_GPIO_NUM,
    .pin_d2         = Y4_GPIO_NUM,
    .pin_d1         = Y3_GPIO_NUM,
    .pin_d0         = Y2_GPIO_NUM,
    .pin_vsync      = VSYNC_GPIO_NUM,
    .pin_href       = HREF_GPIO_NUM,
    .pin_pclk       = PCLK_GPIO_NUM,

    .xclk_freq_hz   = 20000000,
    .ledc_timer     = LEDC_TIMER_0,
    .ledc_channel   = LEDC_CHANNEL_0,
    .pixel_format   = PIXFORMAT_JPEG,
    /*
        FRAMESIZE_96X96,    // 96x96
        FRAMESIZE_QQVGA,    // 160x120
        FRAMESIZE_QCIF,     // 176x144
        FRAMESIZE_HQVGA,    // 240x176
        FRAMESIZE_240X240,  // 240x240
        FRAMESIZE_QVGA,     // 320x240
        FRAMESIZE_CIF,      // 400x296
        FRAMESIZE_HVGA,     // 480x320
        FRAMESIZE_VGA,      // 640x480
        FRAMESIZE_SVGA,     // 800x600
        FRAMESIZE_XGA,      // 1024x768
        FRAMESIZE_HD,       // 1280x720
        FRAMESIZE_SXGA,     // 1280x1024
        FRAMESIZE_UXGA,     // 1600x1200
    */
    //    .frame_size     = FRAMESIZE_QVGA,
    //    .frame_size     = FRAMESIZE_UXGA,
    //    .frame_size     = FRAMESIZE_SVGA,
    //    .frame_size     = FRAMESIZE_VGA,
    .frame_size     = FRAMESIZE_UXGA,
    .jpeg_quality   = 16,
    .fb_count       = 2
  };

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  if (esp_camera_init(&camera_config) != ESP_OK) {
    Serial.println("Error initializing the camera");
    delay(10000);
    ESP.restart();
  }


  //  Configure and connect to WiFi
  IPAddress ip;

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID1, PWD1);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(F("."));
  }
  ip = WiFi.localIP();
  Serial.println(F("WiFi connected"));
  Serial.println("");
  Serial.print("Stream Link: http://");
  Serial.print(ip);
  Serial.println("/mjpeg/1");


  // Start mainstreaming RTOS task
  xTaskCreatePinnedToCore(
    mjpegCB,
    "mjpeg",
    2*1024,
    NULL,
    2,
    &tMjpeg,
    APP_CPU);

  Serial.printf("setup complete: free heap  : %d\n", ESP.getFreeHeap());
}

void loop() {
  // this seems to be necessary to let IDLE task run and do GC
  vTaskDelay(10000);
  // // переключу светодиод
  // digitalWrite(GPIO_NUM_4, !digitalRead(GPIO_NUM_4));
}
