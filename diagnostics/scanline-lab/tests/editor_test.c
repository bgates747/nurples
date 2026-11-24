#include "../src/editor.h"
#include <assert.h>

static Scene s;
static int input(int x,int y,int down){return scene_input(&s,x,y,down,0);}
static void drop(int k,int x,int y){input(k*64+4,12,1);input(x,y,0);}
static void button(int id){input(id*64+8,12,1);input(id*64+8,12,0);}
int main(void){
    scene_init(&s);
    drop(0,1,48); drop(3,511,383);
    assert(s.count==2 && s.obj[0].x==0 && s.obj[1].x==384 && s.obj[1].y==368);
    button(4); assert(s.obj[0].y==s.obj[1].y && s.obj[1].x==384);
    int n,w; scene_peak(&s,&n,&w); assert(n==2 && w==136);
    button(5); scene_peak(&s,&n,&w); assert(n==1 && w==128);
    button(6); assert(s.obj[0].y==s.obj[1].y);
    input(390,115,1);input(300,220,1); assert(s.obj[1].x==294 && s.obj[1].y==217);
    assert(scene_input(&s,300,220,1,1)==1 && s.obj[1].x==384);
    assert(scene_input(&s,300,220,0,1)==0);
    input(0,0,0);input(390,115,1);input(470,10,0);assert(s.count==1);
    drop(2,200,200);
    input(1,114,1);input(1,114,0);assert(s.selected==0);
    input(470,12,1);input(430,12,0);assert(s.count==2 && s.selected==0);
    button(7);assert(s.count==1 && s.obj[0].kind==2 && s.selected==-1);
    button(7);assert(s.count==1);
    for(int i=1;i<SLOTS;i++)drop(1,250,250);
    drop(3,250,250);assert(s.count==SLOTS && s.full);
    input(250,250,1); assert(s.selected==SLOTS-1);
    input(250,250,0); assert(input(5,5,0)==0);
    assert(scene_input(&s,10,10,0,1)==-1);
    return 0;
}
