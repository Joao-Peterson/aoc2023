#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include <math.h>
#include <raylib.h>
#include <rlgl.h>
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

typedef struct brick_t brick_t;
struct brick_t{
	int id;
	point_t head;
	point_t tail;

	brick_t *supports[50];
	int supportsSize;
	brick_t *supportedBy[50];
	int supportedBySize;
	
	bool needed;
};

typedef struct{
	array_t *bricks;
}puzzle_t;

void window(const puzzle_t *p);
void draw_bricks(const puzzle_t *p);
static void DrawTextCodepoint3D(Font font, int codepoint, Vector3 position, float fontSize, bool backface, Color tint);
static void DrawText3D(Font font, const char *text, Vector3 position, float fontSize, float fontSpacing, float lineSpacing, bool backface, Color tint);

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

// parse file
puzzle_t *parseInput(const string *data){
	puzzle_t *p = malloc(sizeof(puzzle_t));
	p->bricks = array_new_wconf(400, true);
	for(size_t i = 0; i < 400; i++)
		array_insert(p->bricks, i, list_new(true));

	string_ite ite = string_split(data, "\n");
	int b = 0;
	foreach(string, line, ite){
		string_ite sep = string_split(&line, "~,");
		
		brick_t *brick = calloc(1, sizeof(brick_t));
		brick->id = b;
		int coord = 0;
		foreach(string, num, sep){
			switch(coord){
				case 0: brick->head.x = string_to_int(&num, 10); break;
				case 1: brick->head.y = string_to_int(&num, 10); break;
				case 2: brick->head.z = string_to_int(&num, 10); break;
				case 3: brick->tail.x = string_to_int(&num, 10); break;
				case 4: brick->tail.y = string_to_int(&num, 10); break;
				case 5: brick->tail.z = string_to_int(&num, 10); break;
			}
			coord++;
		}

		list_t *l = array_get(p->bricks, minz(brick));
		list_push(l, brick);
		b++;
	}
	
	return p;
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

bool intersectXY(brick_t *a, brick_t *b){
	return 
		rangeCheck(a->head.x, a->tail.x, b->head.x, b->tail.x) &&
		rangeCheck(a->head.y, a->tail.y, b->head.y, b->tail.y);
}

void fallBrick(const puzzle_t *p, int max){
	int m = 0;
	
	// heights, from 2, cause bricks on 1 dont fall
	for(size_t i = 2; i < 400; i++){
		list_t *l = array_remove(p->bricks, i);
		list_t *lnew = list_new(true);

		// bricks on height
		for(brick_t *brick = list_queue_pop(l); brick != NULL; brick = list_queue_pop(l)){

			// default is a fall to the ground (1)
			int fall = minz(brick);

			// check against bricks below
			for(int h = i-1; h > 0; h--){
				list_t *layer = array_get(p->bricks, h);
				
				// for every brick below
				for(node_t *nodeBelow = layer->first; nodeBelow != NULL; nodeBelow = nodeBelow->next){
					brick_t *below = (brick_t*)nodeBelow->value;

					// if below can support
					if(intersectXY(brick, below)){
						// only when it can fall the least, since bigger bricks can start at a lower height, and therefore offer support before
						int f = minz(brick) - maxz(below);
						if(f < fall) fall = f;
					}				
				}
			}
			
			if(fall > 1){
				brick->head.z -= fall - 1;
				brick->tail.z -= fall - 1;
				list_push(array_get(p->bricks, minz(brick)), brick);				
				m++;
			}
			else{
				list_push(lnew, brick);
			}
		}

		list_destroy(l);
		array_insert(p->bricks, i, lnew);
		if(m >= max)
			return;
	}
}

bool supports(brick_t *a, brick_t *b){
	return intersectXY(a, b) && minz(b) == maxz(a) + 1;
}

uint64_t part1_old(const puzzle_t *p){
	uint64_t acc = 0;
	// for each height
	for(size_t h = 1; h < 400 - 1; h++){
		const list_t *l = array_get(p->bricks, h);

		list_ite list = list_iterate(l);
		// for each brick on layer
		foreach(brick_t*, brick, list){
			const list_t *lAbove = array_get(p->bricks, maxz(brick) +1);
			list_ite listAbove = list_iterate(lAbove);

			bool needed = false;
			// for each brick above that brick's height
			foreach(brick_t*, brickAbove, listAbove){
				bool supported = false;
				
				// check all heights below the above brick
				for(int i = maxz(brick); i > 0; i--){
					const list_t *cl = array_get(p->bricks, i);
					list_ite clist = list_iterate(cl);
					foreach(brick_t*, cbrick, clist){
						
						// where it's not our brick
						if(cbrick->id == brick->id) continue;

						// if it's supported without our brick
						if(supports(cbrick, brickAbove)){
							supported = true;
							break;
						}
					}

					// if it's supported without our brick
					if(supported) break;
				}

				// if it's not supported, then our brick is needed
				if(!supported){
					needed = true;
					break;
				}
			}

			// if our brick isn't needed, then we could remove it
			if(!needed){
				acc++;
			}
		}
	}

	return acc;
}

void buildTree(const puzzle_t *p){
	for(size_t h = 1; h < 400 - 1; h++){
		// layer
		const list_t *l = array_get(p->bricks, h);

		list_ite list = list_iterate(l);
		// for each brick on layer, compute supported bricks
		foreach(brick_t*, brick, list){
			// layer above
			const list_t *lAbove = array_get(p->bricks, maxz(brick) + 1);
			list_ite listAbove = list_iterate(lAbove);

			// for each brick above that brick's height
			foreach(brick_t*, brickAbove, listAbove){
				// if suppports, add edges
				if(supports(brick, brickAbove)){
					brick->supports[brick->supportsSize] = brickAbove;
					brick->supportsSize++;
					brickAbove->supportedBy[brickAbove->supportedBySize] = brick;
					brickAbove->supportedBySize++;
				}
			}
		}
	}
}

// trees are better :)
uint64_t part1(const puzzle_t *p){
	uint64_t acc = 0;

	for(size_t h = 1; h < 400 - 1; h++){
		// layer
		const list_t *l = array_get(p->bricks, h);
		list_ite list = list_iterate(l);

		foreach(brick_t*, brick, list){
			// if supports nothing
			if(brick->supportsSize == 0){
				acc++;
				continue;
			}

			brick->needed = false;
			for(int s = 0; s < brick->supportsSize; s++){
				if(brick->supports[s]->supportedBySize <= 1){
					brick->needed = true;
					break;
				}
			}

			if(!brick->needed) acc++;
		}
	}

	return acc;
}

priority_t brickUnique(void *a, void *b){
	const brick_t *ba = (brick_t*)a;
	const brick_t *bb = (brick_t*)b;
	return ba->id == bb->id ? priority_reject : priority_accept;
}

uint64_t collapse(brick_t *baseBrick){
	bool falling[1500] = {0};
	list_t *bricks = list_new(false);
	
	// mark base brick as falling
	falling[baseBrick->id] = true;

	// mark as falling and enqueue the bricks that are only supported by the base brick 
	for(int i = 0; i < baseBrick->supportsSize; i++){
		if(baseBrick->supports[i]->supportedBySize == 1){
			list_push_unique(bricks, baseBrick->supports[i], brickUnique);
			falling[baseBrick->supports[i]->id] = true;
		}
	}

	// for each falling brick
	for(
		brick_t *brick = list_queue_pop(bricks); 
		brick != NULL; 
		brick = list_queue_pop(bricks)
	){
		// check each of it's supported bricks
		for(int s = 0; s < brick->supportsSize; s++){
			brick_t *supported = brick->supports[s];

			// if not already falling
			if(falling[supported->id]) continue;

			// check if all supports of the supported brick have already fallen
			bool allFallen = true;
			for(int sb = 0; sb < supported->supportedBySize; sb++){
				if(!falling[supported->supportedBy[sb]->id]){
					allFallen = false;
					break;
				}
			}
			
			// and if all supports have fallen then this brick is also falling
			if(allFallen){
				list_push_unique(bricks, supported, brickUnique);
				falling[supported->id] = true;
			}
		}
	}

	// sum all bricks that are falling
	uint64_t acc = 0;
	for(size_t i = 0; i < 1500; i++)
		acc += falling[i];

	// minus the base brick
	return acc - 1;
}

uint64_t part2(const puzzle_t *p){
	uint64_t acc = 0;

	for(size_t h = 1; h < 400 - 1; h++){
		// layer
		const list_t *l = array_get(p->bricks, h);
		list_ite list = list_iterate(l);

		// for each brick compute how many bricks will fall if destroyed
		foreach(brick_t*, brick, list){
			acc += collapse(brick);
		}
	}

	return acc;
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
		fallBrick(p, INT32_MAX);
		buildTree(p);
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

void window(const puzzle_t *p){
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

	size_t iteration = 0;

    // DisableCursor();                    // Limit cursor to relative movement inside the window
    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second

    // Main game loop
    while(!WindowShouldClose()){        // Detect window close button or ESC key
        // Update
        UpdateCamera(&camera, CAMERA_FREE);
        if(IsKeyPressed('Z')) camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };

		// iteration
        if(IsKeyPressed(KEY_L)){
			fallBrick(p, 10000);
			iteration += 10000;
		};
        if(IsKeyPressed(KEY_K)){
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

void draw_bricks(const puzzle_t *p){
	// iterate over the height
	for(size_t i = 1; i < 400; i++){
		list_t *l = array_get(p->bricks, i);

		// all bricks on that height
		for(node_t *node = l->first; node != NULL; node = node->next){
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
			
			srand(brick->id + 1);
			Color color = {
				rand() % 255,
				rand() % 255,
				rand() % 255,
				255
			};

			DrawCubeV(pos, size, color);
			DrawCubeWiresV(pos, size, BLACK);
			string *num = string_sprint("%d", 10, brick->id);
			DrawText3D(GetFontDefault(), num->raw, (Vector3){pos.x, pos.y + size.y / 2 + 0.001, pos.z}, 5, 0.5, 5, false, BLACK);
			string_destroy(num);
		}
	}
}

// Draw codepoint at specified position in 3D space
static void DrawTextCodepoint3D(Font font, int codepoint, Vector3 position, float fontSize, bool backface, Color tint)
{
    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned points to '?'
    int index = GetGlyphIndex(font, codepoint);
    float scale = fontSize/(float)font.baseSize;

    // Character destination rectangle on screen
    // NOTE: We consider charsPadding on drawing
    position.x += (float)(font.glyphs[index].offsetX - font.glyphPadding)/(float)font.baseSize*scale;
    position.z += (float)(font.glyphs[index].offsetY - font.glyphPadding)/(float)font.baseSize*scale;

    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for outline/glow shader effects
    Rectangle srcRec = { font.recs[index].x - (float)font.glyphPadding, font.recs[index].y - (float)font.glyphPadding,
                         font.recs[index].width + 2.0f*font.glyphPadding, font.recs[index].height + 2.0f*font.glyphPadding };

    float width = (float)(font.recs[index].width + 2.0f*font.glyphPadding)/(float)font.baseSize*scale;
    float height = (float)(font.recs[index].height + 2.0f*font.glyphPadding)/(float)font.baseSize*scale;

    if (font.texture.id > 0)
    {
        const float x = 0.0f;
        const float y = 0.0f;
        const float z = 0.0f;

        // normalized texture coordinates of the glyph inside the font texture (0.0f -> 1.0f)
        const float tx = srcRec.x/font.texture.width;
        const float ty = srcRec.y/font.texture.height;
        const float tw = (srcRec.x+srcRec.width)/font.texture.width;
        const float th = (srcRec.y+srcRec.height)/font.texture.height;

        // if(SHOW_LETTER_BOUNDRY) DrawCubeWiresV((Vector3){ position.x + width/2, position.y, position.z + height/2}, (Vector3){ width, LETTER_BOUNDRY_SIZE, height }, LETTER_BOUNDRY_COLOR);

        rlCheckRenderBatchLimit(4 + 4*backface);
        rlSetTexture(font.texture.id);

        rlPushMatrix();
            rlTranslatef(position.x, position.y, position.z);

            rlBegin(RL_QUADS);
                rlColor4ub(tint.r, tint.g, tint.b, tint.a);

                // Front Face
                rlNormal3f(0.0f, 1.0f, 0.0f);                                   // Normal Pointing Up
                rlTexCoord2f(tx, ty); rlVertex3f(x,         y, z);              // Top Left Of The Texture and Quad
                rlTexCoord2f(tx, th); rlVertex3f(x,         y, z + height);     // Bottom Left Of The Texture and Quad
                rlTexCoord2f(tw, th); rlVertex3f(x + width, y, z + height);     // Bottom Right Of The Texture and Quad
                rlTexCoord2f(tw, ty); rlVertex3f(x + width, y, z);              // Top Right Of The Texture and Quad

                if (backface)
                {
                    // Back Face
                    rlNormal3f(0.0f, -1.0f, 0.0f);                              // Normal Pointing Down
                    rlTexCoord2f(tx, ty); rlVertex3f(x,         y, z);          // Top Right Of The Texture and Quad
                    rlTexCoord2f(tw, ty); rlVertex3f(x + width, y, z);          // Top Left Of The Texture and Quad
                    rlTexCoord2f(tw, th); rlVertex3f(x + width, y, z + height); // Bottom Left Of The Texture and Quad
                    rlTexCoord2f(tx, th); rlVertex3f(x,         y, z + height); // Bottom Right Of The Texture and Quad
                }
            rlEnd();
        rlPopMatrix();

        rlSetTexture(0);
    }
}

// Draw a 2D text in 3D space
static void DrawText3D(Font font, const char *text, Vector3 position, float fontSize, float fontSpacing, float lineSpacing, bool backface, Color tint)
{
    int length = TextLength(text);          // Total length in bytes of the text, scanned by codepoints in loop

    float textOffsetY = 0.0f;               // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;               // Offset X to next character to draw

    float scale = fontSize/(float)font.baseSize;

    for (int i = 0; i < length;)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;

        if (codepoint == '\n')
        {
            // NOTE: Fixed line spacing of 1.5 line-height
            // TODO: Support custom line spacing defined by user
            textOffsetY += scale + lineSpacing/(float)font.baseSize*scale;
            textOffsetX = 0.0f;
        }
        else
        {
            if ((codepoint != ' ') && (codepoint != '\t'))
            {
                DrawTextCodepoint3D(font, codepoint, (Vector3){ position.x + textOffsetX, position.y, position.z + textOffsetY }, fontSize, backface, tint);
            }

            if (font.glyphs[index].advanceX == 0) textOffsetX += (float)(font.recs[index].width + fontSpacing)/(float)font.baseSize*scale;
            else textOffsetX += (float)(font.glyphs[index].advanceX + fontSpacing)/(float)font.baseSize*scale;
        }

        i += codepointByteCount;   // Move text bytes counter to next codepoint
    }
}