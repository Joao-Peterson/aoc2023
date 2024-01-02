#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include <math.h>
#include "src/worker.h"
#include "src/utils.h"
#include "src/string+.h"
#include "src/number.h"
#include "src/hash.h"
#include "src/data.h"

void printMatrix(char **m, size_t w, size_t h){
	for(size_t i = 0; i < h; i++){
		printf("%.*s\n", (int)w, m[i]);
	}
}

typedef struct{
	char param;
	char cmp;
	int value;
	string *jump;
}step_t;

typedef struct{
	string *name;
	step_t steps[10];
	size_t stepsSize;
	string *fallback;
}workflow_t;

typedef struct{
	int x;
	int m;
	int a;
	int s;
}part_t;

typedef struct{
	workflow_t workflows[600];
	size_t workflowsSize;
	part_t parts[250];
	size_t partsSize;
}sheet_t;

void parseWorkflow(const string *s, workflow_t *workflow){
	string_ite_t split = string_split(s, "{}");


	workflow->name = string_next(&split);
	string *steps = string_next(&split);

	string_ite_t splitSteps = string_split(steps, ",");
	for(string *step = string_next(&splitSteps); step != NULL; step = string_next(&splitSteps)){

		if(string_contains(step, ":")){
			string_ite_t ss = string_split(step, ":");

			string *value = string_next(&ss); 
			string *jump = string_next(&ss); 
			
			step_t newstep = {
				.param = step->raw[0],
				.cmp = step->raw[1],
				.value = strtod(value->raw + 2, NULL),
				.jump = jump
			};

			workflow->steps[workflow->stepsSize] = newstep;
			workflow->stepsSize++;

			string_destroy(value);
			string_destroy(step);
		}
		else{
			workflow->fallback = step;
			break;
		}
	}

	string_destroy(steps);
}

void parsePart(const string *s, part_t *part){
	string_ite_t split = string_split(s, "{},");
	for(size_t i = 0; i < 4; i++){
		string *token = string_next(&split);
		switch(i){
			case 0:
				part->x = strtod(token->raw + 2, NULL);
			break;
			case 1:
				part->m = strtod(token->raw + 2, NULL);
			break;
			case 2:
				part->a = strtod(token->raw + 2, NULL);
			break;
			case 3:
				part->s = strtod(token->raw + 2, NULL);
			break;
		}
			
		string_destroy(token);
	}
}

// parse file
void *parseInput(const string *data){

	sheet_t *sheet = calloc(1, sizeof(sheet_t));

	string_ite_t iterator = string_split(data, "\n");
	bool workflows = true;
	for(string *line = string_next(&iterator); line != NULL; line = string_next(&iterator)){
		// change
		if(string_cmp_raw(line, " ")){
			workflows = false;
			continue;
		}
		
		if(workflows){
			workflow_t w = {0};
			parseWorkflow(line, &w);
			sheet->workflows[sheet->workflowsSize] = w;
			sheet->workflowsSize++;
		}
		else{
			part_t p = {0};
			parsePart(line, &p);
			sheet->parts[sheet->partsSize] = p;
			sheet->partsSize++;
		}

		string_destroy(line);
	}

	// for(size_t i = 0; i < sheet->workflowsSize; i++){
	// 	printf("%s{", sheet->workflows[i].name->raw);

	// 	for(size_t j = 0; j < sheet->workflows[i].stepsSize; j++){
	// 		printf("%c%c%d:%s", sheet->workflows[i].steps[j].param, sheet->workflows[i].steps[j].cmp, sheet->workflows[i].steps[j].value, sheet->workflows[i].steps[j].jump->raw);
	// 		printf(",");
	// 	}
		
	// 	printf("%s", sheet->workflows[i].fallback->raw);
	// 	printf("}\n");
	// }

	// printf("\n");
	// for(size_t i = 0; i < sheet->partsSize; i++){
	// 	printf("{x=%d,m=%d,a=%d,s=%d}\n", sheet->parts[i].x, sheet->parts[i].m, sheet->parts[i].a, sheet->parts[i].s);
	// }

	return sheet;
}

bool applyStep(step_t step, part_t part){
	int value;
	switch(step.param){
		case 'x':
			value = part.x;
		break;
		case 'm':
			value = part.m;
		break;
		case 'a':
			value = part.a;
		break;
		case 's':
			value = part.s;
		break;
	}

	if(step.cmp == '>')
		return value > step.value;
	else
		return value < step.value;
}

bool applyWorkflow(const sheet_t *sheet, part_t part, const string *workflow){
	workflow_t w;

	// end condition
	if(workflow->raw[0] == 'A')
		return true;
	else if(workflow->raw[0] == 'R')
		return false;
	
	// search
	for(size_t i = 0; i < sheet->workflowsSize; i++){
		w = sheet->workflows[i];
		if(string_cmp(w.name, workflow))
			break;
	}

	for(size_t s = 0; s < w.stepsSize; s++){
		if(applyStep(w.steps[s], part))
			return applyWorkflow(sheet, part, w.steps[s].jump);
	}

	return applyWorkflow(sheet, part, w.fallback);
}

uint64_t part1(const sheet_t *sheet){
	uint64_t acc = 0;
	for(size_t i = 0; i < sheet->partsSize; i++){
		if(applyWorkflow(sheet, sheet->parts[i], string_from("in"))){
			acc += sheet->parts[i].x;
			acc += sheet->parts[i].m;
			acc += sheet->parts[i].a;
			acc += sheet->parts[i].s;
		}
	}
	return acc;
} 

typedef struct{
	int min;
	int max;
}range_t;

typedef struct{
	range_t param[4];
}ranges_t;

uint64_t applyRangeOnWorkflow(const sheet_t *sheet, ranges_t ranges, const string *workflow, size_t s){
	// no possibilities
	if(workflow->raw[0] == 'R')
		return 0;

	uint64_t acc = 0;

	// return range product
	if(workflow->raw[0] == 'A'){
		acc = 1;
		acc *= (ranges.param[0].max - ranges.param[0].min + 1);
		acc *= (ranges.param[1].max - ranges.param[1].min + 1);
		acc *= (ranges.param[2].max - ranges.param[2].min + 1);
		acc *= (ranges.param[3].max - ranges.param[3].min + 1);

		return acc; 
	}
	
	workflow_t w;
	
	// search
	for(size_t i = 0; i < sheet->workflowsSize; i++){
		w = sheet->workflows[i];
		if(string_cmp(w.name, workflow))
			break;
	}

	step_t step = w.steps[s];
	const char *xmas = "xmas";
	int param = strchr(xmas, step.param) - xmas;

	// change to fallback
	if(s >= w.stepsSize){
		acc += applyRangeOnWorkflow(sheet, ranges, w.fallback, 0);
	}
	// bigger
	else if(step.cmp == '>'){
		// pass all
		if(ranges.param[param].min > step.value){
			acc += applyRangeOnWorkflow(sheet, ranges, step.jump, 0);
		}
		// pass nothing
		else if(ranges.param[param].max <= step.value){
			acc += applyRangeOnWorkflow(sheet, ranges, workflow, s+1);
		}
		// split
		else{
			ranges_t accepted = ranges;
			ranges_t rejected = ranges;
			accepted.param[param].min = step.value + 1;
			rejected.param[param].max = step.value;

			acc += applyRangeOnWorkflow(sheet, accepted, step.jump, 0);
			acc += applyRangeOnWorkflow(sheet, rejected, workflow, s+1);
		}
	}
	// lesser
	else{
		// pass all
		if(ranges.param[param].max < step.value){
			acc += applyRangeOnWorkflow(sheet, ranges, step.jump, 0);
		}
		// pass nothing
		else if(ranges.param[param].min >= step.value){
			acc += applyRangeOnWorkflow(sheet, ranges, workflow, s+1);
		}
		// split
		else{
			ranges_t accepted = ranges;
			ranges_t rejected = ranges;
			accepted.param[param].max = step.value - 1;
			rejected.param[param].min = step.value;

			acc += applyRangeOnWorkflow(sheet, accepted, step.jump, 0);
			acc += applyRangeOnWorkflow(sheet, rejected, workflow, s+1);
		}
	}

	printf("Workflow: %s, step: %lu - %lu\n", workflow->raw, s, acc);
	return acc;
}

uint64_t part2(const sheet_t *sheet){
	ranges_t ranges = {
		.param = {
			{.min = 1, .max = 4000},
			{.min = 1, .max = 4000},
			{.min = 1, .max = 4000},
			{.min = 1, .max = 4000}
		}
	};

	return applyRangeOnWorkflow(sheet, ranges, string_from("in"), 0);
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

	sheet_t *sheet = parseInput(data);
	
	printf("Part 1: %lu\n", part1(sheet));
	printf("Part 2: %lu\n", part2(sheet));

	string_destroy(data);
	return 0;
}