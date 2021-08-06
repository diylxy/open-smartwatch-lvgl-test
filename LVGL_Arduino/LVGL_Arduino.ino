#include <lvgl.h>
#include <lv_examples.h>
#include "Arduino_ESP32SPI.h"
#include "Arduino_GFX.h"     // Core graphics library
#include "Arduino_Display.h" // Various display driver

Arduino_ESP32SPI *bus = new Arduino_ESP32SPI(12, 5, 18, 23, -1);
Arduino_GC9A01 *gfx = new Arduino_GC9A01(bus, 33, 0 /* rotation */, true /* IPS */);

static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

#if USE_LV_LOG != 0
/* Serial debugging */
void my_print(lv_log_level_t level, const char * file, uint32_t line, const char * dsc)
{

    Serial.printf("%s@%d->%s\r\n", file, line, dsc);
    Serial.flush();
}
#endif

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    gfx->startWrite();
    gfx->writeAddrWindow(area->x1, area->y1, w, h);
    gfx->writePixels(&color_p->full, w * h);
    gfx->endWrite();
    lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
bool my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data)
{
    data->state = LV_INDEV_STATE_REL;
    return false; /*Return `false` because we are not buffering and no more data to read*/
}

void setup()
{
    Serial.begin(115200); /* prepare for possible serial debug */

    lv_init();

#if USE_LV_LOG != 0
    lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

    gfx->begin();
    ledcAttachPin(9, 1);
    ledcSetup(1, 12000, 8); // 12 kHz PWM, 8-bit resolution
    ledcWrite(1, 120);//亮度，0-255
    gfx->displayOn();
    
    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

    /*Initialize the display*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 240;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the (dummy) input device driver*/
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

		/* Try an example from the lv_examples repository
		 * https://github.com/lvgl/lv_examples */
		lv_demo_widgets();
}


void loop()
{

    lv_task_handler(); /* let the GUI do its work */
    delay(2);
}
