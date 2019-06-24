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

// Global variables
//---------------------------------
// Wall's border
int border = 10;

// Number of start bacteria
int bacteria_counter = 1000;

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
    int speed;
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
        DNA.matrix[i] = malloc(sizeof(int) * width);
        for(int j = 0; j < width; j++)
            DNA.matrix[i][j] = random(4);
    }

    return DNA;
}
//-------------------------------------------------

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
void bacterium_behavior(int *x, int *y, int rand_num, int speed);

// Verify collision between two bacteria
int verify_collision(int **world, int i, int j, int x, int y, int size);

// If bacterium meet another one then multiply
void multiply(Type **objects ,int thrs_index, int scnd_index);

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

    int n = 0, m = 0;

    //print_obj(objects);
    //Type stats;
    Coord_Reg new_collision = {.x = 0,
                               .y = 0};

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
                        //printf("\nExtract -> (%d)\n", world[i][j]);
                        if(obj.type == space)
                            world[i][j] = 0;
                        else//if(obj.type != space)
                            if(obj.type == food)
                            {
                                obj_food = obj.inanimate;
                                DrawCircle(i, j, 2, obj_food.color);
                            }
                            else if(obj.type == bacteria)
                            {
                                obj_bacteria = obj.bacterium;
                                x = i, y = j;
                                if(!verify_reg(reg, x, y))
                                {
                                    bacterium_behavior(&x, &y, obj_bacteria.DNA.matrix[m][n], obj_bacteria.speed);
                                    //verify_collision(world, i, j, x, y, 3);
                                    DrawRectangle(x, y, 4, 4, obj_bacteria.color);
                                    //DrawPixel(x, y, obj_bacteria.color);

                                    add_in_reg(&reg, x, y);

                                    temp = world[x][y];
                                    world[x][y] = world[i][j];
                                    world[i][j] = temp;
                                }

                                /*if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                                    if(IsMouseHere((int)x - 5,
                                                   (int)y - 5, 10, 10))
                                        stats = obj_bacteria;

                                    if(world[i][j] == stats.index && stats.index != 0)
                                        stats = world[i][j];

                                    draw_bacterium_stats((int)stats.x, (int)stats.y, stats.index, 3);*/
                            }
                    }

            free(reg);
            reg = NULL;

            if(m == 0 && n == 0 || m == 0 && n == 1)
                m = 1;
            else if(m == 1 && n == 0)
                n = 1;
            else if(m == 1 && n == 1)
            {
                m = 0;
                n = 0;
            }

            //print_obj(objects);

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

        //printf("\n%d\n", current->bacterium.health);
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

    while(current != NULL && current->index != index)
        current = current->next;

    // If current is NULL then return space
    if(current == NULL)
        return indexed_object;

    // Decrementation health
    if(current->type == bacteria)
        current->bacterium.health--;

    indexed_object = *current;

    // Destroy object
    if(current->bacterium.health == 0)
        destr_object(objects, index);

    return indexed_object;
}

void destr_object(Type **objects, int index)
{
    Type *current = *objects,
         *prev = NULL;

    if(index == 0)
    {
        prev = *objects;
        *objects = prev->next;
        free(prev);
    }
    else
    {
        prev = current;

        while(current->next != NULL && current->index != index)
        {
            prev = current;
            current = current->next;
        }

        /*if(current->next == NULL)
        {
            prev->next = NULL;
            free(current);
            return;
        }*/

        if(current->index == index)
        {
            prev->next = current->next;
            free(current);
        }
    }
}

void free_DNA(DNA *DNA)
{
    for(int i = 0; i < DNA->ma_height; i++)
        free((*DNA).matrix[i]);
    free((*DNA).matrix);
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
void bacterium_behavior(int *x, int *y, int rand_num, int size)
{
    // Get a random number
    //int rand_num = random(4);
    //printf("\n%d\n", rand_num);

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

int verify_collision(int **world, int i, int j, int x, int y, int size)
{
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
        // Go to superior extreme (i - size)
        j -= (size - 1);

        // Go to right/left extreme
        if(diff_x > 0)
            i += size;
        else if(diff_x < 0)
            i -= size;

        for(int m = 0; m < size*2 - 1; m++)
            if(world[i][j + m] != 0)
            {
                DrawRectangle(i + m - 2, j - 2, 10, 10, RED);
                return world[i + m][j];
            }
    }
    // When bacteria go to North/South
    // Verify if to the up/down border is collision
    // Verify down/up border plus extra (size-1) pixels right and left
    else if(diff_x == 0)
    {
        i -= (size - 1);

        if(diff_y < 0)
            j -= size;
        else if(diff_y > 0)
            j += size;

        for(int m = 0; m < size*2 - 1; m++)
            if(world[i + m][j] != 0)
            {
                DrawRectangle(i - 2, j + m - 2, 10, 10, RED);
                return world[i][j + m];
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
                return world[i][j + m];
            }
            else if(world[i + m][j] != 0)
            {
                DrawRectangle(i - 2, j + m - 2, 10, 10, RED);
                return world[i][j + m];
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
                return world[i][j + m];
            }
            else if(world[i + m][j] != 0)
            {
                DrawRectangle(i - 2, j + m - 2, 10, 10, RED);
                return world[i][j+m];
            }
    }

}

void multiply(Type **objects ,int thrs_index, int scnd_index)
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
    DNA thrs_DNA, scnd_DNA;

    // If parents have the the same size of DNA
    if(thrs_DNA.ma_width == scnd_DNA.ma_width &&
       thrs_DNA.ma_height == scnd_DNA.ma_height)
    {
        // If parents have the same sizes of DNA
        // then copy it in new variables
        int ma_width = thrs_DNA.ma_width;
        int ma_height = thrs_DNA.ma_height;

        // Copy parent's DNA structure in new matrix
        int **thrs_matrix = NULL;
              thrs_matrix = thrs_DNA.matrix;

        int **scnd_matrix = NULL;
              scnd_matrix = scnd_DNA.matrix;

        /*for(int i = 0; i < ma_height; i++)
        {
            printf("\n");
            for(int j = 0; j < ma_width; j++)
                printf("%d ", thrs_matrix[i][j]);

        }*/

        // Child's DNA
        DNA thrs_child, scnd_child,
            thrd_child, frth_child;

        int matrix[ma_height][ma_width];

        // Create children's DNA structure
        for(int i = 0; i < ma_height; i++)
            for(int j = 0; j < ma_width; j++)
            {
                // Thirst child
                thrs_child.matrix[i][j] = thrs_matrix[i][j];
                thrs_child.matrix[ma_height - i][j] = scnd_matrix[ma_height - i][j];

                // Second child
                scnd_child.matrix[i][j] = scnd_matrix[i][j];
                scnd_child.matrix[ma_height - i][j] = thrs_matrix[ma_height - i][j];

                // Third child
                thrd_child.matrix[j][i] = thrs_matrix[j][i];
                thrd_child.matrix[ma_height - j][i] = scnd_matrix[ma_height - j][i];

                // Forth child
                frth_child.matrix[j][i] = scnd_matrix[j][i];
                frth_child.matrix[ma_height - j][i] = thrs_matrix[ma_height - j][i];
            }

            // Children's DNA height
            thrs_child.ma_height = scnd_child.ma_height =
            thrs_child.ma_height = frth_child.ma_height = ma_height;

            // Children's DNA weight
            thrs_child.ma_width = scnd_child.ma_width =
            thrs_child.ma_width = frth_child.ma_width = ma_width;

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
