/**
 * Mancala
 * 4/1/2016
 * Basch
 **/

// TODOS
// Documentation
// Comment functions
// Sort functions and prototypes alphabetically

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define PODS 12
#define START_QTY 4
#define SEEDS (START_QTY * PODS)
#define STORES 2
#define HOLES_IN_BOARD (PODS + STORES)
#define STORE_P1_IDX (HOLES_IN_BOARD / 2 - 1)
#define STORE_P2_IDX (HOLES_IN_BOARD - 1)
#define POD_START_P1_IDX 0
#define POD_START_P2_IDX (HOLES_IN_BOARD / 2)
#define CPU_OFF -1
#define PLAYER_1 0
#define PLAYER_2 1
#define USER_MAX 16

int board[HOLES_IN_BOARD];

int check_pod_across(int pod_idx);
void clear_pods(void);
void draw_board(void);
int get_side(int pod_idx);
int get_side_sum(int pod_idx);
int getUserInput (char *s, int n);
bool in_own_store(int player, int hole);
void init_board(void);
bool is_side_empty(int side);
bool is_store(int hole);
int ltrim (char *s);
bool one_seed_in_pod (int pod_idx);
int parsestr (const char *s);
bool pod_is_empty (int pod_idx);
int rtrim (char *s);
bool set_gameover(bool *gameover);
int select_pod(int player);
int select_pod_cpu(int player);
void set_pod_empty(int pod_idx);
int switch_player(int player);
void transfer_seeds(void);
int traverse_board (int start_idx, int player);
int trim (char *s);
const char *turn_message(unsigned short code);
int ui_set_cpu_options (void);
int update_store (int store_idx, int seeds);

int main (void)
{
    // Init Game
    printf("#######\nMANCALA!\n2016\n#######\n");
    int turn_ctr = 0;
    bool gameover = false;
    int player;
    int cpu = ui_set_cpu_options();
    player = PLAYER_1;
    init_board();
    
    draw_board();
    
    // Game Loop
    while (true)
    {
        ++turn_ctr;
        printf("%sPlayer %d's Turn\n", player == cpu ? "CPU " : "", player + 1);
        
        bool go_again = false;
        
        // int selected = select_pod(player);
        int selected = (player == cpu) ? select_pod_cpu(player) : select_pod(player);
        draw_board();
        
        int last_hole = traverse_board(selected, player);
        int opposite_hole = check_pod_across(last_hole);
        int msg_num = 0;
        
        if (in_own_store(player, last_hole))
        {
            msg_num = 1;
            go_again = true;
        }
        else if (one_seed_in_pod(last_hole) && get_side(last_hole) == player && board[opposite_hole] > 0)
        {
            msg_num = 2;
            int player_store = (player == PLAYER_1 ? STORE_P1_IDX : STORE_P2_IDX);
            
            update_store(player_store, board[opposite_hole]);
            set_pod_empty(opposite_hole);
        }
        
        set_gameover(&gameover);
        if (gameover)
            break;
        
        draw_board();
        
        if (msg_num != 0)
            printf("%s\n", turn_message(msg_num));
        
        if (!go_again)
            player = switch_player(player);
    }
    
    // Game Over
    transfer_seeds();
    printf("Game Over: %d turns.\n", turn_ctr);
    
    if (board[STORE_P1_IDX] > board[STORE_P2_IDX])
    {
        printf("Player 1 wins with score of: %d.\n", board[STORE_P1_IDX]);
    }
    else if  (board[STORE_P1_IDX] < board[STORE_P2_IDX])
    {
        printf("Player 2 wins with score of: %d.\n", board[STORE_P2_IDX]);
    }
    else
    {
        printf("Game is tied at %d.\n", board[STORE_P1_IDX]);
    }
    
    draw_board();
    
    return 0;
}

void init_board(void)
{
    for (int hole = 0; hole <= HOLES_IN_BOARD; hole++)
    {
        if (is_store(hole))
        {
            set_pod_empty(hole);
        }
        else
        {
            board[hole] = START_QTY;
        }
    }
}

// Adds seeds to current value inside store
int update_store (int store_idx, int seeds)
{
    board[store_idx] += seeds;
    return board[store_idx];
}

void draw_board(void)
{
    
    // Line 1 - Player 1 Labels
    printf("             6  5  4  3  2  1       <-- P1\n");
    
    // Line 2 - Player 1 pods
    printf("       ---- ");
    for (int i = HOLES_IN_BOARD / 2 - 2; i >= 0; i--)
    {
        printf("%02d ", board[i]);
    }
    printf("----\n");
    
    // Line 3 - Player stores and middle border
    printf("        %02d", board[STORE_P1_IDX]);
    printf("  -- -- -- -- -- -- ");
    printf(" %02d\n", board[STORE_P2_IDX]);
    
    // Line 4 - Player 2 pods
    printf("       ---- ");
    for (int i = HOLES_IN_BOARD / 2; i < HOLES_IN_BOARD - 1; i++)
    {
        printf("%02d ", board[i]);
    }
    printf("----\n");
    
    // Line 5 - Player 2 Labels
    printf("P2 -->       1  2  3  4  5  6\n");
    
    // Botton Border
    printf("*************************************\n\n");
}

int parsestr (const char *s)
{
    if (strlen(s) != 1)
    {
        printf("Try again\n");
    }
    else if (toupper(s[0]) == 'Q')
    {
        char option[USER_MAX];
        printf("Do you want to quit? [Y/N]\n");
        
        getUserInput(option, sizeof(option));
        
        if (toupper(option[0]) == 'Y')
        {
            printf("Quitting game.\n");
            exit(0);
        }
    }
    else if (isdigit(s[0]))
    {
        int num = s[0] - '0';
        if (!(num >= 1 && num <= 6))
        {
            printf("Please pick a space between 1 and 6, not %d\n", num);
        }
        else
        {
            return num;
        }
    }
    else
    {
        printf("Try again\n");
    }
    
    return -1;
}

// Returns the index of where the last seed was dropped
int traverse_board (int start_idx, int player)
{
    
    int i = start_idx;
    // Remember seed count from selected hole
    int seeds = board[start_idx];
    set_pod_empty(start_idx);
    
    while (seeds > 0)
    {
        i = (i + 1) % HOLES_IN_BOARD;
        // Ensure opponent's store gets no seed
        if((player == PLAYER_1 && i != STORE_P2_IDX) || (player == PLAYER_2 && i != STORE_P1_IDX))
        {
            board[i]++;
            seeds--;
        }
    }
    
    return i;
}

// Returns index of board array on the pod across from index provided as argument
int check_pod_across(int pod_idx)
{
    if (is_store(pod_idx) || pod_idx > PODS || pod_idx < POD_START_P1_IDX)
    {
        return -1;
    }
    
    return PODS - pod_idx;
}

bool pod_is_empty (int pod_idx)
{
    return board[pod_idx] == 0;
}

bool one_seed_in_pod (int pod_idx)
{
    return board[pod_idx] == 1;
}

void set_pod_empty(int pod_idx)
{
    board[pod_idx] = 0;
}

void clear_pods(void)
{
    for (int i = 0; i <= PODS ; i++)
        if (!is_store(i))
            set_pod_empty(i);
}

int get_side_sum(int pod_idx)
{
    int side_sum = 0;
    for (int i = pod_idx; i != STORE_P1_IDX && i != STORE_P2_IDX; i++)
        side_sum += board[i];
    
    return side_sum;
}

// Returns numeric representaion of player 1 or 2
int get_side(int pod_idx)
{
    return pod_idx < POD_START_P2_IDX ? PLAYER_1 : PLAYER_2;
}

int getUserInput (char *s, int n)
{
    if ((fgets(s, n, stdin)) == NULL)
        return -1;
    
    return trim(s);
}

bool is_side_empty(int pod_idx)
{
    return get_side_sum(pod_idx) == 0;
}

int select_pod(int player)
{
    while (true)
    {
        char myStr[USER_MAX];
        int selected = 0;
        int selected_translate;
        int selected_idx;
        
        while (true)
        {
            printf("Select a space [1 - 6], or Quit Game [Q]\n");
            getUserInput(myStr, sizeof(myStr));
            
            if ((selected = parsestr(myStr)) > 0)
                break;
        }
        
        selected_translate = (player == PLAYER_1) ? selected : (selected + POD_START_P2_IDX);
        selected_idx = selected_translate - 1;
        
        if (is_store(selected_idx))
        {
            draw_board();
            printf("Hole %d is a store and cannot be selected. Please select a non-empty pod.\n", selected);
        }
        else if (pod_is_empty(selected_idx))
        {
            draw_board();
            printf("Pod %d does not contain any seeds. Please selected another pod.\n", selected);
        }
        else
        {
            return selected_idx;
        }
    }
}

int select_pod_cpu (int player)
{
    
    const int start = (player == PLAYER_1) ? 0 : POD_START_P2_IDX;
    const int store = (player == PLAYER_1) ? STORE_P1_IDX : STORE_P2_IDX;
    
    // Dumb algorithm
    for (int i = start; i < store; i++)
        if (!pod_is_empty(i))
            return i;
    
    // Have to put this here for compliler. Code should not branch here.
    printf("Error. CPU Player Algorithm Error!\nQuitting Game\n");
    exit(-1);
    
}

bool in_own_store(int player, int hole)
{
    return (player == PLAYER_1 && hole == STORE_P1_IDX)  || (player == PLAYER_2 && hole == STORE_P2_IDX);
}

bool is_store(int hole)
{
    return hole == STORE_P1_IDX || hole == STORE_P2_IDX;
}

// Checks for and sets gameover flag
bool set_gameover(bool *gameover)
{
    *gameover = is_side_empty(POD_START_P1_IDX) || is_side_empty(POD_START_P2_IDX);
    return gameover;
}

void transfer_seeds(void)
{
    // TODO - Works but make so only checks one.
    
    // Transfer remaining pieces to player on non-empty side
    update_store(STORE_P2_IDX, get_side_sum(POD_START_P2_IDX));
    update_store(STORE_P1_IDX, get_side_sum(POD_START_P1_IDX));
    
    clear_pods();
}

int switch_player(int player)
{
    return 1 - player;
}

const char *turn_message(unsigned short code)
{
    switch(code)
    {
        case 1 : return "You get to go again!";
        case 2 : return "Capture opponent's pieces";
        default : return NULL;
    }
}

int rtrim (char *s)
{
    int space_ctr = 0;
    char *ptr = s;
    
    ptr += (strlen(s) - 1 );
    int ctr = strlen(s);
    
    while (ctr >= 0)
    {
        if (!isspace(*ptr))
        {
            *(ptr + 1) = '\0';
            break;
        }
        else
        {
            space_ctr++;
        }
        
        ptr--;
        ctr--;
        
    }
    
    return space_ctr;
}

int ltrim (char *s)
{
    char tmp[USER_MAX];
    char *ptr;
    ptr = s;
    
    while (*ptr != '\0' && isspace(*ptr))
        ptr++;
    
    int space_ctr = ptr - s;
    
    if (space_ctr > 0)
    {
        strncpy(tmp, ptr, sizeof(char) * USER_MAX);
        strncpy(s, tmp, sizeof(char) * USER_MAX);
    }
    
    return space_ctr;
}

int trim (char *s)
{
    return rtrim(s) + ltrim(s);
}

int ui_set_cpu_options (void)
{
    char option[USER_MAX];
    
    while (true)
    {
        printf("Select from Options [0: \"2-Player VS\", 1: \"VS CPU P1\", 2: \"VS CPU P2\"]\n");
        getUserInput(option, sizeof(option));
        switch (option[0])
        {
            case '0' : return CPU_OFF;
            case '1' : return PLAYER_1;
            case '2' : return PLAYER_2;
            default: ;
        }
    }
}