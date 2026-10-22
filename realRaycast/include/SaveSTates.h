#ifndef _GPOINT_STATE_H_
#define _GPOINT_STATE_H_

#include <SDL3Start.h>

enum {
    SST_MODE_TEXT,
    SST_MODE_BINARY
};
typedef Uint8 SST_Mode;

/**
 * Set the mode of read/write for the state file.
 * 
 * \param mode The mode to set to
 */
void SST_SetMode(SST_Mode mode);

/**
 * Write a new state to file.
 * 
 * \param player_x The player x to write
 * \param player_y The player y to write
 * \param player_rot The player rotation to write
 * \return `1` on success or `0` on failure
 */
int SST_AddState(float player_x, float player_y, float player_rot);

/**
 * Load a state from file.
 * 
 * \param state_num Index number of the state to load
 * \param player_x Pointer to the player x to write to
 * \param player_y Pointer to the player y to write to
 * \param player_rot Pointer to the player rotation to write to
 * \return `1` on success or `0` on failure
 */
int SST_LoadState(int state_num, float *player_x, float *player_y, float *player_rot);

/**
 * Remove a state.
 * 
 * \param state_num Index number of the state to remove
 * \return `1` on success or `0` on failure
 */
int SST_RemoveState(int state_num);

/**
 * Clear all states.
 * 
 * \return `1` on success or `0` on failure
 */
int SST_ClearStates(void);

/**
 * Sync state files between different modes.
 * 
 * \return `1` on success or `0` on failure
 */
int SST_SyncStates(void);

#endif