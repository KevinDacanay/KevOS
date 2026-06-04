/**
 * @file shell.h
 * @brief Basic kernel shell interface.
 */

#ifndef KERNEL_SHELL_H
#define KERNEL_SHELL_H

extern "C" {
/**
 * @brief Starts the interactive kernel shell.
 * 
 * This function contains the main loop that prints the prompt,
 * reads user input, and dispatches commands. It should be called
 * after all hardware initialization is complete.
 */
void shell_main(void);
}

#endif