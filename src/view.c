#include <stdio.h>
#include "labwc.h"
#include "ssd.h"

void
view_move_resize(struct view *view, struct wlr_box geo)
{
	view->impl->configure(view, geo);
}

void
view_move(struct view *view, double x, double y)
{
	view->impl->move(view, x, y);
}

void
view_minimize(struct view *view)
{
	if (view->minimized == true) {
		return;
	}
	view->minimized = true;
	view->impl->unmap(view);
}

void
view_unminimize(struct view *view)
{
	if (view->minimized == false) {
		return;
	}
	view->minimized = false;
	view->impl->map(view);
}

/*
 * view_output - return the output that a view is mostly on
 */
static struct wlr_output *
view_output(struct view *view)
{
	struct wlr_output_layout *layout = view->server->output_layout;
	struct wlr_output *output;

	/* TODO: make this a bit more sophisticated */
	output = wlr_output_layout_output_at(layout, view->x + view->w / 2,
					     view->y + view->h / 2);
	return output;
}

void
view_maximize(struct view *view, bool maximize)
{
	if (view->maximized == maximize) {
		return;
	}
	view->impl->maximize(view, maximize);
	if (maximize) {
		struct wlr_output *output = view_output(view);
		if (!output) {
			return;
		}

		struct wlr_output_layout *layout = view->server->output_layout;
		struct wlr_output_layout_output* ol_output =
			wlr_output_layout_get(layout, output);

		view->unmaximized_geometry.x = view->x;
		view->unmaximized_geometry.y = view->y;
		view->unmaximized_geometry.width = view->w;
		view->unmaximized_geometry.height = view->h;

		struct wlr_box box = {
			.x = ol_output->x,
			.y = ol_output->y,
			.width = output->width,
			.height = output->height,
		};
		if (view->ssd.enabled) {
			struct border border = ssd_thickness(view);
			box.x += border.left;
			box.y += border.top;
			box.width -= border.right + border.left;
			box.height -= border.top + border.bottom;
		}
		box.width /= output->scale;
		box.height /= output->scale;
		view_move_resize(view, box);
		view_move(view, box.x, box.y);
		view->maximized = true;
	} else {
		/* unmaximize */
		view_move_resize(view, view->unmaximized_geometry);
		view->maximized = false;
	}
}

void
view_for_each_surface(struct view *view, wlr_surface_iterator_func_t iterator,
		void *user_data)
{
	view->impl->for_each_surface(view, iterator, user_data);
}

void
view_for_each_popup_surface(struct view *view, wlr_surface_iterator_func_t iterator,
		void *data)
{
	if (!view->impl->for_each_popup_surface) {
		return;
	}
	view->impl->for_each_popup_surface(view, iterator, data);
}

