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

#if T3_INCLUDE_LAYOUT_LOWERCASE
const char T3_LAYOUT_LOWERCASE[] =
	"abc\0"  "def\0"  "ghi\0"
	"jkl\0"  "mno\0"  "pqr\0"
	"stu\0"  "vwx\0"  "yz ";
#endif

#if T3_INCLUDE_LAYOUT_UPPERCASE
const char T3_LAYOUT_UPPERCASE[] =
	"ABC\0"  "DEF\0"  "GHI\0"
	"JKL\0"  "MNO\0"  "PQR\0"
	"STU\0"  "VWX\0"  "YZ ";
#endif

#if T3_INCLUDE_LAYOUT_NUMBERS
const char T3_LAYOUT_NUMBERS[] =
	"\x30\x31\0\0"  "\x32\0\0\0"  "\x33\0\0\0"
	"\x34\0\0\0"    "\x35\0\0\0"  "\x36\0\0\0"
	"\x37\0\0\0"    "\x38\0\0\0"  "\x39\0\0";
#endif

#if T3_INCLUDE_LAYOUT_PUNC
const char T3_LAYOUT_PUNC[] =
	".\0\0\0"  "'!\0\0"   ":;\"\0"
	",\0\0\0"  "-\0\0\0"  "@$#\0"
	"?\0\0\0"  "&%\0\0"   "+*=";
#endif

#if T3_INCLUDE_LAYOUT_BRACKETS
const char T3_LAYOUT_BRACKETS[] =
	"()\0\0"   "<>\0\0"    "{}\0\0"
	"/\0\0\0"  "\\\0\0\0"  "[]\0\0"
	"|_\0\0"   "~^`\0"     "¢½\0";
#endif
	
#define _T3_KEYBOARD_SIZE 9 * 4
#define _T3_X_OFFSET 7
#define _T3_Y_OFFSET 74
#define _T3_X_SPACING 45
#define _T3_Y_SPACING 31
#define _T3_BUTTON_WIDTH 40
#define _T3_BUTTON_HEIGHT 26
#define _T3_MODE_TIMEOUT_IN_MS 600

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
	Layer * buttons[9];
	Layer * inputLayer;
	char inputString[T3_MAXLENGTH + 1];
	uint8_t inputLength;
	bool selectionMode;
	AppTimer * timer;
	#if PBL_COLOR
	GColor background;
	GColor keyFace;
	GColor keyText;
	GColor keyHighlight;
	GColor keyShadow;
	GColor editBackground;
	GColor editText;
	GColor editHighlight;
	GColor editShadow;
	GColor pressedKeyFace;
	GColor pressedKeyText;
	GColor pressedKeyHighlight;
	GColor pressedKeyShadow;
	#endif
} T3Window;

typedef struct _t3_InputData {
	T3Window * t3window;
} _t3_InputData;

typedef struct _t3_KeyData {
	T3Window * t3window;
	uint8_t row;
	uint8_t col;
} _t3_KeyData;

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
void _t3_drawInput(Layer * layer, GContext * ctx);
void _t3_drawKey(Layer * layer, GContext * ctx);
void _t3_toggleMode(T3Window * window);
bool _t3_addChar(T3Window * window, char c);
const char * _t3_getCharGroup(const T3Window * window, int row, int col);

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
		
		#if T3_LOGGING
		app_log(APP_LOG_LEVEL_ERROR, "T3Window.c", 141, "No T3 keyboards defined!");
		#endif
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

	#if T3_LOGGING
	app_log(APP_LOG_LEVEL_INFO, "T3Window.c", 160, "Initializing T3 window");
	#endif
	
	w->window = window_create();
	#if PBL_PLATFORM_APLITE
	window_set_fullscreen(w->window, true);
	#endif
	#if PBL_BW
	window_set_background_color(w->window, GColorWhite);
	#endif
	window_set_click_config_provider_with_context(w->window,
		(ClickConfigProvider)_t3_clickConfigProvider, w);

	Layer * windowLayer = window_get_root_layer(w->window);
	
	// Create input label
	w->inputLayer = layer_create_with_data(GRect(4, 4, 136, 64), sizeof(_t3_InputData));
	_t3_InputData * data = layer_get_data(w->inputLayer);
	data->t3window = w;
	layer_set_update_proc(w->inputLayer, _t3_drawInput);
	layer_add_child(windowLayer, w->inputLayer);
	
	// Create button layers
	for(int8_t r = 0; r < 3; ++r) {
		for(int8_t c = 0; c < 3; ++c) {
			uint8_t index = r * 3 + c;
			Layer * layer = layer_create_with_data(
				GRect(
					_T3_X_OFFSET + c * _T3_X_SPACING,
					_T3_Y_OFFSET + r * _T3_Y_SPACING,
					_T3_BUTTON_WIDTH,
					_T3_BUTTON_HEIGHT),
				sizeof(_t3_KeyData));
			_t3_KeyData * data = layer_get_data(layer);
			data->t3window = w;
			data->row = r + 1;
			data->col = c + 1;
			layer_set_update_proc(layer, _t3_drawKey);
			layer_add_child(windowLayer, layer);
			
			w->buttons[index] = layer;
		}
	}
	
	#if PBL_COLOR
	T3_SET_THEME_GRAY(w);
	#endif
	
	w->singleChars[0][1] = '\0';
	w->singleChars[1][1] = '\0';
	w->singleChars[2][1] = '\0';
	
	// Initialize input
	for(uint8_t i = 0; i <= T3_MAXLENGTH; ++i)
		w->inputString[i] = '\0';
	w->inputLength = 0;
	
	#if T3_LOGGING
	app_log(APP_LOG_LEVEL_INFO, "T3Window.c", 228, "T3 window initialized");
	#endif
	
	return w;
}

#if PBL_COLOR
void t3window_set_colors(T3Window * window, GColor background,
						 GColor keyFace, GColor keyText,
						 GColor keyHighlight, GColor keyShadow,
						 GColor editBackground, GColor editText,
						 GColor editHighlight, GColor editShadow) {
	#if T3_LOGGING
	app_log(APP_LOG_LEVEL_INFO, "T3Window.c", 241, "Setting T3 window colors");
	#endif
		
	window->background = background;
	window->keyFace = keyFace;
	window->keyText = keyText;
	window->keyHighlight = keyHighlight;
	window->keyShadow = keyShadow;
	window->editBackground = editBackground;
	window->editText = editText;
	window->editHighlight = editHighlight;
	window->editShadow = editShadow;
	
	window_set_background_color(window->window, background);
	
	t3window_set_pressed_key_colors(window, keyShadow, keyText, keyHighlight, keyShadow);
}

void t3window_set_pressed_key_colors(T3Window * window,
									GColor keyFace, GColor keyText,
									GColor keyHighlight, GColor keyShadow) {
	#if T3_LOGGING
	app_log(APP_LOG_LEVEL_INFO, "T3Window.c", 263, "Setting T3 window pressed key colors");
	#endif
		
	window->pressedKeyFace = keyFace;
	window->pressedKeyText = keyText;
	window->pressedKeyHighlight = keyHighlight;
	window->pressedKeyShadow = keyShadow;
}
#endif

void t3window_destroy(T3Window * window) {
	#if T3_LOGGING
	app_log(APP_LOG_LEVEL_INFO, "T3Window.c", 275, "Destroying T3 window");
	#endif
	
	layer_destroy(window->inputLayer);
	for(int8_t i = 0; i < 9; ++i)
		layer_destroy(window->buttons[i]);
	window_destroy(window->window);
	free(window);
}

void t3window_show(const T3Window * window, bool animated) {
	#if T3_LOGGING
	app_log(APP_LOG_LEVEL_INFO, "T3Window.c", 288, "Showing T3 window");
	#endif
	
	window_stack_push(window->window, animated);
}

void t3window_set_text(T3Window * window, const char * text) {
	#if T3_LOGGING
	app_log(APP_LOG_LEVEL_INFO, "T3Window.c", 296, "Setting T3 window text: %s", text);
	#endif
	
	window->inputLength = strlen(text);
	if(window->inputLength > 0)
		strncpy(window->inputString, text, T3_MAXLENGTH);
	layer_mark_dirty(window->inputLayer);
}

const char * t3window_get_text(const T3Window * window) {
	#if T3_LOGGING
	app_log(APP_LOG_LEVEL_INFO, "T3Window.c", 308, "Getting T3 window text");
	#endif
	
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
	#if T3_LOGGING
	app_log(APP_LOG_LEVEL_INFO, "T3Window.c", 335, "Backspace");
	#endif
	
	T3Window * w = (T3Window*)context;
	if(w->inputLength > 0) {
		w->inputString[--(w->inputLength)] = '\0';
		layer_mark_dirty(w->inputLayer);
	}
}

void _t3_back_click(ClickRecognizerRef recognizer, void * context) {
	#if T3_LOGGING
	app_log(APP_LOG_LEVEL_INFO, "T3Window.c", 347, "Back clicked");
	#endif
		
	T3Window * w = (T3Window*)context;
	if(w->timer != NULL) {
		#if T3_LOGGING
		app_log(APP_LOG_LEVEL_INFO, "T3Window.c", 353, "Cancelling timer");
		#endif

		app_timer_cancel(w->timer);
		w->row = 0;
		w->col = 0;
		// **************************************************** may need to refresh
	} else if(w->selectionMode) {
		#if T3_LOGGING
		app_log(APP_LOG_LEVEL_INFO, "T3Window.c", 362, "Cancelling char selection");
		#endif
		
		_t3_toggleMode(w);
	} else {
		#if T3_LOGGING
		app_log(APP_LOG_LEVEL_INFO, "T3Window.c", 368, "Popping window");
		#endif
		
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
				#if T3_LOGGING
				app_log(APP_LOG_LEVEL_INFO, "T3Window.c", 409, "Cycling keyboard");
				#endif
				
				if(++(window->kb) >= window->keyboardCounts[button])
					window->kb = 0;
			}
		} else {
			#if T3_LOGGING
			app_log(APP_LOG_LEVEL_INFO, "T3Window.c", 417, "Changing keyboard set");
			#endif
			
			window->set = button;
			window->kb = 0;
		}
		layer_mark_dirty(window_get_root_layer(window->window));
	}
}

void _t3_click(T3Window * window, uint8_t row) {
	if(window->selectionMode) {
		_t3_addChar(window, window->singleChars[row - 1][0]);
		_t3_toggleMode(window);
	} else {
		if(window->row != row) {
			#if T3_LOGGING
			app_log(APP_LOG_LEVEL_INFO, "T3Window.c", 434, "Changing row");
			#endif
			
			window->row = row;
			window->col = 1;
			layer_mark_dirty(window_get_root_layer(window->window));
		} else {
			#if T3_LOGGING
			app_log(APP_LOG_LEVEL_INFO, "T3Window.c", 442, "Cycling column");
			#endif
			
			if(++(window->col) > 3)
				window->col = 1;
			layer_mark_dirty(window_get_root_layer(window->window));
		}
		
		if(window->timer == NULL) {
			#if T3_LOGGING
			app_log(APP_LOG_LEVEL_INFO, "T3Window.c", 452, "Starting timer");
			#endif
			
			window->timer = app_timer_register(_T3_MODE_TIMEOUT_IN_MS, _t3_timerCallback, window);
		} else {
			#if T3_LOGGING
			app_log(APP_LOG_LEVEL_INFO, "T3Window.c", 458, "Rescheduling timer");
			#endif
			
			app_timer_reschedule(window->timer, _T3_MODE_TIMEOUT_IN_MS);
		}
	}
}

void _t3_timerCallback(void * context) {
	#if T3_LOGGING
	app_log(APP_LOG_LEVEL_INFO, "T3Window.c", 468, "Timer timeout");
	#endif
	
	T3Window * w = (T3Window*)context;
	const char * text = _t3_getCharGroup(w, w->row, w->col);
	if(text[1] == '\0') {
		_t3_addChar(w, text[0]);
		w->row = 0;
		w->col = 0;
	} else
		_t3_toggleMode(w);
	w->timer = NULL;
	layer_mark_dirty(window_get_root_layer(w->window));
}

void _t3_drawInput(Layer * layer, GContext * context) {
	GRect bounds = layer_get_bounds(layer);
	_t3_InputData * data = layer_get_data(layer);
	
	#if PBL_BW
	graphics_context_set_stroke_color(context, GColorBlack);
	graphics_draw_rect(context, bounds);
	graphics_context_set_text_color(context, GColorBlack);
	#endif
		
	#if PBL_COLOR
	uint8_t h = bounds.size.h - 1;
	uint8_t w = bounds.size.w - 1;
	// Background
	graphics_context_set_fill_color(context, data->t3window->editBackground);
	graphics_fill_rect(context, bounds, 0, GCornerNone);
	// Shadow
	graphics_context_set_stroke_color(context, data->t3window->editShadow);
	graphics_draw_line(context, bounds.origin, GPoint(w, 0));
	graphics_draw_line(context, bounds.origin, GPoint(0, h));
	// Highlight
	graphics_context_set_stroke_color(context, data->t3window->editHighlight);
	graphics_draw_line(context, GPoint(1, h), GPoint(w, h));
	graphics_draw_line(context, GPoint(w, 1), GPoint(w, h));
	// Text
	graphics_context_set_text_color(context, data->t3window->editText);
	#endif
	
	bounds.origin.x += 2;
	bounds.origin.y += 2;
	bounds.size.w -= 4;
	bounds.size.h -= 4;
	graphics_draw_text(context, data->t3window->inputString, fonts_get_system_font(FONT_KEY_GOTHIC_24),
		bounds, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}

void _t3_drawKey(Layer * layer, GContext * context) {
	GRect bounds = layer_get_bounds(layer);
	_t3_KeyData * data = layer_get_data(layer);
	
	if(!data->t3window->selectionMode || data->col == 2) {
		const char * text;
		if(data->t3window->selectionMode) {
			text = data->t3window->singleChars[data->row - 1];
			if(text == NULL)
				return;
		} else
			text = _t3_getCharGroup(data->t3window, data->row, data->col);

		bool isPressed = !data->t3window->selectionMode
			&& data->row == data->t3window->row
			&& data->col == data->t3window->col;

		#if PBL_BW
		if(isPressed) {
			graphics_context_set_stroke_color(context, GColorBlack);
			graphics_draw_rect(context, bounds);
			graphics_context_set_text_color(context, GColorBlack);
		} else {
			graphics_context_set_fill_color(context, GColorBlack);
			graphics_fill_rect(context, bounds, 0, GCornerNone);
			graphics_context_set_text_color(context, GColorWhite);
		}
		#endif

		#if PBL_COLOR
		uint8_t h = _T3_BUTTON_HEIGHT - 1;
		uint8_t w = _T3_BUTTON_WIDTH - 1;
		if(isPressed) {
			// Face
			graphics_context_set_fill_color(context, data->t3window->pressedKeyFace);
			graphics_fill_rect(context, bounds, 0, GCornerNone);
			// Shadow
			graphics_context_set_stroke_color(context, data->t3window->pressedKeyShadow);
			graphics_draw_line(context, bounds.origin, GPoint(w, 0));
			graphics_draw_line(context, bounds.origin, GPoint(0, h));
			// Highlight
			graphics_context_set_stroke_color(context, data->t3window->pressedKeyHighlight);
			graphics_draw_line(context, GPoint(1, h), GPoint(w, h));
			graphics_draw_line(context, GPoint(w, 1), GPoint(w, h));
			// Text
			graphics_context_set_text_color(context, data->t3window->pressedKeyText);
		} else {
			// Face
			graphics_context_set_fill_color(context, data->t3window->keyFace);
			graphics_fill_rect(context, bounds, 0, GCornerNone);
			// Highlight
			graphics_context_set_stroke_color(context, data->t3window->keyHighlight);
			graphics_draw_line(context, bounds.origin, GPoint(w, 0));
			graphics_draw_line(context, bounds.origin, GPoint(0, w));
			// Shadow
			graphics_context_set_stroke_color(context, data->t3window->keyShadow);
			graphics_draw_line(context, GPoint(1, h), GPoint(w, h));
			graphics_draw_line(context, GPoint(w, 1), GPoint(w, h));
			// Text
			graphics_context_set_text_color(context, data->t3window->keyText);
		}
		#endif

		graphics_draw_text(context, text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
			bounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
	}
}

void _t3_toggleMode(T3Window * window) {
	#if T3_LOGGING
	app_log(APP_LOG_LEVEL_INFO, "T3Window.c", 587, "Toggling mode");
	#endif
	
	window->selectionMode = !window->selectionMode;
	
	if(window->selectionMode) {
		const char * cg = _t3_getCharGroup(window, window->row, window->col);
		window->singleChars[0][0] = cg[0];
		window->singleChars[1][0] = cg[1];
		window->singleChars[2][0] = cg[2];
	} else {
		window->row = 0;
		window->col = 0;
	}
	
	layer_mark_dirty(window_get_root_layer(window->window));
}

bool _t3_addChar(T3Window * window, char c) {
	if(window->inputLength < T3_MAXLENGTH) {
		if(c == '\0')
			return false;
		else {
			#if T3_LOGGING
			app_log(APP_LOG_LEVEL_INFO, "T3Window.c", 611, "Adding char: %c", c);
			#endif

			window->inputString[(window->inputLength)++] = c;
			layer_mark_dirty(window->inputLayer);
	
			return true;
		}
	} else
		return false;
}

const char * _t3_getCharGroup(const T3Window * window, int row, int col) {
	if(window->set < 3) {
		const char * charset = window->keyboardSets[window->set][window->kb];
		int index = ((row - 1) * 3) + (col - 1);
		return &charset[index * 4];
	} else
		return NULL;
}
