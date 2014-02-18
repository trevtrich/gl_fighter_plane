//
//  FighterAppAppDelegate.h
//  FighterApp
//
//  Created by Trevor Richardson on 11/3/10.
//  Copyright 2010 Virtual Reality Applications Center. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface FighterAppAppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow *window;
}

@property (assign) IBOutlet NSWindow *window;

@end
