//
//  Spatialite2_3_1_LibEnvAppDelegate.h
//  Spatialite2.3.1_LibEnv
//
//  Created by Lionel Gueganton on 1/29/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class Spatialite2_3_1_LibEnvViewController;

@interface Spatialite2_3_1_LibEnvAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    Spatialite2_3_1_LibEnvViewController *viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet Spatialite2_3_1_LibEnvViewController *viewController;

@end

