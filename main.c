#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <raylib.h>
#define bacteria   'b'
#define child      'c'
#define generation 'g'
#define food       'f'
#define space      's'
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
int bacteria_counter = 1000;

// Exit and continue buttons
bool isobjectempty = false;
bool continue_var = false;
bool exit_var = false;
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


// The buffer of the last object extract
//--------------------------------------
Type *buff_obj = NULL;
//--------------------------------------

// Verify if file exist or isn't empty
bool verify_file(FILE *file);

// Verify if mouse is in this range
int IsMouseHere(int x, int y, int width, int height);

// Main menu
void menu();

// Read object from file
Type read_from_file(FILE *file);

// Return a random number every frame
int random(int n);

// Add an object in list
void add_object(Type **objects, int *index, char type, Type object);

// Generate type
void generate_type(int ***world, Type **objects, int *index, int n, char type, Type object);

// Genetics
//-------------------------------------------------
DNA gen_DNA(int width, int height);
//-------------------------------------------------

// Extract object from list
Type extract_by_index(Type **objects, int index);

// When bacterium meet food the increase health
void increase_health(Type **objects, int thrs_index, int scnd_index);

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

// Verify if register contain just one element
bool is_regiter_almost_empty(Coord_Reg *reg);
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
Type verify_collision(int ***world, Type **objects, Coord_Reg *scnd_coord, int i, int j, int size);

// If bacterium meet another one then multiply
void multiply(int ***world, Type **objects, int *index, Type thrs_parent, Coord_Reg thrs_coord,
                                                        Type scnd_parent, Coord_Reg scnd_coord);

// Clear list from useless objects and keep last objects,
// if it exist
void garbage_collector(Type **objects, int last_index);

// Write object to the file
void write_in_file(FILE *file, Type object);

// Draw generation counter
void draw_generation(gen_counter);

// Draw statistic of selected bacteria
void draw_bacterium_stats(int x, int y, int index, int health, int size);

// Extras function
//--------------------------------------------------
// Print list with objects
void print_obj(Type *objects);

// Print the object that was extracted from the file
void print_obj_from_file(Type obj);
//--------------------------------------------------

// To sort(not ended)
//-------------------------------------------------------------------
void mutation(Type **object)
{
    int   rand_num;
    Type *current = *object;
    int  **matrix = current->bacterium.DNA.matrix;


    DNA curr_DNA = current->bacterium.DNA;

    // Every position from DNA's matrix have 5% for mutation,
    // And 1% for logic mutation
    for(int i = 0; i < curr_DNA.ma_height; i++)
        for(int j = 0; j < curr_DNA.ma_width; j++)
        {
            rand_num = random(100) + 1;
            if(!(rand_num % 20))
            {
                matrix[i][j] = random(8);
                /*if(!(rand_num % 100))
                    matrix[i][j] *= -1;*/
            }
        }
}
//-------------------------------------------------------------------


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

    // Initialize Window
    InitWindow(ScreenWidth, ScreenHeight, "Pandora");
    SetTargetFPS(30);

    // Variables
    //--------------------------------------------------
    // Object's coordinate
    int x, y;

    // List with objects
    Type *objects = NULL;

    // Objects Indexes
    int index = 1;
    // Temporal variable for swap
    int temp;

    // Thirst and second object
    Type obj,
         scnd_obj;

    // m and n response for matrix position in DNA
    int m = 0,
        n = 0;

    // Index of second object
    int scnd_index = 0;

    // If move is -1 it's means that bacteria
    // need to transform she's DNA
    int move;

    // Statistics used for draw_bacterium_stats
    //-----------------------------------------
    Type stats;
         stats.index = 0;

    Coord_Reg stats_reg;
    //-----------------------------------------

    // Thirst and second parent's coordinate
    Coord_Reg thrs_coord,
              scnd_coord;

    // Food's object
    micro_type obj_food;

    // Bacteria's object
    type_bacteria obj_bacteria;

    // Register
    Coord_Reg *reg = NULL;

    // Generation counter
    int gen_counter = 0;
    //--------------------------------------------------

    // Read object from file
    //------------------------------------
    FILE *file = fopen("output.txt", "r");
    obj = read_from_file(file);
    fclose(file);
    //------------------------------------

    // Start with Main Menu
    menu();

    // If continue button was pressed, then read object from file
    //------------------------------------------------------------
    if(!continue_var)
        obj.type = space;
    //------------------------------------------------------------

    // Generate bacteria and food
    // If continue button was pressed then multiply last write member,
    // else start new game with random members
    //-----------------------------------------------------------------------------
    generate_type(&world, &objects, &index, food_counter, food, obj);

    if(obj.type == space)
        generate_type(&world, &objects, &index, bacteria_counter, bacteria, obj);
    else if(obj.type == bacteria || obj.type == child)
        generate_type(&world, &objects, &index, bacteria_counter, generation, obj);
    //-----------------------------------------------------------------------------

    while(!WindowShouldClose() && !exit_var)
    {
        BeginDrawing();
            ClearBackground(WHITE);
            draw_generation(gen_counter);

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


                                    if(move == -1)// && obj_bacteria.DNA.last_transform != what_border(x, y, obj_bacteria.size))
                                        transform_DNA(obj_bacteria.DNA, what_border(x, y, obj_bacteria.size));

                                    // Collision and multiply
                                    //-------------------------------------------------

                                    // Extract the secondary index with which the first object interacts
                                    scnd_obj = verify_collision(&world, &objects, &scnd_coord, i, j, obj_bacteria.size);

                                    // If second object is bacteria then multiply
                                    if(obj.type == bacteria && scnd_obj.type == bacteria)
                                    {
                                        // Coordinate of thirst object
                                        thrs_coord = (Coord_Reg){.x = i,
                                                                 .y = j};

                                        multiply(&world, &objects, &index, obj,      thrs_coord,
                                                                           scnd_obj, scnd_coord);
                                    }

                                    //-------------------------------------------------

                                    // If bacterium health <= 0 then destroy object
                                    if(obj_bacteria.health <= 0 )
                                    {
                                        destr_object(&objects, world[i][j]);
                                        world[i][j] = 0;
                                    }


                                    DrawRectangle(x, y, obj_bacteria.size, obj_bacteria.size, obj_bacteria.color);

                                    add_in_reg(&reg, x, y);

                                    temp = world[x][y];
                                    world[x][y] = world[i][j];
                                    world[i][j] = temp;
                                }

                                // Draw bacterium statistics
                                //----------------------------------------------------------------------------
                                if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                                    if(IsMouseHere(x - 4, y - 4, 10, 10))
                                    {
                                        stats = obj;
                                        stats_reg = (Coord_Reg) {.x = x,
                                                                 .y = y};
                                    }

                                if(obj.index == stats.index)
                                {
                                    stats = obj;
                                    stats_reg = (Coord_Reg) {.x = x,
                                                             .y = y};

                                    if(world[stats_reg.x][stats_reg.y] == 0)
                                        stats.index = 0;
                                }

                                if(stats.index != 0)
                                    draw_bacterium_stats(stats_reg.x,
                                                         stats_reg.y, stats.index, stats.bacterium.health, 3);
                                //----------------------------------------------------------------------------
                            }
                    }

            if(is_regiter_almost_empty(reg))
            {
                // Clear object's list from useless objects
                garbage_collector(&objects, buff_obj->index);

                // If the last object was destroyed before
                // it was written to the file then rewrite in file
                if(bacteria_counter == 1)
                {
                    FILE *file = fopen("output.txt", "w");
                    write_in_file(file, *buff_obj);
                    fclose(file);
                }
                // Else read object from file and create generation
                else
                {
                    FILE *file = fopen("output.txt", "r");
                    *buff_obj = read_from_file(file);
                    fclose(file);
                }

                gen_counter++;
                bacteria_counter += 1000;
                generate_type(&world, &objects, &index, 1000, generation, *buff_obj);
            }
            else
                free_reg(&reg);


            // Every time generate food
            if(food_counter < 1000)
            {
                generate_type(&world, &objects, &index, 1, food, obj);
                food_counter++;
            }

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
    objects = NULL;
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

bool verify_file(FILE *file)
{
    // Verify if file exit
    if(file == NULL)
        return false;

    // Verify if file is empty
    fseek(file, 0, SEEK_END);
    if(ftell(file) == 0)
        return false;
    else
        fseek(file, 0, SEEK_SET);

    return true;
}

// Main Menu
void menu()
{
    Image img = LoadImage("image.jpg");

    Texture2D btn_texture = LoadTextureFromImage(img);
    UnloadImage(img);

    while(1)
    {
        // Drawing Menu
        BeginDrawing();

            ClearBackground(WHITE);


            // Draw start button
            //----------------------------------------------------------------------------------
            DrawText("Start Game", ScreenHeight / 2 - 170, ScreenWidth / 2 - 50, 20, GRAY);
            if(IsMouseHere(ScreenHeight / 2 - 210, ScreenWidth / 2 - 70, 196, 60))
            {
                DrawRectangle(ScreenHeight / 2 - 210, ScreenWidth / 2 - 70, 196, 60, GRAY);
                DrawText("Start Game", ScreenHeight / 2 - 170, ScreenWidth / 2 - 50, 20, WHITE);
            }
            //----------------------------------------------------------------------------------

            // Draw continue button
            //-------------------------------------------------------------------------------------
            if(isobjectempty)
                DrawText("Continue Game", ScreenHeight / 2 - 180, ScreenWidth / 2 + 10, 20, LIGHTGRAY);
            else
            {
                DrawText("Continue Game", ScreenHeight / 2 - 180, ScreenWidth / 2 + 10, 20, GRAY);
                if(IsMouseHere(ScreenHeight / 2 - 210, ScreenWidth / 2 - 10, 196, 60))
                {
                    DrawRectangle(ScreenHeight / 2 - 210, ScreenWidth / 2 - 10, 196, 60, GRAY);
                    DrawText("Continue Game", ScreenHeight / 2 - 180, ScreenWidth / 2 + 10, 20, WHITE);
                }
            }
            //-------------------------------------------------------------------------------------

            // Draw options button
            //-------------------------------------------------------------------------------------
            DrawText("Options", ScreenHeight / 2 - 150, ScreenWidth / 2 + 70, 20, GRAY);
            if(IsMouseHere(ScreenHeight / 2 - 210, ScreenWidth / 2 + 50, 196, 60))
            {
                DrawRectangle(ScreenHeight / 2 - 210, ScreenWidth / 2 + 50, 196, 60, GRAY);
                DrawText("Options", ScreenHeight / 2 - 150, ScreenWidth / 2 + 70, 20, WHITE);
            }
            //-------------------------------------------------------------------------------------

            // Draw exit button
            //-----------------------------------------------------------------------------
            DrawText("Exit", ScreenHeight / 2 - 134, ScreenWidth / 2 + 130, 20, GRAY);
            if(IsMouseHere(ScreenHeight / 2 - 210, ScreenWidth / 2 + 110, 196, 60))
            {
                DrawRectangle(ScreenHeight / 2 - 210, ScreenWidth / 2 + 110, 196, 60, GRAY);
                DrawText("Exit", ScreenHeight / 2 - 134, ScreenWidth / 2 + 130, 20, WHITE);
            }
            //-----------------------------------------------------------------------------

            // Display version's number
            //-------------------------------------
            DrawText("1.3.8.3", ScreenWidth - 75, ScreenHeight - 50, 16, GRAY);
            //-------------------------------------

        EndDrawing();

        // If new game button was pressed then start new game
        //---------------------------------------------------------------------
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            IsMouseHere(ScreenHeight / 2 - 210, ScreenWidth / 2 - 70, 196, 80))
            break;
        //---------------------------------------------------------------------

        // If continue game button was pressed then continue last game
        //---------------------------------------------------------------------
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !isobjectempty &&
            IsMouseHere(ScreenHeight / 2 - 210, ScreenWidth / 2 + 10, 196, 80))
            {
                continue_var = true;
                break;
            }
        //---------------------------------------------------------------------

        // If exit button was pressed then exit from game
        //--------------------------------------------------------------------
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
           IsMouseHere(ScreenHeight / 2 - 210, ScreenWidth / 2 + 90, 196, 80))
        {
            exit_var = true;
            break;
        }
        //--------------------------------------------------------------------
    }

    // De-Initialization
    //------------------------------------------------------
    UnloadTexture(btn_texture);
}

Type read_from_file(FILE *file)
{
    // Variables
    //-----------------------
    int IntColor;
    int size, speed,
        height, width;

    int **matrix;

    DNA curr_DNA;

    Type object;
         object.type = space;
    //-----------------------

    // Read from file
    //---------------------------------------------------
    if(!verify_file(file))
    {
        isobjectempty = true;
        return object;
    }

    fscanf(file, "%c\n", &object.type);
    fscanf(file, "%d\n", &IntColor);
    fscanf(file, "%d\n", &size);
    fscanf(file, "%d\n", &speed);
    fscanf(file, "%d %d\n", &height, &width);

    // Allocate memory to matrix
    //------------------------------------------
    matrix = malloc(sizeof(int) * height);

    for(int i = 0; i < height; i++)
    {
        matrix[i] = malloc(sizeof(int) * width);
        for(int j = 0; j < width; j++)
            fscanf(file, "%d ", &matrix[i][j]);
        fscanf(file, "\n");
    }
    //------------------------------------------

    //----------------------------------------------------

    // Save information in object
    //----------------------------------------------------
    curr_DNA = (DNA) {.ma_height = height,
                      .ma_width = width,
                      .matrix = matrix};

    object.bacterium = (type_bacteria) {.size = size,
                                        .speed = speed,
                                        .color = GetColor(IntColor),
                                        .DNA = curr_DNA};
    //----------------------------------------------------

    return object;
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

void cpy_object(Type **current, Type object)
{
    // Variables
    //-------------------------------------------
    Type *scnd_obj = *current;
    int   height = object.bacterium.DNA.ma_height,
          width  = object.bacterium.DNA.ma_width;
    //-------------------------------------------

    // Allocate memory and copy object matrix
    //------------------------------------------------------
    int  **matrix = malloc(sizeof(int*) * height);
    for(int i = 0; i < height; i++)
    {
        matrix[i] = malloc(sizeof(int) * width);
        for(int j = 0; j < width; j++)
            matrix[i][j] = object.bacterium.DNA.matrix[i][j];
    }
    //------------------------------------------------------

    // Save current object
    //-------------------------------------------------------------------------
    scnd_obj->type = bacteria;
    scnd_obj->bacterium = (type_bacteria) {.health = 100,
                                           .size   = 4,
                                           .speed  = object.bacterium.speed,
                                           .color  = object.bacterium.color,
                                           .DNA    = (DNA) {.ma_height = height,
                                                            .ma_width  = width,
                                                            .matrix    = matrix}
                                           };
    *current = scnd_obj;
    //-------------------------------------------------------------------------
}

void add_object(Type **objects, int *index, char type, Type object)
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
        current->bacterium = (type_bacteria) {.health = 100,
                                              .speed = 1,
                                              .size = 4,
                                              .color = bacteria_color,
                                              .DNA = gen_DNA(8, 8)};
    }
    // When we create generation copy object in every member,
    // And add some mutation
    else if(type == generation)
    {
        cpy_object(&current, object);
        mutation(&current);
    }

    // Save current object
    //---------------------------
    if(*objects == NULL)
        current->next = NULL;
    else
        current->next = *objects;

    *objects = current;
    //---------------------------
}

void generate_type(int ***world, Type **objects, int *index, int n, char type, Type object)
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

        add_object(objects, index, type, object);
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
    for(int i = 0; i < height; i++)
    {
        DNA.matrix[i] = malloc(sizeof(int) * width);
        for(int j = 0; j < width; j++)
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

    // Save world
    //------------------------------------
    new_world[coord.x][coord.y] = *index;
    *world = new_world;
    //------------------------------------

    // Increase counters
    //------------------
    bacteria_counter++;
    (*index)++;
    //------------------
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
        if(current->bacterium.DNA.position >= current->bacterium.DNA.ma_height *
                                              current->bacterium.DNA.ma_width)
            current->bacterium.DNA.position = 0;

        // Save current object in buffer
        buff_obj = current;

        // Decrementation health
        current->bacterium.health--;
    }

    indexed_object = *current;

    return indexed_object;
}

void increase_health(Type **objects, int thrs_index, int scnd_index)
{
    Type *current = buff_obj;

    // If there is another object in the buffer,
    // then we retrieve the object by index
    if(current != NULL && current->index != thrs_index)
    {
        current = *objects;
        while(current != NULL && current->index != thrs_index)
            current = current->next;
    }

    // Increase health
    if(current != NULL && current->index == thrs_index)
        current->bacterium.health += food_energy;

    // If child increase his health to 400,
    // then he become a bacterium
    if(current->type == child && current->bacterium.health >= 400)
    {
        current->type = bacteria;
        current->bacterium.size = 4;
    }

    // Destroy food object
    destr_object(objects, scnd_index);
}

void destr_object(Type **objects, int index)
{
    Type *prev, *current;
          prev = current = *objects;

    // If indexed object is thirst in list
    if((*objects)->index == index)
    {
        prev = *objects;
        *objects = prev->next;
        free_object(prev);
    }
    else
    {
        // Until we find the index object or the end of the list,
        // try to find the indexed list.
        while(current->next != NULL && current->index != index)
        {
            prev = current;
            current = current->next;
        }

        // If we found indexed object and it's the last member
        if(current->next == NULL && current->index == index)
        {
            prev->next = NULL;
            free_object(current);
            return;
        }

        // If the found indexed object is between
        // the head and the tail of the list
        if(current->index == index)
        {
            prev->next = current->next;
            free_object(current);
        }
    }
}



void free_object(Type *object)
{
    if(object->type == bacteria || object->type == child)
    {
        DNA DNA = object->bacterium.DNA;
        for(int i = 0; i < DNA.ma_height; i++)
            free(DNA.matrix[i]);
        free(DNA.matrix);

        bacteria_counter--;
    }
    else if(object->type == food)
        food_counter--;

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

bool is_regiter_almost_empty(Coord_Reg *reg)
{
    if(reg->next == NULL)
        return true;

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

    // Converting number in matrix's index
    //------------------------------------
    *m = position / height;
    *n = position % width;
    //------------------------------------
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

Type verify_collision(int ***world, Type **objects, Coord_Reg *scnd_coord, int i, int j, int size)
{
    Type scnd_obj;
    int **new_world = *world;

    for(int x = i; x < size + i; x++)
        for(int y = j; y < size + j; y++)
        {
            // Skip object index
            if(x == i && y == j)
                continue;

            if(new_world[x][y] != 0)
            {
                scnd_obj = extract_by_index(objects, new_world[x][y]);

                if(scnd_obj.type == food)
                {
                    increase_health(objects, new_world[i][j], scnd_obj.index);
                    new_world[x][y] = 0;
                }
                else if(scnd_obj.type == bacteria)
                {
                    *scnd_coord = (Coord_Reg){.x = x,
                                              .y = y};

                    // Save world
                    *world = new_world;

                    // Return second object
                    return scnd_obj;
                }
            }
        }

    // Save world
    *world = new_world;

    // If don't have collision with another bacteria
    // Then return object with space type
    scnd_obj.type = space;
    return scnd_obj;
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
            Coord_Reg thrd_coord = (Coord_Reg){.x = thrs_coord.x + size / 2,
                                               .y = thrs_coord.y + size / 2,},
                      frth_coord = (Coord_Reg){.x = scnd_coord.x + size / 2,
                                               .y = scnd_coord.y + size / 2,};

            // Add children in object's list
            add_child(world, objects, index, thrs_coord, thrs_child);
            add_child(world, objects, index, scnd_coord, scnd_child);
            add_child(world, objects, index, thrd_coord, thrd_child);
            add_child(world, objects, index, frth_coord, frth_child);
    }
    else
    {
        printf("\nError at multiply!\n");
        printf("Parents don't have the same sizes\n");
        return;
    }
}

void garbage_collector(Type **objects, int last_index)
{
    Type *cursor, *current;
          cursor = current = *objects;

    while(*objects != NULL)
    {
        if(((*objects)->type == bacteria || (*objects)->type == child) &&
            (*objects)->index != last_index)
        {
            cursor = (*objects)->next;
            free_object(objects);
            *objects = cursor;
            bacteria_counter--;
        }
        else
            break;
    }

    current = cursor;
    while(cursor->next != NULL)
    {
        if((cursor->next->type == bacteria || cursor->next->type == child) &&
            cursor->next->index != last_index)
        {
            current = cursor->next;
            cursor->next = current->next;
            free_object(current);
        }
        else
            cursor = cursor->next;
    }
}

void write_in_file(FILE *file, Type object)
{
    int IntColor = ColorToInt(object.bacterium.color);

    // Write object type in file
    fprintf(file, "%c\n", object.type);

    // Write object information in file
    fprintf(file, "%d\n", IntColor);
    fprintf(file, "%d\n", object.bacterium.size);
    fprintf(file, "%d\n", object.bacterium.speed);
    fprintf(file, "%d %d\n", object.bacterium.DNA.ma_height,
                             object.bacterium.DNA.ma_width);
    for(int i = 0; i < object.bacterium.DNA.ma_height; i++)
    {
        for(int j = 0; j < object.bacterium.DNA.ma_width; j++)
            fprintf(file, "%d ", object.bacterium.DNA.matrix[i][j]);
        fprintf(file, "\n");

    }
}

void draw_generation(gen_counter)
{
    char buff[256];
    sprintf(buff, "Gen %d", gen_counter);

    DrawText(buff, ScreenWidth - 56, 6, 18, GRAY);
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

void print_obj_from_file(Type obj)
{
    if(obj.type != space)
    {
        printf("\nType = %c\n", obj.type);
        printf("Size = %d\n", obj.bacterium.size);
        printf("Speed = %d\n", obj.bacterium.speed);
        printf("Color = %d\n", ColorToInt(obj.bacterium.color));
        printf("DNA = (%d, %d)\n", obj.bacterium.DNA.ma_height,
                               obj.bacterium.DNA.ma_width);
        for(int i = 0; i < obj.bacterium.DNA.ma_height; i++)
        {
            for(int j = 0; j < obj.bacterium.DNA.ma_width; j++)
                printf("%d ", obj.bacterium.DNA.matrix[i][j]);
            printf("\n");
        }
    }
    else
        printf("File is empty or don't exist!");
}
