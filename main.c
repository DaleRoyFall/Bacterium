#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <raylib.h>
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

/*  Type of cells:
        b - bacteria
        f - food
        s - space
*/
typedef struct
{
    char  type;
    int   index;
    float x;
    float y;
    float speed;
    float multiply_rate;
    Color color;
}micro_type;

// Main menu
void menu();

// Return a random number every frame
int random(int n);

// Give some behavior to every bacterium
void bacterium_behavior(float *x, float *y, float speed);
//void bacterium_behavior(int *x, int *y, int speed);

// Draw World Matrix
//void draw_world();

// Generate type
void generate_type(micro_type ***world, int n, char type)
{
    int x, y;
    int index;
    Color type_color;
    micro_type **new_world;
                new_world = *world;

    for(int i = 0; i < n; i++)
    {
        x = random(ScreenWidth);
        y = random(ScreenHeight);
        if(type == bacteria)
        {
            type_color = bacteria_color;
            index = i + 1;
        }
        else if(type == food)
        {
            type_color = food_color;
            index = 0;
        }

        new_world[x][y] = (micro_type) {.index = index,
                                        .x = x,
                                        .y = y,
                                        .type = type,
                                        .color = type_color};
    }

    *world = new_world;
}

// Concatenate string and int
char* strcpy(char *str, int n)
{
    int i = 0,
        k = 0;

    // Determine length(str)
    while(str[i] != '\0')
        i++;

    // Convert int in char
    //----------------------------------------
    char *temp_str = malloc(sizeof(int) * 10);
    while(n != 0)
    {
        temp_str[k] = 48 + (n % 10);
        n /= 10;
        i++;    k++;
    }
    temp_str[k] = '\0';
    //----------------------------------------

    // Create a buffer for concatenation
    char *buff = malloc(sizeof(char) * i);

    // Copy string in buffer
    i = 0;
    while(str[i] != '\0')
    {
        buff[i] = str[i];
        i++;
    }

    // Copy int in buffer
    while(k-->= 0)
    {
        buff[i] = temp_str[k];
        i++;
    }

    buff[i] = '\0';

    // Deallocate memory
    free(temp_str);

    return buff;
}

// Draw statistic of selected bacteria
void draw_bacterium_stats(int x, int y, int index, int size)
{
    char *str = strcpy("Index = ", index);

    // Draw stats interface
    DrawRectangle(x, y - 50, 100, 50, BLACK);
    DrawText(str, x + 5, y - 25, 14, WHITE);
    free(str);
}

// Generate bacteria
// Number of start bacteria
int bacteria_counter = 10;

// Main
int main()
{
    // World in a 2D matrix
    micro_type **world;

    // Allocation memory to world's matrix
    //------------------------------------------------------
    world = malloc(sizeof(micro_type*) * ScreenWidth);

    for(int i = 0; i < ScreenWidth; i++)
        world[i] = malloc(sizeof(micro_type) * ScreenHeight);
    //------------------------------------------------------

    // Initialize world
    //------------------------------------------------------
    for(int i = 0; i < ScreenWidth; i++)
        for(int j = 0; j < ScreenHeight; j++)
            world[i][j] = (micro_type){.x = i,
                                       .y = j,
                                       .type = space,
                                       .speed = 0,
                                       .multiply_rate = 0,
                                       .color = WHITE};
    //------------------------------------------------------

    // Bacteria coordinates in integer
    int x, y;

    // Statistics used for draw_bacterium_stats
    micro_type stats = {.index = -1};

    // Initialize Window
    InitWindow(ScreenWidth, ScreenHeight, "Pandora");
    SetTargetFPS(30);

    // Generate bacteria and food
    //-------------------------------------------
    generate_type(&world, 100, food);
    generate_type(&world, bacteria_counter, bacteria);
    //-------------------------------------------

    // Start with Main Menu
    //menu();

    // Temporal cell
    micro_type temp;

    while(!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(WHITE);

            // Draw World
            for(int i = 0; i < ScreenWidth; i++)
                for(int j = 0; j < ScreenHeight; j++)
                {
                    if(world[i][j].type == food)
                        DrawPixel(i, j, world[i][j].color);
                    else if(world[i][j].type == bacteria)
                    {

                        bacterium_behavior(&world[i][j].x, &world[i][j].y, 0.4);

                        x = world[i][j].x;
                        y = world[i][j].y;

                        DrawCircle(x, y, 3, world[i][j].color);

                        // Swap
                        temp = world[i][j];
                        world[i][j] = world[x][y];
                        world[x][y] = temp;

                        //if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                            if(IsMouseHere(i - 10, j - 10, 20, 20) && world[i][j].index != 0)
                                stats = world[i][j];

                        if(world[i][j].index == stats.index)
                        {
                            stats.x = world[i][j].x;
                            stats.y = world[i][j].y;
                            stats.index = world[i][j].index;
                        }

                        draw_bacterium_stats(stats.x, stats.y, stats.index, 3);
                    }
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

/*
 * Control the behavior of bacterium
 * Every time it used a new random number
*/
void bacterium_behavior(float *x, float *y, float speed)
{
    // Get a random number
    int rand_num = random(4);

    // Get the temporal small control
    float size = speed;
    switch(rand_num)
    {
        // Go to S-E else go S
        case 0:
            if((*x + size) < ScreenWidth - 10 && (*y + size) < ScreenHeight - 10)
            {
                *x += size;
                *y += size;
            }
            else if((*y + size) < ScreenHeight - 10)
                *y += size;
            break;

        // Go to N-E else go E
        case 1:
            if((*x + size) < ScreenWidth - 10 && (*y - size) > 10)
            {
                *x += size;
                *y -= size;
            }
            else if((*x + size) < ScreenWidth - 10)
                *x += size;
            break;

        // Go to S-V else go V
        case 2:
            if((*x - size) > 10 && (*y + size) < ScreenHeight - 10)
            {
                *x -= size;
                *y += size;
            }
            else if(*x > 1)
                *x -= size;
            break;

        // Go to S-E else go N
        case 3:
            if((*x - size) > 10 && (*y - size) > 10)
            {
                *x -= size;
                *y -= size;
            }
            else if((*y - size) > 10)
                *y -= size;
            break;
    }
}
