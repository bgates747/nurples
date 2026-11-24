#include "editor.h"
#include <string.h>

const int widths[]={8,16,32,128}, heights[]={8,16,32,16};
enum { NONE, NEW, MOVE, TRASH_CLICK };
static int over_trash(int x,int y){return x>=448 && x<WIDTH && y>=0 && y<24;}
static void delete_selected(Scene *s){
    int i=s->selected;
    if(i<0 || i>=s->count)return;
    memmove(s->obj+i,s->obj+i+1,(s->count-i-1)*sizeof(Item));
    s->count--; s->selected=-1; s->undo=0;
}
static int clamp(int x,int lo,int hi){return x<lo?lo:x>hi?hi:x;}
void scene_init(Scene *s){memset(s,0,sizeof *s); s->selected=-1;}
static void locate(Item *p,int x,int y){
    p->x=clamp(x,0,WIDTH-widths[p->kind]);
    p->y=clamp(y,BAR,HEIGHT-heights[p->kind]);
}
static void arrange(Scene *s,int stagger){
    memcpy(s->saved,s->obj,s->count*sizeof(Item));
    s->undo=1;
    for(int i=0;i<s->count;i++)
        s->obj[i].y=stagger?BAR+(i*32)%(HEIGHT-BAR-32):BAR+64;
}
int scene_input(Scene *s,int x,int y,int down,int esc){
    int press=down&&!s->last_down, release=!down&&s->last_down;
    int escape=esc&&!s->last_esc;
    s->last_down=down; s->last_esc=esc;
    if(escape){
        if(s->cap==NONE)return -1;
        if(s->cap==MOVE)locate(&s->obj[s->selected],s->ox,s->oy);
        s->cap=NONE; return 1;
    }
    if(press){
        s->full=0;
        if(y<BAR){
            if(y>=24)return 0; /* status, not buttons */
            int button=x/64;
            if(over_trash(x,y))s->cap=TRASH_CLICK;
            if(button<KINDS){s->kind=button; s->cap=NEW;}
            if(button==4&&s->count)arrange(s,0);
            if(button==5&&s->count)arrange(s,1);
            if(button==6&&s->undo){
                memcpy(s->obj,s->saved,s->count*sizeof(Item)); s->undo=0;
            }
            return 1;
        }
        /* Later slots overlay earlier slots. Match selection to draw order. */
        s->selected=-1;
        for(int i=s->count-1;i>=0;i--){
            Item p=s->obj[i]; int dx=x-p.x,dy=y-p.y;
            if(dx<0||dy<0||dx>=widths[p.kind]||dy>=heights[p.kind])continue;
            s->selected=i; s->ox=p.x; s->oy=p.y;
            s->gx=dx; s->gy=dy; s->cap=MOVE; break;
        }
        return 1;
    }
    if(release){
        int cap=s->cap; s->cap=NONE;
        if(cap==TRASH_CLICK){
            if(over_trash(x,y))delete_selected(s);
        }else if(cap==NEW&&y>=BAR){
            if(s->count==SLOTS){s->full=1;return 1;}
            s->undo=0; s->selected=s->count++;
            Item *p=&s->obj[s->selected]; p->kind=s->kind;
            locate(p,x-widths[p->kind]/2,y-heights[p->kind]/2);
        }else if(cap==MOVE){
            int i=s->selected;
            if(over_trash(x,y)){
                delete_selected(s);
            }else if(y<BAR)locate(&s->obj[i],s->ox,s->oy);
            else {
                locate(&s->obj[i],x-s->gx,y-s->gy);
                if(s->obj[i].x!=s->ox||s->obj[i].y!=s->oy)s->undo=0;
            }
        }
        return cap!=NONE;
    }
    if(down&&s->cap==MOVE&&y>=BAR){
        Item *p=&s->obj[s->selected]; int px=p->x,py=p->y;
        locate(p,x-s->gx,y-s->gy); return px!=p->x||py!=p->y;
    }
    return 0;
}
void scene_peak(const Scene *s,int *max_objects,int *max_width){
    static int counts[HEIGHT+1],width[HEIGHT+1];
    memset(counts,0,sizeof counts); memset(width,0,sizeof width);
    for(int i=0;i<s->count;i++){
        Item p=s->obj[i]; int end=p.y+heights[p.kind];
        counts[p.y]++;counts[end]--;width[p.y]+=widths[p.kind];width[end]-=widths[p.kind];
    }
    int n=0,w=0; *max_objects=*max_width=0;
    for(int y=BAR;y<HEIGHT;y++){
        n+=counts[y]; w+=width[y];
        if(n>*max_objects)*max_objects=n;
        if(w>*max_width)*max_width=w;
    }
}
