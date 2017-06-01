//
//  VROViewAR.m
//  ViroRenderer
//
//  Created by Raj Advani on 5/31/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#import "VROViewAR.h"
#import "VRORenderer.h"
#import "VROSceneController.h"
#import "VRORenderDelegateiOS.h"
#import "VROTime.h"
#import "VROEye.h"
#import "VRODriverOpenGLiOS.h"
#import "VROHeadTracker.h"
#import "VROApiKeyValidator.h"
#import "VROApiKeyValidatorDynamo.h"
#import "VROInputControllerCardboardiOS.h"
#include "vr/gvr/capi/include/gvr_audio.h"

@interface VROViewAR () {
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VRORenderDelegateiOS> _renderDelegateWrapper;
    std::shared_ptr<VRODriverOpenGL> _driver;
    std::shared_ptr<gvr::AudioApi> _gvrAudio;
    std::unique_ptr<VROHeadTracker> _headTracker;
    std::unique_ptr<VROEye> _eye;
    CADisplayLink *_displayLink;
    int _frame;
    
    double _suspendedNotificationTime;
}

@property (readwrite, nonatomic) id <VROApiKeyValidator> keyValidator;

@end

@implementation VROViewAR

@dynamic renderDelegate;
@dynamic sceneController;

#pragma mark - Initialization


- (instancetype)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if (self) {
        [self initRenderer];
    }
    return self;
}

- (instancetype)initWithFrame:(CGRect)frame context:(EAGLContext *)context {
    self = [super initWithFrame:frame context:context];
    if (self) {
        [self initRenderer];
    }
    return self;
}

- (void)initRenderer {
    // TODO DisplayLink maintains a strong reference to its target, we have to
    //      create a weak proxy or something similar!
    
    if (!self.context) {
        EAGLContext *context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        self.context = context;
    }
    
    /*
     Setup the animation loop for the GLKView.
     */
    _displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(display)];
    [_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
    
    /*
     Setup the GLKView.
     */
    self.enableSetNeedsDisplay = NO;
    self.drawableColorFormat = GLKViewDrawableColorFormatRGB565;
    self.drawableStencilFormat = GLKViewDrawableStencilFormat8;
    self.drawableDepthFormat = GLKViewDrawableDepthFormat16;
    self.drawableMultisample = GLKViewDrawableMultisample4X;
    
    [EAGLContext setCurrentContext:self.context];
    
    /*
     Disable going to sleep, and setup notifications.
     */
    [UIApplication sharedApplication].idleTimerDisabled = YES;
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(orientationDidChange:)
                                                 name:UIApplicationDidChangeStatusBarOrientationNotification
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillResignActive:)
                                                 name:UIApplicationWillResignActiveNotification
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationDidBecomeActive:)
                                                 name:UIApplicationDidBecomeActiveNotification
                                               object:nil];
    
    /*
     Create Viro renderer objects.
     */
    _frame = 0;
    _gvrAudio = std::make_shared<gvr::AudioApi>();
    _gvrAudio->Init(GVR_AUDIO_RENDERING_BINAURAL_HIGH_QUALITY);
    _driver = std::make_shared<VRODriverOpenGLiOS>(self.context, _gvrAudio);
    
    _eye = std::unique_ptr<VROEye>(new VROEye(VROEyeType::Monocular));
    _headTracker = std::unique_ptr<VROHeadTracker>(new VROHeadTracker());
    _headTracker->startTracking([UIApplication sharedApplication].statusBarOrientation);
    
    _suspendedNotificationTime = VROTimeCurrentSeconds();
    _renderer = std::make_shared<VRORenderer>(std::make_shared<VROInputControllerCardboardiOS>());
    
    self.keyValidator = [[VROApiKeyValidatorDynamo alloc] init];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - Settings and Notifications

- (void)orientationDidChange:(NSNotification *)notification {
    _headTracker->updateDeviceOrientation([UIApplication sharedApplication].statusBarOrientation);
}

- (void)applicationWillResignActive:(NSNotification *)notification {
    _displayLink.paused = YES;
}

- (void)applicationDidBecomeActive:(NSNotification *)notification {
    _displayLink.paused = NO;
}

- (void)setRenderDelegate:(id<VRORenderDelegate>)renderDelegate {
    _renderDelegateWrapper = std::make_shared<VRORenderDelegateiOS>(renderDelegate);
    _renderer->setDelegate(_renderDelegateWrapper);
}

- (void)setVrMode:(BOOL)enabled {
    // No-op in AR mode
}

- (void)setDebugHUDEnabled:(BOOL)enabled {
    _renderer->setDebugHUDEnabled(enabled);
}

#pragma mark - Key Validation

- (void)validateApiKey:(NSString *)apiKey withCompletionBlock:(VROViewValidApiKeyBlock)completionBlock {
    // If the user gives us a key, then let them use the API until we successfully checked the key.
    self.suspended = NO;
    
    __weak typeof(self) weakSelf = self;
    
    VROApiKeyValidatorBlock validatorCompletionBlock = ^(BOOL valid) {
        typeof(weakSelf) strongSelf = weakSelf;
        if (!strongSelf) {
            return;
        }
        
        strongSelf.suspended = !valid;
        completionBlock(valid);
        
        NSLog(@"[ApiKeyValidator] The key is %@!", valid ? @"valid" : @"invalid");
    };
    [self.keyValidator validateApiKey:apiKey withCompletionBlock:validatorCompletionBlock];
}

#pragma mark - Camera

- (void)setPointOfView:(std::shared_ptr<VRONode>)node {
    _renderer->setPointOfView(node);
}

- (void)layoutSubviews {
    [super layoutSubviews];
    _renderer->updateRenderViewSize(self.bounds.size.width, self.bounds.size.height);
}

#pragma mark - Scene Loading

- (void)setSceneController:(std::shared_ptr<VROSceneController>) sceneController {
    _sceneController = sceneController;
    _renderer->setSceneController(sceneController, _driver);
}

- (void)setSceneController:(std::shared_ptr<VROSceneController>)sceneController
                  duration:(float)seconds
            timingFunction:(VROTimingFunctionType)timingFunctionType {
    _sceneController = sceneController;
    _renderer->setSceneController(sceneController, seconds, timingFunctionType, _driver);
}

#pragma mark - Frame Listeners

- (std::shared_ptr<VROFrameSynchronizer>)frameSynchronizer {
    return _renderer->getFrameSynchronizer();
}

#pragma mark - Rendering

- (void)drawRect:(CGRect)rect {
    if (!_headTracker->isReady()) {
        return;
    }
    
    @autoreleasepool {
        if (self.suspended) {
            [self renderSuspended];
        }
        else {
            [self renderFrame];
        }
    }
    
    ++_frame;
    ALLOCATION_TRACKER_PRINT();
}

- (void)renderFrame {
    VROViewport viewport(0, 0, self.bounds.size.width  * self.contentScaleFactor,
                               self.bounds.size.height * self.contentScaleFactor);
    VROFieldOfView fov = _renderer->getMonoFOV(viewport.getWidth(), viewport.getHeight());
    
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE); // Must enable writes to clear depth buffer
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    VROMatrix4f headRotation = _headTracker->getHeadRotation().invert();
    _renderer->prepareFrame(_frame, viewport, fov, headRotation, _driver);
    
    VROMatrix4f projectionMatrix = fov.toPerspectiveMatrix(kZNear, _renderer->getFarClippingPlane());
    VROMatrix4f eyeFromHeadMatrix; // Identity
    
    glViewport(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
    _renderer->renderEye(VROEyeType::Monocular, eyeFromHeadMatrix, projectionMatrix, _driver);
    _renderer->endFrame(_driver);
}

- (void)renderSuspended {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    double newTime = VROTimeCurrentSeconds();
    // notify the user about bad keys 5 times a second (every 200ms/.2s)
    if (newTime - _suspendedNotificationTime > .2) {
        perr("Renderer suspended! Do you have a valid key?");
        _suspendedNotificationTime = newTime;
    }
}

- (void)recenterTracking {
    // TODO Implement this, try to share code with VROSceneRendererCardboardOpenGL; maybe
    //      move the functionality into VRORenderer
}

@end
