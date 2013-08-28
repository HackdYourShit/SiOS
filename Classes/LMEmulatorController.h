//
//  LMViewController.h
//  SiOS
//
//  Created by Lucas Menge on 1/2/12.
//  Copyright (c) 2012 Lucas Menge. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "../SNES9XBridge/SIScreenDelegate.h"

@class LMEmulatorControllerView;

@interface LMEmulatorController : UIViewController <SIScreenDelegate> {
  LMEmulatorControllerView* _customView;
  
  UIActionSheet* _actionSheet;
  
  volatile NSThread* _emulationThread;
  
  NSString* _romFileName;
  NSString* _initialSaveFileName;
}

@property (copy) NSString* romFileName;
@property (copy) NSString* initialSaveFileName;

- (void)startWithROM:(NSString*)romFileName;
- (void)flipFrontbuffer;

@end
