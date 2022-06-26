#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <windows.h>
//Hana Begin
//#include "../core/graphics.h"
//#include "../core/image.h"
//#include "../core/macro.h"
//#include "../core/platform.h"
//#include "../core/private.h"
//Hana End

//Hana Begin
#include "graphics.h"
#include "image.h"
#include "macro.h"
#include "platform.h"
//Hana End

struct window {
    HWND handle;
    HDC memory_dc;
    image_t *surface;
    /* common data */
    int should_close;
    char keys[KEY_NUM];
    char buttons[BUTTON_NUM];
    callbacks_t callbacks;
    void *userdata;
    HWND text_handle;
    char* text;
};

/* platform initialization */

static int g_initialized = 0;

#ifdef UNICODE
static const wchar_t *const WINDOW_CLASS_NAME = L"Class";
static const wchar_t *const WINDOW_ENTRY_NAME = L"Entry";
#else
static const char *const WINDOW_CLASS_NAME = "Class";
static const char *const WINDOW_ENTRY_NAME = "Entry";
#endif

/*
 * for virtual-key codes, see
 * https://docs.microsoft.com/en-us/windows/desktop/inputdev/virtual-key-codes
 */
static void handle_key_message(window_t *window, WPARAM virtual_key,
                               char pressed) {
    keycode_t key;
    switch (virtual_key) {
        case 'A':      key = KEY_A;     break;
        case 'D':      key = KEY_D;     break;
        case 'S':      key = KEY_S;     break;
        case 'W':      key = KEY_W;     break;
        case 'Q':      key = KEY_Q;     break;
        case 'E':      key = KEY_E;     break;
        case VK_SPACE: key = KEY_SPACE; break;
        default:       key = KEY_NUM;   break;
    }
    if (key < KEY_NUM) {
        window->keys[key] = pressed;
        if (window->callbacks.key_callback) {
            window->callbacks.key_callback(window, key, pressed);
        }
    }
}

static void handle_button_message(window_t *window, button_t button,
                                  char pressed) {
    window->buttons[button] = pressed;
    if (window->callbacks.button_callback) {
        window->callbacks.button_callback(window, button, pressed);
    }
}

static void handle_scroll_message(window_t *window, float offset) {
    if (window->callbacks.scroll_callback) {
        window->callbacks.scroll_callback(window, offset);
    }
}

static LRESULT CALLBACK process_message(HWND hWnd, UINT uMsg,
                                        WPARAM wParam, LPARAM lParam) {
    window_t *window = (window_t*)GetProp(hWnd, WINDOW_ENTRY_NAME);
    if (window == NULL) {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    } else if (uMsg == WM_CLOSE) {
        window->should_close = 1;
        return 0;
    } else if (uMsg == WM_KEYDOWN) {
        handle_key_message(window, wParam, 1);
        return 0;
    } else if (uMsg == WM_KEYUP) {
        handle_key_message(window, wParam, 0);
        return 0;
    } else if (uMsg == WM_LBUTTONDOWN) {
        handle_button_message(window, BUTTON_L, 1);
        return 0;
    } else if (uMsg == WM_RBUTTONDOWN) {
        handle_button_message(window, BUTTON_R, 1);
        return 0;
    } else if (uMsg == WM_LBUTTONUP) {
        handle_button_message(window, BUTTON_L, 0);
        return 0;
    } else if (uMsg == WM_RBUTTONUP) {
        handle_button_message(window, BUTTON_R, 0);
        return 0;
    } else if (uMsg == WM_MOUSEWHEEL) {
        float offset = GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
        handle_scroll_message(window, offset);
        return 0;
    } else {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

static void register_class(void) {
    ATOM class_atom;
    WNDCLASS window_class;
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = process_message;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = GetModuleHandle(NULL);
    window_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    window_class.lpszMenuName = NULL;
    window_class.lpszClassName = WINDOW_CLASS_NAME;
    window_class.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    class_atom = RegisterClass(&window_class);
    assert(class_atom != 0);
    UNUSED_VAR(class_atom);
}

static void unregister_class(void) {
    UnregisterClass(WINDOW_CLASS_NAME, GetModuleHandle(NULL));
}

static void initialize_path(void) {
#ifdef UNICODE
    wchar_t path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    *wcsrchr(path, L'\\') = L'\0';
    _wchdir(path);
    _wchdir(L"assets");
#else
    char path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    *strrchr(path, '\\') = '\0';
    _chdir(path);
    _chdir("assets");
#endif
}

void platform_initialize(void) {
    assert(g_initialized == 0);
    register_class();
    initialize_path();
    g_initialized = 1;
}

void platform_terminate(void) {
    assert(g_initialized == 1);
    unregister_class();
    g_initialized = 0;
}

/* window related functions */

static HWND create_window(const char *title_, int width, int height) {
    DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT rect;
    HWND handle;

#ifdef UNICODE
    wchar_t title[LINE_SIZE];
    mbstowcs(title, title_, LINE_SIZE);
#else
    const char *title = title_;
#endif

    rect.left = 0;
    rect.top = 0;
    rect.right = width;
    rect.bottom = height;
    AdjustWindowRect(&rect, style, 0);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    handle = CreateWindow(WINDOW_CLASS_NAME, title, style,
                          CW_USEDEFAULT, CW_USEDEFAULT, width, height,
                          NULL, NULL, GetModuleHandle(NULL), NULL);

    assert(handle != NULL);
    return handle;
}

/*
 * for memory device context, see
 * https://docs.microsoft.com/en-us/windows/desktop/gdi/memory-device-contexts
 */
static void create_surface(HWND handle, int width, int height,
                           image_t **out_surface, HDC *out_memory_dc) {
    BITMAPINFOHEADER bi_header;
    HBITMAP dib_bitmap;
    HBITMAP old_bitmap;
    HDC window_dc;
    HDC memory_dc;
    image_t *surface;

    surface = image_create(width, height, 4, FORMAT_LDR);
    free(surface->ldr_buffer);
    surface->ldr_buffer = NULL;

    window_dc = GetDC(handle);
    memory_dc = CreateCompatibleDC(window_dc);
    ReleaseDC(handle, window_dc);

    memset(&bi_header, 0, sizeof(BITMAPINFOHEADER));
    bi_header.biSize = sizeof(BITMAPINFOHEADER);
    bi_header.biWidth = width;
    bi_header.biHeight = -height;  /* top-down */
    bi_header.biPlanes = 1;
    bi_header.biBitCount = 32;
    bi_header.biCompression = BI_RGB;
    dib_bitmap = CreateDIBSection(memory_dc, (BITMAPINFO*)&bi_header,
                                  DIB_RGB_COLORS, (void**)&surface->ldr_buffer,
                                  NULL, 0);
    assert(dib_bitmap != NULL);
    old_bitmap = (HBITMAP)SelectObject(memory_dc, dib_bitmap);
    DeleteObject(old_bitmap);

    *out_surface = surface;
    *out_memory_dc = memory_dc;
}

window_t *window_create(const char *title, int width, int height, int text_width, int text_height, char* text) {
    window_t *window;
    HWND handle;
    image_t *surface;
    HDC memory_dc;
    HWND text_handle;

    assert(g_initialized && width > 0 && height > 0);

    handle = create_window(title, width, height);

    text_handle = CreateWindow(TEXT("static"), TEXT("static"), WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        0, 0, text_width, text_height,
        handle, HMENU(21), GetModuleHandle(NULL), NULL);

    SetBkColor(GetDC(text_handle), RGB(0, 0, 0));
    SetTextColor(GetDC(text_handle), RGB(255, 0, 0));

    create_surface(handle, width, height, &surface, &memory_dc);

    window = (window_t*)malloc(sizeof(window_t));
    memset(window, 0, sizeof(window_t));
    window->handle = handle;
    window->memory_dc = memory_dc;
    window->surface = surface;
    window->text = text;
    window->text_handle = text_handle;

    SetProp(handle, WINDOW_ENTRY_NAME, window);
    ShowWindow(handle, SW_SHOW);
    return window;
}

void window_destroy(window_t *window) {
    ShowWindow(window->handle, SW_HIDE);
    RemoveProp(window->handle, WINDOW_ENTRY_NAME);

    DeleteDC(window->memory_dc);
    DestroyWindow(window->handle);

    window->surface->ldr_buffer = NULL;
    image_release(window->surface);
    free(window);
}

int window_should_close(window_t *window) {
    return window->should_close;
}

void window_set_userdata(window_t *window, void *userdata) {
    window->userdata = userdata;
}

void *window_get_userdata(window_t *window) {
    return window->userdata;
}

static void present_surface(window_t *window) {
    HDC window_dc = GetDC(window->handle);
    HDC memory_dc = window->memory_dc;
    image_t *surface = window->surface;
    int width = surface->width;
    int height = surface->height;

    BitBlt(window_dc, 0, 0, width, height, memory_dc, 0, 0, SRCCOPY);

#ifdef UNICODE
        char* text = window->text;
        wchar_t* wc;
        int len = MultiByteToWideChar(CP_ACP, 0, text, strlen(text), NULL, 0);
        wc = new wchar_t[len + 1];
        MultiByteToWideChar(CP_ACP, 0, text, strlen(text), wc, len);
        wc[len] = '\0';
        SetWindowText(window->text_handle, wc);
        delete[] wc;
#else
        SetWindowText(window->text_handle, window->text);
#endif // !UNICODE
    
    ReleaseDC(window->handle, window_dc);
}

void window_draw_buffer(window_t *window, RenderBuffer *buffer) {
    //Hana Begin
    //private_blit_bgr(buffer, window->surface);
    //Hana End

    int width = window->surface->width;
    int height = window->surface->height;
    int r, c;

    assert(buffer->width == window->surface->width && buffer->height == window->surface->height);
    assert(window->surface->format == FORMAT_LDR && window->surface->channels == 4);

    for (r = 0; r < height; r++) {
        for (c = 0; c < width; c++) {
            int flipped_r = height - 1 - r;
            int src_index = (r * width + c) * 4;
            int dst_index = (flipped_r * width + c) * 4;
            unsigned char* src_pixel = &buffer->color_buffer[src_index];
            unsigned char* dst_pixel = &window->surface->ldr_buffer[dst_index];
            dst_pixel[0] = src_pixel[2];  /* blue */
            dst_pixel[1] = src_pixel[1];  /* green */
            dst_pixel[2] = src_pixel[0];  /* red */
        }
    }

    present_surface(window);
}

/* input related functions */

void input_poll_events(void) {
    MSG message;
    while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

int input_key_pressed(window_t *window, keycode_t key) {
    assert(key >= 0 && key < KEY_NUM);
    return window->keys[key];
}

int input_button_pressed(window_t *window, button_t button) {
    assert(button >= 0 && button < BUTTON_NUM);
    return window->buttons[button];
}

void input_query_cursor(window_t *window, float *xpos, float *ypos) {
    POINT point;
    GetCursorPos(&point);
    ScreenToClient(window->handle, &point);
    *xpos = (float)point.x;
    *ypos = (float)point.y;
}

void input_set_callbacks(window_t *window, callbacks_t callbacks) {
    window->callbacks = callbacks;
}

/* misc platform functions */

static double get_native_time(void) {
    static double period = -1;
    LARGE_INTEGER counter;
    if (period < 0) {
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        period = 1 / (double)frequency.QuadPart;
    }
    QueryPerformanceCounter(&counter);
    return counter.QuadPart * period;
}

float platform_get_time(void) {
    static double initial = -1;
    if (initial < 0) {
        initial = get_native_time();
    }
    return (float)(get_native_time() - initial);
}
