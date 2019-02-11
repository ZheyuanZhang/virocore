//
//  VROBloomTest.h
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROBloomTest_h
#define VROBloomTest_h

#include "VRORendererTest.h"

class VROBloomTest : public VRORendererTest, public VROEventDelegate, public std::enable_shared_from_this<VROBloomTest>{
public:
    
    VROBloomTest();
    virtual ~VROBloomTest();
    
    void build(std::shared_ptr<VRORenderer> renderer,
               std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
               std::shared_ptr<VRODriver> driver);
    std::shared_ptr<VRONode> getPointOfView() {
        return _pointOfView;
    }
    std::shared_ptr<VROSceneController> getSceneController() {
        return _sceneController;
    }
    virtual void onClick(int source,
                         std::shared_ptr<VRONode> node,
                         ClickState clickState,
                         std::vector<float> position);

private:
    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VROText> _bloomStateText;
    int _kernelSize;
    float _sigma;
    bool _useBilinearTextureLookup;
    int _blurPasses;
};

#endif /* VROBloomTest_h */
