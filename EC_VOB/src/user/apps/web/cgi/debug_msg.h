#ifndef DEBUG_MSG__H
#define DEBUG_MSG__H 1

#ifdef __cplusplus
extern "C" {
#endif

void debug_msg(const char *format, ...);
void set_debug_msg_level(int level);
int get_debug_msg_level();

#ifdef __cplusplus
}
#endif

#endif
