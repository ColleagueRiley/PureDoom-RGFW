#if defined(__APPLE__)
#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

#define RGFW_BUFFER

#define RGFW_IMPLEMENTATION

#include "RGFW.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#ifdef RGFW_WINDOWS
#include "mmeapi.h"
#endif

#define DOOM_IMPLEMENT_PRINT 
#define DOOM_IMPLEMENT_FILE_IO 
#define DOOM_IMPLEMENT_MALLOC 
#define DOOM_IMPLEMENT_GETTIME 
#define DOOM_IMPLEMENT_EXIT 
#define DOOM_IMPLEMENT_GETENV 
#include "PureDOOM.h"

#include "stb_image_resize2.h"
#include "miniaudio.h"

/* Resolution DOOM renders at */
#define WIDTH 320
#define HEIGHT 200

doom_key_t RGFWScancode2DOOM[RGFW_keyLast] = {
    [RGFW_tab] = DOOM_KEY_TAB,
    [RGFW_return] = DOOM_KEY_ENTER,
    [RGFW_escape] = DOOM_KEY_ESCAPE,
    [RGFW_space] = DOOM_KEY_CTRL,
    [RGFW_apostrophe] = DOOM_KEY_APOSTROPHE,
    [RGFW_multiply] = DOOM_KEY_MULTIPLY,
    [RGFW_comma] = DOOM_KEY_COMMA,
    [RGFW_minus] = DOOM_KEY_MINUS,
    [RGFW_period] = DOOM_KEY_PERIOD,
    [RGFW_slash] = DOOM_KEY_SLASH,
    [RGFW_0] = DOOM_KEY_0,
    [RGFW_1] = DOOM_KEY_1,
    [RGFW_2] = DOOM_KEY_2,
    [RGFW_3] = DOOM_KEY_3,
    [RGFW_4] = DOOM_KEY_4,
    [RGFW_5] = DOOM_KEY_5,
    [RGFW_6] = DOOM_KEY_6,
    [RGFW_7] = DOOM_KEY_7,
    [RGFW_8] = DOOM_KEY_8,
    [RGFW_9] = DOOM_KEY_9,
    [RGFW_semicolon] = DOOM_KEY_SEMICOLON,
    [RGFW_equals] = DOOM_KEY_EQUALS,
    [RGFW_bracket] = DOOM_KEY_LEFT_BRACKET,
    [RGFW_closeBracket] = DOOM_KEY_RIGHT_BRACKET,
    [RGFW_a] = DOOM_KEY_A,
    [RGFW_b] = DOOM_KEY_B,
    [RGFW_c] = DOOM_KEY_C,
    [RGFW_d] = DOOM_KEY_D,
    [RGFW_e] = DOOM_KEY_E,
    [RGFW_f] = DOOM_KEY_F,
    [RGFW_g] = DOOM_KEY_G,
    [RGFW_h] = DOOM_KEY_H,
    [RGFW_i] = DOOM_KEY_I,
    [RGFW_j] = DOOM_KEY_J,
    [RGFW_k] = DOOM_KEY_K,
    [RGFW_l] = DOOM_KEY_L,
    [RGFW_m] = DOOM_KEY_M,
    [RGFW_n] = DOOM_KEY_N,
    [RGFW_o] = DOOM_KEY_O,
    [RGFW_p] = DOOM_KEY_P,
    [RGFW_q] = DOOM_KEY_Q,
    [RGFW_r] = DOOM_KEY_R,
    [RGFW_s] = DOOM_KEY_S,
    [RGFW_t] = DOOM_KEY_T,
    [RGFW_u] = DOOM_KEY_U,
    [RGFW_v] = DOOM_KEY_V,
    [RGFW_w] = DOOM_KEY_W,
    [RGFW_x] = DOOM_KEY_X,
    [RGFW_y] = DOOM_KEY_Y,
    [RGFW_z] = DOOM_KEY_Z,
    [RGFW_backSpace] = DOOM_KEY_BACKSPACE,
    [RGFW_altL] = DOOM_KEY_ALT,
    [RGFW_altR] = DOOM_KEY_ALT,
    [RGFW_controlL] = DOOM_KEY_CTRL,
    [RGFW_controlR] = DOOM_KEY_CTRL,
    [RGFW_left] = DOOM_KEY_LEFT_ARROW,
    [RGFW_up] = DOOM_KEY_UP_ARROW,
    [RGFW_right] = DOOM_KEY_RIGHT_ARROW,
    [RGFW_down] = DOOM_KEY_DOWN_ARROW,      
    [RGFW_shiftL] = DOOM_KEY_SHIFT,
    [RGFW_shiftR] = DOOM_KEY_SHIFT,
    [RGFW_F1] = DOOM_KEY_F1,
    [RGFW_F2] = DOOM_KEY_F2,
    [RGFW_F3] = DOOM_KEY_F3,
    [RGFW_F4] = DOOM_KEY_F4,
    [RGFW_F5] = DOOM_KEY_F5,
    [RGFW_F6] = DOOM_KEY_F6,
    [RGFW_F7] = DOOM_KEY_F7,
    [RGFW_F8] = DOOM_KEY_F8,
    [RGFW_F9] = DOOM_KEY_F9,
    [RGFW_F10] = DOOM_KEY_F10,
    [RGFW_F11] = DOOM_KEY_F11,
    [RGFW_F12] = DOOM_KEY_F12,
};

doom_key_t RGFWGamepad2DOOM[20] = {
    [RGFW_gamepadY] = DOOM_KEY_TAB,
    [RGFW_gamepadSelect] = DOOM_KEY_ENTER,
    [RGFW_gamepadStart] = DOOM_KEY_ESCAPE,
    [RGFW_gamepadX] = DOOM_KEY_CTRL,
    [RGFW_gamepadA] = DOOM_KEY_E,
    [RGFW_gamepadR2] = DOOM_KEY_CTRL,
    [RGFW_gamepadLeft] = DOOM_KEY_A,
    [RGFW_gamepadRight] = DOOM_KEY_D,
    [RGFW_gamepadUp] = DOOM_KEY_UP_ARROW,
    [RGFW_gamepadDown] = DOOM_KEY_DOWN_ARROW,      
    [RGFW_gamepadL3] = DOOM_KEY_SHIFT,
    
    [RGFW_gamepadHome] = DOOM_KEY_UNKNOWN,
    [RGFW_gamepadB] = DOOM_KEY_UNKNOWN,
    [RGFW_gamepadL2] = DOOM_KEY_UNKNOWN,
    [RGFW_gamepadR3] = DOOM_KEY_UNKNOWN,
    [RGFW_gamepadL1] = DOOM_KEY_UNKNOWN,
    [RGFW_gamepadR1] = DOOM_KEY_UNKNOWN,
};

void gamepad_update(u32 button, u32 press) {
    u32 Dbutton = RGFWGamepad2DOOM[button];
    u32 Dbutton2 = DOOM_KEY_UNKNOWN;

    static char weapon = DOOM_KEY_1;
    if (button == RGFW_gamepadR1 && weapon < DOOM_KEY_7) {
        if (press == 0) weapon++;
        press = !press;
        Dbutton2 = weapon;
    }
    else if (button == RGFW_gamepadL1 && weapon > DOOM_KEY_1 && press == 0) {
        if (press == 0) weapon--;
        press = !press;
        Dbutton2 = weapon;
    }

    switch (Dbutton) {
        case DOOM_KEY_E: Dbutton2 = DOOM_KEY_ENTER; break;
        case DOOM_KEY_UP_ARROW: Dbutton2 = DOOM_KEY_W; break;
        case DOOM_KEY_DOWN_ARROW: Dbutton2 = DOOM_KEY_S; break;
        default: break;
    }
    
    if (press) {
        doom_key_down(Dbutton);
        doom_key_down(Dbutton2);
    } else {
        doom_key_up(Dbutton);
        doom_key_up(Dbutton2);
    }
}

doom_button_t RGFWButton2DOOM[4] = {
    [RGFW_mouseLeft] = DOOM_LEFT_BUTTON,
    [RGFW_mouseRight] = DOOM_RIGHT_BUTTON,
    [RGFW_mouseMiddle] = DOOM_MIDDLE_BUTTON,
};

/* this can also be used to convert BGR to RGB */
void bitmap_rgbToBgr(u8* bitmap, RGFW_area a) {
    /* loop through eacfh *pixel* (not channel) of the buffer */
    u32 x, y;
    for (y = 0; y < a.h; y++) {
        for (x = 0; x < a.w; x++) {
            u32 index = (y * 4 * a.w) + x * 4;

            u8 red = bitmap[index];
            bitmap[index] = bitmap[index + 2];
            bitmap[index + 2] = red;
        }
    }    
}

#if defined(WIN32)
static HMIDIOUT midi_out_handle = 0;
void send_midi_msg(uint32_t midi_msg)
{
    if (midi_out_handle)
    {
        midiOutShortMsg(midi_out_handle, midi_msg);
    }
}
#elif defined(__APPLE__)
AudioUnit audio_unit = 0;
void send_midi_msg(uint32_t midi_msg)
{
    if (audio_unit)
    {
        MusicDeviceMIDIEvent(audio_unit, 
            (midi_msg) & 0xFF,
            (midi_msg >> 8) & 0xFF,
            (midi_msg >> 16) & 0xFF,
            0);
    }
}
#else
void send_midi_msg(uint32_t midi_msg) {}
#endif


u32 tick_midi(u32 interval, void *param)
{
    u32 midi_msg;

    while ((midi_msg = doom_tick_midi()) != 0) send_midi_msg(midi_msg);

#if defined(__APPLE__)
    return (DOOM_MIDI_RATE - 1) * 5e+4; // Weirdly, on Apple music is too slow
#else
    return DOOM_MIDI_RATE * 5e+4;
#endif
}

ma_bool8 g_isPaused = MA_TRUE;
void audio_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {   
    if (g_isPaused)
        return;

    i16* buffer = doom_get_sound_buffer();
    memcpy(pOutput, buffer, frameCount * pDevice->playback.channels * sizeof(ma_uint16));
}


void* thread(void* args) {
    static u64 midi_time = 0;
    static u64 lastTime = 0;

    while (1) {
        if (RGFW_getTimeNS() - lastTime >= midi_time) {
            midi_time = tick_midi(0, 0);
            lastTime = RGFW_getTimeNS();
        }
    }
}

int main(int argc, char** args) {
#ifdef __EMSCRIPTEN__
    RGFW_window* window = RGFW_createWindow("RGFW DOOM", RGFW_RECT(0, 0, 620, 400), RGFW_windowCenter | RGFW_windowAllowDND);  
#else
    RGFW_window* window = RGFW_createWindow("RGFW DOOM", RGFW_RECT(0, 0, 320, 200), RGFW_windowCenter);
#endif

    RGFW_window_initBuffer(window);
    RGFW_area screenSize = RGFW_getScreenSize();
    size_t buffer_stride = screenSize.w * 4;
    size_t doom_stride = WIDTH * 4;

    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format   = ma_format_s16;
    config.playback.channels = 2;
    config.sampleRate        = DOOM_SAMPLERATE;
    config.dataCallback      = audio_callback;
    config.periodSizeInFrames = 512;

    ma_device device, midi_device;
    if (ma_device_init((void*)NULL, &config, &device) != MA_SUCCESS) {
        printf("Failed to init miniaudio device\n");
        return 1;
    }

    ma_device_start(&device);

    // Capture mouse
    RGFW_window_showMouse(window, 0);
    RGFW_window_mouseHold(window, RGFW_AREA(0, 0));

    //-----------------------------------------------------------------------
    // Setup DOOM
    //-----------------------------------------------------------------------

    // Change default bindings to modern
    doom_set_default_int("key_up", DOOM_KEY_W);
    doom_set_default_int("key_down", DOOM_KEY_S);
    doom_set_default_int("key_strafeleft", DOOM_KEY_A);
    doom_set_default_int("key_straferight", DOOM_KEY_D);
    doom_set_default_int("key_use", DOOM_KEY_E);
    doom_set_default_int("key_fire", DOOM_KEY_CTRL);
    doom_set_default_int("mouse_move", 0); // Mouse will not move forward
    
    // Setup resolution
    doom_set_resolution(WIDTH, HEIGHT);

    #ifdef RGFW_WINDOWS
    // Setup MIDI for songs
    if (midiOutGetNumDevs() != 0)
        midiOutOpen(&midi_out_handle, 0, 0, 0, 0);

    #elif defined(__APPLE__)
	AUGraph graph;
	AUNode outputNode, mixerNode, dlsNode;
	NewAUGraph(&graph);
	AudioComponentDescription output = {'auou','ahal','appl',0,0};
	AUGraphAddNode(graph, &output, &outputNode);
	AUGraphOpen(graph);
	AUGraphInitialize(graph);
	AUGraphStart(graph);
	AudioComponentDescription dls = {'aumu','dls ','appl',0,0};
	AUGraphAddNode(graph, &dls, &dlsNode);
	AUGraphNodeInfo(graph, dlsNode, NULL, &audio_unit);
	AudioComponentDescription mixer = {'aumx','smxr','appl',0,0};
	AUGraphAddNode(graph, &mixer, &mixerNode);
	AUGraphConnectNodeInput(graph,mixerNode,0,outputNode,0);
	AUGraphConnectNodeInput(graph,dlsNode,0,mixerNode,0);
	AUGraphUpdate(graph,NULL);
    #endif

    // Initialize doom
    doom_init(argc, args, DOOM_FLAG_MENU_DARKEN_BG);

    // Main loop
    g_isPaused = MA_FALSE;

    int done = 0;

    int active_mouse = 1; // Dev allow us to take mouse out of window

    RGFW_createThread((void*)thread, NULL);

    STBIR_RESIZE resize;

    stbir_resize_init(&resize, NULL, WIDTH, HEIGHT, doom_stride,  window->buffer, 0, 0, buffer_stride, 
                            STBIR_RGBA_NO_AW, STBIR_TYPE_UINT8_SRGB);
    
    resize.fast_alpha = 1;
    resize.horizontal_filter = STBIR_FILTER_BOX;
    resize.vertical_filter = STBIR_FILTER_BOX;
	
	u32 fps = 0;
    double start = RGFW_getTime();
    u32 frames = 0;

    i32 j = 0;
    while (!done) {
        while (RGFW_window_checkEvent(window)) {
            switch (window->event.type) {
                case RGFW_quit:
                    done = 1;
                    break;

                case RGFW_keyPressed:
                    /* 
					 * I don't know a better way to do this without it causing
					 * bugs or having to modify PureDOOM
					*/
					if (window->event.key == RGFW_end || (window->event.key == RGFW_shiftL && RGFW_isPressed(window, RGFW_escape)))
                    {
						RGFW_window_showMouse(window, active_mouse);
                        if (active_mouse)
                            RGFW_window_mouseUnhold(window);
                        else
                            RGFW_window_mouseHold(window, RGFW_AREA(0, 0));
                        
                        active_mouse = !active_mouse;
                    }

                    doom_key_down(RGFWScancode2DOOM[window->event.key]);
                    break;

                case RGFW_keyReleased:
                    doom_key_up(RGFWScancode2DOOM[window->event.key]);
                    break;

                case RGFW_mouseButtonPressed:
                    if (active_mouse) doom_button_down(RGFWButton2DOOM[window->event.button]);
                    break;

                case RGFW_mouseButtonReleased:
                    if (active_mouse) doom_button_up(RGFWButton2DOOM[window->event.button]);
                    break;

                case RGFW_mousePosChanged:
                    if (active_mouse)
                    {
						doom_mouse_move(window->event.vector.x * 10, 0);
					}
                    break;
                case RGFW_gamepadButtonPressed:
                    gamepad_update(window->event.button, 1);
                    break;
                case RGFW_gamepadButtonReleased:
                    gamepad_update(window->event.button, 0);
                    break;
                case RGFW_gamepadAxisMove:
                    // axis 12
                    switch (window->event.whichAxis) {
                        case 0:
                            if (window->event.axis[0].x >= 50) {
                                doom_key_down(DOOM_KEY_D);
                            }
                            else if (window->event.axis[0].x <= -50)
                                doom_key_down(DOOM_KEY_A);
                            else {
                                doom_key_up(DOOM_KEY_A);
                                doom_key_up(DOOM_KEY_D);
                            }
                            
                            if (window->event.axis[0].y >= 50) {
                                doom_key_down(DOOM_KEY_S);
                                doom_key_up(DOOM_KEY_DOWN_ARROW);
                            }
                            else if (window->event.axis[0].y <= -50) {
                                doom_key_down(DOOM_KEY_W);
                                doom_key_up(DOOM_KEY_UP_ARROW);
                            }
                            else {
                                doom_key_up(DOOM_KEY_W);
                                doom_key_up(DOOM_KEY_S);
                                doom_key_up(DOOM_KEY_UP_ARROW);
                                doom_key_up(DOOM_KEY_DOWN_ARROW);
                            }
                            break;                        
                        // axis 2
                        case 1:
                            if (window->event.axis[1].x >= 50) {
                                doom_key_down(DOOM_KEY_RIGHT_ARROW);
                            }
                            else if (window->event.axis[1].x <= -50)
                                doom_key_down(DOOM_KEY_LEFT_ARROW);
                            else {
                                doom_key_up(DOOM_KEY_LEFT_ARROW);
                                doom_key_up(DOOM_KEY_RIGHT_ARROW);
                            }
                            break;
                        default: break;
                    }
                    break;
            }
            if (done) break;
        }
        if (done) break;

        doom_update();

        resize.input_pixels = doom_get_framebuffer(4);
        resize.output_w = resize.output_subw= window->r.w;
        resize.output_h = resize.output_subh = window->r.h; 
        stbir_resize_extended(&resize);

        RGFW_window_swapBuffers(window);
		fps = RGFW_checkFPS(start, frames, 0);
        frames++;
    }
    
#if defined(WIN32)
    if (midi_out_handle) midiOutClose(midi_out_handle);
#endif

    ma_device_uninit(&device);
    RGFW_window_close(window);
    return 0;
}
