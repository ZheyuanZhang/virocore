package com.viro.renderer.jni.event;

import com.viro.renderer.jni.Node;

/**
 * Callback interface for responding to touchpad swipe events, which occur when a finger is
 * swiped across a touchpad Controller.
 */
public interface TouchpadSwipeListener {

    /**
     * Callback when a swipe was been detected on a touchpad controller while hovering over
     * the {@link Node} receiving this event.
     *
     * @param source     The platform specific source ID, which indicates what button or
     *                   component on the Controller triggered the event. See the Controller's
     *                   Guide for information.
     * @param node       The {@link Node} being hovered over at the time of the swipe.
     * @param swipeState The direction of the swipe recorded.
     */
    void onSwipe(int source, Node node, SwipeState swipeState);
}