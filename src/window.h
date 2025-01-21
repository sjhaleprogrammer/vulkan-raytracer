#ifndef WINDOW_H
#define WINDOW_H


struct windowinfo {
    GLFWwindow* window;
    int width, height;
    char APP_LONG_NAME;
    char APP_SHORT_NAME;
};

void create_window(struct windowinfo *window, char *APP_LONG_NAME);

#endif