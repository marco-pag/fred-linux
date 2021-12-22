/*
 * Fred for Linux. Experimental support.
 *
 * Copyright (C) 2018-2021, Marco Pagani, ReTiS Lab.
 * <marco.pag(at)outlook.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
*/

// TODO: old code: replace with a safer parser!

#include "parser.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../utils/dbg_print.h"

#define MAX_LINES       256
#define MAX_TOKENS      64
#define MAX_TOKEN_LEN   256

const char seps[] = " ,\t\n";
const char comm = '#';

struct tokens {
    char **lines_tokens[MAX_LINES];     // Arrays
    int tokens_count[MAX_LINES];        // Tokens for each line (opt)
    int lines_count;                    // Numbers of lines (opt)
};

// The line will be modified
static int tokenize_line_(struct tokens *tokens, char *line)
{
    char *saveptr;
    char *token;
    char *lineptr = line;
    int t_count = 0;

    if (!tokens || !line)
        return -1;

    // Remove newline
    line[strcspn(line, "\n")] = 0;
    // Get first token to check if the line is a comment
    token = strtok_r(lineptr, seps, &saveptr);
    if (!token || token[0] == comm)
        return 0;

    // Allocate a new line
    tokens->lines_tokens[tokens->lines_count] = calloc(MAX_TOKENS, sizeof(char *));
    if (!tokens->lines_tokens[tokens->lines_count])
        return -1;

    // And get all others token
    while (token) {
        // Allocate and save a new token
        tokens->lines_tokens[tokens->lines_count][t_count] = strndup(token, MAX_TOKEN_LEN);
        if (!tokens->lines_tokens[tokens->lines_count][t_count])
            return -1;

        // Update line tokens counter
        t_count++;

        // Next token
        lineptr = saveptr;
        token = strtok_r(lineptr, seps, &saveptr);
    }

    // Update line and token counters
    tokens->tokens_count[tokens->lines_count] = t_count;
    tokens->lines_count++;

    return 0;
}

static int parse_file_(struct tokens *tokens, const char filename[])
{
    FILE *file_p;
    char *line = NULL;
    size_t line_len = 0;
    int retval = 0;

    tokens->lines_count = 0;

    DBG_PRINT("fred_sys: pars: parsing file: %s\n", filename);

    file_p = fopen(filename, "r");
    if (!file_p) {
        ERROR_PRINT("fred_sys: pars: error while opening file: %s\n", filename);
        return -1;
    }

    while (getline(&line, &line_len, file_p) != -1) {
        if (tokenize_line_(tokens, line) != 0) {
            ERROR_PRINT("fred_sys: pars: error while parsing file: %s at line: %d\n",
                    filename, tokens->lines_count);
            retval = -1;
            break;
        }
    }

    fclose(file_p);
    if (line)
        free(line);

    return retval;
}

static void free_tokens_(struct tokens *tokens)
{
    // For each line
    for (int l = 0; tokens->lines_tokens[l] && l < MAX_LINES; ++l) {

        // For each token in the line
        for (int t = 0; tokens->lines_tokens[l][t] && t < MAX_TOKENS; ++t) {

            // Free token string
            free(tokens->lines_tokens[l][t]);
        }

        // Free the line of token
        free(tokens->lines_tokens[l]);
    }
}

//---------------------------------------------------------------------------//

int pars_tokenize(struct tokens **tokens, const char file_name[])
{
    int retval;

    *tokens = calloc(1, sizeof(**tokens));
    if (!(*tokens))
        return -1;

    retval = parse_file_(*tokens, file_name);

    if (retval != 0)
        free_tokens_(*tokens);

    return retval;

}

void pars_free_tokens(struct tokens *tokens)
{
    if (!tokens)
        return;

    // Set zero for debug
    free_tokens_(tokens);

    free(tokens);
}

int pars_get_num_lines(const struct tokens *tokens)
{
    return tokens->lines_count;
}

int pars_get_num_tokens(const struct tokens *tokens, int line)
{
    if (line >= tokens->lines_count)
        return 0;

    return tokens->tokens_count[line];
}

const char *pars_get_token(const struct tokens *tokens, int line, int num)
{
    if (line >= tokens->lines_count || num >= tokens->tokens_count[line])
        return NULL;

    return tokens->lines_tokens[line][num];
}
