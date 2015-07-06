//
//  TestSpatialite2AppDelegate.m
//  TestSpatialite2
//
//  Created by Lionel Gueganton on 1/29/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "SpatialiteTester.h"

@interface SpatialiteTester(privateAPI)
-(void) initDB;
-(void) testPaca1;
-(void) testGeos;

@end


@implementation SpatialiteTester

-(id) init {
	if (self != [super init])
		return nil;
	return self;
}

-(void) startTests {
	NSLog(@"Spatialite version %s", spatialite_version ());
	[self initDB];
	[self testPaca1];
	[self testGeos];
}

-(void) initDB {
	spatialite_init (1);
	//NSString *resPath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"test-2.3.sqlite"];
	NSString *resPath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"pacaRoadsIndex.sqlite"];
	NSLog(@"Opening %@", resPath);
	
	int ret = sqlite3_open_v2 ([resPath cStringUsingEncoding:NSUTF8StringEncoding], &database, SQLITE_OPEN_READONLY, NULL);
	if (ret != SQLITE_OK)
		NSLog(@"error");
	else
		NSLog(@"OPEN OK");
	//return self;
}

-(void) testPaca1 {
	//43.581442,7.119527
	//43.597625,7.116877
	//43.557964,7.013891
	double lat =43.557964;//43.580722;
	double lon =7.013891;//7.120872;
	//CLLocation* location = [[CLLocation alloc] initWithLatitude:lat longitude:lon];
	NSLog(@"================================ TEST1");
	//const char *sql = "SELECT NumPoints(Geometry) FROM Roads WHERE PK_UID=4";
	//SELECT * FROM Roads Where MbrContains(Geometry, MakePoint(7.119527, 43.581442))
	//NSString* req = [NSString stringWithFormat:@"SELECT AsText(GeomFromText('POINT(%.0f %.0f)', %d))", lat*1000000, lon*1000000, 4326];
	NSString* req = [NSString stringWithFormat:@"SELECT * FROM Roads Where MbrContains(Geometry, MakePoint(%f, %f))", lon, lat, 4326];
	//SELECT * FROM Roads Where MbrContains(Geometry, MakePoint(7.120872,43.580722,4326))
	NSLog(@"req is %@", req);
	const char *sql = [req cStringUsingEncoding:NSUTF8StringEncoding];
	sqlite3_stmt *statement;
	int returnValue = sqlite3_prepare_v2(database, sql, -1, &statement, NULL);
	if (returnValue == SQLITE_OK) {
		// We "step" through the results - once for each row.
		while (sqlite3_step(statement) == SQLITE_ROW) {
			char* nameAsChar = (char*)sqlite3_column_text(statement, 2);
			NSString* name  =@"-";
			if (nameAsChar!=NULL)
				name = [NSString stringWithUTF8String:nameAsChar];
			NSLog(@"%@", name);
			//textView.text=[textView.text stringByAppendingString:[NSString stringWithFormat:@"%@\n", name]];
			
		}
	}
}

-(void) testGeos {
	sqlite3_stmt *statement;
	int returnValue = sqlite3_prepare_v2(database, "SELECT HEX(AsBinary(GeomFromText('POINT(10 20)')));", -1, &statement, NULL);
	if (returnValue == SQLITE_OK) {
		// We "step" through the results - once for each row.
		while (sqlite3_step(statement) == SQLITE_ROW) {
			char* resultAsChar = (char*)sqlite3_column_text(statement, 0);
			NSString* name  =@"-";
			if (resultAsChar!=NULL)
				name = [NSString stringWithUTF8String:resultAsChar];
			NSLog(@"%@", name);
			//textView.text=[textView.text stringByAppendingString:[NSString stringWithFormat:@"%@\n", name]];
			
		}
	}
}

- (void)dealloc {
	[super dealloc];
}


@end

