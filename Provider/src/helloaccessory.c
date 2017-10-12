#include "helloaccessory.h"
#include <sensor.h>
#include <dlog.h>
#include <tts.h>
#include <device/haptic.h>
#include <device/power.h>
#include <efl_util.h>


typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *naviframe;
	Evas_Object *rect[10];
	Eext_Circle_Surface *circle_surface;
	Evas_Object *circle_genlist;
	Evas_Object *label;
	Evas_Object *label0;
	Evas_Object *label1;
	Evas_Object *label2;
	Evas_Object *label3;
	Evas_Object *label4;
	Evas_Object *check1;
	tts_h tts;
	haptic_device_h handle;
	haptic_effect_h effect_handle;
	Ecore_Timer *timer1;
	int timer_count;
} appdata_s;

static appdata_s *object;

typedef struct _sensor_info
{
sensor_h sensor; /**< Sensor handle */
sensor_listener_h sensor_listener;
} sensorinfo;


static sensorinfo sensor_info1;
static sensorinfo sensor_info2;
static sensorinfo sensor_info3;

static float value[3] = {0.f,0.f,0.f};
static float current_accel[3] = {0.f,0.f,0.f};
static float current_light = 0.f;
static float current_light_min = 80.f;
static float current_gyro[3] = {0.f,0.f,0.f};
static float current_gyro_max[3] = {0.f,0.f,0.f};
static int introFlag_Nicetomeetyou = 0;
static int mainFlag_Nicetomeetyou = 0;
static int introFlag_Sorry = 0;
static int mainFlag_Sorry = 0;
static Eina_Bool checkState = EINA_FALSE;
static char hello[6] = {"hello"};
static char sorry[6] = {"sorry"};
static char NtoM[18] = {"Nice to Meet you"};

static tts_h
create_tts_handle(appdata_s *ad)
{
	 tts_h tts;
	 tts_create(&tts);
	 tts_prepare(tts);
	 return tts;
}

static void
destroy_tts_handle(tts_h tts)
{
	 tts_destroy(tts); // tts is the TTS handle

}

static void
add_text(tts_h tts, appdata_s *ad,char* text)
{
	 //const char* text = "hello"; // Text for read
	 const char* language = "en_US"; // Language
	 int voice_type = TTS_VOICE_TYPE_FEMALE; // Voice type
	 int speed = TTS_SPEED_AUTO;
	 int utt_id; // Utterance ID for the requested text
	 int ret = tts_add_text(tts, text, language, voice_type, speed, &utt_id);

	 if (TTS_ERROR_NONE != ret)
	 {
		 dlog_print(DLOG_INFO, "tag", "%s err = %d", __func__, ret);
	 }
}

static int get_state(tts_h* tts)
{
	tts_state_e current_state;
	int ret;
	ret = tts_get_state(*tts, &current_state);

    if (TTS_ERROR_NONE != ret)
    {
    	dlog_print(DLOG_INFO, "tag", "%s state = %d", __func__, ret);
    	return -1;
    }
    else
    {
    	dlog_print(DLOG_INFO, "tag", "%s state = %d", __func__, current_state);
    	return (int) current_state;
    }

}


static float get_absolute_max(float value1,float value2)
{
	float v1 = value1 > 0.f ? value1 : -value1;
	float v2 = value2 > 0.f ? value2 : -value2;
	float result = v1 > v2 ? v1:v2;
	return result;
}
static float get_absolute_min(float value1,float value2)
{
	float v1 = value1 < 80.f ? value1 : -value1;
	float v2 = value2 < 80.f ? value2 : -value2;
	float result = v1 < v2 ? v1:v2;
	return result;
}


static void _new_sensor_value1(sensor_h sensor, sensor_event_s *sensor_data, void
*user_data)
{
	if( sensor_data->value_count < 3 )
		return;
	char buf[PATH_MAX];
	appdata_s *ad = (appdata_s*)user_data;

	current_accel[0] = sensor_data->values[0];
	current_accel[1] = sensor_data->values[1];
	current_accel[2] = sensor_data->values[2];

	sprintf(buf, " X : %0.1f / Y : %0.1f / Z : %0.1f",
	sensor_data->values[0], sensor_data->values[1], sensor_data->values[2]);
	elm_object_text_set(ad->label1, buf);
	dlog_print(DLOG_DEBUG, "Acceleration!!!",buf);

	int state = get_state(&ad->tts);
	if ((tts_state_e) state == TTS_STATE_READY || (tts_state_e) state == TTS_STATE_PAUSED)
	{
		//hello
		if(current_light_min<20.0){ //빛센서 미니멈 손목스칠때
			if(introFlag_Nicetomeetyou >= 0 || mainFlag_Nicetomeetyou >= 0){
				current_light_min==80.f;
				if(current_accel[2]>12.0&&current_gyro[1]>=150.0){ //손이 내려갈때 가속도센서 z값 12이상 gyro센서값 150이상
					if(current_accel[2]<10.0&&current_accel[2]>5.0){ //손이 멈출때
						device_haptic_stop(ad->handle, &ad->effect_handle);
						tts_stop(ad->tts);
						state = TTS_STATE_READY;
						introFlag_Sorry = 0; //값 초기화
						mainFlag_Sorry = 0;
						for(int i=0;i<3;i++){
							value[i]=0.f;
							current_accel[i] = 0.f;
							current_gyro[i] = 0.f;
						}
						current_light = 0.f;
						current_light_min = 80.f;
					}else{
						if(introFlag_Nicetomeetyou == 0 && mainFlag_Nicetomeetyou == 0){
							device_haptic_vibrate(ad->handle, 1000, 100, &ad->effect_handle);
							add_text(ad->tts, ad,"hello");
							tts_play(ad->tts);
							send_data(hello);
						}
					}
				}
			}
		}
		//sorry
		if(current_gyro[0]<-150&&current_gyro[1]<-150)
			introFlag_Sorry++;
		if(current_accel[0]>=8.5)
			introFlag_Sorry++;
		if(current_gyro[1]<-200)
			mainFlag_Sorry++;
		if(introFlag_Sorry>=1&&mainFlag_Sorry>0){
			current_light_min = 80.f;
			device_haptic_vibrate(ad->handle, 3000, 100, &ad->effect_handle);
			add_text(ad->tts, ad,"sorry");
			tts_play(ad->tts);
			send_data(sorry);
		}
		if(current_accel[2]<-8.5){
			device_haptic_stop(ad->handle, &ad->effect_handle);
			tts_stop(ad->tts);
			state = TTS_STATE_READY;
			introFlag_Sorry = 0;
			mainFlag_Sorry = 0;
			for(int i=0;i<3;i++){
				value[i]=0.f;
				current_accel[i] = 0.f;
				current_gyro[i] = 0.f;
			}
			current_light = 0.f;
			current_light_min = 80.f;
		}

		//nice to meet you
		if(current_gyro[1]==-150)
			introFlag_Nicetomeetyou++;
		if(current_accel[0] >=8.5)
			introFlag_Nicetomeetyou++;

		if(current_accel[1]<=-20)
			mainFlag_Nicetomeetyou++;
		if(current_gyro[2] >= 200)
			mainFlag_Nicetomeetyou++;


		if(introFlag_Nicetomeetyou>0&&mainFlag_Nicetomeetyou>2){
			current_light_min = 80.f;
			device_haptic_vibrate(ad->handle, 8000, 100, &ad->effect_handle);
			add_text(ad->tts, ad,"Nice to meet you");
			tts_play(ad->tts);
			send_data(NtoM);
		}
		if(mainFlag_Nicetomeetyou>=6){
			device_haptic_stop(ad->handle, &ad->effect_handle);
			tts_stop(ad->tts);
			state = TTS_STATE_READY;
			introFlag_Sorry = 0;
			mainFlag_Sorry = 0;
			for(int i=0;i<3;i++){
				value[i]=0.f;
				current_accel[i] = 0.f;
				current_gyro[i] = 0.f;
			}
			current_light = 0.f;
			current_light_min = 80.f;
			introFlag_Nicetomeetyou = 0;
			mainFlag_Nicetomeetyou = 0;
		}


	}
}

static void _new_sensor_value2(sensor_h sensor, sensor_event_s *sensor_data, void
*user_data)
{

	char buf[PATH_MAX];
	appdata_s *ad = (appdata_s*)user_data;
	current_light = sensor_data->values[0];
	current_light_min = get_absolute_min(current_light_min,sensor_data->values[0]);

	sprintf(buf, "Value - X : %0.1f",sensor_data->values[0]);
	elm_object_text_set(ad->label2, buf);
	dlog_print(DLOG_DEBUG, "Light!!!",buf);

}

static void _new_sensor_value3(sensor_h sensor, sensor_event_s *sensor_data, void
*user_data)
{
	if( sensor_data->value_count < 3 )
		return;
	char buf[PATH_MAX];
	appdata_s *ad = (appdata_s*)user_data;
	current_gyro[0] = sensor_data->values[0];
	current_gyro[1] = sensor_data->values[1];
	current_gyro[2] = sensor_data->values[2];
	sprintf(buf, "X : %0.1f / Y : %0.1f / Z : %0.1f",
	sensor_data->values[0], sensor_data->values[1],sensor_data->values[2]);
	elm_object_text_set(ad->label3, buf);
	dlog_print(DLOG_DEBUG, "GYROSCOPE!!!",buf);

	for(int i=0;i<4;i++)
		current_gyro_max[i] = get_absolute_max(current_gyro_max[i],sensor_data->values[i]);

	//sprintf(buf,"X:%0.1f/Y:%0.1f/Z:%0.1f ",value[0],value[1],value[2]);
	//elm_object_text_set(ad->label2,buf);


}


static void
start_acceleration_sensor(appdata_s *ad)
{
	sensor_get_default_sensor(SENSOR_ACCELEROMETER, &sensor_info1.sensor);
	sensor_create_listener(sensor_info1.sensor, &sensor_info1.sensor_listener);
	sensor_listener_set_event_cb(sensor_info1.sensor_listener, 100, _new_sensor_value1, ad);
	sensor_listener_start(sensor_info1.sensor_listener);
}

static void
start_light_sensor(appdata_s *ad)
{
	sensor_get_default_sensor(SENSOR_LIGHT, &sensor_info2.sensor);
	sensor_create_listener(sensor_info2.sensor, &sensor_info2.sensor_listener);
	sensor_listener_set_event_cb(sensor_info2.sensor_listener, 500, _new_sensor_value2, ad);
	sensor_listener_start(sensor_info2.sensor_listener);
	device_power_request_lock(POWER_LOCK_DISPLAY, 0);
}

static void
start_gyroscope_sensor(appdata_s *ad)
{
	sensor_get_default_sensor(SENSOR_GYROSCOPE, &sensor_info3.sensor);
	sensor_create_listener(sensor_info3.sensor, &sensor_info3.sensor_listener);
	sensor_listener_set_event_cb(sensor_info3.sensor_listener, 100, _new_sensor_value3, ad);
	sensor_listener_start(sensor_info3.sensor_listener);
}

static void
check_changed_cb(void *data,Evas_Object *obj,void *event_info)
{
	appdata_s *ad = data;
	checkState = elm_check_state_get(obj);
	if(checkState == EINA_TRUE){
		start_acceleration_sensor(ad);
		start_light_sensor(ad);
		start_gyroscope_sensor(ad);
		device_haptic_open(0, &ad->handle);
		ad->tts = create_tts_handle(ad);
	}else{
		sensor_listener_stop(sensor_info1.sensor_listener);
		sensor_listener_stop(sensor_info2.sensor_listener);
		sensor_listener_stop(sensor_info3.sensor_listener);
		device_haptic_stop(ad->handle, &ad->effect_handle);
		destroy_tts_handle(ad->tts);
		for(int i=0;i<3;i++){
				value[i]=0.f;
				current_accel[i] = 0.f;
				current_gyro[i] = 0.f;
		}
		current_light = 0.f;
		current_light_min = 80.f;
		introFlag_Sorry = 0;
		mainFlag_Sorry = 0;
		introFlag_Nicetomeetyou = 0;
		mainFlag_Nicetomeetyou = 0;
	}

}


static void win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void _timeout_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!obj) return;
	elm_popup_dismiss(obj);
}

static void _block_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!obj) return;
	elm_popup_dismiss(obj);
}

static void _popup_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!obj) return;
	elm_popup_dismiss(obj);
}
static void _popup_hide_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!obj) return;
	evas_object_del(obj);
}

static void _popup_toast_cb(Evas_Object *parent, char *string)
{
	Evas_Object *popup;


	popup = elm_popup_add(parent);
	elm_object_style_set(popup, "toast/circle");
	elm_popup_orient_set(popup, ELM_POPUP_ORIENT_BOTTOM);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, _popup_hide_cb, NULL);
	evas_object_smart_callback_add(popup, "dismissed", _popup_hide_finished_cb, NULL);
	elm_object_part_text_set(popup, "elm.text", string);

	evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb, NULL);

	elm_popup_timeout_set(popup, 2.0);

	evas_object_smart_callback_add(popup, "timeout", _timeout_cb, NULL);

	evas_object_show(popup);
}

void update_ui(char *data)
{
	//dlog_print(DLOG_INFO, TAG, "Updating UI with data %s", data);
	_popup_toast_cb(object->naviframe, data);
}


typedef struct _item_data {
	int index;
	Elm_Object_Item *item;
} item_data;
/*
static char *_gl_title_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];

	snprintf(buf, 1023, "%s", "HelloAccessory");

	return strdup(buf);
}

static char *_gl_sub_title_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];

	snprintf(buf, 1023, "%s", "Provider");

	return strdup(buf);
}


static void _gl_del(void *data, Evas_Object *obj)
{
	// FIXME: Unrealized callback can be called after this.
	// Accessing Item_Data can be dangerous on unrealized callback.
	item_data *id = data;
	if (id) free(id);
}
*/
static Eina_Bool _naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	ui_app_exit();
	return EINA_FALSE;
}

static void create_base_gui(appdata_s *ad)
{
	Evas_Object *conform = NULL;

	/* Window */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);

	/* Conformant */
	conform = elm_conformant_add(ad->win);
	evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, conform);
	evas_object_show(conform);

	/* Naviframe */
	ad->naviframe = elm_naviframe_add(conform);
	elm_object_content_set(conform, ad->naviframe);

	/* Eext Circle Surface*/
	//ad->circle_surface = eext_circle_surface_naviframe_add(ad->naviframe);

	/* Main View */
	Evas_Object *box = elm_box_add(ad->naviframe);
	//elm_box_padding_set(box,ELM_SCALE_SIZE(100),ELM_SCALE_SIZE(60));
	evas_object_size_hint_weight_set(box,0.0,0.0);
	evas_object_size_hint_align_set(box,0.5,0.5);
	elm_object_content_set(ad->conform,box);
	evas_object_show(box);
		{

			ad->label1= elm_label_add(box);
			elm_object_text_set(ad->label1, "A-");
			evas_object_size_hint_weight_set(ad->label1,0.0,0.0);
			evas_object_size_hint_align_set(ad->label1,0.5,0.5);
			elm_box_pack_end(box,ad->label1);
			evas_object_show(ad->label1);

/*
			Evas_Object *btn = elm_button_add(ad->conform);
			elm_object_text_set(btn,"Init Max Value");
			evas_object_smart_callback_add(btn,"clicked",btn_clicked,ad);
			evas_object_size_hint_weight_set(btn,0.0,0.0);
			evas_object_size_hint_align_set(btn,0.5,0.5);
			elm_box_pack_end(box,btn);
			evas_object_show(btn);
*/

			ad->check1 = elm_check_add(box);
			elm_object_style_set(ad->check1,"on&off");
			elm_check_state_set(ad->check1,EINA_FALSE);
			evas_object_size_hint_weight_set(ad->check1,0.0,0.0);
			evas_object_size_hint_align_set(ad->check1,0.5,0.5);
			evas_object_move(ad->check1,250,160);
			evas_object_smart_callback_add(ad->check1,"changed",check_changed_cb,ad);
			elm_box_pack_end(box,ad->check1);
			evas_object_show(ad->check1);

			ad->label2 = elm_label_add(box);
			elm_object_text_set(ad->label2, "L - ");
			evas_object_size_hint_weight_set(ad->label2,0.0,0.0);
			evas_object_size_hint_align_set(ad->label2,0.5,0.5);
			elm_box_pack_end(box,ad->label2);
			evas_object_show(ad->label2);

			ad->label3 = elm_label_add(box);
			elm_object_text_set(ad->label3, "G - ");
			evas_object_size_hint_weight_set(ad->label3,0.0,0.0);
			evas_object_size_hint_align_set(ad->label3,0.5,0.5);
			elm_box_pack_end(box,ad->label3);
			evas_object_show(ad->label3);



		}


	/* Show window after base gui is set up */
	//create_list_view(ad);

	eext_object_event_callback_add(ad->naviframe, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
	eext_object_event_callback_add(ad->naviframe, EEXT_CALLBACK_MORE, eext_naviframe_more_cb, NULL);

	/* Show window after base gui is set up */
	evas_object_show(ad->win);
}

static bool app_create(void *data)
{
	/* Hook to take necessary actions before main event loop starts
	   Initialize UI resources and application's data
	   If this function returns true, the main loop of application starts
	   If this function returns false, the application is terminated */
	object = data;
	create_base_gui(object);
	initialize_sap();

	return TRUE;
}

static void app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void app_pause(void *data)
{
	device_power_release_lock(POWER_LOCK_DISPLAY);
	/* Take necessary actions when application becomes invisible. */
}

static void app_resume(void *data)
{
	device_power_request_lock(POWER_LOCK_DISPLAY, 0);
	/* Take necessary actions when application becomes visible. */
}

static void app_terminate(void *data)
{
	/* Release all resources. */
}

static void ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int main(int argc, char *argv[])
{
	appdata_s ad = { 0, };
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = { 0, };
	app_event_handler_h handlers[5] = { NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);
	ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, TAG, "ui_app_main() is failed. err = %d", ret);
	}

	return ret;
}
