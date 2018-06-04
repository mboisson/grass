#include <stdio.h>
#include <grass/gis.h>
#include <grass/glocale.h>

#include "proto.h"

void make_cell(const char *output, int maptype)
{
    FILE *fp;

    fp = G_fopen_new("cell", output);
    if (!fp)
	G_fatal_error(_("Unable to create cell/%s file"), output);

    fclose(fp);

    if (maptype == CELL_TYPE)
	return;

    fp = G_fopen_new("fcell", output);
    if (!fp)
	G_fatal_error(_("Unable to create fcell/%s file"), output);

    fclose(fp);
}

void make_link(const struct input *inputs, int num_inputs,
               const char *output)
{
    int i;
    FILE *fp;

    fp = G_fopen_new_misc("cell_misc", "vrt", output);
    if (!fp)
	G_fatal_error(_("Unable to create cell_misc/%s/vrt file"), output);

    for (i = 0; i < num_inputs; i++) {
	const struct input *p = &inputs[i];
	
	fprintf(fp, "%s@%s\n", p->name, p->mapset);
    }

    fclose(fp);
}

void write_fp_format(const char *output, int maptype)
{
    struct Key_Value *key_val;
    const char *type;
    FILE *fp;

    if (maptype == CELL_TYPE)
	return;

    key_val = G_create_key_value();

    type = (maptype == FCELL_TYPE)
	? "float"
	: "double";
    G_set_key_value("type", type, key_val);

    G_set_key_value("byte_order", "xdr", key_val);

    fp = G_fopen_new_misc("cell_misc", "f_format", output);
    if (!fp)
	G_fatal_error(_("Unable to create cell_misc/%s/f_format file"), output);

    if (G_fwrite_key_value(fp, key_val) < 0)
	G_fatal_error(_("Error writing cell_misc/%s/f_format file"), output);

    fclose(fp);

    G_free_key_value(key_val);
}

void write_fp_quant(const char *output)
{
    struct Quant quant;

    Rast_quant_init(&quant);
    Rast_quant_round(&quant);

    Rast_write_quant(output, G_mapset(), &quant);
}

void create_map(const struct input *inputs, int num_inputs, const char *output,
		struct Cell_head *cellhd, int maptype, DCELL dmin, DCELL dmax,
		int have_stats, struct R_stats *ostats, const char *title)
{
    struct History history;
    struct Categories cats;
    struct Colors colors;

    Rast_put_cellhd(output, cellhd);

    make_cell(output, maptype);

    make_link(inputs, num_inputs, output);

    if (maptype == CELL_TYPE) {
	struct Range range;
	range.min = (CELL)dmin;
	range.max = (CELL)dmax;
	range.first_time = 0;
	Rast_write_range(output, &range);
    }
    else {
	struct FPRange fprange;
	fprange.min = dmin;
	fprange.max = dmax;
	fprange.first_time = 0;
	Rast_write_fp_range(output, &fprange);
	write_fp_format(output, maptype);
	write_fp_quant(output);
    }
    G_remove_misc("cell_misc", "stats", output);
    if (have_stats)
	Rast_write_rstats(output, ostats);

    G_verbose_message(_("Creating support files for %s"), output);
    Rast_short_history(output, "raster", &history);
    Rast_command_history(&history);
    Rast_write_history(output, &history);

    if (Rast_read_colors(inputs[0].name, inputs[0].mapset, &colors) == 1)
	Rast_write_colors(output, G_mapset(), &colors);
    Rast_init_cats(NULL, &cats);
    Rast_write_cats((char *)output, &cats);

    if (title)
	Rast_put_cell_title(output, title);

    G_done_msg(_("Link to raster map <%s> created."), output);
}
