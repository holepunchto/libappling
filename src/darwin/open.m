#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#import "../../include/appling.h"

int
appling_open (const appling_app_t *app, const char *argument) {
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

    configuration.arguments = @[ [NSString stringWithFormat:@"%s", argument] ];

    [[NSWorkspace sharedWorkspace]
      openApplicationAtURL:path
             configuration:configuration
         completionHandler:nil];
  }

  return 0;
}
