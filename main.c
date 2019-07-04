#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <raylib.h>
#define bacteria 'b'
#define child    'c'
#define food     'f'
#define space    's'
#define bacteria_color BLUE
#define food_color     LIME

// Initialize with screen dimensions
//----------------------------------
const int ScreenHeight = 800;
const int ScreenWidth = 600;
//----------------------------------

// Global variables
//----------------------------------
// Wall's border
int border = 10;
int food_energy = 100;

// Number of start bacteria and food
int food_counter = 1000;
int bacteria_counter = 100;

// Pause button
bool pause = true;
//---------------------------------

/*  Type of cells:
        b - bacteria
        f - food
        s - space
*/
typedef struct
{
    char type;
    Color color;
}micro_type;

// Simple DNA structure
typedef struct
{
    char last_transform;
    int position;
    int ma_width;
    int ma_height;
    int **matrix;
}DNA;

typedef struct
{
    int   health;
    int   speed;
    int   size;
    Color color;
    DNA   DNA;
}type_bacteria;

typedef struct Type Type;

typedef struct Type
{
    char    type;
    int     index;
    Color   color;
    union
    {
        micro_type    inanimate;
        type_bacteria bacterium;
    };
    Type    *next;
};

// Coordinates register
typedef struct
{
    int x;
    int y;
    struct Coord_Reg *next;
}Coord_Reg;


// Temp buff
//--------------
Type *buff_obj;
//--------------

// Main menu
void menu();

// Return a random number every frame
int random(int n);

// Add an object in list
void add_object(Type **objects, int *index, char type);

// Generate type
void generate_type(int ***world, Type **objects, int *index, int n, char type);

// Genetics
//-------------------------------------------------
DNA gen_DNA(int width, int height);
//-------------------------------------------------

// Extract object from list
Type extract_by_index(Type **objects, int index);

// Destroy object
void destr_object(Type **objects, int index);

// Free object's memory
void free_object(Type *object);

// Register
//------------------------------------------------------------------
// Add data in register
void add_in_reg(Coord_Reg **reg, int x, int y);

// Verify if coordinate are contains in register
bool verify_reg(Coord_Reg *reg, int x, int y);
//------------------------------------------------------------------

// Verify if coordinates don't exit from world
// If sign is space then we don't verify this coordinate
bool is_border(int x, int y, int size, int speed, char x_sign, char y_sign);

// Give some behavior to every bacterium
int bacterium_moving(int *x, int *y, int rand_num, int size, int speed);

// Determine what action form DNA's matrix need to be used
void DNA_action_coord(DNA curr_DNA, int *m, int *n);

// Determine in what border bacterium stuck
char what_border(int x, int y, int size);

// Transform DNA when bacterium meet wall
void transform_DNA(DNA temp_DNA, char name_border);

// Verify collision between two bacteria
int verify_collision(int **world, Coord_Reg *scnd_coord, int i, int j, int x, int y, int size);

// If bacterium meet another one then multiply
void multiply(int ***world, Type **objects, int *index, Type thrs_parent, Coord_Reg thrs_coord,
                                                        Type scnd_parent, Coord_Reg scnd_coord);

// Draw statistic of selected bacteria
void draw_bacterium_stats(int x, int y, int index, int health, int size);

// Print list with objects
void print_obj(Type *objects);

// Main
int main()
{
    // World in a 2D matrix
    // World contains indexes of all objects form object's list
    //---------------------------------------------------------
    int **world;

    // Allocation memory to world's matrix
    world = malloc(sizeof(int*) * ScreenWidth);

    for(int i = 0; i < ScreenWidth; i++)
        world[i] = malloc(sizeof(int) * ScreenHeight);

    // Initialize world
    for(int i = 0; i < ScreenWidth; i++)
        for(int j = 0; j < ScreenHeight; j++)
            world[i][j] = 0;
    //---------------------------------------------------------

    // List with objects
    Type *objects = NULL;

    // Objects Indexes
    int index = 1;

    // Initialize Window
    InitWindow(ScreenWidth, ScreenHeight, "Pandora");
    SetTargetFPS(30);

    // Generate bacteria and food
    //----------------------------------------------------------
    generate_type(&world, &objects, &index, food_counter, food);
    generate_type(&world, &objects, &index, bacteria_counter, bacteria);
    //----------------------------------------------------------

    // Start with Main Menu
    //menu();

    // Temporal variables
    //--------------------------------------------------
    int   x, y;
    int   temp;

    // Thirst and second object
    Type obj,
         scnd_obj;

    int n = 0, m = 0;
    int scnd_index = 0;

    int move;

    // Statistics used for draw_bacterium_stats
    Type stats;
         stats.index = 0;

    // Thirst and second parent's coordinate
    Coord_Reg thrs_coord,
              scnd_coord;

    // Food's object
    micro_type obj_food;

    // Bacteria's object
    type_bacteria obj_bacteria;
    //--------------------------------------------------

    // Register
    Coord_Reg *reg = NULL;

    while(!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(WHITE);

            for(int i = 0; i < ScreenWidth; i++)
                for(int j = 0; j < ScreenHeight; j++)
                    if(world[i][j] != 0)
                    {
                        obj = extract_by_index(&objects, world[i][j]);

                        if(obj.type == space)
                            world[i][j] = 0;
                        else if(obj.type == food)
                            {
                                obj_food = obj.inanimate;
                                DrawRectangle(i, j, 1, 1, obj_food.color);
                            }
                            else if(obj.type == bacteria || obj.type == child)
                            {
                                obj_bacteria = obj.bacterium;
                                x = i, y = j;
                                if(!verify_reg(reg, x, y))
                                {
                                    DNA_action_coord(obj_bacteria.DNA, &m, &n);
                                    move = bacterium_moving(&x, &y, obj_bacteria.DNA.matrix[m][n], obj_bacteria.size,
                                                                                                   obj_bacteria.speed);



                                    if(move == -1 && obj_bacteria.DNA.last_transform != what_border(x, y, obj_bacteria.size))
                                        transform_DNA(obj_bacteria.DNA, what_border(x, y, obj_bacteria.size));

                                    // Collision and multiply
                                    //-------------------------------------------------

                                    // Extract the secondary index with which the first object interacts
                                    scnd_index = verify_collision(world, &scnd_coord, i, j, x, y, obj_bacteria.size);


                                    // Work with last member
                                    //-----------------------------------------------------------
                                    if(bacteria_counter == 1)
                                    {
                                        for(int s = 0; s < obj_bacteria.DNA.ma_width; s++)
                                        {
                                            printf("\n");
                                            for(int t = 0; t < obj_bacteria.DNA.ma_height; t++)
                                                printf("%d ", obj_bacteria.DNA.matrix[s][t]);
                                        }
                                        printf("\nIndex = (%d)", world[i][j]);
                                        abort();
                                    }

                                    //-----------------------------------------------------------

                                    if(scnd_index >= 0)
                                    {
                                        // Coordinate of thirst object
                                        thrs_coord = (Coord_Reg){.x = i,
                                                                 .y = j};

                                        // Extract second object
                                        scnd_obj = extract_by_index(&objects, scnd_index);

                                        // If bacteria meets food then increase his health
                                        if(scnd_obj.type == food)
                                        {
                                            increase_health(&objects, world[i][j]);
                                            destr_object(&objects, scnd_index);
                                            food_counter--;
                                        }

                                        // If second object is bacteria then multiply
                                        if(obj.type == bacteria && scnd_obj.type == bacteria)
                                            multiply(&world, &objects, &index, obj,      thrs_coord,
                                                                               scnd_obj, scnd_coord);

                                        //-------------------------------------------------

                                        // If bacterium health <= 0 then destroy object
                                        if(obj_bacteria.health <= 0)
                                        {
                                            destr_object(&objects, world[i][j]);
                                            bacteria_counter--;
                                        }

                                    }

                                    DrawRectangle(x, y, obj_bacteria.size, obj_bacteria.size, obj_bacteria.color);

                                    add_in_reg(&reg, x, y);

                                    temp = world[x][y];
                                    world[x][y] = world[i][j];
                                    world[i][j] = temp;
                                }

                                // Draw bacterium statistics
                                //-----------------------------------------------------------------
                                if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                                    if(IsMouseHere(x - 4, y - 4, 10, 10))
                                    {
                                        stats.index = world[x][y];
                                        stats.bacterium = obj_bacteria;
                                    }

                                if(world[x][y] == stats.index && stats.index != 0)
                                    draw_bacterium_stats(x, y, stats.index, obj_bacteria.health, 3);
                                //-----------------------------------------------------------------
                            }
                    }

            // Every time generate food
            if(food_counter < 1500)
            {
                generate_type(&world, &objects, &index, 1, food);
                food_counter++;
            }

            free_reg(&reg);

            DrawFPS(10, 10);
        EndDrawing();
    }

    // De-Initialization
    //-----------------------------------------------

    CloseWindow();

    //-----------------------------------------------

    // Free variables
    //-----------------------------------------------
    for(int i = 0; i < ScreenWidth; i++)
        free(world[i]);
    free(world);

    Type *current = objects;
    Type *cursor;
    while(current != NULL)
    {
        cursor = current->next;
        free_object(objects);
        current = cursor;
    }
    //-----------------------------------------------

    return 0;
}

int IsMouseHere(int x, int y, int width, int height)
{
    // Get mouse coordinates
    int mouse_x = GetMouseX();
    int mouse_y = GetMouseY();

    // Determine if mouse if on the coordinates
    if(mouse_x >= x && mouse_x <= (x + width) &&
       mouse_y >= y && mouse_y <= (y + height))
        return 1;
    return 0;
}

// Main Menu
void menu()
{
    Color btn_color;

    Image img = LoadImage("image.jpg");

    Texture2D btn_texture = LoadTextureFromImage(img);
    UnloadImage(img);

    while(1)
    {
        // Drawing Menu
        BeginDrawing();

            ClearBackground(WHITE);

            // Draw start button
            //----------------------------------------------
            if(IsMouseHere(100, 400, img.width, img.height))
                btn_color = WHITE;
            else
                btn_color = BLUE;

            DrawTexture(btn_texture, 100, 400, btn_color);
            //----------------------------------------------

            // Draw exit button
            //----------------------------------------------
            if(IsMouseHere(300, 400, 200, 100))
                btn_color = BLUE;
            else
                btn_color = RED;

            DrawRectangle(300, 400, 200, 100, btn_color);
            //----------------------------------------------

        EndDrawing();



        // If Mouse Right Button was pressed start button then go to game
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && IsMouseHere(100, 400, img.width, img.height))
            break;

        // If Mouse Right Button was pressed exit button then go exit game
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && IsMouseHere(300, 400, 200, 100))
            CloseWindow();
    }

    // De-Initialization
    //------------------------------------------------------
    UnloadTexture(btn_texture);
}

/*
 * Returns an integer in the range [0, n).
 * Uses rand(), and so is affected-by/affects the same seed.
 */
int random(int n)
{
    if ((n - 1) == RAND_MAX)
        return rand();
    else
    {
        // Supporting larger values for n would requires an even more
        // elaborate implementation that combines multiple calls to rand()
        assert (n <= RAND_MAX);

        // Chop off all of the values that would cause skew...
        int end = RAND_MAX / n; // truncate skew
        assert (end > 0);
        end *= n;

        // ... and ignore results from rand() that fall above that limit.
        // (Worst case the loop condition should succeed 50% of the time,
        // so we can expect to bail out of this loop pretty quickly.)
        int r;
        while ((r = rand()) >= end);

        return r % n;
    }
}

void add_object(Type **objects, int *index, char type)
{
    Type *current = malloc(sizeof(Type));

    current->index = *index;
    if(type == food)
    {
        current->type = food;
        current->inanimate = (micro_type) {.color = food_color};
    }
    else if(type == bacteria)
    {
        current->type = bacteria;
        current->bacterium = (type_bacteria) {.health = 1000,
                                              .speed = 1,
                                              .size = 4,
                                              .color = bacteria_color,
                                              .DNA = gen_DNA(8, 8)};
    }

    if(*objects == NULL)
        current->next = NULL;
    else
        current->next = *objects;

    *objects = current;
}

void generate_type(int ***world, Type **objects, int *index, int n, char type)
{
    int x, y;
    int **new_world = *world;

    for(int i = 1; i <= n; i++)
    {
        do
        {
            x = random(ScreenWidth - border);
            y = random(ScreenHeight - border);
        }
        while(new_world[x][y] != 0);

        add_object(objects, index, type);

        // If bacterium is create in border space then
        // Increments it with border
        if(x < border)
            x += border;
        if(y < border)
            y += border;

        new_world[x][y] = *index;

        (*index)++;
    }

    *world = new_world;
}

DNA gen_DNA(int width, int height)
{
    DNA DNA;
    DNA.position = -1;
    DNA.ma_width = width,  DNA.ma_height = height;
    DNA.matrix = malloc(sizeof(int*) * height);

    for(int i = 0; i < width; i++)
    {
        DNA.matrix[i] = malloc(sizeof(int) * width);
        for(int j = 0; j < height; j++)
            DNA.matrix[i][j] = random(8);
    }

    return DNA;
}

void add_child(int ***world, Type **objects, int *index, Coord_Reg coord, DNA child_DNA)
{
    int **new_world = *world;

    // Add in list
    //-------------------------------------------------------
    Type *current = malloc(sizeof(Type));
    current->index = *index;
    current->type = child;
    current->bacterium = (type_bacteria) {.health = 300,
                                          .speed = 1,
                                          .size = 2,
                                          .color = SKYBLUE,
                                          .DNA = child_DNA};
    if(*objects == NULL)
        current->next = NULL;
    else
        current->next = *objects;

    *objects = current;
    //-------------------------------------------------------

    new_world[coord.x][coord.y] = *index;
    *world = new_world;

    (*index)++;
}

Type extract_by_index(Type **objects, int index)
{
    Type *current = *objects;
    Type indexed_object;
         indexed_object.type = space;

    while(current != NULL && current->index != index)
        current = current->next;

    // If current is NULL then return type = space
    if(current == NULL)
        return indexed_object;

    // Some special actions
    if(current->type == bacteria || current->type == child)
    {
        // Move position
        current->bacterium.DNA.position++;
        if(current->bacterium.DNA.position == current->bacterium.DNA.ma_height *
                                              current->bacterium.DNA.ma_width)
            current->bacterium.DNA.position = 0;

        // Decrementation health
        current->bacterium.health--;
    }

    indexed_object = *current;
    buff_obj = current;

    return indexed_object;
}

void increase_health(Type **objects, int index)
{
    Type *current = *objects;

    while(current != NULL && current->index != index)
        current = current->next;

    if(current != NULL && current->index == index)
        current->bacterium.health += food_energy;

    // If child increase his health to 400 then
    if(current->type == child && current->bacterium.health >= 400)
    {
        current->type = bacteria;
        current->color = BLACK;
        current->bacterium.size = 4;
    }
}

void destr_object(Type **objects, int index)
{
    Type *current = *objects,
         *prev = NULL;

    if((*objects)->index == index)
    {
        prev = *objects;
        *objects = prev->next;
        free_object(prev);
    }
    else
    {
        prev = current;

        while(current->next != NULL && current->index != index)
        {
            prev = current;
            current = current->next;
        }

        if(current->next == NULL && current->index == index)
        {
            prev->next = NULL;
            free_object(current);
            return;
        }

        if(current->index == index)
        {
            prev->next = current->next;
            free_object(current);
        }
    }
}

void free_object(Type *object)
{
    if(object->type == bacteria)
    {
        DNA DNA = object->bacterium.DNA;
        for(int i = 0; i < DNA.ma_height; i++)
            free(DNA.matrix[i]);
        free(DNA.matrix);
    }

    free(object);
}

// Register
//-------------------------------------------------
void add_in_reg(Coord_Reg **reg, int x, int y)
{
    Coord_Reg *tmp_reg = malloc(sizeof(Coord_Reg));
    tmp_reg->x = x, tmp_reg->y = y;

    if(reg == NULL)
    {
        *reg = tmp_reg;
        return;
    }

    tmp_reg->next = *reg;
    *reg = tmp_reg;
}

bool verify_reg(Coord_Reg *reg, int x, int y)
{
    while(reg != NULL)
    {
        if(reg->x == x && reg->y == y)
            return true;

        reg = reg->next;
    }

    return false;
}

void free_reg(Coord_Reg **reg)
{
    Coord_Reg *cursor, *current;

    if(*reg == NULL)
        return;

    cursor = *reg;
    while(cursor != NULL)
    {
        current = cursor->next;
        free(cursor);
        cursor = current;
    }

    *reg = NULL;
}
//-------------------------------------------------

bool is_border(int x, int y, int size, int speed, char x_sign, char y_sign)
{
    // Verify x coordinate
    if(x_sign != space)
        switch(x_sign)
        {
            case '-':
                if(x - speed < border)
                    return false;
                break;
            case '+':
                if(x + speed > ScreenWidth - border)
                    return false;
                break;
        }

    // Verify y coordinate
    if(y_sign != space)
        switch(y_sign)
        {
            case '-':
                if(y - speed < border)
                    return false;
                break;
            case '+':
                if(y + speed > ScreenHeight - border)
                    return false;
                break;
        }

    return true;
}

/*
 * Control the behavior of bacterium
 * Every time it used a new random number
*/
int bacterium_moving(int *x, int *y, int rand_num, int size, int speed)
{
    int temp_x = *x, temp_y = *y;
    // Get the temporal small control
    switch(rand_num)
    {
        // Every case describe where bacteria try to move
        // Else means that bacteria, when meet wall try to move in another direction

        // Go to N
        case 0:
            if(is_border(*x, *y, size, speed, space, '-'))
                *y -= speed;
            break;

        // Go to S
        case 1:
            if(is_border(*x, *y, size, speed, space, '+'))
                *y += speed;
            break;

        // Go to E
        case 2:
            if(is_border(*x, *y, size, speed, '+', space))
                *x += speed;
            break;

        // Go to W
        case 3:
            if(is_border(*x, *y, size, speed, '-', space))
                *x -= speed;
            break;

        // Go to N-E
        case 4:
            if(is_border(*x, *y, size, speed, '+', '-'))
            {
                *x += speed;
                *y -= speed;
            }
            break;

        // Go to N-W
        case 5:
            if(is_border(*x, *y, size, speed, '-', '-'))
            {
                *x -= speed;
                *y -= speed;
            }
            break;

        // Go to S-E
        case 6:
            if(is_border(*x, *y, size, speed, '+', '+'))
            {
                *x += speed;
                *y += speed;
            }
            break;

        // Go to S-W
        case 7:
            if(is_border(*x, *y, size, speed, '-', '+'))
            {
                *x -= speed;
                *y += speed;
            }
            break;
    }

    // If switch states meet wall then need to transform DNA
    // And return -1
    if(temp_x == *x && temp_y == *y)
        return (-1);
    return 0;
}

void DNA_action_coord(DNA curr_DNA, int *m, int *n)
{
    int position = curr_DNA.position,
        height = curr_DNA.ma_height,
        width = curr_DNA.ma_width;
    // Converting action coordinate in double variable's binary
    *m = position / height;
    *n = position % width;
}

char what_border(int x, int y, int size)
{
    // Up Border
    if(y == border && x > border)
        return 'U';
    // Down Border
    else if(y == (ScreenHeight - border) && x > border)
        return 'D';
    // Left Border
    else if(x == border && y > border)
        return 'L';
    // Right Border
    else if(x == (ScreenWidth - border) && y > border)
        return 'R';

    return 'C';
}

void transform_DNA(DNA temp_DNA, char name_border)
{
    DNA curr_DNA = temp_DNA;

    /** Change directions in DNA
        For N,S,E and W don't matter what border is
    //----------------------------------------------
        All wall:   Up and Down:    Left and Right:
         N -> E      N-E -> S-E      N-E -> N-W
         E -> S      N-W -> S-W      N-W -> N-E
         S -> W      S-E -> N-E      S-E -> S-W
         W -> N      S-W -> N-W      S-W -> S-E
    //-----------------------------------------------**/

    for(int i = 0; i < curr_DNA.ma_height; i++)
        for(int j = 0; j < curr_DNA.ma_width; j++)
            switch(curr_DNA.matrix[i][j])
            {
                // North
                case 0:
                    curr_DNA.matrix[i][j] = 1;
                    break;

                // South
                case 1:
                    curr_DNA.matrix[i][j] = 0;
                    break;

                // East
                case 2:
                    curr_DNA.matrix[i][j] = 3;
                    break;

                // West
                case 3:
                    curr_DNA.matrix[i][j] = 2;
                    break;

                // North-East
                case 4:
                    if(name_border == 'U' || name_border == 'D')
                        curr_DNA.matrix[i][j] = 6;
                    else if(name_border == 'L' || name_border == 'R')
                        curr_DNA.matrix[i][j] = 5;
                    break;

                // North-West
                case 5:
                    if(name_border == 'U' || name_border == 'D')
                        curr_DNA.matrix[i][j] = 7;
                    else if(name_border == 'L' || name_border == 'R')
                        curr_DNA.matrix[i][j] = 4;
                    break;

                // South-East
                case 6:
                    if(name_border == 'U' || name_border == 'D')
                        curr_DNA.matrix[i][j] = 4;
                    else if(name_border == 'L' || name_border == 'R')
                        curr_DNA.matrix[i][j] = 7;
                    break;

                // South-East
                case 7:
                    if(name_border == 'U' || name_border == 'D')
                        curr_DNA.matrix[i][j] = 5;
                    else if(name_border == 'L' || name_border == 'R')
                        curr_DNA.matrix[i][j] = 6;
                    break;
            }

    curr_DNA.position--;
    buff_obj->bacterium.DNA = curr_DNA;
}

int verify_collision(int **world, Coord_Reg *scnd_coord, int i, int j, int x, int y, int size)
{
    // Coordinate differentiation responds for the directions
    int diff_x = x - i;
    int diff_y = y - j;
    int temp_i, temp_j;

    if(diff_x == 0 && diff_y == 0)
        return 0;

    // When bacteria go to EAST/West
    // Verify if is collision with right/left border
    // Verify left/right border plus extra (size-1) pixels Up and down
    if(diff_y == 0)
    {
        // Go to superior extreme (j - size)
        j -= (size - 1);

        // Go to right/left extreme
        if(diff_x > 0)
            i += size;
        else if(diff_x < 0)
            i -= size;

        for(int m = 0; m < size*2 - 1; m++)
            if(world[i][j + m] != 0)
            {
                //DrawRectangle(i - 2, j + m - 2, 5, 5, RED);
                *scnd_coord = (Coord_Reg){.x = i,
                                          .y = j + m};
                return world[i][j + m];
            }
    }
    // When bacteria go to North/South
    // Verify if to the up/down border is collision
    // Verify down/up border plus extra (size-1) pixels right and left
    else if(diff_x == 0)
    {
        // Go to left extreme (i - size)
        i -= (size - 1);

        // Go to right/left extreme
        if(diff_y > 0)
            j += size;
        else if(diff_y < 0)
            j -= size;

        for(int m = 0; m < size*2 - 1; m++)
            if(world[i + m][j] != 0)
            {
                //DrawRectangle(i + m - 2, j - 2, 5, 5, RED);
                *scnd_coord = (Coord_Reg){.x = i + m,
                                          .y = j};
                return world[i + m][j];
            }
    }

    //When bacteria go to North-East/North-West
    else if(diff_y > 0)
    {
        if(diff_x > 0)
        {
            temp_i = i + 1, temp_j = j + size;
            i += size,      j++;
        }
        else if(diff_x < 0)
        {
            temp_i = i - size, temp_j = j - 1;
            i -= size,         j -= size;
        }

        for(int m = 0; m < (size * 2 - 1); m++)
            if(world[i][j + m] != 0)
            {
                //DrawRectangle(i - 2, j + m - 2, 10, 10, RED);
                *scnd_coord = (Coord_Reg){.x = i,
                                          .y = j + m};
                return world[i][j + m];
            }
            else if(world[i + m][j] != 0)
            {
                //DrawRectangle(i - 2, j + m - 2, 10, 10, RED);
                *scnd_coord = (Coord_Reg){.x = i + m,
                                          .y = j};
                return world[i + m][j];
            }
    }

    // When bacteria go to South-East/South-West
    else if(diff_x > 0)
    {
        if(diff_y > 0)
        {
            temp_i = i + 1, temp_j = j + size;
            i += size,      j++;
        }
        else if(diff_y < 0)
        {
            temp_i = i - size, temp_j = j - 1;
            i -= size,      j -= size;
        }

        for(int m = 0; m < (size * 2 - 1); m++)
            if(world[i][j + m] != 0)
            {
                //DrawRectangle(i - 2, j + m - 2, 10, 10, RED);
                *scnd_coord = (Coord_Reg){.x = i,
                                          .y = j + m};
                return world[i][j + m];
            }
            else if(world[i + m][j] != 0)
            {
                //DrawRectangle(i - 2, j + m - 2, 10, 10, RED);
                *scnd_coord = (Coord_Reg){.x = i + m,
                                          .y = j};
                return world[i + m][j];
            }
    }

    return 0;

}

void multiply(int ***world, Type **objects, int *index, Type thrs_obj, Coord_Reg thrs_coord,
                                                        Type scnd_obj, Coord_Reg scnd_coord)
{
    // Extract parents from objects
    type_bacteria thrs_parent = thrs_obj.bacterium,
                  scnd_parent = scnd_obj.bacterium;

    // Extract indexes from objects
    int thrs_index = thrs_obj.index,
        scnd_index = scnd_obj.index;

    // Thirst DNA, Second DNA
    DNA thrs_DNA = thrs_parent.DNA,
        scnd_DNA = scnd_parent.DNA;

    // If parents have the the same size of DNA
    if(thrs_DNA.ma_width == scnd_DNA.ma_width &&
       thrs_DNA.ma_height == scnd_DNA.ma_height)
    {
        // If parents have the same sizes of DNA
        // then copy it in new variables
        int size = thrs_parent.size;

        int ma_width = thrs_DNA.ma_width;
        int ma_height = thrs_DNA.ma_height;

        // Copy parent's DNA structure in new matrix
        int **thrs_matrix = thrs_DNA.matrix;
        int **scnd_matrix = scnd_DNA.matrix;

        // Generate children's DNA
        DNA thrs_child = gen_DNA(ma_width, ma_height),
            scnd_child = gen_DNA(ma_width, ma_height),
            thrd_child = gen_DNA(ma_width, ma_height),
            frth_child = gen_DNA(ma_width, ma_height);

        // Create children's DNA structure
        for(int i = 0; i < ma_height / 2; i++)
            for(int j = 0; j < ma_width; j++)
            {
                // Thirst child
                thrs_child.matrix[i][j] = thrs_matrix[i][j];
                thrs_child.matrix[ma_height - i - 1][j] = scnd_matrix[ma_height - i - 1][j];

                // Second child
                scnd_child.matrix[i][j] = scnd_matrix[i][j];
                scnd_child.matrix[ma_height - i - 1][j] = thrs_matrix[ma_height - i - 1][j];

                // Third child
                thrd_child.matrix[j][i] = thrs_matrix[j][i];
                thrd_child.matrix[j][ma_width - i - 1] = scnd_matrix[j][ma_width - i - 1];

                // Forth child
                frth_child.matrix[j][i] = scnd_matrix[j][i];
                frth_child.matrix[j][ma_width - i - 1] = thrs_matrix[j][ma_width - i - 1];
            }

            // Children's DNA width
            thrs_child.ma_width = scnd_child.ma_width =
            thrd_child.ma_width = frth_child.ma_width = ma_width;

            // Children's DNA height
            thrs_child.ma_height = scnd_child.ma_height =
            thrd_child.ma_height = frth_child.ma_height = ma_height;

            // Destroy parents
            destr_object(objects, thrs_index);
            destr_object(objects, scnd_index);

            // Determinate coordinates for third and forth children
            Coord_Reg thrd_coord = (Coord_Reg){.x = thrs_coord.x + size/ 2,
                                               .y = thrs_coord.y + size / 2,},
                      frth_coord = (Coord_Reg){.x = scnd_coord.x + size / 2 ,
                                               .y = scnd_coord.y + size / 2,};

            // Add children in object's list
            add_child(world, objects, index, thrs_coord, thrs_child);
            add_child(world, objects, index, scnd_coord, scnd_child);
            add_child(world, objects, index, thrd_coord, thrd_child);
            add_child(world, objects, index, frth_coord, frth_child);

            // Add 2 new bacteria in objects
            bacteria_counter += 2;
    }
    else
    {
        printf("\nError at multiply!\n");
        printf("Parents don't have the same sizes\n");
        return;
    }
}

void draw_bacterium_stats(int x, int y, int index, int health, int size)
{
    char buffer[256];
    sprintf(buffer, "Index = %d", index);

    // Draw stats interface
    DrawRectangle(x, y - 50, 100, 50, BLACK);
    DrawText(buffer, x + 5, y - 40, 14, WHITE);

    // Draw health
    sprintf(buffer, "Health = %d", health);
    DrawText(buffer, x + 5, y - 25, 14, WHITE);
}

void print_obj(Type *objects)
{
    while(objects != NULL)
    {
        printf("(%d, %c)\n", objects->index, objects->type);
        objects = objects->next;
    }
    printf("\n");
}
