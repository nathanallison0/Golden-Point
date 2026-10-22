#include <debugging.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <SDL3Start.h>
#include <DebugTerminal.h>
#include <BasicFont.h>
#include <SaveSTates.h>
#include <engine.h>
#include <constants.h>
#include <SaveSTates.h>

const rgb DG_COLORS[] = {
    C_WHITE,
    C_YELLOW,
    C_RED,
    C_BLUE,
    C_PURPLE,
    C_GREEN,
    C_BLACK
};

char_varlabel char_vls[CHAR_VLS_LEN] = {
    {"show grid lines",  &show_grid_lines},
    {"render walls",  &fp_show_walls},
    {"show mouse coords",  &show_mouse_coords},
    {"show fps", &show_fps},
    {"cap fps", &cap_fps},
    {"show weapon", &fp_show_weapon},
    {"aa level", &aa_level},
    {"enemies behave", &do_enemies},
    {"hide mouse", &hide_mouse},
    {"show crosshair", &show_crosshair},
    {"floor opt", &tester}
};

flt_varlabel flt_vls[FLT_VLS_LEN] = {
    {"sensitivity", &player_sensitivity},
    {"enemy range", &enemy_range},
    {"sky off", &sky_off}
};
alstring *terminal_input;
DT_Interpreter *terminal;

#define CMD_DIR_NAME "./cmd/"
DIR *cmd_dir;

rgb get_dg_color_rgb(DG_Color c) {
    return DG_COLORS[c];
}

int num_fill_dgps = 0;
int max_fill_dgps = FPS / 2;
size_t num_temp_dgps = 0;
fill_dgp *fill_dgp_head;
fill_dgp *fill_dgp_tail;
temp_dgp *temp_dgp_list = NULL;

temp_dgp *get_temp_dgp(int index) {
    return temp_dgp_list + index;
}

size_t get_num_temp_dgps(void) {
    return num_temp_dgps;
}

void clear_temp_dgps(void) {
    num_temp_dgps = 0;
    temp_dgp_list = NULL;
}

fill_dgp *get_fill_dgp_head(void) {
    return fill_dgp_head;
}

void underscore_space(char *str) {
    char *index;
    while ((index = strchr(str, '_'))) {
        *index = ' ';
    }
}

// Convert from string to number
char str_num(char *str, float *dest) {
    // Check for true or false
    if (strcmp(str, "true") == 0) {
        *dest = 1;
        return TRUE;
    }
    if (strcmp(str, "false") == 0) {
        *dest = 0;
        return TRUE;
    }

    // Convert to float
    float temp = atof(str);

    // If zero was returned but the text is not zero,
    // return failure
    if (temp == 0 && str[0] != '0') {
        return FALSE;
    }

    *dest = temp;
    return TRUE;
}

void CMD_echo(int num_args, char **args) {
    if (num_args == 0) {
        DT_ConsolePrint(" ");
        return;
    }
    
    DT_CombineArgs(args, 0, num_args, ' ');
    DT_ConsolePrintln(args[0]);
}

void CMD_set(int num_args, char **args) {
    // If there are zero arguments, print all the variables
    if (num_args == 0) {
        for (int i = 0; i < CHAR_VLS_LEN; i++) {
            DT_ConsolePrintf("%s: %d\n", char_vls[i].label, (int) *char_vls[i].value);
        }
        for (int i = 0; i < FLT_VLS_LEN; i++) {
            DT_ConsolePrintf("%s: %f\n", flt_vls[i].label, *flt_vls[i].value);
        }
    } else {
        // Convert underscores in the first argument to spaces
        underscore_space(args[0]);

        // Find varlabel with a label corresponding to the first argument
        void *varlabel = NULL;
        char is_char = FALSE;

        // Look for int varlabel
        for (int i = 0; i < CHAR_VLS_LEN; i++) {
            if (strcmp(args[0], char_vls[i].label) == 0) {
                varlabel = char_vls + i;
                is_char = TRUE;
                break;
            }
        }

        // Look for float varlabel
        if (!is_char) {
            for (int i = 0; i < FLT_VLS_LEN; i++) {
                if (strcmp(args[0], flt_vls[i].label) == 0) {
                    varlabel = flt_vls + i;
                    break;
                }
            }
        }

        if (varlabel) {
            // If there is one argument, look for a varlabel with that label and
            // print if found
            if (num_args == 1) {
                // Print char
                if (is_char) DT_ConsolePrintf("%s: %d\n", *((Uint8 **) varlabel), (int) *((char_varlabel *) varlabel)->value);

                // Print float
                else         DT_ConsolePrintf("%s: %f\n", *((char **) varlabel), *((flt_varlabel *) varlabel)->value);
            } else if (num_args == 2) {
                // If there are two arguments, try to set the variable using the second argument

                // Get second argument as a number
                // If second argument is valid, set it to the varlabel value
                float new_num;
                char result = str_num(args[1], &new_num);
                
                if (result) {
                    if (is_char) *((char_varlabel *) varlabel)->value = (Uint8) new_num;
                    else         *((flt_varlabel *)varlabel)->value = new_num;
                } else {
                    DT_ConsolePrintf("Err cannot interpret '%s' as a number\n", args[1]);
                }
            }
        } else {
            if (num_args > 2) {
                DT_ConsolePrintln("Err expected from 0 to 2 arguments");
            } else {
                DT_ConsolePrintf("Err no such variable '%s'\nTo reference a variable with spaces, replace spaces with underscores\n", args[0]);
            }
        }
    }
}

void CMD_state(int num_args, char **args) {
    #if __EMSCRIPTEN__
    DT_ConsolePrintln("Feature unavailable");
    #else
    float x, y, rot;

    // If there are zero arguments, print all states
    if (num_args == 0) {
        int i = 0;
        while (SST_LoadState(i, &x, &y, &rot)) {
            DT_ConsolePrintf("%*d %f %f %f\n", 2, i++, x, y, rad_deg(rot));
        }
    } else {
        // Saving a state
        if (strcmp(args[0], "add") == 0) {
            if (num_args == 1) {
                SST_AddState(player->x, player->y, player->angle);
            } else if (num_args == 4) {
                float nums[3];
                int i = -1;
                while (++i < 3) {
                    if (!str_num(args[i + 1], nums + i)) {
                        DT_ConsolePrintf("Err cannot interpret '%s' as a number\n", args[i + 1]);
                        return;
                    }
                }
                SST_AddState(nums[0], nums[1], deg_rad(nums[2]));
            } else {
                DT_ConsolePrintln("Err expected 'add' [x y rot]");
            }
        } else if (strcmp(args[0], "load") == 0) {
            // Loading a state
            if (num_args == 2) {
                float num;
                if (str_num(args[1], &num)) {
                    SST_LoadState(num, &player->x, &player->y, &player->angle);
                } else {
                    DT_ConsolePrintf("Err cannot interpret '%s' as a number\n", args[1]);
                }
            } else {
                DT_ConsolePrintln("Err expected 'load' and the number of the state to load");
            }
        } else if (strcmp(args[0], "remove") == 0) {
            // Removing a state
            if (num_args == 2) {
                float num;
                if (str_num(args[1], &num)) {
                    SST_RemoveState(num);
                } else {
                    DT_ConsolePrintf("Err cannot interpret '%s' as a number\n", args[1]);
                }
            } else {
                DT_ConsolePrintln("Err expected 'remove' and the number of the state to remove");
            }
        } else if (strcmp(args[0], "clear") == 0) {
            // Clearing all states
            if (num_args == 1) {
                SST_ClearStates();
            } else {
                DT_ConsolePrintln("Err expected 'clear' and no other arguments");
            }
        } else {
            DT_ConsolePrintln("Err expected 'add', 'load', 'remove', or 'clear'");
        }
    }
    #endif
}

void CMD_run(int num_args, char **args) {
    #if __EMSCRIPTEN__
    DT_ConsolePrintln("Feature unavailable");
    #else
    if (num_args != 1) {
        DT_ConsolePrintln("Err expected only name of program to run");
        return;
    }

    // Search the directory for the file
    rewinddir(cmd_dir);

    struct dirent *entry;
    while ((entry = readdir(cmd_dir))) {
        // Skip . and ..
        if (entry->d_name[0] == '.') {
            continue;
        }

        // If name matches, execute each line of the file
        if (strcmp(entry->d_name, args[0]) == 0) {
            char *path;
            asprintf(&path, CMD_DIR_NAME "%s", args[0]);
            FILE *f = fopen(path, "r");
            free(path);

            char line[51];
            char *newline;
            while (fgets(line, 51, f)) {
                // Remove newline
                if ((newline = strchr(line, '\n'))) {
                    *newline = '\0';
                }

                // Execute line
                DT_InterpretCommand(terminal, line);
            }

            return;
        }
    }

    DT_ConsolePrintf("Err program '%s' does not exist\n", args[0]);
    #endif
}

const DT_Command terminal_commands[] = {
    {"echo", CMD_echo},
    {"set", CMD_set},
    {"state", CMD_state},
    {"run", CMD_run}
};

char *get_terminal_input_text(void) {
    return terminal_input->text;
}

void terminal_add_char(char c) {
    alstring_append(terminal_input, c);
}

void terminal_pop_char(void) {
    alstring_pop(terminal_input);
}

void terminal_interpret_command(void) {
    DT_ClearConsole();
    
    if (!DT_InterpretCommand(terminal, terminal_input->text)) {
        DT_ConsolePrintln("Err no such command");
    }

    alstring_clear(terminal_input);
}

void add_fill_dgp(float x, float y, DG_Color color_index, int *num_dgps, int max_dgps, struct fill_dgp **head, struct fill_dgp **tail) {
    struct fill_dgp *p = malloc(sizeof(*p));
    p->x = x;
    p->y = y;
    p->color_index = color_index;
    p->next = NULL;
    if (*head) {
        (*tail)->next = p;
        *tail = p;
    } else {
        *head = p;
        *tail = p;
    }

    (*num_dgps)++;
    if (*num_dgps > max_dgps) {
        struct fill_dgp *old_head = *head;
        *head = (*head)->next;
        free(old_head);
        (*num_dgps)--;
    }
}

void add_temp_dgp(float x, float y, DG_Color color_index, size_t *num_dgps, struct temp_dgp **list) {
    (*num_dgps)++;
    *list = realloc(*list, sizeof(struct temp_dgp) * *num_dgps);
    struct temp_dgp *p = *list + (*num_dgps - 1);
    p->x = x;
    p->y = y;
    p->color_index = color_index;
}

char check(char* msg) {
    puts(msg);
    return TRUE;
}

void init_debugging(void) {
    terminal = DT_CreateInterpreter(terminal_commands, sizeof(terminal_commands) / sizeof(DT_Command));

    // Terminal input init
    terminal_input = alstring_init(5);

    // Set save state mode
    SST_SetMode(SST_MODE_BINARY);

    // Load command files directory
    cmd_dir = opendir(CMD_DIR_NAME);

    char *start_file = "start.dt";
    if (cmd_dir) {
        CMD_run(1, &start_file);
    } else {
        printf("Could not open command directory\n");
    }
}

void debugging_end(void) {
    closedir(cmd_dir);
    alstring_destroy(terminal_input);
    DT_DestroyInterpreter(terminal);

    // Clear fill dgps
    fill_dgp *p = fill_dgp_head;
    while (p) {
        fill_dgp *next = p->next;
        free(p);
        p = next;
    }

    // Clear temp dgps
    if (temp_dgp_list) {
        free(temp_dgp_list);
    }
}