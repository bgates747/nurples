#ifndef SCANLAB_EDITOR_H
#define SCANLAB_EDITOR_H

enum { SLOTS=255, WIDTH=512, HEIGHT=384, BAR=48, KINDS=4 };
typedef struct { int x,y,kind; } Item;
typedef struct {
    Item obj[SLOTS], saved[SLOTS];
    int count, selected, undo, cap, kind, ox,oy, gx,gy;
    int last_down,last_esc, full;
} Scene;
extern const int widths[KINDS], heights[KINDS];
void scene_init(Scene *s);
/* Return 0 for no display change, 1 for changed, -1 for exit. */
int scene_input(Scene *s,int x,int y,int down,int esc);
void scene_peak(const Scene *s,int *max_objects,int *max_width);
#endif
