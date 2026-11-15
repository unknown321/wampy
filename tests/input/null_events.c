// Link the production Null backend against pipes and minimal callback stubs.
#include "internal.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

_GLFWlibrary _glfw;
static _GLFWwindow window;
static pthread_t owner;
static int hold, keys, errors, touch;
extern int wampyInputWasReset(void);

int __wrap_ioctl(int fd, unsigned long request, ...) {
    va_list args;
    va_start(args, request);
    unsigned char *state = va_arg(args, unsigned char *);
    va_end(args);
    memset(state, 0, (KEY_MAX + 8) / 8);
    state[KEY_H / 8] |= 1 << (KEY_H % 8);
    return 0;
}

void _glfwInputKey(_GLFWwindow *w, int key, int scancode, int action, int mods) {
    assert(pthread_equal(owner, pthread_self()));
    keys++;
    if (key == KEY_H) hold = action;
}
void _glfwInputMouseClick(_GLFWwindow *w, int key, int action, int mods) { touch++; }
void _glfwInputCursorPos(_GLFWwindow *w, double x, double y) {}
void _glfwInputError(int code, const char *description, ...) { errors++; }

static void event(int fd, int type, int code, int value) {
    struct input_event e = {0};
    e.type = type;
    e.code = code;
    e.value = value;
    assert(write(fd, &e, sizeof(e)) == sizeof(e));
}

int main(void) {
    int power[2], media[2], screen[2];
    assert(pipe2(power, O_NONBLOCK) == 0);
    assert(pipe2(media, O_NONBLOCK) == 0);
    assert(pipe2(screen, O_NONBLOCK) == 0);
    _glfw.keyboard.fdPower = power[0];
    _glfw.keyboard.fd = media[0];
    _glfw.touchscreen.fd = screen[0];
    _glfw.windowListHead = &window;
    owner = pthread_self();
    _glfwPollEventsNull();
    assert(hold == GLFW_PRESS && wampyInputWasReset());
    assert(!wampyInputWasReset());
    int before = keys;
    _glfwPollEventsNull();
    assert(keys == before); // no fabricated event on EAGAIN
    event(media[1], EV_KEY, KEY_H, GLFW_RELEASE);
    event(power[1], EV_KEY, KEY_POWER, GLFW_PRESS);
    event(power[1], EV_KEY, KEY_POWER, GLFW_RELEASE);
    _glfwPollEventsNull();
    assert(hold == GLFW_RELEASE && keys == before + 3);
    event(media[1], EV_SYN, SYN_DROPPED, 0);
    event(media[1], EV_KEY, KEY_H, GLFW_RELEASE);
    event(media[1], EV_SYN, SYN_REPORT, 0);
    _glfwPollEventsNull();
    assert(hold == GLFW_PRESS && wampyInputWasReset());
    before = keys;
    for (int i = 0; i < 40; i++) event(media[1], EV_KEY, KEY_ENTER, GLFW_PRESS);
    _glfwPollEventsNull();
    assert(keys == before + 32);
    _glfwPollEventsNull();
    assert(keys == before + 40);
    event(screen[1], EV_KEY, BTN_TOUCH, GLFW_PRESS);
    _glfwWaitEventsTimeoutNull(0.01);
    assert(touch == 1);
    close(media[1]);
    _glfwPollEventsNull();
    assert(_glfw.keyboard.fd == -1 && errors == 1);
    close(power[0]); close(power[1]); close(screen[0]); close(screen[1]);
    puts("PASS: owner thread, initial HOLD, POWER pair, EAGAIN, SYN_DROPPED, bounded batches, touch, EOF");
}
