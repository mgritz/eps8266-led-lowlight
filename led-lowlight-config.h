#ifndef _LED_LOWLIGHT_CONFIG_H_
#define _LED_LOWLIGHT_CONFIG_H_

#define LED_CONFIG_MAGIC '#'

typedef struct {
	char on_color[8];
	int time_from_min;
	int time_from_hr;
	int time_to_min;
	int time_to_hr;
	int turn_off_delay_s;
	int fader_speed;
} LedConfig;

#endif /* include guard */
