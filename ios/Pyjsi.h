#ifdef __cplusplus
#import "react-native-pyjsi.h"
#endif

#ifdef RCT_NEW_ARCH_ENABLED
#import "RNPyjsiSpec.h"

@interface Pyjsi : NSObject <NativePyjsiSpec>
#else
#import <React/RCTBridgeModule.h>

@interface Pyjsi : NSObject <RCTBridgeModule>
#endif

@end
