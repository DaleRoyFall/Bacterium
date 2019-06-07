#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <raylib.h>
#include <easy_list.h>
#define bacteria 'b'
#define food     'f'
#define space    's'
#define bacteria_color BLUE
#define food_color     LIME

// Initialize with screen dimensions
//---------------------------------
const int ScreenWidth = 600;
const int ScreenHeight = 800;
//---------------------------------

// Global variables
//---------------------------------
// Wall's border
int border = 10;

// Number of start bacteria
int bacteria_counter = 1000;

// Pause button
bool pause = false;
//---------------------------------

/*  Type of cells:
        b - bacteria
        f - food
        s - space
*/
typedef struct
{
    char  type;
    Color color;
}micro_type;

typedef struct
{
    float speed;
    float multiply_rate;
    Color color;
}type_bacteria;

typedef struct
{
    int     index;
    union
    {
        micro_type    inanimate;
        type_bacteria bacterium;
    };
    struct Type    *next;
}Type;

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
void add_object(Type **objects, int index, char type);

// Generate type
void generate_type(int ***world, Type **objects, int n, char type);

// Extract object from list
void extract_by_index(void **obj, Type *objects, int index);

// Register
//-----------------------------------------------------------------
// Add data in register
Coord_Reg * add_in_reg(Coord_Reg *reg, int x, int y)
{
    if(reg == NULL)
    {
        reg = malloc(sizeof(Coord_Reg));
        reg->x = x, reg->y = y;
        return reg;
    }
}


//-----------------------------------------------------------------

// Give some behavior to every bacterium
void bacterium_behavior(int *x, int *y, float speed);

// Draw statistic of selected bacteria
void draw_bacterium_stats(int x, int y, int index, int size);

// Main
int main()
{
    // World in a 2D matrix
    // World contains indexes of all objects form object's list
    //---------------------------------------------------------
    int **world;

    // Allocation memory to world's matrix
    world = malloc(sizeof(micro_type*) * ScreenWidth);

    for(int i = 0; i < ScreenWidth; i++)
        world[i] = malloc(sizeof(micro_type) * ScreenHeight);

    // Initialize world
    for(int i = 0; i < ScreenWidth; i++)
        for(int j = 0; j < ScreenHeight; j++)
            world[i][j] = 0;
    //---------------------------------------------------------

    // List with objects
    Type *objects = NULL;
    add_object(&objects, 1, food);
    add_object(&objects, 2, bacteria);

    // Statistics used for draw_bacterium_stats
    /*type_bacteria stats;
                    stats.index = 0;*/

    // Initialize Window
    InitWindow(ScreenWidth, ScreenHeight, "Pandora");
    SetTargetFPS(30);

    // Generate bacteria and food
    //----------------------------------------------------------
    generate_type(&world, &objects, 100, food);
    generate_type(&world, &objects, bacteria_counter, bacteria);
    //----------------------------------------------------------

    // Start with Main Menu
    //menu();

    // Temporal variables
    //--------------------------------------------------
    int   x, y;
    void *obj;

    // Extract food's object
    extract_by_index(&obj, objects, 1);
    micro_type obj_food = *((micro_type*)obj);

    // Extract bacteria's object
    extract_by_index(&obj, objects, 2);
    type_bacteria obj_bacteria = *((type_bacteria*)obj);
    //--------------------------------------------------

    Coord_Reg *reg;

    while(!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(WHITE);

            for(int i = 0; i < ScreenWidth; i++)
                for(int j = 0; j < ScreenHeight; j++)
                    if(world[i][j] == 1)
                        DrawCircle(i, j, 2, obj_food.color);
                    else if(world[i][j] == 2)
                    {
                        x = i, y = j;
                        bacterium_behavior(&x, &y, obj_bacteria.speed);
                        DrawRectangle(x, y, 3, 3, obj_bacteria.color);


                        world[i][j] = 0;
                        world[x][y] = 2;

                        /*if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                            if(IsMouseHere((int)x - 5,
                                           (int)y - 5, 10, 10))
                                stats = obj_bacteria;

                            if(bacterium[i].index == stats.index && stats.index != 0)
                                stats = bacterium[i];

                            draw_bacterium_stats((int)stats.x, (int)stats.y, stats.index, 3);*/
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

    free(objects);
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

void add_object(Type **objects, int index, char type)
{
    Type *current;
          current = malloc(sizeof(Type));

    current->index = index;
    if(type == food)
        current->inanimate = (micro_type) {.type = food,
                                           .color = food_color};
    else if(type == bacteria)
        current->bacterium = (type_bacteria) {.speed = 1,
                                              .multiply_rate = 0.1,
                                              .color = bacteria_color};

    if(*objects == NULL)
        current->next = NULL;
    else
        current->next = *objects;

    *objects = current;
}

void generate_type(int ***world, Type **objects, int n, char type)
{
    int x, y;
    int index;
    Color type_color;
    int **new_world;
          new_world = *world;

    if(type == food)
        index = 1;
    else if(type == bacteria)
        index = 2;

    for(int i = 0; i < n; i++)
    {
        x = random(ScreenWidth - border);
        y = random(ScreenHeight - border);

        new_world[x][y] = index;
    }

    *world = new_world;
}

void extract_by_index(void **obj, Type *objects, int index)
{
    if(objects == NULL)
        return;

    void *temp_obj;

    while(objects->index != index)
        objects = objects->next;

    if(index == 1)
        temp_obj = &objects->inanimate;
    else if(index == 2)
        temp_obj = &objects->bacterium;

    *obj = temp_obj;
}

/*
 * Control the behavior of bacterium
 * Every time it used a new random number
*/
void bacterium_behavior(int *x, int *y, float size)
{
    // Get a random number
    int rand_num = random(4);

    // Get the temporal small control
    //float size = speed;
    switch(rand_num)
    {
        // Go to S-E else go S
        case 0:
            if((*x + size) < ScreenWidth - border && (*y + size) < ScreenHeight - border)
            {
                *x += size;
                *y += size;
            }
            else if((*y + size) < ScreenHeight - border)
                *y += size;
            break;

        // Go to N-E else go E
        case 1:
            if((*x + size) < ScreenWidth - border && (*y - size) > border)
            {
                *x += size;
                *y -= size;
            }
            else if((*x + size) < ScreenWidth - border)
                *x += size;
            break;

        // Go to S-V else go V
        case 2:
            if((*x - size) > border && (*y + size) < ScreenHeight - border)
            {
                *x -= size;
                *y += size;
            }
            else if((*x - size) > border)
                *x -= size;
            break;

        // Go to S-E else go N
        case 3:
            if((*x - size) > border && (*y - size) > border)
            {
                *x -= size;
                *y -= size;
            }
            else if((*y - size) > border)
                *y -= size;
            break;
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
