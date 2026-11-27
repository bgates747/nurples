/* Application VDPlet: one bounded span of ordinary poses; no firmware hooks. */
#include "vdplet.h"

/* Current air table [12,44), with 6 animation frames per slot. The eZ80
   fixture must verify those public bounds at build time. */
enum { FIRST_AIR = 12, N_AIR = 32, RECORD = 6, ARG_BYTES = 1 + RECORD*N_AIR };
static uint8_t commands[15*N_AIR];
static uint32_t submissions, poses;

__attribute__((visibility("default")))
int32_t vdplet_entry(const vdplet_host_v1 *h, const vdplet_request_v1 *r) {
    const vdplet_host_v1_1 *v = vdplet_vdu_host(h);
    if (!v || !r || r->struct_bytes < sizeof(*r)) return VDPL_E_ABI;
    if (r->operation == VDPL_INIT) { submissions = poses = 0; return VDPL_OK; }
    if (r->operation == VDPL_FINI) return VDPL_OK;
    if (r->operation != VDPL_RUN || r->entry_id > 1 || r->argument_bytes != ARG_BYTES)
        return VDPL_E_ARGUMENT;
    vdplet_view_v1 input;
    input.struct_bytes = sizeof(input);
    input.lease = 0;
    int32_t rc = h->buffer_acquire(h->context, r->argument_buffer, VDPL_READ, 0, ARG_BYTES, &input);
    if (rc) return rc;
    // One final acknowledged check: silent RUN errors must never yield a
    // falsely "fast" report. Both totals are little-endian 24-bit on eZ80.
    if (r->entry_id == 1) {
        const uint8_t *p = input.data;
        uint32_t a = p[0] | ((uint32_t)p[1]<<8) | ((uint32_t)p[2]<<16);
        uint32_t b = p[3] | ((uint32_t)p[4]<<8) | ((uint32_t)p[5]<<16);
        rc = h->buffer_release(h->context, input.lease);
        return rc ? rc : (a && a==submissions && b==poses ? VDPL_OK : VDPL_E_ARGUMENT);
    }
    unsigned n = input.data[0];
    if (n > N_AIR) rc = VDPL_E_ARGUMENT;
    for (unsigned i=0; !rc && i<n; ++i) {
        const uint8_t *s = input.data+1+i*RECORD;
        if (s[0] < FIRST_AIR || s[0] >= FIRST_AIR+N_AIR || s[1] >= 6) {
            rc = VDPL_E_ARGUMENT;
            break;
        }
        uint8_t *d = commands+i*15;
        d[0]=23; d[1]=27; d[2]=4;  d[3]=s[0];
        d[4]=23; d[5]=27; d[6]=10; d[7]=s[1];
        d[8]=23; d[9]=27; d[10]=13;
        d[11]=s[2]; d[12]=s[3]; d[13]=s[4]; d[14]=s[5];
    }
    int32_t release = h->buffer_release(h->context, input.lease);
    if (rc || release || !n) return rc ? rc : release;
    rc = v->vdu_submit(h->context, commands, 15*n);
    if (!rc) { ++submissions; poses += n; }
    return rc;
}
