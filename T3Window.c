/*******************************************************************************
 * T3 Keyboard v1.0
 *
 * Copyright 2014 Chris Nucci (t3@fourbyte.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. 
 ******************************************************************************/

#include "T3Window.h"

const char T3_LAYOUT_LOWERCASE[] =
	"abc\0"  "def\0"  "ghi\0"
	"jkl\0"  "mno\0"  "pqr\0"
	"stu\0"  "vwx\0"  "yz ";
const char T3_LAYOUT_UPPERCASE[] =
	"ABC\0"  "DEF\0"  "GHI\0"
	"JKL\0"  "MNO\0"  "PQR\0"
	"STU\0"  "VWX\0"  "YZ ";
const char T3_LAYOUT_NUMBERS[] =
	"\x30\x31\0\0"  "\x32\0\0\0"  "\x33\0\0\0"
	"\x34\0\0\0"    "\x35\0\0\0"  "\x36\0\0\0"
	"\x37\0\0\0"    "\x38\0\0\0"  "\x39\0\0";
const char T3_LAYOUT_PUNC[] =
	".\0\0\0"  "'!\0\0"   ":;\"\0"
	",\0\0\0"  "-\0\0\0"  "@$#\0"
	"?\0\0\0"  "&%\0\0"   "+*=";
const char T3_LAYOUT_BRACKETS[] =
	"()\0\0"   "<>\0\0"    "{}\0\0"
	"/\0\0\0"  "\\\0\0\0"  "[]\0\0"
	"|_\0\0"   "~^`\0"     "¢½\0";
const uint8_t T3_MAXLENGTH = 24;
const bool T3_LOGGING = false;

const uint8_t _T3_KEYBOARD_SIZE = 9 * 4;
const uint8_t _T3_X_OFFSET = 8;
const uint8_t _T3_Y_OFFSET = 24;
const uint8_t _T3_X_SPACING = 44;
const uint8_t _T3_Y_SPACING = 40;
const uint8_t _T3_BUTTON_WIDTH = 38;
const uint8_t _T3_BUTTON_HEIGHT = 24;
const uint32_t _T3_MODE_TIMEOUT_IN_MS = 600;

typedef struct _t3_T3Window {
	Window * window;
	const char ** keyboardSets[3];
	uint8_t keyboardCounts[3];
	T3CloseHandler closeHandler;
	uint8_t set;
	uint8_t kb;
	uint8_t row;
	uint8_t col;
	char singleChars[3][2];
	TextLayer * buttons[9];
	TextLayer * inputLayer;
	InverterLayer * inverter;
	char * inputString;
	uint8_t inputLength;
	bool inverterShowing;
	bool selectionMode;
	AppTimer * timer;
} T3Window;

bool _t3_validateKeyboard(const char * keyboard);
void _t3_clickConfigProvider(void * context);
void _t3_back_click(ClickRecognizerRef recognizer, void * context);
void _t3_backspace_click(ClickRecognizerRef recognizer, void * context);
void _t3_r1_longclick(ClickRecognizerRef recognizer, void * context);
void _t3_r2_longclick(ClickRecognizerRef recognizer, void * context);
void _t3_r3_longclick(ClickRecognizerRef recognizer, void * context);
void _t3_r1_click(ClickRecognizerRef recognizer, void * context);
void _t3_r2_click(ClickRecognizerRef recognizer, void * context);
void _t3_r3_click(ClickRecognizerRef recognizer, void * context);
void _t3_longclick(T3Window * window, uint8_t button);
void _t3_click(T3Window * window, uint8_t row);
void _t3_timerCallback(void * context);
void _t3_updateInverter(T3Window * window);
void _t3_hideInverter(T3Window * window);
void _t3_updateLabels(const T3Window * window);
void _t3_toggleMode(T3Window * window);
bool _t3_addChar(T3Window * window, uint8_t pos);
const char * _t3_getCharGroup(const T3Window * window, int index);
char * _t3_getSingleChar(T3Window * window, int row);
void _t3_info(const char * message);
void _t3_warn(const char * message);
void _t3_error(const char * message);

T3Window * t3window_create(const char ** set1, uint8_t count1,
						 const char ** set2, uint8_t count2,
						 const char ** set3, uint8_t count3,
						 T3CloseHandler closeHandler) {
	T3Window * w = (T3Window*)malloc(sizeof(T3Window));
	
	if(set1 != NULL && count1 > 0)
		w->set = 0;
	else if(set2 != NULL && count2 > 0)
		w->set = 1;
	else if(set3 != NULL && count3 > 0)
		w->set = 2;
	else {
		w->set = 3;
		_t3_error("No T3 keyboards defined!");
	}
	
	w->kb = 0;
	w->row = 0;
	w->col = 0;
	w->selectionMode = false;
	w->timer = NULL;
	
	w->keyboardSets[0] = set1;
	w->keyboardSets[1] = set2;
	w->keyboardSets[2] = set3;
	w->keyboardCounts[0] = count1;
	w->keyboardCounts[1] = count2;
	w->keyboardCounts[2] = count3;
	w->closeHandler = closeHandler;
	
	_t3_info("Initializing T3 window");
	w->window = window_create();
	window_set_fullscreen(w->window, true);
	window_set_background_color(w->window, GColorWhite);
	window_set_click_config_provider_with_context(w->window,
		(ClickConfigProvider)_t3_clickConfigProvider, w);
	
	Layer * windowLayer = window_get_root_layer(w->window);
	
	// Create button labels
	for(int8_t r = 0; r < 3; ++r) {
		for(int8_t c = 0; c < 3; ++c) {
			uint8_t index = r * 3 + c;
			
			TextLayer * layer = text_layer_create(GRect(
				_T3_X_OFFSET + c * _T3_X_SPACING,
				_T3_Y_OFFSET + r * _T3_Y_SPACING,
				_T3_BUTTON_WIDTH,
				_T3_BUTTON_HEIGHT));
			text_layer_set_font(layer,
				fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
			text_layer_set_text_alignment(layer, GTextAlignmentCenter);
			text_layer_set_background_color(layer, GColorBlack);
			text_layer_set_text_color(layer, GColorWhite);
			layer_add_child(windowLayer, text_layer_get_layer(layer));
			
			w->buttons[index] = layer;
		}
	}
	
	_t3_updateLabels(w);

	// Create inverter layer
	w->inverter = inverter_layer_create(
		GRect(0, 0, _T3_BUTTON_WIDTH - 2, _T3_BUTTON_HEIGHT - 2));
	Layer * layer = inverter_layer_get_layer(w->inverter);
	layer_set_hidden(layer, true);
	layer_add_child(windowLayer, layer);
	w->inverterShowing = false;

	// Create input label
	w->inputLayer = text_layer_create(GRect(10, 138, 124, 30));
	text_layer_set_font(w->inputLayer,
		fonts_get_system_font(FONT_KEY_GOTHIC_24));
	layer_add_child(windowLayer, text_layer_get_layer(w->inputLayer));
	
	// Initialize input
	w->inputString = malloc(T3_MAXLENGTH + 1);
	for(int8_t i = 0; i < T3_MAXLENGTH; ++i)
		w->inputString[i] = '\0';
	w->inputLength = 0;
	
	_t3_info("T3 window initialized");
	
	return w;
}

void t3window_destroy(T3Window * window) {
	_t3_info("Destroying T3 window");
	free(window->inputString);
	text_layer_destroy(window->inputLayer);
	inverter_layer_destroy(window->inverter);
	for(int8_t i = 0; i < 9; ++i)
		text_layer_destroy(window->buttons[i]);
	window_destroy(window->window);
	free(window);
}

void t3window_show(const T3Window * window, bool animated) {
	_t3_info("Showing T3 window");
	window_stack_push(window->window, animated);
}

void t3window_set_text(T3Window * window, const char * text) {
	_t3_info("Setting T3 window text:");
	_t3_info(text);
	window->inputLength = strlen(text);
	if(window->inputLength > 0)
		strncpy(window->inputString, text, window->inputLength);
	window->inputString[window->inputLength] = '\0';
	text_layer_set_text(window->inputLayer, window->inputString);
}

const char * t3window_get_text(const T3Window * window) {
	_t3_info("Getting T3 window text");
	return window->inputString;
}

void _t3_clickConfigProvider(void * context) {
	window_multi_click_subscribe(BUTTON_ID_BACK, 2, 0, 0, true,
		(ClickHandler)_t3_backspace_click);
	window_single_click_subscribe(BUTTON_ID_BACK,
		(ClickHandler)_t3_back_click);
	window_long_click_subscribe(BUTTON_ID_UP, 0,
		(ClickHandler)_t3_r1_longclick, NULL);
	window_long_click_subscribe(BUTTON_ID_SELECT, 0,
		(ClickHandler)_t3_r2_longclick, NULL);
	window_long_click_subscribe(BUTTON_ID_DOWN, 0,
		(ClickHandler)_t3_r3_longclick, NULL);
	window_single_click_subscribe(BUTTON_ID_UP,
		(ClickHandler)_t3_r1_click);
	window_single_click_subscribe(BUTTON_ID_SELECT,
		(ClickHandler)_t3_r2_click);
	window_single_click_subscribe(BUTTON_ID_DOWN,
		(ClickHandler)_t3_r3_click);
}

void _t3_backspace_click(ClickRecognizerRef recognizer, void * context) {
	_t3_info("Backspace");
	T3Window * w = (T3Window*)context;
	if(w->inputLength > 0) {
		w->inputString[--(w->inputLength)] = '\0';
		text_layer_set_text(w->inputLayer, w->inputString);
	}
}

void _t3_back_click(ClickRecognizerRef recognizer, void * context) {
	_t3_info("Back clicked");
	T3Window * w = (T3Window*)context;
	if(w->timer != NULL) {
		_t3_info("Cancelling timer");
		app_timer_cancel(w->timer);
		w->row = 0;
		w->col = 0;
		_t3_hideInverter(w);
	} else if(w->selectionMode) {
		_t3_info("Cancelling char selection");
		_t3_toggleMode(w);
	} else {
		_t3_info("Popping window");
		window_stack_pop(true);
		if(w->closeHandler != NULL)
			w->closeHandler(w->inputString);
	}
}

void _t3_r1_longclick(ClickRecognizerRef recognizer, void * context) {
	_t3_longclick((T3Window*)context, 0);
}

void _t3_r2_longclick(ClickRecognizerRef recognizer, void * context) {
	_t3_longclick((T3Window*)context, 1);
}

void _t3_r3_longclick(ClickRecognizerRef recognizer, void * context) {
	_t3_longclick((T3Window*)context, 2);
}

void _t3_r1_click(ClickRecognizerRef recognizer, void * context) {
	_t3_click((T3Window*)context, 1);
}

void _t3_r2_click(ClickRecognizerRef recognizer, void * context) {
	_t3_click((T3Window*)context, 2);
}

void _t3_r3_click(ClickRecognizerRef recognizer, void * context) {
	_t3_click((T3Window*)context, 3);
}

void _t3_longclick(T3Window * window, uint8_t button) {
	if(window->keyboardCounts[button] > 0) 	{
		if(window->selectionMode)
			_t3_toggleMode(window);
		
		if(window->set == button) {
			if(window->keyboardCounts[button] > 1) {
				_t3_info("Cycling keyboard");
				if(++(window->kb) >= window->keyboardCounts[button])
					window->kb = 0;
				_t3_updateLabels(window);
			}
		} else {
			_t3_info("Changing keyboard set");
			window->set = button;
			window->kb = 0;
			_t3_updateLabels(window);
		}
	}
}

void _t3_click(T3Window * window, uint8_t row) {
	if(window->selectionMode) {
		_t3_addChar(window, row);
		_t3_toggleMode(window);
	} else {
		if(window->row != row) {
			_t3_info("Changing row");
			window->row = row;
			window->col = 1;
			_t3_updateInverter(window);
		} else {
			_t3_info("Cycling column");
			if(++(window->col) > 3)
				window->col = 1;
			_t3_updateInverter(window);
		}
		
		if(window->timer == NULL) {
			_t3_info("Starting timer");
			window->timer = app_timer_register(_T3_MODE_TIMEOUT_IN_MS, _t3_timerCallback, window);
		} else {
			_t3_info("Rescheduling timer");
			app_timer_reschedule(window->timer, _T3_MODE_TIMEOUT_IN_MS);
		}
	}
}

void _t3_timerCallback(void * context) {
	_t3_info("Timer timeout");
	T3Window * w = (T3Window*)context;
	_t3_hideInverter(w);
	char c = *_t3_getSingleChar(w, 1);
	if(c == '\0') {
		_t3_addChar(w, 1);
		w->row = 0;
		w->col = 0;
	} else
		_t3_toggleMode(w);
	w->timer = NULL;
}

void _t3_updateInverter(T3Window * window) {
	if(window->row == 0 && window->col == 0)
		_t3_hideInverter(window);
	else
	{
		_t3_info("Moving inverter");
		Layer * layer = inverter_layer_get_layer(window->inverter);
		layer_set_frame(layer, GRect(
			_T3_X_OFFSET + ((window->col - 1) * _T3_X_SPACING) + 1,
			_T3_Y_OFFSET + ((window->row - 1) * _T3_Y_SPACING) + 1,
			_T3_BUTTON_WIDTH - 2,
			_T3_BUTTON_HEIGHT - 2));
		if(!window->inverterShowing) {
			layer_set_hidden(layer, false);
			window->inverterShowing = true;
		}
	}
}

void _t3_hideInverter(T3Window * window) {
	if(window->inverterShowing) {
		_t3_info("Hiding inverter");
		layer_set_hidden(inverter_layer_get_layer(window->inverter), true);
		window->inverterShowing = false;
	}
}

void _t3_updateLabels(const T3Window * window) {
	_t3_info("Updating labels");
	for(int r = 0; r < 3; ++r) {
		for(int c = 0; c < 3; ++c) {
			uint8_t index = r * 3 + c;
			TextLayer * layer = window->buttons[index];
			const char * group = _t3_getCharGroup(window, index);
			text_layer_set_text(layer, group);
		}
	}
}

void _t3_toggleMode(T3Window * window) {
	_t3_info("Toggling mode");
	window->selectionMode = !window->selectionMode;
	
	for(int r = 0; r < 3; ++r) {
		for(int c = 0; c < 3; ++c) {
			uint8_t index = r * 3 + c;
			TextLayer * layer = window->buttons[index];
			if(c < 2)
				layer_set_hidden(text_layer_get_layer(layer),
					window->selectionMode);
			else {
				if(window->selectionMode) {
					char * c = _t3_getSingleChar(window, r);
					text_layer_set_text(layer, c);
				} else {
					const char * group = _t3_getCharGroup(window, index);
					text_layer_set_text(layer, group);
				}
			}
		}
	}
	
	if(!window->selectionMode) {
		window->row = 0;
		window->col = 0;
	}
}

bool _t3_addChar(T3Window * window, uint8_t pos) {
	if(window->inputLength < T3_MAXLENGTH) {
		char c = *_t3_getSingleChar(window, pos - 1);
		if(c == '\0')
			return false;
		else {
			if(T3_LOGGING) {
				char buffer[14] = "Adding char  \0";
				buffer[12] = c;
				_t3_info(buffer);
			}
			window->inputString[(window->inputLength)++] = c;
			text_layer_set_text(window->inputLayer, window->inputString);
			return true;
		}
	} else
		return false;
}

const char * _t3_getCharGroup(const T3Window * window, int index) {
	if(window->set < 3) {
		const char * charset = window->keyboardSets[window->set][window->kb];
		return &charset[index * 4];
	} else
		return NULL;
}

char * _t3_getSingleChar(T3Window * window, int index) {
	uint8_t i = ((window->row - 1) * 3) + (window->col - 1);
	const char * cg = _t3_getCharGroup(window, i);
	strncpy(window->singleChars[index], &cg[index], 1);
	return window->singleChars[index];
}

void _t3_info(const char * message) {
	if(T3_LOGGING)
		APP_LOG(APP_LOG_LEVEL_INFO, message);
}

void _t3_warn(const char * message) {
	if(T3_LOGGING)
		APP_LOG(APP_LOG_LEVEL_WARNING, message);
}

void _t3_error(const char * message) {
	if(T3_LOGGING)
		APP_LOG(APP_LOG_LEVEL_ERROR, message);
}