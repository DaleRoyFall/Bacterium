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
int bacteria_counter = 10;

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
    float speed;
    float multiply_rate;
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
DNA gen_DNA(int width, int height)
{
    DNA DNA;
    DNA.ma_width = width,  DNA.ma_height = height;
    DNA.matrix = malloc(sizeof(int*) * height);

    for(int i = 0; i < height; i++)
    {
        //printf("\n");
        DNA.matrix[i] = malloc(sizeof(int) * width);
        for(int j = 0; j < width; j++)
        {
            DNA.matrix[i][j] = random(4);
            //printf("%d\t", DNA.matrix[i][j]);
        }
    }
    //printf("\n");

    return DNA;
}
//-------------------------------------------------

/*// Decrementation health
void decr_health()
{

}*/

// Extract object from list
Type extract_by_index(Type **objects, int index);

// Destroy object
void destr_object(Type **objects, int index);

// Register
//-----------------------------------------------
// Add data in register
void add_in_reg(Coord_Reg **reg, int x, int y);

// Verify if coordinate are contains in register
bool verify_reg(Coord_Reg *reg, int x, int y);
//-----------------------------------------------

// Give some behavior to every bacterium
void bacterium_behavior(int *x, int *y, int rand_num, float speed);

// Draw statistic of selected bacteria
void draw_bacterium_stats(int x, int y, int index, int size);

void print_obj(Type *objects)
{
    while(objects != NULL)
    {
        printf("(%d, %c)\n", objects->index, objects->type);
        objects = objects->next;
    }
    printf("\n");
}

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

    //print_obj(objects);

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
                        //printf("main - (%d)\n", obj.index);
                        if(obj.type != space)
                            if(obj.type == food)
                            {
                                obj_food = obj.inanimate;
                                //printf("%d == %d\n", ColorToInt(LIME), ColorToInt(obj_food.color));
                                DrawCircle(i, j, 2, obj_food.color);
                            }
                            else if(obj.type == bacteria)
                            {
                                obj_bacteria = obj.bacterium;
                                x = i, y = j;
                                if(!verify_reg(reg, x, y))
                                {
                                    for(int m = 0; m < obj_bacteria.DNA.ma_height; m++)
                                        for(int n = 0; n < obj_bacteria.DNA.ma_width; n++)
                                        {
                                            bacterium_behavior(&x, &y, obj_bacteria.DNA.matrix[m][n], obj_bacteria.speed);
                                            DrawRectangle(x, y, 3, 3, obj_bacteria.color);
                                        }



                                    add_in_reg(&reg, x, y);

                                    temp = world[x][y];
                                    world[x][y] = world[i][j];
                                    world[i][j] = temp;
                                }

                                /*if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                                    if(IsMouseHere((int)x - 5,
                                                   (int)y - 5, 10, 10))
                                        stats = obj_bacteria;

                                    if(bacterium[i].index == stats.index && stats.index != 0)
                                        stats = bacterium[i];

                                    draw_bacterium_stats((int)stats.x, (int)stats.y, stats.index, 3);*/
                            }
                    }

            free(reg);
            reg = NULL;

            print_obj(objects);

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
    //free(obj);
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
    Type *current;
          current = malloc(sizeof(Type));

    current->index = *index;
    if(type == food)
    {
        current->type = food;
        current->inanimate = (micro_type) {.color = food_color};
    }
    else if(type == bacteria)
    {
        current->type = bacteria;
        current->bacterium = (type_bacteria) {.health = random(100),
                                              .speed = 1,
                                              .multiply_rate = 0.1,
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
    Color type_color;
    int **new_world;
          new_world = *world;

    for(int i = 1; i <= n; i++)
    {
        add_object(objects, index, type);

        x = random(ScreenWidth - border);
        y = random(ScreenHeight - border);

        new_world[x][y] = *index;

        (*index)++;
    }

    *world = new_world;
}

Type extract_by_index(Type **objects, int index)
{
    Type *current = *objects;
    Type indexed_object;
         indexed_object.type = space;

    if(objects == NULL)
        return;

    while(current != NULL && current->index != index)
        current = current->next;

    // Decrementation health
    if(current->type == bacteria)
        current->bacterium.health--;

    if(current == NULL)
        return indexed_object;

    indexed_object = *current;

    // Destroy object
    if(current->bacterium.health == 0)
    {

        printf("\nindex = %d\n", current->index);
        destr_object(objects, index);

        print_obj(*objects);
    }

    return indexed_object;
}

void destr_object(Type **objects, int index)
{
    Type *current = *objects,
         *prev = NULL;
    if(index==0)
    {
        prev = *objects;
        *objects=prev->next;
        free(prev);
    } else
    {
    prev = current;
    while(current->next != NULL && current->index != index)
    {
        prev = current;
        current = current->next;
    }
    if(current->index==index)
    {
    prev->next = current->next;
    free(current);
    } else printf("\nMai Oleg ai un BUG :)  [ index out of range] \n");

    }
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
//-------------------------------------------------

/*
 * Control the behavior of bacterium
 * Every time it used a new random number
*/
void bacterium_behavior(int *x, int *y, int rand_num, float size)
{
    // Get a random number
    //int rand_num = random(4);

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
