// SPDX-License-Identifier: Apache-2.0

#include <QWindow>
#include <Metal/Metal.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

void setup_hdr_metal(QWindow *win) {
    // Retrieve the native NSView from Qt's window handle
    NSView *nativeView = (__bridge NSView *)reinterpret_cast<void *>(win->winId());
    if (!nativeView) return;

    // Force the view to use a layer
    nativeView.wantsLayer = YES;
    CAMetalLayer *metalLayer = (CAMetalLayer *)nativeView.layer;
    
    if ([metalLayer isKindOfClass:[CAMetalLayer class]]) {
        // 2. Configure for Apple EDR (Extended Dynamic Range)
        metalLayer.wantsExtendedDynamicRangeContent = YES; // Opt into HDR
        metalLayer.pixelFormat = MTLPixelFormatRGBA16Float; // Require 16-bit Float precision
        
        // Apply Extended Linear Display P3 (where 1.0 is SDR white, and >1.0 is HDR)
        /*CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceExtendedLinearDisplayP3);
        metalLayer.colorspace = colorSpace;
        CGColorSpaceRelease(colorSpace);*/
    }
}
