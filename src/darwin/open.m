#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#import "../../include/appling.h"

int
appling_open(const appling_app_t *app, const char *argument) {
  @autoreleasepool {
    NSURL *path = [NSURL fileURLWithPath:[NSString stringWithFormat:@"%s", app->path]];

    NSMutableArray<NSString *> *components = [NSMutableArray arrayWithArray:path.pathComponents];

    while (components.count > 0) {
      NSString *component = [components lastObject];

      if ([component.pathExtension isEqualToString:@"app"]) break;

      [components removeLastObject];
    }

    if (components.count == 0) return -1;

    path = [NSURL fileURLWithPathComponents:components];

    NSWorkspaceOpenConfiguration *configuration = [NSWorkspaceOpenConfiguration configuration];

    configuration.createsNewApplicationInstance = YES;

    if (argument) {
      configuration.arguments = @[ [NSString stringWithFormat:@"%s", argument] ];
    }

    __block NSError *error = nil;

    dispatch_semaphore_t sem = dispatch_semaphore_create(0);

    [[NSWorkspace sharedWorkspace]
      openApplicationAtURL:path
             configuration:configuration
         completionHandler:^(NSRunningApplication *app, NSError *err) {
           error = err;

           dispatch_semaphore_signal(sem);
         }];

    dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);

    dispatch_release(sem);

    if (error) return -1;
  }

  return 0;
}
