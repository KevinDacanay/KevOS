// include/signal.h

#ifndef _SIGNAL_H
#define _SIGNAL_H 1

// Signal constants - minimal set
#define SIGABRT 6  // Abnormal termination
#define SIGFPE  8  // Floating point exception
#define SIGILL  4  // Illegal instruction
#define SIGINT  2  // Interrupt (Ctrl+C)
#define SIGSEGV 11 // Segmentation fault
#define SIGTERM 15 // Termination request

#ifdef __cplusplus
extern "C" {
#endif

// Function to send a signal to the current process
int raise(int sig);

// Optional: Define a signal handler function type
typedef void (*__sighandler_t)(int);

// Function to set a signal handler (stub for now)
__sighandler_t signal(int sig, __sighandler_t handler);

#ifdef __cplusplus
}
#endif

#endif // _SIGNAL_H
