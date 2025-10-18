#if defined(__APPLE__)
#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

#define RGFW_BUFFER

#define RGFW_IMPLEMENTATION

#if defined(__linux__)
    #define _INPUT_EVENT_CODES_H
#endif
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
    [RGFW_kpMultiply] = DOOM_KEY_MULTIPLY,
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

doom_button_t RGFWButton2DOOM[4] = {
    [RGFW_mouseLeft] = DOOM_LEFT_BUTTON,
    [RGFW_mouseRight] = DOOM_RIGHT_BUTTON,
    [RGFW_mouseMiddle] = DOOM_MIDDLE_BUTTON,
};

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
#elif defined(__linux__)
#define _POSIX_C_SOURCE
#include <alsa/asoundlib.h>
static snd_rawmidi_t* midi_out_handle = NULL;

void send_midi_msg(uint32_t midi_msg)
{
    if (midi_out_handle)
    {
        unsigned char msg[3];
        msg[0] = midi_msg & 0xFF;
        msg[1] = (midi_msg >> 8) & 0xFF;
        msg[2] = (midi_msg >> 16) & 0xFF;
        snd_rawmidi_write(midi_out_handle, msg, 3);
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

#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
#include <mach/mach_time.h>
#else
#include <time.h>
#include <sys/time.h>
#endif

#ifndef _WIN32
#include <pthread.h>
#endif

u64 getTimeNS(void) {
#ifdef _WIN32
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (u64)((counter.QuadPart * 1000000000LL) / frequency.QuadPart);
#elif __APPLE__
    static mach_timebase_info_data_t timebase = {0};
    if (timebase.denom == 0) {
        mach_timebase_info(&timebase);
    }
    return (ticks * timebase.numer) / timebase.denom;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (u64)(ts.tv_sec * 1000000000 + ts.tv_nsec);
#endif
}

void createThread(RGFW_proc proc, void* args) {
#ifdef _WIN32
    HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)proc, args, 0, NULL);
#else
    pthread_t thread;
    pthread_create(&thread, NULL, (void*(*)(void*))proc, args);
#endif
}

void* thread(void* args) {
    static u64 midi_time = 0;
    static u64 lastTime = 0;

    while (1) {
        if (getTimeNS() - lastTime >= midi_time) {
            midi_time = tick_midi(0, 0);
            lastTime = getTimeNS();
        }
    }
}

int main(int argc, char** args) {
    RGFW_window* window = RGFW_createWindow("RGFW DOOM", 0, 0, 320, 200, RGFW_windowCenter);

	RGFW_monitor mon = RGFW_window_getMonitor(window);

	u8* buffer = RGFW_ALLOC(mon.mode.w * mon.mode.h * 4);
    RGFW_surface* surface = RGFW_createSurface(buffer, mon.mode.w, mon.mode.h, RGFW_formatRGBA8);

    size_t buffer_stride = mon.mode.w * 4;
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
    RGFW_window_holdMouse(window);

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
#elif defined(__linux__)
    snd_rawmidi_open(NULL, &midi_out_handle, "virtual", 0);
    if (midi_out_handle == NULL)
        printf("failed to get linux midi handle\n");
#endif

    // Initialize doom
    doom_init(argc, args, DOOM_FLAG_MENU_DARKEN_BG);

    // Main loop
    g_isPaused = MA_FALSE;

    int done = 0;

    int active_mouse = 1; // Dev allow us to take mouse out of window

    createThread((void*)thread, NULL);

    STBIR_RESIZE resize;

    stbir_resize_init(&resize, NULL, WIDTH, HEIGHT, doom_stride,  buffer, 0, 0, buffer_stride,
                            STBIR_RGBA_NO_AW, STBIR_TYPE_UINT8_SRGB);

    resize.fast_alpha = 1;
    resize.horizontal_filter = STBIR_FILTER_BOX;
    resize.vertical_filter = STBIR_FILTER_BOX;

    i32 j = 0;
    while (!done) {
		RGFW_event event;
        while (RGFW_window_checkEvent(window, &event)) {
            switch (event.type) {
                case RGFW_quit:
                    done = 1;
                    break;

                case RGFW_keyPressed:
                    /*
					 * I don't know a better way to do this without it causing
					 * bugs or having to modify PureDOOM
					*/
					if (event.key.value == RGFW_end || (event.key.value == RGFW_shiftL && RGFW_window_isKeyDown(window, RGFW_escape)))
                    {
						RGFW_window_showMouse(window, active_mouse);
                        if (active_mouse)
                            RGFW_window_unholdMouse(window);
                        else
                            RGFW_window_holdMouse(window);

                        active_mouse = !active_mouse;
                    }

                    doom_key_down(RGFWScancode2DOOM[event.key.value]);
                    break;

                case RGFW_keyReleased:
                    doom_key_up(RGFWScancode2DOOM[event.key.value]);
                    break;

                case RGFW_mouseButtonPressed:
                    if (active_mouse) doom_button_down(RGFWButton2DOOM[event.button.value]);
                    break;

                case RGFW_mouseButtonReleased:
                    if (active_mouse) doom_button_up(RGFWButton2DOOM[event.button.value]);
                    break;

                case RGFW_mousePosChanged:
                    if (active_mouse)
                    {
						doom_mouse_move(event.mouse.vecX * 10, 0);
					}
                    break;
            }
            if (done) break;
        }
        if (done) break;

        doom_update();

		i32 width, height;
		RGFW_window_getSize(window, &width, &height);

        resize.input_pixels = doom_get_framebuffer(4);
        resize.output_w = resize.output_subw= width;
        resize.output_h = resize.output_subh = height;
        stbir_resize_extended(&resize);

        RGFW_window_blitSurface(window, surface);
    }

#if defined(WIN32)
    if (midi_out_handle) midiOutClose(midi_out_handle);
#elif defined(__linux__)
    snd_rawmidi_close(midi_out_handle);
#endif

    RGFW_surface_free(surface);
	RGFW_FREE(buffer);

    ma_device_uninit(&device);
    RGFW_window_close(window);
    return 0;
}

