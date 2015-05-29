# T3 Keyboard

The T3 Keyboard provides a fast and intuitive method for textual input on the Pebble smartwatch.

# Usage

#### 1. Add T3Window.h and T3Window.c to your Pebble project.
```c
#include "T3Window.h"
```
#### 2. Define keyboard sets and layouts:
```c
const char MY_KEYBOARD_LAYOUT[] =
    "abc\0"  "def\0"  "ghi\0"
    "jkl\0"  "mno\0"  "pqr\0"
	"stu\0"  "vwx\0"  "yz "
const char * myKeyboardSet[] = {MY_KEYBOARD_LAYOUT};
```
Or use predefined ones:
```c
const char * keyboardSet1[] = {T3_KB_LOWERCASE, T3_KB_UPPERCASE};
const char * keyboardSet2[] = {T3_KB_NUMBERS};
const char * keyboardSet3[] = {T3_KB_SPECIAL1, T3_KB_SPECIAL2};
```
#### 3. Define a handler to get the entered text:
```c
void myCloseHandler(const char * text) {
    // Do something
}
```
#### 4. Create the window:
```c
T3Window * myT3Window = t3window_create(
    keyboardSet1, 2,
    keyboardSet2, 1,
    keyboardSet3, 2,
    (T3CloseHandler)myCloseHandler);
```
#### 5. (Optional) Change the keyboard colors:
```c
t3window_set_colors(myT3Window, GColorPictonBlue,
	GColorBlueMoon,	GColorWhite,
	GColorElectricBlue,	GColorDukeBlue,
	GColorWhite, GColorBlack,
	GColorElectricBlue, GColorDukeBlue);
```
Or use predefined themes (more to be added over time):
```c
T3_SET_THEME_BLUE(myT3Window);
```
#### 5. Show the window:
```c
t3window_show(myT3Window, true);
```

# Keyboard Layout Definition
A keyboard layout is defined by a 36-character string: Nine key definitions, each key consisting of four characters.  A key may start with up to three printable characters and the rest is filled with null terminators.  Keys are defined in the order of: top-left, top-center, top-right, middle-left, middle-center, middle-right, bottom-left, bottom-center, bottom-right.

> **layout** := key key key key key key key key key

> **key** := null null null null | char null null null | char char null null | char char char null

> **char** := *any printable character*

> **null** := ```\0```

# Interface Documentation
## Macros
### T3_LOGGING
Whether diagnostic information of keyboard events should be logged. To enable logging, set to 1.

### T3_INCLUDE_LAYOUT_LOWERCASE
Whether to build the pre-defined lower-case keyboard into the app. It is recommended that you set this to 0 if you are not using it in order to reduce memory usage.

### T3_INCLUDE_LAYOUT_UPPERCASE
Whether to build the pre-defined upper-case keyboard into the app. It is recommended that you set this to 0 if you are not using it in order to reduce memory usage.

### T3_INCLUDE_LAYOUT_NUMBERS
Whether to build the pre-defined number keyboard into the app. It is recommended that you set this to 0 if you are not using it in order to reduce memory usage.

### T3_INCLUDE_LAYOUT_PUNC
Whether to build the pre-defined punctuation keyboard into the app. It is recommended that you set this to 0 if you are not using it in order to reduce memory usage.

### T3_INCLUDE_LAYOUT_BRACKETS
Whether to build the pre-defined bracket keyboard into the app. It is recommended that you set this to 0 if you are not using it in order to reduce memory usage.

### T3_MAXLENGTH
The maximum number of characters that the user may enter.

### T3_SET_THEME_GRAY(t3window)
Sets a pre-defined gray color theme to the window. (Default theme)

### T3_SET_THEME_BLUE(t3window)
Sets a pre-defined blue color theme to the window.

### T3_SET_THEME_RED(t3window)
Sets a pre-defined red color theme to the window.

### T3_SET_THEME_GREEN(t3window)
Sets a pre-defined green color theme to the window.

## Constants
### const char T3_LAYOUT_LOWERCASE[]
This is a pre-defined keyboard layout with lower-case letters.

||||
|:-:|:-:|:-:|
|abc|def|ghi|
|jkl|mno|pqr|
|stu|vwx|yz |

### const char T3_LAYOUT_UPPERCASE[]
This is a pre-defined keyboard layout with upper-case letters.

||||
|:-:|:-:|:-:|
|ABC|DEF|GHI|
|JKL|MNO|PQR|
|STU|VWX|YZ |

### const char T3_LAYOUT_NUMBERS[]
This is a pre-defined keyboard layout with numbers.

||||
|:-:|:-:|:-:|
|01|2|3|
|4|5|6|
|7|8|9|

### const char T3_LAYOUT_PUNC[]
This is a pre-defined keyboard layout with punctuation, operators, etc.

||||
|:-:|:-:|:-:|
|.|'!|:;"|
|,|-|@$#|
|?|&%|+*=|

### const char T3_LAYOUT_BRACKETS[]
This is a pre-defined keyboard layout with brackets, slashes, and other miscellaneous characters.

||||
|:-:|:-:|:-:|
|()|<>|{}|
|/|\\|[]|
|&#124;_|~^\`|¢½|

## Structures
### T3Window
This holds information about the T3 Keyboard Window. It is created with ```t3window_create()``` and must be passed to the other interface functions.

### Handlers
### void (*T3CloseHandler)(const char * text)
This is a handler that is fired when the user accepts their entered text and closes the window.  It is used by ```t3window_create()```.

|Parameter|Description|
|---|---|
|text|The entered text.|

## Functions
### t3window_create
```c
T3Window * t3window_create(
    const char ** set1, uint8_t count1,
    const char ** set2, uint8_t count2,
    const char ** set3, uint8_t count3,
    T3CloseHandler closeHandler)
```
Creates a new T3Window, given the keyboard layouts and callback function.

A keyboard set contains keyboard layouts. If a set contains multiple layouts, the user will be able to cycle through them by holding the UP, SELECT, or DOWN buttons. At least one keyboard layout must be defined and the first keyboard found, starting with set 1, will be shown by default.

|Parameter|Description|
|---|---|
|**set1**|A pointer to an array of strings containing the keyboard layouts that the user may cycle through using the UP button. This may be null.|
|**count1**|The number of keyboard layouts in set1. This should be 0 if set1 is null.|
|**set2**|A pointer to an array of strings containing the keyboard layouts that the user may cycle through using the SELECT button. This may be null.|
|**count2**|The number of keyboard layouts in set2. This should be 0 if set2 is null.|
|**set3**|A pointer to an array of strings containing the keyboard layouts that the user may cycle through using the DOWN button. This may be null.|
|**count3**|The number of keyboard layouts in set3. This should be 0 if set3 is null.|
|**closeHandler**|The T3CloseHandler to fire when the keyboard closes. This may be null.|

#### Returns
A new ```T3Window``` structure.

### t3window_destroy
```c
void t3window_destroy(T3Window * window)
```
Destroys a ```T3Window``` previously created by ```t3window_create()```.

|Parameter|Description|
|---|---|
|**window**|The ```T3Window``` to destroy.|

### t3window_set_colors
```c
void t3window_set_colors(
	T3Window * window, GColor background,
	GColor keyFace, GColor keyText,
	GColor keyHighlight, GColor keyShadow,
	GColor editBackground, GColor editText,
	GColor editHighlight, GColor editShadow)
```
Sets the window colors if color is supported. Pressed key colors are inferred, but they may be overridden using ```t3_window_set_pressed_key_colors()```. If not called, a default scheme is used.

|Parameter|Description|
|---|---|
|**window**|The ```T3Window``` whose colors to set.|
|**background**|The background color of the window.|
|**keyFace**|The primary color of the keys.|
|**keyText**|The color of the key text.|
|**keyHighlight**|The highlight color of the keys.|
|**keyShadow**|The shadow color of the keys.|
|**editBackground**|The background color of the edit area.|
|**editText**|The color of the edit area text.|
|**editHighlight**|The highlight color of the edit area.|
|**editShadow**|The shadow color of the edit area.|

### t3window_set_pressed_key_colors
```c
void t3window_set_pressed_key_colors(
	T3Window * window,
	GColor keyFace, GColor keyText,
	GColor keyHighlight, GColor keyShadow)
```
Overrides the colors used for a key that is pressed. This is called with inferred colors by ```t3window_set_colors()```. Note that a pressed key is presented as sunken, so the highlight is on the bottom and right sides and the shadow is on the top and left sides.

|Parameter|Description|
|---|---|
|**window**|The ```T3Window``` whose colors to set.|
|**keyFace**|The primary color of the pressed key.|
|**keyText**|The color of the pressed key text.|
|**keyHighlight**|The highlight color of the pressed key.|
|**keyShadow**|The shadow color of the pressed key.|

### t3window_show
```c
void t3window_show(const T3Window * window, bool animated)
```
Places the ```T3Window``` on the window stack.

This call is equivalent to:
```c
window_stack_push(myT3Window->window, animated);
```
|Parameter|Description|
|---|---|
|**window**|The ```T3Window``` to display.|
|**animated**|Whether to use the ```window_stack_push()``` animation.|

### t3window_set_text
```c
void t3window_set_text(T3Window * window, const char * text)
```
Sets the initial entered text in the ```T3Window```.

|Parameter|Description|
|---|---|
|**window**|The ```T3Window``` whose text to set.|
|**text**|A pointer to the text to display. This will be copied locally, up to the ```T3_MAXLENGTH```, so a stack-allocated string may be used.|

### t3window_get_text
```c
const char * t3window_get_text(const T3Window * window)
```
Gets the entered text from the ```T3Window```.

|Parameter|Description|
|---|---|
|**window**|The ```T3Window``` whose text to get.|

#### Returns
A pointer to the text displayed in the window.
