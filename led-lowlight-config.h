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

static inline String
hmString(int h, int min)
{
	String rv = (h < 10) ? ("0" + String(h)) : String(h);
	rv += ":" + ((min < 10) ? ("0" + String(min)) : String(min));
	return rv;
}

// will be something like "hh%3Amm"
static inline void
htmlTime2hm(String s, int *h, int *min)
{
	*h = s.substring(0, 2).toInt();
	*min = s.substring(5, 7).toInt();
}

#endif /* include guard */
