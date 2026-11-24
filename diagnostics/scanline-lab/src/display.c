#include <agon/mos.h>
#include <agon/vdp.h>
#include <stdio.h>
#include <string.h>
#include "editor.h"

static Scene scene;
static Item displayed[SLOTS];
static int visible, defined;
enum { ART=63000, RESTORE=63985, CLEAR=63986, MARK=63987, PRESENT=63989 };
#define LE(x) ((x)&255),((x)>>8)
#define BUF(x) 23,0,0xa0,LE(x)
/* Same buffered feature transaction as Nurples: distinguish absent from 0. */
static void feature(int begin){
    static char set[]={23,0,248,2,0,0,0}, clear[]={23,0,249,2,0}, zero[]={0};
    static char mark[]={BUF(PRESENT),5,2,0,0,1, BUF(RESTORE),48,128,5,0,2,0};
    static char snapshot[]={BUF(MARK),6,64,2,0};
    static char restore[]={BUF(CLEAR),6,2,LE(PRESENT),0,0,0,
                           BUF(RESTORE),6,2,LE(PRESENT),0,0,1};
    if(!begin)mos_puts(restore,sizeof restore,0);
    int ids[]={RESTORE,CLEAR,MARK,PRESENT};
    for(int i=0;i<4;i++)vdp_adv_clear_buffer(ids[i]);
    if(!begin)return;
    vdp_adv_write_block_data(RESTORE,sizeof set,set);
    vdp_adv_write_block_data(CLEAR,sizeof clear,clear);
    vdp_adv_write_block_data(MARK,sizeof mark,mark);
    vdp_adv_write_block_data(PRESENT,1,zero);
    mos_puts(snapshot,sizeof snapshot,0); mos_puts(set,sizeof set,0);
}
static void make_art(void){
    static char pixels[2048];
    unsigned char colors[]={0xcc,0xf0,0xcf,0xf3};
    for(int k=0;k<KINDS;k++){
        int w=widths[k],h=heights[k];
        for(int y=0;y<h;y++)for(int x=0;x<w;x++)
            pixels[y*w+x]=(x==0||y==0||x==w-1||y==h-1)?0xff:colors[k];
        vdp_adv_clear_buffer(ART+k);
        vdp_adv_write_block_data(ART+k,w*h,pixels);
        vdp_adv_select_bitmap(ART+k); vdp_adv_bitmap_from_buffer(w,h,1);
    }
}
static void draw_scene(void){
    int n=scene.count;
    if(n<visible)vdp_activate_sprites(n);
    for(int i=0;i<n;i++){
        Item p=scene.obj[i];
        if(i<visible && !memcmp(&p,displayed+i,sizeof p))continue;
        vdp_select_sprite(i);
        if(i>=defined){
            vdp_clear_sprite();
            for(int k=0;k<KINDS;k++)vdp_adv_add_sprite_bitmap(ART+k);
            vdp_set_hardware_sprite(); defined=i+1;
        }
        vdp_nth_sprite_frame(p.kind); vdp_move_sprite_to(p.x,p.y); vdp_show_sprite();
        displayed[i]=p;
    }
    if(n>visible)vdp_activate_sprites(n);
    visible=n;
    int peak,w; scene_peak(&scene,&peak,&w);
    vdp_cursor_tab(0,3);
    printf("Sprites:%3d  Peak/line:%3d  Max summed width:%5d   %-7s",
           n,peak,w,scene.full?"FULL":"");
    vdp_cursor_tab(0,4);
    if(scene.selected>=0){
        Item p=scene.obj[scene.selected];
        printf("Selected:%3d  X:%3d Y:%3d  %3dx%2d       ",
               scene.selected,p.x,p.y,widths[p.kind],heights[p.kind]);
    }else mos_putstring("Selected: none                             ");
}
int main(void){
    int old=getsysvar_scrMode();
    vdp_mode(20); vdp_get_scr_dims(false);
    for(int i=0;i<90;i++){
        waitvblank();
        if(getsysvar_scrMode()==20 && getsysvar_scrwidth()==WIDTH &&
           getsysvar_scrheight()==HEIGHT)break;
        if(i==89){vdp_mode(old); puts("Scanlab requires 512x384 mode 20."); return 1;}
    }
    vdp_logical_scr_dims(false); vdp_cursor_enable(false); vdp_cls();
    vdp_activate_sprites(0); feature(1); make_art(); scene_init(&scene);
    const char *buttons[]={" 8x8"," 16x16"," 32x32"," 128x16"," Align"," Spread"," Undo"," Trash"};
    for(int i=0;i<8;i++){
        vdp_cursor_tab(i*8,1); vdp_set_text_bg_colour(i%2?4:1);
        printf("%-8s",buttons[i]);
    }
    vdp_set_text_bg_colour(0); draw_scene();
    vdp_mouse_enable(); vdp_mouse_set_position(256,16);
    for(;;){
        waitvblank();
        int x=getsysvar_mouseX(), y=getsysvar_mouseY();
        if(x>=WIDTH)x=WIDTH-1;
        if(y>=HEIGHT)y=HEIGHT-1;
        int r=scene_input(&scene,x,y,getsysvar_mouseButtons()&1,mos_getkbmap()[14]&1);
        if(r<0)break;
        if(r)draw_scene();
    }
    vdp_mouse_disable(); vdp_activate_sprites(0);
    for(int i=0;i<defined;i++){vdp_select_sprite(i);vdp_clear_sprite();vdp_set_software_sprite();}
    feature(0);
    for(int k=0;k<KINDS;k++)vdp_adv_clear_buffer(ART+k);
    vdp_mode(old); vdp_cursor_enable(true);
    return 0;
}
