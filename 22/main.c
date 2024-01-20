#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include <math.h>
#include <raylib.h>
#include "src/worker.h"
#include "src/ite.h"
#include "src/string+.h"
#include "src/number.h"
#include "src/hash.h"
#include "src/data.h"

typedef struct{
	int x;
	int y;
	int z;
}point_t;

typedef struct{
	int id;
	point_t head;
	point_t tail;
}brick_t;

typedef struct{
	brick_t *bricks;
	size_t bricksSize;
	list_t *orderedBricks;
}puzzle_t;

int minx(brick_t *brick){
	return (brick->head.x < brick->tail.x ? brick->head.x : brick->tail.x);
}
int miny(brick_t *brick){
	return (brick->head.y < brick->tail.y ? brick->head.y : brick->tail.y);
}
int minz(brick_t *brick){
	return (brick->head.z < brick->tail.z ? brick->head.z : brick->tail.z);
}
int maxz(brick_t *brick){
	return (brick->head.z < brick->tail.z ? brick->tail.z : brick->head.z);
}

priority_t brickCmp(void *pa, void *pb){
	brick_t *a = (brick_t*)pa;
	brick_t *b = (brick_t*)pb;

	if(minz(a) < minz(b))
		return priority_right;
	else
		return priority_left;
}

// parse file
puzzle_t *parseInput(const string *data){
	puzzle_t *p = malloc(sizeof(puzzle_t));
	p->bricks = calloc(1500, sizeof(brick_t));
	p->bricksSize = 0;
	p->orderedBricks = list_new(false);

	string_ite ite = string_split(data, "\n");
	int b = 0;
	foreach(string, line, ite){
		string_ite sep = string_split(&line, "~,");
		
		brick_t brick = {0};
		brick.id = b;
		int coord = 0;
		foreach(string, num, sep){
			switch(coord){
				case 0: brick.head.x = string_to_int(&num, 10); break;
				case 1: brick.head.y = string_to_int(&num, 10); break;
				case 2: brick.head.z = string_to_int(&num, 10); break;
				case 3: brick.tail.x = string_to_int(&num, 10); break;
				case 4: brick.tail.y = string_to_int(&num, 10); break;
				case 5: brick.tail.z = string_to_int(&num, 10); break;
			}
			coord++;
		}

		p->bricks[b] = brick;
		list_priority_push(p->orderedBricks, &(p->bricks[b]), brickCmp);

		// printf("%d: %d,%d,%d - %d,%d,%d\n", brick, p->bricks[brick].head.x, p->bricks[brick].head.y, p->bricks[brick].head.z, p->bricks[brick].tail.x, p->bricks[brick].tail.y, p->bricks[brick].tail.z);

		b++;
		p->bricksSize++;
	}
	
	return p;
}

void draw_bricks(puzzle_t *p){
	srand(10);

	for(node_t *node = p->orderedBricks->first; node != NULL; node = node->next){
		brick_t *brick = (brick_t*)node->value;
		Vector3 size = {
			(float)abs(brick->head.x - brick->tail.x) + 1,
			(float)abs(brick->head.z - brick->tail.z) + 1,
			(float)abs(brick->head.y - brick->tail.y) + 1,
		};

		Vector3 pos = {
			size.x / 2.0f + minx(brick),
			size.y / 2.0f + minz(brick) - 1,
			size.z / 2.0f + miny(brick),
		};
		
		Color color = {
			rand() % 255,
			rand() % 255,
			rand() % 255,
			255
		};

		DrawCubeV(pos, size, color);
		DrawCubeWiresV(pos, size, BLACK);
	}
}

bool rangeCheck(int x1, int x2, int y1, int y2){
	if(x2 < x1){
		x1 += x2;
		x2 = x1 - x2;
	}

	if(y2 < y1){
		y1 += y2;
	}

	if(
		(y1 >= x1 && y1 <= x2) ||
		(y2 >= x1 && y1 <= x2)
	)
		return true;
	
	return false;
}

bool supported(brick_t *a, brick_t *b){
	return 
		rangeCheck(a->head.x, a->tail.x, b->head.x, b->tail.x) &&
		rangeCheck(a->head.y, a->tail.y, b->head.y, b->tail.y);
}

void *list_pop_node(list_t *l, node_t *node){
	void *res = NULL;

	if(node == l->first){
		l->first = node->next;
		if(l->size == 1)
			l->last = NULL;
		else
			l->first->prev = NULL;
	}
	else if(node == l->last){
		l->last = node->prev;
		if(l->size == 1)
			l->first = NULL;
		else
			node->prev->next = NULL;
	}
	else{
		node->prev->next = node->next;
		node->next->prev = node->prev;
	}

	res = node->value;
	free(node);
	l->size--;
	return res;
}

void fallBrick(puzzle_t *p, int max){
	int i = 0;
	for(node_t *brickNode = p->orderedBricks->first; brickNode != NULL; brickNode = brickNode->next){
		brick_t *brick = (brick_t*)brickNode->value;

		if(minz(brick) <= 1) continue;

		// check against bricks below
		for(node_t *node = brickNode->prev; node != NULL; node = node->prev){
			brick_t *below = (brick_t*)node->value;

			if(supported(brick, below)){
				int fall = minz(brick) - (maxz(below) + 1);
				brick->head.z -= fall;
				brick->tail.z -= fall;

				// reorder list
				if(fall > 0){
					list_pop_node(p->orderedBricks, brickNode);
					list_priority_push(p->orderedBricks, brick, brickCmp);
					brickNode = node;
					
					i++;
					if(i >= max) return;
				}

				break;
			}
		}
	}
}

uint64_t part1(puzzle_t *p){
	return 0;
}

uint64_t part2(puzzle_t *p){
	return 0;
}

void window(puzzle_t *p){
	const int screenWidth = 800;
    const int screenHeight = 450;

	// SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera free");
	
    // Define the camera to look into our 3d world
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    Vector3 cubePosition = { 0.0f, 1.0f, 0.0f };

	size_t iteration = 0;

    // DisableCursor();                    // Limit cursor to relative movement inside the window
    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second

    // Main game loop
    while(!WindowShouldClose()){        // Detect window close button or ESC key
        // Update
        UpdateCamera(&camera, CAMERA_FREE);
        if(IsKeyPressed('Z')) camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };

		// iteration
        if(IsKeyPressed(KEY_LEFT_SHIFT)){
			fallBrick(p, 1);
			iteration++;
		};

        // Draw
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                draw_bricks(p);

                DrawGrid(10, 1.0f);
				DrawLine3D((Vector3){0}, (Vector3){10, 0, 0}, RED);
				DrawLine3D((Vector3){0}, (Vector3){0, 0, 10}, GREEN);
				DrawLine3D((Vector3){0}, (Vector3){0, 10, 0}, BLUE);

            EndMode3D();

			// ui
            DrawRectangle( 10, 10, 200, 150, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines( 10, 10, 200, 150, BLUE);
            DrawText("Free camera default controls:", 		20, 20, 10, BLACK);
            DrawText("- Mouse Wheel to Zoom in-out",  		40, 40, 10, DARKGRAY);
            DrawText("- Mouse Wheel Pressed to Pan",  		40, 60, 10, DARKGRAY);
            DrawText("- Z to zoom to (0, 0, 0)",      		40, 80, 10, DARKGRAY);
			string *ite = string_sprint("Iteration: %d", 50, iteration);
            DrawText(ite->raw, 		20, 100, 10, BLACK);
			string_destroy(ite);

        EndDrawing();
    }

    CloseWindow();        // Close window and OpenGL context
}

int main(int argc, char**argv){
	string *data;
	
	// from arg filename
	if(argc > 1){
		data = string_from_filename(argv[1], NULL);
	}
	// from pipe
	else{
		data = string_from_file(stdin, NULL);
	}

	bool interactive = false;
	if(argc > 2)
		interactive = (argv[2]);

	puzzle_t *p = parseInput(data);
	
	// resolution
	if(!interactive){
		printf("Part 1: %lu\n", part1(p));
		printf("Part 2: %lu\n", part2(p));
	}
	// interactive
	else{
		window(p);
	}

	string_destroy(data);
	return 0;
}