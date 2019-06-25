/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#ifndef GRAPHS_H_
#define GRAPHS_H_

#include "stdio.h"

#define GRAPH_START_X 56
#define GRAPH_START_Y (480 - 5)
#define GRAPH_Y_LENGHT 100

typedef struct graphs_measurement_struct
{
  uint32_t ui32_sum_value;
} graphs_measurement_t;

typedef struct graphs_struct
{
  uint8_t ui8_title[17];
  uint32_t ui32_data[256 * 4]; // holds up to 1h of data
  uint32_t ui32_graph_data_y_min;
  uint32_t ui32_graph_data_y_min_counter;
  uint32_t ui32_graph_data_y_max;
  uint32_t ui32_graph_data_y_max_counter;
  uint32_t ui32_data_y_rate_per_pixel_x100;
  uint32_t ui32_data_y_last_value;
  uint32_t ui32_draw_x_last_index;
  uint32_t ui32_data_end_index;
  uint32_t ui32_data_start_index;
  graphs_measurement_t measurement;
} graphs_t;

void graphs_draw(void);
void graphs_init(void);
graphs_t *get_graphs(void);

#endif /* GRAPH_H_ */
