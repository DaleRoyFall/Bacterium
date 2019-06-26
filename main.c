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

// Number of start bacteria
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
    int ma_width;
    int ma_height;
    int **matrix;
}DNA;

typedef struct
{
    int   health;
    int   speed;
    //float multiply_rate;
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
bool is_border(int x, int y, int size, char x_sign, char y_sign);

// Give some behavior to every bacterium
void bacterium_behavior(int *x, int *y, int rand_num, int speed);

// Determine what action form DNA's matrix need to be used
void DNA_action_coord(int *action_coord, int *m, int *n);

// Verify collision between two bacteria
int verify_collision(int **world, Coord_Reg *scnd_coord, int i, int j, int x, int y, int size);

// If bacterium meet another one then multiply
void multiply(int ***world, Type **objects, int *index, int thrs_index, Coord_Reg thrs_coord,
                                                        int scnd_index, Coord_Reg scnd_coord);

// Draw statistic of selected bacteria
void draw_bacterium_stats(int x, int y, int index, int size);

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

    // Statistics used for draw_bacterium_stats
    /*type_bacteria stats;
                    stats.index = 0;*/

    // Initialize Window
    InitWindow(ScreenWidth, ScreenHeight, "Pandora");
    SetTargetFPS(30);

    // Generate bacteria and food
    //----------------------------------------------------------
    generate_type(&world, &objects, &index, 10, food);
    generate_type(&world, &objects, &index, bacteria_counter, bacteria);
    //----------------------------------------------------------

    // Start with Main Menu
    //menu();

    // Temporal variables
    //--------------------------------------------------
    int   x, y;
    int   temp;
    Type obj;

    int action_coord = 0,
        n = 0, m = 0;
    int scnd_index = 0;

    // Objects with stats
    Type stats;

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
                                DrawCircle(i, j, 2, obj_food.color);
                            }
                            else if(obj.type == bacteria || obj.type == child)
                            {
                                obj_bacteria = obj.bacterium;
                                x = i, y = j;
                                if(!verify_reg(reg, x, y))
                                {
                                    bacterium_behavior(&x, &y, obj_bacteria.DNA.matrix[m][n], obj_bacteria.speed);

                                    // Collision and multiply
                                    //-------------------------------------------------
                                    thrs_coord = (Coord_Reg){.x = i,
                                                             .y = j};

                                    scnd_index = verify_collision(world, &scnd_coord, i, j, x, y, obj_bacteria.size);
                                    if(scnd_index > 0 && obj.type != child)
                                        multiply(&world, &objects, &index, world[i][j], thrs_coord,
                                                                           scnd_index, scnd_coord);
                                    //-------------------------------------------------

                                    DrawRectangle(x, y, obj_bacteria.size, obj_bacteria.size, obj_bacteria.color);

                                    add_in_reg(&reg, x, y);

                                    temp = world[x][y];
                                    world[x][y] = world[i][j];
                                    world[i][j] = temp;
                                }

                                /*if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                                    if(IsMouseHere(x - 4, y - 4, 10, 10))
                                    {
                                        stats.index = world[x][y];
                                        stats.bacterium = obj_bacteria;
                                    }

                                if(world[i][j] == stats.index && stats.index != 0)
                                    draw_bacterium_stats(i, j, stats.index, 3);*/
                            }
                    }

            free_reg(&reg);

            DNA_action_coord(&action_coord, &m, &n);

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
        current->bacterium = (type_bacteria) {.health = random(1000),
                                              .speed = 1,
                                              .size = 4,
                                              .color = bacteria_color,
                                              .DNA = gen_DNA(2, 2)};
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
        add_object(objects, index, type);

        x = random(ScreenWidth - border);
        y = random(ScreenHeight - border);

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
    DNA.ma_width = width,  DNA.ma_height = height;
    DNA.matrix = malloc(sizeof(int*) * height);

    for(int i = 0; i < width; i++)
    {
        DNA.matrix[i] = malloc(sizeof(int) * width);
        for(int j = 0; j < height; j++)
            DNA.matrix[i][j] = random(4);
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
    current->bacterium = (type_bacteria) {.health = 1000,
                                          .speed = 1,
                                          .size = 2,
                                          .color = LIME,
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

    // If current is NULL then return space
    if(current == NULL)
        return indexed_object;

    // Destroy object
    //if(current->bacterium.health <= 0)
      //  destr_object(objects, index);

    // Decrementation health
    if(current->type == bacteria)
        current->bacterium.health--;

    indexed_object = *current;

    return indexed_object;
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

        if(current->next == NULL)
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
    DNA DNA = object->bacterium.DNA;
    for(int i = 0; i < DNA.ma_height; i++)
        free(DNA.matrix[i]);
    free(DNA.matrix);
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

bool is_border(int x, int y, int size, char x_sign, char y_sign)
{
    // Verify x coordinate
    if(x_sign != space)
        switch(x_sign)
        {
            case '-':
                if(x - size < border)
                    return false;
            case '+':
                if(x + size > ScreenWidth - border - size)
                    return false;
        }

    // Verify y coordinate
    if(y_sign != space)
        switch(y_sign)
        {
            case '-':
                if(y - size < border)
                    return false;
            case '+':
                if(y + size > ScreenHeight - border - size)
                    return false;
        }

    return true;
}

/*
 * Control the behavior of bacterium
 * Every time it used a new random number
*/
void bacterium_behavior(int *x, int *y, int rand_num, int size)
{
    // Get the temporal small control
    switch(rand_num)
    {
        // Go to S-E else go S
        case 0:
            if(is_border(*x, *y, size, '+', '+'))
            {
                *x += size;
                *y += size;
            }
            else if(is_border(*x, *y, size, space, '+'))
                *y += size;
            break;

        // Go to N-E else go E
        case 1:
            if(is_border(*x, *y, size, '+', '-'))
            {
                *x += size;
                *y -= size;
            }
            else if(is_border(*x, *y, size, '+', space))
                *x += size;
            break;

        // Go to S-V else go V
        case 2:
            if(is_border(*x, *y, size, '-', '+'))
            {
                *x -= size;
                *y += size;
            }
            else if(is_border(*x, *y, size, '-', space))
                *x -= size;
            break;

        // Go to S-E else go N
        case 3:
            if(is_border(*x, *y, size, '-', '-'))
            {
                *x -= size;
                *y -= size;
            }
            else if(is_border(*x, *y, size, space, '-'))
                *y -= size;
            break;
    }
}

void DNA_action_coord(int *action_coord, int *m, int *n)
{
    // Converting action coordinate in double variable's binary
    *n = (*action_coord) % 2;
    *m = (*action_coord) / 2 % 2;

    (*action_coord)++;
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
    // Verify if to the right/left border is collision
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
                DrawRectangle(i - 2, j + m - 2, 5, 5, RED);
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
                DrawRectangle(i + m - 2, j - 2, 5, 5, RED);
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
                DrawRectangle(i - 2, j + m - 2, 10, 10, RED);
                *scnd_coord = (Coord_Reg){.x = i,
                                          .y = j + m};
                return world[i][j + m];
            }
            else if(world[i + m][j] != 0)
            {
                DrawRectangle(i - 2, j + m - 2, 10, 10, RED);
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
                DrawRectangle(i - 2, j + m - 2, 10, 10, RED);
                *scnd_coord = (Coord_Reg){.x = i,
                                          .y = j + m};
                return world[i][j + m];
            }
            else if(world[i + m][j] != 0)
            {
                DrawRectangle(i - 2, j + m - 2, 10, 10, RED);
                *scnd_coord = (Coord_Reg){.x = i + m,
                                          .y = j};
                return world[i + m][j];
            }
    }

    return 0;

}

void multiply(int ***world, Type **objects, int *index, int thrs_index, Coord_Reg thrs_coord,
                                                        int scnd_index, Coord_Reg scnd_coord)
{
    Type temp_obj;

    // Thirst parent, second parent
    type_bacteria thrs_parent, scnd_parent;

    // Extract thirst parent
    temp_obj = extract_by_index(objects, thrs_index);
    thrs_parent = temp_obj.bacterium;

    // Extract second parent
    temp_obj = extract_by_index(objects, scnd_index);
    scnd_parent = temp_obj.bacterium;

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

        /*printf("\nThirst parent\n");
        for(int i = 0; i < ma_height; i++)
        {
            printf("\n");
            for(int j = 0; j < ma_width; j++)
                printf("%d ", thrs_matrix[i][j]);
        }

        printf("\n");

        printf("\nSecond parent\n");
        for(int i = 0; i < ma_height; i++)
        {
            printf("\n");
            for(int j = 0; j < ma_width; j++)
                printf("%d ", scnd_matrix[i][j]);
        }*/

        //int matrix[ma_width][ma_height];

        // Generate children's DNA
        DNA thrs_child = gen_DNA(ma_width, ma_height),
            scnd_child = gen_DNA(ma_width, ma_height),
            thrd_child = gen_DNA(ma_width, ma_height),
            frth_child = gen_DNA(ma_width, ma_height);

        // Create children's DNA structure
        for(int i = 0; i < ma_width; i++)
            for(int j = 0; j < ma_height; j++)
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

            /*printf("\nThirst child\n");
            for(int i = 0; i < ma_height; i++)
            {
                printf("\n");
                for(int j = 0; j < ma_width; j++)
                    printf("%d ", thrs_child.matrix[i][j]);
            }

            printf("\nSecond child\n");
            for(int i = 0; i < ma_height; i++)
            {
                printf("\n");
                for(int j = 0; j < ma_width; j++)
                    printf("%d ", scnd_child.matrix[i][j]);
            }

            printf("\nThird child\n");
            for(int i = 0; i < ma_height; i++)
            {
                printf("\n");
                for(int j = 0; j < ma_width; j++)
                    printf("%d ", thrd_child.matrix[i][j]);
            }

            printf("\nForth child\n");
            for(int i = 0; i < ma_height; i++)
            {
                printf("\n");
                for(int j = 0; j < ma_width; j++)
                    printf("%d ", frth_child.matrix[i][j]);
            }*/

            /** Remember to destroy parents **/
            destr_object(objects, thrs_index);
            destr_object(objects, scnd_index);

            int temp_x = thrs_coord.x + size + 2;
            int temp_y = thrs_coord.y;

            printf("Here = (%d, %d)|(%d, %d)", thrs_coord.x, thrs_coord.y, temp_x, temp_y);
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

            printf("\nChildren are created!\n");

    }
    else
    {
        printf("\nError at multiply!\n");
        printf("Parents don't have the same sizes\n");
        return;
    }
}

void draw_bacterium_stats(int x, int y, int index, int size)
{
    char buffer[256];
    sprintf(buffer, "Index = %d", index);

    // Draw stats interface
    DrawRectangle(x, y - 50, 100, 50, BLACK);
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
