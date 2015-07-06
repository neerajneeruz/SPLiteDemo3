//
//  TestSpatialite2AppDelegate.h
//  TestSpatialite2
//
//  Created by Lionel Gueganton on 1/29/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <spatialite/sqlite3.h>
#import <spatialite.h>

@interface SpatialiteTester : NSObject {
    sqlite3* database;
}

-(void) startTests;

@end

