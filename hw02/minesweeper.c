#include <stdbool.h>
#include "minesweeper.h"
#include "stdio.h"
#include "stdlib.h"

#define UNUSED(A) (void) (A)

/* ************************************************************** *
 *                         HELP FUNCTIONS                         *
 * ************************************************************** */


int count_of_surrounding(size_t cols, size_t rows,
                         uint16_t board[rows][cols],
                         size_t col, size_t row, bool (*f)(uint16_t)) {
    int mine_count = 0;
    int offset_x[] = {0, 1, 1, 1, 0, -1, -1, -1};
    int offset_y[] = {-1, -1, 0, 1, 1, 1, 0, -1};
    for (int i = 0; i < 8; i++) {
        int nx = col + offset_x[i];
        int ny = row + offset_y[i];
        if (nx < 0 || nx > (int) cols - 1 || ny < 0 || ny > (int) rows - 1)
            continue;
        if (f(board[ny][nx]))
            mine_count++;
    }
    return mine_count;
}

uint16_t convert_from8_to16(uint8_t dataFirst, uint8_t dataSecond) {
    uint16_t dataBoth = 0x0000;
    dataBoth = dataFirst;
    dataBoth = dataBoth << 8;
    dataBoth |= dataSecond;
    return dataBoth;
}

uint8_t *convert_from16_to8(uint16_t dataAll) {
    static uint8_t arrayData[2] = {0x00, 0x00};

    *(arrayData) = (dataAll >> 8) & 0x00FF;
    arrayData[1] = dataAll & 0x00FF;
    return arrayData;
}

bool is_flag(uint16_t cell) {
    uint8_t *cell_decomposition = convert_from16_to8(cell);
    return cell_decomposition[0] == 'F' ||
           cell_decomposition[0] == 'f' ||
           cell_decomposition[0] == 'W' ||
           cell_decomposition[0] == 'w';
}

bool is_mine(uint16_t cell) {
    uint8_t *cell_decomposition = convert_from16_to8(cell);
    return cell_decomposition[0] == 'M' ||
           cell_decomposition[0] == 'm' ||
           cell_decomposition[0] == 'F' ||
           cell_decomposition[0] == 'f';
}

bool is_revealed(uint16_t cell) {
    uint8_t *cell_decomposition = convert_from16_to8(cell);
    return cell_decomposition[0] == '.' ||
           (cell_decomposition[0] >= '0' && cell_decomposition[0] <= '8');
}

int get_number(uint16_t cell) {
    uint8_t *cell_decomposition = convert_from16_to8(cell);
    if (is_mine(cell))
        return 0;
    return (int) cell_decomposition[1];
}

/* ************************************************************** *
 *                         INPUT FUNCTIONS                        *
 * ************************************************************** */

int is_valid_val(char val) {
    char all_values[] = {'X', 'x', 'M', 'm', 'F', 'f', '.', 'W', 'w'};
    for (int i = 0; i < 9; i++) {
        if (all_values[i] == val)
            return 0;
    }
    if (val >= '0' && val <= '8')
        return val - '0';
    else
        return -1;
}

bool set_cell(uint16_t *cell, char val) {
    if (cell == NULL)
        return false;
    int validation = is_valid_val(val);
    if (validation >= 0) {
        uint8_t surrounding_mines = 0;
        if (validation != 0)
            surrounding_mines = validation;
        *cell = convert_from8_to16(val, surrounding_mines);
        return true;
    }
    return false;
}

int load_board(size_t rows, size_t cols, uint16_t board[rows][cols]) {
    char input_val;
    size_t loaded_chars = 0;
    while ((input_val = getchar()) != EOF && loaded_chars < rows * cols) {
        if (is_valid_val(input_val) >= 0) {
            set_cell(&board[loaded_chars / cols][loaded_chars % cols], input_val);
            loaded_chars++;
            if (loaded_chars == rows * cols)
                break;
        }
    }
    if (loaded_chars < rows * cols)
        return -1;
    return postprocess(rows, cols, board);
}

void update_surr_mines(uint16_t *cell, uint8_t new_count) {
    uint8_t *cell_decomposition = convert_from16_to8(*cell);
    *cell = convert_from8_to16(cell_decomposition[0], new_count);
}

bool mine_in_corner(size_t rows, size_t cols, uint16_t board[rows][cols]) {
    return is_mine(board[0][0]) || is_mine(board[0][cols - 1]) ||
           is_mine(board[rows - 1][0]) || is_mine(board[rows - 1][cols - 1]);
}

int postprocess(size_t rows, size_t cols, uint16_t board[rows][cols]) {
    // TODO: Implement me
    int mines_count = 0;
    if (rows < MIN_SIZE || cols < MIN_SIZE ||
        cols > MAX_SIZE || rows > MAX_SIZE)
        return -1;
    if (mine_in_corner(rows, cols, board))
        return -1;
    for (int i = 0; i < (int) rows; i++) {
        for (int j = 0; j < (int) cols; j++) {
            int surr_mines = count_of_surrounding(cols, rows, board, j, i, &is_mine);
            uint8_t *cell_decomposition = convert_from16_to8(board[i][j]);
            if (is_revealed(board[i][j]) && cell_decomposition[0] != '.') {
                if (cell_decomposition[0] - '0' != surr_mines)
                    return -1;
            }
            update_surr_mines(&board[i][j], surr_mines);
            if (is_mine(board[i][j]))
                mines_count++;
        }
    }
    if (mines_count == 0)
        return -1;
    return mines_count;

}

/* ************************************************************** *
 *                        OUTPUT FUNCTIONS                        *
 * ************************************************************** */
void print_cols_numbering(size_t cols) {
    printf("   ");
    for (int i = 0; i < (int) cols; i++) {
        if (i > 9)
            printf(" %d ", i);
        else
            printf("  %d ", i);
    }
    printf("\n");
}

void print_top(size_t cols) {
    printf("   +");
    for (int i = 0; i < (int) cols; i++) {
        printf("---+");
    }
    printf("\n");
}

void print_row(size_t cols, size_t rows, uint16_t board[rows][cols], int row) {
    if (row < 10)
        printf(" %d |", row);
    else
        printf("%d |", row);
    for (int i = 0; i < (int) cols; i++) {
        char val = show_cell(board[row][i]);
        switch (val) {
            case 'X':
                printf("XXX");
                break;
            case 'F':
                printf("_F_");
                break;
            case ' ':
                printf("   ");
                break;
            case 'B':
                printf(" M ");
                break;
            default:
                printf(" %c ", val);
                break;
        }
        printf("|");
    }
    printf("\n");
}

int print_board(size_t rows, size_t cols, uint16_t board[rows][cols]) {
    print_cols_numbering(cols);
    for (int i = 0; i < (int) rows; i++) {
        print_top(cols);
        print_row(cols, rows, board, i);
    }
    print_top(cols);
    return 0;
}

char show_cell(uint16_t cell) {
    uint8_t *cell_decomp = convert_from16_to8(cell);
    if (cell_decomp[0] == 'm' || cell_decomp[0] == 'M' ||
        cell_decomp[0] == 'X' || cell_decomp[0] == 'x')
        return 'X';
    else if (cell_decomp[0] == 'f' || cell_decomp[0] == 'F' ||
             cell_decomp[0] == 'W' || cell_decomp[0] == 'w')
        return 'F';
    else if (cell_decomp[0] >= '1' && cell_decomp[0] <= '8')
        return cell_decomp[0];
    else if (cell_decomp[0] == '0')
        return ' ';
    else if (cell_decomp[0] == 'B')
        return 'M';
    else if (cell_decomp[0] == '.') {
        if (cell_decomp[1] == 0)
            return ' ';
        return cell_decomp[1] + '0';
    } else
        return -1;
}

/* ************************************************************** *
 *                    GAME MECHANIC FUNCTIONS                     *
 * ************************************************************** */

int reveal_cell(size_t rows, size_t cols, uint16_t board[rows][cols], size_t row, size_t col) {
    if (row >= rows || col >= cols || (int) row < 0 || (int) col < 0)
        return -1;
    int ret_val = reveal_single(&board[row][col]);
    if (get_number(board[row][col]) == 0 && ret_val == 0)
        reveal_floodfill(rows, cols, board, row, col);
    return ret_val;
}

int reveal_single(uint16_t *cell) {
    if (cell == NULL)
        return -1;
    uint8_t *cell_decomposition = convert_from16_to8(*cell);
    int count = cell_decomposition[1];
    if (is_revealed(*cell) || is_flag(*cell))
        return -1;
    else if (is_mine(*cell)) {
        *cell = convert_from8_to16('B', count);
        return 1;
    }
    *cell = convert_from8_to16(cell_decomposition[1] + '0', count);
    return 0;
}

void reveal_floodfill(size_t rows, size_t cols, uint16_t board[rows][cols], size_t row, size_t col) {
    int offset_x[] = {0, 1, 1, 1, 0, -1, -1, -1};
    int offset_y[] = {-1, -1, 0, 1, 1, 1, 0, -1};
    for (int i = 0; i < 8; i++) {
        int nx = col + offset_x[i];
        int ny = row + offset_y[i];
        if (nx < 0 || nx > (int) cols - 1 || ny < 0 || ny > (int) rows - 1)
            continue;
        if (is_revealed(board[ny][nx]))
            continue;
        flag_cell(rows, cols, board, ny, nx);
        reveal_single(&board[ny][nx]);
        if (get_number(board[ny][nx]) == 0)
            reveal_floodfill(rows, cols, board, ny, nx);
    }
}

int flag_cell(size_t rows, size_t cols, uint16_t board[rows][cols], size_t row, size_t col) {
    int flag_count = 0;
    uint8_t* cell_decomp = convert_from16_to8(board[row][col]);
    char symbol = cell_decomp[0];
    if (is_revealed(board[row][col]))
        return INT16_MIN;
    else if (symbol == 'F' || symbol == 'f')
        set_cell(&board[row][col], 'M');
    else if (symbol == 'W' || symbol == 'w')
        set_cell(&board[row][col], 'X');
    else {
        if (is_mine(board[row][col]))
            set_cell(&board[row][col], 'F');
        else
            set_cell(&board[row][col], 'W');
    }
    int mines_count = postprocess(rows, cols, board);
    for (int i = 0; i < (int) rows; i++) {
        for (int j = 0; j < (int) cols; j++) {
            if (is_flag(board[i][j]))
                flag_count++;
        }
    }
    printf("%c\n", symbol);
    return mines_count - flag_count;
}

bool is_solved(size_t rows, size_t cols, uint16_t board[rows][cols]) {
    int unrevealed = 0;
    for (int i = 0; i < (int) rows; i++) {
        for (int j = 0; j < (int) cols; j++) {
            if (!is_revealed(board[i][j]))
                unrevealed++;
        }
    }
    return unrevealed == postprocess(rows, cols, board);
}

/* ************************************************************** *
 *                         BONUS FUNCTIONS                        *
 * ************************************************************** */
int randint(int n) {
    if ((n - 1) == RAND_MAX) {
        return rand();
    } else {
        int end = RAND_MAX / n;
        end *= n;
        int r;
        while ((r = rand()) >= end);
        return r % n;
    }
}


int generate_random_board(size_t rows, size_t cols, uint16_t board[rows][cols], size_t mines) {
    if (rows < MIN_SIZE || rows > MAX_SIZE ||
        cols < MIN_SIZE || cols > MAX_SIZE)
        return -1;
    if (mines == 0 || mines > rows * cols - 4)
        return -1;
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            set_cell(&board[i][j], 'X');
        }
    }
    while (mines != 0) {
        int rand_x = randint(cols - 1);
        int rand_y = randint(rows - 1);
        if ((rand_x == 0 && rand_y == 0) || is_mine(board[rand_y][rand_x]))
            continue;
        set_cell(&board[rand_y][rand_x], 'M');
        mines--;
    }
    return postprocess(rows, cols, board);
}

bool is_covered(uint16_t cell) {
    uint8_t *cell_decomp = convert_from16_to8(cell);
    return cell_decomp[0] == 'X' || cell_decomp[0] == 'x';
}

void set_surrounding(size_t rows, size_t cols, uint16_t board[rows][cols], size_t row, size_t col) {
    int offset_x[] = {0, 1, 1, 1, 0, -1, -1, -1};
    int offset_y[] = {-1, -1, 0, 1, 1, 1, 0, -1};
    for (int i = 0; i < 8; i++) {
        int nx = col + offset_x[i];
        int ny = row + offset_y[i];
        if (nx < 0 || nx > (int) cols - 1 || ny < 0 || ny > (int) rows - 1)
            continue;
        if (is_covered(board[ny][nx]))
            set_cell(&board[ny][nx], 'F');
    }
}

int find_mines(size_t rows, size_t cols, uint16_t board[rows][cols]) {
    if (rows < MIN_SIZE || rows > MAX_SIZE ||
        cols < MIN_SIZE || cols > MAX_SIZE)
        return -1;
    char input_val;
    int found = 0;
    size_t loaded_chars = 0;
    while ((input_val = getchar()) != EOF && loaded_chars < rows * cols) {
        if (input_val == 'x' || input_val == 'X' ||
            (input_val <= '8' && input_val >= '0')) {
            set_cell(&board[loaded_chars / cols][loaded_chars % cols], input_val);
            loaded_chars++;
            if (loaded_chars == rows * cols)
                break;
        }
    }
    if (loaded_chars < rows * cols)
        return -1;
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            uint8_t *cell_decomp = convert_from16_to8(board[i][j]);
            char symbol = cell_decomp[0];
            if (is_covered(board[i][j]) || symbol == '0' || symbol == 'F')
                continue;
            int value = symbol - '0';
            int surr_covered = count_of_surrounding(cols, rows, board, j, i, &is_covered);
            if (value == surr_covered) {
                set_surrounding(rows, cols, board, i, j);
                found += surr_covered;
            }

        }
    }
    if (found == 0)
        return -1;
    return found;
}

