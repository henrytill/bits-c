#import <Foundation/Foundation.h>
#import <stdio.h>
#import <stdlib.h>

@interface Greeter : NSObject {
  NSString *_message;
}
@property(nonatomic, copy) NSString *message;
- (instancetype)initWithMessage:(NSString *)message;
+ (instancetype)defaultGreeter;
- (void)greet;
@end

@implementation Greeter
@synthesize message = _message; // This creates getter and setter methods
- (instancetype)initWithMessage:(NSString *)message {
  self = [super init];
  if (self) {
    _message = [message copy];
  }
  return self;
}
+ (instancetype)defaultGreeter {
  return [[[self alloc] initWithMessage:@"Hello, world!"] autorelease];
}
- (void)greet {
  NSLog(@"%@: %@", NSStringFromClass([self class]), self.message);
}
- (void)dealloc {
  [_message release];
  [super dealloc];
}
@end

int main(int argc, char *argv[]) {
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  Greeter *defaultGreeter = [Greeter defaultGreeter];
  [defaultGreeter greet];
  Greeter *customGreeter = [[[Greeter alloc] initWithMessage:@"Welcome to Objective-C!"] autorelease];
  [customGreeter greet];
  NSLog(@"Property access: %@", customGreeter.message);
  customGreeter.message = @"Foo!";
  [customGreeter greet];
  [pool drain];
  return EXIT_SUCCESS;
}