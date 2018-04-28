//
//  ARUtils_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ANDROID_ARUTILS_JNI_H
#define ANDROID_ARUTILS_JNI_H

#include <VROARPlaneAnchor.h>
#include <VROARHitTestResult.h>

#include "VRODefines.h"
#include VRO_C_INCLUDE

// Helper functions to create a Java ARAnchor object
VRO_OBJECT ARUtilsCreateJavaARAnchorFromAnchor(std::shared_ptr<VROARAnchor> anchor);
VRO_FLOAT_ARRAY ARUtilsCreateFloatArrayFromVector3f(VROVector3f vector);
VRO_FLOAT_ARRAY ARUtilsCreateFloatArrayFromMatrix(VROMatrix4f matrix);
VRO_FLOAT_ARRAY ARUtilsCreateFloatArrayFromBoundingBox(VROBoundingBox boundingBox);
VRO_STRING ARUtilsCreateStringFromAlignment(VROARPlaneAlignment alignment);
VRO_OBJECT ARUtilsCreateARHitTestResult(VROARHitTestResult result);
VRO_FLOAT_ARRAY ARUtilsCreatePointsArray(std::vector<VROVector3f> points);

// Helper functions to create a Java ARPointCloud object
VRO_OBJECT ARUtilsCreateARPointCloud(std::shared_ptr<VROARPointCloud> pointCloud);


#endif //ANDROID_ARUTILS_JNI_H
