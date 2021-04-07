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

#ifndef PARSER_H_
#define PARSER_H_

struct tokens;

int pars_tokenize(struct tokens **tokens, const char file_name[]);

void pars_free_tokens(struct tokens *tokens);

int pars_get_num_lines(const struct tokens *tokens);

int pars_get_num_tokens(const struct tokens *tokens, int line);

const char *pars_get_token(const struct tokens *tokens, int line, int num);

#endif /* PARSER_H_ */
