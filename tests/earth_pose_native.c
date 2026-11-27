/* Portable host ABI mock. The separately linked Xtensa package must use
   the official SDK and the real runtime validator on hardware. */
#include <assert.h>
#include <string.h>
#include "vdplet.h"
extern int32_t vdplet_entry(const vdplet_host_v1 *, const vdplet_request_v1 *);
static uint8_t data[193], expected[480];
static unsigned expecting, borrowed, fail_submit;
static int32_t acquire(void *c, uint16_t id, uint16_t access, uint32_t offset,
                       uint32_t n, vdplet_view_v1 *out) {
    assert(!c && id==23 && !borrowed && access==VDPL_READ && !offset && n==193);
    borrowed=1; out->lease=7; out->bytes=n; out->data=data; return VDPL_OK;
}
static int32_t release(void *c, uint32_t lease) {
    assert(!c && borrowed && lease==7); borrowed=0; return VDPL_OK;
}
static int32_t submit(void *c,const uint8_t *p,uint32_t n) {
    assert(!c && !borrowed && n==expecting && !memcmp(p,expected,n));
    return fail_submit? VDPL_E_BUSY:VDPL_OK;
}
int main(void) {
    vdplet_host_v1_1 h={0};
    h.base.struct_bytes=sizeof(h); h.base.abi_major=1; h.base.abi_minor=1;
    h.base.capabilities=5; h.base.buffer_acquire=acquire; h.base.buffer_release=release;
    h.vdu_submit=submit;
    vdplet_request_v1 r={.struct_bytes=sizeof(r),.operation=VDPL_INIT};
    assert(!vdplet_entry(&h.base,&r));
    r.operation=VDPL_RUN; r.argument_buffer=23; r.argument_bytes=193;
    const unsigned counts[]={0,1,12,32,3};
    for (unsigned k=0;k<5;k++) {
        unsigned n=counts[k]; data[0]=n; expecting=15*n;
        for(unsigned i=0;i<n;i++) {
            uint8_t *s=data+1+6*i;
            s[0]=12+(7*i)%32; s[1]=(n+i)%6; s[2]=255-i; s[3]=2*i;
            s[4]=3*i; s[5]=255-i;
            const uint8_t rec[]={23,27,4,s[0],23,27,10,s[1],23,27,13,s[2],s[3],s[4],s[5]};
            memcpy(expected+15*i,rec,15);
        }
        assert(!vdplet_entry(&h.base,&r) && !borrowed);
    }
    fail_submit=1;
    assert(vdplet_entry(&h.base,&r)==VDPL_E_BUSY && !borrowed);
    data[2]=6; assert(vdplet_entry(&h.base,&r)==VDPL_E_ARGUMENT && !borrowed);
    data[1]=44; data[2]=0; assert(vdplet_entry(&h.base,&r)==VDPL_E_ARGUMENT);
    data[0]=33; assert(vdplet_entry(&h.base,&r)==VDPL_E_ARGUMENT && !borrowed);
    memset(data,0,6); data[0]=4; data[3]=48; r.entry_id=1;
    assert(!vdplet_entry(&h.base,&r) && !borrowed);
    data[0]=5; assert(vdplet_entry(&h.base,&r)==VDPL_E_ARGUMENT);
    r.operation=VDPL_FINI; assert(!vdplet_entry(&h.base,&r));
    h.base.capabilities=1; assert(vdplet_entry(&h.base,&r)==VDPL_E_ABI);
}
