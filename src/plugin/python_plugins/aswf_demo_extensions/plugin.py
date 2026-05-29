#!/bin/env python
# SPDX-License-Identifier: Apache-2.0

from xstudio.plugin import PluginBase
from xstudio.core import get_global_playhead_events_atom, event_atom, viewport_atom, viewport_playhead_atom, name_atom
from xstudio.core import transform_matrix_atom, viewport_pan_atom, viewport_scale_atom, V2f
from xstudio.api.auxiliary import ActorConnection
from xstudio.api.intrinsic import Viewport
from xstudio.api.module import ModuleBase

# subclass from the Viewport class. This gives us access to the 
# active viewport in the xSTUDIO Main Window and its attribute
# events
class ViewportLocal(Viewport):
    def __init__(self, connection):

        Viewport.__init__(
            self,
            connection,
            active_viewport = True
            )

    def attribute_changed(self, attr, role):
        print("ViewportLocal: {} changed to {}".format(attr.name, attr.value()))

    @property
    def name(self):
        return self.connection.request_receive(self.remote, name_atom())[0]

    @property
    def pan(self):
        return self.connection.request_receive(self.remote, viewport_pan_atom())[0]

    @pan.setter
    def pan(self, value):
        self.connection.send(self.remote, viewport_pan_atom(), value)

    @property
    def scale(self):
        return self.connection.request_receive(self.remote, viewport_scale_atom())[0]

    @scale.setter
    def scale(self, value):
        self.connection.request_receive(self.remote, viewport_scale_atom(), value)[0]

# Similar to the ViewportLocal class above, we can also make a class that attaches
# to a playhead in the backend. This lets us get attribute updates from the 
# playhead.
class PlayheadLocal(ModuleBase):

    def __init__(self, connection, playhead_remote):

        ModuleBase.__init__(
            self,
            connection,
            playhead_remote
            )

    def attribute_changed(self, attr, role):
        print("PlayheadLocal: {} changed to {}".format(attr.name, attr.value()))        

class ASWFExtensionPlugin(PluginBase):

    def __init__(self, connection):

        PluginBase.__init__(
            self,
            connection,
            name="DemoPluginPython"
            )

        self.menu_id = self.insert_menu_item(
            menu_model_name="main menu bar",
            menu_text="Reset Pan/Zoom",
            menu_path="ASWF Demo Extensions",
            menu_item_position=100.0,
            callback=self.reset_pan_zoom)

        self.set_submenu_position(
            menu_model_name="main menu bar",
            submenu_path="ASWF Demo Extensions",
            menu_item_position=10.0)

        # Subscribe to global playhead events. We will get various
        # events relating to the creation of viewports, viewports
        # being attached to playheads, and on-screen media changing
        # plus quite granular events like frame changes on playhead.        
        self.subscribe_to_global_playhead_events(
            self.on_global_playhead_event
        )
        try:
            self.viewport = ViewportLocal(connection)
        except:
            # plugin instanced before a viewport has been created.
            self.viewport = None
        self.viewport_playhead = None

    def on_global_playhead_event(self, event):

        if isinstance(event[0], event_atom) and isinstance(event[1], viewport_atom)\
            and isinstance(event[2], str):
            # this message comes in when a viewport is created - we re-instance our
            # ViewportLocal to ensure we are watching the current 'active' viewport
            # since xSTUDIO can have multiple viewports and they can be created
            # ands destroyed by the user as the switch or change layouts in the GUI.
            self.viewport = ViewportLocal(self.connection)        

        if isinstance(event[0], event_atom) and isinstance(event[1], viewport_playhead_atom):
            # this message is telling us the playhead attached to the given viewport has change.
            if event[2] == self.viewport.name:
                # if the viewport in question is the 'active' viewport that we are already
                # watching then we update our playhead connection to the new playheadßßß
                self.viewport_playhead = PlayheadLocal(self.connection, event[3])

        if isinstance(event[0], event_atom) and isinstance(event[1], viewport_atom) and isinstance(event[2], transform_matrix_atom):
            print ("Transform matrix changed ", str(event[4]))
            print ("PAN ", self.viewport.pan, "SCALE ", self.viewport.scale)

    def reset_pan_zoom(self):
        if self.viewport:
            self.viewport.pan = V2f(0.0, 0.0)
            self.viewport.scale = 1.0

# This method is required by xSTUDIO
def create_plugin_instance(connection):
    return ASWFExtensionPlugin(connection)
