/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
#include "muse_dialog.h"
int muse_dialog_height(int measured,int reserved)
{
    if(measured<=0||reserved<=0)return 0;
    if(measured>reserved)measured=reserved;
    return measured+2*MUSE_DIALOG_PAD_Y;
}
int muse_dialog_rects(int w,int h,muse_dialog_rect_t *out,int capacity)
{
    if(!out||capacity<MUSE_DIALOG_RECT_COUNT||w<32||h<32)return 0;
    static const uint32_t colors[]={MUSE_DIALOG_OUTER,MUSE_DIALOG_GAP,MUSE_DIALOG_INNER,MUSE_DIALOG_FILL};
    int n=0;
    for(int layer=0;layer<4;layer++) {
        int inset=layer*2,cut=8-layer*2;
        int width=w-2*inset,height=h-2*inset,half=cut/2;
        out[n++]=(muse_dialog_rect_t){inset+cut,inset,width-2*cut,height,colors[layer]};
        out[n++]=(muse_dialog_rect_t){inset+half,inset+half,width-2*half,height-2*half,colors[layer]};
        out[n++]=(muse_dialog_rect_t){inset,inset+cut,width,height-2*cut,colors[layer]};
    }
    return n;
}
